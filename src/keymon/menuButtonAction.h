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

#include "./input_fd.h"

#define NUM_TOOLS 6

static char tools_short_names[NUM_TOOLS][STR_MAX] = {
    "dot_clean", "cue_gen", "favsort-az", "favsort-sys", "favfix", "recents"};
static MainUIState tools_states[NUM_TOOLS] = {FAVORITES, FAVORITES, FAVORITES,
                                              RECENTS, GAMES};

static SystemState menu_last_state = MODE_UNKNOWN;
static int menu_last_pressed = 0;
static int menu_long_press_timeout = 700;
static bool menu_ignore_next = false;

void _action_runApp(const char *app_dir_name)
{
    run_app(app_dir_name);
    write_mainui_state(MAIN_MENU, 0, 10);
    kill_mainUI();
}

void _action_runTool(const char *tool_name, MainUIState return_state)
{
    FILE *fp;
    char cmd[STR_MAX * 4];
    sprintf(cmd, "cd /mnt/SDCARD/.tmp_update; ./bin/tweaks --apply_tool \"%s\"",
            tool_name);
    file_put_sync(fp, "/tmp/cmd_to_run.sh", "%s", cmd);
    write_mainui_state(return_state, 0, 10);
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
                _action_runTool(tools_short_names[i], tools_states[i]);
                return;
            }
    }
}

//
//    Terminate retroarch before kill/shotdown processes to save progress
//
bool terminate_retroarch(void)
{
    char fname[16];
    pid_t pid = process_searchpid("retroarch");
    if (!pid)
        pid = process_searchpid("ra32");

    if (pid) {
        screenshot_system();

        // send signal
        kill(pid, SIGCONT);
        usleep(100000);
        kill(pid, SIGTERM);
        // wait for terminate
        sprintf(fname, "/proc/%d", pid);

        uint32_t count = 20; // 4s
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

void action_MainUI_contextMenu(void)
{
    print_debug("Sending keys (contextMenu)");
    keyinput_enable();
    keyinput_send(HW_BTN_MENU, RELEASED);
    // quietMainUI();    // Will be enabled again when MainUI "click" sounds will be affected to L2 instead of L1 (requires a new MainUI)
}

void action_MainUI_gameSwitcher(void)
{
    run_gameSwitcher();
    kill_mainUI();
}

void action_MainUI_resumeGame(void)
{
    run_resumeGame();
    kill_mainUI();
}

void action_RA_gameSwitcher(void)
{
    run_gameSwitcher();
    terminate_retroarch();
}

void action_RA_exitToMenu(void) { terminate_retroarch(); }

void action_RA_quickSwitch(void)
{
    run_quickSwitch();
    terminate_retroarch();
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
    default:
        break;
    }
}

void menuButtonEvent_singlePress(void)
{
    switch (system_state) {
    case MODE_MAIN_UI:
        activate_MainUI_action(settings.mainui_single_press);
        break;
    case MODE_GAME:
        activate_RA_action(settings.ingame_single_press);
        break;
    default:
        break;
    }
}

void menuButtonEvent_longPress(void)
{
    switch (system_state) {
    case MODE_MAIN_UI:
        short_pulse();
        activate_MainUI_action(settings.mainui_long_press);
        break;
    case MODE_GAME:
        if (settings.ingame_long_press != 0)
            short_pulse();
        activate_RA_action(settings.ingame_long_press);
        break;
    default:
        break;
    }
}

void menuButtonEvent_doublePress(void)
{
    switch (system_state) {
    case MODE_MAIN_UI:
        activate_MainUI_action(settings.mainui_double_press);
        break;
    case MODE_GAME:
        activate_RA_action(settings.ingame_double_press);
        break;
    default:
        break;
    }
}

bool _hapticSinglePress(void)
{
    switch (system_state) {
    case MODE_MAIN_UI:
        return settings.mainui_single_press != 0;
    case MODE_GAME:
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
    switch (system_state) {
    case MODE_MAIN_UI:
        return true;
    case MODE_GAME:
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
        quietMainUI();    // Will be enabled again when MainUI "click" sounds will be affected to L2 instead of L1 (requires a new MainUI)
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
        menu_last_state = system_state;
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
