#ifndef UI_APPLICATION_HPP__
#define UI_APPLICATION_HPP__

#include <unistd.h>

#include <SDL/SDL.h>
#include <SDL/SDL_ttf.h>

#include "Screen.hpp"
#include "Element.hpp"
#include "Image.hpp"

using namespace UI;

namespace UI {

class Application
{
public:
    VideoStream video;
    Screen screen;

    Application(int _w, int _h) : video(_w, _h), screen(_w, _h) {
        SDL_Init(SDL_INIT_VIDEO);
        SDL_ShowCursor(SDL_DISABLE);
        SDL_EnableKeyRepeat(300, 50);
        TTF_Init();
    }

    ~Application() {
        TTF_Quit();
        SDL_Quit();
    }

    int run() {
        Element main_view(0, 0, screen.w, screen.h);
        
        Element header(0, 0, screen.w, 0.125 * screen.h);
        main_view.appendChild(&header);

        Image previewMsg("./res/noThemePreview.png");
        SDL_Rect previewRect = {60, 60};
        ((Surface*)&previewMsg)->blit((Surface*)&screen, &previewRect);

        main_view.render(&screen);

        sleep(2);

        return 0;
    }
};

}

#endif // UI_APPLICATION_HPP__
