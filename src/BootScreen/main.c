#include <SDL/SDL.h>
#include <SDL/SDL_image.h>
#include <SDL/SDL_ttf.h>
#include <stdbool.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h> 

#include </usr/local/include/mi_sys.h>

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

	TTF_Init();
	
	TTF_Font*	font; 
	SDL_Color	color={255,255,255,0};
	font = TTF_OpenFont("/customer/app/Exo-2-Bold-Italic.ttf", 30);


	SDL_Surface* video = SDL_SetVideoMode(640,480, 32, SDL_HWSURFACE|SDL_DOUBLEBUF);
	SDL_Surface* screen = SDL_CreateRGBSurface(SDL_HWSURFACE, 640,480, 32, 0,0,0,0);

	SDL_Surface* image;
	
	int bShowBat = 0 ;
	if (argc > 1){
		if (strcmp(argv[1],"Boot") == 0) {
			image = IMG_Load("bootScreen.png");
		}
		else if (strcmp(argv[1],"End_Save") == 0) {
			image = IMG_Load("Screen_Off_Save.png");
			bShowBat = 1;
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
	SDL_Rect 	rectVersion = { 5, 445, 120, 40};
	SDL_BlitSurface(imageVer, NULL, screen, &rectVersion);

	if (bShowBat == 1){
		
		SDL_Surface*	imageBatt;
		SDL_Rect	rectBatt;
		SDL_Rect	rectBatteryIcon;	
		
		// Check current battery value
		char *currBat;
		int nBat;
			
		fp = fopen ( "/tmp/percBat" , "rb" );
		if( fp > 0 ) {
			fseek( fp , 0L , SEEK_END);
			lSize = ftell( fp );
			rewind( fp );
			currBat = (char*)calloc( 1, lSize+1 );
			if( !currBat ) fclose(fp),fputs("memory alloc fails",stderr),exit(1);
		
			if( 1!=fread( currBat , lSize, 1 , fp) )
  			fclose(fp),free(currBat),fputs("entire read fails",stderr),exit(1);
			fclose(fp);	
		
			sscanf(currBat, "%d", &nBat);
			
			if (nBat != 500){
				// If the handheld is not charging
				char tempBat[] = "%";
    			strcat(currBat,tempBat);
			}
			else{
			strcpy(currBat, "");
			}
		}	
		
		// Battery icon
		SDL_Surface* imageBatteryIcon;
		if (nBat > 100){
			imageBatteryIcon = IMG_Load("battCharge.png");
		} else if (nBat >= 80){
				imageBatteryIcon = IMG_Load("batt100.png");
			}
				else if (nBat >= 60){
					imageBatteryIcon = IMG_Load("batt80.png");	
				}
					else if (nBat >= 40){
						imageBatteryIcon = IMG_Load("batt60.png");	
					}
						else if (nBat >= 20){
							imageBatteryIcon = IMG_Load("batt40.png");	
						}
							else if (nBat >= 10){
								imageBatteryIcon = IMG_Load("batt20.png");	
							}
								else if (nBat >= 0){
									imageBatteryIcon = IMG_Load("batt0.png");	
								}
	
		rectBatteryIcon = {541,447,13,27};
		rectBatt = { 566, 440, 113, 29};
	
		imageBatt = TTF_RenderUTF8_Blended(font, currBat, color);
		free(currBat);
		
		SDL_BlitSurface(imageBatt, NULL, screen, &rectBatt);	
		SDL_BlitSurface(imageBatteryIcon, NULL, screen, &rectBatteryIcon);
		
	}
	

	
	SDL_BlitSurface(screen, NULL, video, NULL); 
	SDL_Flip(video);
	SDL_BlitSurface(screen, NULL, video, NULL); 
	SDL_Flip(video);

	
	if (argc > 1){
		if (strcmp(argv[1],"Boot") != 0) {
				remove(".offOrder");
				system("sync");
				system("reboot");
				system("sleep 10");
		}	
	}

		
   	SDL_FreeSurface(screen);
    
   	SDL_FreeSurface(video);
    SDL_Quit();
	
    return EXIT_SUCCESS;
}
