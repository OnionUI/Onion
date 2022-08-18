#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdint.h>
#include <string.h>
#include <linux/fb.h>
#include <linux/input.h>
#include <fcntl.h>
#include <stdbool.h>
#include <libgen.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <signal.h>
#include <SDL/SDL.h>
#include <SDL/SDL_image.h>
#include <SDL/SDL_ttf.h>

// #include "SDL/SDL_rotozoom.h"
#include "png/png.h"

#include "utils/utils.h"
#include "utils/json.h"
#include "utils/hash.h"
#include "utils/msleep.h"
#include "utils/log.h"
#include "utils/keystate.h"
#include "system/keymap_sw.h"

#define MAXHISTORY 50
#define MAXHROMNAMESIZE 250
#define MAXHROMPATHSIZE 150

#define ROM_SCREENS_DIR "/mnt/SDCARD/Saves/CurrentProfile/romScreens"
#define ROM_DB_PATH "/mnt/SDCARD/Saves/CurrentProfile/saves/playActivity.db"
#define HISTORY_PATH "/mnt/SDCARD/Saves/CurrentProfile/lists/content_history.lpl"
#define SYSTEM_CONFIG "/appconfigs/system.json"

#define MAXFILENAMESIZE 250
#define MAXSYSPATHSIZE 80

#define MAXHRACOMMAND 500
#define LOWBATRUMBLE 10

// Max number of records in the DB
#define MAXVALUES 1000

#define GPIO_DIR1 "/sys/class/gpio/"
#define GPIO_DIR2 "/sys/devices/gpiochip0/gpio/"

static bool quit = false;
static void sigHandler(int sig)
{
    switch (sig) {
        case SIGINT:
        case SIGTERM:
            quit = true;
            break;
        default: break;
    }
}

char sTotalTimePlayed[50];

// Game history list
typedef struct
{
    uint32_t hash;
    char name[MAXHROMNAMESIZE];
    char RACommand[STR_MAX * 2 + 80] ;
    char totalTime[30];
    int jsonIndex;
} Game_s;
static Game_s game_list[MAXHISTORY];

static int game_list_len = 0;
static int current_game = 0;

static cJSON* json_root = NULL;
static cJSON* items = NULL;

// Play activity database
struct structPlayActivity
{
    char name[100];
    int playTime;
}
rom_list[MAXVALUES];
int rom_list_len = 0;
int bDisplayBoxArt = 0;

int getMiyooLum(void)
{
    cJSON* request_json = json_load(SYSTEM_CONFIG);
    int brightness = 10;
    json_getInt(request_json, "brightness", &brightness);
    cJSON_free(request_json);
    return brightness;
}

void setMiyooLum(int nLum)
{
    cJSON* request_json = json_load(SYSTEM_CONFIG);
    cJSON* itemBrightness = cJSON_GetObjectItem(request_json, "brightness");

    cJSON_SetNumberValue(itemBrightness, nLum);

    json_save(request_json, SYSTEM_CONFIG);
    cJSON_free(request_json);
}

void SetRawBrightness(int val) // val = 0-100
{
    FILE *fp;
    file_put(fp, "/sys/class/pwm/pwmchip0/pwm0/duty_cycle", "%d", val);
}

void SetBrightness(int value) // value = 0-10
{
    SetRawBrightness(value==0 ? 6 : value * 10);
    setMiyooLum(value);
}

int readRomDB()
{
    FILE *fp;
    int totalTimePlayed = 0 ;
    // Check to avoid corruption
    if (exists(ROM_DB_PATH)) {
        FILE * file = fopen(ROM_DB_PATH, "rb");
        fread(rom_list, sizeof(rom_list), 1, file);
        rom_list_len = 0;

        for (int i=0; i<MAXVALUES; i++){
            if (strlen(rom_list[i].name) == 0)
                break;
            totalTimePlayed += rom_list[rom_list_len].playTime;
            rom_list_len++;
        }

        int h = totalTimePlayed / 3600;
        sprintf(sTotalTimePlayed, "%dh", h);
        fclose(file);
    }
    return 1;
}

