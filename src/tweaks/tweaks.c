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

#include "./menus.h"

#define FRAMES_PER_SECOND 60

int main(int argc, char *argv[])
{
	print_debug("Debug logging enabled");

	SDL_Init(SDL_INIT_VIDEO);
	SDL_ShowCursor(SDL_DISABLE);
	SDL_EnableKeyRepeat(300, 50);
	TTF_Init();

	SDL_Surface* video = SDL_SetVideoMode(640, 480, 32, SDL_HWSURFACE);
	SDL_Surface* screen = SDL_CreateRGBSurface(SDL_HWSURFACE, 640, 480, 32, 0, 0, 0, 0);
	
	theme_backgroundBlit(screen);
	SDL_BlitSurface(screen, NULL, video, NULL);
	SDL_Flip(video);

	settings_load();
	lang_load();

	time_t battery_last_modified = 0;
    int current_percentage = battery_getPercentage();
	int old_percentage = current_percentage;
    SDL_Surface* battery = theme_batterySurface(current_percentage);

	menuMain();
	bool title_changed = true;

	theme_renderFooter(screen);
    theme_renderStandardHint(screen, lang_get(LANG_SELECT), lang_get(LANG_BACK));

	bool quit = false;
	bool changed = true;
	KeyState keystate[320] = {0};

	uint32_t acc_ticks = 0,
			 last_ticks = SDL_GetTicks(),
			 time_step = 1000 / FRAMES_PER_SECOND;

	while (!quit) {
		uint32_t ticks = SDL_GetTicks();
		acc_ticks += ticks - last_ticks;
		last_ticks = ticks;

		if (updateKeystate(keystate, &quit)) {
			if (keystate[SW_BTN_UP] >= PRESSED) {
				list_keyUp(menu_stack[level], keystate[SW_BTN_UP] == REPEATING);
				changed = true;
			}
			else if (keystate[SW_BTN_DOWN] >= PRESSED) {
				list_keyDown(menu_stack[level], keystate[SW_BTN_DOWN] == REPEATING);
				changed = true;
			}
			else if (keystate[SW_BTN_LEFT] >= PRESSED) {
				list_keyLeft(menu_stack[level], keystate[SW_BTN_LEFT] == REPEATING);
				changed = true;
			}
			else if (keystate[SW_BTN_RIGHT] >= PRESSED) {
				list_keyRight(menu_stack[level], keystate[SW_BTN_RIGHT] == REPEATING);
				changed = true;
			}
			else if (keystate[SW_BTN_Y] == PRESSED) {
				list_resetCurrentItem(menu_stack[level]);
				changed = true;
			}
			else if (keystate[SW_BTN_A] == PRESSED) {
				list_activateItem(menu_stack[level]);
				title_changed = true;
				changed = true;
			}
			else if (keystate[SW_BTN_B] == PRESSED) {
				if (level == 0)
					quit = true;
				else {
					menu_stack[level] = NULL;
					level--;
					title_changed = true;
					changed = true;
				}
			}
		}

		if (quit)
			break;

		if (file_isModified("/tmp/percBat", &battery_last_modified)) {
			current_percentage = battery_getPercentage();

			if (current_percentage != old_percentage) {
				SDL_FreeSurface(battery);
				battery = theme_batterySurface(current_percentage);
				old_percentage = current_percentage;
				title_changed = true;
			}
		}

		if (acc_ticks >= time_step) {
			if (title_changed)
				theme_renderHeader(screen, battery, menu_stack[level]->title, false);

			if (changed) {
				theme_renderList(screen, menu_stack[level]);
				theme_renderFooterStatus(screen, menu_stack[level]->active_pos + 1, menu_stack[level]->item_count);
			
				SDL_BlitSurface(screen, NULL, video, NULL); 
				SDL_Flip(video);

				changed = false;
			}

			acc_ticks -= time_step;
		}
	}

	// Clear the screen when exiting
	SDL_FillRect(video, NULL, 0);
	SDL_Flip(video);
	
	lang_free();
	list_free(&menu_main);
	resources_free();
	SDL_FreeSurface(battery);
   	SDL_FreeSurface(screen);
   	SDL_FreeSurface(video);
    SDL_Quit();
	
    return EXIT_SUCCESS;
}
