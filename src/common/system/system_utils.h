#ifndef SETTINGS_UTILS_H__
#define SETTINGS_UTILS_H__

#include "system/settings.h"
#include "system/settings_sync.h"
#include "system/state.h"
#include "utils/file.h"

//
//    [onion] Check retroarch running & savestate_auto_save in retroarch.cfg is
//    true
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
        settings_shm_read();
        kill(system_state_pid, SIGKILL);
        display_reset();
    }
}

void set_gameSwitcher(void)
{
    flag_set("/mnt/SDCARD/.tmp_update/", ".runGameSwitcher", true);
}

#endif // SETTINGS_UTILS_H__
