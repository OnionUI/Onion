#include <SDL/SDL.h>
#include <SDL/SDL_image.h>
#include <SDL/SDL_ttf.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdint.h>
#include <string.h>
#include <fcntl.h>
#include <stdbool.h>
#include <sys/stat.h>
#include "sys/ioctl.h"
#include <dirent.h>
#include <stdlib.h>

#include "system/keymap_hw.h"
#include "system/keymap_sw.h"
#include "utils/utils.h"
#include "utils/log.h"
#include "utils/msleep.h"
#include "utils/keystate.h"

#ifndef DT_DIR
#define DT_DIR 4
#endif

#define PACKAGE_LAYER_1 "data/Layer1"
#define PACKAGE_LAYER_2 "data/Layer2"
#define PACKAGE_LAYER_3 "data/Layer3"

// Max number of records in the DB
#define NUMBER_OF_LAYERS 200
#define MAX_LAYER_NAME_SIZE 256
#define MAY_LAYER_DISPLAY 35

static char layers[3][NUMBER_OF_LAYERS][MAX_LAYER_NAME_SIZE];
static int bInstall[3][NUMBER_OF_LAYERS];
static int bInstallChange[3][NUMBER_OF_LAYERS];
static int nb_Layers[3];
static int nSelection = 0;
static int nListPosition = 0;
static int nTab = 0;
static int allActivated = 0;

static SDL_Surface *video = NULL,
                   *screen = NULL,
                   *surfaceBackground = NULL,
                   *surfaceSelection = NULL,
                   *surfaceTableau = NULL,
                   *surfacesTabSelection = NULL,
                   *surfaceScroller = NULL,
                   *surfaceCheck = NULL,
                   *surfaceCross = NULL;

static SDL_Color color_white = {255, 255, 255};

void setLayersInstall (int bInstallValue, int bInstallValueChange)
{
    for(int n = 0 ; n < 3 ; n++){
        for(int i = 0 ; i < NUMBER_OF_LAYERS ; i++){
            bInstall[n][i] = bInstallValue;
            bInstallChange[n][i] = bInstallValueChange;
        }
    }
}

void appUninstall(char *basePath, int nT, int index, int strlenBase)
{
    char path[1000];
    char pathInstalledApp[1000];
    char *basePathDestination = "/mnt/SDCARD";

    struct dirent *dp;
    DIR *dir = opendir(basePath);

    // Unable to open directory stream
    if (!dir)
        return;

    while ((dp = readdir(dir)) != NULL) {
        if (strcmp(dp->d_name, ".") != 0 && strcmp(dp->d_name, "..") != 0) {
            // Construct new path from our base path
            sprintf(path, "%s/%s", basePath, dp->d_name);

            if (exists(path)) {
                sprintf(pathInstalledApp, "/mnt/SDCARD%s", path + strlenBase);

                if (is_file(pathInstalledApp))
                    remove(pathInstalledApp);
                
                if (is_dir(path))
                    appUninstall(path, nT, index, strlenBase);

                if (is_dir(pathInstalledApp))
                    rmdir(pathInstalledApp);
            }
        }
    }

    closedir(dir);
}

bool checkAppInstalled(const char *basePath, int nT, int index, int base_len, int level)
{
    char path[1000];
    char pathInstalledApp[1000];

    struct dirent *dp;
    DIR *dir = opendir(basePath);

    int run = 1;
    bool is_installed = true;

    // Unable to open directory stream
    if (!dir)
        return true;

    while ((dp = readdir(dir)) != NULL && run == 1) {
        if (strcmp(dp->d_name, ".") != 0 && strcmp(dp->d_name, "..") != 0) {
            // Construct new path from our base path
            sprintf(path, "%s/%s", basePath, dp->d_name);

            if (exists(path)) {
                sprintf(pathInstalledApp, "/mnt/SDCARD%s", path + base_len);

                if (!exists(pathInstalledApp))
                    is_installed = false;
                else if (dp->d_type == DT_DIR)
                    is_installed = checkAppInstalled(path, nT, index, base_len, level + 1);

                if (!is_installed)
                    run = 0;
            }
        }
    }

    if (level == 0 && is_installed)
        bInstall[nT][index] = 1;

    closedir(dir);

    return is_installed;
}

