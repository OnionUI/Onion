#ifndef THEME_LOAD_H__
#define THEME_LOAD_H__

#include <SDL/SDL.h>
#include <SDL/SDL_image.h>
#include <SDL/SDL_ttf.h>

#include "utils/str.h"
#include "utils/file.h"

#define SYSTEM_CONFIG "/appconfigs/system.json"
#define FALLBACK_FONT "/customer/app/Exo-2-Bold-Italic.ttf"
#define FALLBACK_PATH "/mnt/SDCARD/miyoo/app/"
#define SYSTEM_RESOURCES "/mnt/SDCARD/.tmp_update/res/"
#define THEME_OVERRIDES "/mnt/SDCARD/Saves/CurrentProfile/theme"

int theme_getImagePath(const char *theme_path, const char* name, char* out_path)
{
    int load_mode = 2;
    char rel_path[STR_MAX],
         image_path[STR_MAX*2];
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

SDL_Surface* theme_loadImage(const char *theme_path, const char* name)
{
    char image_path[512];
    theme_getImagePath(theme_path, name, image_path);
    return IMG_Load(image_path);
}

TTF_Font* theme_loadFont(const char *theme_path, const char* font, int size)
{
    char font_path[STR_MAX*2];
    if (font[0] == '/')
        strncpy(font_path, font, STR_MAX*2-1);
    else
        snprintf(font_path, STR_MAX*2, "%s%s", theme_path, font);
    return TTF_OpenFont(exists(font_path) ? font_path : FALLBACK_FONT, size);
}

char* theme_getPath(char *theme_path)
{
    return file_parseKeyValue(SYSTEM_CONFIG, "theme", theme_path, ':');
}

#endif // THEME_LOAD_H__
