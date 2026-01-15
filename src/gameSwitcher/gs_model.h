#ifndef GAME_SWITCHER_MODEL_H
#define GAME_SWITCHER_MODEL_H

#include <SDL/SDL.h>
#include <pthread.h>
#include <stdbool.h>

#include "SDL/SDL_rotozoom.h"
#include "system/display.h"
#include "utils/retroarch_cmd.h"
#include "utils/sdl_direct_fb.h"
#include "utils/str.h"

#define MAX_HISTORY 100

#define ROM_TEMP_DIR "/mnt/SDCARD/.tmp_update/.tmp"
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
    bool is_running;
} Game_s;

static Game_s game_list[MAX_HISTORY];
static int game_list_len = 0;

#endif // GAME_SWITCHER_MODEL_H
