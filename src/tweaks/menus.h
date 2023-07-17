#ifndef TWEAKS_MENUS_H__
#define TWEAKS_MENUS_H__

#include <SDL/SDL_image.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "components/list.h"
#include "system/device_model.h"
#include "system/display.h"
#include "utils/apps.h"

#include "./actions.h"
#include "./appstate.h"
#include "./formatters.h"
#include "./icons.h"
#include "./network.h"
#include "./reset.h"
#include "./tools.h"
#include "./values.h"

void menu_systemDisplay(void *_)
{
    if (!_menu_system_display._created) {
        display_init();
        _menu_system_display = list_create_with_title(1, LIST_SMALL, "Display");
        list_addItem(&_menu_system_display,
                     (ListItem){
                         .label = "OSD bar size",
                         .item_type = MULTIVALUE,
                         .value_max = 15,
                         .value_formatter = formatter_meterWidth,
                         .value = value_meterWidth(),
                         .action = action_meterWidth});
    }
    menu_stack[++menu_level] = &_menu_system_display;
    header_changed = true;
}

void menu_systemStartup(void *_)
{
    if (!_menu_system_startup._created) {
        _menu_system_startup = list_create(3, LIST_SMALL);
        strcpy(_menu_system_startup.title, "Startup");
        list_addItem(&_menu_system_startup,
                     (ListItem){
                         .label = "Auto-resume last game",
                         .item_type = TOGGLE,
                         .value = (int)settings.startup_auto_resume,
                         .action = action_setStartupAutoResume});
        list_addItem(&_menu_system_startup,
                     (ListItem){
                         .label = "Start application",
                         .item_type = MULTIVALUE,
                         .value_max = 3,
                         .value_labels = {"MainUI", "GameSwitcher", "RetroArch", "AdvanceMENU"},
                         .value = settings.startup_application,
                         .action = action_setStartupApplication});
        list_addItem(&_menu_system_startup,
                     (ListItem){
                         .label = "MainUI: Start tab",
                         .item_type = MULTIVALUE,
                         .value_max = 5,
                         .value_formatter = formatter_startupTab,
                         .value = settings.startup_tab,
                         .action = action_setStartupTab});
    }
    menu_stack[++menu_level] = &_menu_system_startup;
    header_changed = true;
}

bool _writeDateString(char *label_out)
{
    char new_label[STR_MAX];
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
    strftime(new_label, STR_MAX - 1, "Now: %Y-%m-%d %H:%M:%S", &tm);
    if (strncmp(new_label, label_out, STR_MAX) != 0) {
        strcpy(label_out, new_label);
        return true;
    }
    return false;
}

void menu_datetime(void *_)
{
    if (!_menu_date_time._created) {
        _menu_date_time = list_create(6, LIST_SMALL);
        strcpy(_menu_date_time.title, "Date and time");
        list_addItem(&_menu_date_time,
                     (ListItem){
                         .label = "[DATESTRING]",
                         .disabled = 1,
                         .action = NULL});

        network_loadState();

        if (DEVICE_ID == MIYOO354 || network_state.ntp) {
            list_addItem(&_menu_date_time,
                         (ListItem){
                             .label = "Set automatically from network",
                             .item_type = TOGGLE,
                             .value = (int)network_state.ntp,
                             .action = network_setNtpState});
        }
        if (DEVICE_ID == MIYOO354) {
            list_addItem(&_menu_date_time,
                         (ListItem){
                             .label = "Wait for NTP update (startup)",
                             .item_type = TOGGLE,
                             .disabled = !network_state.ntp,
                             .value = (int)network_state.ntp_wait,
                             .action = network_setNtpWaitState});
            list_addItem(&_menu_date_time,
                         (ListItem){
                             .label = "Get time zone via IP address",
                             .item_type = TOGGLE,
                             .disabled = !network_state.ntp,
                             .value = !network_state.manual_tz,
                             .action = network_setTzManualState});
            list_addItem(&_menu_date_time,
                         (ListItem){
                             .label = "Select time zone",
                             .item_type = MULTIVALUE,
                             .disabled = !network_state.ntp || !network_state.manual_tz,
                             .value_max = 48,
                             .value_formatter = formatter_timezone,
                             .value = value_timezone(),
                             .action = network_setTzSelectState});
        }
        list_addItem(&_menu_date_time,
                     (ListItem){
                         .label = "Emulated time skip",
                         .item_type = MULTIVALUE,
                         .disabled = network_state.ntp,
                         .value_max = 24,
                         .value_formatter = formatter_timeSkip,
                         .value = settings.time_skip,
                         .action = action_setTimeSkip});
    }
    _writeDateString(_menu_date_time.items[0].label);
    menu_stack[++menu_level] = &_menu_date_time;
    header_changed = true;
}

