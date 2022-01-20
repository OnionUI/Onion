#include <stdio.h>
#include <unistd.h>
#include <stdint.h>
#include <SDL/SDL.h>
#include <SDL/SDL_ttf.h>
#include "gfx.h"

//
//	PrintStr Main
//
int main(int argc, char* argv[]) {
	TTF_Font*	font;
	SDL_Color	color={255,155,58,0};
	SDL_Surface*	screen;
	SDL_Surface*	image;
	SDL_Rect	rect;
	int16_t		height, center_y;

	if (argc == 2) {
		SDL_Init(SDL_INIT_VIDEO);
		GFX_Init();
		TTF_Init();

		font = TTF_OpenFont("/customer/app/Exo-2-Bold-Italic.ttf", 24);
		image = TTF_RenderUTF8_Blended(font, argv[1], color);
		height = image->w * 3 / 4;
		center_y = (height-image->h)/2;
		rect = { 0, center_y, (uint16_t)image->w, (uint16_t)(center_y+image->h)};

		screen = GFX_CreateRGBSurface(SDL_SWSURFACE, image->w, height, 32, 0,0,0,0);
		GFX_ClearSurface(screen);
		SDL_BlitSurface(image, NULL, screen, &rect);
		GFX_Flip(screen);

		// Quit
		TTF_CloseFont(font);
		TTF_Quit();
		GFX_FreeSurface(screen);
		GFX_Quit();
		SDL_Quit();
	}
	return 0;
}
