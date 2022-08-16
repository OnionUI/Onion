#ifndef TWEAKS_APPSTATE_H__
#define TWEAKS_APPSTATE_H__

#include "utils/keystate.h"
#include "components/list.h"

static int level = 0;
static List *menu_stack[5];

static bool quit = false;
static bool header_changed = true;
static bool list_changed = true;
static bool footer_changed = true;
static bool battery_changed = true;
static KeyState keystate[320] = {(KeyState)0};

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