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
 
// Max number of records in the DB
#define NUMBER_OF_THEMES 100
#define MAX_THEME_NAME_SIZE 256


#define	BUTTON_A	KEY_SPACE
#define	BUTTON_B	KEY_LEFTCTRL  

#define	BUTTON_X	KEY_LEFTSHIFT
#define	BUTTON_Y	KEY_LEFTALT  

#define	BUTTON_START	KEY_ENTER
#define	BUTTON_SELECT	KEY_RIGHTCTRL

#define	BUTTON_MENU	KEY_ESC
#define	BUTTON_POWER	KEY_POWER

#define	BUTTON_L2	KEY_TAB
#define	BUTTON_R2	KEY_BACKSPACE


 

int tailleStructure = 0;

void logMessage(char* Message) {
	FILE *file = fopen("log_TS_Message.txt", "a");

    char valLog[200];
    sprintf(valLog, "%s%s", Message, "\n");
    fputs(valLog, file);
	fclose(file); 
}


bool file_exists (char *filename) {
  struct stat   buffer;   
  return (stat (filename, &buffer) == 0);
}


int main(void) {
	
	uint32_t		val;
	uint32_t		l2_pressed = 0;
	uint32_t		r2_pressed = 0;
	uint32_t		menu_pressed = 0;

	uint32_t		a_pressed = 0;
	uint32_t		b_pressed = 0;
//	uint32_t		x_pressed = 0;
//	uint32_t		y_pressed = 0;
	uint32_t		start_pressed = 0;
	uint32_t		select_pressed = 0;	

	uint32_t		left_pressed = 0;
	uint32_t		right_pressed = 0;		
	
	
  	DIR *dp;
  	struct dirent *ep;  
  	
  	int nb_themes = 0;
  	char themes[NUMBER_OF_THEMES][MAX_THEME_NAME_SIZE];
	char cThemePath[250]; 
  	dp = opendir ("./Themes");
	
  	if (dp != NULL)
  	{
    	while (ep = readdir (dp)){
    		
			sprintf(cThemePath, "./Themes/%s/preview.png", ep->d_name);
    		if (file_exists(cThemePath) == 1){
    	    	
    	    	strcpy(themes[nb_themes], ep->d_name);		
    	    	nb_themes ++;    		
    		}
    	}      	
    	(void) closedir (dp);
  	}
  	else{
  		perror ("Couldn't open the directory");
  	}
    	

	
	
	int current_page = 0;

	
	SDL_Init(SDL_INIT_VIDEO);
	SDL_ShowCursor(SDL_DISABLE);
	TTF_Init();
	
	SDL_Surface* video = SDL_SetVideoMode(640,480, 32, SDL_HWSURFACE);
	SDL_Surface* screen = SDL_CreateRGBSurface(SDL_HWSURFACE, 640,480, 32, 0,0,0,0);
	TTF_Font* font40 = TTF_OpenFont("/customer/app/Exo-2-Bold-Italic.ttf", 40);
	TTF_Font* font25 = TTF_OpenFont("/customer/app/Exo-2-Bold-Italic.ttf", 25);
	TTF_Font* font30 = TTF_OpenFont("/customer/app/Exo-2-Bold-Italic.ttf", 30);
	SDL_Color color_white={255,255,255,0};
	// Prepare for Poll button input
	int			input_fd;
	input_fd = open("/dev/input/event0", O_RDONLY);
	struct input_event	ev;
	
	int nCurrentPage = 0;


	SDL_Surface* surfaceThemeBackground;
	SDL_Surface* imagePages;	
	
	SDL_Rect rectTheme1 = { 0, 0, 320, 240};
	SDL_Rect rectTheme2 = { 320, 0, 320, 240};
	SDL_Rect rectTheme3 = { 0, 240, 320, 240};
	SDL_Rect rectTheme4 = { 320, 240, 320, 240};
	SDL_Rect rectPages = { 561, 430, 90, 44};
	SDL_Rect rectThemeDescription = { 10, 175, 600, 44};
	 
	SDL_Rect rectWhiteSelection = { 319, 0, 321, 241};
	
	//char* tempt = "BirdShot";
	sprintf(cThemePath, "./Themes/%s/preview.png", themes[nCurrentPage]);
	surfaceThemeBackground = IMG_Load(cThemePath);
	
	char cPages[10];
	sprintf(cPages,"%d/%d",(nCurrentPage+1),nb_themes);
	imagePages = TTF_RenderUTF8_Blended(font30, cPages, color_white);

	SDL_BlitSurface(surfaceThemeBackground, NULL, screen, NULL);
	SDL_BlitSurface(imagePages, NULL, screen, &rectPages);

	SDL_BlitSurface(screen, NULL, video, NULL); 
	SDL_Flip(video);

	int levelPage = 0; 
	FILE *fp;
	long lSize;	

	while( read(input_fd, &ev, sizeof(ev)) == sizeof(ev) ) {
		
		val = ev.value	; 
		 
		if (( ev.type != EV_KEY ) || ( val > 1 )) continue;
		if ( ev.code == BUTTON_L2 ) {
				l2_pressed = val;
		}
		else
			if ( ev.code == BUTTON_R2 ) {
					r2_pressed = val;
			}
			else
				if ( ev.code == BUTTON_START ) {
						start_pressed = val;
				}
				else
					if ( ev.code == BUTTON_SELECT ) {
							select_pressed = val;
					}
					else	
						if ( ev.code == BUTTON_MENU ) {
								menu_pressed = val;
						}
						else
							if ( ev.code == BUTTON_A ) {
									a_pressed = val;
							}
							else
								if ( ev.code == BUTTON_B ) {
										b_pressed = val;
								}
								else
									if ( ev.code == KEY_LEFT ) {
											left_pressed = val;
									}
									else
										if ( ev.code == KEY_RIGHT ) {
												right_pressed = val;
										}								
										
		if (b_pressed) {			
			if (levelPage==0){
				//exit program
				break;
			}
			else{
				levelPage = 0;
			}
		}
				
		if (a_pressed) {			
			if (levelPage==1){
				//Launch script then exit
				char cCommandScript[100];
				sprintf(cCommandScript, "./themeInstall.sh %s",themes[nCurrentPage]);
				system(cCommandScript);
				break;
			}
			else{
				levelPage = 1;
			}
		}		
		
		if (right_pressed) {			
			if (nCurrentPage < (nb_themes-1)){
				nCurrentPage ++;
			}
		}
		if (left_pressed) {			
			if (nCurrentPage > 0){
				nCurrentPage --;
			}
		}

		if (levelPage==0){
			sprintf(cThemePath, "./Themes/%s/preview.png", themes[nCurrentPage]);
			surfaceThemeBackground = IMG_Load(cThemePath);
			SDL_BlitSurface(surfaceThemeBackground, NULL, screen, NULL);
			
			sprintf(cPages,"%d/%d",(nCurrentPage+1),nb_themes);
			imagePages = TTF_RenderUTF8_Blended(font30, cPages, color_white);
			SDL_BlitSurface(imagePages, NULL, screen, &rectPages);
	
	
		}
		else {
			surfaceThemeBackground = IMG_Load("themeDetail.png");
			SDL_BlitSurface(surfaceThemeBackground, NULL, screen, NULL);
			
			char *currPlay;
			sprintf(cThemePath, "./Themes/%s/description.txt", themes[nCurrentPage]);
			fp = fopen ( cThemePath, "rb" );
			if( fp > 0 ) {
				fseek( fp , 0L , SEEK_END);
				lSize = ftell( fp );
				rewind( fp );
				currPlay = (char*)calloc( 1, lSize+1 );
				if( !currPlay ) fclose(fp),fputs("memory alloc fails",stderr),exit(1);
			
				if( 1!=fread( currPlay , lSize, 1 , fp) )
  				fclose(fp),free(currPlay),fputs("entire read fails",stderr),exit(1);
				fclose(fp);	
				imagePages = TTF_RenderUTF8_Blended(font40, currPlay, color_white);
				SDL_BlitSurface(imagePages, NULL, screen, &rectThemeDescription);
			}
			
		
		}
		

		SDL_BlitSurface(screen, NULL, video, NULL); 
		SDL_Flip(video);
		
		SDL_FreeSurface(imagePages);
		SDL_FreeSurface(surfaceThemeBackground);
	

	}

	
    return EXIT_SUCCESS;
}
