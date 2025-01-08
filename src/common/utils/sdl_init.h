#ifndef UTILS_SDL_INIT_H__
#define UTILS_SDL_INIT_H__

#include <SDL/SDL.h>
#include <SDL/SDL_image.h>
#include <SDL/SDL_ttf.h>

#ifdef HAS_AUDIO
#include <SDL/SDL_mixer.h>
#endif

#include "system/display.h"

static SDL_Surface *video;
static SDL_Surface *screen;

bool SDL_InitDefault(void)
{
    display_getRenderResolution();

#ifdef HAS_AUDIO
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);
#else
    SDL_Init(SDL_INIT_VIDEO);
#endif
    SDL_ShowCursor(SDL_DISABLE);
    SDL_EnableKeyRepeat(300, 50);
    TTF_Init();

    video = SDL_SetVideoMode(g_display.width, g_display.height, 32, SDL_HWSURFACE);
    screen = SDL_CreateRGBSurface(SDL_HWSURFACE, g_display.width, g_display.height, 32, 0, 0, 0, 0);

#ifdef HAS_AUDIO
    if (Mix_OpenAudio(48000, 32784, 2, 4096) < 0)
        return false;
#endif

    return true;
}

#endif // UTILS_SDL_INIT_H__
