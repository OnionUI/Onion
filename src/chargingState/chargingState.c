#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <stdint.h>
#include <pthread.h>
#include <string.h>
#include <fcntl.h>
#include <assert.h>
#include <time.h>
#include <sys/ioctl.h>
#include <linux/fb.h>
#include <linux/input.h>
#include <poll.h>
#include <SDL/SDL.h>
#include <SDL/SDL_image.h>

#include "utils/msleep.h"
#include "utils/keystate.h"
#include "utils/log.h"
#include "system/battery.h"
#include "system/system.h"
#include "system/display.h"
#include "system/settings.h"
#include "system/keymap_sw.h"
#include "system/keymap_hw.h"
#include "system/rumble.h"
#include "theme/config.h"

#define SHUTDOWN_TIMEOUT 500
#define DISPLAY_TIMEOUT 10000

static bool quit = false;
static bool suspended = false;
static int input_fd;
static struct input_event ev;
static struct pollfd fds[1];

void getImageDir(const char *theme_path, char *image_dir)
{
    char image0_path[STR_MAX * 2];

    sprintf(image0_path, "%s/skin/extra/chargingState0.png", THEME_OVERRIDES);
    if (exists(image0_path)) {
        sprintf(image_dir, "%s/skin/extra", THEME_OVERRIDES);
        return;
    }

    sprintf(image0_path, "%sskin/extra/chargingState0.png", theme_path);
    if (exists(image0_path)) {
        sprintf(image_dir, "%sskin/extra", theme_path);
        return;
    }
    
    strcpy(image_dir, "res");
}

void suspend(bool enabled, SDL_Surface *video)
{
    suspended = enabled;
    if (suspended) {
        SDL_FillRect(video, NULL, 0);
        SDL_Flip(video);
    }
    // system_powersave(suspended);
    display_setScreen(!suspended);
}

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

int main(void)
{
    signal(SIGINT, sigHandler);
    signal(SIGTERM, sigHandler);

    bool turn_off = false;

    settings_load();
    display_setBrightness(settings.brightness);

    char image_dir[STR_MAX];
    getImageDir(settings.theme, image_dir);

    SDL_Init(SDL_INIT_VIDEO);
    SDL_ShowCursor(SDL_DISABLE);
    SDL_EnableKeyRepeat(300, 50);

    SDL_Surface* video = SDL_SetVideoMode(640, 480, 32, SDL_HWSURFACE);
    SDL_Surface* screen = SDL_CreateRGBSurface(SDL_HWSURFACE, 640, 480, 32, 0, 0, 0, 0);

    int frame_delay = 80;
    int frame_count = 0;
    SDL_Surface *frames[24];
    SDL_Surface *image;

    for (int i = 0; i < 24; i++) {
        char image_path[STR_MAX];
        sprintf(image_path, "%s/chargingState%d.png", image_dir, i);
        if ((image = IMG_Load(image_path)))
            frames[frame_count++] = image;
    }

    printf_debug("Frame count: %d\n", frame_count);

    // Prepare for Poll button input
    input_fd = open("/dev/input/event0", O_RDONLY);
    memset(&fds, 0, sizeof(fds));
    fds[0].fd = input_fd;
    fds[0].events = POLLIN;

    KeyState keystate[320] = {RELEASED};
    bool power_pressed = false;

    if (frame_delay <= 0)
        frame_delay = 80;

    int ticks = 0,
        display_ticks = 0,
        shutdown_ticks = 0,
        display_max = DISPLAY_TIMEOUT / frame_delay,
        shutdown_max = SHUTDOWN_TIMEOUT / frame_delay;

    int current_frame = 0;

    // Set the CPU to powersave (charges faster?)
    system_powersave_on();

    while (!quit) {
        if (suspended) {
            if (poll(fds, 1, 1000)) {
                read(input_fd, &ev, sizeof(ev));
                if (ev.type != EV_KEY || ev.value > REPEATING) continue;
                if (ev.code == HW_BTN_POWER && ev.value == PRESSED)
                    suspend(false, video);
            }
        }

        if (!battery_isCharging()) {
            quit = true;
            turn_off = true;
            break;
        }

        if (suspended)
            continue;

        if (updateKeystate(keystate, &quit, true)) {
            if (keystate[SW_BTN_POWER] == REPEATING && shutdown_ticks >= shutdown_max) {
                short_pulse();
                quit = true; // power on
            }
            else if (keystate[SW_BTN_POWER] == PRESSED) {
                shutdown_ticks = 0;
                power_pressed = true;
            }
            else if (keystate[SW_BTN_POWER] == RELEASED && power_pressed) {
                suspend(true, video); // toggle display
                power_pressed = false;
            }
            display_ticks = 0;
        }

        if (quit)
            break;

        if (!suspended && display_ticks >= display_max)
            suspend(true, video);

        // Clear screen
        SDL_FillRect(screen, NULL, 0);

        if (current_frame < frame_count) {
            SDL_Surface *frame = frames[current_frame];
            SDL_Rect frame_rect = {320 - frame->w / 2, 240 - frame->h / 2};
            SDL_BlitSurface(frame, NULL, screen, &frame_rect);
            current_frame = (current_frame + 1) % frame_count;
        }

        SDL_BlitSurface(screen, NULL, video, NULL);
        SDL_Flip(video);
        
        ticks++;
        display_ticks++;
        shutdown_ticks++;
        msleep(frame_delay);
    }

    // Clear the screen when exiting
    SDL_FillRect(video, NULL, 0);
    SDL_Flip(video);

    #ifndef PLATFORM_MIYOOMINI
    msleep(100);
    #endif

    for (int i = 0; i < frame_count; i++)
        SDL_FreeSurface(frames[i]);
    SDL_FreeSurface(screen);
    SDL_FreeSurface(video);
    SDL_Quit();

    #ifdef PLATFORM_MIYOOMINI
    if (turn_off) {
        display_setScreen(false);
        system("sync; reboot; sleep 10");
    }
    else {
        display_setScreen(true);
    }
    #endif

    // restore CPU performance mode
    system_powersave_off();

    return EXIT_SUCCESS;
}
