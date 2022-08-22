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
#define SHUTDOWN_TIMEOUT 500

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

	SDL_BlitSurface(theme_background(), NULL, screen, NULL);
	SDL_BlitSurface(screen, NULL, video, NULL); 
	SDL_Flip(video);

	print_debug("Reading battery percentage...");
    int battery_percentage = battery_getPercentage();
	printf_debug(LOG_SUCCESS, "read battery percentage");

	if (pargc == 0) {
		pargs[pargc++] = lang_get(LANG_OK);
		pargs[pargc++] = lang_get(LANG_CANCEL);
	}

	List list = list_create(pargc, LIST_SMALL);

	for (i = 0; i < pargc; i++) {
		ListItem item = {
			.action_id = i,
			.action = NULL
		};
		strncpy(item.label, pargs[i], STR_MAX - 1);
		printf_debug("Adding list item: %s (%d)\n", item.label, item.action_id);
		list_addItem(&list, item);
	}

	bool has_title = strlen(title_str) > 0;

	SDL_Surface *message = NULL;
	SDL_Rect message_rect;
	bool has_message = strlen(message_str) > 0;

	if (has_message) {
		char *str = str_replace(message_str, "\\n", "\n");
		printf_debug("Message: %s\n", str);
		message = theme_textboxSurface(str, resource_getFont(TITLE), theme()->grid.color, ALIGN_CENTER);
		list.scroll_height = 3;
		message_rect.x = 320 - message->w / 2;
		message_rect.y = 60 + (6 - list.scroll_height) * 30 - message->h / 2;
		free(str);
	}

	bool quit = false;
	bool list_changed = true;
	bool header_changed = true;
	bool battery_changed = true;
	bool first_draw = true;

	SDL_Event event;
	Uint8 keystate[320] = {0};
	int	input_fd;
	input_fd = open("/dev/input/event0", O_RDONLY);
	struct input_event ev;
	
	int return_code = -1;

	uint32_t shutdown_timer = 0,
			 acc_ticks = 0,
			 last_ticks = SDL_GetTicks(),
			 time_step = 1000 / FRAMES_PER_SECOND;

	while (!quit) {
		uint32_t ticks = SDL_GetTicks();
		acc_ticks += ticks - last_ticks;
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
						list_keyUp(&list, repeating);
						list_changed = true;
						break;
					case SW_BTN_DOWN:
						list_keyDown(&list, repeating);
						list_changed = true;
						break;
					default: break;
				}
			}
			else if (event.type == SDL_KEYUP) {
				switch (key) {
					case SW_BTN_A:
						return_code = list_currentItem(&list)->action_id;
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

		if (battery_hasChanged(ticks, &battery_percentage))
			battery_changed = true;

		if (acc_ticks >= time_step) {
			if (header_changed) {
				theme_renderHeader(screen, has_title ? title_str : NULL, !has_title);
			}
			if (list_changed) {
				theme_renderList(screen, &list);
				theme_renderListFooter(screen, list.active_pos + 1, list.item_count, lang_get(LANG_SELECT), required ? NULL : lang_get(LANG_BACK));

				if (has_message)
					SDL_BlitSurface(message, NULL, screen, &message_rect);
			}

			if (battery_changed)
				theme_renderHeaderBattery(screen, battery_getPercentage());

			header_changed = false;
			list_changed = false;
			battery_changed = false;
			first_draw = false;
			SDL_BlitSurface(screen, NULL, video, NULL); 
			SDL_Flip(video);

			acc_ticks -= time_step;
		}
	}

	// Clear the screen when exiting
	SDL_FillRect(video, NULL, 0);
	SDL_Flip(video);

	lang_free();
	free(pargs);
	
	print_debug("Freeing list...");
	list_free(&list);
	printf_debug(LOG_SUCCESS, "freed list");

	resources_free();

	if (has_message)
		SDL_FreeSurface(message);

   	SDL_FreeSurface(screen);
   	SDL_FreeSurface(video);
    SDL_Quit();
	
    return return_code;
}
