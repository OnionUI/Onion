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

#include "utils/log.h"
#include "system/battery.h"
#include "system/settings.h"
#include "system/display.h"
#include "system/keymap_hw.h"
#include "system/keymap_sw.h"
#include "system/lang.h"
#include "theme/theme.h"
#include "theme/background.h"
#include "components/list.h"

#define FRAMES_PER_SECOND 30
#define CHECK_BATTERY_TIMEOUT 30000 //ms
#define SHUTDOWN_TIMEOUT 500

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

	int pargc = 0;
	char **pargs = NULL;
	char title_str[STR_MAX] = "";
	char message_str[STR_MAX] = "";
	bool required = false;

	pargs = malloc(100 * sizeof(char*));

	int i;
	for (i = 1; i < argc; i++) {
		if (argv[i][0] == '-') {
			if (strcmp(argv[i], "-t") == 0 || strcmp(argv[i], "--title") == 0) {
				strncpy(title_str, argv[++i], STR_MAX-1);
				continue;
			}
			if (strcmp(argv[i], "-m") == 0 || strcmp(argv[i], "--message") == 0) {
				strncpy(message_str, argv[++i], STR_MAX-1);
				continue;
			}
			if (strcmp(argv[i], "-r") == 0 || strcmp(argv[i], "--required") == 0) {
				required = true;
				continue;
			}
		}
		pargs[pargc] = malloc((STR_MAX + 1) * sizeof(char));
		strncpy(pargs[pargc], argv[i], STR_MAX);
		pargc++;
	}

	printf_debug(LOG_SUCCESS, "parsed command line arguments");

	SDL_Init(SDL_INIT_VIDEO);
	SDL_ShowCursor(SDL_DISABLE);
	SDL_EnableKeyRepeat(300, 50);
	TTF_Init();

	SDL_Surface* video = SDL_SetVideoMode(640, 480, 32, SDL_HWSURFACE);
	SDL_Surface* screen = SDL_CreateRGBSurface(SDL_HWSURFACE, 640, 480, 32, 0, 0, 0, 0);

	print_debug("Loading settings...");
	settings_load();
	printf_debug(LOG_SUCCESS, "loaded settings");

	print_debug("Loading language file...");
	lang_load();
	printf_debug(LOG_SUCCESS, "loaded language file");

	print_debug("Loading theme config...");
	Theme_s theme = theme_loadFromPath(settings.theme);
	printf_debug(LOG_SUCCESS, "loaded theme config");

	theme_backgroundLoad(&theme);
	SDL_BlitSurface(theme_background, NULL, screen, NULL);
	SDL_BlitSurface(screen, NULL, video, NULL); 
	SDL_Flip(video);

	ThemeImages res_requests[RES_MAX_REQUESTS] = RESOURCES;
	Resources_s res = theme_loadResources(&theme, res_requests);

	print_debug("Reading battery percentage...");
    int current_percentage = battery_getPercentage();
	int old_percentage = current_percentage;
	printf_debug(LOG_SUCCESS, "read battery percentage");
    SDL_Surface* battery = theme_batterySurface(&theme, &res, current_percentage);

	if (pargc == 0) {
		pargs[pargc++] = lang_get(LANG_OK);
		pargs[pargc++] = lang_get(LANG_CANCEL);
	}

	List list = list_create(pargc, LIST_SMALL);

	for (i = 0; i < pargc; i++) {
		ListItem item = { .action = i };
		strcpy(item.label, pargs[i]);
		list_addItem(&list, item);
	}

	bool has_title = strlen(title_str) > 0;

	SDL_Surface *message = NULL;
	SDL_Rect message_rect;
	bool has_message = strlen(message_str) > 0;

	if (has_message) {
		char *str = str_replace(message_str, "\\n", "\n");
		message = theme_textboxSurface(&theme, str, res.fonts.title, theme.grid.color, ALIGN_CENTER);
		list.scroll_height = 3;
		message_rect.x = 320 - message->w / 2;
		message_rect.y = 60 + (6 - list.scroll_height) * 30 - message->h / 2;
		free(str);
	}

	bool quit = false;
	bool changed = true;
	bool first_draw = true;

	SDL_Event event;
	Uint8 keystate[320] = {0};
	int	input_fd;
	input_fd = open("/dev/input/event0", O_RDONLY);
	struct input_event ev;
	
	int return_code = -1;
	time_t battery_last_modified = 0;

	uint32_t shutdown_timer = 0,
			 acc_ticks = 0,
			 last_ticks = SDL_GetTicks(),
			 time_step = 1000 / FRAMES_PER_SECOND;

	while (!quit) {
		uint32_t ticks = SDL_GetTicks(),
				 delta = ticks - last_ticks;
		acc_ticks += delta;
		last_ticks = ticks;

		if (!first_draw) {
			read(input_fd, &ev, sizeof(ev));
			int val = ev.value;

			if (ev.type == EV_KEY && val <= 2 && ev.code == HW_BTN_POWER) {
				if (val == 2 && (ticks - shutdown_timer) > SHUTDOWN_TIMEOUT)
					quit = true;
				else if (val == 1)
					shutdown_timer = ticks;
			}
		}

		while (SDL_PollEvent(&event)) {
			if (event.type == SDL_QUIT)
				quit = true;

			SDLKey key = event.key.keysym.sym;
			bool repeating = false;
			
			if (event.type == SDL_KEYDOWN) {
				if (keystate[key] == 1)
					repeating = true;
				keystate[key] = 1;
				switch (key) {
					case SW_BTN_UP:
						list_moveUp(&list, repeating);
						changed = true;
						break;
					case SW_BTN_DOWN:
						list_moveDown(&list, repeating);
						changed = true;
						break;
					default: break;
				}
			}
			else if (event.type == SDL_KEYUP) {
				switch (key) {
					case SW_BTN_A:
						return_code = list_applyAction(&list);
						quit = true;
						break;
					case SW_BTN_B:
						if (!required)
							quit = true;
						break;
					default: break;
				}
				keystate[key] = 0;
			}
		}

		if (quit)
			break;

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
				SDL_BlitSurface(theme_background, NULL, screen, NULL);
				theme_renderHeader(&theme, &res, screen, battery, has_title ? title_str : NULL, !has_title);

				if (has_message)
					SDL_BlitSurface(message, NULL, screen, &message_rect);

				theme_renderList(&theme, &res, screen, &list);
				theme_renderListFooter(&theme, &res, screen, list.active_pos + 1, list.item_count, lang_get(LANG_SELECT), required ? NULL : lang_get(LANG_BACK));
			
				SDL_BlitSurface(screen, NULL, video, NULL); 
				SDL_Flip(video);

				changed = false;
				first_draw = false;
			}

			acc_ticks -= time_step;
		}
	}

	// Clear the screen when exiting
	SDL_FillRect(video, NULL, 0);
	SDL_Flip(video);
	
	lang_free();
	free(pargs);
	list_free(&list);
	theme_freeResources(&res);
	if (has_message)
		SDL_FreeSurface(message);
	theme_backgroundFree();
	SDL_FreeSurface(battery);
   	SDL_FreeSurface(screen);
   	SDL_FreeSurface(video);
    SDL_Quit();
	
    return return_code;
}
