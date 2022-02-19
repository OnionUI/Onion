#include <SDL/SDL.h>
#include <SDL/SDL_image.h>
#include <SDL/SDL_ttf.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdint.h>
#include <string.h>
#include <linux/fb.h>
#include <linux/input.h>
#include <fcntl.h>
#include <stdbool.h>  
#include <sys/stat.h>  
#include "sys/ioctl.h"
#include <dirent.h>

bool file_exists (char *filename) {
  struct stat   buffer;   
  return (stat (filename, &buffer) == 0);
}


int main(void) {
	SDL_Init(SDL_INIT_VIDEO);
	SDL_ShowCursor(SDL_DISABLE);
	TTF_Init();

	SDL_Surface* video = SDL_SetVideoMode(640,480, 32, SDL_HWSURFACE);
	SDL_Surface* screen = SDL_CreateRGBSurface(SDL_HWSURFACE, 640,480, 32, 0,0,0,0);
	
	SDL_Surface* image = IMG_Load("waitingBG.png");
	

	TTF_Font* font35 = TTF_OpenFont("/customer/app/Exo-2-Bold-Italic.ttf", 35);
	SDL_Color color_white={255,255,255,0};
	SDL_Rect rectMessage = { 10, 420 , 603, 48};
	SDL_Surface* surfaceMessage ;
	int run = 1;

	while (run) {

		SDL_BlitSurface(image, NULL, screen, NULL);
		
		if (file_exists("/mnt/SDCARD/.tmp_update/.ressources/.coreInstalled") == 1){
			surfaceMessage = TTF_RenderUTF8_Blended(font35, "Core installed, cleaning ...", color_white);
			run = 0;
		}
		
		else if (file_exists("/mnt/SDCARD/.tmp_update/.ressources/.unpacked") == 1){
			surfaceMessage = TTF_RenderUTF8_Blended(font35, "Installing core ...", color_white);
		}
		else {
			surfaceMessage = TTF_RenderUTF8_Blended(font35, "Unpacking ...", color_white);
		}
		
		
		SDL_BlitSurface(surfaceMessage, NULL, screen, &rectMessage);
	
		SDL_BlitSurface(screen, NULL, video, NULL); 
		SDL_Flip(video);
		
		sleep(2);

	}
	
   	SDL_FreeSurface(screen);
   	SDL_FreeSurface(surfaceMessage);
   	SDL_FreeSurface(video);
    SDL_Quit();
	
    return EXIT_SUCCESS;
}
