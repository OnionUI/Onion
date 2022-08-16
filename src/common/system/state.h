#ifndef SYSTEM_STATE_H__
#define SYSTEM_STATE_H__

#include "utils/utils.h"
#include "utils/flags.h"
#include "utils/process.h"
#include "./display.h"

#define CMD_TO_RUN_PATH "/mnt/SDCARD/.tmp_update/cmd_to_run.sh"
#define RETROARCH_CONFIG "/mnt/SDCARD/RetroArch/.retroarch/retroarch.cfg"

typedef enum { MODE_UNKNOWN, MODE_MAIN_UI, MODE_SWITCHER, MODE_GAME, MODE_APPS } MenuMode;

//
//    [onion] Check retroarch running & savestate_auto_save in retroarch.cfg is true
//
int check_autosave(void)
{
    char value[STR_MAX];
    file_parseKeyValue(RETROARCH_CONFIG, "savestate_auto_save", value, '=');
    return strcmp(value, "true") == 0;
}

bool check_gameActive(void)
{
    if (!exists(CMD_TO_RUN_PATH))
        return false;
    const char *cmd = file_read(CMD_TO_RUN_PATH);
    return strstr(cmd, "retroarch") != NULL || strstr(cmd, "/mnt/SDCARD/Emu/") != NULL || strstr(cmd, "/mnt/SDCARD/RApp/") != NULL;
}

bool check_isMainUI(void)
{
    return !exists(CMD_TO_RUN_PATH) && process_isRunning("MainUI");
}

bool check_isGameSwitcher(void)
{
    return exists("/mnt/SDCARD/.tmp_update/.runGameSwitcher") && process_isRunning("gameSwitcher");
}

void run_gameSwitcher(bool enabled)
{
    flag_set("/mnt/SDCARD/.tmp_update/", ".runGameSwitcher", enabled);
}

void force_gameSwitcher(void)
{
    pid_t pid;
    if ((pid = process_searchpid("MainUI"))) {
        run_gameSwitcher(true);
        kill(pid, SIGKILL);
        display_reset();
    }
}

MenuMode state_getMenuMode(void)
{
    if (check_gameActive())
        return MODE_GAME;
    if (check_isMainUI())
        return MODE_MAIN_UI;
    if (check_isGameSwitcher())
        return MODE_SWITCHER;
    return MODE_APPS;
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

#endif // SYSTEM_STATE_H__
