#include <stdbool.h>
#include <SDL/SDL.h>
#include <SDL/SDL_image.h>
#include <SDL/SDL_ttf.h>

#include "utils/utils.h"
#include "utils/msleep.h"
#include "system/battery.h"
#include "system/settings.h"
#include "theme/theme.h"
#include "theme/background.h"

void drawInfoPanel(SDL_Surface *screen, SDL_Surface *video, const char *title_str, const char *message_str)
{
	SDL_BlitSurface(theme_background(), NULL, screen, NULL);
	SDL_BlitSurface(screen, NULL, video, NULL); 
	SDL_Flip(video);

	bool has_title = strlen(title_str) > 0;
	bool has_message = strlen(message_str) > 0;

	SDL_Surface *message = NULL;
	SDL_Rect message_rect = {320, 240};

	theme_renderHeader(screen, battery_getPercentage(), has_title ? title_str : NULL, !has_title);
	theme_renderFooter(screen);

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

int main(int argc, char *argv[])
{
	char title_str[STR_MAX] = "";
	char message_str[STR_MAX] = "";
	char image_path[STR_MAX] = "";
	bool wait_confirm = true;

	for (int i = 1; i < argc; i++) {
		if (argv[i][0] == '-') {
			if (strcmp(argv[i], "-t") == 0 || strcmp(argv[i], "--title") == 0)
				strncpy(title_str, argv[++i], STR_MAX-1);
			else if (strcmp(argv[i], "-m") == 0 || strcmp(argv[i], "--message") == 0)
				strncpy(message_str, argv[++i], STR_MAX-1);
			else if (strcmp(argv[i], "-i") == 0 || strcmp(argv[i], "--image") == 0)
				strncpy(image_path, argv[++i], STR_MAX-1);
			else if (strcmp(argv[i], "-a") == 0 || strcmp(argv[i], "--auto") == 0)
				wait_confirm = false;
		}
	}

	SDL_Init(SDL_INIT_VIDEO);
	SDL_ShowCursor(SDL_DISABLE);
	TTF_Init();
	
	SDL_Surface *video = SDL_SetVideoMode(640, 480, 32, SDL_HWSURFACE);
	SDL_Surface *screen = SDL_CreateRGBSurface(SDL_HWSURFACE, 640, 480, 32, 0, 0, 0, 0);

	if (exists(image_path)) {
		SDL_Surface *image = IMG_Load(image_path);
		SDL_Rect image_rect = {320 - image->w / 2, 240 - image->h / 2};
		SDL_BlitSurface(image, NULL, screen, &image_rect);
		SDL_FreeSurface(image);
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
				quit = true;
		}
	}

	if (!wait_confirm)
		msleep(2000);

	SDL_FillRect(screen, NULL, 0);
	SDL_BlitSurface(screen, NULL, video, NULL);
	SDL_Flip(video);

	msleep(100);

   	SDL_FreeSurface(screen);
   	SDL_FreeSurface(video);
    SDL_Quit();
	
    return EXIT_SUCCESS;
}
