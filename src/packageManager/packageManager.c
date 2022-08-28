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
#include "utils/surfaceSetAlpha.h"

#ifndef DT_DIR
#define DT_DIR 4
#endif

#define PACKAGE_LAYER_1 "/mnt/SDCARD/Packages/Emu"
#define PACKAGE_LAYER_2 "/mnt/SDCARD/Packages/App"
#define PACKAGE_LAYER_3 "/mnt/SDCARD/Packages/RApp"

// Max number of records in the DB
#define NUMBER_OF_LAYERS 200
#define MAX_LAYER_NAME_SIZE 256
#define MAY_LAYER_DISPLAY 35

typedef struct package_s {
    char name[STR_MAX];
    bool installed;
    bool changed;
    bool complete;
} Package;

static Package packages[3][NUMBER_OF_LAYERS];
static int package_count[3];
static int package_installed_count[] = {0, 0, 0};
static int nSelection = 0;
static int nListPosition = 0;
static int nTab = 0;
static int changes_installs = 0;
static int changes_removals = 0;

static SDL_Surface *video = NULL,
                   *screen = NULL,
                   *surfaceBackground = NULL,
                   *surfaceSelection = NULL,
                   *surfaceTableau = NULL,
                   *surfacesTabSelection = NULL,
                   *surfaceScroller = NULL,
                   *surfaceCheck = NULL,
                   *surfaceCross = NULL;

static TTF_Font *font25 = NULL;
static TTF_Font *font35 = NULL;

static SDL_Color color_white = {255, 255, 255};

int changesTotal(void)
{
    return changes_installs + changes_removals;
}

void setLayersInstall(bool should_install, int layer)
{
    for (int i = 0; i < package_count[layer]; i++) {
        Package *package = &packages[layer][i];
        bool new_value = package->installed ? !should_install : should_install;

        if (package->changed != new_value) {
            package->changed = new_value;

            if (package->installed) {
                changes_removals += new_value ? 1 : -1;
                if (!package->complete)
                    changes_installs += new_value ? -1 : 1;
            }
            else changes_installs += new_value ? 1 : -1;
        }
    }
}

void layerToggleAll(int layer) {
    bool all_on = true;

    for (int i = 0; i < package_count[layer]; i++) {
        Package *package = &packages[layer][i];
        if (package->installed == package->changed) {
            all_on = false;
            break;
        }
    }

    setLayersInstall(!all_on, layer);
}

void layerReset(int layer) {
    for (int i = 0; i < package_count[layer]; i++) {
        Package *package = &packages[layer][i];

        if (package->changed) {
            if (package->installed) {
                changes_removals--;
                if (!package->complete)
                    changes_installs++;
            }
            else changes_installs--;

            package->changed = false;
        }
    }
}

void appUninstall(char *basePath, int strlenBase)
{
    char path[1000];
    char pathInstalledApp[1000];

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
                    appUninstall(path, strlenBase);

                if (is_dir(pathInstalledApp))
                    rmdir(pathInstalledApp);
            }
        }
    }

    closedir(dir);
}

bool checkAppInstalled(const char *basePath, int base_len, int level, bool complete)
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
        if (strcmp(dp->d_name, ".") == 0 || strcmp(dp->d_name, "..") == 0)
            continue;

        // Ignore "Roms" folder - it does not indicate installed status
        if (level == 0 && strcmp(dp->d_name, "Roms") == 0)
            continue;

        // Construct new path from our base path
        sprintf(path, "%s/%s", basePath, dp->d_name);

        if (exists(path)) {
            sprintf(pathInstalledApp, "/mnt/SDCARD%s", path + base_len);

            if (!exists(pathInstalledApp))
                is_installed = false;
            else if (dp->d_type == DT_DIR)
                is_installed = checkAppInstalled(path, base_len, level + 1, complete);

            if (!complete && level >= 2 && exists(pathInstalledApp))
                return true;

            if ((complete || level < 2) && !is_installed)
                run = 0;
        }
    }

    closedir(dir);
    return is_installed;
}

