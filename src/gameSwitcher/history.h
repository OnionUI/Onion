#ifndef GAME_SWITCHER_HISTORY_H
#define GAME_SWITCHER_HISTORY_H

#include <SDL/SDL.h>
#include <pthread.h>
#include <stdbool.h>

#include "SDL/SDL_rotozoom.h"
#include "system/display.h"
#include "system/screenshot.h"
#include "system/state.h"
#include "utils/file.h"
#include "utils/json.h"
#include "utils/log.h"
#include "utils/str.h"

#include "../playActivity/cacheDB.h"

#define MAX_HISTORY 100
#define ROM_SCREENS_DIR "/mnt/SDCARD/Saves/CurrentProfile/romScreens"
#define HISTORY_PATH "/mnt/SDCARD/Saves/CurrentProfile/lists/content_history.lpl"
#define CONFIG_DIR "/mnt/SDCARD/Saves/CurrentProfile/config"
#define STATES_DIR "/mnt/SDCARD/Saves/CurrentProfile/states"
#define RETROARCH_CONFIG_PATH "/mnt/SDCARD/RetroArch/.retroarch/retroarch.cfg"
#define ASPECT_RATIO_OPTION "video_dingux_ipu_keep_aspect"
#define INTEGER_SCALING_OPTION "video_scale_integer"

typedef struct {
    char label[STR_MAX * 2];
    char rompath[STR_MAX * 2];
    char imgpath[STR_MAX * 2];
    char launch[STR_MAX * 2];
    int type;
    int lineNo;
} RecentItem;

// Game history list
typedef struct {
    RecentItem recentItem;
    SDL_Surface *romScreen;
    char rom_name[STR_MAX * 2];
    char name[STR_MAX * 2];
    char shortname[STR_MAX * 2];
    char core_name[STR_MAX * 2];
    char core_path[STR_MAX * 2];
    char totalTime[100];
    int index;
    bool processed;
} Game_s;

void processItem(Game_s *game);

static bool __initial_romscreens_loaded = false;
static pthread_t romscreen_thread_pt;
static pthread_mutex_t thread_mutex = PTHREAD_MUTEX_INITIALIZER;

static Game_s game_list[MAX_HISTORY];
static int game_list_len = 0;

static cJSON *g_cachedRetroArchHistory = NULL;

bool parseJsonToRecentItem(const char *jsonStr, RecentItem *recentItem, int lineNo)
{
    cJSON *json = cJSON_Parse(jsonStr);
    if (json == NULL) {
        print_debug("Error parsing JSON");
        return false;
    }

    cJSON *type = cJSON_GetObjectItemCaseSensitive(json, "type");
    if (!cJSON_IsNumber(type) || (type->valueint != 5 && type->valueint != 17)) {
        cJSON_Delete(json);
        return false;
    }

    cJSON *label = cJSON_GetObjectItemCaseSensitive(json, "label");
    cJSON *rompath = cJSON_GetObjectItemCaseSensitive(json, "rompath");
    cJSON *imgpath = cJSON_GetObjectItemCaseSensitive(json, "imgpath");
    cJSON *launch = cJSON_GetObjectItemCaseSensitive(json, "launch");

    if (cJSON_IsString(label) && (label->valuestring != NULL)) {
        strncpy(recentItem->label, label->valuestring, sizeof(recentItem->label) - 1);
    }
    if (cJSON_IsString(rompath) && (rompath->valuestring != NULL)) {
        strncpy(recentItem->rompath, rompath->valuestring, sizeof(recentItem->rompath) - 1);
    }
    if (cJSON_IsString(imgpath) && (imgpath->valuestring != NULL)) {
        strncpy(recentItem->imgpath, imgpath->valuestring, sizeof(recentItem->imgpath) - 1);
    }
    if (cJSON_IsString(launch) && (launch->valuestring != NULL)) {
        strncpy(recentItem->launch, launch->valuestring, sizeof(recentItem->launch) - 1);
    }
    recentItem->type = type->valueint;
    recentItem->lineNo = lineNo;

    // Check if rompath contains a colon (':') and split it into launch and rompath
    char *colonPosition = strchr(recentItem->rompath, ':');
    if (colonPosition != NULL) {
        int position = (int)(colonPosition - recentItem->rompath);

        char firstPart[position + 1];
        strncpy(firstPart, recentItem->rompath, position);
        firstPart[position] = '\0';

        char secondPart[strlen(recentItem->rompath) - position];
        strcpy(secondPart, colonPosition + 1);

        strcpy(recentItem->launch, firstPart);
        strcpy(recentItem->rompath, secondPart);
    }

    cJSON_Delete(json);
    return true;
}

