#ifndef THEME_RESOURCES_H__
#define THEME_RESOURCES_H__

#include <SDL/SDL.h>
#include <SDL/SDL_mixer.h>
#include <SDL/SDL_ttf.h>
#include <stdbool.h>

#include "system/lang.h"
#include "utils/flags.h"
#include "utils/log.h"

#include "./config.h"

#define RES_MAX_REQUESTS 200

typedef enum theme_images {
    NULL_IMAGE,
    BG_TITLE,
    LOGO,
    BATTERY_0,
    BATTERY_20,
    BATTERY_50,
    BATTERY_80,
    BATTERY_100,
    BATTERY_CHARGING,
    BG_LIST_S,
    BG_LIST_L,
    HORIZONTAL_DIVIDER,
    PROGRESS_DOT,
    TOGGLE_ON,
    TOGGLE_OFF,
    BG_FOOTER,
    BUTTON_A,
    BUTTON_B,
    LEFT_ARROW,
    RIGHT_ARROW,
    LEFT_ARROW_WB,
    RIGHT_ARROW_WB,
    POP_BG,
    EMPTY_BG,
    PREVIEW_BG,
    BRIGHTNESS_0,
    BRIGHTNESS_1,
    BRIGHTNESS_2,
    BRIGHTNESS_3,
    BRIGHTNESS_4,
    BRIGHTNESS_5,
    BRIGHTNESS_6,
    BRIGHTNESS_7,
    BRIGHTNESS_8,
    BRIGHTNESS_9,
    BRIGHTNESS_10,
    LEGEND_GAMESWITCHER,
    images_count
} ThemeImages;

typedef enum theme_fonts {
    NULL_FONT,
    TITLE,
    HINT,
    GRID1x4,
    GRID3x4,
    LIST,
    BATTERY,
    fonts_count
} ThemeFonts;

typedef struct Theme_Resources {
    Theme_s theme;
    Theme_s theme_back;
    bool _theme_loaded;
    SDL_Surface *surfaces[(int)images_count];
    TTF_Font *fonts[(int)fonts_count];
    SDL_Surface *background;
    bool _background_loaded;
    Mix_Music *bgm;
    Mix_Chunk *sound_change;
} Resources_s;

static Resources_s resources = {._theme_loaded = false,
                                .background = NULL,
                                ._background_loaded = false,
                                .bgm = NULL,
                                .sound_change = NULL};

Theme_s *theme(void)
{
    if (!resources._theme_loaded) {
        resources.theme = theme_load();
        resources.theme_back = theme_loadFromPath(resources.theme.path, false);
        resources._theme_loaded = true;
    }
    return &resources.theme;
}

