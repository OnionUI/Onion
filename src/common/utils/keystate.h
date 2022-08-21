#ifndef KEYSTATE_H__
#define KEYSTATE_H__

#include <stdbool.h>

#include <SDL/SDL_keysym.h>

typedef enum
{
    RELEASED,
    PRESSED,
    REPEATING
} KeyState;

bool updateKeystate(KeyState* keystate, bool *quit_flag, bool enabled, SDLKey *changed_key);

#endif // KEYSTATE_H__
