#ifndef TWEAKS_MENUS_H__
#define TWEAKS_MENUS_H__

#include <stdio.h>
#include <unistd.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <SDL/SDL_image.h>

#include "components/list.h"

#include "./appstate.h"
#include "./formatters.h"
#include "./values.h"
#include "./actions.h"
#include "./tools.h"
#include "./reset.h"

static List _menu_main;
static List _menu_system;
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

void menu_free_all(void)
{
	list_free(&_menu_main);
	list_free(&_menu_system);
	list_free(&_menu_system_startup);
	list_free(&_menu_button_action);
	list_free(&_menu_button_action_mainui_menu);
	list_free(&_menu_button_action_ingame_menu);
	list_free(&_menu_user_interface);
	list_free(&_menu_theme_overrides);
	list_free(&_menu_battery_percentage);
	list_free(&_menu_advanced);
	list_free(&_menu_reset_settings);
	list_free(&_menu_tools);
}

void menu_systemStartup(void *_)
{
	if (!_menu_system_startup._created) {
		_menu_system_startup = list_create(4, LIST_SMALL);
		strcpy(_menu_system_startup.title, "Startup");
		list_addItem(&_menu_system_startup, (ListItem){
			.label = "Auto-resume last game",
			.item_type = TOGGLE,
			.value = (int)settings.startup_auto_resume,
			.action = action_setStartupAutoResume
		});
		list_addItem(&_menu_system_startup, (ListItem){
			.label = "Start application",
			.item_type = MULTIVALUE,
			.value_max = 2,
			.value_labels = {"MainUI", "GameSwitcher", "RetroArch"},
			.value = settings.startup_application,
			.action = action_setStartupApplication
		});
		list_addItem(&_menu_system_startup, (ListItem){
			.label = "MainUI: Start tab",
			.item_type = MULTIVALUE,
			.value_max = 5,
			.value_formatter = formatter_startupTab,
			.value = settings.startup_tab,
			.action = action_setStartupTab
		});
		list_addItem(&_menu_system_startup, (ListItem){
			.label = "Emulated time skip",
			.item_type = MULTIVALUE,
			.value_max = 24,
			.value_formatter = formatter_timeSkip,
			.value = settings.time_skip,
			.action = action_setTimeSkip
		});
	}
	menu_stack[++menu_level] = &_menu_system_startup;
	header_changed = true;
}

void menu_system(void *_)
{
	if (!_menu_system._created) {
		_menu_system = list_create(3, LIST_SMALL);
		strcpy(_menu_system.title, "System");
		list_addItem(&_menu_system, (ListItem){
			.label = "Startup...",
			.action = menu_systemStartup
		});
		list_addItem(&_menu_system, (ListItem){
			.label = "Save and exit when battery <4%",
			.item_type = TOGGLE,
			.value = (int)settings.low_battery_autosave,
			.action = action_setLowBatteryAutoSave
		});
		list_addItem(&_menu_system, (ListItem){
			.label = "Vibration intensity",
			.item_type = MULTIVALUE,
			.value_max = 3,
			.value_labels = {"Off", "Low", "Normal", "High"},
			.value = settings.vibration,
			.action = action_setVibration
		});
	}
	menu_stack[++menu_level] = &_menu_system;
	header_changed = true;
}

void menu_buttonActionMainUIMenu(void *_)
{
	if (!_menu_button_action_mainui_menu._created) {
		_menu_button_action_mainui_menu = list_create(3, LIST_SMALL);
		strcpy(_menu_button_action_mainui_menu.title, "MainUI: Menu button");
		list_addItem(&_menu_button_action_mainui_menu, (ListItem){
			.label = "Single press",
			.item_type = MULTIVALUE,
			.value_max = 2,
			.value_labels = BUTTON_MAINUI_LABELS,
			.value = settings.mainui_single_press,
			.action_id = 0,
			.action = action_setMenuButtonKeymap
		});
		list_addItem(&_menu_button_action_mainui_menu, (ListItem){
			.label = "Long press",
			.item_type = MULTIVALUE,
			.value_max = 2,
			.value_labels = BUTTON_MAINUI_LABELS,
			.value = settings.mainui_long_press,
			.action_id = 1,
			.action = action_setMenuButtonKeymap
		});
		list_addItem(&_menu_button_action_mainui_menu, (ListItem){
			.label = "Double press",
			.item_type = MULTIVALUE,
			.value_max = 2,
			.value_labels = BUTTON_MAINUI_LABELS,
			.value = settings.mainui_double_press,
			.action_id = 2,
			.action = action_setMenuButtonKeymap
		});
	}
	menu_stack[++menu_level] = &_menu_button_action_mainui_menu;
	header_changed = true;
}

