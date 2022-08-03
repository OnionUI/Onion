#ifndef KEYMAP_H__
#define KEYMAP_H__

#include <linux/input.h>
#include "SDL/SDL.h"

// for use with SDL_Event: event.key.keysym.sym
#define SW_BTN_UP       SDLK_UP
#define SW_BTN_DOWN     SDLK_DOWN
#define SW_BTN_LEFT     SDLK_LEFT
#define SW_BTN_RIGHT    SDLK_RIGHT
#define SW_BTN_A        SDLK_SPACE
#define SW_BTN_B        SDLK_LCTRL
#define SW_BTN_X        SDLK_LSHIFT
#define SW_BTN_Y        SDLK_LALT
#define SW_BTN_L1       SDLK_e
#define SW_BTN_R1       SDLK_t
#define SW_BTN_L2       SDLK_TAB
#define SW_BTN_R2       SDLK_BACKSPACE
#define SW_BTN_SELECT   SDLK_RCTRL
#define SW_BTN_START    SDLK_RETURN
#define SW_BTN_MENU     SDLK_ESCAPE
#define SW_BTN_POWER    SDLK_POWER

// for use with input_event: event.code
#define HW_BTN_UP       KEY_UP
#define HW_BTN_DOWN     KEY_DOWN
#define HW_BTN_LEFT     KEY_LEFT
#define HW_BTN_RIGHT    KEY_RIGHT
#define HW_BTN_A        KEY_SPACE
#define HW_BTN_B        KEY_LEFTCTRL
#define HW_BTN_X        KEY_LEFTSHIFT
#define HW_BTN_Y        KEY_LEFTALT
#define HW_BTN_L1       KEY_E
#define HW_BTN_R1       KEY_T
#define HW_BTN_L2       KEY_TAB
#define HW_BTN_R2       KEY_BACKSPACE
#define HW_BTN_SELECT   KEY_RIGHTCTRL
#define HW_BTN_START    KEY_ENTER
#define HW_BTN_MENU     KEY_ESC
#define HW_BTN_POWER    KEY_POWER

#endif // KEYMAP_H__