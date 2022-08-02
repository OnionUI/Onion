#ifndef THEME_RESOURCES_H__
#define THEME_RESOURCES_H__

#include <SDL/SDL.h>
#include "./theme.h"

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