void menu_buttonActionInGameMenu(void *_)
{
	if (!_menu_button_action_ingame_menu._created) {
		_menu_button_action_ingame_menu = list_create(3, LIST_SMALL);
		strcpy(_menu_button_action_ingame_menu.title, "In-game: Menu button");
		list_addItem(&_menu_button_action_ingame_menu, (ListItem){
			.label = "Single press",
			.item_type = MULTIVALUE,
			.value_max = 3,
			.value_labels = BUTTON_INGAME_LABELS,
			.value = settings.ingame_single_press,
			.action_id = 3,
			.action = action_setMenuButtonKeymap
		});
		list_addItem(&_menu_button_action_ingame_menu, (ListItem){
			.label = "Long press",
			.item_type = MULTIVALUE,
			.value_max = 3,
			.value_labels = BUTTON_INGAME_LABELS,
			.value = settings.ingame_long_press,
			.action_id = 4,
			.action = action_setMenuButtonKeymap
		});
		list_addItem(&_menu_button_action_ingame_menu, (ListItem){
			.label = "Double press",
			.item_type = MULTIVALUE,
			.value_max = 3,
			.value_labels = BUTTON_INGAME_LABELS,
			.value = settings.ingame_double_press,
			.action_id = 5,
			.action = action_setMenuButtonKeymap
		});
	}
	menu_stack[++menu_level] = &_menu_button_action_ingame_menu;
	header_changed = true;
}

void menu_buttonAction(void *_)
{
	if (!_menu_button_action._created) {
		_menu_button_action = list_create(5, LIST_SMALL);
		strcpy(_menu_button_action.title, "Button shortcuts");
		list_addItem(&_menu_button_action, (ListItem){
			.label = "Menu single press vibration",
			.item_type = TOGGLE,
			.value = (int)settings.menu_button_haptics,
			.action = action_setMenuButtonHaptics
		});
		list_addItem(&_menu_button_action, (ListItem){
			.label = "In-game: Menu button...",
			.action = menu_buttonActionInGameMenu
		});
		list_addItem(&_menu_button_action, (ListItem){
			.label = "MainUI: Menu button...",
			.action = menu_buttonActionMainUIMenu
		});

		getInstalledApps();
		list_addItem(&_menu_button_action, (ListItem){
			.label = "MainUI: X button",
			.item_type = MULTIVALUE,
			.value_max = installed_apps_count + NUM_TOOLS + 1,
			.value = value_appShortcut(0),
			.value_formatter = formatter_appShortcut,
			.action_id = 0,
			.action = action_setAppShortcut
		});
		list_addItem(&_menu_button_action, (ListItem){
			.label = "MainUI: Y button",
			.item_type = MULTIVALUE,
			.value_max = installed_apps_count + NUM_TOOLS + 1,
			.value = value_appShortcut(1),
			.value_formatter = formatter_appShortcut,
			.action_id = 1,
			.action = action_setAppShortcut
		});
	}
	menu_stack[++menu_level] = &_menu_button_action;
	header_changed = true;
}

