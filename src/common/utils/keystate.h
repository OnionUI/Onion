#ifndef KEYSTATE_H__
#define KEYSTATE_H__

#include <SDL/SDL.h>

typedef enum
{
    RELEASED,
    PRESSED,
    REPEATING
} KeyState;

static SDL_Event keystate_event;

bool updateKeystate(KeyState keystate[320], bool *quit_flag)
{
    while (SDL_PollEvent(&keystate_event)) {
        SDLKey key = keystate_event.key.keysym.sym;

        switch (keystate_event.type) {
            case SDL_QUIT:
                *quit_flag = true;
                break;
            case SDL_KEYDOWN:
                if (keystate[key] != RELEASED)
                    keystate[key] = REPEATING;
                else
                    keystate[key] = PRESSED;
                return true;
            case SDL_KEYUP:
                keystate[key] = RELEASED;
                return true;
            default: break;
        }
    }

    return false;
}

#endif
