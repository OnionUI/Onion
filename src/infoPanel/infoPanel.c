#include <stdbool.h>
#include <stdlib.h>

#include "utils/sdl_init.h"
#include "utils/utils.h"
#include "utils/msleep.h"
#include "utils/json.h"
#include "utils/file.h"
#include "system/battery.h"
#include "system/keymap_sw.h"
#include "system/settings.h"
#include "system/lang.h"
#include "theme/theme.h"
#include "theme/background.h"
#include "imagesCache.h"
#include "imagesBrowser.h"
#include "appstate.h"

#define FRAMES_PER_SECOND 60

static char **g_images_paths;
static char **g_images_titles;
static int g_images_paths_count = 0;
static int g_image_index = -1;

static bool loadImagesPathsFromJson(const char* config_path, char ***images_paths, int *images_paths_count,
	char ***images_titles)
{
	const char *json_str = NULL;

    if (!(json_str = file_read(config_path)))
	{
		return false;
	}

    // Get JSON objects
	cJSON* json_root = cJSON_Parse(json_str);
	cJSON* json_images_array = cJSON_GetObjectItem(json_root, "images");
	*images_paths_count = cJSON_GetArraySize(json_images_array);
	*images_paths = (char**)malloc(*images_paths_count * sizeof(char*));
	*images_titles = (char**)malloc(*images_paths_count * sizeof(char*));

	const int images_count = *images_paths_count;
	for (int i = 0; i < *images_paths_count; i++)
	{
		(*images_paths)[i] = (char*)malloc(STR_MAX * sizeof(char));
		static const int g_title_max_length = 50;
		(*images_titles)[i] = (char*)malloc(g_title_max_length * sizeof(char));

		const cJSON* json_image_item = cJSON_GetArrayItem(json_images_array, i);
		if (!json_image_item)
		{
			(*images_paths_count)--;
			continue;
		}
		const cJSON* json_image_path = cJSON_GetObjectItem(json_image_item, "path");
		if (!json_image_path)
		{
			(*images_paths_count)--;
			continue;
		}
		const char* image_path = cJSON_GetStringValue(json_image_path);
		strncpy((*images_paths)[i], image_path, STR_MAX);

		cJSON* json_image_title = cJSON_GetObjectItem(json_image_item, "title");
		if (!json_image_title)
		{
			continue;
		}
		char* image_title = cJSON_GetStringValue(json_image_title);
		strncpy((*images_titles)[i], image_title, g_title_max_length);
	}

	cJSON_free(json_root);

    return true;
}

static void drawInfoPanel(SDL_Surface *screen, SDL_Surface *video, const char *title_str, char *message_str)
{
	bool has_title = strlen(title_str) > 0;
	bool has_message = strlen(message_str) > 0;

	SDL_Surface *message = NULL;
	SDL_Rect message_rect = {320, 240};

	theme_renderHeader(screen, has_title ? title_str : NULL, !has_title);

	if (has_message)
	{
		const char *str = str_replace(message_str, "\\n", "\n");
		message = theme_textboxSurface(str, resource_getFont(TITLE), theme()->grid.color, ALIGN_CENTER);
		message_rect.x -= message->w / 2;
		message_rect.y -= message->h / 2;
		SDL_BlitSurface(message, NULL, screen, &message_rect);
		SDL_FreeSurface(message);
	}
}

static void drawImage(const char *image_path, SDL_Surface *screen)
{
	SDL_Surface *image = IMG_Load(image_path);
	if (image)
	{
		SDL_Rect image_rect = {320 - image->w / 2, 240 - image->h / 2};
		SDL_BlitSurface(image, NULL, screen, &image_rect);
		SDL_FreeSurface(image);
	}
}

static void sdlQuit(SDL_Surface *screen, SDL_Surface *video)
{
	SDL_FreeSurface(screen);
   	SDL_FreeSurface(video);
    SDL_Quit();
}

static const char *getFilename(const char *full_path) {
    const char *slash = strrchr(full_path, '/');
    if (!slash || slash == full_path)
	{
		return "";
	}
    return slash + 1;
}