void menu_batteryPercentage(void *_)
{
	if (!_menu_battery_percentage._created) {
		_menu_battery_percentage = list_create(6, LIST_SMALL);
		strcpy(_menu_battery_percentage.title, "Battery percentage");
		list_addItem(&_menu_battery_percentage, (ListItem){
			.label = "Visible",
			.item_type = MULTIVALUE,
			.value_max = 2,
			.value_labels = THEME_TOGGLE_LABELS,
			.value = value_batteryPercentageVisible(),
			.action = action_batteryPercentageVisible
		});
		list_addItem(&_menu_battery_percentage, (ListItem){
			.label = "Font family",
			.item_type = MULTIVALUE,
			.value_max = num_font_families,
			.value_formatter = formatter_fontFamily,
			.value = value_batteryPercentageFontFamily(),
			.action = action_batteryPercentageFontFamily
		});
		list_addItem(&_menu_battery_percentage, (ListItem){
			.label = "Font size",
			.item_type = MULTIVALUE,
			.value_max = num_font_sizes,
			.value_formatter = formatter_fontSize,
			.value = value_batteryPercentageFontSize(),
			.action = action_batteryPercentageFontSize
		});
		list_addItem(&_menu_battery_percentage, (ListItem){
			.label = "Position",
			.item_type = MULTIVALUE,
			.value_max = 2,
			.value_labels = {"-", "Left", "Right"},
			.value = value_batteryPercentagePosition(),
			.action = action_batteryPercentagePosition
		});
		list_addItem(&_menu_battery_percentage, (ListItem){
			.label = "Horizontal offset",
			.item_type = MULTIVALUE,
			.value_max = 21,
			.value_formatter = formatter_positionOffset,
			.value = value_batteryPercentageOffsetX(),
			.action = action_batteryPercentageOffsetX
		});
		list_addItem(&_menu_battery_percentage, (ListItem){
			.label = "Vertical offset",
			.item_type = MULTIVALUE,
			.value_max = 21,
			.value_formatter = formatter_positionOffset,
			.value = value_batteryPercentageOffsetY(),
			.action = action_batteryPercentageOffsetY
		});
	}
	menu_stack[++menu_level] = &_menu_battery_percentage;
	header_changed = true;
}

void menu_themeOverrides(void *_)
{
	if (!_menu_theme_overrides._created) {
		_menu_theme_overrides = list_create(7, LIST_SMALL);
		strcpy(_menu_theme_overrides.title, "Theme overrides");
		list_addItem(&_menu_theme_overrides, (ListItem){
			.label = "Battery percentage...",
			.action = menu_batteryPercentage
		});
		list_addItem(&_menu_theme_overrides, (ListItem){
			.label = "Hide icon labels",
			.item_type = MULTIVALUE,
			.value_max = 2,
			.value_labels = THEME_TOGGLE_LABELS,
			.value = value_hideLabelsIcons(),
			.action = action_hideLabelsIcons
		});
		list_addItem(&_menu_theme_overrides, (ListItem){
			.label = "Hide hint labels",
			.item_type = MULTIVALUE,
			.value_max = 2,
			.value_labels = THEME_TOGGLE_LABELS,
			.value = value_hideLabelsHints(),
			.action = action_hideLabelsHints
		});
		// list_addItem(&_menu_theme_overrides, (ListItem){
		// 	.label = "[Title] Font size", .item_type = MULTIVALUE, .value_max = num_font_sizes, .value_formatter = formatter_fontSize
		// });
		// list_addItem(&_menu_theme_overrides, (ListItem){
		// 	.label = "[List] Font size", .item_type = MULTIVALUE, .value_max = num_font_sizes, .value_formatter = formatter_fontSize
		// });
		// list_addItem(&_menu_theme_overrides, (ListItem){
		// 	.label = "[Hint] Font size", .item_type = MULTIVALUE, .value_max = num_font_sizes, .value_formatter = formatter_fontSize
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
		list_addItem(&_menu_user_interface, (ListItem){
			.label = "Show recents",
			.item_type = TOGGLE,
			.value = settings.show_recents,
			.action = action_setShowRecents
		});
		list_addItem(&_menu_user_interface, (ListItem){
			.label = "Show expert mode",
			.item_type = TOGGLE,
			.value = settings.show_expert,
			.action = action_setShowExpert
		});
		list_addItem(&_menu_user_interface, (ListItem){
			.label = "Low battery warning",
			.item_type = MULTIVALUE,
			.value_max = 5,
			.value_formatter = formatter_battWarn,
			.value = settings.low_battery_warn_at / 5,
			.action = action_setLowBatteryWarnAt
		});
		list_addItem(&_menu_user_interface, (ListItem){
			.label = "Theme overrides...",
			.action = menu_themeOverrides
		});
	}
	menu_stack[++menu_level] = &_menu_user_interface;
	header_changed = true;
}

