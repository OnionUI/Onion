#include <SDL/SDL.h>
#include <SDL/SDL_image.h>
#include <SDL/SDL_ttf.h>
#include "SDL_rotozoom.h"
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

void logMessage(char* Message) {
	FILE *file = fopen("log_turnMessage.txt", "a");
	/*char tempMess[] = "\r\n";
    strcat(Message,tempMess);
    */
    char valLog[200];
    sprintf(valLog, "%s %s", Message, "\n");
    fputs(valLog, file);
	fclose(file); 
}

bool file_exists (char *filename) {
  struct stat   buffer;   
  return (stat (filename, &buffer) == 0);
}
int bDisplayBoxArt = 0;

int main(void) {
	
	SDL_Color	color={255,255,255,0};
	TTF_Font*	font; 
	SDL_Surface*	imageBatt;
	SDL_Surface*	imageLum;
	SDL_Surface*	imagePlay;

	SDL_Rect	rectBatt;
	SDL_Rect	rectLum;
	SDL_Rect	rectWhiteLigne;
	SDL_Rect	rectMenuBar;
	SDL_Rect	rectTime;
	SDL_Rect	rectBatteryIcon;	
	SDL_Rect	rectTuto;

	int bTuto = file_exists(".modeTuto");
	
	if ( file_exists(".modeTuto2") == 1) {
		remove(".modeTuto2");
		bTuto = 5;
	}

	// Check current brightness value
	FILE *fp;
	long lSize;
	char *currLum;
	fp = fopen ( "/mnt/SDCARD/.tmp_update/brightSett" , "rb" );
	if( fp > 0 ) {
		fseek( fp , 0L , SEEK_END);
		lSize = ftell( fp );
		rewind( fp );
		currLum = (char*)calloc( 1, lSize+1 );
		if( !currLum ) fclose(fp),fputs("memory alloc fails",stderr),exit(1);
	
		if( 1!=fread( currLum , lSize, 1 , fp) )
  		fclose(fp),free(currLum),fputs("entire read fails",stderr),exit(1);
		fclose(fp);
		char tempLum[] = "/10";
    	strcat(currLum,tempLum);
	}
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
	
	// Check current playtime
	char *currPlay;
	fp = fopen ( "/mnt/SDCARD/App/PlayActivity/currentTotalTime" , "rb" );
	if( fp > 0 ) {
		fseek( fp , 0L , SEEK_END);
		lSize = ftell( fp );
		rewind( fp );
		currPlay = (char*)calloc( 1, lSize+1 );
		if( !currPlay ) fclose(fp),fputs("memory alloc fails",stderr),exit(1);
	
		if( 1!=fread( currPlay , lSize, 1 , fp) )
  		fclose(fp),free(currPlay),fputs("entire read fails",stderr),exit(1);
		fclose(fp);	
	}
	
	logMessage(currPlay);
	
	SDL_Init(SDL_INIT_VIDEO);
	SDL_ShowCursor(SDL_DISABLE);
	TTF_Init();
	   
	SDL_Surface* video = SDL_SetVideoMode(640,480, 32, SDL_HWSURFACE);
	SDL_Surface* screen = SDL_CreateRGBSurface(SDL_HWSURFACE, 640,480, 32, 0,0,0,0);
		
	font = TTF_OpenFont("/customer/app/Exo-2-Bold-Italic.ttf", 30);
	imageLum = TTF_RenderUTF8_Blended(font, currLum, color);
	imageBatt = TTF_RenderUTF8_Blended(font, currBat, color);
	imagePlay = TTF_RenderUTF8_Blended(font, currPlay, color);	
	
	free(currLum);
	free(currBat);
	free(currPlay);
	
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
							
	SDL_Surface* imageBackground = IMG_Load("bootScr.png");
	for (int i = 1; i < 150; ++i){
		if (file_exists(".menuStart")) {
			remove(".menuStart");
			imageBackground = IMG_Load("screenshotGame.bmp");
	 		break;    
	 	}
	usleep(100);
	}
	
	SDL_Surface* imageMenuBar = IMG_Load("menuBar.png");
	SDL_Surface* imagewhiteLigne = IMG_Load("whiteLigne.png");
	
	SDL_Surface* imageTuto;
	
	rectTime = { 244, 9, 150, 29};
	rectMenuBar = {0,0,640,59};
	rectLum = { 419, 9, 113, 29};
	rectBatteryIcon = {541,16,13,27};
	rectBatt = { 566, 9, 113, 29};
	
	int bShowBoot = 0;

	SDL_BlitSurface(imageBackground, NULL, screen, NULL);

	int nTuto = 0;

	for (int i = 1; i < 67; ++i){
		
		rectWhiteLigne = { -(i*10), 56, 640, 5};
				
		if ((i == 1) || (i == 33)){
			if (bTuto > 0) {
				char sImageTuto[20];
				sprintf(sImageTuto,"./Tuto/tuto0%d.png",bTuto);
				logMessage(sImageTuto);
				imageTuto = IMG_Load(sImageTuto);
				if(bTuto<4){
					bTuto ++;	
				}
					
			}
		}
		

		if (bTuto > 0) {
			SDL_BlitSurface(imageBackground, NULL, screen, NULL);
		}	
				

		SDL_BlitSurface(imageMenuBar, NULL, screen, &rectMenuBar);
		
		if ((bTuto >= 0)&&(bTuto < 4)){
			
			SDL_BlitSurface(imagewhiteLigne, NULL, screen, &rectWhiteLigne);	
		}
			
		SDL_BlitSurface(imageBatt, NULL, screen, &rectBatt);
		SDL_BlitSurface(imageLum, NULL, screen, &rectLum);
		SDL_BlitSurface(imagePlay, NULL, screen, &rectTime);
		SDL_BlitSurface(imageBatteryIcon, NULL, screen, &rectBatteryIcon);
		
		if (bTuto > 0) {
			SDL_BlitSurface(imageTuto, NULL, screen, NULL);
		}	
		
		
		if (file_exists(".menuSelect")){
			break;
		}
		
		if ( file_exists(".menuStart") || file_exists(".menuA") ) {
	
			if ( bTuto > 0){
				if (bTuto < 4){
					remove (".menuStart");
					remove (".menuA");
				}
				else{
					if ((bTuto == 4)&&file_exists(".menuStart")){
						remove(".menuA");
						remove(".modeTuto");
						int fd = creat(".modeTuto2", 777);
						close(fd); 	
						break ;
					}
					else if ((bTuto == 5)&&file_exists(".menuA")){
						remove(".menuStart");
						break ;
					}
					
					
	
				}
			}
			else {
				break;
			}
		}
		
		if ((i % 6) == 0){
		// Brightness update
			FILE *fp;
			long lSize;
			char *currLum;
			fp = fopen ( "/mnt/SDCARD/.tmp_update/brightSett" , "rb" );
			if( fp > 0 ) {
				fseek( fp , 0L , SEEK_END);
				lSize = ftell( fp );
				rewind( fp );
				currLum = (char*)calloc( 1, lSize+1 );
				if( !currLum ) fclose(fp),fputs("memory alloc fails",stderr),exit(1);
			
				if( 1!=fread( currLum , lSize, 1 , fp) )
  				fclose(fp),free(currLum),fputs("entire read fails",stderr),exit(1);
				fclose(fp);
				char tempLum[] = "/10";
    			strcat(currLum,tempLum);
				imageLum = TTF_RenderUTF8_Blended(font, currLum, color);
				free(currLum);
			}
			
			
		}		
	

	
		if (i == 66){
			// Show onion logo
			if (bTuto > 0)
			{
				i = 0;
			}			
			bShowBoot = 1;
			}	
		
		// HW to HW rotates automatically on stock
		SDL_BlitSurface(screen, NULL, video, NULL); 
		SDL_Flip(video);
		usleep(50000);
	}


	screen = SDL_CreateRGBSurface(SDL_HWSURFACE, 640,480, 32, 0,0,0,0);
	
	if (bShowBoot == 1){
		imageBackground = IMG_Load("bootScr.png");
		SDL_BlitSurface(imageBackground, NULL, screen, NULL);
	}
	 
	
	SDL_BlitSurface(screen, NULL, video, NULL); 
	SDL_Flip(video);
	
   	SDL_FreeSurface(screen);
   	SDL_FreeSurface(video);
   	
   	SDL_FreeSurface(imageBackground);  	
   	SDL_FreeSurface(imageMenuBar);
   	SDL_FreeSurface(imagewhiteLigne);
   	SDL_FreeSurface(imagePlay);
   	
  	SDL_FreeSurface(imageBatt);
  	SDL_FreeSurface(imageLum);
    SDL_Quit();
		
    return EXIT_SUCCESS;
}
