#include <SDL/SDL.h>
#include <SDL/SDL_image.h>
#include <SDL/SDL_ttf.h>
#include <dirent.h>
#include <linux/input.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include "components/list.h"
#include "system/battery.h"
#include "system/display.h"
#include "system/keymap_hw.h"
#include "system/keymap_sw.h"
#include "system/lang.h"
#include "system/settings.h"
#include "theme/background.h"
#include "theme/sound.h"
#include "theme/theme.h"
#include "utils/keystate.h"
#include "utils/log.h"
#include "utils/sdl_init.h"

#define FRAMES_PER_SECOND 60
#define SHUTDOWN_TIMEOUT 500

#define MAX_ELEMENTS 100

static bool quit = false;
static KeyState keystate[320] = {(KeyState)0};

void __showInfoDialog(const char *title, const char *message)
{
    bool confirm_quit = false;
    SDLKey changed_key = SDLK_UNKNOWN;

    SDL_Surface *background_surface = SDL_CreateRGBSurface(SDL_HWSURFACE, 640, 480, 32, 0, 0, 0, 0);
    SDL_BlitSurface(screen, NULL, background_surface, NULL);

    theme_renderDialog(screen, title, message, false);
    SDL_BlitSurface(screen, NULL, video, NULL);
    SDL_Flip(video);

    while (!confirm_quit) {
        if (updateKeystate(keystate, &confirm_quit, true, &changed_key)) {
            if ((changed_key == SW_BTN_A || changed_key == SW_BTN_B || changed_key == SW_BTN_SELECT) && keystate[changed_key] == PRESSED) {
                confirm_quit = true;
                sound_change();
            }
        }
    }
    SDL_FreeSurface(background_surface);
}