int main(int argc, char *argv[])
{
	char title_str[STR_MAX] = "";
	char message_str[STR_MAX] = "";
	char image_path[STR_MAX] = "";
	char images_json_path[STR_MAX] = "";
	char images_dir_path[STR_MAX] = "";
	bool wait_confirm = true;
	bool show_theme_controls = false;
	bool info_panel_mode = false;

	for (int i = 1; i < argc; i++)
	{
		if (argv[i][0] == '-')
		{
			if (strcmp(argv[i], "-t") == 0 || strcmp(argv[i], "--title") == 0)
				strncpy(title_str, argv[++i], STR_MAX-1);
			else if (strcmp(argv[i], "-m") == 0 || strcmp(argv[i], "--message") == 0)
				strncpy(message_str, argv[++i], STR_MAX-1);
			else if (strcmp(argv[i], "-i") == 0 || strcmp(argv[i], "--image") == 0)
				strncpy(image_path, argv[++i], STR_MAX-1);
			else if (strcmp(argv[i], "-j") == 0 || strcmp(argv[i], "--images-json") == 0)
				strncpy(images_json_path, argv[++i], STR_MAX-1);
			else if (strcmp(argv[i], "-d") == 0 || strcmp(argv[i], "--directory") == 0)
				strncpy(images_dir_path, argv[++i], STR_MAX-1);
			else if (strcmp(argv[i], "-s") == 0 || strcmp(argv[i], "--show-theme-controls") == 0)
				show_theme_controls = true;
			else if (strcmp(argv[i], "-a") == 0 || strcmp(argv[i], "--auto") == 0)
				wait_confirm = false;
		}
	}

	signal(SIGINT, sigHandler);
    signal(SIGTERM, sigHandler);

	SDL_InitDefault(true);

	settings_load();
	lang_load();

	int battery_percentage = battery_getPercentage();

	uint32_t acc_ticks = 0;
	uint32_t last_ticks = SDL_GetTicks();
	uint32_t time_step = 1000 / FRAMES_PER_SECOND;

	bool cache_used = false;
	if (exists(image_path))
	{
		g_images_paths_count = 1;
		g_image_index = 0;
		SDL_BlitSurface(theme_background(), NULL, screen, NULL);
		drawImage(image_path, screen);
	}
	else if(exists(images_json_path))
	{
		if (loadImagesPathsFromJson(images_json_path, &g_images_paths, &g_images_paths_count, &g_images_titles)
			&& g_images_paths_count > 0)
		{
			g_image_index = 0;
			SDL_BlitSurface(theme_background(), NULL, screen, NULL);
			drawImageByIndex(0, g_image_index, g_images_paths, g_images_paths_count, screen, &cache_used);
		}
		else
		{
			sdlQuit(screen, video);
			return EXIT_FAILURE;
		}
	}
	else if (exists(images_dir_path))
	{
		if (loadImagesPathsFromDir(images_dir_path, &g_images_paths, &g_images_paths_count)
			&& g_images_paths_count > 0)
		{
			g_image_index = 0;
			SDL_BlitSurface(theme_background(), NULL, screen, NULL);
			drawImageByIndex(0, g_image_index, g_images_paths, g_images_paths_count, screen, &cache_used);
		}
		else
		{
			sdlQuit(screen, video);
			return EXIT_FAILURE;
		}
	}
	else if (strlen(title_str) > 0 && strlen(message_str) > 0)
	{
		info_panel_mode = true;
		show_theme_controls = true;
	}
	else
	{
		sdlQuit(screen, video);
		return EXIT_FAILURE;
	}

	SDL_Event event;

	while (!quit && wait_confirm)
	{
		uint32_t ticks = SDL_GetTicks();
		acc_ticks += ticks - last_ticks;
		last_ticks = ticks;

		while (SDL_PollEvent(&event))
		{
			if (event.type == SDL_KEYDOWN)
			{
				bool navigation_pressed = true;
				bool navigating_forward = true;
				const SDLKey key_pressed = event.key.keysym.sym;
				switch(key_pressed) {
				case SW_BTN_A:
				case SW_BTN_RIGHT:
					navigating_forward = true;
					break;
				case SW_BTN_B:
				case SW_BTN_LEFT:
					navigating_forward = false;
					break;
				case SW_BTN_MENU:
					quit = true;
					continue;
					break;
				case SW_BTN_Y:
					show_theme_controls = !show_theme_controls;
					SDL_BlitSurface(theme_background(), NULL, screen, NULL);
					drawImageByIndex(g_image_index, g_image_index, g_images_paths, g_images_paths_count, screen, &cache_used);
					all_changed = true;
					continue;
					break;
				default:
					navigation_pressed = false;
					break;
				}
				
				if (!navigation_pressed) 
				{
					continue;
				}
				if (navigating_forward && key_pressed == SW_BTN_RIGHT && g_image_index == g_images_paths_count - 1
					|| !navigating_forward && key_pressed == SW_BTN_LEFT && g_image_index == 0
					|| info_panel_mode && (key_pressed == SW_BTN_RIGHT || key_pressed == SW_BTN_LEFT))
				{
					continue;
				}
				if (info_panel_mode // drawing info panel
					|| (navigating_forward && g_image_index == g_images_paths_count - 1) // exit after last image
					|| (!navigating_forward && g_image_index == 0)) // or when navigating backwards from the first image
				{
					quit = true;
					continue;
				}
				const int current_index = g_image_index;
				navigating_forward ? g_image_index++ : g_image_index--;
				SDL_BlitSurface(theme_background(), NULL, screen, NULL);
				drawImageByIndex(g_image_index, current_index, g_images_paths, g_images_paths_count, screen, &cache_used);
				all_changed = true;
			}
		}

		if (show_theme_controls && all_changed) {
			header_changed = true;
			footer_changed = true;
			battery_changed = true;
		}

		if (quit)
			break;

		if (show_theme_controls)
		{
			if (battery_hasChanged(ticks, &battery_percentage))
				battery_changed = true;
		
			if (acc_ticks >= time_step)
			{
				if (header_changed || battery_changed)
				{
					if (info_panel_mode)
					{
						SDL_BlitSurface(theme_background(), NULL, screen, NULL);
						drawInfoPanel(screen, video, title_str, message_str);
					}
					else if (g_images_titles)
					{
						const char *title = g_images_titles[g_image_index];
						theme_renderHeader(screen, title, !title);
					}
					else
					{
						const char *current_image_path = image_path;
						if (g_images_paths_count > 0 && g_images_paths && g_image_index >= 0)
						{
							current_image_path = g_images_paths[g_image_index];
						}
						const char *filename = getFilename(current_image_path);
						theme_renderHeader(screen, filename, !filename);
					}
				}
				
				if (footer_changed)
				{
					theme_renderFooter(screen);
					const char * a_btn_text = lang_get(LANG_NEXT);
					const char * b_btn_text = lang_get(LANG_BACK);
					if (info_panel_mode || g_images_paths_count == 1)
					{
						a_btn_text = lang_get(LANG_OK);
						b_btn_text = NULL;
					}
					else if (g_image_index == g_images_paths_count - 1)
					{
						a_btn_text = lang_get(LANG_EXIT);
					}
					else if (g_image_index == 0)
					{
						b_btn_text = lang_get(LANG_EXIT);
					}
					theme_renderStandardHint(screen, a_btn_text, b_btn_text);
				}

				if (footer_changed && !info_panel_mode && g_images_paths_count > 1)
					theme_renderFooterStatus(screen, g_image_index + 1, g_images_paths_count);

				if (header_changed || battery_changed)
					theme_renderHeaderBattery(screen, battery_percentage);

				if (header_changed || footer_changed || battery_changed)
				{
					SDL_BlitSurface(screen, NULL, video, NULL); 
					SDL_Flip(video);
				}

				header_changed = false;
				footer_changed = false;
				battery_changed = false;
				all_changed = false;

				acc_ticks -= time_step;
			}
		}
		else
		{
			SDL_BlitSurface(screen, NULL, video, NULL); 
			SDL_Flip(video);
		}
		msleep(4);
	}

	if (g_images_paths != NULL)
	{
		for (int i = 0; i < g_images_paths_count; i++)
        	free(g_images_paths[i]);
    	free(g_images_paths);
	}
	if (g_images_titles != NULL)
	{
		for (int i = 0; i < g_images_paths_count; i++)
        	free(g_images_titles[i]);
		free(g_images_titles);
	}

	if (!wait_confirm)
		msleep(2000);

	// Clear the screen when exiting
	SDL_FillRect(video, NULL, 0);
	SDL_Flip(video);

	cleanImagesCache();

   	lang_free();
	resources_free();
   	SDL_FreeSurface(screen);
   	SDL_FreeSurface(video);

	#ifndef PLATFORM_MIYOOMINI
	msleep(200); // to clear SDL input on quit
	#endif

    SDL_Quit();
	
    return EXIT_SUCCESS;
}
