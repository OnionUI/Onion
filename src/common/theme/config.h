#ifndef THEME_CONFIG_H__
#define THEME_CONFIG_H__

#include <string.h>
#include <SDL/SDL.h>
#include <SDL/SDL_image.h>
#include <SDL/SDL_ttf.h>

#include "utils/utils.h"
#include "utils/json.h"
#include "color.h"
#include "load.h"

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
    if (!json_getString(root, "font", dest->font) && fallback)
        strcpy(dest->font, fallback->font);
    if (!json_getInt(root, "size", &dest->size) && fallback)
        dest->size = fallback->size;
    if (!json_color(root, "color", &dest->color) && fallback)
        dest->color = fallback->color;
}

bool theme_applyConfig(Theme_s* config, const char* config_path, bool use_fallbacks)
{
	const char *json_str = NULL;

    if (!(json_str = file_read(config_path)))
		return false;

    // Get JSON objects
	cJSON* json_root = cJSON_Parse(json_str);
	cJSON* json_batteryPercentage = cJSON_GetObjectItem(json_root, "batteryPercentage");
	cJSON* json_title = cJSON_GetObjectItem(json_root, "title");
	cJSON* json_hint = cJSON_GetObjectItem(json_root, "hint");
	cJSON* json_currentpage = cJSON_GetObjectItem(json_root, "currentpage");
	cJSON* json_total = cJSON_GetObjectItem(json_root, "total");
	cJSON* json_grid = cJSON_GetObjectItem(json_root, "grid");
	cJSON* json_list = cJSON_GetObjectItem(json_root, "list");

    json_getString(json_root, "name", config->name);
    json_getString(json_root, "author", config->author);
    json_getString(json_root, "description", config->description);
    json_getBool(json_root, "hideIconTitle", &config->hideIconTitle);

    json_fontStyle(json_title, &config->title, NULL);
    json_fontStyle(json_hint, &config->hint, use_fallbacks ? &config->title : NULL);
    json_fontStyle(json_currentpage, &config->currentpage, use_fallbacks ? &config->hint : NULL);
    json_fontStyle(json_total, &config->total, use_fallbacks ? &config->hint : NULL);
    json_fontStyle(json_list, &config->list, use_fallbacks ? &config->title : NULL);

    json_getString(json_grid, "font", config->grid.font);
    json_getInt(json_grid, "grid1x4", &config->grid.grid1x4);
    json_getInt(json_grid, "grid3x4", &config->grid.grid3x4);
    json_color(json_grid, "color", &config->grid.color);
    json_color(json_grid, "selectedcolor", &config->grid.selectedcolor);

    json_getBool(json_batteryPercentage, "visible", &config->batteryPercentage.visible);
    if (!json_getString(json_batteryPercentage, "font", config->batteryPercentage.font) && use_fallbacks)
        strcpy(config->batteryPercentage.font, config->hint.font);
    json_getInt(json_batteryPercentage, "size", &config->batteryPercentage.size);
    if (!json_color(json_batteryPercentage, "color", &config->batteryPercentage.color) && use_fallbacks)
        config->batteryPercentage.color = config->hint.color;
    json_getInt(json_batteryPercentage, "offsetX", &config->batteryPercentage.offsetX);
    json_getInt(json_batteryPercentage, "offsetY", &config->batteryPercentage.offsetY);
    json_getBool(json_batteryPercentage, "onleft", &config->batteryPercentage.onleft);

	cJSON_free(json_root);

    return true;
}

Theme_s theme_loadFromPath(const char* theme_path)
{
    Theme_s config = {
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

    strncpy(config.path, theme_path, STR_MAX-1);
    int len = strlen(config.path);
    if (config.path[len-1] != '/') {
        config.path[len] = '/';
        config.path[len+1] = '\0';
    }

    char config_path[STR_MAX * 2];
    snprintf(config_path, STR_MAX * 2 - 1, "%sconfig.json", config.path);

	if (!exists(config_path))
		sprintf(config_path, "%sconfig.json", FALLBACK_PATH);
	
    theme_applyConfig(&config, config_path, true);

    // apply overrides
    sprintf(config_path, "%s/config.json", THEME_OVERRIDES);
	if (exists(config_path))
		theme_applyConfig(&config, config_path, false);

	return config;
}

Theme_s theme_load(void)
{
    char theme_path[STR_MAX];
	theme_getPath(theme_path);
	return theme_loadFromPath(theme_path);
}

#endif // THEME_CONFIG_H__
