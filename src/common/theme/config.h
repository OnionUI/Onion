#ifndef THEME_CONFIG_H__
#define THEME_CONFIG_H__

#include <string.h>
#include <SDL/SDL.h>
#include <SDL/SDL_image.h>
#include <SDL/SDL_ttf.h>

#include "utils/utils.h"
#include "utils/json.h"
#include "color.h"

#define FALLBACK_FONT "/customer/app/Exo-2-Bold-Italic.ttf"
#define FALLBACK_PATH "/mnt/SDCARD/miyoo/app/"
#define SYSTEM_RESOURCES "/mnt/SDCARD/.tmp_update/res/"

typedef struct Theme_BatteryPercentage
{
    bool visible;
    char font[STR_MAX];
    int size;
    SDL_Color color;
    int offsetX;
    int offsetY;
    bool onleft;
} BatteryPercentage_s;

typedef struct Theme_FontStyle
{
    char font[STR_MAX];
    int size;
    SDL_Color color;
} FontStyle_s;

typedef struct Theme_GridStyle
{
    char font[STR_MAX];
    int grid1x4;
    int grid3x4;
    SDL_Color color;
    SDL_Color selectedcolor;
} GridStyle_s;

typedef struct Theme
{
	char path[STR_MAX];
    char name[STR_MAX];
    char author[STR_MAX];
    char description[STR_MAX];
    bool hideIconTitle;
    BatteryPercentage_s batteryPercentage;
    FontStyle_s title;
    FontStyle_s hint;
    FontStyle_s currentpage;
    FontStyle_s total;
    GridStyle_s grid;
    FontStyle_s list;
} Theme_s;

// Extend json getters

bool json_color(cJSON* root, const char* key, SDL_Color* dest)
{
    cJSON* json_object = cJSON_GetObjectItem(root, key);
    if (json_object) {
        *dest = hex2sdl(cJSON_GetStringValue(json_object));
        return true;
    }
    return false;
}

void json_fontStyle(cJSON* root, FontStyle_s* dest, FontStyle_s* fallback)
{
    if (!json_string(root, "font", dest->font) && fallback)
        strcpy(dest->font, fallback->font);
    if (!json_int(root, "size", &dest->size) && fallback)
        dest->size = fallback->size;
    if (!json_color(root, "color", &dest->color) && fallback)
        dest->color = fallback->color;
}

bool theme_getImagePath(Theme_s* theme, const char* name, char* out_path)
{
    char rel_path[STR_MAX], image_path[STR_MAX*2];
    sprintf(rel_path, "skin/%s.png", name);
    sprintf(image_path, "%s%s", theme->path, rel_path);
    bool exists = file_exists(image_path);

    if (!exists) {
        if (strncmp(name, "extra/", 6) == 0) {
            sprintf(rel_path, "%s.png", name + 6);
            sprintf(image_path, "%s%s", SYSTEM_RESOURCES, rel_path);
        }
        else {
            sprintf(image_path, "%s%s", FALLBACK_PATH, rel_path);
        }
    }

    if (out_path)
        sprintf(out_path, "%s", image_path);
    
    return exists;
}

SDL_Surface* theme_loadImage(Theme_s* theme, const char* name)
{
    char image_path[512];
    theme_getImagePath(theme, name, image_path);
    return IMG_Load(image_path);
}

TTF_Font* theme_loadFont(Theme_s* theme, const char* font, int size)
{
    char font_path[STR_MAX*2];
    if (font[0] == '/')
        strncpy(font_path, font, STR_MAX*2-1);
    else
        snprintf(font_path, STR_MAX*2, "%s%s", theme->path, font);
    return TTF_OpenFont(file_exists(font_path) ? font_path : FALLBACK_FONT, size);
}

