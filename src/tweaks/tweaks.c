#include <stdio.h>
#include <unistd.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <sys/stat.h>
#include <dirent.h>
#include <linux/input.h>

#include "utils/sdl_init.h"
#include "utils/file.h"
#include "utils/log.h"
#include "utils/keystate.h"
#include "system/battery.h"
#include "system/settings.h"
#include "system/display.h"
#include "system/keymap_sw.h"
#include "system/lang.h"
#include "theme/theme.h"
#include "theme/background.h"
#include "theme/sound.h"
#include "components/list.h"

#include "./appstate.h"
#include "./menus.h"

#define FRAMES_PER_SECOND 60

int main(int argc, char *argv[])
{
	print_debug("Debug logging enabled");

	char apply_tool[STR_MAX] = "";
	bool use_display = true;

	for (int i = 1; i < argc; i++) {
		if (strcmp(argv[i], "--apply_tool") == 0)
			strncpy(apply_tool, argv[++i], STR_MAX - 1);
		else if (strcmp(argv[i], "--no_display") == 0)
			use_display = false;
	}
	
    signal(SIGINT, sigHandler);
    signal(SIGTERM, sigHandler);

	if (use_display || strlen(apply_tool) == 0)
		SDL_InitDefault(true);

	settings_load();
	lang_load();

	// Apply tool via command line
	if (strlen(apply_tool) > 0) {
		for (int i = 0; i < NUM_TOOLS; i++) {
			if (strncmp(tools_short_names[i], apply_tool, STR_MAX - 1) == 0) {
				printf("Tool '%s':\n", tools_short_names[i]);
				(*tools_pt[i])(NULL);
				break;
			}
		}
		return 0;
	}

    int battery_percentage = battery_getPercentage();

	menu_main();

	uint32_t acc_ticks = 0,
			 last_ticks = SDL_GetTicks(),
			 time_step = 1000 / FRAMES_PER_SECOND;

	bool menu_combo_pressed = false;
	bool key_changed = false;
	SDLKey changed_key;

	while (!quit) {
		uint32_t ticks = SDL_GetTicks();
		acc_ticks += ticks - last_ticks;
		last_ticks = ticks;

		if (updateKeystate(keystate, &quit, keys_enabled, &changed_key)) {
			if (keystate[SW_BTN_MENU] >= PRESSED && changed_key != SW_BTN_MENU)
				menu_combo_pressed = true;

			if (keystate[SW_BTN_UP] >= PRESSED) {
				key_changed = list_keyUp(menu_stack[menu_level], keystate[SW_BTN_UP] == REPEATING);
			}
			else if (keystate[SW_BTN_DOWN] >= PRESSED) {
				key_changed = list_keyDown(menu_stack[menu_level], keystate[SW_BTN_DOWN] == REPEATING);
			}
			else if (keystate[SW_BTN_LEFT] >= PRESSED) {
				key_changed = list_keyLeft(menu_stack[menu_level], keystate[SW_BTN_LEFT] == REPEATING);
			}
			else if (keystate[SW_BTN_RIGHT] >= PRESSED) {
				key_changed = list_keyRight(menu_stack[menu_level], keystate[SW_BTN_RIGHT] == REPEATING);
			}
			else if (keystate[SW_BTN_Y] == PRESSED) {
				key_changed = list_resetCurrentItem(menu_stack[menu_level]);
			}
			else if (keystate[SW_BTN_A] == PRESSED) {
				if (list_currentItem(menu_stack[menu_level])->action != NULL) {
					sound_change();
					skip_next_change = true;
				}
				key_changed = list_activateItem(menu_stack[menu_level]) || header_changed;
			}
			else if (changed_key == SW_BTN_MENU && keystate[SW_BTN_MENU] == RELEASED) {
				if (!menu_combo_pressed)
					quit = true;
				menu_combo_pressed = false;
			}
			else if (keystate[SW_BTN_B] == PRESSED) {
				if (menu_level == 0)
					quit = true;
				else {
					menu_stack[menu_level] = NULL;
					menu_level--;
					header_changed = true;
					key_changed = true;
				}
			}

			if (!skip_next_change) {
				if (key_changed)
					sound_change();
			}
			else skip_next_change = false;

			list_changed = list_changed || key_changed;
			key_changed = false;
		}

		if (reset_menus)
			menu_resetAll();

		if (all_changed) {
			header_changed = true;
			list_changed = true;
			footer_changed = true;
			battery_changed = true;
		}

		if (quit)
			break;

		if (battery_hasChanged(ticks, &battery_percentage))
			battery_changed = true;

		if (acc_ticks >= time_step) {
			if (header_changed || battery_changed)
				theme_renderHeader(screen, menu_stack[menu_level]->title, false);

			if (list_changed)
				theme_renderList(screen, menu_stack[menu_level]);
			
			if (footer_changed) {
				theme_renderFooter(screen);
				theme_renderStandardHint(screen, lang_get(LANG_SELECT), lang_get(LANG_BACK));
			}

			if (footer_changed || list_changed)
				theme_renderFooterStatus(screen, menu_stack[menu_level]->active_pos + 1, menu_stack[menu_level]->item_count);

			if (header_changed || battery_changed)
				theme_renderHeaderBattery(screen, battery_percentage);

			if (header_changed || list_changed || footer_changed || battery_changed) {
				SDL_BlitSurface(screen, NULL, video, NULL); 
				SDL_Flip(video);
			}

			header_changed = false;
			footer_changed = false;
			list_changed = false;
			battery_changed = false;
			all_changed = false;

			acc_ticks -= time_step;
		}
	}

	// Clear the screen when exiting
	SDL_FillRect(video, NULL, 0);
	SDL_Flip(video);

	settings_save();
	value_setFrameThrottle();
	value_setSwapTriggers();

	lang_free();
	menu_free_all();
	resources_free();
   	SDL_FreeSurface(screen);
   	SDL_FreeSurface(video);

	#ifndef PLATFORM_MIYOOMINI
	msleep(200); // to clear SDL input on quit
	#endif

    SDL_Quit();
	
    return EXIT_SUCCESS;
}
