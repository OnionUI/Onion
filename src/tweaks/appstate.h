#ifndef TWEAKS_APPSTATE_H__
#define TWEAKS_APPSTATE_H__

#include <signal.h>

#include "utils/sdl_init.h"
#include "utils/keystate.h"
#include "components/list.h"

static int menu_level = 0;
static List *menu_stack[5];

static bool quit = false;
static bool all_changed = true;
static bool header_changed = true;
static bool list_changed = true;
static bool footer_changed = true;
static bool battery_changed = true;
static KeyState keystate[320] = {(KeyState)0};
static bool keys_enabled = true;
static bool reset_menus = false;
static bool skip_next_change = false;

static void sigHandler(int sig)
{
    switch (sig) {
        case SIGINT:
        case SIGTERM:
            quit = true;
            break;
        default: break;
    }
}

#endif // TWEAKS_APPSTATE_H__