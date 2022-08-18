#ifndef SYSTEM_STATE_H__
#define SYSTEM_STATE_H__

#include <unistd.h>

#include "utils/utils.h"
#include "utils/flags.h"
#include "utils/process.h"
#include "./display.h"
#include "./screenshot.h"

#define CMD_TO_RUN_PATH "/mnt/SDCARD/.tmp_update/cmd_to_run.sh"
#define RETROARCH_CONFIG "/mnt/SDCARD/RetroArch/.retroarch/retroarch.cfg"
#define HISTORY_PATH "/mnt/SDCARD/Saves/CurrentProfile/lists/content_history.lpl"

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

bool check_gameActive(void)
{
    if (!exists(CMD_TO_RUN_PATH))
        return false;
    const char *cmd = file_read(CMD_TO_RUN_PATH);
    if (strstr(cmd, "retroarch") != NULL || strstr(cmd, "/mnt/SDCARD/Emu/") != NULL || strstr(cmd, "/mnt/SDCARD/RApp/") != NULL) {
        system_state_pid = 0;
        return true;
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
    else if (check_gameActive())
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

//
//    Terminate retroarch before kill/shotdown processes to save progress
//
bool terminate_retroarch(void) {
    char fname[16];
    pid_t pid = process_searchpid("retroarch");
    if (!pid) pid = process_searchpid("ra32");

    if (pid) {
        screenshot_system();
        
        // send signal
        kill(pid, SIGCONT); usleep(100000); kill(pid, SIGTERM);
        // wait for terminate
        sprintf(fname, "/proc/%d", pid);

        uint32_t count = 20; // 4s
        while (--count && exists(fname))
            usleep(200000); // 0.2s

        return true;
    }

    return false;
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
