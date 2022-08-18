#ifndef MENU_BUTTON_ACTION_H__
#define MENU_BUTTON_ACTION_H__

#include <time.h>

#include "system/state.h"
#include "system/settings.h"
#include "system/rumble.h"
#include "system/clock.h"

#include "./input_fd.h"

static SystemState menu_last_state = MODE_UNKNOWN;
static int menu_last_pressed = 0;
static int menu_long_press_timeout = 1000;
static bool menu_ignore_next = false;

void quietMainUI(void)
{
    if (system_state == MODE_MAIN_UI) {
        print_debug("Sending L2 to quiet MainUI");
        keyinput_send(HW_BTN_L2, PRESSED);
        keyinput_send(HW_BTN_L2, RELEASED);
        print_debug("Done (quietMainUI)");
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

bool _hapticSinglePress(void)
{
    switch (system_state) {
        case MODE_MAIN_UI:
            return settings.mainui_single_press != 0;
        case MODE_GAME:
            return settings.ingame_single_press != 0;
        default: break;
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
        default: break;
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
            keyinput_enable();
            menuButtonEvent_longPress();
            comboKey = true;  // this will avoid to trigger short press action
        }
    }
    else if (val == RELEASED) {
        if (menu_ignore_next) {
            menu_ignore_next = false;
            return comboKey;
        }

        if (_hapticSinglePress())
            menu_super_short_pulse();

        while(1) {
            if (poll(fds, 1, 300 - (getTicks() - menu_last_pressed)) > 0) {
                if (!keyinput_isValid()) continue;

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
