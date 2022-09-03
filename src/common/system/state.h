#ifndef SYSTEM_STATE_H__
#define SYSTEM_STATE_H__

#include <unistd.h>

#include "utils/utils.h"
#include "utils/flags.h"
#include "utils/process.h"
#include "./settings.h"
#include "./display.h"

typedef enum system_state_e
{
    MODE_UNKNOWN,
    MODE_MAIN_UI,
    MODE_SWITCHER,
    MODE_GAME,
    MODE_APPS
} SystemState;
static SystemState system_state = MODE_UNKNOWN;
static pid_t system_state_pid = 0;

bool check_isRetroArch(void)
{
    if (!exists(CMD_TO_RUN_PATH))
        return false;
    const char *cmd = file_read(CMD_TO_RUN_PATH);
    if (strstr(cmd, "retroarch") != NULL || strstr(cmd, "/mnt/SDCARD/Emu/") != NULL || strstr(cmd, "/mnt/SDCARD/RApp/") != NULL) {
        pid_t pid;
        if ((pid = process_searchpid("retroarch")) != 0 || (pid = process_searchpid("ra32")) != 0) {
            system_state_pid = pid;
            return true;
        }
    }
    return false;
}

bool check_isMainUI(void)
{
    pid_t pid;
    if (!exists(CMD_TO_RUN_PATH) && (pid = process_searchpid("MainUI")) != 0) {
        system_state_pid = pid;
        return true;
    }
    return false;
}

bool check_isGameSwitcher(void)
{
    pid_t pid;
    if (exists("/mnt/SDCARD/.tmp_update/.runGameSwitcher") && (pid = process_searchpid("gameSwitcher")) != 0) {
        system_state_pid = pid;
        return true;
    }
    return false;
}

void system_state_update(void)
{
    if (check_isGameSwitcher())
        system_state = MODE_SWITCHER;
    else if (check_isRetroArch())
        system_state = MODE_GAME;
    else if (check_isMainUI())
        system_state = MODE_MAIN_UI;
    else system_state = MODE_APPS;

    #ifdef LOG_DEBUG
    switch (system_state) {
        case MODE_MAIN_UI: print_debug("System state: Main UI"); break;
        case MODE_SWITCHER: print_debug("System state: Game Switcher"); break;
        case MODE_GAME: print_debug("System state: RetroArch"); break;
        case MODE_APPS: print_debug("System state: Apps"); break;
        default: print_debug("System state: Unknown"); break;
    }
    #endif
}

size_t state_getAppName(char *out, const char *str)
{
    char *end;
    size_t out_size;
    
    str += 19;
    end = strchr(str, ';');
    
    out_size = (end - str) < STR_MAX-1 ? (end - str) : STR_MAX-1;
    memcpy(out, str, out_size);
    out[out_size] = 0;
    
    return out_size;
}

//
//    [onion] Check retroarch running & savestate_auto_save in retroarch.cfg is true
//
int check_autosave(void)
{
    char value[STR_MAX];
    file_parseKeyValue(RETROARCH_CONFIG, "savestate_auto_save", value, '=', 0);
    return strcmp(value, "true") == 0;
}

void kill_mainUI(void)
{
    if (system_state == MODE_MAIN_UI) {
        kill(system_state_pid, SIGKILL);
        display_reset();
    }
}

void run_gameSwitcher(void)
{
    flag_set("/mnt/SDCARD/.tmp_update/", ".runGameSwitcher", true);
}

static char ***_installed_apps;
static int installed_apps_count = 0;
static bool installed_apps_loaded = false;

bool _getAppDirAndConfig(const char *app_dir_name, char *out_app_dir, char *out_config_path)
{
    memset(out_app_dir, 0, STR_MAX * sizeof(char));
    memset(out_config_path, 0, STR_MAX * sizeof(char));

    strcpy(out_app_dir, "/mnt/SDCARD/App/");
    strncat(out_app_dir, app_dir_name, 128);

    if (!is_dir(out_app_dir))
        return false;

    strcpy(out_config_path, out_app_dir);
    strcat(out_config_path, "/config.json");

    if (!is_file(out_config_path))
        return false;

    return true;
}

char*** getInstalledApps()
{
    DIR *dp;
    struct dirent *ep;
    char app_dir[STR_MAX], config_path[STR_MAX];

    if (!installed_apps_loaded) {
        if ((dp = opendir("/mnt/SDCARD/App")) == NULL)
            return false;

        _installed_apps = (char***)malloc(100 * sizeof(char**));

        while ((ep = readdir(dp))) {
            if (ep->d_type != DT_DIR || strcmp(ep->d_name, ".") == 0 || strcmp(ep->d_name, "..") == 0)
                continue;
            int i = installed_apps_count;
    
            if (!_getAppDirAndConfig(ep->d_name, app_dir, config_path))
                continue;

            _installed_apps[i] = (char**)malloc(2 * sizeof(char*));
            _installed_apps[i][0] = (char*)malloc(STR_MAX * sizeof(char));
            _installed_apps[i][1] = (char*)malloc(STR_MAX * sizeof(char));

            strncpy(_installed_apps[i][0], ep->d_name, STR_MAX - 1);
            file_parseKeyValue(config_path, "label", _installed_apps[i][1], ':', 0);

            printf_debug("app %d: %s (%s)\n", i, _installed_apps[i][0], _installed_apps[i][1]);

            installed_apps_count++;
        }

        installed_apps_loaded = true;
    }

    return _installed_apps;
}

