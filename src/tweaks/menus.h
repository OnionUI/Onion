#ifndef TWEAKS_MENUS_H__
#define TWEAKS_MENUS_H__

#include <stdio.h>
#include <unistd.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>

#include "components/list.h"
#include "./formatters.h"

static int level = 0;
static List *menu_stack[5];

static List menu_main;
static List menu_system;
static List menu_button_action;
static List menu_user_interface;
static List menu_theme_overrides;
static List menu_advanced;
static List menu_reset_settings;
static List menu_tools;

void menuSystem(void *_)
{
	if (!menu_system._created) {
		menu_system = list_create(4, LIST_SMALL);
		strcpy(menu_system.title, "System");
		list_addItem(&menu_system, (ListItem){
			.label = "Auto-resume last game", .item_type = TOGGLE, .value = 1
		});
		list_addItem(&menu_system, (ListItem){
			.label = "Save and exit when battery <4%", .item_type = TOGGLE, .value = 1
		});
		list_addItem(&menu_system, (ListItem){
			.label = "Start application", .item_type = MULTIVALUE, .value_max = 2, .value_labels = {"MainUI", "GameSwitcher", "RetroArch"}
		});
		list_addItem(&menu_system, (ListItem){
			.label = "Vibration", .item_type = MULTIVALUE, .value_max = 3, .value_labels = {"Off", "Low", "Normal", "High"}, .value = 2
		});
	}
	menu_stack[++level] = &menu_system;
}

void menuButtonAction(void *_)
{
	if (!menu_button_action._created) {
		menu_button_action = list_create(7, LIST_SMALL);
		strcpy(menu_button_action.title, "Menu button");
		list_addItem(&menu_button_action, (ListItem){
			.label = "Vibrate on single press", .item_type = TOGGLE, .value = 1
		});
		list_addItem(&menu_button_action, (ListItem){
			.label = "[MainUI] Single press", .item_type = MULTIVALUE, .value_max = 2, .value_labels = BUTTON_MAINUI_LABELS, .value = 0
		});
		list_addItem(&menu_button_action, (ListItem){
			.label = "[MainUI] Long press", .item_type = MULTIVALUE, .value_max = 2, .value_labels = BUTTON_MAINUI_LABELS, .value = 1
		});
		list_addItem(&menu_button_action, (ListItem){
			.label = "[MainUI] Double press", .item_type = MULTIVALUE, .value_max = 2, .value_labels = BUTTON_MAINUI_LABELS, .value = 2
		});
		list_addItem(&menu_button_action, (ListItem){
			.label = "[In-game] Single press", .item_type = MULTIVALUE, .value_max = 3, .value_labels = BUTTON_INGAME_LABELS, .value = 0
		});
		list_addItem(&menu_button_action, (ListItem){
			.label = "[In-game] Long press", .item_type = MULTIVALUE, .value_max = 3, .value_labels = BUTTON_INGAME_LABELS, .value = 1
		});
		list_addItem(&menu_button_action, (ListItem){
			.label = "[In-game] Double press", .item_type = MULTIVALUE, .value_max = 3, .value_labels = BUTTON_INGAME_LABELS, .value = 2
		});
	}
	menu_stack[++level] = &menu_button_action;
}

void menuThemeOverrides(void *_)
{
	if (!menu_theme_overrides._created) {
		menu_theme_overrides = list_create(7, LIST_SMALL);
		strcpy(menu_theme_overrides.title, "Theme overrides");
		list_addItem(&menu_theme_overrides, (ListItem){
			.label = "Battery percentage", .item_type = MULTIVALUE, .value_max = 2, .value_labels = THEME_TOGGLE_LABELS
		});
		list_addItem(&menu_theme_overrides, (ListItem){
			.label = "Hide icon labels", .item_type = MULTIVALUE, .value_max = 2, .value_labels = THEME_TOGGLE_LABELS
		});
		list_addItem(&menu_theme_overrides, (ListItem){
			.label = "Font family", .item_type = MULTIVALUE, .value_max = num_font_families, .value_formatter = fontFamilyLabels
		});
		list_addItem(&menu_theme_overrides, (ListItem){
			.label = "[Title] Font size", .item_type = MULTIVALUE, .value_max = num_font_sizes, .value_formatter = fontSizeLabels
		});
		list_addItem(&menu_theme_overrides, (ListItem){
			.label = "[List] Font size", .item_type = MULTIVALUE, .value_max = num_font_sizes, .value_formatter = fontSizeLabels
		});
		list_addItem(&menu_theme_overrides, (ListItem){
			.label = "[Hint] Font size", .item_type = MULTIVALUE, .value_max = num_font_sizes, .value_formatter = fontSizeLabels
		});
		list_addItem(&menu_theme_overrides, (ListItem){
			.label = "[Battery] Font size", .item_type = MULTIVALUE, .value_max = num_font_sizes, .value_formatter = fontSizeLabels
		});
	}
	menu_stack[++level] = &menu_theme_overrides;
}

