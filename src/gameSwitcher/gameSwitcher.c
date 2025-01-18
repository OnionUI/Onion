#include <SDL/SDL.h>
#include <SDL/SDL_image.h>
#include <SDL/SDL_ttf.h>
#include <fcntl.h>
#include <libgen.h>
#include <linux/fb.h>
#include <pthread.h>
#include <sqlite3/sqlite3.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <unistd.h>

#include "png/png.h"

#include "system/battery.h"
#include "system/lang.h"
#include "system/settings.h"
#include "system/state.h"
#include "theme/background.h"
#include "theme/sound.h"
#include "theme/theme.h"
#include "utils/config.h"
#include "utils/msleep.h"
#include "utils/surfaceSetAlpha.h"

#include "gs_appState.h"
#include "gs_history.h"
#include "gs_keystate.h"
#include "gs_overlay.h"
#include "gs_render.h"

int main(int argc, char *argv[])
{
    appState.is_overlay = argc > 1 && strcmp(argv[1], "--overlay") == 0;

    log_setName("gameSwitcher");
    print_debug("\n\nDebug logging enabled");

    signal(SIGINT, sigHandler);
    signal(SIGTERM, sigHandler);

    init(INIT_ALL);

    readFirstEntry();
    overlay_init();
    loadRomScreens();

    settings_load();
    lang_load();

    mkdirs("/mnt/SDCARD/.tmp_update/config/gameSwitcher");

    appState.show_time = config_flag_get("gameSwitcher/showTime");
    appState.show_total = !config_flag_get("gameSwitcher/hideTotal");
    appState.show_legend = !config_flag_get("gameSwitcher/hideLegend");
    appState.view_mode = appState.view_restore = config_flag_get("gameSwitcher/minimal") ? VIEW_MINIMAL : VIEW_NORMAL;

    appState.transparent_bg = SDL_CreateRGBSurface(0, g_display.width, g_display.height, 32, 0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000);
    SDL_FillRect(appState.transparent_bg, NULL, 0xBE000000);

    int battery_percentage = battery_getPercentage();

    appState.last_ticks = SDL_GetTicks();
    appState.legend_start = appState.last_ticks;
    appState.brightness_start = appState.last_ticks;

    appState.custom_header = loadOptionalImage("extra/gs-top-bar");
    appState.custom_footer = loadOptionalImage("extra/gs-bottom-bar");

    appState.header_height = getHeightOrDefault(appState.custom_header, 60.0 * g_scale);
    appState.footer_height = getHeightOrDefault(appState.custom_footer, 60.0 * g_scale);

    print_debug("gameSwitcher started\n");

    while (!appState.quit) {
        uint32_t ticks = SDL_GetTicks();
        appState.acc_ticks += ticks - appState.last_ticks;
        appState.last_ticks = ticks;

        if (appState.show_legend && ticks - appState.legend_start > appState.legend_timeout) {
            appState.show_legend = false;
            config_flag_set("gameSwitcher/hideLegend", true);
            appState.changed = true;
        }

        if (appState.brightness_changed && ticks - appState.brightness_start > appState.brightness_timeout) {
            appState.brightness_changed = false;
            appState.changed = true;
        }

        handleKeystate(&appState);

        if (battery_hasChanged(ticks, &battery_percentage))
            appState.changed = true;

        if (appState.acc_ticks >= appState.time_step) {
            appState.acc_ticks -= appState.time_step;

            if (!appState.changed && !appState.brightness_changed && (appState.surfaceGameName == NULL || appState.surfaceGameName->w <= appState.game_name_max_width))
                continue;

            Game_s *game = &game_list[appState.current_game];
            processItem(game);

            if (appState.changed) {
                SDL_FillRect(screen, NULL, 0);

                if (game_list_len == 0) {
                    appState.current_bg = NULL;
                    SDL_Surface *empty = resource_getSurface(EMPTY_BG);
                    SDL_Rect empty_rect = {(g_display.width - empty->w) / 2, (g_display.height - empty->h) / 2};
                    SDL_BlitSurface(empty, NULL, screen, &empty_rect);
                }
                else {
                    appState.current_bg = loadRomScreen(appState.current_game);

                    if (appState.current_bg != NULL) {
                        renderCentered(appState.current_bg, appState.view_mode, NULL, NULL);
                    }
                }
            }

            if (appState.view_mode != VIEW_FULLSCREEN && game_list_len > 0 && !appState.pop_menu_open) {
                renderGameName(&appState);
            }

            if (!appState.changed && !appState.brightness_changed) {
                render();
                continue;
            }

            if (appState.view_mode == VIEW_NORMAL && !appState.pop_menu_open) {
                renderFooter(&appState);
            }

            if (appState.view_mode == VIEW_NORMAL) {
                renderHeader(&appState, battery_percentage);
            }

            renderLegend(&appState);
            renderBrightness(&appState);

            if (!appState.first_render) {
                renderPopMenu(&appState);
            }

            render();

            if (appState.first_render) {
                appState.first_render = false;
                readHistory();
                loadRomScreens();
            }
            else {
                appState.changed = false;
            }
        }
    }

    if (appState.exit_to_menu) {
        print_debug("Exiting to menu");
        remove("/mnt/SDCARD/.tmp_update/.runGameSwitcher");
        remove("/mnt/SDCARD/.tmp_update/cmd_to_run.sh");
        overlay_exit();
        SDL_FillRect(screen, NULL, 0);
        render();
    }
    else if (currentGame()->is_running) {
        if (appState.current_bg != NULL) {
            SDL_FillRect(screen, NULL, 0);
            renderCentered(appState.current_bg, VIEW_FULLSCREEN, NULL, NULL);
        }
        overlay_resume();
    }
    else {
        printf_debug("Resuming game - current_game : %i - index: %i", appState.current_game, game_list[appState.current_game].index);
        resumeGame(game_list[appState.current_game].index);
        overlay_exit();
        render_showFullscreenMessage("LOADING", true);
    }

#ifndef PLATFORM_MIYOOMINI
    msleep(200);
#endif

    if (appState.custom_header != NULL)
        SDL_FreeSurface(appState.custom_header);
    if (appState.custom_footer != NULL)
        SDL_FreeSurface(appState.custom_footer);
    if (appState.surfaceGameName != NULL)
        SDL_FreeSurface(appState.surfaceGameName);
    if (appState.transparent_bg != NULL)
        SDL_FreeSurface(appState.transparent_bg);

    resources_free();

    freeRomScreens();
    ra_freeHistory();

    deinit();

    return EXIT_SUCCESS;
}
