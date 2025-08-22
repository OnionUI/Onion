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

// Check if this is the first GameSwitcher launch after boot with an autoresumed game
static bool isAutoResumedFirstLaunch(void)
{
    // Check for a marker file that indicates GameSwitcher has been launched this session
    const char *marker_file = "/tmp/.gameswitcher_launched";
    
    // Check if we're in overlay mode (launched from a game)
    if (!appState.is_overlay) {
        // Not launched from a game, so not an autoresume scenario
        // Create the marker file for this session
        FILE *fp = fopen(marker_file, "w");
        if (fp) {
            fclose(fp);
        }
        return false;
    }
    
    // We're in overlay mode, check if marker exists
    if (access(marker_file, F_OK) == 0) {
        // Marker exists, so GameSwitcher was already launched this session
        // This is a normal game-to-game switch
        return false;
    }
    
    // We're in overlay mode and no marker exists
    // This means we're being launched from an autoresumed game after boot
    
    // Additional check: verify RetroArch is actually running with content
    RetroArchStatus_s status;
    if (retroarch_getStatus(&status) == 0 && 
        status.state != RETROARCH_STATE_CONTENTLESS && 
        status.state != RETROARCH_STATE_UNKNOWN) {
        
        // Create the marker file now
        FILE *fp = fopen(marker_file, "w");
        if (fp) {
            fclose(fp);
        }
        
        return true;
    }
    
    // Create the marker file anyway
    FILE *fp = fopen(marker_file, "w");
    if (fp) {
        fclose(fp);
    }
    
    return false;
}

// Handle the autoresume scenario by exiting RetroArch and restarting fresh
static void handleAutoResumeScenario(void)
{
    // Stop playActivity immediately to prevent artificial time logging
    system("playActivity stop_all &");
    
    // The screenshot has already been captured and saved before this function was called
    
    // Now show the message to the user
    render_showFullscreenMessage("RESTARTING", true);
    msleep(500);
    
    // Pause retroarch to stop it from updating the display
    retroarch_pause();
    msleep(200);  // Give pause time to take effect
    
    // Gracefully shutdown RetroArch
    system("killall -TERM retroarch");
    
    // Adaptive wait time - check more frequently at first, then slow down
    // This handles both fast and slow cores better
    int wait_count = 0;
    bool retroarch_exited = false;
    
    // First 5 seconds - check every 250ms (fast cores)
    for (int i = 0; i < 20 && !retroarch_exited; i++) {
        msleep(250);
        if (system("pidof retroarch > /dev/null") != 0) {
            retroarch_exited = true;
            break;
        }
        wait_count++;
    }
    
    // Next 5 seconds - check every 500ms (slower cores)
    if (!retroarch_exited) {
        for (int i = 0; i < 10; i++) {
            msleep(500);
            if (system("pidof retroarch > /dev/null") != 0) {
                retroarch_exited = true;
                break;
            }
            wait_count++;
        }
    }
    
    // Force kill if still running
    if (!retroarch_exited) {
        temp_flag_set(".forceKillRetroarch", true);
        system("killall -9 retroarch");
        msleep(1500);  // Extra time after force kill for cleanup
    } else {
        // Even if it exited gracefully, give it time to clean up
        msleep(1000 + (wait_count * 50));  // More wait time for slower cores
    }
    
    // Clear any resume flags and command scripts
    remove("/mnt/SDCARD/.tmp_update/.runGameSwitcher");
    remove("/mnt/SDCARD/.tmp_update/cmd_to_run.sh");
    
    // Write a state file to indicate we came from mainui
    FILE *fp = fopen("/tmp/prev_state", "w");
    if (fp) {
        fprintf(fp, "mainui\n");
        fclose(fp);
    }
    
    // Additional wait to ensure system is fully stabilized
    // Longer for cases where we had to wait longer for RetroArch
    msleep(1500 + (wait_count * 100));
    
    // Clear any stale input events that accumulated during shutdown
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        // Discard all pending events
    }
    
    // Flush any remaining system messages
    sync();
    
    // Set flag to indicate normal OS mode
    appState.exit_to_menu = false;  // Stay in GameSwitcher
    appState.quit = false;           // Don't quit
    appState.is_overlay = false;     // Treat as non-overlay mode now
    appState.changed = true;
    appState.first_render = true;    // Force a fresh render cycle
    
    // Reset the game list position
    appState.current_game = 0;
    appState.current_game_changed = true;
    
    // Update the current background to use the fresh screenshot
    if (game_list_len > 0 && game_list[0].romScreen != NULL) {
        appState.current_bg = game_list[0].romScreen;
    }
    
    // Clear and refresh the screen multiple times to ensure display sync
    for (int i = 0; i < 3; i++) {
        SDL_FillRect(screen, NULL, 0);
        SDL_Flip(screen);
        msleep(100);
    }
    
    // Final stabilization delay
    msleep(500);
}

