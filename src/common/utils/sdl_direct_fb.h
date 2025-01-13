#ifndef UTILS_SDL_DIRECT_FB_H
#define UTILS_SDL_DIRECT_FB_H

#include <SDL/SDL.h>
#include <SDL/SDL_rotozoom.h>
#include <linux/input.h>
#include <sys/poll.h>

#include "system/display.h"
#include "system/keymap_hw.h"
#include "system/keymap_sw.h"
#include "theme/load.h"
#include "utils/keystate.h"
#include "utils/log.h"
#include "utils/sdl_init.h"

#ifdef PLATFORM_MIYOOMINI
static bool _render_direct_to_fb = true;
#else
static bool _render_direct_to_fb = false;
#endif
static const char *INPUT_DEVICE = "/dev/input/event0";
static int _input_fd = -1;
static struct pollfd _fds[1];
static bool keyinput_disabled = false;

#define INIT_PNG 1
#define INIT_TTF 2
#define INIT_INPUT 4
#define INIT_AUDIO 8
#define INIT_ALL (INIT_PNG | INIT_TTF | INIT_INPUT | INIT_AUDIO)

void init(int flags)
{
    display_init(_render_direct_to_fb);

    if (g_display.width != 640 || g_display.height != 480) {
        theme_initScaling((double)g_display.width / 640.0, zoomSurface);
    }

    if (_render_direct_to_fb) {
        screen = SDL_CreateRGBSurface(SDL_SWSURFACE, g_display.width, g_display.height, 32, 0, 0, 0, 0);
        if (flags & INIT_PNG)
            IMG_Init(IMG_INIT_PNG);
        if (flags & INIT_TTF)
            TTF_Init();
        if (flags & INIT_INPUT) {
            _input_fd = open(INPUT_DEVICE, O_RDONLY);
            if (_input_fd == -1) {
                perror("Failed to open input device");
                exit(1);
            }
            memset(&_fds, 0, sizeof(_fds));
            _fds[0].fd = _input_fd;
            _fds[0].events = POLLIN;
        }
    }
    else {
        SDL_InitDefault();
    }
}

void render(void)
{
    if (_render_direct_to_fb) {
        int numBuffers = g_display.vinfo.yres_virtual / g_display.vinfo.yres;
        for (int b = 0; b < numBuffers; b++) {
            display_writeBuffer(b, &g_display, (uint32_t *)screen->pixels, (rect_t){0, 0, screen->w, screen->h}, true, false);
        }
    }
    else {
        SDL_BlitSurface(screen, NULL, video, NULL);
        SDL_Flip(video);
    }
}

void deinit(void)
{
    if (screen != NULL) {
        SDL_FreeSurface(screen);
    }
    TTF_Quit();
    IMG_Quit();

    if (_render_direct_to_fb) {
        if (_input_fd != -1) {
            close(_input_fd);
        }
    }
    else {
        SDL_FreeSurface(video);
        SDL_Quit();
    }

    display_close();
}

/**
 * @brief stop input event for other processes
 *
 */
void keyinput_disable(void)
{
    if (keyinput_disabled || _input_fd == -1)
        return;
    while (ioctl(_input_fd, EVIOCGRAB, 1) < 0) {
        usleep(100000);
    }
    keyinput_disabled = true;
    print_debug("Keyinput disabled");
}

/**
 * @brief restart input event for other processes
 *
 */
void keyinput_enable(void)
{
    if (!keyinput_disabled || _input_fd == -1)
        return;
    while (ioctl(_input_fd, EVIOCGRAB, 0) < 0) {
        usleep(100000);
    }
    keyinput_disabled = false;
    print_debug("Keyinput enabled");
}

SDLKey _translate_input(int key)
{
    switch (key) {
    case HW_BTN_UP:
        return SW_BTN_UP;
    case HW_BTN_DOWN:
        return SW_BTN_DOWN;
    case HW_BTN_LEFT:
        return SW_BTN_LEFT;
    case HW_BTN_RIGHT:
        return SW_BTN_RIGHT;
    case HW_BTN_A:
        return SW_BTN_A;
    case HW_BTN_B:
        return SW_BTN_B;
    case HW_BTN_X:
        return SW_BTN_X;
    case HW_BTN_Y:
        return SW_BTN_Y;
    case HW_BTN_L1:
        return SW_BTN_L1;
    case HW_BTN_R1:
        return SW_BTN_R1;
    case HW_BTN_L2:
        return SW_BTN_L2;
    case HW_BTN_R2:
        return SW_BTN_R2;
    case HW_BTN_SELECT:
        return SW_BTN_SELECT;
    case HW_BTN_START:
        return SW_BTN_START;
    case HW_BTN_MENU:
        return SW_BTN_MENU;
    case HW_BTN_POWER:
        return SW_BTN_POWER;
    default:
        return SDLK_UNKNOWN;
    }
}

bool _updateKeystate(KeyState keystate[320], bool *quit_flag, bool enabled, SDLKey *changed_key)
{
    if (!_render_direct_to_fb) {
        return updateKeystate(keystate, quit_flag, enabled, changed_key);
    }

    bool retval = false;
    struct input_event ev;

    if (_input_fd == -1) {
        return false;
    }

    while (poll(_fds, 1, 0) > 0) {
        if (read(_input_fd, &ev, sizeof(ev)) != sizeof(ev)) {
            fprintf(stderr, "Failed to read input event\n");
            return false;
        }

        if (!enabled)
            continue;

        if (ev.type == EV_KEY) {
            SDLKey key = _translate_input(ev.code);

            switch (ev.value) {
            case 1: // Key press
                if (keystate[key] != RELEASED)
                    keystate[key] = REPEATING;
                else
                    keystate[key] = PRESSED;
                if (changed_key != NULL)
                    *changed_key = key;
                retval = true;
                break;
            case 0: // Key release
                keystate[key] = RELEASED;
                if (changed_key != NULL)
                    *changed_key = key;
                retval = true;
                break;
            }
        }
        else if (ev.type == EV_SYN && ev.code == SYN_DROPPED) {
            // Handle SYN_DROPPED if necessary
            fprintf(stderr, "Event dropped\n");
        }
    }

    usleep(4000); // Sleep for 4 milliseconds

    return retval;
}

#endif // UTILS_SDL_DIRECT_FB_H
