#include "SDL/SDL_rotozoom.h"
#include <SDL/SDL.h>
#include <SDL/SDL_image.h>
#include <SDL/SDL_ttf.h>
#include <dirent.h>
#include <fcntl.h>
#include <libgen.h>
#include <linux/fb.h>
#include <linux/input.h>
#include <pthread.h>
#include <signal.h>
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
#include "system/keymap_sw.h"
#include "system/lang.h"
#include "system/settings.h"
#include "theme/background.h"
#include "theme/sound.h"
#include "theme/theme.h"
#include "utils/config.h"
#include "utils/file.h"
#include "utils/hash.h"
#include "utils/json.h"
#include "utils/keystate.h"
#include "utils/log.h"
#include "utils/msleep.h"
#include "utils/sdl_init.h"
#include "utils/str.h"
#include "utils/surfaceSetAlpha.h"

#include "../playActivity/cacheDB.h"
#include "../playActivity/playActivityDB.h"

#define MAXHISTORY 100
#define MAXHROMNAMESIZE 250
#define MAXHROMPATHSIZE 150
#define MAXSTATES 10
#define MAXSTATESORTFAILS 5

#define RA_SAVESTATES_DIR "/mnt/SDCARD/Saves/CurrentProfile/states"
#define ROM_SCREENS_DIR "/mnt/SDCARD/Saves/CurrentProfile/romScreens"
#define HISTORY_PATH "/mnt/SDCARD/Saves/CurrentProfile/lists/content_history.lpl"
#define CACHE_PATH "/mnt/SDCARD/App/GameSwitcherMod/gameSwitcher.json"

#define MAXFILENAMESIZE 250
#define MAXSYSPATHSIZE 80

#define MAXHRACOMMAND 500
#define LOWBATRUMBLE 10

// Max number of records in the DB
#define MAXVALUES 1000

#define VIEW_NORMAL 0
#define VIEW_MINIMAL 1
#define VIEW_FULLSCREEN -1

static bool quit = false;
static bool exit_to_menu = false;

//static pthread_t thread_pt;

static void sigHandler(int sig)
{
    switch (sig) {
    case SIGINT:
    case SIGTERM:
        exit_to_menu = true;
        quit = true;
        break;
    default:
        break;
    }
}

static char sTotalTimePlayed[50] = "";

// Game history list
typedef struct {
    uint32_t hash;
    char name[MAXHROMNAMESIZE];
    char rom_name[MAXHROMNAMESIZE];
    char shortname[STR_MAX];
    char core[STR_MAX];
    char core_name[STR_MAX];
    char RACommand[STR_MAX * 2 + 80];
    char totalTime[100];
    char savingStatesPath[STR_MAX];
    bool currentAutoStateSaved;
    bool savingStatesSorted;
    int savingStatesNum;
    int jsonIndex;
    int is_duplicate;
    SDL_Surface *romScreen;
    char romScreenPath[STR_MAX * 2];
    char path[PATH_MAX];
} Game_s;
static Game_s game_list[MAXHISTORY];

static int game_list_len = 0;
static int current_game = 0;
static int current_state = 0; // 0 - auto, 1 ... MAXSTATES - manual saved states

static SDL_Surface *surfaceGameName = NULL;
static int gameNameScrollX = 0;
static int gameNameScrollSpeed = 10;
static int gameNameScrollStart = 20;
static int gameNameScrollEnd = 20;

static cJSON *json_root = NULL;
static cJSON *json_items = NULL;
static cJSON *jsonCache_root = NULL;

//static bool __initial_romscreens_loaded = false;

const bool __getSavingStatesPathCached(const char *core_name, char *saving_states_path)
{
    if (jsonCache_root == NULL) {
        jsonCache_root = json_load(CACHE_PATH);
    }

    return json_getString(jsonCache_root, core_name, saving_states_path);
}

const bool __cacheSavingStatesPath(const char *core_name, const char *states_path)
{
    if (!exists(CACHE_PATH)) {
        FILE *fp;

        if ((fp = fopen(CACHE_PATH, "w+")) == NULL)
            return false;

        fprintf(fp, "{\n");
        fprintf(fp, "}");

        fflush(fp);
        fsync(fileno(fp));
        fclose(fp);
    }

    if (jsonCache_root == NULL) {
        jsonCache_root = json_load(CACHE_PATH);
    }

    const bool saveResult = json_forceSetString(jsonCache_root, core_name, states_path);

    if (saveResult)
        json_save(jsonCache_root, CACHE_PATH);

    return saveResult;
}

