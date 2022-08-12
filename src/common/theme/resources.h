#ifndef THEME_RESOURCES_H__
#define THEME_RESOURCES_H__

#include <stdbool.h>
#include <SDL/SDL.h>
#include <SDL/SDL_ttf.h>

#include "utils/log.h"
#include "./config.h"

#define RES_MAX_REQUESTS 200

typedef enum
{
    TR_NULL,
    TR_BG_TITLE,
    TR_LOGO,
    TR_BATTERY_0,
    TR_BATTERY_20,
    TR_BATTERY_50,
    TR_BATTERY_80,
    TR_BATTERY_100,
    TR_BATTERY_CHARGING,
    TR_BG_LIST_S,
    TR_BG_LIST_L,
    TR_HORIZONTAL_DIVIDER,
    TR_TOGGLE_ON,
    TR_TOGGLE_OFF,
    TR_BG_FOOTER,
    TR_BUTTON_A,
    TR_BUTTON_B
} ThemeImages;

typedef struct Theme_Surfaces
{
    SDL_Surface* bg_title;
    SDL_Surface* logo;
    SDL_Surface* battery_0;
    SDL_Surface* battery_20;
    SDL_Surface* battery_50;
    SDL_Surface* battery_80;
    SDL_Surface* battery_100;
    SDL_Surface* battery_charging;
    SDL_Surface* bg_list_small;
    SDL_Surface* bg_list_large;
    SDL_Surface* horizontal_divider;
    SDL_Surface* toggle_on;
    SDL_Surface* toggle_off;
    SDL_Surface* bg_footer;
    SDL_Surface* button_a;
    SDL_Surface* button_b;
} Theme_Surfaces_s;

typedef struct Theme_Fonts
{
    TTF_Font* title;
    TTF_Font* hint;
    TTF_Font* grid1x4;
    TTF_Font* grid3x4;
    TTF_Font* list;
    TTF_Font* battery;
} Theme_Fonts_s;

typedef struct Theme_Resources
{
    Theme_Surfaces_s surfaces;
    Theme_Fonts_s fonts;
} Resources_s;

Resources_s theme_loadResources(Theme_s* t, ThemeImages requests[RES_MAX_REQUESTS])
{
    Resources_s res = {
        .fonts = {
            .title = theme_loadFont(t->path, t->title.font, t->title.size),
            .hint = theme_loadFont(t->path, t->hint.font, t->hint.size),
            .grid1x4 = theme_loadFont(t->path, t->grid.font, t->grid.grid1x4),
            .grid3x4 = theme_loadFont(t->path, t->grid.font, t->grid.grid3x4),
            .list = theme_loadFont(t->path, t->list.font, t->list.size),
            .battery = theme_loadFont(t->path, t->batteryPercentage.font, t->batteryPercentage.size)
        }
    };
    Theme_Surfaces_s* s = &res.surfaces;
    int real_location, backup_location;

    for (int i = 0; i < RES_MAX_REQUESTS; i++) {
        switch (requests[i]) {
            case TR_BG_TITLE: s->bg_title = theme_loadImage(t->path, "bg-title"); break;
            case TR_LOGO: s->logo = theme_loadImage(t->path, "miyoo-topbar"); break;
            case TR_BATTERY_0: s->battery_0 = theme_loadImage(t->path, "power-0%-icon"); break;
            case TR_BATTERY_20: s->battery_20 = theme_loadImage(t->path, "power-20%-icon"); break;
            case TR_BATTERY_50: s->battery_50 = theme_loadImage(t->path, "power-50%-icon"); break;
            case TR_BATTERY_80: s->battery_80 = theme_loadImage(t->path, "power-80%-icon"); break;
            case TR_BATTERY_100:
                real_location = theme_getImagePath(t->path, "power-full-icon", NULL);
                backup_location = theme_getImagePath(t->path, "power-full-icon_back", NULL);
                s->battery_100 = theme_loadImage(t->path, real_location == backup_location ? "power-full-icon_back" : "power-full-icon");
                break;
            case TR_BATTERY_CHARGING: s->battery_charging = theme_loadImage(t->path, "ic-power-charge-100%"); break;
            case TR_BG_LIST_S: s->bg_list_small = theme_loadImage(t->path, "bg-list-s"); break;
            case TR_BG_LIST_L: s->bg_list_large = theme_loadImage(t->path, "bg-list-l"); break;
            case TR_HORIZONTAL_DIVIDER: s->horizontal_divider = theme_loadImage(t->path, "div-line-h"); break;
            case TR_TOGGLE_ON: s->toggle_on = theme_loadImage(t->path, "extra/toggle-on"); break;
            case TR_TOGGLE_OFF: s->toggle_off = theme_loadImage(t->path, "extra/toggle-off"); break;
            case TR_BG_FOOTER: s->bg_footer = theme_loadImage(t->path, "tips-bar-bg"); break;
            case TR_BUTTON_A: s->button_a = theme_loadImage(t->path, "icon-A-54"); break;
            case TR_BUTTON_B: s->button_b = theme_loadImage(t->path, "icon-B-54"); break;
            default: i = RES_MAX_REQUESTS; break;
        }
    }

    return res;
}

void theme_freeResources(Resources_s* res)
{
    Theme_Surfaces_s* s = &res->surfaces;
    Theme_Fonts_s* f = &res->fonts;

    SDL_FreeSurface(s->bg_title);
    SDL_FreeSurface(s->logo);
    SDL_FreeSurface(s->battery_0);
    SDL_FreeSurface(s->battery_20);
    SDL_FreeSurface(s->battery_50);
    SDL_FreeSurface(s->battery_80);
    SDL_FreeSurface(s->battery_100);
    SDL_FreeSurface(s->battery_charging);
    SDL_FreeSurface(s->bg_list_small);
    SDL_FreeSurface(s->bg_list_large);
    SDL_FreeSurface(s->horizontal_divider);
    SDL_FreeSurface(s->toggle_on);
    SDL_FreeSurface(s->toggle_off);
    SDL_FreeSurface(s->bg_footer);
    SDL_FreeSurface(s->button_a);
    SDL_FreeSurface(s->button_b);

    TTF_CloseFont(f->title);
    TTF_CloseFont(f->hint);
    TTF_CloseFont(f->grid1x4);
    TTF_CloseFont(f->grid3x4);
    TTF_CloseFont(f->list);
    TTF_CloseFont(f->battery);
}

#endif // THEME_RESOURCES_H__
