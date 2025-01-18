#ifndef MENU_BUTTON_ACTION_H__
#define MENU_BUTTON_ACTION_H__

#include <time.h>

#include "system/clock.h"
#include "system/keymap_hw.h"
#include "system/rumble.h"
#include "system/screenshot.h"
#include "system/settings.h"
#include "system/state.h"
#include "system/system_utils.h"
#include "utils/apps.h"
#include "utils/flags.h"

#include "../tweaks/tools_defs.h"
#include "./input_fd.h"
#include "utils/retroarch_cmd.h"

static SystemState menu_pressed_state = MODE_UNKNOWN;
static int menu_last_pressed = 0;
static int menu_long_press_timeout = 700;
static bool menu_ignore_next = false;

void _action_runApp(const char *app_dir_name)
{
    set_cmd_app(app_dir_name);
    write_mainui_state(MAIN_MENU, 0, 10);
    kill_mainUI();
}

void _action_runTool(const char *tool_name)
{
    FILE *fp;
    char cmd[STR_MAX * 4];
    sprintf(cmd, "cd /mnt/SDCARD/.tmp_update; ./bin/tweaks --apply_tool \"%s\"",
            tool_name);
    file_put_sync(fp, "/tmp/cmd_to_run.sh", "%s", cmd);
    write_mainui_state(MAIN_MENU, 0, 10);
    kill_mainUI();
}

void applyExtraButtonShortcut(int button)
{
    if (system_state != MODE_MAIN_UI)
        return;

    int i;
    char *action =
        button == 0 ? settings.mainui_button_x : settings.mainui_button_y;
    InstalledApp *apps = getInstalledApps(false);

    if (button == 1 && strcmp(action, "glo") == 0) {
        temp_flag_set("launch_alt", true);
    }
    else if (strncmp(action, "app:", 4) == 0) {
        for (i = 0; i < installed_apps_count; i++)
            if (strcmp(action + 4, apps[i].dirName) == 0) {
                _action_runApp(apps[i].dirName);
                return;
            }
    }
    else if (strncmp(action, "tool:", 5) == 0) {
        for (i = 0; i < NUM_TOOLS; i++)
            if (strcmp(action + 5, tools_short_names[i]) == 0) {
                _action_runTool(tools_short_names[i]);
                return;
            }
    }
}

//
//    Terminate retroarch before kill/shotdown processes to save progress
//
bool terminate_retroarch(void)
{
    char fname[20];
    pid_t pid = process_searchpid("retroarch");
    if (!pid)
        pid = process_searchpid("ra32");

    if (pid) {

        // send signal
        kill(pid, SIGCONT);
        usleep(100000);
        retroarch_quit();
        // wait for terminate
        sprintf(fname, "/proc/%d", pid);

        uint32_t count = 20; // 4s
        while (--count && exists(fname))
            usleep(200000); // 0.2s
        return true;
    }
    return false;
}

//
//    Terminate drastic before kill/shotdown processes to save progress
//
bool terminate_drastic(void)
{
    pid_t pid = process_searchpid("drastic");
    char fname[20];

    if (pid) {
        // If swap L<>L2 is on, the off button combo becomes 1 + 15 instead of 1 + 18
        if (temp_flag_get("drastic_swap_l1l2")) {
            system("sendkeys 1 1, 15 1");
            usleep(200000); // 0.2s
            system("sendkeys 1 0, 15 0");
        }
        else {
            system("sendkeys 1 1, 18 1");
            usleep(200000); // 0.2s
            system("sendkeys 1 0, 18 0");
        };

        sprintf(fname, "/proc/%d", pid);
        uint32_t count = 150; // 30s

        while (--count && exists(fname))
            usleep(200000); // 0.2s
        return true;
    }
    return false;
}

void quietMainUI(void)
{
    if (system_state == MODE_MAIN_UI) {
        print_debug("Sending L1 to quiet MainUI");
        keyinput_send(HW_BTN_L1, PRESSED);
        keyinput_send(HW_BTN_L1, RELEASED);
        print_debug("Done (quietMainUI)");
    }
}

void enableSavingMessage(void)
{
    temp_flag_set(".displaySavingMessage", true);
}

void displaySavingMessage(void)
{
    if (temp_flag_get(".displaySavingMessage")) {
        temp_flag_set(".displaySavingMessage", false);
        system("infoPanel --message \"SAVING\" --persistent --romscreen &");
        temp_flag_set("dismiss_info_panel", true);
    }
}

void action_MainUI_contextMenu(void)
{
    print_debug("Sending keys (contextMenu)");
    keyinput_enable();
    keyinput_send(HW_BTN_MENU, RELEASED);
    quietMainUI();
}

void action_MainUI_gameSwitcher(void)
{
    set_gameSwitcher();
    kill_mainUI();
}

void action_MainUI_resumeGame(void)
{
    set_resumeGame();
    kill_mainUI();
}

static void _saveAndQuitRetroArch(bool quickSwitch)
{
    if (check_autosave()) {
        enableSavingMessage();
        retroarch_pause();
        screenshot_system();
        displaySavingMessage();
    }
    else {
        display_clear();
    }
    if (quickSwitch)
        set_quickSwitch();
    terminate_retroarch();
}

void action_RA_gameSwitcher(void)
{
    if (exists("/mnt/SDCARD/.tmp_update/.runGameSwitcher"))
        return;
    set_gameSwitcher();
    retroarch_pause();
    system("(gameSwitcher --overlay && touch /tmp/state_changed) &");
    system_state_update();
}

