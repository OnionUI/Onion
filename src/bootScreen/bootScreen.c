#include <stdlib.h>
#include <stdio.h>
#include <SDL/SDL.h>
#include <SDL/SDL_image.h>
#include <SDL/SDL_ttf.h>

#include "utils/file.h"
#include "utils/flags.h"
#include "utils/battery.h"

int main(int argc, char *argv[])
{
    // Boot : Loading screen
    // End_Save : Ending screen with save
    // End : Ending screen without save

    SDL_Init(SDL_INIT_VIDEO);
    TTF_Init();
    
    TTF_Font* font = TTF_OpenFont("/customer/app/Exo-2-Bold-Italic.ttf", 30); 
    SDL_Color color = {255, 255, 255};

    SDL_Surface* video = SDL_SetVideoMode(640, 480, 32, SDL_HWSURFACE | SDL_DOUBLEBUF);
    SDL_Surface* screen = SDL_CreateRGBSurface(SDL_HWSURFACE, 640, 480, 32, 0, 0, 0, 0);

    SDL_Surface* background;    
    int bShowBat = 0;

    if (argc > 1) {
        if (strcmp(argv[1],"Boot") == 0) {
            background = IMG_Load("res/bootScreen.png");
        }
        else if (strcmp(argv[1],"End_Save") == 0) {
            background = IMG_Load("res/Screen_Off_Save.png");
            bShowBat = 1;
        }
        else if (strcmp(argv[1],"End") == 0) {
            background = IMG_Load("res/Screen_Off.png");
            bShowBat = 1;
        }
    }
    else {
        background = IMG_Load("res/bootScreen.png");
    }
    
    SDL_BlitSurface(background, NULL, screen, NULL);
    SDL_FreeSurface(background);

    char version_str[12];
    const char *current_version = file_read("/mnt/SDCARD/.tmp_update/onionVersion/version.txt");
    sprintf(version_str, "v%s", current_version);
        
    SDL_Surface* imageVer = TTF_RenderUTF8_Blended(font, version_str, color);
    SDL_Rect rectVersion = {5, 445, 120, 40};
    SDL_BlitSurface(imageVer, NULL, screen, &rectVersion);
    SDL_FreeSurface(imageVer);

    if (bShowBat == 1) {
        SDL_Rect rectBatt = {566, 440, 113, 29};
        SDL_Rect rectBatteryIcon = {541, 447, 13, 27};    
        
        // Check current battery value
        char currBat[5];
        int nBat = battery_getPercentage();
        sprintf(currBat, "%d%%", nBat);
        SDL_Surface* battery_text = TTF_RenderUTF8_Blended(font, currBat, color);
        
        // Battery icon
        SDL_Surface* battery_icon;
        if (nBat > 100)
            battery_icon = IMG_Load("res/battCharge.png");
        else if (nBat >= 80)
            battery_icon = IMG_Load("res/batt100.png");
        else if (nBat >= 60)
            battery_icon = IMG_Load("res/batt80.png");
        else if (nBat >= 40)
            battery_icon = IMG_Load("res/batt60.png");
        else if (nBat >= 20)
            battery_icon = IMG_Load("res/batt40.png");
        else if (nBat >= 10)
            battery_icon = IMG_Load("res/batt20.png");
        else if (nBat >= 0)
            battery_icon = IMG_Load("res/batt0.png");
    
        SDL_BlitSurface(battery_text, NULL, screen, &rectBatt);
        SDL_BlitSurface(battery_icon, NULL, screen, &rectBatteryIcon);
        
        SDL_FreeSurface(battery_text);
        SDL_FreeSurface(battery_icon);
    }
    
    SDL_BlitSurface(screen, NULL, video, NULL); 
    SDL_Flip(video);

    if (argc > 1 && strcmp(argv[1], "Boot") != 0)
        temp_flag_set(".offOrder", false);

    SDL_FreeSurface(screen);
    SDL_FreeSurface(video);
    SDL_Quit();
    
    return EXIT_SUCCESS;
}