void menu_system(void *_)
{
    if (!_menu_system._created) {
        _menu_system = list_create(5, LIST_SMALL);
        strcpy(_menu_system.title, "System");
        list_addItem(&_menu_system,
                     (ListItem){
                         .label = "Display...",
                         .action = menu_systemDisplay});
        list_addItem(&_menu_system,
                     (ListItem){
                         .label = "Startup...",
                         .action = menu_systemStartup});
        list_addItem(&_menu_system,
                     (ListItem){
                         .label = "Date and time...",
                         .action = menu_datetime});
        list_addItem(&_menu_system,
                     (ListItem){
                         .label = "Save and exit when battery <4%",
                         .item_type = TOGGLE,
                         .value = (int)settings.low_battery_autosave,
                         .action = action_setLowBatteryAutoSave});
        list_addItem(&_menu_system,
                     (ListItem){
                         .label = "Vibration intensity",
                         .item_type = MULTIVALUE,
                         .value_max = 3,
                         .value_labels = {"Off", "Low", "Normal", "High"},
                         .value = settings.vibration,
                         .action = action_setVibration});
    }
    menu_stack[++menu_level] = &_menu_system;
    header_changed = true;
}

void menu_buttonActionMainUIMenu(void *_)
{
    if (!_menu_button_action_mainui_menu._created) {
        _menu_button_action_mainui_menu = list_create(3, LIST_SMALL);
        strcpy(_menu_button_action_mainui_menu.title, "MainUI: Menu button");
        list_addItem(&_menu_button_action_mainui_menu,
                     (ListItem){
                         .label = "Single press",
                         .item_type = MULTIVALUE,
                         .value_max = 2,
                         .value_labels = BUTTON_MAINUI_LABELS,
                         .value = settings.mainui_single_press,
                         .action_id = 0,
                         .action = action_setMenuButtonKeymap});
        list_addItem(&_menu_button_action_mainui_menu,
                     (ListItem){
                         .label = "Long press",
                         .item_type = MULTIVALUE,
                         .value_max = 2,
                         .value_labels = BUTTON_MAINUI_LABELS,
                         .value = settings.mainui_long_press,
                         .action_id = 1,
                         .action = action_setMenuButtonKeymap});
        list_addItem(&_menu_button_action_mainui_menu,
                     (ListItem){
                         .label = "Double press",
                         .item_type = MULTIVALUE,
                         .value_max = 2,
                         .value_labels = BUTTON_MAINUI_LABELS,
                         .value = settings.mainui_double_press,
                         .action_id = 2,
                         .action = action_setMenuButtonKeymap});
    }
    menu_stack[++menu_level] = &_menu_button_action_mainui_menu;
    header_changed = true;
}