void loadRessources()
{
    int nT = 0;
    char cCommand[500];
    for (int nT = 0 ; nT < 3 ; nT ++){
        DIR *dp;
        struct dirent *ep;

        char cSingleRessource[250];

        char ressourcesPath[200] ;
        nb_Layers[nT]=0;

        switch(nT) {
            case 0: sprintf(ressourcesPath, "%s", PACKAGE_LAYER_1); break;
            case 1: sprintf(ressourcesPath, "%s", PACKAGE_LAYER_2); break;
            case 2: sprintf(ressourcesPath, "%s", PACKAGE_LAYER_3); break;
            default: break;
        }

        if (!exists(ressourcesPath))
            continue;

        dp = opendir(ressourcesPath);

        if (dp != NULL) {
            while ((ep = readdir (dp)) && (nb_Layers[nT]<NUMBER_OF_LAYERS)){
                char cShort[MAX_LAYER_NAME_SIZE];
                strcpy(cShort, ep->d_name);
                cShort[MAY_LAYER_DISPLAY] = '\0';
                size_t len = strlen(cShort);
                if ((len > 2)||(cShort[0]!='.')){
                    // Installation check
                    char basePath[1000];
                    sprintf(basePath,"%s/%s", ressourcesPath, cShort);

                    checkAppInstalled(basePath, nT , nb_Layers[nT], strlen(basePath), 0);

                    strcpy(layers[nT][nb_Layers[nT]],cShort);
                    nb_Layers[nT] ++;
                }

            }
            closedir(dp);
        }
        else{
            perror ("Couldn't open the directory");
        }
    }

    // Sort function
    for (int nT = 0 ; nT < 3 ; nT ++){
        char tempFolder[MAX_LAYER_NAME_SIZE];
        int bInstallTemp;

        int bFound = 1;
        while (bFound == 1){
            bFound = 0;
            for (int i = 0 ; i < nb_Layers[nT]-1; i ++){
                if (strcmp(layers[nT][i], layers[nT][i+1])>0){

                    strcpy(tempFolder, layers[nT][i+1]);
                    strcpy(layers[nT][i+1], layers[nT][i]);
                    strcpy(layers[nT][i], tempFolder);

                    bInstallTemp = bInstall[nT][i+1];
                    bInstall[nT][i+1] = bInstall[nT][i];
                    bInstall[nT][i] = bInstallTemp;

                    bFound = 1 ;
                }
            }

        }
    }
}

void displayLayersNames(){
    SDL_Rect rectRessName = {35, 92, 80, 20};
    SDL_Surface* surfaceRessName;
    TTF_Font* font25 = TTF_OpenFont("/customer/app/Exo-2-Bold-Italic.ttf", 25);

    for (int i = 0 ; i < 7 ; i++){
        if ((i + nListPosition) < nb_Layers[nTab]){
            surfaceRessName = TTF_RenderUTF8_Blended(font25, layers[nTab][i + nListPosition], color_white);
            rectRessName.y = 92 + i * 47;
            SDL_BlitSurface(surfaceRessName, NULL, screen, &rectRessName);
        }
    }
    TTF_CloseFont(font25);
    SDL_FreeSurface(surfaceRessName);
}

void displayLayersInstall(){
    SDL_Rect rectInstall = {567, 96, 27, 27};

    for (int i = 0 ; i < 7 ; i++){
        if ((i + nListPosition) < nb_Layers[nTab]){

            rectInstall.y = 96 + i * 47;
            if (bInstall[nTab][i + nListPosition] == 1){
                SDL_BlitSurface(surfaceCheck, NULL, screen, &rectInstall);
            }
            else {
                SDL_BlitSurface(surfaceCross, NULL, screen, &rectInstall);
            }

        }
    }

}

void showScroller()
{
    int shiftY = 0;
    if (nb_Layers[nTab] - 7 > 0)
        shiftY = (int)(nListPosition * 311 / (nb_Layers[nTab] - 7));
    SDL_Rect rectSroller = { 608, 86 + shiftY, 16, 16};
    SDL_BlitSurface(surfaceScroller, NULL, screen, &rectSroller);
}

bool confirmDoNothing(KeyState keystate[320])
{
    bool quit = false;
    SDL_Surface* image = IMG_Load("res/confirmDoNothing.png");

    SDL_BlitSurface(image, NULL, screen, NULL);
    SDL_BlitSurface(screen, NULL, video, NULL);
    SDL_Flip(video);

    while (!quit) {
        if (updateKeystate(keystate, &quit)) {
            if (*(keystate + SW_BTN_A) == PRESSED)
                return true;
            if (*(keystate + SW_BTN_B) == PRESSED)
                return false;
        }
    }

    SDL_FreeSurface(image);
}