bool getAppPosition(const char *app_dir_name, int *currpos, int *total)
{
    bool found = false;
    *currpos = 0;
    *total = 0;

    getInstalledApps();

    for (int i = 0; i < installed_apps_count; i++) {
        if (strncmp(app_dir_name, _installed_apps[i][0], STR_MAX) == 0) {
            *currpos = i;
            found = true;
            break;
        }
    }
    *total = installed_apps_count;

    printf_debug("app pos: %d (total: %d)\n", *currpos, *total);

    return found;
}

void run_app(const char *app_dir_name)
{
    char app_dir[STR_MAX], config_path[STR_MAX];
    
    if (!_getAppDirAndConfig(app_dir_name, app_dir, config_path))
        return;

    char launch[STR_MAX];
    file_parseKeyValue(config_path, "launch", launch, ':', 0);

    if (strlen(launch) == 0)
        return;

    FILE *fp;
    char cmd[STR_MAX * 4];
    snprintf(cmd, STR_MAX * 4 - 1, "cd %s; chmod a+x ./%s; LD_PRELOAD=/mnt/SDCARD/miyoo/app/../lib/libpadsp.so ./%s", app_dir, launch, launch);
    file_put_sync(fp, "/tmp/cmd_to_run.sh", "%s", cmd);
}

typedef enum mainui_states
{
    MAIN_MENU,
    RECENTS,
    FAVORITES,
    GAMES,
    EXPERT,
    APPS
} MainUIState;

void write_mainui_state(MainUIState state, int currpos, int total)
{
    FILE *fp;
    char state_str[STR_MAX];
    int title_num = 0,
        page_type = 0,
        page_size = 6,
        page_start = 0,
        page_end,
        main_currpos = 0,
        main_page_start = 0,
        main_page_end;

    switch (state) {
        case MAIN_MENU: remove("/tmp/state.json"); return;
        case RECENTS: title_num = 18; page_type = 10; main_currpos = 0; break;
        case FAVORITES: title_num = 1; page_type = 2; main_currpos = 1; break;
        case GAMES: title_num = 2; page_type = 1; page_size = 8; main_currpos = 2; break;
        case EXPERT: title_num = 0; page_type = 16; page_size = 9; main_currpos = 3; break;
        case APPS: title_num = 107; page_type = 3; page_size = 4; main_currpos = 4; break;
        default: return;
    }

    int main_total = 6;
    if (!settings.show_recents) {
        if (main_currpos > 0)
            main_currpos--;
        main_total--;
    }
    if (!settings.show_expert) {
        if (state == APPS)
            main_currpos--;
        main_total--;
    }

    if (main_currpos + 4 > main_total)
        main_page_start = main_total - 4;
    main_page_end = main_page_start + 3;

    if (currpos + page_size > total)
        page_start = total - page_size;
    else
        page_start = currpos;
    page_end = page_start + page_size - 1;

    sprintf(state_str, "{\"list\":[{\"title\":132,\"type\":0,\"currpos\":%d,\"pagestart\":%d,\"pageend\":%d},{\"title\":%d,\"type\":%d,\"currpos\":%d,\"pagestart\":%d,\"pageend\":%d}]}", main_currpos, main_page_start, main_page_end, title_num, page_type, currpos, page_start, page_end);

    file_put_sync(fp, "/tmp/state.json", "%s", state_str);
}

//
//    [onion] get recent filename from content_history.lpl
//
char* history_getRecentPath(char *filename) {
    file_parseKeyValue(HISTORY_PATH, "path", filename, ':', 0);
    if (*filename == 0) return NULL;
    return filename;
}

char* history_getRecentCommand(char *RACommand, int index) {
    char rom_path[STR_MAX],
         core_path[STR_MAX];
    
    file_parseKeyValue(HISTORY_PATH, "path", rom_path, ':', index);
    if (*rom_path == 0) return NULL;

    file_parseKeyValue(HISTORY_PATH, "core_path", core_path, ':', index);
    if (*core_path == 0) return NULL;

    snprintf(RACommand, STR_MAX * 3, "LD_PRELOAD=/mnt/SDCARD/miyoo/lib/libpadsp.so ./retroarch -v -L \"%s\" \"%s\"", core_path, rom_path);

    return RACommand;
}

void run_resumeGame(void)
{
    FILE *fp;
    char RACommand[STR_MAX * 3];
    remove("/mnt/SDCARD/.tmp_update/.runGameSwitcher");
    if (history_getRecentCommand(RACommand, 0) != NULL)
        file_put_sync(fp, CMD_TO_RUN_PATH, "%s", RACommand);
    printf_debug("resume game: %s\n", RACommand);
}

void run_quickSwitch(void)
{
    FILE *fp;
    char RACommand[STR_MAX * 3];
    remove("/mnt/SDCARD/.tmp_update/.runGameSwitcher");
    if (history_getRecentCommand(RACommand, 1) != NULL) {
        file_put(fp, CMD_TO_RUN_PATH, "%s", RACommand);
        temp_flag_set("quick_switch", true);
        sync();
        printf_debug("quick switch: %s\n", RACommand);
    }
}

#endif // SYSTEM_STATE_H__
