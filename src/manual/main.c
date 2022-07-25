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

int main(void) {
	SDL_Init(SDL_INIT_VIDEO);
	SDL_ShowCursor(SDL_DISABLE);

	SDL_Surface* video = SDL_SetVideoMode(640,480, 32, SDL_HWSURFACE);
	SDL_Surface* screen = SDL_CreateRGBSurface(SDL_HWSURFACE, 640,480, 32, 0,0,0,0);

	SDL_Surface* infoPanel = IMG_Load("infoPanel.png");
	SDL_Surface* message = IMG_Load("message.png");
	SDL_Surface* messageEmpty = IMG_Load("messageEmpty.png");
	SDL_Surface* frame1 = IMG_Load("frame1.png");
	SDL_Surface* frame2 = IMG_Load("frame2.png");
	SDL_Surface* frame3 = IMG_Load("frame3.png");
	SDL_Surface* frame4 = IMG_Load("frame4.png");
	SDL_Surface* frameEmpty = IMG_Load("frameEmpty.png");
	SDL_Surface* bootScr = IMG_Load("bootScr.png");
	
	
	SDL_Rect RectMessage = { 70, 245, 498, 174};
	
	int run = 1;
	int nFrame = 0;
	int dragonPosition = -102;
	SDL_BlitSurface(infoPanel, NULL, screen, NULL);	
	SDL_BlitSurface(message, NULL, screen, &RectMessage);
	int diplayMessage = 0;
	while ((run) || (dragonPosition < 629)){
	
	
		SDL_Rect RectDragon = { 11+dragonPosition, 166,  90, 65};

		if (dragonPosition % (96*2) == 0){
			diplayMessage = 0;
			
		} else if (dragonPosition % 96 == 0){
			diplayMessage = 1;
		}

		// for a smooth animation
		if (diplayMessage == 0) {
			SDL_BlitSurface(message, NULL, screen, &RectMessage);
		}
		else{
			SDL_BlitSurface(messageEmpty, NULL, screen, &RectMessage);
		}
		



		switch ( nFrame ) {
			case 0 : 
				SDL_BlitSurface(frame1, NULL, screen, &RectDragon);
				break;
			case 1 : 
				SDL_BlitSurface(frame2, NULL, screen, &RectDragon);
				break;
			case 2 : 
				SDL_BlitSurface(frame3, NULL, screen, &RectDragon);
				break;
			case 3 : 
				SDL_BlitSurface(frame4, NULL, screen, &RectDragon);
				break;
		}
		
		
		
		SDL_BlitSurface(screen, NULL, video, NULL); 
		SDL_Flip(video);		
		
		SDL_BlitSurface(frameEmpty, NULL, screen, &RectDragon);
		
		if (dragonPosition % 6 == 0){
			nFrame ++;
		}
		
	
		
		if (nFrame > 3){
			nFrame = 0;
		}
		
		dragonPosition += 2;
		//sleep(0.05);


		SDL_Event event;
		while (SDL_PollEvent(&event)) {
			if (event.type==SDL_KEYDOWN) {
				run = 0;

				break;
			}
		}
	

		
		
	}
	SDL_BlitSurface(bootScr, NULL, screen, NULL);
	SDL_BlitSurface(screen, NULL, video, NULL); 
	SDL_Flip(video);		
	
	
   	SDL_FreeSurface(infoPanel);
   	SDL_FreeSurface(message);
   	SDL_FreeSurface(frame1);
   	SDL_FreeSurface(frame2);
   	SDL_FreeSurface(frame3);
   	SDL_FreeSurface(frame4);
   	SDL_FreeSurface(bootScr);
   	SDL_FreeSurface(messageEmpty);
   	SDL_FreeSurface(frameEmpty); 	 	

   	SDL_FreeSurface(video);
    SDL_Quit();
	
    return EXIT_SUCCESS;
}
