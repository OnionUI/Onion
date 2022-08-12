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
#include "system/screenshot.h"
#include "theme/theme.h"
#include "theme/background.h"
#include "components/list.h"

#define FRAMES_PER_SECOND 60

#define RESOURCES { \
	TR_BG_TITLE, \
	TR_LOGO, \
	TR_BATTERY_0, \
	TR_BATTERY_20, \
	TR_BATTERY_50, \
	TR_BATTERY_80, \
	TR_BATTERY_100, \
	TR_BATTERY_CHARGING, \
	TR_BG_LIST_S, \
	TR_BG_LIST_L, \
	TR_HORIZONTAL_DIVIDER, \
	TR_TOGGLE_ON, \
	TR_TOGGLE_OFF, \
	TR_BG_FOOTER, \
	TR_BUTTON_A, \
	TR_BUTTON_B \
}

int main(int argc, char *argv[])
{
	print_debug("Debug logging enabled");

	SDL_Init(SDL_INIT_VIDEO);
	SDL_ShowCursor(SDL_DISABLE);
	SDL_EnableKeyRepeat(300, 50);
	TTF_Init();

	SDL_Surface* video = SDL_SetVideoMode(640, 480, 32, SDL_HWSURFACE);
	SDL_Surface* screen = SDL_CreateRGBSurface(SDL_HWSURFACE, 640, 480, 32, 0, 0, 0, 0);
	SDL_SetAlpha(screen, 0, 0);

	settings_load();
	lang_load();

	char theme_path[STR_MAX];
	theme_getPath(theme_path);
	printf_debug("Theme: %s\n", theme_path);

	Theme_s theme = theme_loadFromPath(settings.theme);
	theme_backgroundLoad(settings.theme);
	theme_backgroundBlit(video);
	SDL_Flip(video);

	ThemeImages res_requests[RES_MAX_REQUESTS] = RESOURCES;
	Resources_s res = theme_loadResources(&theme, res_requests);

	time_t battery_last_modified = 0;
    int current_percentage = battery_getPercentage();
	int old_percentage = current_percentage;
    SDL_Surface* battery = theme_batterySurface(&theme, &res, current_percentage);

	print_debug("Creating list...");
	List list = list_create(10, LIST_LARGE);

	for (int i = 0; i < 10; i++) {
		ListItem item = {.is_toggle = true};
		sprintf(item.label, "List item %d", i + 1);
		sprintf(item.description, "This will enable setting #%d", i + 1);
		list_addItem(&list, item);
	}
	printf_debug(LOG_SUCCESS, "created list");

	bool quit = false;
	bool changed = true;
	KeyState keystate[320] = {0};
	bool keychanged = false;

	uint32_t acc_ticks = 0,
			 last_ticks = SDL_GetTicks(),
			 time_step = 1000 / FRAMES_PER_SECOND;

	while (!quit) {
		uint32_t ticks = SDL_GetTicks();
		acc_ticks += ticks - last_ticks;
		last_ticks = ticks;

		if ((keychanged = updateKeystate(keystate, &quit))) {
			if (keystate[SW_BTN_UP] != RELEASED)
				list_moveUp(&list, keystate[SW_BTN_UP] == REPEATING);
			else if (keystate[SW_BTN_DOWN] != RELEASED)
				list_moveDown(&list, keystate[SW_BTN_DOWN] == REPEATING);
			else if (keystate[SW_BTN_A] == PRESSED)
				list_applyAction(&list);
			else if (keystate[SW_BTN_B] == PRESSED)
				quit = true;
		}

		if (quit)
			break;

		if (keychanged)
			changed = true;

		if (file_isModified("/tmp/percBat", &battery_last_modified)) {
			current_percentage = battery_getPercentage();

			if (current_percentage != old_percentage) {
				SDL_FreeSurface(battery);
				battery = theme_batterySurface(&theme, &res, current_percentage);
				old_percentage = current_percentage;
				changed = true;
			}
		}

		if (acc_ticks >= time_step) {
			if (changed) {
				theme_backgroundBlit(screen);
				theme_renderHeader(&theme, &res, screen, battery, "Tweaks", false);

				theme_renderList(&theme, &res, screen, &list);
				theme_renderListFooter(&theme, &res, screen, list.active_pos + 1, list.item_count, lang_get(LANG_SELECT), lang_get(LANG_BACK));
			
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
	list_free(&list);
	theme_freeResources(&res);
	theme_backgroundFree();
	SDL_FreeSurface(battery);
   	SDL_FreeSurface(screen);
   	SDL_FreeSurface(video);
    SDL_Quit();
	
    return EXIT_SUCCESS;
}
