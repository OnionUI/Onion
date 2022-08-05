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


#include "../common/system.h"
#include "../common/keymap_hw.h"
#include "../common/utils.h"


// Max number of records in the DB
#define MAXVALUES 1000


typedef struct structRom {                  /*struct called list*/
	char name[100];
	int playTime;
} rom_list_s;
static rom_list_s rom_list[MAXVALUES];     
static int rom_list_len = 0;


int readRomDB(){

  	// Check to avoid corruption
  	if (file_exists("/mnt/SDCARD/Saves/CurrentProfile/saves/playActivity.db") == 1){
  	
    	FILE * file= fopen("/mnt/SDCARD/Saves/CurrentProfile/saves/playActivity.db", "rb");

		if (file != NULL) {

    		fread(rom_list, sizeof(rom_list), 1, file);
    		
     		rom_list_len = 0;
    		
    		for (int i=0; i<MAXVALUES; i++){
    			if (strlen(rom_list[i].name) != 0){
    				rom_list_len++;
    			}
    			
    		}
    		fclose(file);
		}
		else {
    		// The file exists but could not be opened
    		// Something went wrong, the program is terminated
    		return -1;
		}
	}
	return 1;
}



void writeRomDB(void){
	FILE * file= fopen("/mnt/SDCARD/Saves/CurrentProfile/saves/playActivity.db", "wb");
	if (file != NULL) {
    	fwrite(rom_list, sizeof(rom_list), 1, file);
    	fclose(file);
	}
}


