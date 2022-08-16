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
#include "theme/config.h"

#define SHUTDOWN_TIMEOUT 500
#define DISPLAY_TIMEOUT 10000

static bool quit = false;
static bool suspended = false;

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

int main(void)
{
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

    int frame_count = 0;
    SDL_Surface *frames[24];
    SDL_Surface *image;

    for (int i = 0; i < 24; i++) {
        char image_path[STR_MAX];
        sprintf(image_path, "%s/chargingState%d.png", image_dir, i);
        if ((image = IMG_Load(image_path)))
            frames[frame_count++] = image;
    }

    int fps = frame_count > 1 ? 12 : 1;
    printf_debug("Frame count: %d\n", frame_count);

    KeyState keystate[320] = {RELEASED};
    bool keychange = false;
    bool power_pressed = false;

    uint32_t display_timer = 0,
             shutdown_timer = 0,
             acc_ticks = 0,
             last_ticks = SDL_GetTicks(),
             time_step = 1000 / fps;

    int current_frame = 0;

    // Set the CPU to powersave (charges faster?)
    system_powersave_on();

    while (!quit) {
        uint32_t ticks = SDL_GetTicks(),
                 delta = ticks - last_ticks;
        last_ticks = ticks;

        if ((keychange = updateKeystate(keystate, &quit))) {
            if (keystate[SW_BTN_POWER] == REPEATING && (ticks - shutdown_timer) > SHUTDOWN_TIMEOUT)
                quit = true; // power on
            else if (keystate[SW_BTN_POWER] == PRESSED) {
                shutdown_timer = ticks;
                power_pressed = true;
            }
            else if (keystate[SW_BTN_POWER] == RELEASED && power_pressed) {
                suspend(!suspended, video); // toggle display
                power_pressed = false;
            }
            display_timer = ticks;
        }

        if (!battery_isCharging()) {
            quit = true;
            turn_off = true;
        }

        if (quit)
            break;

        if (!suspended && (ticks - display_timer) > DISPLAY_TIMEOUT)
            suspend(true, video);

        if (suspended)
            continue;

        acc_ticks += delta;

        while (acc_ticks >= time_step) {
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

            acc_ticks -= time_step;
        }
    }

    // Clear the screen when exiting
    SDL_FillRect(video, NULL, 0);
    SDL_Flip(video);

    for (int i = 0; i < frame_count; i++)
        SDL_FreeSurface(frames[i]);
    SDL_FreeSurface(screen);
    SDL_FreeSurface(video);
    SDL_Quit();

    msleep(100);

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
