#include <SDL/SDL.h>
#include <SDL/SDL_image.h>
#include <SDL/SDL_ttf.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "system/battery.h"
#include "system/settings.h"
#include "theme/config.h"
#include "theme/render/battery.h"
#include "theme/resources.h"
#include "utils/file.h"
#include "utils/flags.h"
#include "utils/log.h"

int main(int argc, char *argv[])
{
    // Boot : Loading screen
    // Save : Transitional panel with autosave (Game -> GS)
    // End_Save : Ending screen with save
    // End : Ending screen without save

    SDL_Init(SDL_INIT_VIDEO);

    char theme_path[STR_MAX];
    theme_getPath(theme_path);

    SDL_Surface *video = SDL_SetVideoMode(640, 480, 32, SDL_HWSURFACE | SDL_DOUBLEBUF);
    SDL_Surface *screen = SDL_CreateRGBSurface(SDL_HWSURFACE, 640, 480, 32, 0, 0, 0, 0);

    SDL_Surface *background;
    bool show_battery = true;
    bool show_version = true;

    int size_message_font = 18;
    int str_message_x = 620;
    int str_message_y = 450;
    int str_message_centered = 0;
    char str_message_font[STR_MAX];

    if (argc > 1 && strcmp(argv[1], "Splash_Text") == 0) {
        background = theme_loadImage(theme_path, "background");
        size_message_font = 28;
        str_message_x = 320;
        str_message_y = 240;
        str_message_centered = 1;
        strcpy(str_message_font, theme()->title.font);
    }
    
    else if (argc > 1 && strcmp(argv[1], "End_Save") == 0) {
        background = theme_loadImage(theme_path, "extra/Screen_Off_Save");
        strcpy(str_message_font, theme()->hint.font);
    }
    else if (argc > 1 && strcmp(argv[1], "End") == 0) {
        background = theme_loadImage(theme_path, "extra/Screen_Off");
        strcpy(str_message_font, theme()->hint.font);
    }
    else if (argc > 1 && strcmp(argv[1], "lowBat") == 0) {
        background = theme_loadImage(theme_path, "extra/lowBat");
        strcpy(str_message_font, theme()->hint.font);
        show_version = false;
    }
    else {
        background = theme_loadImage(theme_path, "extra/bootScreen");
        strcpy(str_message_font, theme()->hint.font);
    }

    char message_str[STR_MAX] = "";
    if (argc > 2) {
        strncpy(message_str, argv[2], STR_MAX - 1);
    }

    if (background) {
        SDL_BlitSurface(background, NULL, screen, NULL);
        SDL_FreeSurface(background);
    }

    TTF_Init();

    TTF_Font *font_message = theme_loadFont(theme_path, str_message_font, size_message_font);
    TTF_Font *font_version = theme_loadFont(theme_path, theme()->hint.font, 18);
  
    SDL_Color color = theme()->list.color;

    if (strlen(message_str) > 0) {
        SDL_Surface *message = TTF_RenderUTF8_Blended(font_message, message_str, color);
        if (message) {
            SDL_BlitSurface(message, NULL, screen, &(SDL_Rect){str_message_x - (message->w / (str_message_centered + 1)), str_message_y - message->h / 2});
            SDL_FreeSurface(message);
        }
    }

    if (show_version) {
        const char *version_str = file_read("/mnt/SDCARD/.tmp_update/onionVersion/version.txt");
        if (strlen(version_str) > 0) {
            SDL_Surface *version = TTF_RenderUTF8_Blended(font_version, version_str, color);
            if (version) {
                SDL_BlitSurface(version, NULL, screen, &(SDL_Rect){20, 450 - version->h / 2});
                SDL_FreeSurface(version);
            }
        }
    }

    if (show_battery) {
        SDL_Surface *battery = theme_batterySurface(battery_getPercentage());
        SDL_Rect battery_rect = {596 - battery->w / 2, 30 - battery->h / 2};
        SDL_BlitSurface(battery, NULL, screen, &battery_rect);
        SDL_FreeSurface(battery);
        resources_free();
    }

    // Blit twice, to clear the video buffer
    SDL_BlitSurface(screen, NULL, video, NULL);
    SDL_Flip(video);
    SDL_BlitSurface(screen, NULL, video, NULL);
    SDL_Flip(video);

    if (argc > 1 && strcmp(argv[1], "Boot") != 0)
        temp_flag_set(".offOrder", false);

#ifndef PLATFORM_MIYOOMINI
    sleep(4); // for debugging purposes
#endif

    SDL_FreeSurface(screen);
    SDL_FreeSurface(video);
    SDL_Quit();

    return EXIT_SUCCESS;
}
