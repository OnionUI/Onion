#ifndef THEME_H__
#define THEME_H__

#include <string.h>
#include <SDL/SDL.h>
#include "cJSON/cJSON.h"

#include "utils.h"

#define STR_MAX 256

#define SYSTEM_SETTINGS "/appconfigs/system.json"
#define FALLBACK_FONT "/customer/app/Exo-2-Bold-Italic.ttf"
#define FALLBACK_PATH "/mnt/SDCARD/miyoo/app/"

#define THEME_POWER_0 "power-0%-icon"
#define THEME_POWER_20 "power-20%-icon"
#define THEME_POWER_50 "power-50%-icon"
#define THEME_POWER_80 "power-80%-icon"
#define THEME_POWER_100 "power-full-icon"

typedef struct Theme_BatteryPercentage
{
    bool visible;
    char font[STR_MAX];
    int size;
    SDL_Color color;
    int offset;
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

SDL_Color hex2sdl(char *input) {
	char *ptr;
    if (input[0] == '#')
        input++;
    unsigned long value = strtoul(input, &ptr, 16);
    SDL_Color color = {
    	(value >> 16) & 0xff,
    	(value >> 8) & 0xff,
    	(value >> 0) & 0xff
	};
    return color;
}

bool getThemePath(char* theme_path)
{
    bool ret = false;
	const char *json_str = NULL;

	if (!(json_str = file_readAll(SYSTEM_SETTINGS))) {
        sprintf(theme_path, "%s", FALLBACK_PATH);
		return ret;
    }

	cJSON* json_root = cJSON_Parse(json_str);
	cJSON* json_theme = cJSON_GetObjectItem(json_root, "theme");

    if (json_theme) {
	    strncpy(theme_path, cJSON_GetStringValue(json_theme), STR_MAX-1);
        ret = true;
    }

	cJSON_free(json_root);
	return ret;
}

bool getStringValue(cJSON* root, const char* key, char* dest)
{
    cJSON* json_object = cJSON_GetObjectItem(root, key);
    if (json_object) {
        strncpy(dest, cJSON_GetStringValue(json_object), STR_MAX-1);
        return true;
    }
    return false;
}

bool getBoolValue(cJSON* root, const char* key, bool* dest)
{
    cJSON* json_object = cJSON_GetObjectItem(root, key);
    if (json_object) {
        *dest = cJSON_IsTrue(json_object);
        return true;
    }
    return false;
}

bool getNumberValue(cJSON* root, const char* key, int* dest)
{
    cJSON* json_object = cJSON_GetObjectItem(root, key);
    if (json_object) {
        *dest = cJSON_GetNumberValue(json_object);
        return true;
    }
    return false;
}

bool getColorValue(cJSON* root, const char* key, SDL_Color* dest)
{
    cJSON* json_object = cJSON_GetObjectItem(root, key);
    if (json_object) {
        *dest = hex2sdl(cJSON_GetStringValue(json_object));
        return true;
    }
    return false;
}

void getFontStyle(cJSON* root, FontStyle_s* dest, FontStyle_s* fallback)
{
    if (!getStringValue(root, "font", dest->font) && fallback)
        strcpy(dest->font, fallback->font);
    if (!getNumberValue(root, "size", &dest->size) && fallback)
        dest->size = fallback->size;
    if (!getColorValue(root, "color", &dest->color) && fallback)
        dest->color = fallback->color;
}

void theme_getImagePath(Theme_s* theme, const char* name, char* image_path)
{
    char rel_path[STR_MAX];
    sprintf(rel_path, "skin/%s.png", name);
    sprintf(image_path, "%s%s", theme->path, rel_path);
    if (!file_exists(image_path))
        sprintf(image_path, "%s%s", FALLBACK_PATH, rel_path);
}

SDL_Surface* theme_loadImage(Theme_s* theme, const char* name)
{
    char image_path[512];
    theme_getImagePath(theme, name, image_path);
    return IMG_Load(image_path);
}

TTF_Font* theme_loadFont(Theme_s* theme, const char* font, int size)
{
    char font_path[STR_MAX];
    if (font[0] == '/')
        strncpy(font_path, font, STR_MAX-1);
    else
        snprintf(font_path, STR_MAX, "%s%s", theme->path, font);
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
            .visible = true,
            .font = FALLBACK_FONT,
            .size = 24,
            .color = {255, 255, 255},
            .offset = 0,
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
            .font = FALLBACK_FONT,
            .size = 40,
            .color = {255, 255, 255}
        },
        .total = {
            .font = FALLBACK_FONT,
            .size = 40,
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
    char config_path[STR_MAX];
    snprintf(config_path, STR_MAX, "%s%s", theme.path, "config.json");

	if (!(json_str = file_readAll(config_path)))
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

    getStringValue(json_root, "name", theme.name);
    getStringValue(json_root, "author", theme.author);
    getStringValue(json_root, "description", theme.description);
    getBoolValue(json_root, "hideIconTitle", &theme.hideIconTitle);

    getFontStyle(json_title, &theme.title, NULL);
    getFontStyle(json_hint, &theme.hint, NULL);
    getFontStyle(json_currentpage, &theme.currentpage, &theme.hint);
    getFontStyle(json_total, &theme.total, &theme.hint);

    getStringValue(json_grid, "font", theme.grid.font);
    getNumberValue(json_grid, "grid1x4", &theme.grid.grid1x4);
    getNumberValue(json_grid, "grid3x4", &theme.grid.grid3x4);
    getColorValue(json_grid, "color", &theme.grid.color);
    getColorValue(json_grid, "selectedcolor", &theme.grid.selectedcolor);

    getFontStyle(json_list, &theme.list, NULL);

    getBoolValue(json_batteryPercentage, "visible", &theme.batteryPercentage.visible);
    if (!getStringValue(json_batteryPercentage, "font", theme.batteryPercentage.font))
        strcpy(theme.batteryPercentage.font, theme.hint.font);
    getNumberValue(json_batteryPercentage, "size", &theme.batteryPercentage.size);
    if (!getColorValue(json_batteryPercentage, "color", &theme.batteryPercentage.color))
        theme.batteryPercentage.color = theme.hint.color;
    getNumberValue(json_batteryPercentage, "offset", &theme.batteryPercentage.offset);
    getBoolValue(json_batteryPercentage, "onleft", &theme.batteryPercentage.onleft);

	cJSON_free(json_root);
	return theme;
}

Theme_s loadTheme(void)
{
    char theme_path[STR_MAX];
    getThemePath(theme_path);
    return loadThemeFromPath(theme_path);
}

#endif // THEME_H__