int searchRomDB(char* romName){
    int position = -1;

    for (int i = 0 ; i < rom_list_len ; i++) {
        if (strcmp(rom_list[i].name,romName) == 0){
            position = i;
            break;
        }
    }
    return position;
}


void readHistory()
{
    // History extraction
    game_list_len = 0;

    if (!exists(HISTORY_PATH))
        return;

    char path[STR_MAX], core_path[STR_MAX];

    json_root = json_load(HISTORY_PATH);
    items = cJSON_GetObjectItem(json_root, "items");

    for (int nbGame = 0; nbGame < MAXHISTORY; nbGame++) {
        cJSON *subitem = cJSON_GetArrayItem(items, nbGame);

        if (subitem == NULL)
            continue;

        if (!json_getString(subitem, "path", path) || !json_getString(subitem, "core_path", core_path))
            continue;

        if (!exists(core_path) || !exists(path))
            continue;

        Game_s *game = &game_list[game_list_len];
        sprintf(game->RACommand, "LD_PRELOAD=/mnt/SDCARD/miyoo/lib/libpadsp.so ./retroarch -v -L \"%s\" \"%s\"", core_path, path);
        strcpy(game->name, basename(path));
        game->hash = FNV1A_Pippip_Yurii(path, strlen(path));
        game->jsonIndex = nbGame;

        // Rom name
        int nTimePosition = searchRomDB(game->name);

        if (nTimePosition >= 0){
            int nTime = rom_list[nTimePosition].playTime;
            if (nTime >= 0) {
                int h = nTime / 3600;
                int m = (nTime - 3600 * h) / 60;
                sprintf(game->totalTime, "%d:%02d / %s", h, m, sTotalTimePlayed);
            }
        }

        game_list_len++;
    }
}

void removeCurrentItem() {
    if (items != NULL)
        cJSON_DeleteItemFromArray(items, game_list[current_game].jsonIndex);
    json_save(json_root, HISTORY_PATH);
}

SDL_Surface* loadRomscreen(Game_s *game)
{
    char currPicture[STR_MAX];
    sprintf(currPicture, ROM_SCREENS_DIR "/%"PRIu32".png", game->hash);
    if (!exists(currPicture))
        sprintf(currPicture, ROM_SCREENS_DIR "/%s.png", file_removeExtension(game->name));
    if (exists(currPicture))
        return IMG_Load(currPicture);
    return NULL;
}

int checkQuitAction(void)
{
    FILE *fp;
    char prev_state[10];
    file_get(fp, "/tmp/prev_state", "%s", prev_state);
    if (strncmp(prev_state, "mainui", 6) == 0)
        return 1;
    return 0;
}