int main(int argc, char *argv[])
{
    bool show_confirm = false;

    if (argc > 1 && strcmp(argv[1], "--confirm") == 0)
        show_confirm = true;

    SDL_Init(SDL_INIT_VIDEO);
    SDL_ShowCursor(SDL_DISABLE);
    SDL_EnableKeyRepeat(300, 50);
    TTF_Init();

	video = SDL_SetVideoMode(640,480, 32, SDL_HWSURFACE);
	screen = SDL_CreateRGBSurface(SDL_HWSURFACE, 640,480, 32, 0,0,0,0);

	surfaceBackground = IMG_Load("res/bgApp.png");
	surfaceSelection = IMG_Load("res/selection.png");
	surfaceTableau = IMG_Load("res/tableau.png");
	surfacesTabSelection = IMG_Load("res/selectionTitle.png");
	surfaceScroller = IMG_Load("res/scroller.png");
	surfaceCheck = IMG_Load("res/checked.png");
	surfaceCross = IMG_Load("res/cross.png");

    SDL_Surface *loadingScreen = IMG_Load("res/loading.png");
    SDL_BlitSurface(loadingScreen, NULL, screen, NULL);
    SDL_BlitSurface(screen, NULL, video, NULL);
    SDL_Flip(video);
    SDL_FreeSurface(loadingScreen);

    setLayersInstall (0,0);
    loadRessources();

    SDL_Color color_pink = {136, 97, 252};
    SDL_Rect rectSelection = {15, 84, 593, 49};
    SDL_Rect rectTitle = {457, 9, 200, 50};
    SDL_Rect rectTabSelection = {15, 59, 199, 26};

    bool quit = false;
    bool changed = true;
    KeyState keystate[320] = {0};

    int changes_total = 0;
    bool apply_changes = false;

    while (!quit) {
        if (updateKeystate(keystate, &quit)) {
            if (keystate[SW_BTN_RIGHT] >= PRESSED) {
                if (nTab < 2) {
                    nTab++;
                    nSelection = 0;
                    nListPosition = 0;
                    changed = true;
                }
            }
            if (keystate[SW_BTN_LEFT] >= PRESSED) {
                if (nTab > 0) {
                    nTab--;
                    nSelection = 0;
                    nListPosition = 0;
                    changed = true;
                }
            }

            if (keystate[SW_BTN_R1] >= PRESSED && nb_Layers[nTab] > 0) {
                if ((nListPosition + 14) <nb_Layers[nTab]){
                    nListPosition += 7;
                }
                else if ((nListPosition + 7) <nb_Layers[nTab]){
                    nListPosition = nb_Layers[nTab] - 7;
                    nSelection = 6 ;
                }
                changed = true;
            }

            if (keystate[SW_BTN_L1] >= PRESSED && nb_Layers[nTab] > 0) {
                if ((nListPosition - 7) > 0) {
                    nListPosition -= 7;
                }
                else {
                    nListPosition = 0;
                    nSelection = 0;

                }
                changed = true;
            }

            if (keystate[SW_BTN_DOWN] >= PRESSED && nb_Layers[nTab] > 0) {
                if (nSelection < 6){
                    nSelection ++;
                }
                else if ((nSelection+nListPosition) < nb_Layers[nTab]-1){
                    nListPosition++;
                }
                changed = true;
            }
            if (keystate[SW_BTN_UP] >= PRESSED && nb_Layers[nTab] > 0) {
                if (nSelection > 0){
                    nSelection --;
                }
                else if (nListPosition > 0){
                    nListPosition--;
                }
                changed = true;
            }

            if (keystate[SW_BTN_B] == PRESSED || keystate[SW_BTN_START] == PRESSED) {
                if (keystate[SW_BTN_START] == PRESSED)
                    apply_changes = true;

                if (show_confirm) {
                    changes_total = 0;
                    
                    for (int i = 0; i < 3; i++)
                        for (int j = 0; j < nb_Layers[i]; j++)
                            if (bInstallChange[i][j] == 1)
                                changes_total++;

                    if (changes_total > 0 || confirmDoNothing(&keystate)) {
                        quit = true;
                    }
                }
                else {
                    quit = true;
                }
                changed = true;
            }

            if (keystate[SW_BTN_A] == PRESSED && nb_Layers[nTab] > 0) {
                if (nListPosition+nSelection<nb_Layers[nTab]){
                    if (bInstall[nTab][nListPosition+nSelection] == 1){
                        bInstall[nTab][nListPosition+nSelection] = 0;
                        bInstallChange[nTab][nListPosition+nSelection] = 1;
                    }
                    else{
                        bInstall[nTab][nListPosition+nSelection] = 1;
                        bInstallChange[nTab][nListPosition+nSelection] = 1;
                    }
                    if (nSelection < 6){
                        nSelection ++;
                    }
                    else if ((nSelection+nListPosition) < nb_Layers[nTab]-1){
                        nListPosition++;
                    }
                    changed = true;
                }
            }
        }

        if (quit)
            break;

        if (changed) {
            rectSelection.y = 84 + nSelection * 47;
            rectTabSelection.x = 15 + (199 * nTab);

            SDL_BlitSurface(surfaceBackground, NULL, screen, NULL);
            SDL_BlitSurface(surfacesTabSelection, NULL, screen, &rectTabSelection);
            SDL_BlitSurface(surfaceTableau, NULL, screen, NULL);
            SDL_BlitSurface(surfaceSelection, NULL, screen, &rectSelection);

            if (nb_Layers[nTab] > 0){
                displayLayersNames();
                showScroller();
                displayLayersInstall();
            }

            SDL_BlitSurface(screen, NULL, video, NULL);
            SDL_Flip(video);

            changed = false;
        }
    }

    if (apply_changes) {
        // installation
        char param1[250];
        char param2[60];
        char ressourcesPath[250];
        char cCommand[500];

        SDL_Surface* surfaceBackground = IMG_Load("/mnt/SDCARD/.tmp_update/res/waitingBG.png");
        SDL_Surface* surfaceMessage;
        TTF_Font* font35 = TTF_OpenFont("/customer/app/Exo-2-Bold-Italic.ttf", 35);

        for (int nT = 0 ; nT < 3 ; nT ++){
            switch(nT) {
                case 0: sprintf(ressourcesPath, "%s", PACKAGE_LAYER_1); break;
                case 1: sprintf(ressourcesPath, "%s", PACKAGE_LAYER_2); break;
                case 2: sprintf(ressourcesPath, "%s", PACKAGE_LAYER_3); break;
                default: break;
            }

            if (!exists(ressourcesPath))
                continue;

            sprintf(param1,"%s%d","/mnt/SDCARD/App/The_Onion_Installer/data/Layer",(nT+1));

            SDL_Rect rectMessage = { 10, 420 , 603, 48};

            for (int nLayer = 0 ; nLayer < nb_Layers[nT] ; nLayer++){
                if (bInstallChange[nT][nLayer] != 1)
                    continue;

                if (bInstall[nT][nLayer] == 1){
                    surfaceMessage = TTF_RenderUTF8_Blended(font35, layers[nT][nLayer], color_white);
                    SDL_BlitSurface(surfaceBackground, NULL, screen, NULL);
                    SDL_BlitSurface(surfaceMessage, NULL, screen, &rectMessage);
                    SDL_FreeSurface(surfaceMessage);
                    SDL_BlitSurface(screen, NULL, video, NULL);
                    SDL_Flip(video);

                    sprintf(cCommand, "cd /mnt/SDCARD/App/The_Onion_Installer; ./install.sh \"%s\" \"%s\"",param1, layers[nT][nLayer]);
                    system(cCommand);
                }
                else {
                    // app uninstallation
                    char pathAppUninstal[1000];
                    pathAppUninstal[0]='\0';
                    strcat(pathAppUninstal,param1);
                    strcat(pathAppUninstal,"/");
                    strcat(pathAppUninstal,layers[nT][nLayer]);
                    appUninstall(pathAppUninstal, nT, nLayer, strlen(pathAppUninstal));
                    // A second pass for deleting the empty folders
                    appUninstall(pathAppUninstal, nT, nLayer, strlen(pathAppUninstal));
                }
            }

        }
        TTF_CloseFont(font35);
    }

    TTF_Quit();
    if (surfaceCheck != NULL) SDL_FreeSurface(surfaceCheck);
    if (surfaceCross != NULL) SDL_FreeSurface(surfaceCross);
    SDL_FreeSurface(surfaceBackground);
    SDL_FreeSurface(surfaceTableau);
    SDL_FreeSurface(surfaceSelection);
    SDL_FreeSurface(surfaceScroller);
    SDL_FreeSurface(surfacesTabSelection);
    SDL_Quit();

    msleep(100);

    return EXIT_SUCCESS;
}