void displayRomDB(void){
	printf("--------------------------------\n");
	for (int i = 0 ; i < rom_list_len ; i++) {	
			printf("rom_list name: %s\n", rom_list[i].name);
			
			char cPlayTime[15];
			sprintf(cPlayTime, "%d", rom_list[i].playTime);
			printf("playtime: %s\n", cPlayTime);
	 }
	printf("--------------------------------\n");

}


	
int searchRomDB(char* romName){
	int position = -1;
	
	for (int i = 0 ; i < rom_list_len ; i++) {
		if (strcmp(rom_list[i].name,romName) == 0){
			position = i;
			break;
		}
	}
	return position;
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
	uint32_t		power_pressed = 0;	
	
		
	
	
	SDL_Init(SDL_INIT_VIDEO);
	SDL_ShowCursor(SDL_DISABLE);
	TTF_Init();
	int			input_fd;
	struct input_event	ev;
	 
	// Prepare for Poll HW_BTN input
	input_fd = open("/dev/input/event0", O_RDONLY);

	SDL_Surface* video = SDL_SetVideoMode(640,480, 32, SDL_HWSURFACE);
	SDL_Surface* screen = SDL_CreateRGBSurface(SDL_HWSURFACE, 640,480, 32, 0,0,0,0);
			
	
	TTF_Font* font40 = TTF_OpenFont("/customer/app/Exo-2-Bold-Italic.ttf", 40);
	TTF_Font* font25 = TTF_OpenFont("/customer/app/Exo-2-Bold-Italic.ttf", 25);
	TTF_Font* font30 = TTF_OpenFont("/customer/app/Exo-2-Bold-Italic.ttf", 30);
	TTF_Font* fontRomName25 = TTF_OpenFont("/customer/app/wqy-microhei.ttc", 25);
	
	SDL_Color color_white={255,255,255,0};
	SDL_Color color_lilla={136,97,252,0};
	
	SDL_Surface* imageBackground = IMG_Load("background.png");
	
	SDL_Surface* imageRomPosition;
	SDL_Surface* imageRomPlayTime;
	SDL_Surface* imageRomName;

 	SDL_Surface* imagePages;
 	SDL_Surface* imageMileage;

	// Loading DB
	if (readRomDB()  == -1){
		// To avoid a DB overwrite
			return EXIT_SUCCESS;
	}

	//displayRomDB();
	// Sorting DB
	rom_list_s tempStruct; 
	int bFound = 1;
	
	while (bFound == 1){
		bFound = 0;
		for (int i = 0 ; i < (rom_list_len-1) ; i++){
			if (rom_list[i].playTime < rom_list[i+1].playTime){

				tempStruct = rom_list[i+1];
				rom_list[i+1] = rom_list[i];
				rom_list[i] = tempStruct;
				bFound = 1 ;
			}	
		}

	}

	//writeRomDB();
		
	//	Mileage
	int ntotalTime = 0;
	for (int i = 0 ; i < rom_list_len ; i++) {	
		ntotalTime+=rom_list[i].playTime;
	}

	char cTotalHandheldMileage[30];	 
		
	//sprintf(cPages, "%d",ntotalTime);
	//logMessage("Mileage");	
	//logMessage(cPages);	
	int h, m;
	h = (ntotalTime/3600); 		
	m = (ntotalTime -(3600*h))/60;		
	sprintf(cTotalHandheldMileage, "%d:%02d", h,m);		
	
	//displayRomDB();
	
	int nPages = (int)((rom_list_len-1)/4)+1;
	char cMessage[50];
			
	SDL_BlitSurface(imageBackground, NULL, screen, NULL);			

	SDL_Rect rectPosition ;
	SDL_Rect rectRomPlayTime ;
	SDL_Rect rectRomNames ;
	
	SDL_Rect rectPages = { 561, 430, 90, 44};
	SDL_Rect rectMileage = { 484, 8, 170, 42};
	
	int nCurrentPage = 0;
	char cPosition[5];
	char cPlayTime[20];
	char cTotalTimePlayed[50];


	char cPages[10];
	
	sprintf(cPages,"%d/%d",(nCurrentPage+1),nPages);
	imagePages = TTF_RenderUTF8_Blended(font30, cPages, color_white);
	imageMileage = TTF_RenderUTF8_Blended(font30, cTotalHandheldMileage, color_white);
	
	
	for (int i = 0 ; i < 4 ; i++) {	
		sprintf(cPosition, "%d", i+1);
		h = (rom_list[i].playTime/3600); 		
		m = (rom_list[i].playTime -(3600*h))/60;	
			
		if (strlen(rom_list[i].name) != 0){
			sprintf(cTotalTimePlayed, "%d:%02d", h,m);	
		}	
		else{
			sprintf(cTotalTimePlayed, "");
		}
				
		
		char *bnameWOExt = file_removeExtension(rom_list[i].name);
		imageRomPosition = TTF_RenderUTF8_Blended(font40, cPosition, color_lilla);
		imageRomPlayTime = TTF_RenderUTF8_Blended(font40, cTotalTimePlayed, color_white);
		imageRomName = TTF_RenderUTF8_Blended(fontRomName25, bnameWOExt , color_white);

		SDL_Rect rectPosition = { 16, 78+(90*i), 76, 39};
		SDL_Rect rectRomPlayTime = { 77, 66+(90*i), 254, 56};
		SDL_Rect rectRomNames = { 78, 104+(90*i), 600, 40};	
		
		SDL_BlitSurface(imageRomPosition, NULL, screen, &rectPosition);	
		SDL_BlitSurface(imageRomPlayTime, NULL, screen, &rectRomPlayTime);	
		SDL_BlitSurface(imageRomName, NULL, screen, &rectRomNames);		
	}
	
	SDL_BlitSurface(imagePages, NULL, screen, &rectPages);	
	SDL_BlitSurface(imageMileage, NULL, screen, &rectMileage);
	
	SDL_BlitSurface(screen, NULL, video, NULL); 
	SDL_Flip(video);



	while( read(input_fd, &ev, sizeof(ev)) == sizeof(ev) ) {
		
		val = ev.value	; 
		 
		if (( ev.type != EV_KEY ) || ( val > 1 )) continue;
		if ( ev.code == HW_BTN_L2 ) {
				l2_pressed = val;
		}
		else
			if ( ev.code == HW_BTN_R2 ) {
					r2_pressed = val;
			}
			else
				if ( ev.code == HW_BTN_START ) {
						start_pressed = val;
				}
				else
					if ( ev.code == HW_BTN_SELECT ) {
							select_pressed = val;
					}
					else	
						if ( ev.code == HW_BTN_MENU ) {
								menu_pressed = val;
						}
						else
							if ( ev.code == HW_BTN_A ) {
									a_pressed = val;
							}
							else
								if ( ev.code == HW_BTN_B ) {
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
									else
										if ( ev.code == HW_BTN_POWER ) {
												power_pressed = val;
										}									
			
		if (power_pressed) {	
			system_shutdown();
			break;
		}	
										
		if (b_pressed) {			
			break;
		}		
		
		if (right_pressed) {			
			if (nCurrentPage < (nPages-1)){
				nCurrentPage ++;
			}
		}
		if (left_pressed) {			
			if (nCurrentPage > 0){
				nCurrentPage --;
			}
		}
		

		// Page update
		SDL_BlitSurface(imageBackground, NULL, screen, NULL);
		
		sprintf(cPages,"%d/%d",(nCurrentPage+1),nPages);
		imagePages = TTF_RenderUTF8_Blended(font30, cPages, color_white);
		
		SDL_BlitSurface(imagePages, NULL, screen, &rectPages);	
		SDL_BlitSurface(imageMileage, NULL, screen, &rectMileage);	
		for (int i = 0 ; i < 4 ; i++) {
			
		sprintf(cPosition, "%d", (int)((nCurrentPage*4)+i+1));
			
		h = (rom_list[(nCurrentPage*4)+i].playTime/3600); 		
		m = (rom_list[(nCurrentPage*4)+i].playTime -(3600*h))/60;		
		
		if (strlen(rom_list[(nCurrentPage*4)+i].name) != 0){
			sprintf(cTotalTimePlayed, "%d:%02d", h,m);	
		}	
		else{
			sprintf(cTotalTimePlayed, "");
		}			
		
		char *bnameWOExt = file_removeExtension(rom_list[(nCurrentPage*4)+i].name);
		imageRomPosition = TTF_RenderUTF8_Blended(font40, cPosition, color_lilla);
		imageRomPlayTime = TTF_RenderUTF8_Blended(font40, cTotalTimePlayed, color_white);
		imageRomName = TTF_RenderUTF8_Blended(fontRomName25, bnameWOExt , color_white);

		SDL_Rect rectPosition = { 16, 78+(90*i), 76, 39};
		SDL_Rect rectRomPlayTime = { 77, 66+(90*i), 254, 56};
		SDL_Rect rectRomNames = { 78, 104+(90*i), 600, 40};	
		
		SDL_BlitSurface(imageRomPosition, NULL, screen, &rectPosition);	
		SDL_BlitSurface(imageRomPlayTime, NULL, screen, &rectRomPlayTime);	
		SDL_BlitSurface(imageRomName, NULL, screen, &rectRomNames);	


	
		}
		SDL_BlitSurface(screen, NULL, video, NULL); 
		SDL_Flip(video);

	}
	//sleep(0.25);
	
   	SDL_FreeSurface(screen);
   	SDL_FreeSurface(video);
    SDL_Quit();
	
    return EXIT_SUCCESS;
}