void menu_buttonActionInGameMenu(void *_)
{
    if (!_menu_button_action_ingame_menu._created) {
        _menu_button_action_ingame_menu = list_create(3, LIST_SMALL);
        strcpy(_menu_button_action_ingame_menu.title, "In-game: Menu button");
        list_addItem(&_menu_button_action_ingame_menu,
                     (ListItem){
                         .label = "Single press",
                         .item_type = MULTIVALUE,
                         .value_max = 3,
                         .value_labels = BUTTON_INGAME_LABELS,
                         .value = settings.ingame_single_press,
                         .action_id = 3,
                         .action = action_setMenuButtonKeymap});
        list_addItem(&_menu_button_action_ingame_menu,
                     (ListItem){
                         .label = "Long press",
                         .item_type = MULTIVALUE,
                         .value_max = 3,
                         .value_labels = BUTTON_INGAME_LABELS,
                         .value = settings.ingame_long_press,
                         .action_id = 4,
                         .action = action_setMenuButtonKeymap});
        list_addItem(&_menu_button_action_ingame_menu,
                     (ListItem){
                         .label = "Double press",
                         .item_type = MULTIVALUE,
                         .value_max = 3,
                         .value_labels = BUTTON_INGAME_LABELS,
                         .value = settings.ingame_double_press,
                         .action_id = 5,
                         .action = action_setMenuButtonKeymap});
    }
    menu_stack[++menu_level] = &_menu_button_action_ingame_menu;
    header_changed = true;
}

void menu_buttonAction(void *_)
{
    if (!_menu_button_action._created) {
        _menu_button_action = list_create(6, LIST_SMALL);
        strcpy(_menu_button_action.title, "Button shortcuts");
        list_addItem(&_menu_button_action,
                     (ListItem){
                         .label = "Menu single press vibration",
                         .item_type = TOGGLE,
                         .value = (int)settings.menu_button_haptics,
                         .action = action_setMenuButtonHaptics});
        list_addItem(&_menu_button_action,
                     (ListItem){
                         .label = "In-game: Menu button...",
                         .action = menu_buttonActionInGameMenu});
        list_addItem(&_menu_button_action,
                     (ListItem){
                         .label = "MainUI: Menu button...",
                         .action = menu_buttonActionMainUIMenu});

        getInstalledApps(true);
        list_addItem(&_menu_button_action,
                     (ListItem){
                         .label = "MainUI: X button",
                         .item_type = MULTIVALUE,
                         .value_max = installed_apps_count + NUM_TOOLS,
                         .value = value_appShortcut(0),
                         .value_formatter = formatter_appShortcut,
                         .action_id = 0,
                         .action = action_setAppShortcut});
        list_addItem(&_menu_button_action,
                     (ListItem){
                         .label = "MainUI: Y button",
                         .item_type = MULTIVALUE,
                         .value_max = installed_apps_count + NUM_TOOLS + 1,
                         .value = value_appShortcut(1),
                         .value_formatter = formatter_appShortcut,
                         .action_id = 1,
                         .action = action_setAppShortcut});
        list_addItem(&_menu_button_action,
                     (ListItem){
                         .label = "Power single press",
                         .item_type = MULTIVALUE,
                         .value_max = 1,
                         .value_labels = {"Standby", "Shutdown"},
                         .value = (int)settings.disable_standby,
                         .action = action_setDisableStandby});
    }
    menu_stack[++menu_level] = &_menu_button_action;
    header_changed = true;
}

