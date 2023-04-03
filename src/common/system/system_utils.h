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

void kill_wifi(void)
{
    FILE *fp;
    char cmd[STR_MAX * 4];
    snprintf(cmd, STR_MAX * 4 - 1,
             "pkill -9 wpa_supplicant; pkill -9 udhcpc; pkill -9 sshd;");
    file_put_sync(fp, "/tmp/cmd_to_run.sh", "%s", cmd);
}

void kill_mainUI(void)
{
    if (system_state == MODE_MAIN_UI) {
        settings_shm_read();
        temp_flag_set("mainui_killed", true);
        sync();
        kill_wifi();
        kill(system_state_pid, SIGKILL);
        display_reset();
    }
}

void run_gameSwitcher(void)
{
    flag_set("/mnt/SDCARD/.tmp_update/", ".runGameSwitcher", true);
}

#endif // SETTINGS_UTILS_H__