SDL_Surface *_loadImage(ThemeImages request)
{
    Theme_s *t = theme();
    int real_location, backup_location;

    if (request == BATTERY_0 ||
        request == BATTERY_20 ||
        request == BATTERY_50 ||
        request == BATTERY_80 ||
        request == BATTERY_100 ||
        request == BATTERY_CHARGING) {
        temp_flag_set("hasBatteryDisplay", true);
    }

    switch (request) {
    case BG_TITLE:
        return theme_loadImage(t->path, "bg-title");
    case LOGO:
        return theme_loadImage(t->path, "miyoo-topbar");
    case BATTERY_0:
        return theme_loadImage(t->path, "power-0%-icon");
    case BATTERY_20:
        return theme_loadImage(t->path, "power-20%-icon");
    case BATTERY_50:
        return theme_loadImage(t->path, "power-50%-icon");
    case BATTERY_80:
        return theme_loadImage(t->path, "power-80%-icon");
    case BATTERY_100:
        real_location = theme_getImagePath(t->path, "power-full-icon", NULL);
        backup_location =
            theme_getImagePath(t->path, "power-full-icon_back", NULL);
        return theme_loadImage(t->path, real_location == backup_location
                                            ? "power-full-icon_back"
                                            : "power-full-icon");
    case BATTERY_CHARGING:
        return theme_loadImage(t->path, "ic-power-charge-100%");
    case BG_LIST_S:
        return theme_loadImage(t->path, "bg-list-s");
    case BG_LIST_L:
        return theme_loadImage(t->path, "bg-list-l");
    case HORIZONTAL_DIVIDER:
        return theme_loadImage(t->path, "div-line-h");
    case PROGRESS_DOT:
        return theme_loadImage(t->path, "progress-dot");
    case TOGGLE_ON:
        return theme_loadImage(t->path, "extra/toggle-on");
    case TOGGLE_OFF:
        return theme_loadImage(t->path, "extra/toggle-off");
    case BG_FOOTER:
        return theme_loadImage(t->path, "tips-bar-bg");
    case BUTTON_A:
        return theme_loadImage(t->path, "icon-A-54");
    case BUTTON_B:
        return theme_loadImage(t->path, "icon-B-54");
    case LEFT_ARROW:
        return theme_loadImage(t->path, "icon-left-arrow-24");
    case RIGHT_ARROW:
        return theme_loadImage(t->path, "icon-right-arrow-24");
    case LEFT_ARROW_WB:
        return theme_loadImage(t->path, "extra/arrowLeft");
    case RIGHT_ARROW_WB:
        return theme_loadImage(t->path, "extra/arrowRight");
    case POP_BG:
        return theme_loadImage(t->path, "pop-bg");
    case EMPTY_BG:
        return theme_loadImage(t->path, "Empty");
    case PREVIEW_BG:
        return theme_loadImage(t->path, "preview-bg");
    case BRIGHTNESS_0:
        return theme_loadImage(t->path, "extra/lum0");
    case BRIGHTNESS_1:
        return theme_loadImage(t->path, "extra/lum1");
    case BRIGHTNESS_2:
        return theme_loadImage(t->path, "extra/lum2");
    case BRIGHTNESS_3:
        return theme_loadImage(t->path, "extra/lum3");
    case BRIGHTNESS_4:
        return theme_loadImage(t->path, "extra/lum4");
    case BRIGHTNESS_5:
        return theme_loadImage(t->path, "extra/lum5");
    case BRIGHTNESS_6:
        return theme_loadImage(t->path, "extra/lum6");
    case BRIGHTNESS_7:
        return theme_loadImage(t->path, "extra/lum7");
    case BRIGHTNESS_8:
        return theme_loadImage(t->path, "extra/lum8");
    case BRIGHTNESS_9:
        return theme_loadImage(t->path, "extra/lum9");
    case BRIGHTNESS_10:
        return theme_loadImage(t->path, "extra/lum10");
    case LEGEND_GAMESWITCHER:
        return theme_loadImage(t->path, "extra/gs-legend");
    default:
        break;
    }
    return NULL;
}

TTF_Font *_loadFont(ThemeFonts request)
{
    Theme_s *t = theme();
    TTF_Font *font = NULL;

    switch (request) {
    case TITLE:
        return theme_loadFont(t->path, t->title.font, t->title.size);
    case HINT:
        return theme_loadFont(t->path, t->hint.font, t->hint.size);
    case GRID1x4:
        return theme_loadFont(t->path, t->grid.font, t->grid.grid1x4);
    case GRID3x4:
        return theme_loadFont(t->path, t->grid.font, t->grid.grid3x4);
    case LIST:
        font = theme_loadFont(t->path, t->list.font, t->list.size);
        TTF_SetFontStyle(font, TTF_STYLE_BOLD);
        return font;
    case BATTERY:
        return theme_loadFont(t->path, t->batteryPercentage.font,
                              t->batteryPercentage.size);
    default:
        break;
    }

    return NULL;
}

SDL_Surface *resource_getSurface(ThemeImages request)
{
    if (resources.surfaces[request] == NULL)
        resources.surfaces[request] = _loadImage(request);
    return resources.surfaces[request];
}

