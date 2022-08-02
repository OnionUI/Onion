#ifndef THEME_RESOURCES_H__
#define THEME_RESOURCES_H__

#include <SDL/SDL.h>
#include "./theme.h"
#include "./rotate180.h"

#define TR_BACKGROUND       0
#define TR_BG_TITLE         1
#define TR_LOGO             2

typedef struct Theme_Surfaces
{
    SDL_Surface* background;
    SDL_Surface* bg_title;
    SDL_Surface* logo;
} Theme_Surfaces_s;

typedef struct Theme_Rects
{
    SDL_Rect background;
    SDL_Rect bg_title;
    SDL_Rect logo;
} Theme_Rects_s;

typedef struct Theme_Resources
{
    Theme_Surfaces_s surfaces;
    Theme_Rects_s rects;
} Resources_s;

Resources_s theme_loadResources(Theme_s* theme, int requests[], int num_requests)
{
    int i, req;
    Resources_s res = {
        .rects = {
            .logo = {20}
        }
    };
    Theme_Surfaces_s* surfaces = &res.surfaces;
    Theme_Rects_s* rects = &res.rects;

    for (i = 0; i < num_requests; i++) {
        req = requests[i];

        if (req == TR_BACKGROUND)
            surfaces->background = rotate180(theme_loadImage(theme, "background"));
        else if (req == TR_BG_TITLE)
			surfaces->bg_title = theme_loadImage(theme, "bg-title");
        else if (req == TR_LOGO) {
			surfaces->logo = theme_loadImage(theme, "miyoo-topbar");
            rects->logo.y = (60 - surfaces->logo->h) / 2;
        }
    }

    return res;
}

void _free_surface(SDL_Surface* ptr)
{
    if (ptr)
        SDL_FreeSurface(ptr);
}

void theme_freeResources(Resources_s* res)
{
    Theme_Surfaces_s* ptr = &res->surfaces;
    _free_surface(ptr->background);
    _free_surface(ptr->bg_title);
    _free_surface(ptr->logo);
}

#endif // THEME_RESOURCES_H__