int main(void)
{
    print_debug("Debug logging enabled");

    signal(SIGINT, sigHandler);
    signal(SIGTERM, sigHandler);

    SDL_Color    color = {255,255,255,0};
    TTF_Font*    font;
    SDL_Surface*    imagePlay;
    SDL_Surface*    imageGameName;
    SDL_Surface*     surfaceArrowLeft = IMG_Load("res/arrowLeft.png");
    SDL_Surface*     surfaceArrowRight = IMG_Load("res/arrowRight.png");

    SDL_Rect    rectBatt = { 566, -1, 113, 29};
    SDL_Rect    rectLum = { 106, 59, 40, 369};
    SDL_Rect    rectWhiteLigne = { 0, 0, 1280, 39};
    SDL_Rect    rectMenuBar = {0,0,640,480};
    SDL_Rect    rectTime = { 263, -1, 150, 39};
    SDL_Rect    rectBatteryIcon = {541,6,13,27};
    SDL_Rect    rectFooterHelp = { 420, 441, 220, 39};
    SDL_Rect    rectGameName = { 9, 439, 640, 39};

    SDL_Rect     rectArrowLeft = { 6, 217, 28, 32};
    SDL_Rect     rectArrowRight = { 604, 217, 28, 32};

    readRomDB();
    readHistory();

    print_debug("Read ROM DB and history");

    int nExitToMiyoo = 0;

    SDL_Surface* imageMenuBar = IMG_Load("res/menuBar.png");
    SDL_Surface* imageTuto;

    int bShowBoot = 0;

    int input_fd = open("/dev/input/event0", O_RDONLY);
    struct input_event ev;

    int firstPass = 1;

    SDL_Init(SDL_INIT_VIDEO);
    SDL_ShowCursor(SDL_DISABLE);
    SDL_EnableKeyRepeat(300, 50);
    TTF_Init();

    SDL_Surface *video = SDL_SetVideoMode(640, 480, 32, SDL_HWSURFACE);
    SDL_Surface *screen = SDL_CreateRGBSurface(SDL_HWSURFACE, 640, 480, 32, 0, 0, 0, 0);

    font = TTF_OpenFont("/customer/app/Exo-2-Bold-Italic.ttf", 30);

    print_debug("Loading images");

    SDL_Surface* imageBackgroundDefault = IMG_Load("res/bootScreen.png");
    SDL_Surface* imageBackgroundLowBat = IMG_Load("res/lowBat.png");
    SDL_Surface* imageBackgroundNoGame= IMG_Load("res/noGame.png");
    SDL_Surface* imageRemoveDialog= IMG_Load("res/removeDialog.png");
    SDL_Surface* imageBackgroundGame = NULL;
    SDL_Surface* imageFooterHelp = IMG_Load("res/footerHelp.png");

    uint32_t acc_ticks = 0,
			 last_ticks = SDL_GetTicks(),
			 time_step = 1000 / 60;

    bool changed = true;
    KeyState keystate[320] = {(KeyState)0};
    bool menu_pressed = false;

	while (!quit) {
		uint32_t ticks = SDL_GetTicks();
		acc_ticks += ticks - last_ticks;
		last_ticks = ticks;

        int bBrightChange = 0;
        if (firstPass > 0) {
            // First screen draw
            firstPass ++;

            // Needs another refresh
            if (firstPass > 2)
                firstPass = 0;

            print_debug("loading rom screen");
            imageBackgroundGame = loadRomscreen(&game_list[current_game]);

            if (game_list_len > 1)
                SDL_BlitSurface(surfaceArrowRight, NULL, screen, &rectArrowRight);
        }
        else if (updateKeystate(keystate, &quit, true)) {
			if (keystate[SW_BTN_MENU] == PRESSED)
                menu_pressed = true;

            if (keystate[SW_BTN_MENU] == RELEASED && menu_pressed) {
                quit = true;
                break;
            }

            if (keystate[SW_BTN_RIGHT] >= PRESSED) {
                if (current_game < game_list_len - 1) {
                    current_game++;

                    if (imageBackgroundGame != NULL)
                        SDL_FreeSurface(imageBackgroundGame);
                    imageBackgroundGame = loadRomscreen(&game_list[current_game]);
                }
            }

            if (keystate[SW_BTN_LEFT] >= PRESSED) {
                if (current_game > 0) {
                    current_game--;
                    if (imageBackgroundGame != NULL)
                        SDL_FreeSurface(imageBackgroundGame);
                    imageBackgroundGame = loadRomscreen(&game_list[current_game]);
                }
            }

            // if (y_pressed) {
            //     int fd = creat(".menuActivity", 777);
            //     close(fd);
            //     break;
            // }

            if (keystate[SW_BTN_START] == PRESSED) {
                nExitToMiyoo = 1;
                break;
            }

            if (keystate[SW_BTN_A] == PRESSED)
                break;
            
            if (keystate[SW_BTN_B] == PRESSED) {
                nExitToMiyoo = checkQuitAction();
                quit = true;
                break;
            }

            if (keystate[SW_BTN_UP] >= PRESSED){
                // Change brightness
                bBrightChange = 1;
                int brightVal = getMiyooLum();
                if (brightVal < 10) {
                    brightVal++;
                    SetBrightness(brightVal);
                }
            }

            if (keystate[SW_BTN_DOWN] >= PRESSED){
                // Change brightness
                bBrightChange = 1;
                    int brightVal = getMiyooLum();
                    if (brightVal > 0) {
                        brightVal--;
                        SetBrightness(brightVal);
                    }
            }
        }

        long lSize;
        FILE *fp;

        if (game_list_len == 0) {
            SDL_BlitSurface(imageBackgroundNoGame, NULL, screen, NULL);
        }
        else /*if (nBat > LOWBATRUMBLE)*/ {
            if (imageBackgroundGame != NULL)
                SDL_BlitSurface(imageBackgroundGame, NULL, screen, NULL);
            else
                SDL_BlitSurface(imageBackgroundDefault, NULL, screen, NULL);
        }
        // else {
        //     SDL_BlitSurface(imageBackgroundLowBat, NULL, screen, NULL);
        // }

        if (current_game > 0) {
            SDL_BlitSurface(surfaceArrowLeft, NULL, screen, &rectArrowLeft);
        }

        if (current_game < game_list_len - 1) {
            SDL_BlitSurface(surfaceArrowRight, NULL, screen, &rectArrowRight);
        }

        SDL_BlitSurface(imageMenuBar, NULL, screen, &rectMenuBar);

        if (game_list_len > 0) {
            imageGameName = TTF_RenderUTF8_Blended(font, file_removeExtension(game_list[current_game].name), color);
            SDL_BlitSurface(imageGameName, NULL, screen, &rectGameName);
        }

        SDL_BlitSurface(imageFooterHelp, NULL, screen, &rectFooterHelp);

        if (bBrightChange == 1) {
            // Display luminosity slider
            int nLum = getMiyooLum();
            char imageLumName[STR_MAX];
            sprintf(imageLumName, "res/lum%d.png", nLum);
            SDL_Surface* imageLum = IMG_Load(imageLumName);

            SDL_BlitSurface(imageLum, NULL, screen, &rectLum);
            SDL_FreeSurface(imageLum);
        }

        imagePlay = TTF_RenderUTF8_Blended(font, game_list[current_game].totalTime, color);
        SDL_BlitSurface(imagePlay, NULL, screen, &rectTime);

        if (keystate[SW_BTN_X] == PRESSED) {
            if (game_list_len != 0) {
                SDL_BlitSurface(imageRemoveDialog, NULL, screen, NULL);
                SDL_BlitSurface(screen, NULL, video, NULL);
                SDL_Flip(video);

                while (!quit) {
                    if (updateKeystate(keystate, &quit, true)) {
                        if (keystate[SW_BTN_A] == PRESSED) {
                            removeCurrentItem();
                            readHistory();
                            if (current_game > 0)
                                current_game--;
                            firstPass = 1;
                            break;
                        }
                        if (keystate[SW_BTN_B] == PRESSED) {
                            firstPass = 1;
                            break;
                        }
                    }
                }
            }
        }

        SDL_BlitSurface(screen, NULL, video, NULL);
        SDL_Flip(video);
    }

    screen = SDL_CreateRGBSurface(SDL_HWSURFACE, 640,480, 32, 0,0,0,0);

    remove("/mnt/SDCARD/.tmp_update/.runGameSwitcher");
    remove("/mnt/SDCARD/.tmp_update/cmd_to_run.sh");
    if (nExitToMiyoo != 1){
        print_debug("Resuming game");
        FILE *file = fopen("/mnt/SDCARD/.tmp_update/cmd_to_run.sh", "w");
        fputs(game_list[current_game].RACommand, file);
        fclose(file);
    }
    else {
        print_debug("Exiting to menu");
    }

    #ifndef PLATFORM_MIYOOMINI
    msleep(200);
    #endif

    cJSON_free(json_root);

    SDL_BlitSurface(screen, NULL, video, NULL);
    SDL_Flip(video);

    SDL_FreeSurface(screen);
    SDL_FreeSurface(video);

    SDL_FreeSurface(imageBackgroundDefault);
    SDL_FreeSurface(imageBackgroundLowBat);
    //SDL_FreeSurface(imageBackgroundGame);

    SDL_FreeSurface(imageMenuBar);

    SDL_FreeSurface(imagePlay);
    SDL_FreeSurface(surfaceArrowLeft);
    SDL_FreeSurface(surfaceArrowRight);

    SDL_Quit();

    return EXIT_SUCCESS;
}
