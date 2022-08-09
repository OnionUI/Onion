#include <stdlib.h>
#include <stdio.h>
#include <SDL/SDL.h>
#include <SDL/SDL_image.h>
#include <SDL/SDL_ttf.h>

#include "utils/file.h"
#include "utils/flags.h"
#include "utils/battery.h"
#include "utils/log.h"
#include "system/settings.h"
#include "theme/theme.h"

#define RESOURCES { \
	TR_BATTERY_0, \
	TR_BATTERY_20, \
	TR_BATTERY_50, \
	TR_BATTERY_80, \
	TR_BATTERY_100, \
	TR_BATTERY_CHARGING \
}
#define NUM_RESOURCES 6

int main(int argc, char *argv[])
{
    // Boot : Loading screen
    // End_Save : Ending screen with save
    // End : Ending screen without save

    SDL_Init(SDL_INIT_VIDEO);
    TTF_Init();

    settings_load();
    Theme_s theme = loadThemeFromPath(settings.theme);

    TTF_Font* font = theme_loadFont(&theme, theme.hint.font, theme.hint.size);
    SDL_Color color = theme.total.color;

    SDL_Surface* video = SDL_SetVideoMode(640, 480, 32, SDL_HWSURFACE | SDL_DOUBLEBUF);
    SDL_Surface* screen = SDL_CreateRGBSurface(SDL_HWSURFACE, 640, 480, 32, 0, 0, 0, 0);

    SDL_Surface* background;
    int bShowBat = 0;

    if (argc > 1 && strcmp(argv[1], "End_Save") == 0) {
        background = theme_loadImage(&theme, "extra/Screen_Off_Save");
        bShowBat = 1;
    }
    else if (argc > 1 && strcmp(argv[1], "End") == 0) {
        background = theme_loadImage(&theme, "extra/Screen_Off");
        bShowBat = 1;
    }
    else {
        char path[256];
        printf_debug("Background: (%d) %s\n", theme_getImagePath(&theme, "extra/bootScreen", path) ? 1 : 0, path);
        background = theme_loadImage(&theme, "extra/bootScreen");
    }

    SDL_BlitSurface(background, NULL, screen, NULL);
    SDL_FreeSurface(background);

    char version_str[12];
    sprintf(version_str, "v%s", file_read("/mnt/SDCARD/.tmp_update/onionVersion/version.txt"));

    SDL_Surface* version = TTF_RenderUTF8_Blended(font, version_str, color);
    SDL_Rect rectVersion = {20, 450 - version->h / 2};
    SDL_BlitSurface(version, NULL, screen, &rectVersion);
    SDL_FreeSurface(version);

    if (bShowBat == 1) {
        enum theme_Images res_requests[NUM_RESOURCES] = RESOURCES;
        Resources_s res = theme_loadResources(&theme, res_requests, NUM_RESOURCES);

        if (file_exists("/tmp/percBat")) {
            SDL_Surface* battery = theme_batterySurface(&theme, &res, battery_getPercentage());
            SDL_Rect battery_rect = {596 - battery->w / 2, 450 - battery->h / 2};
            SDL_BlitSurface(battery, NULL, screen, &battery_rect);
            SDL_FreeSurface(battery);
        }

        theme_freeResources(&res);
    }

    // Blit twice, to clear the video buffer
    SDL_BlitSurface(screen, NULL, video, NULL);
    SDL_Flip(video);
    SDL_BlitSurface(screen, NULL, video, NULL);
    SDL_Flip(video);

    if (argc > 1 && strcmp(argv[1], "Boot") != 0)
        temp_flag_set(".offOrder", false);

    SDL_FreeSurface(screen);
    SDL_FreeSurface(video);
    SDL_Quit();

    return EXIT_SUCCESS;
}