void menu_batteryPercentage(void *_)
{
    if (!_menu_battery_percentage._created) {
        _menu_battery_percentage = list_create(6, LIST_SMALL);
        strcpy(_menu_battery_percentage.title, "Battery percentage");
        list_addItem(&_menu_battery_percentage,
                     (ListItem){
                         .label = "Visible",
                         .item_type = MULTIVALUE,
                         .value_max = 2,
                         .value_labels = THEME_TOGGLE_LABELS,
                         .value = value_batteryPercentageVisible(),
                         .action = action_batteryPercentageVisible});
        list_addItem(&_menu_battery_percentage,
                     (ListItem){
                         .label = "Font family",
                         .item_type = MULTIVALUE,
                         .value_max = num_font_families,
                         .value_formatter = formatter_fontFamily,
                         .value = value_batteryPercentageFontFamily(),
                         .action = action_batteryPercentageFontFamily});
        list_addItem(&_menu_battery_percentage,
                     (ListItem){
                         .label = "Font size",
                         .item_type = MULTIVALUE,
                         .value_max = num_font_sizes,
                         .value_formatter = formatter_fontSize,
                         .value = value_batteryPercentageFontSize(),
                         .action = action_batteryPercentageFontSize});
        list_addItem(&_menu_battery_percentage,
                     (ListItem){
                         .label = "Position",
                         .item_type = MULTIVALUE,
                         .value_max = 2,
                         .value_labels = {"-", "Left", "Right"},
                         .value = value_batteryPercentagePosition(),
                         .action = action_batteryPercentagePosition});
        list_addItem(&_menu_battery_percentage,
                     (ListItem){
                         .label = "Horizontal offset",
                         .item_type = MULTIVALUE,
                         .value_max = 21,
                         .value_formatter = formatter_positionOffset,
                         .value = value_batteryPercentageOffsetX(),
                         .action = action_batteryPercentageOffsetX});
        list_addItem(&_menu_battery_percentage,
                     (ListItem){
                         .label = "Vertical offset",
                         .item_type = MULTIVALUE,
                         .value_max = 21,
                         .value_formatter = formatter_positionOffset,
                         .value = value_batteryPercentageOffsetY(),
                         .action = action_batteryPercentageOffsetY});
    }
    menu_stack[++menu_level] = &_menu_battery_percentage;
    header_changed = true;
}

void menu_themeOverrides(void *_)
{
    if (!_menu_theme_overrides._created) {
        _menu_theme_overrides = list_create(7, LIST_SMALL);
        strcpy(_menu_theme_overrides.title, "Theme overrides");
        list_addItem(&_menu_theme_overrides,
                     (ListItem){
                         .label = "Battery percentage...",
                         .action = menu_batteryPercentage});
        list_addItem(&_menu_theme_overrides,
                     (ListItem){
                         .label = "Hide icon labels",
                         .item_type = MULTIVALUE,
                         .value_max = 2,
                         .value_labels = THEME_TOGGLE_LABELS,
                         .value = value_hideLabelsIcons(),
                         .action = action_hideLabelsIcons});
        list_addItem(&_menu_theme_overrides,
                     (ListItem){
                         .label = "Hide hint labels",
                         .item_type = MULTIVALUE,
                         .value_max = 2,
                         .value_labels = THEME_TOGGLE_LABELS,
                         .value = value_hideLabelsHints(),
                         .action = action_hideLabelsHints});
        // list_addItem(&_menu_theme_overrides, (ListItem){
        // 	.label = "[Title] Font size", .item_type = MULTIVALUE,
        // .value_max = num_font_sizes, .value_formatter = formatter_fontSize
        // });
        // list_addItem(&_menu_theme_overrides, (ListItem){
        // 	.label = "[List] Font size", .item_type = MULTIVALUE, .value_max
        // = num_font_sizes, .value_formatter = formatter_fontSize
        // });
        // list_addItem(&_menu_theme_overrides, (ListItem){
        // 	.label = "[Hint] Font size", .item_type = MULTIVALUE, .value_max
        // = num_font_sizes, .value_formatter = formatter_fontSize
        // });
    }
    menu_stack[++menu_level] = &_menu_theme_overrides;
    header_changed = true;
}

