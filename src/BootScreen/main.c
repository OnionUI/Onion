#include <SDL/SDL.h>
#include <SDL/SDL_image.h>
#include <SDL/SDL_ttf.h>
#include <stdbool.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h> 

bool file_exists (char *filename) {
  struct stat   buffer;   
  return (stat (filename, &buffer) == 0);
}

int main(int argc, char *argv[]) {
	
	// Boot 

	// Boot : Loading screen
	// End_Save : Ending screen with save
	// End : Ending screen without save
	
	SDL_Init(SDL_INIT_VIDEO);
	SDL_ShowCursor(SDL_DISABLE);
	TTF_Init();
	
	
	TTF_Font*	font; 
	SDL_Color	color={255,255,255,0};
	font = TTF_OpenFont("/customer/app/Exo-2-Bold-Italic.ttf", 30);
	SDL_Rect 	rectVersion = { 5, 445, 120, 40};

	
	SDL_Surface* video = SDL_SetVideoMode(640,480, 32, SDL_HWSURFACE);
	SDL_Surface* screen = SDL_CreateRGBSurface(SDL_HWSURFACE, 640,480, 32, 0,0,0,0);
	
	SDL_Surface* image;
	
	if (argc > 1){
		if (strcmp(argv[1],"Boot") == 0) {
			image = IMG_Load("bootScreen.png");
		}
		else if (strcmp(argv[1],"End_Save") == 0) {
			image = IMG_Load("Screen_Off_Save.png");
		}
		else if (strcmp(argv[1],"End") == 0) {
			image = IMG_Load("Screen_Off.png");
		}
		
	}
	else{
		image = IMG_Load("bootScreen.png");
	}
	
	char *cVersion;
	long lSize;
	FILE *fp;
	fp = fopen ( "/mnt/SDCARD/.tmp_update/onionVersion/version.txt" , "rb" );
	if( fp > 0 ) {
		fseek( fp , 0L , SEEK_END);
		lSize = ftell( fp );
		rewind( fp );
		cVersion = (char*)calloc( 1, lSize+1 );
		if( !cVersion ) fclose(fp),fputs("memory alloc fails",stderr),exit(1);
		
		if( 1!=fread( cVersion , lSize, 1 , fp) )
  		fclose(fp),free(cVersion),fputs("entire read fails",stderr),exit(1);
		fclose(fp);
	}

		
	SDL_Surface* imageVer = TTF_RenderUTF8_Blended(font, cVersion, color);
	
	SDL_BlitSurface(image, NULL, screen, NULL);
	SDL_BlitSurface(imageVer, NULL, screen, &rectVersion);
	
	SDL_BlitSurface(screen, NULL, video, NULL); 
	SDL_Flip(video);
	
	/*
	if (argc > 1){
		if (strcmp(argv[1],"0") == 0) {
			// Wait for the device to turn off
			sleep(100);
		}

	}
		*/
	if (argc > 1){
		if (strcmp(argv[1],"Boot") != 0) {
				remove(".offOrder");
				system("reboot");
				sleep(10);
		}	
	}

		
   	SDL_FreeSurface(screen);
   	SDL_FreeSurface(video);
    SDL_Quit();
	
    return EXIT_SUCCESS;
}
