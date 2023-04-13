#include "sys/ioctl.h"
#include <SDL/SDL.h>
#include <SDL/SDL_image.h>
#include <SDL/SDL_ttf.h>
#include <fcntl.h>
#include <linux/fb.h>
#include <linux/input.h>
#include <signal.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include "system/keymap_sw.h"
#include "system/system.h"
#include "utils/file.h"
#include "utils/keystate.h"
#include "utils/log.h"
#include "utils/str.h"

#include "../playActivity/playActivity.h"

static bool quit = false;

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
    log_setName("playActivityUI");
    printf_debug("main(%d, %s)\n", argc, argv[1]);
    
    open_db();
    PlayActivity **play_activities = find_play_activities("");
    close_db();
    if (play_activities == NULL) {
        return 1;
    }
    int play_activities_len = sizeof(PlayActivity *)/sizeof(play_activities);
    printf_debug("play_activities_len = %d\n", play_activities_len);
    
    signal(SIGINT, sigHandler);
    signal(SIGTERM, sigHandler);
    
    SDL_Init(SDL_INIT_VIDEO);
    SDL_ShowCursor(SDL_DISABLE);
    SDL_EnableKeyRepeat(300, 50);
    TTF_Init();
    
    SDL_Surface *video = SDL_SetVideoMode(640, 480, 32, SDL_HWSURFACE);
    SDL_Surface *screen =
    SDL_CreateRGBSurface(SDL_HWSURFACE, 640, 480, 32, 0, 0, 0, 0);
    
    TTF_Font *font40 = TTF_OpenFont("/customer/app/Exo-2-Bold-Italic.ttf", 40);
    TTF_Font *font30 = TTF_OpenFont("/customer/app/Exo-2-Bold-Italic.ttf", 30);
    TTF_Font *fontRomName25 =
    TTF_OpenFont("/customer/app/wqy-microhei.ttc", 25);
    
    SDL_Color color_white = {255, 255, 255};
    SDL_Color color_lilla = {136, 97, 252};
    
    SDL_Surface *imageBackground = IMG_Load("res/background.png");
    
    SDL_Surface *imageRomPosition;
    SDL_Surface *imageRomPlayTime;
    SDL_Surface *imageRomName;
    
    SDL_Surface *imagePages;
    SDL_Surface *imageMileage;
    
    SDL_BlitSurface(imageBackground, NULL, screen, NULL);
    
    SDL_Rect rectPages = {561, 430, 90, 44};
    SDL_Rect rectMileage = {484, 8, 170, 42};
    
    //    Mileage
    int ntotalTime = 0;
    for (int i = 0; i < play_activities_len; i++)
        ntotalTime += play_activities[i]->play_time;
    
    char cTotalHandheldMileage[30];
    
    int h, m;
    h = ntotalTime / 3600;
    m = (ntotalTime - 3600 * h) / 60;
    sprintf(cTotalHandheldMileage, "%d:%02d", h, m);
    
    int nPages = (int)((play_activities_len - 1) / 4 + 1);
    int nCurrentPage = 0;
    char cPosition[5];
    char cTotalTimePlayed[50];
    
    char cPages[10];
    
    sprintf(cPages, "%d/%d", nCurrentPage + 1, nPages);
    imagePages = TTF_RenderUTF8_Blended(font30, cPages, color_white);
    imageMileage =
    TTF_RenderUTF8_Blended(font30, cTotalHandheldMileage, color_white);
    
    for (int i = 0; i < 4; i++) {
        sprintf(cPosition, "%d", i + 1);
        
        h = play_activities[i]->play_time / 3600;
        m = (play_activities[i]->play_time - 3600 * h) / 60;
        
        if (strlen(play_activities[i]->name) != 0)
            sprintf(cTotalTimePlayed, "%d:%02d", h, m);
        else
            memset(cTotalTimePlayed, 0, sizeof(cTotalTimePlayed));
        
        imageRomPosition =
        TTF_RenderUTF8_Blended(font40, cPosition, color_lilla);
        imageRomPlayTime =
        TTF_RenderUTF8_Blended(font40, cTotalTimePlayed, color_white);
        imageRomName = TTF_RenderUTF8_Blended(fontRomName25, play_activities[i]->name,
                                              color_white);
        
        SDL_Rect rectPosition = {16, 78 + 90 * i, 76, 39};
        SDL_Rect rectRomPlayTime = {77, 66 + 90 * i, 254, 56};
        SDL_Rect rectRomNames = {78, 104 + 90 * i, 600, 40};
        
        SDL_BlitSurface(imageRomPosition, NULL, screen, &rectPosition);
        
        if (play_activities_len > i) {
            SDL_BlitSurface(imageRomPlayTime, NULL, screen, &rectRomPlayTime);
            SDL_BlitSurface(imageRomName, NULL, screen, &rectRomNames);
        }
    }
    
    SDL_BlitSurface(imagePages, NULL, screen, &rectPages);
    SDL_BlitSurface(imageMileage, NULL, screen, &rectMileage);
    
    SDL_BlitSurface(screen, NULL, video, NULL);
    SDL_Flip(video);
    
    bool changed;
    KeyState keystate[320] = {(KeyState)0};
    
    while (!quit) {
        
        msleep(20);
        changed = false;
        
        if (updateKeystate(keystate, &quit, true, NULL)) {
            if (keystate[SW_BTN_B] == PRESSED)
                quit = true;
            
            if (keystate[SW_BTN_RIGHT] >= PRESSED) {
                if (nCurrentPage < nPages - 1) {
                    nCurrentPage++;
                    changed = true;
                }
            }
            
            if (keystate[SW_BTN_LEFT] >= PRESSED) {
                if (nCurrentPage > 0) {
                    nCurrentPage--;
                    changed = true;
                }
            }
        }
        
        if (!changed)
            continue;
        
        // Page update
        SDL_BlitSurface(imageBackground, NULL, screen, NULL);
        
        sprintf(cPages, "%d/%d", nCurrentPage + 1, nPages);
        imagePages = TTF_RenderUTF8_Blended(font30, cPages, color_white);
        
        SDL_BlitSurface(imagePages, NULL, screen, &rectPages);
        SDL_BlitSurface(imageMileage, NULL, screen, &rectMileage);
        
        for (int i = 0; i < 4; i++) {
            PlayActivity *curr = play_activities[nCurrentPage * 4 + i];
            sprintf(cPosition, "%d", (int)(nCurrentPage * 4 + i + 1));
            h = curr->play_time / 3600;
            m = (curr->play_time - 3600 * h) / 60;
            
            if (strlen(curr->name) != 0)
                sprintf(cTotalTimePlayed, "%d:%02d", h, m);
            else
                memset(cTotalTimePlayed, 0, sizeof(cTotalTimePlayed));
            
            imageRomPosition =
            TTF_RenderUTF8_Blended(font40, cPosition, color_lilla);
            imageRomPlayTime =
            TTF_RenderUTF8_Blended(font40, cTotalTimePlayed, color_white);
            imageRomName =
            TTF_RenderUTF8_Blended(fontRomName25, curr->name, color_white);
            
            SDL_Rect rectPosition = {16, 78 + 90 * i, 76, 39};
            SDL_Rect rectRomPlayTime = {77, 66 + 90 * i, 254, 56};
            SDL_Rect rectRomNames = {78, 104 + 90 * i, 600, 40};
            
            SDL_BlitSurface(imageRomPosition, NULL, screen, &rectPosition);
            if (play_activities_len > (nCurrentPage * 4 + i)) {
                SDL_BlitSurface(imageRomPlayTime, NULL, screen,
                                &rectRomPlayTime);
                SDL_BlitSurface(imageRomName, NULL, screen, &rectRomNames);
            }
        }
        
        SDL_BlitSurface(screen, NULL, video, NULL);
        SDL_Flip(video);
    }
    
    // free memory for each struct
    for (int i = 0; i < sizeof(PlayActivity)/sizeof(play_activities); i++) {
        free(play_activities[i]);
    }
    // free memory for the array of struct pointers
    free(play_activities);
    
    SDL_FreeSurface(screen);
    SDL_FreeSurface(video);
    
#ifndef PLATFORM_MIYOOMINI
    msleep(200); // to clear SDL input on quit
#endif
    //  #18808
    SDL_Quit();
    return EXIT_SUCCESS;
}
