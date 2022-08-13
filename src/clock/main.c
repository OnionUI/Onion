/*
Copyright (c) 2020 Gameblabla

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <SDL/SDL.h>
#include "font_drawing.h"
#include "gfx.h"
#include "system/keymap_sw.h"

SDL_Surface* sdl_screen, *screen;

char tmp_str[64];
int buttons;

uint32_t date_selected = 1, month_selected = 1, year_selected = 1970, hour_selected = 0, minute_selected = 0, seconds_selected = 0;
uint32_t february_days = 28;
uint32_t update_clock = 0;

char final_long_string[512];

uint8_t string_tmp[2][512];

int select_cursor = 0;

/* I could probably make this smaller */
static void Dont_go_over_days()
{
	if (date_selected < 1)
	{
		date_selected = 1;
	}
	
	if (month_selected > 12)
	{
		month_selected = 12;
	}
	
	if (month_selected < 1)
	{
		month_selected = 1;
	}
	
	if (year_selected > 2100)
	{
		year_selected = 2100;
	}
	
	if (year_selected < 1970)
	{
		year_selected = 1970;
	}
	
	switch(month_selected)
	{
		case 2:
			if (date_selected > february_days) date_selected = february_days;
			break;
		case 4:
		case 6:
		case 9:
		case 11:
			if (date_selected > 30) date_selected = 30;
			break;
		default:
			if (date_selected > 31) date_selected = 31;
			break;
	}
}

static void Check_leap_year()
{
	if ( ((year_selected % 4 == 0) && (year_selected % 100 != 0)) || (year_selected % 400 == 0)) february_days = 29;
	else february_days = 28;
}

static void Dont_go_over_hour()
{
	if (hour_selected > 23)
	{
		hour_selected = 0;
	}
	
	if (minute_selected > 59)
	{
		minute_selected = 0;
	}
	
	if (seconds_selected > 59)
	{
		seconds_selected = 0;
	}
}

int main (int argc, char *argv[]) 
{
	SDL_Event event;
	int quit = 0;
	SDL_Rect rectS = {0,0,213,100};
	SDL_Rect rectD = {0,120,639,240};
	
	SDL_Init(SDL_INIT_VIDEO);
	GFX_Init();
	SDL_ShowCursor(0);
	screen = GFX_CreateRGBSurface(0, 640, 480, 16, 0,0,0,0);
	sdl_screen = GFX_CreateRGBSurface(0, 213, 100, 16, 0,0,0,0);
	if ((!sdl_screen)||(!screen))
	{
		printf("Can't set video mode\n");
		return 0;
	}

	/* Actual time is retrieved */
	/* Here, we don't need to check if year is a leap year as it's being taken care 
	 * of by the system but when the user change the date, we do need to detect it.
	 * */

	time_t t = time(NULL);
	struct tm tm = *localtime(&t);
	
	date_selected = tm.tm_mday;
	month_selected = tm.tm_mon + 1;
	year_selected = tm.tm_year + 1900;
	hour_selected = tm.tm_hour;
	minute_selected = tm.tm_min;
	seconds_selected = tm.tm_sec;
	
	SDL_EnableKeyRepeat(500, 50);
	
	while(!quit)
	{
		while (SDL_PollEvent(&event)) 
		{
			switch(event.type) 
			{
				case SDL_KEYUP:
					switch(event.key.keysym.sym) 
					{
						/* SDLK_HOME for OD needs the Power button to be detected upon release. */
						case SDLK_HOME:
							quit = 1;
						break;
						default:
						break;
					}
				break;
				case SDL_KEYDOWN:
					switch(event.key.keysym.sym) 
					{
						case SDLK_UP:
						switch(select_cursor)
						{
							case 0:
								date_selected++;
							break;
							case 1:
								month_selected++;
							break;
							case 2:
								year_selected++;
							break;
							case 3:
								hour_selected++;
							break;
							case 4:
								minute_selected++;
							break;
							case 5:
								seconds_selected++;
							break;
						}
						break;
						case SDLK_DOWN:
						switch(select_cursor)
						{
							case 0:
								date_selected--;
							break;
							case 1:
								month_selected--;
							break;
							case 2:
								year_selected--;
							break;
							case 3:
								hour_selected--;
							break;
							case 4:
								minute_selected--;
							break;
							case 5:
								seconds_selected--;
							break;
						}
						break;
						case SDLK_LEFT:
						select_cursor--;
						if (select_cursor < 0) select_cursor = 0;
						break;
						case SDLK_RIGHT:
						select_cursor++;
						if (select_cursor > 5) select_cursor = 5;
						break;
						case SDLK_RETURN:
							quit = 1;
							update_clock = 1;
						break;
						case SW_BTN_B:
							quit = 1;
							break;
						default:
						break;
					}
				break;
				case SDL_QUIT:
					quit = 1;
				break;
			}
		}
		
		Check_leap_year();
		Dont_go_over_days();
		Dont_go_over_hour();
		
		GFX_FillRect(sdl_screen, NULL, 0);
		
		print_string("Please set the Clock", SDL_MapRGB(sdl_screen->format,255,255,255), 0, 10, 5, sdl_screen->pixels);
		
		snprintf(tmp_str, sizeof(tmp_str), "%02d/%02d/%04d %02d:%02d:%02d", date_selected, month_selected, year_selected, hour_selected, minute_selected, seconds_selected);
		print_string(tmp_str, SDL_MapRGB(sdl_screen->format,255,255,255), 0, 26, 30, sdl_screen->pixels);
		
		if (select_cursor == 2) {
			print_string("^^^^", SDL_MapRGB(sdl_screen->format,255,255,255), 0, 26+(select_cursor * 24), 50, sdl_screen->pixels);
		} else {
			print_string("^^", SDL_MapRGB(sdl_screen->format,255,255,255), 0, 26+(select_cursor*24)+(select_cursor>2?16:0), 50, sdl_screen->pixels);
		}
		
		print_string("Start: Update time/Quit", SDL_MapRGB(sdl_screen->format,255,255,255), 0, 10, 70, sdl_screen->pixels);
		print_string("B: Quit", SDL_MapRGB(sdl_screen->format,255,255,255), 0, 10, 90, sdl_screen->pixels);
		
		/* Print back buffer to the final screen */
		SDL_SoftStretch(sdl_screen, &rectS, screen, &rectD);
		
		/* Flip the screen so it gets displayed*/
		GFX_Flip(screen);
	}
	
	/* Make sure to clean up the allocated surfaces before exiting.
	 * Most of the time when an SDL app leaks, it's for that reason.
	 * */
	if (sdl_screen)
	{
		GFX_FreeSurface(sdl_screen);
		sdl_screen = NULL;
	}
	if (screen)
	{
		GFX_FreeSurface(screen);
		screen = NULL;
	}
	
	GFX_Quit();
	SDL_Quit();
	
	if (update_clock == 1)
	{
		snprintf(final_long_string, sizeof(final_long_string), "date -s '%d-%d-%d %d:%d:%d';hwclock --utc -w", year_selected, month_selected, date_selected, hour_selected, minute_selected, seconds_selected);
		execlp("/bin/sh","/bin/sh", "-c", final_long_string, (char *)NULL);
	}
	
	return 0;
}