void setEntryDefaultValues(Game_s *game, int index)
{
    game->romScreen = NULL;
    game->totalTime[0] = '\0';
    game->processed = false;

    strcpy(game->name, "");
    strcpy(game->shortname, "");
    strcpy(game->core_name, "");
    strcpy(game->core_path, "");
    game->index = index;
}

/**
 * @brief Read the first entry from the history file
 *
 */
void readFirstEntry()
{
    FILE *file;
    char line[STR_MAX * 6];

    file = fopen(getMiyooRecentFilePath(), "r");
    if (file == NULL) {
        print_debug("Error opening file");
        return;
    }

    int lineNo = -1;
    bool found = false;

    while (fgets(line, sizeof(line), file) != NULL) {
        ++lineNo;

        if (parseJsonToRecentItem(line, &game_list[0].recentItem, lineNo)) {
            found = true;
            break;
        }
    }

    if (found) {
        setEntryDefaultValues(&game_list[0], 0);
        game_list_len = 1;
    }

    fclose(file);
}

/**
 * @brief History extraction
 *
 */
void readHistory()
{
    FILE *file;
    char line[STR_MAX * 6];
    int numRecents = 0;

    const char *recentFilePath = getMiyooRecentFilePath();

    file = fopen(recentFilePath, "r");
    if (file == NULL) {
        print_debug("Error opening file");
        return;
    }

    int lineNo = 0;

    while ((fgets(line, sizeof(line), file) != NULL) && (numRecents < MAX_HISTORY)) {
        ++lineNo;

        if (!parseJsonToRecentItem(line, &game_list[numRecents].recentItem, lineNo)) {
            continue;
        }

        // Check for duplicates by looping over the list
        bool isDuplicate = false;
        for (int i = 0; i < numRecents; i++) {
            if (strcmp(game_list[i].recentItem.rompath, game_list[numRecents].recentItem.rompath) == 0) {
                isDuplicate = true;
                break;
            }
        }

        if (isDuplicate) {
            file_delete_line(recentFilePath, lineNo);
            lineNo--;
            continue;
        }

        if (!exists(game_list[numRecents].recentItem.rompath) || !exists(game_list[numRecents].recentItem.launch)) {
            continue;
        }

        if (!game_list[numRecents].processed) {
            setEntryDefaultValues(&game_list[numRecents], numRecents);
        }

        numRecents++;
    }

    fclose(file);
    game_list_len = numRecents;
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

typedef enum {
    ROM_SCREEN_NONE = 0,
    ROM_SCREEN_STATE,
    ROM_SCREEN_HASH,
    ROM_SCREEN_ARTWORK
} RomScreenType_e;

RomScreenType_e findRomScreen(const Game_s *game, char *currPicture)
{
    // Check if save state image exists
    // if (strlen(game->core_name) != 0) {
    //     sprintf(currPicture, STATES_DIR "/%s/%s.state.auto.png", game->core_name, game->rom_name);
    //     printf_debug("Checking for save state image: %s\n", currPicture);
    //     if (exists(currPicture)) {
    //         return ROM_SCREEN_STATE;
    //     }
    // }

    // Check if hashed rom screen exists
    uint32_t hash = FNV1A_Pippip_Yurii(game->recentItem.rompath, strlen(game->recentItem.rompath));
    sprintf(currPicture, ROM_SCREENS_DIR "/%" PRIu32 ".png", hash);
    printf_debug("Checking for hashed rom screen: %s\n", currPicture);
    if (exists(currPicture)) {
        return ROM_SCREEN_HASH;
    }

    // Check if artwork exists
    sprintf(currPicture, game->recentItem.imgpath);
    printf_debug("Checking for artwork: %s\n", currPicture);
    if (exists(currPicture)) {
        return ROM_SCREEN_ARTWORK;
    }

    return ROM_SCREEN_NONE;
}

typedef struct {
    bool keepAspect;
    bool integerScaling;
} ScalingMode_s;

void scaleRomScreen(Game_s *game, ScalingMode_s mode)
{
    // Zoom the image to fit the screen
    double zx = (double)(DISPLAY_WIDTH) / game->romScreen->w;
    double zy = (double)(DISPLAY_HEIGHT) / game->romScreen->h;

    if (mode.integerScaling) {
        zx = (int)zx;
        zy = (int)zy;
    }

    // Scale the image to fit application window
    zx *= 640.0 / (double)(DISPLAY_WIDTH);
    zy *= 480.0 / (double)(DISPLAY_HEIGHT);

    if (mode.keepAspect) {
        if (zx < zy)
            zy = zx;
        else
            zx = zy;
    }

    SDL_Surface *zoomed = zoomSurface(game->romScreen, zx, zy, SMOOTHING_OFF);
    SDL_FreeSurface(game->romScreen);
    game->romScreen = zoomed;
}

bool getBoolFromConfig(const char *cfg_path, bool *out_value, const char *key)
{
    char value[STR_MAX * 2];
    file_parseKeyValue(cfg_path, key, value, '=', 0);
    if (strcmp(value, "true") == 0) {
        *out_value = true;
        return true;
    }
    else if (strcmp(value, "false") == 0) {
        *out_value = false;
        return true;
    }
    return false;
}

bool getConfigOverrideOption(const Game_s *game, const char *key, bool defaultValue)
{
    bool result = false;
    char cfg_path[4096];

    // Game override
    snprintf(cfg_path, sizeof(cfg_path), CONFIG_DIR "/%s/%s.cfg", game->core_name, game->rom_name);
    if (getBoolFromConfig(cfg_path, &result, key)) {
        printf_debug("Game override: %s=%s\n", key, result ? "true" : "false");
        return result;
    }

    // Content directory override
    char *contentDir = file_dirname(game->recentItem.rompath);
    snprintf(cfg_path, sizeof(cfg_path), CONFIG_DIR "/%s/%s.cfg", game->core_name, file_basename(contentDir));
    free(contentDir);
    if (getBoolFromConfig(cfg_path, &result, key)) {
        printf_debug("Content directory override: %s=%s\n", key, result ? "true" : "false");
        return result;
    }

    // Core override
    snprintf(cfg_path, sizeof(cfg_path), CONFIG_DIR "/%s/%s.cfg", game->core_name, game->core_name);
    if (getBoolFromConfig(cfg_path, &result, key)) {
        printf_debug("Core override: %s=%s\n", key, result ? "true" : "false");
        return result;
    }

    // Global override
    if (getBoolFromConfig(RETROARCH_CONFIG_PATH, &result, key)) {
        return result;
    }

    return defaultValue;
}

ScalingMode_s getDynamicScalingMode(const Game_s *game)
{
    return (ScalingMode_s){
        getConfigOverrideOption(game, ASPECT_RATIO_OPTION, true),
        getConfigOverrideOption(game, INTEGER_SCALING_OPTION, false),
    };
}

SDL_Surface *loadRomScreen(int index)
{
    if (index < 0 || index >= game_list_len)
        return NULL;

    Game_s *game = &game_list[index];

    pthread_mutex_lock(&thread_mutex);

    if (game->romScreen == NULL && game->processed) {
        char currPicture[STR_MAX * 2];
        RomScreenType_e romScreenType = findRomScreen(game, currPicture);
        if (romScreenType != ROM_SCREEN_NONE) {
            game->romScreen = IMG_Load(currPicture);

            if (game->romScreen == NULL) {
                printf_debug("Error loading image: %s\n", currPicture);
            }
            else if (romScreenType == ROM_SCREEN_STATE) {
                scaleRomScreen(game, getDynamicScalingMode(game));
            }
            else if (romScreenType == ROM_SCREEN_ARTWORK && (game->romScreen->w > 640 || game->romScreen->h > 480)) {
                scaleRomScreen(game, (ScalingMode_s){true, false});
            }
        }
    }

    if (__initial_romscreens_loaded) {
        unloadRomScreen(index + 5);
        if (index > 5) {
            unloadRomScreen(index - 5);
        }
    }

    pthread_mutex_unlock(&thread_mutex);

    return game->romScreen;
}

void freeRomScreens()
{
    for (int i = 0; i < game_list_len; i++) {
        Game_s *game = &game_list[i];

        if (game->romScreen != NULL) {
            SDL_FreeSurface(game->romScreen);
            game->romScreen = NULL;
        }
    }
}

static void *_loadRomScreensThread(void *_)
{
    for (int i = 0; i < 10 && i < game_list_len; i++) {
        Game_s *game = &game_list[i];

        if (game->romScreen == NULL)
            loadRomScreen(i);
    }

    __initial_romscreens_loaded = true;

    return NULL;
}

void loadRomScreens()
{
    pthread_create(&romscreen_thread_pt, NULL, _loadRomScreensThread, NULL);
}

bool getGameName(char *name_out, const char *rom_path)
{
    CacheDBItem *cache_item = cache_db_find(rom_path);
    if (cache_item != NULL) {
        strcpy(name_out, cache_item->name);
        free(cache_item);
        return true;
    }
    return false;
}

bool loadRetroArchHistory(const char *jsonFilePath)
{
    if (g_cachedRetroArchHistory != NULL) {
        cJSON_Delete(g_cachedRetroArchHistory);
        g_cachedRetroArchHistory = NULL;
    }

    FILE *file = fopen(jsonFilePath, "r");
    if (file == NULL) {
        print_debug("Error opening JSON file");
        return false;
    }

    fseek(file, 0, SEEK_END);
    long fileSize = ftell(file);
    fseek(file, 0, SEEK_SET);

    char *fileContent = (char *)malloc(fileSize + 1);
    fread(fileContent, 1, fileSize, file);
    fileContent[fileSize] = '\0';
    fclose(file);

    g_cachedRetroArchHistory = cJSON_Parse(fileContent);
    free(fileContent);

    if (g_cachedRetroArchHistory == NULL) {
        print_debug("Error parsing JSON");
        return false;
    }

    return true;
}

void freeRetroArchHistory()
{
    if (g_cachedRetroArchHistory != NULL) {
        cJSON_Delete(g_cachedRetroArchHistory);
        g_cachedRetroArchHistory = NULL;
    }
}

bool findItemInRetroArchHistory(Game_s *game)
{
    if (g_cachedRetroArchHistory == NULL && !loadRetroArchHistory(HISTORY_PATH)) {
        print_debug("Error loading RetroArch history");
        return false;
    }

    cJSON *items = cJSON_GetObjectItemCaseSensitive(g_cachedRetroArchHistory, "items");
    if (!cJSON_IsArray(items)) {
        return false;
    }

    char *cleanPath = file_resolvePath(game->recentItem.rompath);
    if (cleanPath == NULL) {
        return false;
    }

    printf_debug("Searching for: %s\n", cleanPath);

    cJSON *item;
    cJSON_ArrayForEach(item, items)
    {
        cJSON *path = cJSON_GetObjectItemCaseSensitive(item, "path");
        if (cJSON_IsString(path) && (path->valuestring != NULL)) {
            if (strcmp(path->valuestring, cleanPath) == 0) {
                // Found the matching item
                cJSON *core_path = cJSON_GetObjectItemCaseSensitive(item, "core_path");

                if (cJSON_IsString(core_path) && (core_path->valuestring != NULL)) {
                    strncpy(game->core_path, core_path->valuestring, sizeof(game->core_path) - 1);
                }

                printf_debug("Found item in RetroArch history: %s\n", path->valuestring);
                printf_debug("Core path: %s\n", game->core_path);

                free(cleanPath);
                return true;
            }
        }
    }

    free(cleanPath);
    return false;
}

void getCoreNameFromInfo(Game_s *game)
{
    if (strlen(game->core_name) == 0) {
        if (strlen(game->core_path) == 0) {
            return;
        }

        char *basePath = file_removeExtension(game->core_path);
        if (basePath != NULL) {
            char infoPath[STR_MAX * 2];
            snprintf(infoPath, sizeof(infoPath), "%s.info", basePath);
            file_parseKeyValue(infoPath, "corename", game->core_name, '=', 0);
            printf_debug("Core name: %s\n", game->core_name);
            free(basePath);
        }
    }
}

void processItem(Game_s *game)
{
    if (game->processed) {
        return;
    }

    game->processed = true;

    char *rom_name = file_removeExtension(file_basename(game->recentItem.rompath));
    strcpy(game->rom_name, rom_name);
    free(rom_name);

    if (!getGameName(game->name, game->recentItem.rompath)) {
        strcpy(game->name, game->rom_name);
    }

    file_cleanName(game->shortname, game->name);

    if (findItemInRetroArchHistory(game)) {
        getCoreNameFromInfo(game);
    }

    if (game->romScreen == NULL) {
        game->romScreen = loadRomScreen(game->index);
    }
}

void getLaunchCommand(Game_s *game, char *launchCommand)
{
    snprintf(launchCommand, 4096, "LD_PRELOAD=/mnt/SDCARD/miyoo/app/../lib/libpadsp.so \"%s\" \"%s\"", game->recentItem.launch, game->recentItem.rompath);
}

#endif // GAME_SWITCHER_HISTORY_H