#ifndef PACMAN_GLOBALS_H__
#define PACMAN_GLOBALS_H__

#include <SDL/SDL.h>
#include <SDL/SDL_image.h>
#include <SDL/SDL_ttf.h>
#include <dirent.h>
#include <fcntl.h>
#include <libgen.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include "utils/str.h"

#define PACKAGE_DIR "/mnt/SDCARD/App/PackageManager/data/"

// Max number of records in the DB
#define LAYER_ITEM_COUNT 200
#define MAX_LAYER_NAME_SIZE 256
#define MAY_LAYER_DISPLAY 35

typedef struct package_s {
    char name[STR_MAX];
    bool installed;
    bool changed;
    bool complete;
    bool has_roms;
} Package;

static char layer_names[][STR_MAX] = {"VERIFIED", "APPS", "EXPERT", "SUMMARY"};
static char layer_dirs[][STR_MAX] = {PACKAGE_DIR "Emu", PACKAGE_DIR "App",
                                     PACKAGE_DIR "RApp", ""};
static bool layer_check_roms[] = {true, false, true, false};
static const int tab_count = 4;
static const int summary_tab = tab_count - 1;

static Package packages[4][LAYER_ITEM_COUNT];
static int package_count[] = {0, 0, 0, 0};
static int package_installed_count[] = {0, 0, 0, 0};
static int nSelection = 0;
static int nListPosition = 0;
static int nTab = 0;
static int changes_installs[] = {0, 0, 0, 0};
static int changes_removals[] = {0, 0, 0, 0};

// Memory is free-ed by SDL_Quit
static SDL_Surface *video = NULL;

static SDL_Surface *screen = NULL, *surfaceBackground = NULL,
                   *surfaceSelection = NULL, *surfaceTableau = NULL,
                   *surfaceScroller = NULL, *surfaceMarker = NULL,
                   *surfaceCheck = NULL, *surfaceCross = NULL,
                   *surfaceDotActive = NULL, *surfaceDotNeutral = NULL,
                   *surfaceDotApplyActive = NULL,
                   *surfaceDotApplyNeutral = NULL,
                   *surfaceDotChanged = NULL,
                   *surfaceIconHasRoms = NULL;

static SDL_Rect rectSummaryPos = {30, 85};
static SDL_Surface *surfaceSummary = NULL;
static int summaryScrollY = 0;
static int scrollSpeed = 28;

static TTF_Font *font18 = NULL;
static TTF_Font *font25 = NULL;
static TTF_Font *font35 = NULL;

static SDL_Color color_white = {255, 255, 255};
static SDL_Color color_red = {247, 145, 145};
static SDL_Color color_green = {154, 247, 145};

void initResources(void)
{
    SDL_Init(SDL_INIT_VIDEO);
    SDL_ShowCursor(SDL_DISABLE);
    SDL_EnableKeyRepeat(300, 50);
    TTF_Init();

    video = SDL_SetVideoMode(640, 480, 32, SDL_HWSURFACE);
    screen = SDL_CreateRGBSurface(SDL_HWSURFACE, 640, 480, 32, 0, 0, 0, 0);

    surfaceBackground = IMG_Load("res/bgApp.png");
    surfaceSelection = IMG_Load("res/selection.png");
    surfaceTableau = IMG_Load("res/tableau.png");
    surfaceScroller = IMG_Load("res/scroller.png");
    surfaceMarker = IMG_Load("res/marker.png");
    surfaceDotActive = IMG_Load("res/dot-a.png");
    surfaceDotNeutral = IMG_Load("res/dot-n.png");
    surfaceDotApplyActive = IMG_Load("res/dot-apply-a.png");
    surfaceDotApplyNeutral = IMG_Load("res/dot-apply-n.png");
    surfaceDotChanged = IMG_Load("res/changed.png");
    surfaceIconHasRoms = IMG_Load("res/has_roms.png");
    surfaceCheck = IMG_Load("/mnt/SDCARD/.tmp_update/res/toggle-on.png");
    surfaceCross = IMG_Load("/mnt/SDCARD/.tmp_update/res/toggle-off.png");

    font18 = TTF_OpenFont("/customer/app/Exo-2-Bold-Italic.ttf", 18);
    font25 = TTF_OpenFont("/customer/app/Exo-2-Bold-Italic.ttf", 25);
    font35 = TTF_OpenFont("/customer/app/Exo-2-Bold-Italic.ttf", 35);
}

void freeResources(void)
{
    TTF_CloseFont(font18);
    TTF_CloseFont(font25);
    TTF_CloseFont(font35);
    TTF_Quit();

    SDL_FreeSurface(screen);
    SDL_FreeSurface(surfaceSummary);
    SDL_FreeSurface(surfaceCheck);
    SDL_FreeSurface(surfaceCross);
    SDL_FreeSurface(surfaceBackground);
    SDL_FreeSurface(surfaceTableau);
    SDL_FreeSurface(surfaceSelection);
    SDL_FreeSurface(surfaceScroller);
    SDL_FreeSurface(surfaceMarker);
    SDL_FreeSurface(surfaceDotActive);
    SDL_FreeSurface(surfaceDotNeutral);
    SDL_FreeSurface(surfaceDotApplyActive);
    SDL_FreeSurface(surfaceDotApplyNeutral);
    SDL_FreeSurface(surfaceDotChanged);
    SDL_FreeSurface(surfaceIconHasRoms);
    SDL_Quit();
}

#endif // PACMAN_GLOBALS_H__