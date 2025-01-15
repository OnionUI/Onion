#include <SDL/SDL.h>
#include <SDL/SDL_image.h>
#include <SDL/SDL_ttf.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "system/battery.h"
#include "system/settings.h"
#include "theme/config.h"
#include "theme/render/bootScreen.h"
#include "theme/resources.h"
#include "utils/file.h"
#include "utils/flags.h"
#include "utils/log.h"
#include "utils/sdl_direct_fb.h"

int main(int argc, char *argv[])
{
    // Boot : Loading screen
    // End_Save : Ending screen with save
    // End : Ending screen without save

    init(INIT_PNG);

    if (argc > 1 && strcmp(argv[1], "clear") == 0) {
        SDL_FillRect(screen, NULL, 0);
        render();
        return EXIT_SUCCESS;
    }

    char theme_path[STR_MAX];
    theme_getPath(theme_path);

    ThemeImages background = BOOT_SCREEN;
    bool show_battery = false;
    bool show_version = true;

    if (argc > 1 && strcmp(argv[1], "End_Save") == 0) {
        background = SCREEN_OFF_SAVE;
        show_battery = true;
    }
    else if (argc > 1 && strcmp(argv[1], "End") == 0) {
        background = SCREEN_OFF;
        show_battery = true;
    }
    else if (argc > 1 && strcmp(argv[1], "lowBat") == 0) {
        background = LOW_BAT;
        if (!background) {
            show_battery = true;
        }
        show_version = false;
    }

    char message_str[STR_MAX] = "";
    if (argc > 2) {
        strncpy(message_str, argv[2], STR_MAX - 1);
    }

    TTF_Init();

    char version_str[STR_MAX] = "";
    if (show_version) {
        char *version = file_read("/mnt/SDCARD/.tmp_update/onionVersion/version.txt");
        if (strlen(version) > 0) {
            strncpy(version_str, version, STR_MAX - 1);
        }
        free(version);
    }

    theme_renderBootScreen(screen, background, version_str, message_str, show_battery ? battery_getPercentage() : -1);

    // Blit twice, to clear the video buffer
    render();
    render();

    resources_free();

#ifndef PLATFORM_MIYOOMINI
    sleep(4); // for debugging purposes
#endif

    TTF_Quit();

    deinit();

    return EXIT_SUCCESS;
}