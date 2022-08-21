#ifndef UTILS_SDL_INIT_H__
#define UTILS_SDL_INIT_H__

#include <stdbool.h>

static bool sdl_has_audio = false;

typedef struct SDL_Surface SDL_Surface;

bool SDL_InitDefault(bool include_audio, SDL_Surface *video, SDL_Surface *screen);

#endif // UTILS_SDL_INIT_H__