void action_RA_exitToMenu(void)
{
    _saveAndQuitRetroArch(false);
}

void action_RA_quickSwitch(void)
{
    _saveAndQuitRetroArch(true);
}

void action_RA_toggleMenu(void)
{
    retroarch_toggleMenu();
}

void action_drastic_gameSwitcher(void)
{
    enableSavingMessage();
    screenshot_system();
    set_gameSwitcher();
    terminate_drastic();
}

void action_drastic_exitToMenu(void)
{
    enableSavingMessage();
    screenshot_system();
    terminate_drastic();
}

void action_drastic_quickSwitch(void)
{
    screenshot_system();
    set_quickSwitch();
    terminate_drastic();
}

void activate_MainUI_action(int action_id)
{
    switch (action_id) {
    case 1:
        action_MainUI_gameSwitcher();
        break;
    case 2:
        action_MainUI_resumeGame();
        break;
    default:
        break;
    }
}

void activate_RA_action(int action_id)
{
    switch (action_id) {
    case 1:
        action_RA_gameSwitcher();
        break;
    case 2:
        action_RA_exitToMenu();
        break;
    case 3:
        action_RA_quickSwitch();
        break;
    case 4:
        action_RA_toggleMenu();
        break;
    default:
        break;
    }
}

void activate_drastic_action(int action_id)
{
    switch (action_id) {
    case 1:
        action_drastic_gameSwitcher();
        break;
    case 2:
        action_drastic_exitToMenu();
        break;
    case 3:
        action_drastic_quickSwitch();
        break;
    default:
        break;
    }
}

void menuButtonEvent_singlePress(void)
{
    switch (menu_pressed_state) {
    case MODE_MAIN_UI:
        activate_MainUI_action(settings.mainui_single_press);
        break;
    case MODE_GAME:
        activate_RA_action(settings.ingame_single_press);
        break;
    case MODE_DRASTIC:
        activate_drastic_action(settings.ingame_single_press);
        break;
    default:
        break;
    }
}

void menuButtonEvent_longPress(void)
{
    switch (menu_pressed_state) {
    case MODE_MAIN_UI:
        short_pulse();
        activate_MainUI_action(settings.mainui_long_press);
        break;
    case MODE_GAME:
        if (settings.ingame_long_press != 0)
            short_pulse();
        activate_RA_action(settings.ingame_long_press);
        break;
    case MODE_DRASTIC:
        if (settings.ingame_long_press != 0)
            short_pulse();
        activate_drastic_action(settings.ingame_long_press);
        break;
    default:
        break;
    }
}

void menuButtonEvent_doublePress(void)
{
    switch (menu_pressed_state) {
    case MODE_MAIN_UI:
        activate_MainUI_action(settings.mainui_double_press);
        break;
    case MODE_GAME:
        activate_RA_action(settings.ingame_double_press);
        break;
    case MODE_DRASTIC:
        activate_drastic_action(settings.ingame_double_press);
        break;
    default:
        break;
    }
}

bool _hapticSinglePress(void)
{
    switch (menu_pressed_state) {
    case MODE_MAIN_UI:
        return settings.mainui_single_press != 0;
    case MODE_GAME:
        return settings.ingame_single_press != 0;
    case MODE_DRASTIC:
        return settings.ingame_single_press != 0;
    default:
        break;
    }
    return false;
}

bool _hapticDoublePress(void)
{
    if (_hapticSinglePress())
        return false;
    switch (menu_pressed_state) {
    case MODE_MAIN_UI:
        return true;
    case MODE_GAME:
        return settings.ingame_double_press != 0;
    case MODE_DRASTIC:
        return settings.ingame_double_press != 0;
    default:
        break;
    }
    return false;
}

bool menuButtonAction(uint32_t val, bool comboKey)
{
    if (comboKey) {
        keyinput_enable();
        quietMainUI();
        return val != RELEASED;
    }

    if (val == PRESSED) {
        if (system_state == MODE_MAIN_UI) {
            if (settings.mainui_single_press != 0)
                keyinput_disable();
            menu_long_press_timeout = 300;
        }
        else {
            menu_long_press_timeout = 700;
        }
        menu_last_pressed = getMilliseconds();
        menu_pressed_state = system_state;
    }
    else if (val == REPEAT) {
        if (getMilliseconds() - menu_last_pressed >= menu_long_press_timeout) {
            print_debug("LONG-PRESS");
            keyinput_enable();
            menuButtonEvent_longPress();
            comboKey = true; // this will avoid to trigger short press action
        }
    }
    else if (val == RELEASED) {
        if (menu_ignore_next) {
            menu_ignore_next = false;
            return comboKey;
        }

        if (_hapticSinglePress())
            menu_super_short_pulse();

        while (1) {
            if (settings.ingame_double_press != 0 &&
                poll(fds, 1, 300 - (getMilliseconds() - menu_last_pressed)) >
                    0) {
                if (!keyinput_isValid())
                    continue;

                if (ev.code == HW_BTN_MENU && ev.value == PRESSED) {
                    print_debug("DOUBLE-PRESS");
                    if (_hapticDoublePress())
                        menu_super_short_pulse();
                    menuButtonEvent_doublePress();
                    menu_ignore_next = true;
                }
                break;
            }

            print_debug("SINGLE-PRESS");
            menuButtonEvent_singlePress();
            break;
        }

        keyinput_enable();
    }

    return comboKey;
}

#endif // MENU_BUTTON_ACTION_H__