int main(int argc, char *argv[])
{
    // Check if another instance is already running
    FILE *pid_file = fopen("/tmp/gameswitcher.pid", "r");
    if (pid_file) {
        int existing_pid;
        if (fscanf(pid_file, "%d", &existing_pid) == 1) {
            fclose(pid_file);
            // Check if the process is actually running
            char proc_path[256];
            snprintf(proc_path, sizeof(proc_path), "/proc/%d", existing_pid);
            if (access(proc_path, F_OK) == 0) {
                // If launched from OS (no overlay flag), kill the existing instance
                if (!(argc > 1 && strcmp(argv[1], "--overlay") == 0)) {
                    kill(existing_pid, SIGTERM);
                    msleep(500);
                }
                else {
                    // Overlay mode but another instance exists - just exit
                    return EXIT_SUCCESS;
                }
            }
        }
        else {
            fclose(pid_file);
        }
    }
    
    // Write our PID
    pid_file = fopen("/tmp/gameswitcher.pid", "w");
    if (pid_file) {
        fprintf(pid_file, "%d", getpid());
        fclose(pid_file);
    }
    
    appState.is_overlay = argc > 1 && strcmp(argv[1], "--overlay") == 0;

    log_setName("gameSwitcher");
    print_debug("\n\nDebug logging enabled");

    signal(SIGINT, sigHandler);
    signal(SIGTERM, sigHandler);

    init(INIT_ALL);

    // Check for autoresume scenario BEFORE any game processing
    bool was_autoresumed = isAutoResumedFirstLaunch();
    
    readFirstEntry();
    
    if (was_autoresumed) {
        // Capture fresh screenshot BEFORE overlay_init takes the stale one
        if (game_list_len > 0) {
            Game_s *game = &game_list[0];
            
            // Capture current framebuffer
            SDL_Surface *fresh_screenshot = SDL_CreateRGBSurface(SDL_SWSURFACE, g_display.width, g_display.height, 32, 0, 0, 0, 0);
            display_readCurrentBuffer(&g_display, (uint32_t *)fresh_screenshot->pixels, 
                                    (rect_t){0, 0, g_display.width, g_display.height}, true, false);
            
            // Save it immediately
            char romScreenPath[STR_MAX];
            uint32_t hash = FNV1A_Pippip_Yurii(game->recentItem.rompath, strlen(game->recentItem.rompath));
            snprintf(romScreenPath, sizeof(romScreenPath), ROM_SCREENS_DIR "/%" PRIu32 ".png", hash);
            screenshot_save((uint32_t *)fresh_screenshot->pixels, romScreenPath, false);
            
            // Set it as the game's screenshot
            if (game->romScreen != NULL) {
                SDL_FreeSurface(game->romScreen);
            }
            game->romScreen = fresh_screenshot;
        }
        
        handleAutoResumeScenario();
        // Don't call overlay_init() since we've already cleaned up RetroArch
    }
    else {
        // Normal case - call overlay_init
        overlay_init();
    }
    
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
    
    // Clean up PID file
    remove("/tmp/gameswitcher.pid");

    deinit();

    return EXIT_SUCCESS;
}