void loadResources()
{
    DIR *dp;
    struct dirent *ep;
    char basePath[1000];

    for (int nT = 0; nT < 3; nT++) {
        char data_path[200];
        package_count[nT] = 0;

        switch(nT) {
            case 0: sprintf(data_path, "%s", PACKAGE_LAYER_1); break;
            case 1: sprintf(data_path, "%s", PACKAGE_LAYER_2); break;
            case 2: sprintf(data_path, "%s", PACKAGE_LAYER_3); break;
            default: break;
        }

        if (!exists(data_path) || (dp = opendir(data_path)) == NULL)
            continue;

        while ((ep = readdir(dp)) && package_count[nT] < NUMBER_OF_LAYERS) {
            char cShort[MAX_LAYER_NAME_SIZE];
            strcpy(cShort, ep->d_name);

            const char *file_name = ep->d_name;            
            if (file_name[0] != '.') {
                // Installation check
                sprintf(basePath,"%s/%s", data_path, file_name);

                bool is_installed = checkAppInstalled(basePath, strlen(basePath), 0, false);
                bool is_complete = is_installed ? checkAppInstalled(basePath, strlen(basePath), 0, true) : false;

                Package package = {
                    .installed = is_installed,
                    .changed = false,
                    .complete = is_complete
                };

                if (is_installed) {
                    package_installed_count[nT]++;

                    if (!is_complete)
                        changes_installs++;
                }
                
                strcpy(package.name, file_name);

                packages[nT][package_count[nT]] = package;
                package_count[nT]++;
            }

        }
        
        closedir(dp);
    }

    Package temp;

    // Sort function
    for (int nT = 0; nT < 3; nT++) {
        bool found = true;

        while (found) {
            found = false;

            for (int i = 0; i < package_count[nT] - 1; i++) {
                Package *package_a = &packages[nT][i];
                Package *package_b = &packages[nT][i + 1];

                if (strcmp(package_a->name, package_b->name) > 0) {
                    temp = *package_a;
                    *package_a = *package_b;
                    *package_b = temp;
                    found = true;
                }
            }
        }
    }
}

SDL_Surface* createLabelSurface(Package *package)
{
    SDL_Surface *textbox = SDL_CreateRGBSurface(0, 593, 49, 32,
        0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000); /* important */
    SDL_FillRect(textbox, NULL, 0x000000FF);

    char label_text[STR_MAX], parens[STR_MAX] = "";
    strncpy(label_text, package->name, STR_MAX - 1);

    if (strchr(package->name, '(') != NULL) {
        parens[0] = '(';
        strncat(parens, str_split(label_text, "("), STR_MAX - 2);
    }

    SDL_Surface *label_surface = TTF_RenderUTF8_Blended(font25, label_text, color_white);
    SDL_SetAlpha(label_surface, 0, 0); /* important */
    SDL_Rect label_pos = {0, 0};
    SDL_BlitSurface(label_surface, NULL, textbox, &label_pos);
    
    if (package->installed && !package->complete) strcat(parens, " [!]");
    else if (package->changed) strcat(parens, "*");

    if (strlen(parens) > 0) {
        SDL_Surface *parens_surface = TTF_RenderUTF8_Blended(font25, parens, color_white);
        SDL_SetAlpha(parens_surface, 0, 0); /* important */
        surfaceSetAlpha(parens_surface, 120);
        SDL_Rect parens_pos = {label_surface->w, 0};
        SDL_BlitSurface(parens_surface, NULL, textbox, &parens_pos);
        SDL_FreeSurface(parens_surface);
    }

    SDL_FreeSurface(label_surface);

    return textbox;
}

void displayLayersNames(){
    SDL_Rect rectResName = {35, 92, 80, 20};
    SDL_Surface *surfaceResName;
    for (int i = 0 ; i < 7 ; i++){
        if ((i + nListPosition) < package_count[nTab]) {
            Package *package = &packages[nTab][i + nListPosition];
            surfaceResName = createLabelSurface(package);
            rectResName.y = 92 + i * 47;
            SDL_BlitSurface(surfaceResName, NULL, screen, &rectResName);
            SDL_FreeSurface(surfaceResName);
        }
    }
}

void displayLayersInstall(){
    SDL_Rect rectInstall = {600 - surfaceCheck->w, 96};

    for (int i = 0 ; i < 7 ; i++) {
        if ((i + nListPosition) < package_count[nTab]) {
            Package *package = &packages[nTab][i + nListPosition];
            rectInstall.y = 108 - surfaceCheck->h / 2 + i * 47;
            if (package->installed != package->changed)
                SDL_BlitSurface(surfaceCheck, NULL, screen, &rectInstall);
            else
                SDL_BlitSurface(surfaceCross, NULL, screen, &rectInstall);
        }
    }
}

void showScroller()
{
    int shiftY = 0;
    if (package_count[nTab] - 7 > 0)
        shiftY = (int)(nListPosition * 311 / (package_count[nTab] - 7));
    SDL_Rect rectSroller = { 608, 86 + shiftY, 16, 16};
    SDL_BlitSurface(surfaceScroller, NULL, screen, &rectSroller);
}