/**
 * @brief Remember to free manually !
 * 
 * @param request 
 * @return SDL_Surface* 
 */
SDL_Surface *resource_getSurfaceCopy(ThemeImages request)
{
    return _loadImage(request);
}

TTF_Font *resource_getFont(ThemeFonts request)
{
    if (resources.fonts[request] == NULL)
        resources.fonts[request] = _loadFont(request);
    return resources.fonts[request];
}

void resource_reloadFont(ThemeFonts request)
{
    if (resources.fonts[request] != NULL)
        TTF_CloseFont(resources.fonts[request]);
    resources.fonts[request] = _loadFont(request);
}

Mix_Chunk *resource_getSoundChange(void)
{
    if (resources.sound_change == NULL) {
        char sound_path[STR_MAX * 2];
        snprintf(sound_path, STR_MAX * 2 - 1, "%ssound/change.wav",
                 theme()->path);
        if (!is_file(sound_path))
            strcpy(sound_path, "/mnt/SDCARD/miyoo/app/sound/change.wav");
        if (is_file(sound_path))
            resources.sound_change = Mix_LoadWAV(sound_path);
    }
    return resources.sound_change;
}

Mix_Music *resource_getBGM(void)
{
    if (resources.bgm == NULL) {
        char sound_path[STR_MAX * 2];
        snprintf(sound_path, STR_MAX * 2 - 1, "%ssound/bgm.mp3", theme()->path);
        if (!is_file(sound_path))
            strcpy(sound_path, "/mnt/SDCARD/miyoo/app/sound/bgm.mp3");
        if (is_file(sound_path))
            resources.bgm = Mix_LoadMUS(sound_path);
    }
    return resources.bgm;
}

SDL_Surface *resource_getBrightness(int brightness)
{
    switch (brightness) {
    case 0:
        return resource_getSurface(BRIGHTNESS_0);
    case 1:
        return resource_getSurface(BRIGHTNESS_1);
    case 2:
        return resource_getSurface(BRIGHTNESS_2);
    case 3:
        return resource_getSurface(BRIGHTNESS_3);
    case 4:
        return resource_getSurface(BRIGHTNESS_4);
    case 5:
        return resource_getSurface(BRIGHTNESS_5);
    case 6:
        return resource_getSurface(BRIGHTNESS_6);
    case 7:
        return resource_getSurface(BRIGHTNESS_7);
    case 8:
        return resource_getSurface(BRIGHTNESS_8);
    case 9:
        return resource_getSurface(BRIGHTNESS_9);
    case 10:
        return resource_getSurface(BRIGHTNESS_10);
    default:
        break;
    }
    return NULL;
}

void resources_free()
{
    temp_flag_set("hasBatteryDisplay", false);

    for (int i = 0; i < images_count; i++)
        if (resources.surfaces[i] != NULL)
            SDL_FreeSurface(resources.surfaces[i]);

    for (int i = 0; i < fonts_count; i++)
        if (resources.fonts[i] != NULL)
            TTF_CloseFont(resources.fonts[i]);

    if (resources._background_loaded)
        SDL_FreeSurface(resources.background);

    if (resources.sound_change != NULL)
        Mix_FreeChunk(resources.sound_change);

    if (resources.bgm != NULL)
        Mix_FreeMusic(resources.bgm);

    if (_theme_overrides_changed) {
        bool hide_labels_icons = resources.theme_back.hideLabels.icons,
             hide_labels_hints = resources.theme_back.hideLabels.hints;
        theme_getOverride("hideLabels", "icons", &hide_labels_icons,
                          cJSON_True);
        theme_getOverride("hideLabels", "hints", &hide_labels_hints,
                          cJSON_True);
        lang_removeIconLabels(hide_labels_icons, hide_labels_hints);
    }

    theme_saveOverrides();
    theme_freeOverrides();
}

#endif // THEME_RESOURCES_H__
