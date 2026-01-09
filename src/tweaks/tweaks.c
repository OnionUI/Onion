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
#include "system/device_model.h"
#include "system/display.h"
#include "system/keymap_sw.h"
#include "system/lang.h"
#include "system/settings.h"
#include "theme/background.h"
#include "theme/sound.h"
#include "theme/theme.h"
#include "utils/file.h"
#include "utils/keystate.h"
#include "utils/log.h"
#include "utils/sdl_init.h"

#include "./appstate.h"
#include "./info_dialog.h"
#include "./menus.h"

#define FRAMES_PER_SECOND 60

bool check_menuHasChanges(List *menu)
{
    bool modified = false;
    if (menu->_created) {
        for (int i = 0; i < menu->item_count; i++) {
            ListItem *item = &menu->items[i];
            if (item->item_type != ACTION) {
                modified |= item->value != item->_reset_value;
            }
        }
    }
    return modified;
}

void check_networkChanged(void)
{
    bool modified = temp_flag_get("network_changed");
    modified |= check_menuHasChanges(&_menu_network);
    modified |= check_menuHasChanges(&_menu_telnet);
    modified |= check_menuHasChanges(&_menu_ftp);
    modified |= check_menuHasChanges(&_menu_http);
    modified |= check_menuHasChanges(&_menu_ssh);
    modified |= check_menuHasChanges(&_menu_date_time);
    temp_flag_set("network_changed", modified);
}