bool confirmDoNothing(KeyState *keystate)
{
    bool quit = false;
    SDL_Surface* image = IMG_Load("res/confirmDoNothing.png");

    SDL_BlitSurface(image, NULL, screen, NULL);
    SDL_BlitSurface(screen, NULL, video, NULL);
    SDL_Flip(video);

    bool confirm = false;

    while (!quit) {
        if (updateKeystate(keystate, &quit, true, NULL)) {
            if (keystate[SW_BTN_A] == PRESSED)
                confirm = true;
            else if (keystate[SW_BTN_A] == RELEASED && confirm)
                quit = true;
            else if (keystate[SW_BTN_B] == PRESSED)
                quit = true;
        }
    }

    SDL_FreeSurface(image);

    return confirm;
}

int main(int argc, char *argv[])
{
    bool show_confirm = false;
    bool reapply_all = false;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--confirm") == 0)
            show_confirm = true;
        else if (strcmp(argv[i], "--reapply") == 0)
            reapply_all = true;
        else {
            printf("unknown argument: %s\n", argv[i]);
            return EXIT_FAILURE;
        }
    }

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
	surfaceCheck = IMG_Load("/mnt/SDCARD/.tmp_update/res/toggle-on.png");
	surfaceCross = IMG_Load("/mnt/SDCARD/.tmp_update/res/toggle-off.png");

    font25 = TTF_OpenFont("/customer/app/Exo-2-Bold-Italic.ttf", 25);
    font35 = TTF_OpenFont("/customer/app/Exo-2-Bold-Italic.ttf", 35);

    SDL_Surface *loadingScreen = IMG_Load("res/loading.png");
    SDL_BlitSurface(loadingScreen, NULL, screen, NULL);
    SDL_BlitSurface(screen, NULL, video, NULL);
    SDL_Flip(video);
    SDL_FreeSurface(loadingScreen);

    loadResources();

    SDL_Rect rectSelection = {14, 84, 593, 49};
    SDL_Rect rectTabSelection = {14, 59, 199, 26};

    bool quit = false;
    bool state_changed = true;
    KeyState keystate[320] = {0};

    bool apply_changes = false;

    while (!quit) {
        if (updateKeystate(keystate, &quit, true, NULL)) {
            if (keystate[SW_BTN_RIGHT] >= PRESSED) {
                if (nTab < 2) {
                    nTab++;
                    nSelection = 0;
                    nListPosition = 0;
                    state_changed = true;
                }
            }
            if (keystate[SW_BTN_LEFT] >= PRESSED) {
                if (nTab > 0) {
                    nTab--;
                    nSelection = 0;
                    nListPosition = 0;
                    state_changed = true;
                }
            }

            if (keystate[SW_BTN_R1] >= PRESSED && package_count[nTab] > 0) {
                if ((nListPosition + 14) <package_count[nTab]){
                    nListPosition += 7;
                }
                else if ((nListPosition + 7) <package_count[nTab]){
                    nListPosition = package_count[nTab] - 7;
                    nSelection = 6 ;
                }
                state_changed = true;
            }

            if (keystate[SW_BTN_L1] >= PRESSED && package_count[nTab] > 0) {
                if ((nListPosition - 7) > 0) {
                    nListPosition -= 7;
                }
                else {
                    nListPosition = 0;
                    nSelection = 0;

                }
                state_changed = true;
            }

            if (keystate[SW_BTN_DOWN] >= PRESSED && package_count[nTab] > 0) {
                if (nSelection < 6){
                    nSelection ++;
                }
                else if ((nSelection+nListPosition) < package_count[nTab]-1){
                    nListPosition++;
                }
                state_changed = true;
            }
            if (keystate[SW_BTN_UP] >= PRESSED && package_count[nTab] > 0) {
                if (nSelection > 0){
                    nSelection --;
                }
                else if (nListPosition > 0){
                    nListPosition--;
                }
                state_changed = true;
            }

            if (keystate[SW_BTN_B] == PRESSED || keystate[SW_BTN_START] == PRESSED) {
                if (keystate[SW_BTN_START] == PRESSED)
                    apply_changes = true;

                if (show_confirm) {
                    if (apply_changes) {
                        if (changesTotal() > 0 || (reapply_all && package_installed_count[0] > 0) || confirmDoNothing(keystate))
                            quit = true;
                    }
                    else if (confirmDoNothing(keystate))
                        quit = true;
                }
                else
                    quit = true;
                state_changed = true;
            }

            if (keystate[SW_BTN_A] == PRESSED && package_count[nTab] > 0) {
                int pos = nListPosition + nSelection;
                if (pos < package_count[nTab]) {
                    Package *package = &packages[nTab][pos];
                    package->changed = !package->changed;

                    if (package->installed) {
                        changes_removals += package->changed ? 1 : -1;
                        if (!package->complete)
                            changes_installs += package->changed ? -1 : 1;
                    }
                    else changes_installs += package->changed ? 1 : -1;
                    state_changed = true;
                }
            }

            if (keystate[SW_BTN_X] == PRESSED && package_count[nTab] > 0) {
                layerToggleAll(nTab);
                state_changed = true;
            }

            if (keystate[SW_BTN_Y] == PRESSED && package_count[nTab] > 0) {
                layerReset(nTab);
                state_changed = true;
            }
        }

        if (quit)
            break;

        if (state_changed) {
            rectSelection.y = 83 + nSelection * 47;
            rectTabSelection.x = 14 + (199 * nTab);

            SDL_BlitSurface(surfaceBackground, NULL, screen, NULL);
            SDL_BlitSurface(surfacesTabSelection, NULL, screen, &rectTabSelection);
            SDL_BlitSurface(surfaceTableau, NULL, screen, NULL);
            SDL_BlitSurface(surfaceSelection, NULL, screen, &rectSelection);

            if (package_count[nTab] > 0){
                displayLayersNames();
                showScroller();
                displayLayersInstall();
            }

            if (changesTotal() > 0) {
                char status_str[STR_MAX] = "";
                if (changes_installs > 0)
                    sprintf(status_str, "+%d", changes_installs);
                if (changes_removals > 0) {
                    int len = strlen(status_str);
                    if (len > 0) {
                        strcpy(status_str + len, "  ");
                        len += 2;
                    }
                    sprintf(status_str + len, " −%d", changes_removals);
                }
                SDL_Surface *status = TTF_RenderUTF8_Blended(font25, status_str, color_white);
                SDL_Rect status_rect = {620 - status->w, 30 - status->h / 2};
                SDL_BlitSurface(status, NULL, screen, &status_rect);
                SDL_FreeSurface(status);
            }

            SDL_BlitSurface(screen, NULL, video, NULL);
            SDL_Flip(video);

            state_changed = false;
        }
    }

    if (apply_changes) {
        // installation
        char data_path[STR_MAX];
        char cmd[STR_MAX * 2 + 50];

        SDL_Surface* surfaceBackground = IMG_Load("/mnt/SDCARD/.tmp_update/res/waitingBG.png");
        SDL_Surface* surfaceMessage;

        for (int nT = 0 ; nT < 3 ; nT ++){
            switch(nT) {
                case 0: sprintf(data_path, "%s", PACKAGE_LAYER_1); break;
                case 1: sprintf(data_path, "%s", PACKAGE_LAYER_2); break;
                case 2: sprintf(data_path, "%s", PACKAGE_LAYER_3); break;
                default: break;
            }

            if (!exists(data_path))
                continue;

            SDL_Rect rectMessage = {10, 420 , 603, 48};

            for (int nLayer = 0; nLayer < package_count[nT]; nLayer++) {
                Package *package = &packages[nT][nLayer];

                bool should_apply = (package->installed && !package->complete) || package->changed;
                bool should_install = package->installed != package->changed || (package->installed && !package->complete && !package->changed);

                if (!reapply_all && !should_apply)
                    continue;

                if (should_install) {
                    printf_debug("Installing %s...\n", package->name);
                    SDL_BlitSurface(surfaceBackground, NULL, screen, NULL);

                    surfaceMessage = TTF_RenderUTF8_Blended(font35, package->name, color_white);
                    SDL_BlitSurface(surfaceMessage, NULL, screen, &rectMessage);
                    SDL_FreeSurface(surfaceMessage);

                    SDL_BlitSurface(screen, NULL, video, NULL);
                    SDL_Flip(video);

                    sprintf(cmd, "./install.sh \"%s\" \"%s\"", data_path, package->name);
                    system(cmd);
                }
                else if (package->installed) {
                    printf_debug("Removing %s...\n", package->name);
                    // app uninstallation
                    char pathAppUninstall[1000];
                    sprintf(pathAppUninstall, "%s/%s", data_path, package->name);
                    appUninstall(pathAppUninstall, strlen(pathAppUninstall));
                }
            }

        }
    }

    msleep(200);

    TTF_CloseFont(font25);
    TTF_CloseFont(font35);
    TTF_Quit();
    SDL_FreeSurface(surfaceCheck);
    SDL_FreeSurface(surfaceCross);
    SDL_FreeSurface(surfaceBackground);
    SDL_FreeSurface(surfaceTableau);
    SDL_FreeSurface(surfaceSelection);
    SDL_FreeSurface(surfaceScroller);
    SDL_FreeSurface(surfacesTabSelection);
    SDL_Quit();

    return EXIT_SUCCESS;
}
