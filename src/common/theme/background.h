#ifndef THEME_BACKGROUND_H__
#define THEME_BACKGROUND_H__

#include <stdbool.h>
#include <SDL/SDL.h>
#include <SDL/SDL_ttf.h>

#include "./load.h"
#include "utils/rotate180.h"

static SDL_Surface *theme_background;
static bool theme_background_loaded = false;

void theme_backgroundLoad(const char *theme_path)
{
    theme_background = rotate180(theme_loadImage(theme_path, "background"));
    theme_background_loaded = true;
}

void theme_backgroundBlit(SDL_Surface *video)
{
    if (!theme_background_loaded)
        return;
    SDL_BlitSurface(theme_background, NULL, video, NULL);
}

void theme_backgroundFree(void)
{
    if (theme_background_loaded)
        SDL_FreeSurface(theme_background);
}

#endif // THEME_BACKGROUND_H__