void menu_resetSettings(void *_)
{
	if (!_menu_reset_settings._created) {
		_menu_reset_settings = list_create(6, LIST_SMALL);
		strcpy(_menu_reset_settings.title, "Reset settings");
		list_addItem(&_menu_reset_settings, (ListItem){
			.label = "Reset tweaks",
			.action = action_resetTweaks
		});
		list_addItem(&_menu_reset_settings, (ListItem){
			.label = "Reset theme overrides",
			.action = action_resetThemeOverrides
		});
		list_addItem(&_menu_reset_settings, (ListItem){
			.label = "Reset MainUI settings",
			.action = action_resetMainUI
		});
		list_addItem(&_menu_reset_settings, (ListItem){
			.label = "Reset RetroArch main configuration",
			.action = action_resetRAMain
		});
		list_addItem(&_menu_reset_settings, (ListItem){
			.label = "Reset all RetroArch core overrides",
			.action = action_resetRACores
		});
		list_addItem(&_menu_reset_settings, (ListItem){
			.label = "Reset everything",
			.action = action_resetAll
		});
	}
	menu_stack[++menu_level] = &_menu_reset_settings;
	header_changed = true;
}

void menu_advanced(void *_)
{
	if (!_menu_advanced._created) {
		_menu_advanced = list_create(3, LIST_SMALL);
		strcpy(_menu_advanced.title, "Advanced");
		list_addItem(&_menu_advanced, (ListItem){
			.label = "Fast forward multiplier",
			.item_type = MULTIVALUE,
			.value_max = 50,
			.value = value_getFrameThrottle(),
			.value_formatter = formatter_fastForward,
			.action = action_advancedSetFrameThrottle
		});
		list_addItem(&_menu_advanced, (ListItem){
			.label = "Swap triggers (L<>L2, R<>R2)",
			.item_type = TOGGLE,
			.value = value_getSwapTriggers(),
			.action = action_advancedSetSwapTriggers
		});
		if (exists(RESET_CONFIGS_PAK)) {
			list_addItem(&_menu_advanced, (ListItem){
				.label = "Reset settings...", .action = menu_resetSettings
			});
		}
	}
	menu_stack[++menu_level] = &_menu_advanced;
	header_changed = true;
}

void menu_tools(void *_)
{
	if (!_menu_tools._created) {
		_menu_tools = list_create(5, LIST_SMALL);
		strcpy(_menu_tools.title, "Tools");
		list_addItem(&_menu_tools, (ListItem){
			.label = "Favorites: Sort alphabetically",
			.action = tool_favoritesSortAlpha
		});
		list_addItem(&_menu_tools, (ListItem){
			.label = "Favorites: Sort by system",
			.action = tool_favoritesSortSystem
		});
		list_addItem(&_menu_tools, (ListItem){
			.label = "Favorites: Fix thumbnails and duplicates",
			.action = tool_favoritesFix
		});
		list_addItem(&_menu_tools, (ListItem){
			.label = "Remove apps from recents",
			.action = tool_recentsRemoveApps
		});
		list_addItem(&_menu_tools, (ListItem){
			.label = "Remove OSX system files",
			.action = tool_removeMacFiles
		});
	}
	menu_stack[++menu_level] = &_menu_tools;
	header_changed = true;
}

void menu_main(void)
{
	if (!_menu_main._created) {
		_menu_main = list_create(5, LIST_LARGE);
		strcpy(_menu_main.title, "Tweaks");
		list_addItem(&_menu_main, (ListItem){
			.label = "System",
			.description = "Startup, save and exit, vibration",
			.action = menu_system,
			.icon_ptr = (void*)IMG_Load("res/tweaks_system.png")
		});
		list_addItem(&_menu_main, (ListItem){
			.label = "Button shortcuts",
			.description = "Customize global button actions",
			.action = menu_buttonAction,
			.icon_ptr = (void*)IMG_Load("res/tweaks_menu_button.png")
		});
		list_addItem(&_menu_main, (ListItem){
			.label = "User interface",
			.description = "Extra menus, low batt. warn., theme",
			.action = menu_userInterface,
			.icon_ptr = (void*)IMG_Load("res/tweaks_user_interface.png")
		});
		list_addItem(&_menu_main, (ListItem){
			.label = "Advanced",
			.description = "Emulator tweaks, reset settings",
			.action = menu_advanced,
			.icon_ptr = (void*)IMG_Load("res/tweaks_advanced.png")
		});
		list_addItem(&_menu_main, (ListItem){
			.label = "Tools",
			.description = "Favorites, clean files",
			.action = menu_tools,
			.icon_ptr = (void*)IMG_Load("res/tweaks_tools.png")
		});
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
