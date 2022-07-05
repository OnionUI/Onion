#include <SDL/SDL.h>
#include <SDL/SDL_image.h>
#include <SDL/SDL_ttf.h>

void append(char subject[], const char insert[], int pos) {
    char buf[300] = {}; // 100 so that it's big enough. fill with 0
    // or you could use malloc() to allocate sufficient space

    strncpy(buf, subject, pos); // copy at most first pos characters
    int len = strlen(buf);
    strcpy(buf+len, insert); // copy all of insert[] at the end
    len += strlen(insert);  // increase the length by length of insert[]
    strcpy(buf+len, subject+pos); // copy the rest

    strcpy(subject, buf);   // copy it back to subject
    // deallocate buf[] here, if used malloc()
}

int main(void) {
	SDL_Init(SDL_INIT_VIDEO);
	TTF_Init();

	SDL_Rect rectInstructions = { 21, 76, 600, 450};;
	
	SDL_Color color_white={255,255,255,0};
	TTF_Font* fontRomName25 = TTF_OpenFont("/customer/app/Exo-2-Bold-Italic.ttf", 25);

	SDL_ShowCursor(SDL_DISABLE);
	   
	// 640x480, HW to HW blit, 1:1 crisp pixels, rightside up
	
	SDL_Surface* video = SDL_SetVideoMode(640,480, 32, SDL_HWSURFACE);
	SDL_Surface* screen = SDL_CreateRGBSurface(SDL_HWSURFACE, 640,480, 32, 0,0,0,0);
	SDL_Surface* image = IMG_Load("infoPanel.png");
	
	SDL_BlitSurface(image, NULL, screen, NULL);
	char test[200];
	strcpy(test,"ceci est un texte vraiment trètrès long afin de tester le retour à la ligne. A mon avis il ne va rien n'y avoir du tuot en effet liil faut tout lui dire  !!!");
	int len = strlen(test);
	
	for (int i=1; i<len ; i++){
		if ((i%10) == 0){
			append(test, "\r\n", i);
			
		} 
	
	}
	SDL_Surface* imageInstructions = TTF_RenderUTF8_Blended(fontRomName25, test , color_white);
	SDL_BlitSurface(imageInstructions, NULL, screen, &rectInstructions);

	// 320x240, HW to HW blit, blurry upscaling, rightside up
	// SDL_Surface* video = SDL_SetVideoMode(320,240, 32, SDL_HWSURFACE);
	// SDL_Surface* screen = SDL_CreateRGBSurface(SDL_HWSURFACE, 320,240, 32, 0,0,0,0);
	// SDL_Surface* image = IMG_Load("320x240.png");
	// SDL_BlitSurface(image, NULL, screen, NULL);
	
	// DO NOT USE
	// 320x240, SW to HW blit, blurry upscaling, upside down
	// SDL_Surface* video = SDL_SetVideoMode(320,240, 32, SDL_HWSURFACE);
	// SDL_Surface* image = IMG_Load("320x240.png");
	// SDL_BlitSurface(image, NULL, video, NULL);
	
	int run = 1;
	while (run) {
		// HW to HW rotates automatically on stock
		SDL_BlitSurface(screen, NULL, video, NULL); 
		SDL_Flip(video);

		SDL_Event event;
		while (SDL_PollEvent(&event)) {
			if (event.type==SDL_KEYDOWN) {
				run = 0;
				break;
			}
		}
	}
	
   	SDL_FreeSurface(screen);
   	SDL_FreeSurface(video);
    SDL_Quit();
	
    return EXIT_SUCCESS;
}
