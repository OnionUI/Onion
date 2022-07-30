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
#include <SDL/SDL.h>
#include <SDL/SDL_image.h>
#include <SDL/SDL_ttf.h>

#include "cJSON/cJSON.h"
   
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

char* load_file(char const* path)
{
    char* buffer = 0;
    long length;
    FILE * f = fopen (path, "rb"); 

    if (f)
    {
      fseek (f, 0, SEEK_END);
      length = ftell (f);
      fseek (f, 0, SEEK_SET);
      buffer = (char*)malloc ((length+1)*sizeof(char));
      if (buffer)
      {
        fread (buffer, sizeof(char), length, f);
      }
      fclose (f);
    }
    buffer[length] = '\0';

    return buffer;
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
  	dp = opendir ("/mnt/SDCARD/Themes");
	
  	if (dp != NULL)
  	{
    	while (ep = readdir (dp)){
    		
			sprintf(cThemePath, "/mnt/SDCARD/Themes/%s/config.json", ep->d_name);
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
	int hideIconTitle = 0;
    	int useThemeLang = 0;
	
	SDL_Init(SDL_INIT_VIDEO);
	SDL_ShowCursor(SDL_DISABLE);
	TTF_Init();
	
	SDL_Surface* video = SDL_SetVideoMode(640,480, 32, SDL_HWSURFACE);
	SDL_Surface* screen = SDL_CreateRGBSurface(SDL_HWSURFACE, 640,480, 32, 0,0,0,0);
	TTF_Font* font40 = TTF_OpenFont("/customer/app/Exo-2-Bold-Italic.ttf", 40);
	TTF_Font* font21 = TTF_OpenFont("/customer/app/Exo-2-Bold-Italic.ttf", 21);
	TTF_Font* font30 = TTF_OpenFont("/customer/app/Exo-2-Bold-Italic.ttf", 30);
	

	SDL_Color color_white={255,255,255,0};
	// Prepare for Poll button input
	int			input_fd;
	input_fd = open("/dev/input/event0", O_RDONLY);
	struct input_event	ev;
	
	int nCurrentPage = 0;


	SDL_Surface* surfaceThemeBackground;

	SDL_Surface* imagePages;	
	SDL_Surface* imageThemeNom;	
	
	SDL_Surface* surfaceBackGround = IMG_Load(".appRessources/background.png");
	SDL_Surface* surfaceArrowLeft = IMG_Load(".appRessources/arrowLeft.png");
	SDL_Surface* surfaceArrowRight = IMG_Load(".appRessources/arrowRight.png");
	
	SDL_Rect rectArrowLeft = { 24, 210, 28, 32};
	SDL_Rect rectArrowRight = { 588, 210, 28, 32};
	SDL_Rect rectPages = { 559, 440, 85, 54};
	SDL_Rect rectThemeDescription = { 10, 175, 600, 44};
	SDL_Rect rectThemeBackground = { 80, 46, 480, 360};
	SDL_Rect rectImageThemeNom = { 77, 7, 557, 54};	
	int levelPage = 0; 
	FILE *fp;
	long lSize;	
	
	//char* tempt = "BirdShot";
	sprintf(cThemePath, "/mnt/SDCARD/Themes/%s/preview.png", themes[nCurrentPage]);
	
	if (file_exists(cThemePath) == 1){
		surfaceThemeBackground = IMG_Load(cThemePath);
	}
	
	else{
		surfaceThemeBackground = IMG_Load(".appRessources/noThemePreview.png");
	}
	
	char cPages[10];
	sprintf(cPages,"%d/%d",(nCurrentPage+1),nb_themes);
	imagePages = TTF_RenderUTF8_Blended(font30, cPages, color_white);

	SDL_BlitSurface(surfaceThemeBackground, NULL, screen, &rectThemeBackground);
	
	SDL_BlitSurface(surfaceBackGround, NULL, screen, NULL);
	//SDL_BlitSurface(surfaceArrowLeft, NULL, screen, &rectArrowLeft);
	SDL_BlitSurface(surfaceArrowRight, NULL, screen, &rectArrowRight);
	
	SDL_BlitSurface(imagePages, NULL, screen, &rectPages);
	
	sprintf(cThemePath, "/mnt/SDCARD/Themes/%s/config.json", themes[0]);


	if (file_exists(cThemePath) == 1){
		const char *request_body = load_file(cThemePath);	
		
		cJSON* request_json = NULL;
		cJSON* themeName;
		
		if (request_body != NULL){
			request_json = cJSON_Parse(request_body);	
			
			themeName = cJSON_GetObjectItem(request_json, "name");
			
			imageThemeNom = TTF_RenderUTF8_Blended(font21, cJSON_GetStringValue(themeName), color_white);
			SDL_BlitSurface(imageThemeNom, NULL, screen, &rectImageThemeNom);
		}

    }

		
	SDL_BlitSurface(screen, NULL, video, NULL); 
	SDL_Flip(video);
	
	SDL_FreeSurface(surfaceThemeBackground);


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
				
				
				cJSON* request_json = NULL;
				cJSON* itemTheme;
			
				char sBrightness[20]; 
				
				const char *request_body = load_file("/appconfigs/system.json");
				request_json = cJSON_Parse(request_body);
				itemTheme = cJSON_GetObjectItem(request_json, "theme");

				logMessage("cJSON_GetStringValue(itemTheme)");
				logMessage(cJSON_GetStringValue(itemTheme));

				char cThemePath[200];
				sprintf(cThemePath, "/mnt/SDCARD/Themes/%s/", themes[nCurrentPage]);
				logMessage(cThemePath);
				logMessage("  ");
			
				cJSON_SetValuestring(itemTheme, cThemePath);
				

				FILE *file = fopen("/appconfigs/system.json", "w");	
				
				char *test = cJSON_Print(request_json);	
				fputs(test, file);
				fclose(file); 					
				char themeLangInstall[] = "./themeLangInstall.sh ";
				logMessage("installing language");
				if (useThemeLang) {
				    strcat(themeLangInstall, cThemePath);
				    strcat(themeLangInstall, "lang");
				} else if (hideIconTitle == 1){
				    strcat(themeLangInstall, "/mnt/SDCARD/APP/ThemeSwitcher/.appRessources/hideTitle");
				} else {
				    strcat(themeLangInstall, "/mnt/SDCARD/APP/ThemeSwitcher/.appRessources/showTitle");
				}
				logMessage(themeLangInstall);
				system(themeLangInstall);
				
					
					
					
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
		
			sprintf(cThemePath, "/mnt/SDCARD/Themes/%s/preview.png", themes[nCurrentPage]);
			
			if (file_exists(cThemePath) == 1){
				surfaceThemeBackground = IMG_Load(cThemePath);
			}
			
			else{
				surfaceThemeBackground = IMG_Load(".appRessources/noThemePreview.png");
			}
				
			SDL_BlitSurface(surfaceThemeBackground, NULL, screen, &rectThemeBackground);
			
		
			SDL_BlitSurface(surfaceBackGround, NULL, screen, NULL);
			
			if (nCurrentPage != 0){
				SDL_BlitSurface(surfaceArrowLeft, NULL, screen, &rectArrowLeft);
			}
			if (nCurrentPage != (nb_themes-1)){
				SDL_BlitSurface(surfaceArrowRight, NULL, screen, &rectArrowRight);
			}
			
			sprintf(cPages,"%d/%d",(nCurrentPage+1),nb_themes);
			imagePages = TTF_RenderUTF8_Blended(font30, cPages, color_white);			
			SDL_BlitSurface(imagePages, NULL, screen, &rectPages);
			
			sprintf(cThemePath, "/mnt/SDCARD/Themes/%s/config.json", themes[nCurrentPage]);
		
			if (file_exists(cThemePath) == 1){
				const char *request_body = load_file(cThemePath);	
				
				cJSON* request_json = NULL;
				cJSON* themeName;
				
				if (request_body != NULL){
					request_json = cJSON_Parse(request_body);	
					
					themeName = cJSON_GetObjectItem(request_json, "name");
					
					imageThemeNom = TTF_RenderUTF8_Blended(font21, cJSON_GetStringValue(themeName), color_white);
					SDL_BlitSurface(imageThemeNom, NULL, screen, &rectImageThemeNom);
				}
		
    		}

			SDL_FreeSurface(surfaceThemeBackground);	
		}
		else {
			surfaceThemeBackground = IMG_Load(".appRessources/themeDetail.png");
			SDL_BlitSurface(surfaceThemeBackground, NULL, screen, NULL);

			sprintf(cThemePath, "/mnt/SDCARD/Themes/%s/config.json", themes[nCurrentPage]);
		
			if (file_exists(cThemePath) == 1){
				const char *request_body = load_file(cThemePath);	
				
				cJSON* request_json = NULL;
				cJSON* themeName;
				cJSON* themeIconTitle;
				cJSON* themeLang;
				
				if (request_body != NULL){
					request_json = cJSON_Parse(request_body);	
					
					themeName = cJSON_GetObjectItem(request_json, "name");
					themeIconTitle = cJSON_GetObjectItem(request_json, "hideIconTitle");
                    themeLang = cJSON_GetObjectItem(request_json, "useThemeLang");
					
					
					if (cJSON_IsTrue(themeLang)){
                        useThemeLang = 1;
                    }
                    else if (cJSON_IsTrue(themeIconTitle)){
                            hideIconTitle = 1;
                    }
									
					imagePages = TTF_RenderUTF8_Blended(font40, cJSON_GetStringValue(themeName), color_white);
					SDL_BlitSurface(imagePages, NULL, screen, &rectThemeDescription);
				}
		
    		}
			
			
		
		}
		
		

		SDL_BlitSurface(screen, NULL, video, NULL); 
		SDL_Flip(video);
		

	}
		SDL_FreeSurface(imagePages);
		SDL_FreeSurface(surfaceThemeBackground);
		SDL_FreeSurface(surfaceBackGround);	
		SDL_FreeSurface(surfaceArrowLeft);	
		SDL_FreeSurface(surfaceArrowRight);	
	
    return EXIT_SUCCESS;
}