// Find the folder name for saving states for the selected emulator
const char *getSavingStatesPath(const char *core_name)
{
    char savingStatesPath[STR_MAX * 3];

    if (__getSavingStatesPathCached(core_name, savingStatesPath))
        return strdup(savingStatesPath);

    char command[STR_MAX * 5];
    char emuMasked[STR_MAX];

    FILE *find;

    // Extract the emulator name from the brackets, remove starting 'Beetle' word for Supafaust, replace ' ' and '-' chars by mask symbol
    sprintf(command, "echo \"%s\" | awk -F '[()]' '{print $2}' | sed -e 's/^.*Beetle //' | sed -r 's/[- ]+/*/g'", core_name);
    find = popen(command, "r");
    if (find == NULL) {
        perror("Error executing awk, sed command");
        exit(EXIT_FAILURE);
    }

    // Read the output of the previous commands and extract masked emulator name
    while (fgets(emuMasked, sizeof(emuMasked), find) != NULL) {
        emuMasked[strcspn(emuMasked, "\n")] = '\0'; // Remove trailing newline character
    }

    pclose(find);

    // Use the 'find' command to search for emulator subdirectories of the RA_SAVESTATES_DIR path
    sprintf(command, "find \"%s\" -iname \"%s\" -and -type d -prune", RA_SAVESTATES_DIR, emuMasked);
    find = popen(command, "r");
    if (find == NULL) {
        perror("Error executing find command");
        exit(EXIT_FAILURE);
    }

    // Read the output of the find command and extract matching folder names
    while (fgets(savingStatesPath, sizeof(savingStatesPath), find) != NULL) {
        savingStatesPath[strcspn(savingStatesPath, "\n")] = '\0'; // Remove trailing newline character
    }

    pclose(find);

    __cacheSavingStatesPath(core_name, savingStatesPath);

    return strdup(savingStatesPath);
}

// Perform auto sort of saving states for selected game
void sortSavingStates()
{
    if ((current_game < 0) && (current_game >= game_list_len))
        return;

    Game_s *game = &game_list[current_game];

    if (game->savingStatesSorted)
        return;

    char statePath[STR_MAX * 3];
    char stateScrPath[STR_MAX * 3 + 4];
    int currentStateNum = 0, currentStep = 0;
    int sortingFails = 0;

    while (sortingFails < MAXSTATESORTFAILS) {
        currentStep++;

        // Construct new path for state number i
        sprintf(statePath, "%s/%s.state%i", game->savingStatesPath, game->rom_name, currentStep);
        sprintf(stateScrPath, "%s.png", statePath);

        if (exists(statePath)) {
            currentStateNum++;

            // state files renaming required
            if (currentStateNum != currentStep) {
                char newStatePath[STR_MAX * 3];
                char newStateScrPath[STR_MAX * 3 + 4];

                sprintf(newStatePath, "%s/%s.state%i", game->savingStatesPath, game->rom_name, currentStateNum);
                rename(statePath, newStatePath);

                if (exists(stateScrPath)) {
                    sprintf(newStateScrPath, "%s.png", newStatePath);
                    rename(stateScrPath, newStateScrPath);
                }
            }
        }
        else {
            sortingFails++;
        }
    }

    game->savingStatesNum = currentStateNum;
}

// Duplicate current auto state as last saving state
void saveCurrentAutoState()
{
    Game_s *game = &game_list[current_game];

    if (game->currentAutoStateSaved ||
        (current_state != 0) ||
        (game->savingStatesNum < 0) ||
        (game->savingStatesNum >= MAXSTATES))
        return;

    char statePath[STR_MAX * 3];
    char stateScrPath[STR_MAX * 3 + 4];

    sprintf(statePath, "%s/%s.state.auto", game->savingStatesPath, game->rom_name);
    sprintf(stateScrPath, "%s.png", statePath);

    if (!exists(statePath))
        return;

    char newStatePath[STR_MAX * 3];
    char newStateScrPath[STR_MAX * 3 + 4];

    game->savingStatesNum++;
    game->currentAutoStateSaved = true;

    sprintf(newStatePath, "%s/%s.state%i", game->savingStatesPath, game->rom_name, game->savingStatesNum);
    sprintf(newStateScrPath, "%s.png", newStatePath);

    file_copy(statePath, newStatePath);
    file_copy(stateScrPath, newStateScrPath);

    current_state = game->savingStatesNum;
}

