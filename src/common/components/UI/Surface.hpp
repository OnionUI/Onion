#ifndef UI_SURFACE_HPP__
#define UI_SURFACE_HPP__

#include <SDL/SDL.h>

namespace UI {

class Surface
{
public:
    Surface(SDL_Surface *_s = NULL) {
        surface = _s;
    }

    ~Surface() {
        if (surface != NULL)
            SDL_FreeSurface(surface);
    }

    void blit(Surface *dst, SDL_Rect *dstrect = NULL, SDL_Rect *srcrect = NULL) {
        if (surface != NULL && dst->surface != NULL)
            SDL_BlitSurface(surface, srcrect, dst->surface, dstrect);
    }
protected:
    SDL_Surface *surface;
};

}

#endif // UI_SURFACE_HPP__
