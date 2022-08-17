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

bool updateKeystate(KeyState keystate[320], bool *quit_flag, bool enabled)
{
    bool retval = false;

    while (SDL_PollEvent(&keystate_event)) {
        SDLKey key = keystate_event.key.keysym.sym;

        if (!enabled)
            continue;

        switch (keystate_event.type) {
            case SDL_QUIT:
                *quit_flag = true;
                break;
            case SDL_KEYDOWN:
                if (keystate[key] != RELEASED)
                    keystate[key] = REPEATING;
                else
                    keystate[key] = PRESSED;
                retval = true;
                break;
            case SDL_KEYUP:
                keystate[key] = RELEASED;
                retval = true;
                break;
            default: break;
        }
    }

    return retval;
}

#endif
