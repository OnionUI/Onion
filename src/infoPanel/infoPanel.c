#include <stdbool.h>
#include <stdlib.h>
#include <SDL/SDL.h>
#include <SDL/SDL_image.h>
#include <SDL/SDL_ttf.h>

#include "utils/utils.h"
#include "utils/msleep.h"
#include "utils/json.h"
#include "utils/file.h"
#include "system/battery.h"
#include "system/keymap_sw.h"
#include "system/settings.h"
#include "theme/theme.h"
#include "theme/background.h"
#include "imagesCache.h"

static char **g_images_paths;
static int g_images_paths_count = 0;
static int g_image_index = 0;

bool loadImagesPaths(const char* config_path, char ***images_paths, int *images_paths_count)
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

	for (int i = 0; i < *images_paths_count; i++)
	{
		(*images_paths)[i] = (char*)malloc(STR_MAX * sizeof(char));

		cJSON* json_image_path_item = cJSON_GetArrayItem(json_images_array, i);
		cJSON* json_image_path = cJSON_GetObjectItem(json_image_path_item, "path");
		char* image_path = cJSON_GetStringValue(json_image_path);
		strcpy((*images_paths)[i], image_path);
	}

	cJSON_free(json_root);

    return true;
}

void drawInfoPanel(SDL_Surface *screen, SDL_Surface *video, const char *title_str, char *message_str)
{
	SDL_BlitSurface(theme_background(), NULL, screen, NULL);
	SDL_BlitSurface(screen, NULL, video, NULL); 
	SDL_Flip(video);

	bool has_title = strlen(title_str) > 0;
	bool has_message = strlen(message_str) > 0;

	SDL_Surface *message = NULL;
	SDL_Rect message_rect = {320, 240};

	theme_renderHeader(screen, has_title ? title_str : NULL, !has_title);
	theme_renderFooter(screen);
	theme_renderHeaderBattery(screen, battery_getPercentage());

	if (has_message) {
		char *str = str_replace(message_str, "\\n", "\n");
		message = theme_textboxSurface(str, resource_getFont(TITLE), theme()->grid.color, ALIGN_CENTER);
		message_rect.x -= message->w / 2;
		message_rect.y -= message->h / 2;
		SDL_BlitSurface(message, NULL, screen, &message_rect);
		SDL_FreeSurface(message);
	}

	resources_free();
}

static void drawImage(const char *image_path, SDL_Surface *screen)
{
	SDL_Surface *image = IMG_Load(image_path);
	if (image) {
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

int main(int argc, char *argv[])
{
	char title_str[STR_MAX] = "";
	char message_str[STR_MAX] = "";
	char image_path[STR_MAX] = "";
	char images_json_path[STR_MAX] = "";
	bool wait_confirm = true;

	for (int i = 1; i < argc; i++) {
		if (argv[i][0] == '-') {
			if (strcmp(argv[i], "-t") == 0 || strcmp(argv[i], "--title") == 0)
				strncpy(title_str, argv[++i], STR_MAX-1);
			else if (strcmp(argv[i], "-m") == 0 || strcmp(argv[i], "--message") == 0)
				strncpy(message_str, argv[++i], STR_MAX-1);
			else if (strcmp(argv[i], "-i") == 0 || strcmp(argv[i], "--image") == 0)
				strncpy(image_path, argv[++i], STR_MAX-1);
			else if (strcmp(argv[i], "-j") == 0 || strcmp(argv[i], "--images-json") == 0)
				strncpy(images_json_path, argv[++i], STR_MAX-1);
			else if (strcmp(argv[i], "-a") == 0 || strcmp(argv[i], "--auto") == 0)
				wait_confirm = false;
		}
	}

	SDL_Init(SDL_INIT_VIDEO);
	SDL_ShowCursor(SDL_DISABLE);
	TTF_Init();
	
	SDL_Surface *video = SDL_SetVideoMode(640, 480, 32, SDL_HWSURFACE);
	SDL_Surface *screen = SDL_CreateRGBSurface(SDL_HWSURFACE, 640, 480, 32, 0, 0, 0, 0);

	bool cache_used = false;
	if (exists(image_path)) {
		g_images_paths_count = 1;
		drawImage(image_path, screen);
	}
	else if(exists(images_json_path))
	{
		if (loadImagesPaths(images_json_path, &g_images_paths, &g_images_paths_count))
		{
			if (g_images_paths_count > 0)
			{
				drawImageByIndex(0, g_image_index, g_images_paths, g_images_paths_count, screen, &cache_used);
			}
		}
		else
		{
			sdlQuit(screen, video);
			return EXIT_FAILURE;
		}
	}
	else {
		drawInfoPanel(screen, video, title_str, message_str);
	}

	SDL_BlitSurface(screen, NULL, video, NULL);
	SDL_Flip(video);
	
	bool quit = false;
	SDL_Event event;

	while (!quit && wait_confirm) {
		while (SDL_PollEvent(&event)) {
			if (event.type == SDL_KEYDOWN)
			{
				bool navigation_pressed = true;
				bool navigating_forward = true;
				switch(event.key.keysym.sym) {
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
				default:
					navigation_pressed = false;
					break;
				}
				if (!navigation_pressed) 
				{
					continue;
				}
				if ((navigating_forward && g_image_index == g_images_paths_count - 1) // exit after last image
					|| (!navigating_forward && g_image_index == 0)) // or when navigating backwards from the first image
				{
					quit = true;
				}
				else
				{
					const int current_index = g_image_index;
					navigating_forward ? g_image_index++ : g_image_index--;
					drawImageByIndex(g_image_index, current_index, g_images_paths, g_images_paths_count, screen, &cache_used);

					SDL_BlitSurface(screen, NULL, video, NULL);
					SDL_Flip(video);
				}
			}
		}
		msleep(4);
	}

	if (g_images_paths != NULL)
	{
		for (int i = 0; i < g_images_paths_count; i++)
        	free(g_images_paths[i]);
    	free(g_images_paths);
	}

	if (!wait_confirm)
		msleep(2000);

	SDL_FillRect(screen, NULL, 0);
	SDL_BlitSurface(screen, NULL, video, NULL);
	SDL_Flip(video);

	msleep(100);

	cleanImagesCache();

   	sdlQuit(screen, video);
	
    return EXIT_SUCCESS;
}