Theme_s loadThemeFromPath(const char* theme_path)
{
    Theme_s theme = {
        .path = FALLBACK_PATH,
        .name = "",
        .author = "",
        .description = "",
        .hideIconTitle = false,
        .batteryPercentage = {
            .visible = false,
            .font = FALLBACK_FONT,
            .size = 24,
            .color = {255, 255, 255},
            .offsetX = 0,
            .offsetY = 0,
            .onleft = false
        },
        .title = {
            .font = FALLBACK_FONT,
            .size = 36,
            .color = {255, 255, 255}
        },
        .hint = {
            .font = FALLBACK_FONT,
            .size = 40,
            .color = {255, 255, 255}
        },
        .currentpage = {
            .color = {255, 255, 255}
        },
        .total = {
            .color = {255, 255, 255}
        },
        .grid = {
            .font = FALLBACK_FONT,
            .grid1x4 = 24,
            .grid3x4 = 18,
            .color = {104, 104, 104},
            .selectedcolor = {255, 255, 255}
        },
        .list = {
            .font = FALLBACK_FONT,
            .size = 24,
            .color = {255, 255, 255}
        }
    };

    strncpy(theme.path, theme_path, STR_MAX-1);
    int len = strlen(theme.path);
    if (theme.path[len-1] != '/') {
        theme.path[len] = '/';
        theme.path[len+1] = '\0';
    }

	const char *json_str = NULL;
    char config_path[STR_MAX * 2];
    snprintf(config_path, STR_MAX * 2 - 1, "%s%s", theme.path, "config.json");

	if (!(json_str = file_read(config_path)))
		return theme;

    // Get JSON objects
	cJSON* json_root = cJSON_Parse(json_str);
	cJSON* json_batteryPercentage = cJSON_GetObjectItem(json_root, "batteryPercentage");
	cJSON* json_title = cJSON_GetObjectItem(json_root, "title");
	cJSON* json_hint = cJSON_GetObjectItem(json_root, "hint");
	cJSON* json_currentpage = cJSON_GetObjectItem(json_root, "currentpage");
	cJSON* json_total = cJSON_GetObjectItem(json_root, "total");
	cJSON* json_grid = cJSON_GetObjectItem(json_root, "grid");
	cJSON* json_list = cJSON_GetObjectItem(json_root, "list");

    json_string(json_root, "name", theme.name);
    json_string(json_root, "author", theme.author);
    json_string(json_root, "description", theme.description);
    json_bool(json_root, "hideIconTitle", &theme.hideIconTitle);

    json_fontStyle(json_title, &theme.title, NULL);
    json_fontStyle(json_hint, &theme.hint, &theme.title);
    json_fontStyle(json_currentpage, &theme.currentpage, &theme.hint);
    json_fontStyle(json_total, &theme.total, &theme.hint);
    json_fontStyle(json_list, &theme.list, &theme.title);

    json_string(json_grid, "font", theme.grid.font);
    json_int(json_grid, "grid1x4", &theme.grid.grid1x4);
    json_int(json_grid, "grid3x4", &theme.grid.grid3x4);
    json_color(json_grid, "color", &theme.grid.color);
    json_color(json_grid, "selectedcolor", &theme.grid.selectedcolor);

    json_bool(json_batteryPercentage, "visible", &theme.batteryPercentage.visible);
    if (!json_string(json_batteryPercentage, "font", theme.batteryPercentage.font))
        strcpy(theme.batteryPercentage.font, theme.hint.font);
    json_int(json_batteryPercentage, "size", &theme.batteryPercentage.size);
    if (!json_color(json_batteryPercentage, "color", &theme.batteryPercentage.color))
        theme.batteryPercentage.color = theme.hint.color;
    json_int(json_batteryPercentage, "offsetX", &theme.batteryPercentage.offsetX);
    json_int(json_batteryPercentage, "offsetY", &theme.batteryPercentage.offsetY);
    json_bool(json_batteryPercentage, "onleft", &theme.batteryPercentage.onleft);

	cJSON_free(json_root);
	return theme;
}

#endif // THEME_CONFIG_H__
