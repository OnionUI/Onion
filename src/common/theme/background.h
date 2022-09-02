#ifndef THEME_BACKGROUND_H__
#define THEME_BACKGROUND_H__

#include <stdbool.h>
#include <SDL/SDL.h>
#include <SDL/SDL_ttf.h>

#include "./load.h"
#include "./resources.h"
#include "utils/rotate180.h"

void theme_backgroundLoad(void)
{
    char theme_path[STR_MAX];
    resources.background = rotate180(theme_loadImage(theme_getPath(theme_path), "background"));
    resources._background_loaded = true;
}

SDL_Surface* theme_background(void)
{
    if (!resources._background_loaded)
        theme_backgroundLoad();
    return resources.background;
}

void theme_backgroundBlit(SDL_Surface *video)
{
    SDL_BlitSurface(theme_background(), NULL, video, NULL);
}

#endif // THEME_BACKGROUND_H__
