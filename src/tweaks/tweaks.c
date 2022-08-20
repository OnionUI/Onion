#include <stdio.h>
#include <unistd.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <sys/stat.h>
#include <dirent.h>
#include <linux/input.h>
#include <SDL/SDL.h>
#include <SDL/SDL_image.h>
#include <SDL/SDL_ttf.h>

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

	if (use_display || strlen(apply_tool) == 0) {
		SDL_Init(SDL_INIT_VIDEO);
		SDL_ShowCursor(SDL_DISABLE);
		SDL_EnableKeyRepeat(300, 50);
		TTF_Init();

		video = SDL_SetVideoMode(640, 480, 32, SDL_HWSURFACE);
		screen = SDL_CreateRGBSurface(SDL_HWSURFACE, 640, 480, 32, 0, 0, 0, 0);
	}

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

	print_debug("Got to main loop");

	while (!quit) {
		uint32_t ticks = SDL_GetTicks();
		acc_ticks += ticks - last_ticks;
		last_ticks = ticks;

		if (all_changed) {
			header_changed = true;
			list_changed = true;
			footer_changed = true;
			battery_changed = true;
		}

		if (updateKeystate(keystate, &quit, keys_enabled)) {
			if (keystate[SW_BTN_UP] >= PRESSED) {
				list_keyUp(menu_stack[menu_level], keystate[SW_BTN_UP] == REPEATING);
				list_changed = true;
			}
			else if (keystate[SW_BTN_DOWN] >= PRESSED) {
				list_keyDown(menu_stack[menu_level], keystate[SW_BTN_DOWN] == REPEATING);
				list_changed = true;
			}
			else if (keystate[SW_BTN_LEFT] >= PRESSED) {
				list_keyLeft(menu_stack[menu_level], keystate[SW_BTN_LEFT] == REPEATING);
				list_changed = true;
			}
			else if (keystate[SW_BTN_RIGHT] >= PRESSED) {
				list_keyRight(menu_stack[menu_level], keystate[SW_BTN_RIGHT] == REPEATING);
				list_changed = true;
			}
			else if (keystate[SW_BTN_Y] == PRESSED) {
				list_resetCurrentItem(menu_stack[menu_level]);
				list_changed = true;
			}
			else if (keystate[SW_BTN_A] == PRESSED) {
				list_activateItem(menu_stack[menu_level]);
				list_changed = true;
			}
			else if (keystate[SW_BTN_B] == PRESSED) {
				if (menu_level == 0)
					quit = true;
				else {
					menu_stack[menu_level] = NULL;
					menu_level--;
					header_changed = true;
					list_changed = true;
				}
			}
			else if (keystate[SW_BTN_MENU] == PRESSED)
				quit = true;
		}

		if (quit)
			break;

		if (battery_hasChanged(ticks, &battery_percentage))
			battery_changed = true;

		if (acc_ticks >= time_step) {
			if (header_changed)
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
	
	lang_free();
	list_free(&_menu_main);
	resources_free();
   	SDL_FreeSurface(screen);
   	SDL_FreeSurface(video);

	#ifndef PLATFORM_MIYOOMINI
	msleep(200); // to clear SDL input on quit
	#endif

    SDL_Quit();
	
    return EXIT_SUCCESS;
}