// Copy selected saving state to the auto state
void playCurrentState()
{
    if (current_state <= 0)
        return;

    Game_s *game = &game_list[current_game];

    char statePath[STR_MAX * 3];
    char stateScrPath[STR_MAX * 3 + 4];
    char selStatePath[STR_MAX * 3];
    char selStateScrPath[STR_MAX * 3 + 4];

    sprintf(statePath, "%s/%s.state.auto", game->savingStatesPath, game->rom_name);
    sprintf(stateScrPath, "%s.png", statePath);

    sprintf(selStatePath, "%s/%s.state%i", game->savingStatesPath, game->rom_name, current_state);
    sprintf(selStateScrPath, "%s.png", selStatePath);

    if (!exists(statePath) ||
        !exists(stateScrPath) ||
        !exists(selStatePath) ||
        !exists(selStateScrPath))
        return;

    //remove(statePath);
    //remove(stateScrPath);

    file_copy(selStatePath, statePath);
    file_copy(selStateScrPath, stateScrPath);
}

// Delete selected saving state
void removeCurrentState()
{
    if (current_state <= 0)
        return;

    Game_s *game = &game_list[current_game];

    char selStatePath[STR_MAX * 3];
    char selStateScrPath[STR_MAX * 3 + 4];

    sprintf(selStatePath, "%s/%s.state%i", game->savingStatesPath, game->rom_name, current_state);
    sprintf(selStateScrPath, "%s.png", selStatePath);

    if (exists(selStatePath) && exists(selStateScrPath)) {
        remove(selStatePath);
        remove(selStateScrPath);
        game->savingStatesSorted = false;
    }
}

void unloadRomScreen(int index)
{
    if (index < 0 || index >= game_list_len)
        return;

    Game_s *game = &game_list[index];

    if (game->romScreen != NULL) {
        SDL_FreeSurface(game->romScreen);
        game->romScreen = NULL;
    }
}

void freeRomScreens()
{
    for (int i = 0; i < game_list_len; i++) {
        unloadRomScreen(i);
    }
}

SDL_Surface *loadRomScreen(int index)
{
    if (index < 0 || index >= game_list_len)
        return NULL;

    Game_s *game = &game_list[index];

    unloadRomScreen(index);

    if (game->romScreen == NULL) {
        char stateScrPath[STR_MAX * 3 + 4];

        if (current_state > 0) {
            sprintf(stateScrPath, "%s/%s.state%i.png", game->savingStatesPath, game->rom_name, current_state);
        }
        else {
            sprintf(stateScrPath, "%s/%s.state.auto.png", game->savingStatesPath, game->rom_name);
        }

        if (exists(stateScrPath)) {
            game->romScreen = IMG_Load(stateScrPath);
        }
    }

    /*if (__initial_romscreens_loaded) {
        unloadRomScreen(index + 5);
        if (index > 5) {
            unloadRomScreen(index - 5);
        }
    }*/

    return game->romScreen;
}

/*static void *_loadRomScreensThread(void *_)
{
    for (int i = 0; i < 10 && i < game_list_len; i++) {
        Game_s *game = &game_list[i];

        if (game->romScreen == NULL)
            loadRomScreen(i);
    }

    __initial_romscreens_loaded = true;

    return NULL;
}*/

void getGameName(char *name_out, const char *rom_path)
{
    CacheDBItem *cache_item = cache_db_find(rom_path);
    if (cache_item != NULL) {
        strcpy(name_out, cache_item->name);
        free(cache_item);
    }
    else {
        strcpy(name_out, file_removeExtension(basename(strdup(rom_path))));
    }
}

