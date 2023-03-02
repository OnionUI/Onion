#ifndef ROTATE_180_H__
#define ROTATE_180_H__

#include "SDL/SDL_rotozoom.h"
#include <SDL/SDL.h>

SDL_Surface *rotate180(SDL_Surface *original)
{
    SDL_Surface *rotated = rotozoomSurface(original, 180.0, 1.0, 0);
    SDL_FillRect(original, NULL, SDL_MapRGB(original->format, 255, 0, 0));
    SDL_Rect rect = {-2, -2};
    SDL_BlitSurface(rotated, NULL, original, &rect);
    SDL_FreeSurface(rotated);
    return original;
}

#endif // ROTATE_180_H__
