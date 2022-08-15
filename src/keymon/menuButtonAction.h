#ifndef MENU_BUTTON_ACTION_H__
#define MENU_BUTTON_ACTION_H__

#include <sys/time.h>

#include "system/state.h"
#include "system/settings.h"
#include "system/rumble.h"
#include "system/clock.h"

#include "./input_fd.h"

static SystemState menu_last_state = MODE_UNKNOWN;
static int menu_last_pressed = 0;
static int menu_long_press_timeout = 1000;
static bool menu_ignore_next = false;

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

void action_RA_exitToMenu(void)
{
    terminate_retroarch();
}

void action_RA_quickSwitch(void)
{
    run_quickSwitch();
    terminate_retroarch();
}

void activate_MainUI_action(int action_id)
{
    switch (action_id) {
        case 1: action_MainUI_gameSwitcher(); break;
        case 2: action_MainUI_resumeGame(); break;
        default: break;
    }
}

void activate_RA_action(int action_id)
{
    switch (action_id) {
        case 1: action_RA_gameSwitcher(); break;
        case 2: action_RA_exitToMenu(); break;
        case 3: action_RA_quickSwitch(); break;
        default: break;
    }
}

void menuButtonEvent_singlePress(void)
{
    switch (system_state) {
        case MODE_MAIN_UI: activate_MainUI_action(settings.mainui_single_press); break;
        case MODE_GAME: activate_RA_action(settings.ingame_single_press); break;
        default: break;
    }
}

void menuButtonEvent_longPress(void)
{
    switch (system_state) {
        case MODE_MAIN_UI: short_pulse(); activate_MainUI_action(settings.mainui_long_press); break;
        case MODE_GAME: if (settings.ingame_long_press != 0) short_pulse(); activate_RA_action(settings.ingame_long_press); break;
        default: break;
    }
}

void menuButtonEvent_doublePress(void)
{
    switch (system_state) {
        case MODE_MAIN_UI: activate_MainUI_action(settings.mainui_double_press); break;
        case MODE_GAME: activate_RA_action(settings.ingame_double_press); break;
        default: break;
    }
}

bool menuButtonAction(uint32_t val, bool comboKey)
{
    if (comboKey) {
        keyinput_enable();
        return val != RELEASED;
    }

    if (val == PRESSED) {
        system_state_update();
        settings_load();
        if (system_state == MODE_MAIN_UI) {
            if (settings.mainui_single_press != 0)
                keyinput_disable();
            menu_long_press_timeout = 300;
        }
        else {
            menu_long_press_timeout = 1000;
        }
        menu_last_pressed = getTicks();
        menu_last_state = system_state;
    }
    else if (val == REPEAT) {
        if (getTicks() - menu_last_pressed >= menu_long_press_timeout) {
            print_debug("LONG-PRESS");
            menuButtonEvent_longPress();
            comboKey = true;  // this will avoid to trigger short press action
        }
    }
    else if (val == RELEASED) {
        if (menu_ignore_next) {
            menu_ignore_next = false;
            return comboKey;
        }

        if (system_state == MODE_MAIN_UI || (system_state == MODE_GAME && settings.ingame_single_press != 0))
            menu_super_short_pulse();

        while(1) {
            if (poll(fds, 1, 300 - (getTicks() - menu_last_pressed)) > 0) {
                read(input_fd, &ev, sizeof(ev));

                if (ev.type != EV_KEY || ev.value >= REPEAT)
                    continue;

                print_debug("DOUBLE-PRESS");
                if (ev.code == HW_BTN_MENU) {
                    if (system_state == MODE_GAME && settings.ingame_single_press == 0 && settings.ingame_double_press != 0)
                        menu_super_short_pulse();
                    menuButtonEvent_doublePress();
                    menu_ignore_next = true;
                }
                keyinput_enable();
                break;
            }

            print_debug("SINGLE-PRESS");
            menuButtonEvent_singlePress();
            keyinput_enable();
            break;
        }
    }

    return comboKey;
}

#endif // MENU_BUTTON_ACTION_H__
