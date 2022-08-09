#include <stdio.h>
#include <unistd.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>  
#include <sys/stat.h>
#include <dirent.h>
#include <SDL/SDL.h>
#include <SDL/SDL_image.h>
#include <SDL/SDL_ttf.h>

#include "utils/utils.h"
#include "utils/msleep.h"
#include "utils/log.h"

#define TIMEOUT_S 60

int main(int argc, char *argv[])
{
	// The percentage to start at
	int start_at = 0;
	// The amount of progress constituting 100% of sub-install
	int total_offset = 100;
	// The initial message - if `/tmp/.update_msg` isn't found
	char message_str[STR_MAX] = "Preparing installation...";

	int i;
	for (i = 1; i < argc; i++) {
		if (strcmp(argv[i], "-b") == 0 || strcmp(argv[i], "--begin") == 0)
			start_at = atoi(argv[++i]);
		else if (strcmp(argv[i], "-t") == 0 || strcmp(argv[i], "--total") == 0)
			total_offset = atoi(argv[++i]);
		else if (strcmp(argv[i], "-m") == 0 || strcmp(argv[i], "--message") == 0)
			strncpy(message_str, argv[++i], STR_MAX-1);
		else {
			printf_debug("Error: Unknown argument '%s'\n", argv[i]);
			exit(1);
		}
	}

	SDL_Init(SDL_INIT_VIDEO);
	SDL_ShowCursor(SDL_DISABLE);
	TTF_Init();

	SDL_Surface* video = SDL_SetVideoMode(640, 480, 32, SDL_HWSURFACE);
	SDL_Surface* screen = SDL_CreateRGBSurface(SDL_HWSURFACE, 640, 480, 32, 0, 0, 0, 0);
	
	SDL_Surface* waiting_bg = IMG_Load("res/waitingBG.png");

	TTF_Font* font = TTF_OpenFont("/customer/app/Exo-2-Bold-Italic.ttf", 36);
	SDL_Color fg_color = {255, 255, 255, 0};

	Uint32 progress_bg = SDL_MapRGB(video->format, 29, 30, 37);
	Uint32 progress_color = SDL_MapRGB(video->format, 114, 71, 194);
	Uint32 failed_color = SDL_MapRGB(video->format, 194, 71, 71);

	SDL_Rect rectMessage = {10, 414};
	SDL_Rect rectProgress = {0, 470, 0, 10};
	SDL_Surface* message;

	bool quit = false;
	bool failed = false;
	int progress = 0;
	int progress_div = 100 / total_offset;

	SDL_Event event;
	int start = SDL_GetTicks();

	while (!quit) {
		while (SDL_PollEvent(&event)) {
			if (event.type == SDL_QUIT) {
				quit = true;
				break;
			}
		}

		int now = SDL_GetTicks();
		unsigned long seconds = 0.3;

		SDL_BlitSurface(waiting_bg, NULL, screen, NULL);
		
		if (file_exists(".installed")) {
			progress = 100;
			quit = true;
		}
		
		if (file_exists(".installFailed")) {
			sprintf(message_str, "Installation failed");
			progress = 100;
			failed = true;
			quit = true;
		}
		else if (file_exists("/tmp/.update_msg")) {
			file_readLastLine("/tmp/.update_msg", message_str);
			long n = 0;
			if (str_getLastNumber(message_str, &n))
				progress = (int)(start_at + n / progress_div);
			start = now; // reset timeout
		}
		else if (!quit && now - start > TIMEOUT_S * 1000) {
			sprintf(message_str, "The installation timed out, exiting...");
			progress = 100;
			failed = true;
			quit = true;
		}

		if (quit)
			msleep_interrupt = 1;

		rectProgress.w = 640;
		SDL_FillRect(screen, &rectProgress, progress_bg);

		if (progress > 0.0) {
			rectProgress.w = (Uint16)(6.4 * progress);
			SDL_FillRect(screen, &rectProgress, failed ? failed_color : progress_color);
		}
		
		message = TTF_RenderUTF8_Blended(font, message_str, fg_color);		
		SDL_BlitSurface(message, NULL, screen, &rectMessage);
	
		SDL_BlitSurface(screen, NULL, video, NULL); 
		SDL_Flip(video);
		
		msleep(seconds * 1000);
	}

	if (file_exists(".installed") && file_exists(".waitConfirm")) {
		quit = false;

		while (!quit) {
			while (SDL_PollEvent(&event)) {
				SDLKey key = event.key.keysym.sym;
				if (event.type == SDL_KEYUP)
					quit = true;
			}
		}

		remove(".waitConfirm");
		SDL_FillRect(screen, NULL, 0);
		SDL_BlitSurface(screen, NULL, video, NULL); 
		SDL_Flip(video);
	}
	
	SDL_FreeSurface(waiting_bg);
   	SDL_FreeSurface(message);
   	SDL_FreeSurface(screen);
   	SDL_FreeSurface(video);
    SDL_Quit();
	
    return EXIT_SUCCESS;
}
