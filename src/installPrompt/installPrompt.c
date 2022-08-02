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

#include "../common/theme.h"
#include "../common/theme_resources.h"
#include "../common/menu.h"
#include "../common/rotate180.h"

int main(void)
{
	SDL_Init(SDL_INIT_VIDEO);
	SDL_ShowCursor(SDL_DISABLE);
	TTF_Init();

	SDL_Surface* video = SDL_SetVideoMode(640, 480, 32, SDL_HWSURFACE);
	SDL_Surface* screen = SDL_CreateRGBSurface(SDL_HWSURFACE, 640, 480, 32, 0, 0, 0, 0);

	Theme_s theme = loadThemeFromPath("/mnt/SDCARD/Themes/Blueprint by Aemiii91");
	Resources_s resources = {
		.surfaces = {
			.background = rotate180(theme_loadImage(&theme, "background")),
			.bg_title = theme_loadImage(&theme, "bg-title"),
			.logo = theme_loadImage(&theme, "miyoo-topbar")
		},
		.rects = {}
	};
	Theme_Surfaces_s* surfaces = &resources.surfaces;
	Theme_Rects_s* rects = &resources.rects;
	
	rects->logo.x = 20;
	rects->logo.y = (60 - surfaces->logo->h) / 2;

	TTF_Font* font = TTF_OpenFont("/customer/app/Exo-2-Bold-Italic.ttf", 36);
	SDL_Color color_white = {255, 255, 255, 0};
	Uint32 progress_color = SDL_MapRGB(video->format, 114, 71, 194);

	SDL_Rect message_rect = {10, 424, 603, 48};
	SDL_Rect progress_rect = {0, 420, 0, 60};
	SDL_Surface* message;

	bool exit = false;
	SDL_Event event;

	while (!exit) {
		while (SDL_PollEvent(&event)) {
			if (event.type == SDL_QUIT) {
				exit = true;
				break;
			}
		}

		SDL_BlitSurface(surfaces->background, NULL, screen, NULL);
		SDL_BlitSurface(surfaces->bg_title, NULL, screen, NULL);
		SDL_BlitSurface(surfaces->logo, NULL, screen, &rects->logo);
		
		message = TTF_RenderUTF8_Blended(font, "", color_white);		
		SDL_BlitSurface(message, NULL, screen, &message_rect);
	
		SDL_BlitSurface(screen, NULL, video, NULL); 
		SDL_Flip(video);
	}
	
	theme_freeResources(&resources);
   	SDL_FreeSurface(message);
   	SDL_FreeSurface(screen);
   	SDL_FreeSurface(video);
    SDL_Quit();
	
    return EXIT_SUCCESS;
}