void showInfoDialog(List *list)
{
    ListItem *item = list_currentItem(list);
    __showInfoDialog(item->label, item->info_note);
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

int main(int argc, char *argv[])
{
    print_debug("Debug logging enabled, prompt v2!");

    signal(SIGINT, sigHandler);
    signal(SIGTERM, sigHandler);

    int pargc = 0;
    char **pargs = NULL;
    char title_str[STR_MAX] = "";
    char message_str[STR_MAX] = "";
    bool required = false;
    int selected = 0;
    bool has_info = false;

    pargs = malloc(MAX_ELEMENTS * sizeof(char *));

    int i;
    for (i = 1; i < argc; i++) {
        if (argv[i][0] == '-') {
            if (strcmp(argv[i], "-t") == 0 || strcmp(argv[i], "--title") == 0) {
                strncpy(title_str, argv[++i], STR_MAX - 1);
                continue;
            }
            if (strcmp(argv[i], "-m") == 0 ||
                strcmp(argv[i], "--message") == 0) {
                strncpy(message_str, argv[++i], STR_MAX - 1);
                continue;
            }
            if (strcmp(argv[i], "-r") == 0 ||
                strcmp(argv[i], "--required") == 0) {
                required = true;
                continue;
            }
            if (strcmp(argv[i], "-i") == 0 ||
                strcmp(argv[i], "--info") == 0) {
                has_info = true;
                continue;
            }
            if (strcmp(argv[i], "-s") == 0 ||
                strcmp(argv[i], "--selected") == 0) {
                selected = atoi(argv[++i]);
                continue;
            }
        }
        if (pargc < MAX_ELEMENTS && strlen(argv[i]) > 0) {
            pargs[pargc] = malloc((STR_MAX + 1) * sizeof(char));
            strncpy(pargs[pargc], argv[i], STR_MAX);
            pargc++;
        }
    }

    printf_debug(LOG_SUCCESS, "parsed command line arguments");

    SDL_InitDefault();

    settings_load();
    lang_load();

    int battery_percentage = battery_getPercentage();

    if (pargc == 0) {
        pargs[pargc++] = (char *)lang_get(LANG_OK, LANG_FALLBACK_OK);
        pargs[pargc++] = (char *)lang_get(LANG_CANCEL, LANG_FALLBACK_CANCEL);
    }

    List list = list_create(pargc, LIST_SMALL);
    if (has_info) {
        for (i = 0; i < (pargc >> 1); i++) {
            ListItem item = {.action_id = i, .action = NULL};
            strncpy(item.label, pargs[i], STR_MAX - 1);
            strncpy(item.info_note, pargs[i + (pargc >> 1)], STR_MAX - 1);
            printf_debug("Adding list item: %s %s (%d)\n", item.label, item.info_note, item.action_id);
            list_addItemWithInfoNote(&list, item, item.info_note);
        }
    }
    else {
        for (i = 0; i < pargc; i++) {
            ListItem item = {.action_id = i, .action = NULL};
            strncpy(item.label, pargs[i], STR_MAX - 1);
            printf_debug("Adding list item: %s (%d)\n", item.label, item.action_id);
            list_addItem(&list, item);
        }
    }

    list_scrollTo(&list, selected);

    bool has_title = strlen(title_str) > 0;

    SDL_Surface *message = NULL;
    SDL_Rect message_rect;
    bool has_message = strlen(message_str) > 0;

    if (has_message) {
        char *str = str_replace(message_str, "\\n", "\n");
        printf_debug("Message: %s\n", str);
        message = theme_textboxSurface(str, resource_getFont(TITLE),
                                       theme()->grid.color, ALIGN_CENTER);
        free(str);

        if (message) {
            int max_scroll_height = (360 - (message->h + 20)) / 60;
            if (max_scroll_height == 0)
                max_scroll_height = 1;
            else if (max_scroll_height > 6)
                max_scroll_height = 6;
            if (list.item_count < max_scroll_height)
                list.scroll_height = list.item_count;
            else
                list.scroll_height = max_scroll_height;
            message_rect.x = 320 - message->w / 2;
            message_rect.y =
                60 + (6 - list.scroll_height) * 30 - message->h / 2;
        }
        else {
            has_message = false;
        }
    }

    bool list_changed = true;
    bool header_changed = true;
    bool footer_changed = true;
    bool battery_changed = true;

    SDLKey changed_key = SDLK_UNKNOWN;
    bool key_changed = false;
    bool info_showned = false;

#ifdef PLATFORM_MIYOOMINI
    bool first_draw = true;
    int input_fd;
    input_fd = open("/dev/input/event0", O_RDONLY);
    struct input_event ev;
    uint32_t shutdown_timer = 0;
#endif

    int return_code = -1;

    uint32_t acc_ticks = 0, last_ticks = SDL_GetTicks(),
             time_step = 1000 / FRAMES_PER_SECOND;

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
            if (keystate[SW_BTN_DOWN] >= PRESSED) {
                key_changed =
                    list_keyDown(&list, keystate[SW_BTN_DOWN] == REPEATING);
                list_changed = true;
            }
            else if (keystate[SW_BTN_UP] >= PRESSED) {
                key_changed =
                    list_keyUp(&list, keystate[SW_BTN_UP] == REPEATING);
                list_changed = true;
            }

            if (changed_key == SW_BTN_A && keystate[SW_BTN_A] == RELEASED) {
                return_code = list_currentItem(&list)->action_id;
                quit = true;
            }
            if (changed_key == SW_BTN_SELECT && keystate[SW_BTN_SELECT] == PRESSED) {
                if (list_hasInfoNote(&list)) {
                    sound_change();
                    showInfoDialog(&list);
                }
                info_showned = true;
                key_changed = true;

                header_changed = true; // We need to do this because some themes will darken the more times you check the info
                footer_changed = true; // So we need to force it to reload
            }

            else if (!required && changed_key == SW_BTN_B &&
                     keystate[SW_BTN_B] == PRESSED) {
                quit = true;
            }
        }

        if (key_changed || quit) {
            sound_change();
            key_changed = false;
        }

        if (quit)
            break;

        if (battery_hasChanged(ticks, &battery_percentage))
            battery_changed = true;

        if (acc_ticks >= time_step) {
            if (header_changed || battery_changed) {
                theme_renderHeader(screen, has_title ? title_str : NULL,
                                   !has_title);
            }
            if (list_changed || info_showned) {
                theme_renderList(screen, &list);

                if (has_message)
                    SDL_BlitSurface(message, NULL, screen, &message_rect);
            }

            if (footer_changed) {
                theme_renderFooter(screen);
                theme_renderStandardHint(
                    screen, lang_get(LANG_SELECT, LANG_FALLBACK_SELECT),
                    required ? NULL : lang_get(LANG_BACK, LANG_FALLBACK_BACK));
            }

            if (footer_changed || list_changed)
                theme_renderFooterStatus(screen, list.active_pos + 1,
                                         list.item_count);

            if (header_changed || battery_changed)
                theme_renderHeaderBattery(screen, battery_getPercentage());

            footer_changed = false;
            header_changed = false;
            list_changed = false;
            battery_changed = false;
            info_showned = false;

#ifdef PLATFORM_MIYOOMINI
            first_draw = false;
#endif

            SDL_BlitSurface(screen, NULL, video, NULL);
            SDL_Flip(video);

            acc_ticks -= time_step;
        }
    }

    // Clear the screen when exiting
    SDL_FillRect(video, NULL, 0);
    SDL_Flip(video);

    lang_free();

    if (pargs != NULL) {
        for (i = 0; i < pargc; i++) {
            free(pargs[i]);
        }
        free(pargs);
    }

#ifdef PLATFORM_MIYOOMINI
    close(input_fd);
#endif

    print_debug("Freeing list...");
    list_free(&list);
    printf_debug(LOG_SUCCESS, "freed list");

    if (has_message)
        SDL_FreeSurface(message);

    Mix_CloseAudio();

    resources_free();
    SDL_FreeSurface(screen);
    SDL_FreeSurface(video);

#ifndef PLATFORM_MIYOOMINI
    msleep(200); // to clear SDL input on quit
#endif
    SDL_Quit();

    return return_code;
}