// History extraction
void readHistory()
{
    game_list_len = 0;
    current_state = 0;

    if (!exists(HISTORY_PATH)) {
        print_debug("History file missing");
        return;
    }

    char rom_path[STR_MAX], core_path[STR_MAX], core_name[STR_MAX];

    if (json_items == NULL) {
        json_root = json_load(HISTORY_PATH);
        json_items = cJSON_GetObjectItem(json_root, "items");
    }

    for (int nbGame = 0; nbGame < MAXHISTORY; nbGame++) {
        cJSON *subitem = cJSON_GetArrayItem(json_items, nbGame);

        if (subitem == NULL)
            break;

        if (!json_getString(subitem, "path", rom_path) ||
            !json_getString(subitem, "core_path", core_path) ||
            !json_getString(subitem, "core_name", core_name))
            continue;

        if (strncmp("/mnt/SDCARD/App", rom_path, 15) == 0)
            continue;

        if (!exists(core_path) || !exists(rom_path))
            continue;

        Game_s *game = &game_list[game_list_len];
        game->hash = FNV1A_Pippip_Yurii(rom_path, strlen(rom_path));
        game->jsonIndex = nbGame;
        game->romScreen = NULL;
        game->is_duplicate = 0;
        game->totalTime[0] = '\0';
        game->savingStatesNum = 0;
        game->currentAutoStateSaved = false;
        game->savingStatesSorted = false;
        sprintf(game->RACommand, "LD_PRELOAD=/mnt/SDCARD/miyoo/lib/libpadsp.so ./retroarch -v -L \"%s\" \"%s\"", core_path, rom_path);
        getGameName(game->name, rom_path);
        strcpy(game->path, rom_path);
        strcpy(game->core, basename(core_path));
        str_split(game->core, "_libretro");
        file_cleanName(game->shortname, game->name);
        strcpy(game->core_name, core_name);
        strcpy(game->savingStatesPath, getSavingStatesPath(core_name));
        strcpy(game->rom_name, file_removeExtension(basename(strdup(rom_path))));

        printf_debug("Game loaded:\n"
                     "\tname: '%s' (%s)\n"
                     "\tcmd: '%s'\n"
                     "\thash: %" PRIu32 "\n"
                     "\tidx: %d\n"
                     "\n",
                     game->name, game->shortname,
                     game->RACommand,
                     game->hash,
                     game->jsonIndex);

        // Check for duplicates
        for (int i = 0; i < game_list_len; i++) {
            Game_s *other = &game_list[i];
            if (other->hash == game->hash) {
                other->is_duplicate += 1;
                game->is_duplicate = other->is_duplicate;
            }
        }

        game_list_len++;
    }

    //pthread_create(&thread_pt, NULL, _loadRomScreensThread, NULL);
}

