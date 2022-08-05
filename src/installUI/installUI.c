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

#include "utils/msleep.h"
#include "utils/utils.h"

#define TIMEOUT_S 60


int main(void)
{
	SDL_Init(SDL_INIT_VIDEO);
	SDL_ShowCursor(SDL_DISABLE);
	TTF_Init();

	SDL_Surface* video = SDL_SetVideoMode(640, 480, 32, SDL_HWSURFACE);
	SDL_Surface* screen = SDL_CreateRGBSurface(SDL_HWSURFACE, 640, 480, 32, 0, 0, 0, 0);
	
	SDL_Surface* waiting_bg = IMG_Load("res/waitingBG.png");

	TTF_Font* font = TTF_OpenFont("/customer/app/Exo-2-Bold-Italic.ttf", 36);
	SDL_Color color_white = {255, 255, 255, 0};
	Uint32 progress_bg = SDL_MapRGB(video->format, 29, 30, 37);
	Uint32 progress_color = SDL_MapRGB(video->format, 114, 71, 194);
	SDL_Rect rectMessage = {10, 414};
	SDL_Rect rectProgress = {0, 470, 0, 10};
	SDL_Surface* surfaceMessage;

	bool exit = false;
	SDL_Event event;
	int start = SDL_GetTicks();
	char msg[MAX_LEN] = "Preparing installation...";
	double progress = 0.0;

	while (!exit) {
		while (SDL_PollEvent(&event)) {
			if (event.type == SDL_QUIT) {
				exit = true;
				break;
			}
		}

		int now = SDL_GetTicks();
		unsigned long seconds = 0.3;

		SDL_BlitSurface(waiting_bg, NULL, screen, NULL);
		
		if (file_exists(".installed")) {
			progress = 1.0;
			exit = true;
		}
		
		if (file_exists(".installFailed")) {
			sprintf(msg, "Installation failed");
			progress = 0.0;
			exit = true;
		}
		else if (file_exists("/tmp/.update_msg")) {
			file_readLastLine("/tmp/.update_msg", msg);
			long n = 0;
			if (str_getLastNumber(msg, &n))
				progress = (double)n / 100;
			start = now; // reset timeout
		}
		else if (now - start > TIMEOUT_S * 1000) {
			sprintf(msg, "The installation timed out, exiting...");
			progress = 0.0;
			exit = true;
		}

		if (exit)
			seconds = 2;

		rectProgress.w = 640;
		SDL_FillRect(screen, &rectProgress, progress_bg);

		if (progress > 0.0) {
			rectProgress.w = 640 * progress;
			SDL_FillRect(screen, &rectProgress, progress_color);
		}
		
		surfaceMessage = TTF_RenderUTF8_Blended(font, msg, color_white);		
		SDL_BlitSurface(surfaceMessage, NULL, screen, &rectMessage);
	
		SDL_BlitSurface(screen, NULL, video, NULL); 
		SDL_Flip(video);
		
		msleep(seconds * 1000);
	}
	
	SDL_FreeSurface(waiting_bg);
   	SDL_FreeSurface(surfaceMessage);
   	SDL_FreeSurface(screen);
   	SDL_FreeSurface(video);
    SDL_Quit();
	
    return EXIT_SUCCESS;
}
