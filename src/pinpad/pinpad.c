#include <SDL/SDL.h>
#include <SDL/SDL_ttf.h>
#include <fcntl.h>
#include <linux/input.h>
#include <signal.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "system/battery.h"
#include "system/keymap_hw.h"
#include "system/keymap_sw.h"
#include "system/lang.h"
#include "system/settings.h"
#include "theme/background.h"
#include "theme/color.h"
#include "theme/sound.h"
#include "theme/theme.h"
#include "utils/keystate.h"
#include "utils/log.h"
#include "utils/msleep.h"
#include "utils/sdl_init.h"
#include "utils/str.h"

#define FRAMES_PER_SECOND 60
#define SHUTDOWN_TIMEOUT 500
#define PIN_LEN 4

static bool quit = false;
static bool confirmed = false;
static KeyState keystate[320] = {(KeyState)0};

static int digits[PIN_LEN] = {0, 0, 0, 0};
static int cursor = 0;

static char title_str[STR_MAX] = "Enter PIN";
static char message_str[STR_MAX] = "";
static char initial_value[STR_MAX] = "";

static void parse_initial_digits(const char *value)
{
    int idx = 0;

    for (int i = 0; value[i] != '\0' && idx < PIN_LEN; i++) {
        if (value[i] >= '0' && value[i] <= '9') {
            digits[idx++] = value[i] - '0';
        }
    }
}

static void parse_args(int argc, char *argv[])
{
    for (int i = 1; i < argc; i++) {
        if (argv[i][0] == '-') {
            if ((strcmp(argv[i], "-t") == 0 || strcmp(argv[i], "--title") == 0) && i + 1 < argc) {
                strncpy(title_str, argv[++i], STR_MAX - 1);
                continue;
            }
            if ((strcmp(argv[i], "-m") == 0 || strcmp(argv[i], "--message") == 0) && i + 1 < argc) {
                strncpy(message_str, argv[++i], STR_MAX - 1);
                continue;
            }
            if ((strcmp(argv[i], "-i") == 0 || strcmp(argv[i], "--initial") == 0) && i + 1 < argc) {
                strncpy(initial_value, argv[++i], STR_MAX - 1);
                continue;
            }
        }
    }

    parse_initial_digits(initial_value);
}

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

static void render_pinpad(SDL_Surface *message_surface, SDL_Surface *hint_surface, int battery_percentage)
{
    SDL_Color border_normal = theme()->grid.color;
    SDL_Color border_active = theme()->grid.selectedcolor;
    SDL_Color digit_normal = theme()->list.color;
    SDL_Color digit_active = theme()->grid.selectedcolor;

    Uint32 bg_fill = colorToUint((SDL_Color){24, 24, 24});

    theme_renderHeader(screen, title_str, false);
    theme_renderHeaderBattery(screen, battery_percentage);

    if (message_surface != NULL) {
        SDL_Rect msg_rect = {
            .x = (g_display.width - message_surface->w) / 2,
            .y = (int)(88.0 * g_scale)
        };
        SDL_BlitSurface(message_surface, NULL, screen, &msg_rect);
    }

    int slot_w = (int)(72.0 * g_scale);
    int slot_h = (int)(96.0 * g_scale);
    int gap = (int)(22.0 * g_scale);
    int total_w = PIN_LEN * slot_w + (PIN_LEN - 1) * gap;
    int start_x = (g_display.width - total_w) / 2;
    int start_y = (int)(190.0 * g_scale);
    int border = (int)(3.0 * g_scale);

    for (int i = 0; i < PIN_LEN; i++) {
        bool active = (i == cursor);
        SDL_Color border_color = active ? border_active : border_normal;
        SDL_Color digit_color = active ? digit_active : digit_normal;

        SDL_Rect outer = {
            .x = start_x + i * (slot_w + gap),
            .y = start_y,
            .w = slot_w,
            .h = slot_h
        };
        SDL_FillRect(screen, &outer, colorToUint(border_color));

        SDL_Rect inner = {
            .x = outer.x + border,
            .y = outer.y + border,
            .w = outer.w - border * 2,
            .h = outer.h - border * 2
        };
        SDL_FillRect(screen, &inner, bg_fill);

        char digit_str[2] = {(char)('0' + digits[i]), '\0'};
        SDL_Surface *digit_surface = TTF_RenderUTF8_Blended(resource_getFont(TITLE), digit_str, digit_color);
        if (digit_surface != NULL) {
            SDL_Rect digit_rect = {
                .x = outer.x + (outer.w - digit_surface->w) / 2,
                .y = outer.y + (outer.h - digit_surface->h) / 2
            };
            SDL_BlitSurface(digit_surface, NULL, screen, &digit_rect);
            SDL_FreeSurface(digit_surface);
        }
    }

    if (hint_surface != NULL) {
        SDL_Rect hint_rect = {
            .x = (g_display.width - hint_surface->w) / 2,
            .y = (int)(322.0 * g_scale)
        };
        SDL_BlitSurface(hint_surface, NULL, screen, &hint_rect);
    }

    theme_renderFooter(screen);
    theme_renderStandardHint(screen, "Confirm", "Cancel");
}

