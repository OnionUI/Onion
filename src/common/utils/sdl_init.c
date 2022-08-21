#include "sdl_init.h"

#include <SDL/SDL.h>
#include <SDL/SDL_ttf.h>
#include <SDL/SDL_mixer.h>

bool SDL_InitDefault(bool include_audio, SDL_Surface *video, SDL_Surface *screen)
{
    SDL_Init(include_audio ? (SDL_INIT_VIDEO | SDL_INIT_AUDIO) : SDL_INIT_VIDEO);
    SDL_ShowCursor(SDL_DISABLE);
    SDL_EnableKeyRepeat(300, 50);
    TTF_Init();

    video = SDL_SetVideoMode(640, 480, 32, SDL_HWSURFACE);
    screen = SDL_CreateRGBSurface(SDL_HWSURFACE, 640, 480, 32, 0, 0, 0, 0);

    if (!include_audio || Mix_OpenAudio(48000, 32784, 2, 4096) < 0)
        return false;

    sdl_has_audio = include_audio;

    return true;
}