#ifndef UI_SCREEN_HPP__
#define UI_SCREEN_HPP__

#include "Surface.hpp"

using namespace UI;

namespace UI {

class Screen : Surface
{
public:
    int w;
    int h;

    Screen(int _w, int _h) : w(_w), h(_h), Surface(SDL_CreateRGBSurface(SDL_HWSURFACE, w, h, 32, 0, 0, 0, 0)) {};

    void display(Surface *src) {
        src->blit(this);
    }
};

class VideoStream : Surface
{
public:
    int w;
    int h;

    VideoStream(int _w, int _h) : w(_w), h(_h), Surface(SDL_SetVideoMode(w, h, 32, SDL_HWSURFACE)) {};

    void display(Screen *screen) {
        ((Surface*)screen)->blit(this);
        SDL_Flip(surface);
    }
};

}

#endif // UI_SCREEN_HPP__