int main(int argc, char *argv[])
{
    signal(SIGINT, sigHandler);
    signal(SIGTERM, sigHandler);

    parse_args(argc, argv);

    if (!SDL_InitDefault()) {
        return 1;
    }

    settings_load();
    lang_load();

    int battery_percentage = battery_getPercentage();

    SDL_Surface *message_surface = NULL;
    if (message_str[0] != '\0') {
        char *msg_with_newlines = str_replace(message_str, "\\n", "\n");
        message_surface = theme_textboxSurface(msg_with_newlines, resource_getFont(LIST), theme()->grid.color, ALIGN_CENTER);
        free(msg_with_newlines);
    }

    SDL_Surface *hint_surface = theme_textboxSurface(
        "D-Pad to edit pin",
        resource_getFont(HINT),
        theme()->hint.color,
        ALIGN_CENTER);

    bool key_changed = false;
    SDLKey changed_key = SDLK_UNKNOWN;

#ifdef PLATFORM_MIYOOMINI
    bool first_draw = true;
    int input_fd = open("/dev/input/event0", O_RDONLY);
    struct input_event ev;
    uint32_t shutdown_timer = 0;
#endif

    uint32_t acc_ticks = 0;
    uint32_t last_ticks = SDL_GetTicks();
    uint32_t time_step = 1000 / FRAMES_PER_SECOND;

    while (!quit) {
        uint32_t ticks = SDL_GetTicks();
        acc_ticks += ticks - last_ticks;
        last_ticks = ticks;

#ifdef PLATFORM_MIYOOMINI
        if (!first_draw) {
            read(input_fd, &ev, sizeof(ev));
            int val = ev.value;
            if (ev.type == EV_KEY && val <= 2 && ev.code == HW_BTN_POWER) {
                if (val == 2 && (ticks - shutdown_timer) > SHUTDOWN_TIMEOUT)
                    quit = true;
                else if (val == 1)
                    shutdown_timer = ticks;
            }
        }
#endif

        if (updateKeystate(keystate, &quit, true, &changed_key)) {
            if (keystate[SW_BTN_UP] >= PRESSED) {
                digits[cursor] = (digits[cursor] + 1) % 10;
                key_changed = true;
            }
            else if (keystate[SW_BTN_DOWN] >= PRESSED) {
                digits[cursor] = (digits[cursor] + 9) % 10;
                key_changed = true;
            }
            else if (changed_key == SW_BTN_LEFT && keystate[SW_BTN_LEFT] == PRESSED) {
                if (cursor > 0)
                    cursor--;
                key_changed = true;
            }
            else if (changed_key == SW_BTN_RIGHT && keystate[SW_BTN_RIGHT] == PRESSED) {
                if (cursor < PIN_LEN - 1)
                    cursor++;
                key_changed = true;
            }

            if ((changed_key == SW_BTN_START && keystate[SW_BTN_START] == PRESSED) ||
                (changed_key == SW_BTN_A && keystate[SW_BTN_A] == RELEASED) ||
                (changed_key == SW_BTN_B && keystate[SW_BTN_B] == RELEASED)) {
                confirmed = true;
                quit = true;
                key_changed = true;
            }
            else if (changed_key == SW_BTN_SELECT && keystate[SW_BTN_SELECT] == PRESSED) {
                confirmed = false;
                quit = true;
                key_changed = true;
            }
        }

        if (key_changed) {
            sound_change();
            key_changed = false;
        }

        if (battery_hasChanged(ticks, &battery_percentage)) {
            // battery value refreshed; render below will pick it up.
        }

        if (acc_ticks >= time_step) {
            render_pinpad(message_surface, hint_surface, battery_percentage);

#ifdef PLATFORM_MIYOOMINI
            first_draw = false;
#endif

            SDL_BlitSurface(screen, NULL, video, NULL);
            SDL_Flip(video);
            acc_ticks -= time_step;
        }
    }

    SDL_FillRect(video, NULL, 0);
    SDL_Flip(video);

    if (confirmed) {
        printf("PIN:%d%d%d%d\n", digits[0], digits[1], digits[2], digits[3]);
    }

    lang_free();

#ifdef PLATFORM_MIYOOMINI
    close(input_fd);
#endif

    if (message_surface)
        SDL_FreeSurface(message_surface);
    if (hint_surface)
        SDL_FreeSurface(hint_surface);

    Mix_CloseAudio();
    resources_free();
    SDL_FreeSurface(screen);
    SDL_FreeSurface(video);

#ifndef PLATFORM_MIYOOMINI
    msleep(200);
#endif

    SDL_Quit();

    return confirmed ? 0 : 1;
}