void menu_userInterface(void *_)
{
    if (!_menu_user_interface._created) {
        _menu_user_interface = list_create(4, LIST_SMALL);
        strcpy(_menu_user_interface.title, "User interface");
        list_addItem(&_menu_user_interface,
                     (ListItem){
                         .label = "Show recents",
                         .item_type = TOGGLE,
                         .value = settings.show_recents,
                         .action = action_setShowRecents});
        list_addItem(&_menu_user_interface,
                     (ListItem){
                         .label = "Show expert mode",
                         .item_type = TOGGLE,
                         .value = settings.show_expert,
                         .action = action_setShowExpert});
        list_addItem(&_menu_user_interface,
                     (ListItem){
                         .label = "Low battery warning",
                         .item_type = MULTIVALUE,
                         .value_max = 5,
                         .value_formatter = formatter_battWarn,
                         .value = settings.low_battery_warn_at / 5,
                         .action = action_setLowBatteryWarnAt});
        list_addItem(&_menu_user_interface,
                     (ListItem){
                         .label = "Theme overrides...",
                         .action = menu_themeOverrides});
    }
    menu_stack[++menu_level] = &_menu_user_interface;
    header_changed = true;
}

void menu_resetSettings(void *_)
{
    if (!_menu_reset_settings._created) {
        _menu_reset_settings = list_create(7, LIST_SMALL);
        strcpy(_menu_reset_settings.title, "Reset settings");
        list_addItem(&_menu_reset_settings,
                     (ListItem){
                         .label = "Reset system tweaks",
                         .action = action_resetTweaks});
        list_addItem(&_menu_reset_settings,
                     (ListItem){
                         .label = "Reset theme overrides",
                         .action = action_resetThemeOverrides});
        list_addItem(&_menu_reset_settings,
                     (ListItem){
                         .label = "Reset MainUI settings",
                         .action = action_resetMainUI});
        list_addItem(&_menu_reset_settings,
                     (ListItem){
                         .label = "Reset RetroArch main configuration",
                         .action = action_resetRAMain});
        list_addItem(&_menu_reset_settings,
                     (ListItem){
                         .label = "Reset all RetroArch core overrides",
                         .action = action_resetRACores});
        list_addItem(&_menu_reset_settings,
                     (ListItem){
                         .label = "Reset AdvanceMENU/MAME/MESS",
                         .action = action_resetAdvanceMENU});
        list_addItem(&_menu_reset_settings,
                     (ListItem){
                         .label = "Reset everything", .action = action_resetAll});
    }
    menu_stack[++menu_level] = &_menu_reset_settings;
    header_changed = true;
}

void menu_advanced(void *_)
{
    if (!_menu_advanced._created) {
        _menu_advanced = list_create(4, LIST_SMALL);
        strcpy(_menu_advanced.title, "Advanced");
        list_addItem(&_menu_advanced,
                     (ListItem){
                         .label = "Swap triggers (L<>L2, R<>R2)",
                         .item_type = TOGGLE,
                         .value = value_getSwapTriggers(),
                         .action = action_advancedSetSwapTriggers});
        list_addItem(&_menu_advanced,
                     (ListItem){
                         .label = "Fast forward rate",
                         .item_type = MULTIVALUE,
                         .value_max = 50,
                         .value = value_getFrameThrottle(),
                         .value_formatter = formatter_fastForward,
                         .action = action_advancedSetFrameThrottle});
        if (DEVICE_ID == MIYOO354) {
            list_addItem(&_menu_advanced,
                         (ListItem){
                             .label = "LCD undervolt",
                             .item_type = MULTIVALUE,
                             .value_max = 4,
                             .value_labels = {"Off", "0.1V", "0.2V",
                                              "0.3V", "0.4V"},
                             .value = value_getLcdVoltage(),
                             .action = action_advancedSetLcdVoltage});
        }
        if (exists(RESET_CONFIGS_PAK)) {
            list_addItem(&_menu_advanced,
                         (ListItem){
                             .label = "Reset settings...",
                             .action = menu_resetSettings});
        }
    }
    menu_stack[++menu_level] = &_menu_advanced;
    header_changed = true;
}

