#ifndef TWEAKS_APPSTATE_H__
#define TWEAKS_APPSTATE_H__

#include <signal.h>

#include "components/list.h"
#include "utils/keystate.h"
#include "utils/sdl_init.h"

static int menu_level = 0;
static List *menu_stack[5];

bool isMenu(List *target)
{
    return menu_stack[menu_level]->_created && target->_created && menu_stack[menu_level]->_id == target->_id;
}

static List _menu_network;
static List _menu_wifi;
static List _menu_telnet;
static List _menu_ftp;
static List _menu_wps;
static List _menu_http;
static List _menu_ssh;
static List _menu_smbd;
static List _menu_vnc;

void menu_network_free_all(void)
{
    list_free(&_menu_network, NULL);
    list_free(&_menu_wifi, NULL);
    list_free(&_menu_telnet, NULL);
    list_free(&_menu_ftp, NULL);
    list_free(&_menu_wps, NULL);
    list_free(&_menu_http, NULL);
    list_free(&_menu_ssh, NULL);
    list_free(&_menu_smbd, NULL);
    list_free(&_menu_vnc, NULL);
}

static List _menu_icons;
static List _menu_icon_packs;
static List _menu_console_icons;
static List _menu_app_icons;
static List _menu_expert_icons;
static List _menu_temp;

void menu_icons_free_all(void)
{
    list_free(&_menu_icons, NULL);
    list_free(&_menu_icon_packs, NULL);
    list_free(&_menu_console_icons, NULL);
    list_free(&_menu_app_icons, NULL);
    list_free(&_menu_expert_icons, NULL);
    list_free(&_menu_temp, NULL);
}

static List _menu_main;
static List _menu_system;
static List _menu_date_time;
static List _menu_system_display;
static List _menu_user_blue_light;
static List _menu_system_startup;
static List _menu_button_action;
static List _menu_button_action_mainui_menu;
static List _menu_button_action_ingame_menu;
static List _menu_user_interface;
static List _menu_theme_overrides;
static List _menu_battery_percentage;
static List _menu_advanced;
static List _menu_reset_settings;
static List _menu_tools;
static List _menu_tools_m3uGenerator;
static List _menu_diagnostics;
static List _menu_screen_recorder;
static List _menu_user_blue_light;

void menu_free_all(void)
{
    list_free(&_menu_main, NULL);
    list_free(&_menu_system, NULL);
    list_free(&_menu_date_time, NULL);
    list_free(&_menu_system_display, NULL);
    list_free(&_menu_system_startup, NULL);
    list_free(&_menu_button_action, NULL);
    list_free(&_menu_button_action_mainui_menu, NULL);
    list_free(&_menu_button_action_ingame_menu, NULL);
    list_free(&_menu_user_interface, NULL);
    list_free(&_menu_theme_overrides, NULL);
    list_free(&_menu_battery_percentage, NULL);
    list_free(&_menu_advanced, NULL);
    list_free(&_menu_reset_settings, NULL);
    list_free(&_menu_tools, NULL);
    list_free(&_menu_tools_m3uGenerator, NULL);
    list_free(&_menu_diagnostics, NULL);
    list_free(&_menu_screen_recorder, NULL);
    list_free(&_menu_user_blue_light, NULL);

    menu_icons_free_all();
    menu_network_free_all();
}

static bool quit = false;
static bool all_changed = true;
static bool header_changed = true;
static bool list_changed = true;
static bool footer_changed = true;
static bool battery_changed = true;
static KeyState keystate[320] = {(KeyState)0};
static bool keys_enabled = true;
static bool reset_menus = false;
static bool skip_next_change = false;
static bool blf_changing = false;
static bool prev_blf_changing = false;

static bool _disable_confirm = false;
static SDL_Surface *background_cache = NULL;

static char ip_address_label[STR_MAX];

static void sigHandler(int sig)
{
    switch (sig) {
    case SIGINT:
    case SIGTERM:
        quit = true;
        break;
    default:
        break;
    }
}

#endif // TWEAKS_APPSTATE_H__