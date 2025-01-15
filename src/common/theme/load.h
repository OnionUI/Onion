#ifndef THEME_LOAD_H__
#define THEME_LOAD_H__

#include <SDL/SDL.h>
#include <SDL/SDL_image.h>
#include <SDL/SDL_ttf.h>

#include "utils/file.h"
#include "utils/json.h"
#include "utils/str.h"

#define SYSTEM_CONFIG "/mnt/SDCARD/system.json"
#define FALLBACK_FONT "/customer/app/Exo-2-Bold-Italic.ttf"
#define FALLBACK_PATH "/mnt/SDCARD/miyoo/app/"
#define SYSTEM_RESOURCES "/mnt/SDCARD/.tmp_update/res/"
#define THEME_OVERRIDES "/mnt/SDCARD/Saves/CurrentProfile/theme"
#define FALLBACK_THEME_PATH "/mnt/SDCARD/miyoo/app/"

typedef SDL_Surface *(*ScaleSurfaceFunc)(SDL_Surface *surface, double xScale, double yScale, int smoothing);

static ScaleSurfaceFunc scaleSurfaceFunc = NULL;
static double g_scale = 1.0;

void theme_initScaling(double scale, ScaleSurfaceFunc scaleSurface)
{
    g_scale = scale;
    scaleSurfaceFunc = scaleSurface;
}

SDL_Rect theme_scaleRect(SDL_Rect rect)
{
    if (g_scale == 1.0)
        return rect;
    rect.x = (double)rect.x * g_scale;
    rect.y = (double)rect.y * g_scale;
    rect.w = (double)rect.w * g_scale;
    rect.h = (double)rect.h * g_scale;
    return rect;
}

int theme_getImagePath(const char *theme_path, const char *name, char *out_path)
{
    int load_mode = 2;
    char rel_path[STR_MAX], image_path[STR_MAX * 2];
    sprintf(rel_path, "skin/%s.png", name);

    sprintf(image_path, THEME_OVERRIDES "/%s", rel_path);
    bool override_exists = exists(image_path);

    if (!override_exists) {
        load_mode = 1;
        sprintf(image_path, "%s%s", theme_path, rel_path);
        bool theme_exists = exists(image_path);

        if (!theme_exists) {
            load_mode = 0;
            if (strncmp(name, "extra/", 6) == 0) {
                sprintf(rel_path, "%s.png", name + 6);
                sprintf(image_path, "%s%s", SYSTEM_RESOURCES, rel_path);
            }
            else {
                sprintf(image_path, "%s%s", FALLBACK_PATH, rel_path);
            }
        }
    }

    if (out_path)
        sprintf(out_path, "%s", image_path);

    return load_mode;
}

SDL_Surface *theme_loadImage(const char *theme_path, const char *name)
{
    char image_path[512];
    theme_getImagePath(theme_path, name, image_path);

    printf_debug("Loading image: %s\n", image_path);

    SDL_Surface *image = IMG_Load(image_path);

    if (!image) {
        printf_debug("Failed to load image: %s\n", image_path);
        return NULL;
    }

    if (image->format->BitsPerPixel != 32) {
        // Normalize to 32-bit surface with alpha channel
        SDL_Surface *converted = SDL_CreateRGBSurface(SDL_SWSURFACE, image->w, image->h, 32,
                                                      0x000000ff, 0x0000ff00, 0x00ff0000, 0xff000000);
        SDL_BlitSurface(image, NULL, converted, NULL);
        SDL_FreeSurface(image);
        image = converted;
    }

    if (g_scale != 1.0 && scaleSurfaceFunc) {
        SDL_Surface *scaled = scaleSurfaceFunc(image, g_scale, g_scale, 1);
        SDL_FreeSurface(image);
        image = scaled;
    }

    return image;
}

TTF_Font *theme_loadFont(const char *theme_path, const char *font, int size)
{
    char font_path[STR_MAX * 2];
    if (font[0] == '/')
        strncpy(font_path, font, STR_MAX * 2 - 1);
    else
        snprintf(font_path, STR_MAX * 2, "%s%s", theme_path, font);
    if (g_scale != 1.0)
        size = (int)(size * g_scale);
    return TTF_OpenFont(exists(font_path) ? font_path : FALLBACK_FONT, size);
}

char *theme_getPath(char *theme_path)
{
    cJSON *j = json_load(SYSTEM_CONFIG);
    json_getString(j, "theme", theme_path);
    cJSON_Delete(j);

    if (strcmp(theme_path, "./") == 0 || !is_dir(theme_path)) {
        strcpy(theme_path, FALLBACK_THEME_PATH);
    }

    return theme_path;
}

#endif // THEME_LOAD_H__