void menu_tools(void *_)
{
    if (!_menu_tools._created) {
        _menu_tools = list_create(NUM_TOOLS, LIST_SMALL);
        strcpy(_menu_tools.title, "Tools");
        list_addItem(&_menu_tools,
                     (ListItem){
                         .label = "Remove OSX system files",
                         .action = tool_removeMacFiles});
        list_addItem(&_menu_tools,
                     (ListItem){
                         .label = "Generate CUE files for PSX games",
                         .action = tool_generateCueFiles});
        list_addItem(&_menu_tools,
                     (ListItem){
                         .label = "Generate game list for short name roms",
                         .action = tool_buildShortRomGameList});
    }
    menu_stack[++menu_level] = &_menu_tools;
    header_changed = true;
}

void *_get_menu_icon(const char *name)
{
    char path[STR_MAX * 2] = {0};
    const char *config_path = "/mnt/SDCARD/App/Tweaks/config.json";

    if (is_file(config_path)) {
        cJSON *config = json_load(config_path);
        char icon_path[STR_MAX];
        if (json_getString(config, "icon", icon_path))
            snprintf(path, STR_MAX * 2 - 1, "%s/%s.png", dirname(icon_path),
                     name);
    }

    if (!is_file(path))
        snprintf(path, STR_MAX * 2 - 1, "res/%s.png", name);

    return (void *)IMG_Load(path);
}

void menu_main(void)
{
    if (!_menu_main._created) {
        _menu_main = list_create(7, LIST_LARGE);
        strcpy(_menu_main.title, "Tweaks");
        list_addItem(&_menu_main,
                     (ListItem){
                         .label = "System",
                         .description = "Startup, save and exit, vibration",
                         .action = menu_system,
                         .icon_ptr = _get_menu_icon("tweaks_system")});
        if (DEVICE_ID == MIYOO354) {
            list_addItem(&_menu_main,
                         (ListItem){
                             .label = "Network",
                             .description = "Setup networking",
                             .action = menu_network,
                             .icon_ptr = _get_menu_icon("tweaks_network")});
        }
        list_addItem(&_menu_main,
                     (ListItem){
                         .label = "Button shortcuts",
                         .description = "Customize global button actions",
                         .action = menu_buttonAction,
                         .icon_ptr = _get_menu_icon("tweaks_menu_button")});
        list_addItem(&_menu_main,
                     (ListItem){
                         .label = "User interface",
                         .description = "Extra menus, low batt. warn., theme",
                         .action = menu_userInterface,
                         .icon_ptr = _get_menu_icon("tweaks_user_interface")});
        list_addItem(&_menu_main,
                     (ListItem){
                         .label = "Advanced",
                         .description = "Emulator tweaks, reset settings",
                         .action = menu_advanced,
                         .icon_ptr = _get_menu_icon("tweaks_advanced")});
        list_addItem(&_menu_main,
                     (ListItem){
                         .label = "Tools",
                         .description = "Favorites, clean files",
                         .action = menu_tools,
                         .icon_ptr = _get_menu_icon("tweaks_tools")});
        list_addItem(&_menu_main,
                     (ListItem){
                         .label = "Icons",
                         .description = "Change system icons",
                         .action = menu_icons,
                         .icon_ptr = _get_menu_icon("tweaks_icons")});
    }
    menu_level = 0;
    menu_stack[0] = &_menu_main;
    header_changed = true;
}

void menu_resetAll(void)
{
    int current_state[10][2];
    int current_level = menu_level;
    for (int i = 0; i <= current_level; i++) {
        current_state[i][0] = menu_stack[i]->active_pos;
        current_state[i][1] = menu_stack[i]->scroll_pos;
    }
    menu_free_all();
    menu_main();
    for (int i = 0; i <= current_level; i++) {
        menu_stack[i]->active_pos = current_state[i][0];
        menu_stack[i]->scroll_pos = current_state[i][1];
        if (i < current_level)
            list_activateItem(menu_stack[i]);
    }
    reset_menus = false;
}

#endif