int main(int argc, char *argv[])
{
    log_setName("tweaks");
    print_debug("Debug logging enabled");

    getDeviceModel();

    char apply_tool[STR_MAX] = "";
    bool use_display = true;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--apply_tool") == 0)
            strncpy(apply_tool, argv[++i], STR_MAX - 1);
        else if (strcmp(argv[i], "--no_display") == 0)
            use_display = false;
    }

    signal(SIGINT, sigHandler);
    signal(SIGTERM, sigHandler);

    if (use_display || strlen(apply_tool) == 0)
        SDL_InitDefault();

    settings_load();

    lang_load();

    // Apply tool via command line
    if (strlen(apply_tool) > 0) {
        for (int i = 0; i < NUM_TOOLS; i++) {
            if (strncmp(tools_short_names[i], apply_tool, STR_MAX - 1) == 0) {
                printf("Tool '%s':\n", tools_short_names[i]);
                (*tools_pt[i])(NULL);
                break;
            }
        }
        return 0;
    }

    int battery_percentage = battery_getPercentage();

    menu_main();

    uint32_t acc_ticks = 0, last_ticks = SDL_GetTicks(),
             time_step = 1000 / FRAMES_PER_SECOND;

    bool menu_combo_pressed = false;
    bool key_changed = false;
    SDLKey changed_key = SDLK_UNKNOWN;

    bool show_help_tooltip = !config_flag_get(".tweaksHelpCompleted");

    while (!quit) {
        uint32_t ticks = SDL_GetTicks();
        acc_ticks += ticks - last_ticks;
        last_ticks = ticks;

        if (updateKeystate(keystate, &quit, keys_enabled, &changed_key)) {
            if (keystate[SW_BTN_MENU] >= PRESSED && changed_key != SW_BTN_MENU)
                menu_combo_pressed = true;

            if (keystate[SW_BTN_UP] >= PRESSED) {
                key_changed = list_keyUp(menu_stack[menu_level], keystate[SW_BTN_UP] == REPEATING);
            }
            else if (keystate[SW_BTN_DOWN] >= PRESSED) {
                key_changed = list_keyDown(menu_stack[menu_level], keystate[SW_BTN_DOWN] == REPEATING);
            }
            else if (keystate[SW_BTN_LEFT] >= PRESSED) {
                key_changed = list_keyLeft(menu_stack[menu_level], keystate[SW_BTN_LEFT] == REPEATING);
            }
            else if (keystate[SW_BTN_RIGHT] >= PRESSED) {
                key_changed = list_keyRight(menu_stack[menu_level], keystate[SW_BTN_RIGHT] == REPEATING);
            }
            else if (keystate[SW_BTN_Y] == PRESSED) {
                key_changed = list_resetCurrentItem(menu_stack[menu_level]);
            }
            else if (keystate[SW_BTN_SELECT] == PRESSED) {
                if (list_hasInfoNote(menu_stack[menu_level])) {
                    sound_change();
                    showInfoDialog(menu_stack[menu_level]);
                }
            }
            else if (keystate[SW_BTN_A] == PRESSED) {
                if (list_currentItem(menu_stack[menu_level])->action != NULL) {
                    sound_change();
                    skip_next_change = true;
                    keystate[SW_BTN_A] = RELEASED;
                }
                key_changed = list_activateItem(menu_stack[menu_level]) || header_changed;
            }
            else if (changed_key == SW_BTN_MENU && keystate[SW_BTN_MENU] == RELEASED) {
                if (!menu_combo_pressed)
                    quit = true;
                menu_combo_pressed = false;
            }
            else if (keystate[SW_BTN_B] == PRESSED) {
                if (menu_level == 0)
                    quit = true;
                else {
                    menu_stack[menu_level] = NULL;
                    menu_level--;
                    header_changed = true;
                    key_changed = true;
                }
            }

            if (!skip_next_change) {
                if (key_changed)
                    sound_change();
            }
            else
                skip_next_change = false;

            list_changed = list_changed || key_changed;
            key_changed = false;
        }

        if (reset_menus)
            menu_resetAll();

        if (all_changed) {
            header_changed = true;
            list_changed = true;
            footer_changed = true;
            battery_changed = true;
        }

        if (quit)
            break;

        if (show_help_tooltip) {
            show_help_tooltip = false;
            config_flag_set(".tweaksHelpCompleted", true);
            __showInfoDialog("Welcome to Tweaks!",
                             ":: TOOLTIPS ::\n"
                             " \n"
                             "Press SELECT to view a tooltip\n"
                             "describing the selected option."
                             " \n"
                             "Press any button to close");
        }

        if (battery_hasChanged(ticks, &battery_percentage))
            battery_changed = true;

        blf_changing = exists("/tmp/blue_light_script.lock");

        if (acc_ticks >= time_step) {
            if (isMenu(&_menu_date_time) || isMenu(&_menu_user_blue_light)) {
                if (isMenu(&_menu_date_time)) {
                    if (_writeDateString(_menu_date_time.items[0].label)) {
                        list_changed = true;
                    }
                }
                if (IS_MIYOO_PLUS_OR_FLIP()) {
                    if (isMenu(&_menu_user_blue_light)) {
                        if (_writeDateString(_menu_user_blue_light.items[0].label)) {
                            list_changed = true;
                        }
                    }
                }
            }
            if (isMenu(&_menu_network) || isMenu(&_menu_wifi)) {
                network_loadState();
                if (netinfo_getIpAddress(ip_address_label, network_state.hotspot ? "wlan1" : "wlan0")) {
                    if (_menu_network._created)
                        strcpy(_menu_network.items[0].label, ip_address_label);
                    if (_menu_wifi._created)
                        strcpy(_menu_wifi.items[0].label, ip_address_label);
                    list_changed = true;
                }
            }

            if (header_changed || battery_changed)
                theme_renderHeader(screen, menu_stack[menu_level]->title, false);

            if (list_changed)
                theme_renderList(screen, menu_stack[menu_level]);

            if (footer_changed) {
                theme_renderFooter(screen);
                theme_renderStandardHint(screen, lang_get(LANG_SELECT, LANG_FALLBACK_SELECT), lang_get(LANG_BACK, LANG_FALLBACK_BACK));
            }

            if (footer_changed || list_changed)
                theme_renderFooterStatus(screen, menu_stack[menu_level]->active_pos + 1, menu_stack[menu_level]->item_count);

            if (header_changed || battery_changed)
                theme_renderHeaderBattery(screen, battery_percentage);

            if (header_changed || list_changed || footer_changed || battery_changed) {
                SDL_BlitSurface(screen, NULL, video, NULL);
                SDL_Flip(video);
            }

            if (blf_changing != prev_blf_changing) {
                reset_menus = true;
                prev_blf_changing = blf_changing;
            }

            header_changed = false;
            footer_changed = false;
            list_changed = false;
            battery_changed = false;
            all_changed = false;

            acc_ticks -= time_step;
        }
    }

    // Clear the screen when exiting
    SDL_FillRect(video, NULL, 0);
    SDL_Flip(video);

    settings_save();
    value_setFrameThrottle();
    value_setSwapTriggers();

    if (IS_MIYOO_PLUS_OR_FLIP()) {
        value_setLcdVoltage();
        check_networkChanged();
    }

    Mix_CloseAudio();

    network_freeSmbShares();
    diags_freeEntries();

    display_close();

    lang_free();
    menu_free_all();
    resources_free();
    SDL_FreeSurface(screen);
    SDL_FreeSurface(video);

#ifndef PLATFORM_MIYOOMINI
    msleep(200); // to clear SDL input on quit
#endif

    SDL_Quit();

    return EXIT_SUCCESS;
}
