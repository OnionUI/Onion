#include "stringPrinting.h"

#include <SDL/SDL.h>

#include "font_drawing.h"

#define SDL_BLACK_COLOR 0

uint16_t GetSDLColor(SDL_Surface* sdl_screen, uint8_t red, uint8_t green, uint8_t blue)
{
	return SDL_MapRGB(sdl_screen->format, red, green, blue);
}

uint16_t GetSDLWhiteColor(SDL_Surface* sdl_screen)
{
	return GetSDLColor(sdl_screen, 255, 255, 255);
}

uint16_t GetSDLDarkGrayColor(SDL_Surface* sdl_screen)
{
	return GetSDLColor(sdl_screen, 70, 70, 70);
}

void PrintString(const char *string, SDL_Surface* sdl_screen, enum Color fg_color, int x, int y)
{
	uint16_t color = SDL_BLACK_COLOR;
	switch (fg_color)
	{
	case WHITE_COLOR:
		color = GetSDLWhiteColor(sdl_screen);
		break;
	case DARK_GRAY_COLOR:
		color = GetSDLDarkGrayColor(sdl_screen);
		break;
	default:
		break;
	}
	print_string(string, color, SDL_BLACK_COLOR, x, y, sdl_screen->pixels);
}

void PrintWhiteString(const char *string, SDL_Surface* sdl_screen, int x, int y)
{
	PrintString(string, sdl_screen, WHITE_COLOR, x, y);
}