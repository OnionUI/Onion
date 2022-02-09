#include <SDL/SDL.h>
#include <SDL/SDL_image.h>
#include <unistd.h>

int main(void) {
	SDL_Init(SDL_INIT_VIDEO);
	SDL_ShowCursor(SDL_DISABLE);
	
	SDL_Surface* video = SDL_SetVideoMode(640,480, 32, SDL_HWSURFACE);
	SDL_Surface* screen = SDL_CreateRGBSurface(SDL_HWSURFACE, 640,480, 32, 0,0,0,0);
	SDL_Surface* image = IMG_Load("bootScr.png");
	SDL_BlitSurface(image, NULL, screen, NULL);
	
	SDL_BlitSurface(screen, NULL, video, NULL); 
	SDL_Flip(video);

	
   	SDL_FreeSurface(screen);
   	SDL_FreeSurface(video);
    SDL_Quit();
	
    return EXIT_SUCCESS;
}
