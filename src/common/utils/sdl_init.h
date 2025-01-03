#ifndef UTILS_SDL_INIT_H__
#define UTILS_SDL_INIT_H__

#include <SDL/SDL.h>
#include <SDL/SDL_image.h>
#include <SDL/SDL_mixer.h>
#include <SDL/SDL_ttf.h>

#include "system/display.h"

static SDL_Surface *video;
static SDL_Surface *screen;
static bool sdl_has_audio = false;

bool SDL_InitDefault(bool include_audio)
{
    display_init(false);

    SDL_Init(include_audio ? (SDL_INIT_VIDEO | SDL_INIT_AUDIO)
                           : SDL_INIT_VIDEO);
    SDL_ShowCursor(SDL_DISABLE);
    SDL_EnableKeyRepeat(300, 50);
    TTF_Init();

    video = SDL_SetVideoMode(g_display.width, g_display.height, 32, SDL_HWSURFACE);
    screen = SDL_CreateRGBSurface(SDL_HWSURFACE, g_display.width, g_display.height, 32, 0, 0, 0, 0);

    if (!include_audio || Mix_OpenAudio(48000, 32784, 2, 4096) < 0)
        return false;

    sdl_has_audio = include_audio;

    return true;
}

#endif // UTILS_SDL_INIT_H__
