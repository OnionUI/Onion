#include <SDL/SDL.h>
#include <SDL/SDL_image.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <pthread.h>
#include <string.h>
#include <fcntl.h>
#include <assert.h>
#include <time.h>
#include "sys/ioctl.h"

static int is_charging = 0;
void checkCharging(void) {
    int i = 0;
    FILE *file = fopen("/sys/devices/gpiochip0/gpio/gpio59/value", "r");
    if (file!=NULL) {
        fscanf(file, "%i", &i);
        fclose(file);
    }
    is_charging = i;
}

void SetRawBrightness(int val) {  // val = 0-100
    int fd = open("/sys/class/pwm/pwmchip0/pwm0/duty_cycle", O_WRONLY);
    if (fd>=0) {
        dprintf(fd,"%d",val);
        close(fd);
    }
}

void SetBrightness(int value) {  // value = 0-10
    SetRawBrightness(value*10);
}

int main(void) {
	SDL_Init(SDL_INIT_VIDEO);
	SDL_ShowCursor(SDL_DISABLE);
	   
	// 640x480, HW to HW blit, 1:1 crisp pixels, rightside up
	SDL_Surface* video = SDL_SetVideoMode(640,480, 32, SDL_HWSURFACE);
	SDL_Surface* screen = SDL_CreateRGBSurface(SDL_HWSURFACE, 640,480, 32, 0,0,0,0);
	
	SDL_Surface* image0 = IMG_Load("chargingState0.png");
	SDL_Surface* image1 = IMG_Load("chargingState1.png");
	SDL_Surface* image2 = IMG_Load("chargingState2.png");
	SDL_Surface* image3 = IMG_Load("chargingState3.png");
	SDL_Surface* image4 = IMG_Load("chargingState4.png");
	SDL_Surface* image5 = IMG_Load("chargingState5.png");
	
	SetBrightness(8);
	
	for (int i = 1; i < 10; ++i){
		
		SDL_BlitSurface(image0, NULL, screen, NULL);
		SDL_BlitSurface(screen, NULL, video, NULL); 
		SDL_Flip(video);
		usleep(50000);
		
		SDL_BlitSurface(image1, NULL, screen, NULL);
		SDL_BlitSurface(screen, NULL, video, NULL); 
		SDL_Flip(video);
		usleep(50000);	
		
		SDL_BlitSurface(image2, NULL, screen, NULL);
		SDL_BlitSurface(screen, NULL, video, NULL); 
		SDL_Flip(video);
		usleep(50000);	
		
		SDL_BlitSurface(image3, NULL, screen, NULL);
		SDL_BlitSurface(screen, NULL, video, NULL); 
		SDL_Flip(video);
		usleep(50000);	
		
		SDL_BlitSurface(image4, NULL, screen, NULL);
		SDL_BlitSurface(screen, NULL, video, NULL); 
		SDL_Flip(video);
		usleep(50000);	
		
		SDL_BlitSurface(image5, NULL, screen, NULL);
		SDL_BlitSurface(screen, NULL, video, NULL); 
		SDL_Flip(video);
		usleep(50000);	
		
		checkCharging();
		if (is_charging == 0){
			break;
		}
		
	}
	

	int run = 1;
	SetBrightness(8);
	//usleep(5000000);	//5s
	screen = SDL_CreateRGBSurface(SDL_HWSURFACE, 640,480, 32, 0,0,0,0);
	SDL_BlitSurface(screen, NULL, video, NULL); 
	SDL_Flip(video);
	
	SetBrightness(0);
	
	SDL_Event event;
	
	while ((SDL_PollEvent(&event))||(run==1)) {
		if (event.type==SDL_KEYDOWN) {
			if (event.key.keysym.sym == SDLK_SPACE){	    
				SetBrightness(8);
				run = 0;
			}
		}	
		checkCharging();
		if (is_charging == 0){
			system("reboot");	
		}
		usleep(100000);
	}
	
	
   	SDL_FreeSurface(screen);
  
   	SDL_FreeSurface(video);
    SDL_Quit();
	
    return EXIT_SUCCESS;
}