void menuUserInterface(void *_)
{
	if (!menu_user_interface._created) {
		menu_user_interface = list_create(4, LIST_SMALL);
		strcpy(menu_user_interface.title, "User interface");
		list_addItem(&menu_user_interface, (ListItem){
			.label = "Show recents", .item_type = TOGGLE
		});
		list_addItem(&menu_user_interface, (ListItem){
			.label = "Show expert mode", .item_type = TOGGLE
		});
		list_addItem(&menu_user_interface, (ListItem){
			.label = "Low battery warning", .item_type = MULTIVALUE, .value_max = 5, .value_formatter = battWarnLabels, .value = 3
		});
		list_addItem(&menu_user_interface, (ListItem){
			.label = "Theme overrides...", .action = menuThemeOverrides
		});
	}
	menu_stack[++level] = &menu_user_interface;
}

void menuResetSettings(void *_)
{
	if (!menu_reset_settings._created) {
		menu_reset_settings = list_create(5, LIST_SMALL);
		strcpy(menu_reset_settings.title, "Reset settings");
		list_addItem(&menu_reset_settings, (ListItem){
			.label = "Reset RetroArch main configuration"
		});
		list_addItem(&menu_reset_settings, (ListItem){
			.label = "Reset RetroArch core configuration..." // TODO: This needs to lead to a dynamic menu listing the cores
		});
		list_addItem(&menu_reset_settings, (ListItem){
			.label = "Reset MainUI settings"
		});
		list_addItem(&menu_reset_settings, (ListItem){
			.label = "Reset tweaks"
		});
		list_addItem(&menu_reset_settings, (ListItem){
			.label = "Reset all"
		});
	}
	menu_stack[++level] = &menu_reset_settings;
}

void menuAdvanced(void *_)
{
	if (!menu_advanced._created) {
		menu_advanced = list_create(3, LIST_SMALL);
		strcpy(menu_advanced.title, "Advanced");
		list_addItem(&menu_advanced, (ListItem){
			.label = "Fast forward multiplier", .item_type = MULTIVALUE, .value_max = 50, .value_formatter = fastForwardLabels
		});
		list_addItem(&menu_advanced, (ListItem){
			.label = "Swap L/R with L2/R2", .item_type = TOGGLE
		});
		list_addItem(&menu_advanced, (ListItem){
			.label = "Reset settings...", .action = menuResetSettings
		});
	}
	menu_stack[++level] = &menu_advanced;
}

void menuTools(void *_)
{
	if (!menu_tools._created) {
		menu_tools = list_create(4, LIST_SMALL);
		strcpy(menu_tools.title, "Tools");
		list_addItem(&menu_tools, (ListItem){
			.label = "[Favorites] Sort alphabetically"
		});
		list_addItem(&menu_tools, (ListItem){
			.label = "[Favorites] Sort by system"
		});
		list_addItem(&menu_tools, (ListItem){
			.label = "[Favorites] Fix thumbnails"
		});
		list_addItem(&menu_tools, (ListItem){
			.label = "Remove OSX system files"
		});
	}
	menu_stack[++level] = &menu_tools;
}

void menuMain(void)
{
	if (!menu_main._created) {
		menu_main = list_create(5, LIST_LARGE);
		strcpy(menu_main.title, "Tweaks");
		list_addItem(&menu_main, (ListItem){ .label = "System", .description = "Startup, save and exit, vibration", .action = menuSystem });
		list_addItem(&menu_main, (ListItem){ .label = "Menu button", .description = "Customize menu button actions", .action = menuButtonAction });
		list_addItem(&menu_main, (ListItem){ .label = "User interface", .description = "Extra menus, low batt. warn., theme", .action = menuUserInterface });
		list_addItem(&menu_main, (ListItem){ .label = "Advanced", .description = "Emulator tweaks, reset settings", .action = menuAdvanced });
		list_addItem(&menu_main, (ListItem){ .label = "Tools", .description = "Favorites, clean files", .action = menuTools });
	}
	menu_stack[0] = &menu_main;
}

#endif