void removeCurrentItem()
{
    Game_s *game = &game_list[current_game];

    printf_debug("removing: %s\n", game->name);

    unloadRomScreen(current_game);

    if (game->is_duplicate > 0) {
        // Check for duplicates
        for (int i = 0; i < game_list_len; i++) {
            Game_s *other = &game_list[i];
            if (other->hash == game->hash)
                other->is_duplicate -= 1;
        }
    }

    if (json_items != NULL)
        cJSON_DeleteItemFromArray(json_items, game->jsonIndex);

    json_save(json_root, HISTORY_PATH);

    if ((strlen(game->romScreenPath) > 0) && is_file(game->romScreenPath))
        remove(game->romScreenPath);

    // Copy next element value to current element
    for (int i = current_game; i < game_list_len - 1; i++) {
        game_list[i] = game_list[i + 1];
        game_list[i].jsonIndex -= 1;
    }

    game_list_len--;
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
    log_setName("gameSwitcher");
    print_debug("Debug logging enabled");

    signal(SIGINT, sigHandler);
    signal(SIGTERM, sigHandler);

    SDL_InitDefault(true);

    SDL_BlitSurface(theme_background(), NULL, screen, NULL);
    SDL_BlitSurface(screen, NULL, video, NULL);
    SDL_Flip(video);

    readHistory();

    settings_load();
    lang_load();

    SDL_Color color_white = {255, 255, 255};
    SDL_Surface *transparent_bg = SDL_CreateRGBSurface(
        0, 640, 480, 32, 0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000);
    SDL_FillRect(transparent_bg, NULL, 0xBE000000);

    SDL_Surface *arrow_left = resource_getSurface(LEFT_ARROW_WB);
    SDL_Surface *arrow_right = resource_getSurface(RIGHT_ARROW_WB);
    int game_name_padding = arrow_left->w + 20;
    int game_name_max_width = 640 - 2 * game_name_padding;
    SDL_Rect game_name_size = {0, 0};

    int battery_percentage = battery_getPercentage();

    bool changed = true;
    bool current_game_changed = true;

    KeyState keystate[320] = {(KeyState)0};
    bool menu_pressed = false;
    bool combo_key = false;
    bool select_pressed = false;
    bool select_combo_key = false;

    mkdirs("/mnt/SDCARD/.tmp_update/config/gameSwitcher");

    bool view_min = config_flag_get("gameSwitcher/minimal");
    bool show_time = config_flag_get("gameSwitcher/showTime");
    bool show_total = !config_flag_get("gameSwitcher/hideTotal");
    bool show_legend = !config_flag_get("gameSwitcher/hideLegend");
    int view_mode = view_min ? VIEW_MINIMAL : VIEW_NORMAL, view_restore;

    SDLKey changed_key = SDLK_UNKNOWN;
    int button_y_repeat = 0;

    uint32_t acc_ticks = 0, last_ticks = SDL_GetTicks(), time_step = 1000 / 30;

    uint32_t legend_start = last_ticks;
    uint32_t legend_timeout = 5000;

    char header_path[STR_MAX], footer_path[STR_MAX];
    bool use_custom_header = theme_getImagePath(theme()->path, "extra/gs-top-bar-v2", header_path);
    bool use_custom_footer = theme_getImagePath(theme()->path, "extra/gs-bottom-bar-v2", footer_path);
    SDL_Surface *custom_header = use_custom_header ? IMG_Load(header_path) : NULL;
    SDL_Surface *custom_footer = use_custom_footer ? IMG_Load(footer_path) : IMG_Load("/mnt/SDCARD/App/GameSwitcherMod/res/gs-legend.png");

    int header_height = use_custom_header ? custom_header->h : 60;
    if (header_height == 1)
        header_height = 0;
    int footer_height = use_custom_footer ? custom_footer->h : 60;
    if (footer_height == 1)
        footer_height = 0;

    SDL_Surface *current_bg = NULL;

    while (!quit) {
        uint32_t ticks = SDL_GetTicks();
        acc_ticks += ticks - last_ticks;
        last_ticks = ticks;

        if (show_legend && ticks - legend_start > legend_timeout) {
            show_legend = false;
            config_flag_set("gameSwitcher/hideLegend", true);
            changed = true;
        }

        if (updateKeystate(keystate, &quit, true, &changed_key)) {
            if (menu_pressed && changed_key != SW_BTN_MENU)
                combo_key = true;
            if (select_pressed && changed_key != SW_BTN_SELECT)
                select_combo_key = true;

            if (keystate[SW_BTN_MENU] == PRESSED)
                menu_pressed = true;

            if (menu_pressed && keystate[SW_BTN_MENU] == RELEASED) {
                if (!combo_key) {
                    quit = true;
                    break;
                }
                menu_pressed = false;
                combo_key = false;
            }

            if (keystate[SW_BTN_RIGHT] >= PRESSED) {
                if (current_game < game_list_len - 1) {
                    current_game++;
                    current_game_changed = true;
                    changed = true;
                    current_state = 0;
                }
            }

            if (keystate[SW_BTN_LEFT] >= PRESSED) {
                if (current_game > 0) {
                    current_game--;
                    current_game_changed = true;
                    changed = true;
                    current_state = 0;
                }
            }

            if (keystate[SW_BTN_UP] >= PRESSED) {
                if (current_state > 0) {
                    current_state--;
                    current_game_changed = true;
                    changed = true;
                }
            }

            if (keystate[SW_BTN_DOWN] >= PRESSED) {
                Game_s *game = &game_list[current_game];
                if (current_state < game->savingStatesNum) {
                    current_state++;
                    current_game_changed = true;
                    changed = true;
                }
            }

            if (keystate[SW_BTN_START] == PRESSED) {
                exit_to_menu = true;
                break;
            }

            if (keystate[SW_BTN_A] == PRESSED) {
                playCurrentState();
                break;
            }

            if (keystate[SW_BTN_B] == PRESSED) {
                if (current_state == 0) {
                    saveCurrentAutoState();
                    current_game_changed = true;
                    changed = true;
                }
                else {
                    if (strcmp(settings.language, "ru.lang") == 0) {
                        theme_renderDialog(
                            screen, "Удаление сохранения",
                            "Вы уверены, что хотите\nудалить выбранное сохранение?",
                            true);
                    }
                    else {
                        theme_renderDialog(
                            screen, "Delete save",
                            "Are you sure you want to\ndelete selected save?",
                            true);
                    }
                    SDL_BlitSurface(screen, NULL, video, NULL);
                    SDL_Flip(video);
                    sound_change();

                    while (!quit) {
                        if (updateKeystate(keystate, &quit, true, NULL)) {
                            if (keystate[SW_BTN_A] == PRESSED) {
                                removeCurrentState();
                                sortSavingStates();
                                current_state--;
                                current_game_changed = true;
                                changed = true;
                                break;
                            }
                            if (keystate[SW_BTN_B] == PRESSED) {
                                changed = true;
                                break;
                            }
                        }
                    }
                }
            }

            if (combo_key ||
                (select_pressed && ((changed_key == SW_BTN_L2 &&
                                     keystate[SW_BTN_L2] == RELEASED) ||
                                    (changed_key == SW_BTN_R2 &&
                                     keystate[SW_BTN_R2] == RELEASED)))) {
                settings_load();
                changed = true;
            }

            if (changed_key == SW_BTN_SELECT) {
                if (keystate[SW_BTN_SELECT] == PRESSED)
                    select_pressed = true;
                if (keystate[SW_BTN_SELECT] == RELEASED) {
                    if (!select_combo_key) {
                        show_legend = true;
                        legend_start = last_ticks;

                        if (!show_time && !show_total)
                            show_time = true, show_total = false;
                        else if (show_time && !show_total)
                            show_time = true, show_total = true;
                        else
                            show_time = false, show_total = false;

                        config_flag_set("gameSwitcher/showTime", show_time);
                        config_flag_set("gameSwitcher/hideTotal", !show_total);

                        changed = true;
                    }
                    select_pressed = false;
                    select_combo_key = false;
                }
            }

            if (changed_key == SW_BTN_Y && keystate[SW_BTN_Y] == RELEASED) {
                if (button_y_repeat < 75) {
                    view_mode = view_mode == VIEW_FULLSCREEN ? view_restore
                                                             : !view_mode;
                    config_flag_set("gameSwitcher/minimal", view_mode == VIEW_MINIMAL);
                    changed = true;
                }
                button_y_repeat = 0;
            }

            if (keystate[SW_BTN_X] == PRESSED) {
                if (game_list_len != 0) {
                    theme_renderDialog(
                        screen, "Remove from history",
                        "Are you sure you want to\nremove game from history?",
                        true);
                    SDL_BlitSurface(screen, NULL, video, NULL);
                    SDL_Flip(video);
                    sound_change();

                    while (!quit) {
                        if (updateKeystate(keystate, &quit, true, NULL)) {
                            if (keystate[SW_BTN_A] == PRESSED) {
                                removeCurrentItem();
                                if (current_game > 0)
                                    current_game--;
                                current_game_changed = true;
                                changed = true;
                                break;
                            }
                            if (keystate[SW_BTN_B] == PRESSED) {
                                changed = true;
                                break;
                            }
                        }
                    }
                }
            }

            if (changed)
                sound_change();
        }

        if (keystate[SW_BTN_Y] == PRESSED && view_mode != VIEW_FULLSCREEN) {
            button_y_repeat++;
            if (button_y_repeat >= 75) {
                view_restore = view_mode;
                view_mode = VIEW_FULLSCREEN;
                changed = true;
            }
        }

        if (battery_hasChanged(ticks, &battery_percentage))
            changed = true;

        if (acc_ticks >= time_step) {
            acc_ticks -= time_step;

            if (!changed && (surfaceGameName == NULL || surfaceGameName->w <= game_name_max_width))
                continue;

            if (changed) {
                SDL_BlitSurface(theme_background(), NULL, screen, NULL);

                if (game_list_len == 0) {
                    current_bg = NULL;
                    SDL_Surface *empty = resource_getSurface(EMPTY_BG);
                    SDL_Rect empty_rect = {320 - empty->w / 2,
                                           240 - empty->h / 2};
                    SDL_BlitSurface(empty, NULL, screen, &empty_rect);
                }
                else {
                    current_bg = loadRomScreen(current_game);
                    if (current_bg != NULL) {
                        if (current_bg->w < 640) {
                            const double zoomFactor = 640 / (double)current_bg->w;
                            current_bg = rotozoomSurface(current_bg, 0.0, zoomFactor, 0);
                        }
                        SDL_Rect current_bg_rect = {320 - current_bg->w / 2, 240 - current_bg->h / 2};
                        SDL_BlitSurface(current_bg, NULL, screen, &current_bg_rect);
                    }
                }
            }

            Game_s *game = &game_list[current_game];

            if (view_mode != VIEW_FULLSCREEN && game_list_len > 0) {
                SDL_Rect game_name_bg_size = {0, 0, 640, 120};
                SDL_Rect game_name_bg_pos = {0, 360};

                if (view_mode == VIEW_NORMAL) {
                    game_name_bg_size.x = game_name_bg_pos.x =
                        theme()->frame.border_left;
                    game_name_bg_size.w -= theme()->frame.border_left +
                                           theme()->frame.border_right;
                }

                game_name_bg_size.y = game_name_bg_pos.y =
                    view_mode == VIEW_NORMAL ? (480 - footer_height - 30) : 440;

                SDL_BlitSurface(transparent_bg, &game_name_bg_size, screen,
                                &game_name_bg_pos);

                if (current_game > 0) {
                    SDL_Rect arrow_left_rect = {theme()->frame.border_left + 10,
                                                game_name_bg_pos.y + 20 -
                                                    arrow_left->h / 2};
                    SDL_BlitSurface(arrow_left, NULL, screen, &arrow_left_rect);
                }

                if (current_game < game_list_len - 1) {
                    SDL_Rect arrow_right_rect = {
                        630 - theme()->frame.border_right - arrow_right->w,
                        game_name_bg_pos.y + 20 - arrow_right->h / 2};
                    SDL_BlitSurface(arrow_right, NULL, screen,
                                    &arrow_right_rect);
                }

                char game_name_str[STR_MAX * 2 + 4];

                if (game->is_duplicate > 0)
                    snprintf(game_name_str, STR_MAX * 2 + 3, "%s (%s)",
                             game->shortname, game->core);
                else
                    strcpy(game_name_str, game->shortname);

                if (current_state > 0)
                    snprintf(game_name_str, STR_MAX * 2 + 3, "%s (Save %i)", strdup(game_name_str), current_state);

                if (current_game_changed) {
                    if (surfaceGameName != NULL)
                        SDL_FreeSurface(surfaceGameName);
                    surfaceGameName = TTF_RenderUTF8_Blended(
                        resource_getFont(TITLE), game_name_str, color_white);
                    game_name_size.w = surfaceGameName->w < game_name_max_width
                                           ? surfaceGameName->w
                                           : game_name_max_width;
                    game_name_size.h = surfaceGameName->h;
                    gameNameScrollX =
                        -gameNameScrollStart * gameNameScrollSpeed;

                    sortSavingStates();
                }

                SDL_Rect game_name_rect = {320 - surfaceGameName->w / 2,
                                           game_name_bg_pos.y + 20 -
                                               surfaceGameName->h / 2};
                if (game_name_rect.x < game_name_padding)
                    game_name_rect.x = game_name_padding;

                game_name_size.x =
                    gameNameScrollX < (surfaceGameName->w - game_name_size.w)
                        ? (gameNameScrollX > 0 ? gameNameScrollX : 0)
                        : surfaceGameName->w - game_name_size.w;

                SDL_BlitSurface(surfaceGameName, &game_name_size, screen,
                                &game_name_rect);

                if (surfaceGameName->w > game_name_max_width) {
                    gameNameScrollX += gameNameScrollSpeed;

                    if (gameNameScrollX >
                        (surfaceGameName->w - game_name_size.w +
                         gameNameScrollEnd * gameNameScrollSpeed))
                        gameNameScrollX =
                            -gameNameScrollStart * gameNameScrollSpeed;
                }
            }

            if (!changed) {
                SDL_BlitSurface(screen, NULL, video, NULL);
                SDL_Flip(video);
                continue;
            }

            if (view_mode == VIEW_NORMAL) {
                if (use_custom_footer && custom_footer) {
                    if (footer_height > 0) {
                        SDL_Rect footer_rect = {0, 480 - custom_footer->h};
                        SDL_BlitSurface(custom_footer, NULL, screen,
                                        &footer_rect);
                    }
                }
                else {
                    //theme_renderFooter(screen);

                    /*theme_renderStandardHint(
                        screen, lang_get(LANG_RESUME, LANG_FALLBACK_RESUME),
                        lang_get(LANG_BACK, LANG_FALLBACK_BACK));*/

                    if (current_state == 0) {
                        if (!game->currentAutoStateSaved) {
                            theme_renderStandardHint(screen, "RESUME", "SAVE");
                        }
                        else {
                            theme_renderStandardHint(screen, "RESUME", NULL);
                        }
                    }
                    else {
                        theme_renderStandardHint(screen, "RESUME", "DELETE");
                    }

                    if (footer_height > 0) {
                        SDL_Rect footer_rect = {640 - custom_footer->w, 480 - custom_footer->h - 9};
                        SDL_BlitSurface(custom_footer, NULL, screen, &footer_rect);
                    }

                    /*theme_renderFooterStatus(
                        screen, game_list_len > 0 ? current_game + 1 : 0,
                        game_list_len);*/
                }
            }

            if (view_mode == VIEW_NORMAL) {
                char title_str[STR_MAX] = "Game Switcher Mod";
                if (show_time && game_list_len > 0) {
                    if (strlen(game->totalTime) == 0) {
                        str_serializeTime(game->totalTime, play_activity_get_play_time(game->path));
                    }
                    strcpy(title_str, game->totalTime);

                    if (show_total) {
                        if (strlen(sTotalTimePlayed) == 0) {
                            str_serializeTime(sTotalTimePlayed, play_activity_get_total_play_time());
                        }
                        sprintf(title_str + strlen(title_str), " / %s", sTotalTimePlayed);
                    }
                }

                if (use_custom_header && custom_header) {
                    if (header_height > 0) {
                        SDL_BlitSurface(custom_header, NULL, screen, NULL);
                        SDL_Surface *title = TTF_RenderUTF8_Blended(
                            resource_getFont(TITLE), title_str,
                            theme()->title.color);
                        if (title) {
                            SDL_Rect title_rect = {320 - title->w / 2,
                                                   (header_height - title->h) /
                                                       2};
                            SDL_BlitSurface(title, NULL, screen, &title_rect);
                            SDL_FreeSurface(title);
                        }
                        theme_renderHeaderBatteryCustom(
                            screen, battery_percentage, header_height);
                    }
                }
                else {
                    //theme_renderHeader(screen, title_str, false);

                    SDL_Rect header_bg_size = {0, 0, 640, 40};

                    SDL_BlitSurface(transparent_bg, &header_bg_size, screen, NULL);

                    SDL_Surface *title = TTF_RenderUTF8_Blended(
                        resource_getFont(TITLE), title_str,
                        theme()->title.color);
                    if (title) {
                        SDL_Rect title_rect = {320 - title->w / 2, (header_bg_size.h - title->h) / 2};
                        SDL_BlitSurface(title, NULL, screen, &title_rect);
                        SDL_FreeSurface(title);
                    }
                    theme_renderHeaderBatteryCustom(screen, battery_percentage, header_bg_size.h);

                    //theme_renderHeaderBattery(screen, battery_percentage);
                }
            }

            if (show_legend && view_mode != VIEW_FULLSCREEN) {
                SDL_Surface *legend = resource_getSurface(LEGEND_GAMESWITCHER);
                SDL_Rect legend_rect = {640 - legend->w,
                                        view_mode == VIEW_NORMAL ? header_height
                                                                 : 0};
                SDL_BlitSurface(legend, NULL, screen, &legend_rect);
            }

            SDL_BlitSurface(screen, NULL, video, NULL);
            SDL_Flip(video);

            changed = false;
            current_game_changed = false;
        }
    }

    // seamless game load
    //screen = SDL_CreateRGBSurface(SDL_HWSURFACE, 640, 480, 32, 0, 0, 0, 0);

    remove("/mnt/SDCARD/.tmp_update/.runGameSwitcher");
    remove("/mnt/SDCARD/.tmp_update/cmd_to_run.sh");

    if (exit_to_menu)
        print_debug("Exiting to menu");
    else {
        print_debug("Resuming game");
        FILE *file = fopen("/mnt/SDCARD/.tmp_update/cmd_to_run.sh", "w");
        fputs(game_list[current_game].RACommand, file);
        fclose(file);
    }

#ifndef PLATFORM_MIYOOMINI
    msleep(200);
#endif

    if (json_root != NULL)
        cJSON_free(json_root);

    if (jsonCache_root != NULL)
        cJSON_free(jsonCache_root);

    // seamless game load
    /*SDL_BlitSurface(screen, NULL, video, NULL);
    SDL_Flip(video);
    SDL_BlitSurface(screen, NULL, video, NULL);
    SDL_Flip(video);*/

    if (custom_header != NULL)
        SDL_FreeSurface(custom_header);
    if (custom_footer != NULL)
        SDL_FreeSurface(custom_footer);
    if (surfaceGameName != NULL)
        SDL_FreeSurface(surfaceGameName);

    resources_free();
    SDL_FreeSurface(transparent_bg);

    freeRomScreens();

    SDL_FreeSurface(screen);
    SDL_FreeSurface(video);

    TTF_Quit();
    SDL_Quit();

    return EXIT_SUCCESS;
}
