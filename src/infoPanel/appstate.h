#ifndef INFOPANEL_APPSTATE_H__
#define INFOPANEL_APPSTATE_H__

#include <signal.h>

static bool quit = false;
static bool all_changed = true;
static bool header_changed = true;
static bool footer_changed = true;
static bool battery_changed = true;

static void sigHandler(int sig)
{
    switch (sig) {
    case SIGINT:
    case SIGTERM:
        quit = true;
        break;
    default:
        break;
    }
}

#endif // INFOPANEL_APPSTATE_H__
