#ifndef GAME_SWITCHER_OVERLAY_H
#define GAME_SWITCHER_OVERLAY_H

#include <pthread.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "system/battery.h"
#include "system/screenshot.h"
#include "utils/msleep.h"
#include "utils/str.h"

#include "gs_appState.h"
#include "gs_model.h"
#include "gs_render.h"

static pthread_t autosave_thread_pt;
static bool autosave_thread_running = false;

void setFbAsFirstRomScreen(void)
{
    if (game_list_len == 0)
        return;

    Game_s *game = &game_list[0];

    if (game->romScreen != NULL) {
        SDL_FreeSurface(game->romScreen);
        game->romScreen = NULL;
    }

    game->romScreen = SDL_CreateRGBSurface(SDL_SWSURFACE, g_display.width, g_display.height, 32, 0, 0, 0, 0);
    display_readCurrentBuffer(&g_display, (uint32_t *)game->romScreen->pixels, (rect_t){0, 0, g_display.width, g_display.height}, true, false);

    if (game->romScreen == NULL) {
        print_debug("Error creating fb surface\n");
    }
}

static bool _isContentNameInInfo(const char *content_info, const char *content_name)
{
    const char *found = strstr(content_info, content_name);
    if (found != NULL) {
        return *(found - 1) == ',' && *(found + strlen(content_name)) == ',';
    }
    return false;
}

static void *_saveRomScreenAndStateThread(void *arg)
{
    Game_s *game = &game_list[0];

    if (game->romScreen != NULL && game->is_running) {
        char romScreenPath[STR_MAX];
        uint32_t hash = FNV1A_Pippip_Yurii(game->recentItem.rompath, strlen(game->recentItem.rompath));
        snprintf(romScreenPath, sizeof(romScreenPath), ROM_SCREENS_DIR "/%" PRIu32 ".png", hash);

        screenshot_save((uint32_t *)game->romScreen->pixels, romScreenPath, false);

        printf_debug("Saved rom screen: %s\n", romScreenPath);
    }

    retroarch_autosave();

    autosave_thread_running = false;
    return NULL;
}

void overlay_init()
{
    if (!appState.is_overlay) {
        return;
    }

    retroarch_pause();
    system("playActivity stop_all &");
    setFbAsFirstRomScreen();

    RetroArchStatus_s status;
    if (retroarch_getStatus(&status) == -1) {
        print_debug("Error getting RetroArch status");
        return;
    }

    printf_debug("RetroArch status: %d\n", status.state);
    printf_debug("Content info: %s\n", status.content_info);

    if (status.state == RETROARCH_STATE_CONTENTLESS || status.state == RETROARCH_STATE_UNKNOWN) {
        print_debug("RetroArch is not running a game");
        return;
    }

    if (status.state == RETROARCH_STATE_PLAYING) {
        retroarch_pause();
    }

    Game_s *game = &game_list[0];
    game->is_running = _isContentNameInInfo(status.content_info, game->rom_name);
    printf_debug("Game is running: %d\n", game->is_running);

    // start autosave thread
    autosave_thread_running = true;
    pthread_create(&autosave_thread_pt, NULL, _saveRomScreenAndStateThread, NULL);
}

void overlay_resume(void)
{
    if (appState.is_overlay) {
        if (autosave_thread_running) {
            // backup screen SDL_Surface
            SDL_Surface *screen_backup = SDL_CreateRGBSurface(SDL_SWSURFACE, screen->w, screen->h, 32, 0, 0, 0, 0);
            SDL_BlitSurface(screen, NULL, screen_backup, NULL);

            render_showFullscreenMessage("SAVING", false);

            // wait for autosave thread to finish
            pthread_join(autosave_thread_pt, NULL);

            SDL_BlitSurface(screen_backup, NULL, screen, NULL);
            SDL_FreeSurface(screen_backup);
        }

        render();

        retroarch_unpause();
        system("playActivity resume &");

        msleep(200);

        remove("/mnt/SDCARD/.tmp_update/.runGameSwitcher");
    }
}

void overlay_exit(void)
{
    if (appState.is_overlay) {
        if (autosave_thread_running) {
            render_showFullscreenMessage("SAVING", false);

            // wait for autosave thread to finish
            pthread_join(autosave_thread_pt, NULL);
        }

        // force kill retroarch
        temp_flag_set(".forceKillRetroarch", true);
        system("killall -9 retroarch");
    }
}

#endif // GAME_SWITCHER_OVERLAY_H
