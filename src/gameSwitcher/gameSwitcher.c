#include <stdio.h>
#include <unistd.h>
#include <fcntl.h> 
#include <stdint.h>
#include <string.h>
#include <linux/fb.h>
#include <linux/input.h>
#include <fcntl.h>
#include <stdbool.h>
#include <libgen.h>
#include <sys/stat.h>   
#include <sys/ioctl.h>
#include <SDL/SDL.h>
#include <SDL/SDL_image.h>
#include <SDL/SDL_ttf.h>

// #include "SDL/SDL_rotozoom.h"
#include "cjson/cJSON.h"
#include "png/png.h"

#include "utils/utils.h"
#include "system/keymap_hw.h"

#define MAXHISTORY 50
#define MAXHROMNAMESIZE 250
#define MAXHROMPATHSIZE 150

#define ROM_SCREENS_DIR "/mnt/SDCARD/Saves/CurrentProfile/states/romScreens"

#define MAXFILENAMESIZE 250
#define MAXSYSPATHSIZE 80

#define MAXHRACOMMAND 500
#define LOWBATRUMBLE 10

// Max number of records in the DB
#define MAXVALUES 1000

#define	GPIO_DIR1 "/sys/class/gpio/"
#define	GPIO_DIR2 "/sys/devices/gpiochip0/gpio/"

char sTotalTimePlayed[50];

// Game history list
static struct game_s {                 
	char name[MAXHROMNAMESIZE];
	char RACommand[500] ;
	char totalTime[30];
	int jsonIndex;
}
game_list[MAXHISTORY];   
            
static int game_list_len = 0;
static int current_game = 0;

static cJSON* request_json = NULL;
static cJSON* items = NULL;
	
// Play activity database
struct structPlayActivity {                  
             char name[100]   ;
             int playTime ;
            }
            romList[MAXVALUES];        
int tailleStructPlayActivity = 0;


void IMG_SavePNG (SDL_Surface *SurfaceImage, char* pathImage){
	int width = SurfaceImage->w;
	int height = SurfaceImage->h;

	png_structp	png_ptr;
	png_infop	info_ptr;
	FILE		*fp;

	fp = fopen(pathImage, "wb");

	if (fp) {
		Uint32* linebuffer = (Uint32 *)malloc(SurfaceImage->pitch);
		Uint32* src = (Uint32 *)SurfaceImage->pixels;

		png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, 0, 0, 0);
		info_ptr = png_create_info_struct(png_ptr);
		png_init_io(png_ptr, fp);
		png_set_IHDR(png_ptr, info_ptr, width, height, 8, PNG_COLOR_TYPE_RGBA,
			PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);
		png_write_info(png_ptr, info_ptr);

		for (int y = 0 ; y < height ; y++){
			for (int x = 0 ; x < width ; x++){
				Uint32 pix = *src++;
				/* Use pix only when alpha is non-zero */
				linebuffer[x] = (pix & 0xFF000000) ? (pix & 0xFF00FF00) | (pix & 0xFF0000)>>16 | (pix & 0xFF)<<16 : 0;
				/* Following is also fine, but the above creates a cleaner png
				linebuffer[x] = (pix & 0xFF00FF00) | (pix & 0xFF0000)>>16 | (pix & 0xFF)<<16;
				*/
			}
			png_write_row(png_ptr, (png_bytep)linebuffer);
		}

		png_write_end(png_ptr, info_ptr);
		png_destroy_write_struct(&png_ptr, &info_ptr);
		fflush(fp);
		fsync(fileno(fp));
		fclose(fp);

		free(linebuffer);
	}
}

int bDisplayBoxArt = 0;

void setMiyooLum(int nLum){

	cJSON* request_json = NULL;
	cJSON* itemBrightness;

	char sBrightness[20];
	
	const char *request_body = file_read("/appconfigs/system.json");
	request_json = cJSON_Parse(request_body);
	itemBrightness = cJSON_GetObjectItem(request_json, "brightness");

	int dBrightness = cJSON_GetNumberValue(itemBrightness);
	sprintf(sBrightness, "%d", dBrightness);
	 
	
	cJSON_SetNumberValue(itemBrightness, nLum);

	FILE *file = fopen("/appconfigs/system.json", "w");	
	char *test = cJSON_Print(request_json);	
	fputs(test, file);
	fclose(file); 	
}
void SetRawBrightness(int val) {  // val = 0-100
    int fd = open("/sys/class/pwm/pwmchip0/pwm0/duty_cycle", O_WRONLY);
    if (fd>=0) {
        dprintf(fd,"%d",val);
        close(fd); 
    }
}


void SetBrightness(int value) {  // value = 0-10
    SetRawBrightness(value==0?6:value*10);   
    setMiyooLum(value); 
}



int getMiyooLum(void){
 
	cJSON* request_json = NULL;
	cJSON* itemBrightness;

	char sBrightness[20]; 
	
	const char *request_body = file_read("/appconfigs/system.json");
	request_json = cJSON_Parse(request_body);
	itemBrightness = cJSON_GetObjectItem(request_json, "brightness");

	int dBrightness = cJSON_GetNumberValue(itemBrightness);
		
	return dBrightness;
}


int readRomDB(){
	int totalTimePlayed = 0 ;
  	// Check to avoid corruption
  	if (file_exists("/mnt/SDCARD/Saves/CurrentProfile/saves/playActivity.db") == 1){
  	
    	FILE * file= fopen("/mnt/SDCARD/Saves/CurrentProfile/saves/playActivity.db", "rb");

		if (file != NULL) {

    		fread(romList, sizeof(romList), 1, file);
    		
     		tailleStructPlayActivity = 0;
    		
    		for (int i=0; i<MAXVALUES; i++){
    			if (strlen(romList[i].name) == 0){
    				break;
    			}
    			totalTimePlayed += romList[tailleStructPlayActivity].playTime;
    			
    			tailleStructPlayActivity++;
    		}

			int h;
			
			h = (totalTimePlayed/3600); 
			
			//m = (totalTimePlayed -(3600*h))/60;	

			sprintf(sTotalTimePlayed, "%dh", h);
			
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

int searchRomDB(char* romName){
	int position = -1;
	
	for (int i = 0 ; i < tailleStructPlayActivity ; i++) {
		if (strcmp(romList[i].name,romName) == 0){
			position = i;
			break;
		}
	}
	return position;
}


void readHistory()
{
	// History extraction
	game_list_len = 0;
	const char *request_body = file_read("/mnt/SDCARD/Saves/CurrentProfile/lists/content_history.lpl");	
	if (request_body != NULL){
			request_json = cJSON_Parse(request_body);	 
			items = cJSON_GetObjectItem(request_json, "items");	
									
			for (int nbGame = 0 ; nbGame < MAXHISTORY ; nbGame ++){
				cJSON* path = NULL;
				cJSON* core_path = NULL;
				
				cJSON * subitem = cJSON_GetArrayItem(items, nbGame);
    			
    			if (subitem != NULL){
    				path = cJSON_GetObjectItem(subitem, "path");
					core_path = cJSON_GetObjectItem(subitem, "core_path");
					char *cPath = cJSON_Print(path) ;
					char *cCore_path = cJSON_Print(core_path) ;
	    				
   					
					if ((strlen(cCore_path)>1) && (strlen(cPath)>1)) {
						// Quote character removal
						
						char *cCore_path1 = cCore_path+1;
						char *cPath1 = cPath+1;
						
						cCore_path1[strlen(cCore_path1)-1] = '\0';
						cPath1[strlen(cPath1)-1] = '\0';
				
				
						if ((file_exists(cCore_path1) == 1) && (file_exists(cPath1) == 1)){
						
							char RACommand[MAXHRACOMMAND];
						
							sprintf(RACommand, "LD_PRELOAD=/mnt/SDCARD/miyoo/lib/libpadsp.so ./retroarch -v -L %s %s", cJSON_Print(core_path), cJSON_Print(path));
												
							// Rom name 
							char *bname;
    						char *path2 = strdup(cJSON_Print(path));
       						
    						// File name
    						bname = (char *)basename(path2);
    						
    						// Cut the last " character
    						if (strlen( bname ) > 0){
    							bname[strlen( bname )-1] = '\0';
    						}
  							bname = (char *)basename(path2);  
    						
    						bname[MAXHROMNAMESIZE-1] = '\0';
    						
    						int nTimePosition = searchRomDB(bname);
    						
    						if (nTimePosition >= 0){
    							
    							int nTime = romList[nTimePosition].playTime;
    							
    							if (nTime >=  0) {				
								int h, m;
								
								h = (nTime/3600); 
								
								m = (nTime -(3600*h))/60;	

								sprintf(game_list[nbGame].totalTime, "%d:%02d / %s", h,m,sTotalTimePlayed);
								
								} 							
    						}
	
    						strcpy(game_list[game_list_len].name, bname);
    						strcpy(game_list[game_list_len].RACommand, RACommand);
							game_list[game_list_len].jsonIndex = nbGame;
    						game_list_len ++; 				
						}
					}
    			}
    			else {
    				break;
    			}
    				
			}	
	}
}




void rumble(uint32_t val) {
	int fd;
	const char str_export[] = "48";
	const char str_direction[] = "out";
	char value[1];
	value[0] = ((val&1)^1) + 0x30;
	char filename[128];

	concat(filename, GPIO_DIR1, "export");
	fd = open(filename, O_WRONLY); if (fd >= 0) { write(fd, str_export, 2); close(fd); }
	concat(filename, GPIO_DIR2, "gpio48/direction");
	fd = open(filename, O_WRONLY); if (fd >= 0) { write(fd, str_direction, 3); close(fd); }
	concat(filename, GPIO_DIR2, "gpio48/value");
	fd = open(filename, O_WRONLY); if (fd >= 0) { write(fd, value, 1); close(fd); }
}


void short_pulse(void) {
	rumble(1);
	usleep(100000);		// 0.1s
	rumble(0);				
}
void super_short_pulse(void) {
	rumble(1);
	usleep(50000);		// 0.1s
	rumble(0);				
}

void removeCurrentItem(){
	if (items != NULL)
		cJSON_DeleteItemFromArray(items, game_list[current_game].jsonIndex);
	
	FILE *file = fopen("/mnt/SDCARD/Saves/CurrentProfile/lists/content_history.lpl", "w");
	
	char *test = cJSON_Print(request_json);
	
	fputs(test, file);
	fclose(file); 

}

int main(void) {

	SDL_Color	color = {255,255,255,0};
	TTF_Font*	font; 
	SDL_Surface*	imageBatt;
	SDL_Surface*	imagePlay;
	SDL_Surface*	imageGameName;
	SDL_Surface* 	surfaceArrowLeft = IMG_Load("res/arrowLeft.png");
	SDL_Surface* 	surfaceArrowRight = IMG_Load("res/arrowRight.png");

	SDL_Rect	rectBatt = { 566, -1, 113, 29};
	SDL_Rect	rectLum = { 106, 59, 40, 369};
	SDL_Rect	rectWhiteLigne = { 0, 0, 1280, 39};
	SDL_Rect	rectMenuBar = {0,0,640,480};
	SDL_Rect	rectTime = { 263, -1, 150, 39};
	SDL_Rect	rectBatteryIcon = {541,6,13,27};
	SDL_Rect	rectFooterHelp = { 420, 441, 220, 39};
	SDL_Rect	rectGameName = { 9, 439, 640, 39};

	SDL_Rect 	rectArrowLeft = { 6, 217, 28, 32};
	SDL_Rect 	rectArrowRight = { 604, 217, 28, 32};
	

	uint32_t		val;
	uint32_t		l2_pressed = 0;
	uint32_t		r2_pressed = 0;
	uint32_t		menu_pressed = 0;

	uint32_t		a_pressed = 0;
	uint32_t		b_pressed = 0;
	uint32_t		up_pressed = 0;
	uint32_t		down_pressed = 0;
	uint32_t		x_pressed = 0;
	uint32_t		y_pressed = 0;
	uint32_t		start_pressed = 0;
	uint32_t		power_pressed = 0;
	uint32_t		select_pressed = 0;	
	uint32_t		left_pressed = 0;
	uint32_t		right_pressed = 0;	
	readRomDB();
	readHistory();
	
	int nExitToMiyoo = 0;
	
	
	long lSize;
	FILE *fp;
    
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
	if (nBat > 100)
		imageBatteryIcon = IMG_Load("res/battCharge.png");
	else if (nBat >= 80)
		imageBatteryIcon = IMG_Load("res/batt100.png");
	else if (nBat >= 60)
		imageBatteryIcon = IMG_Load("res/batt80.png");
	else if (nBat >= 40)
		imageBatteryIcon = IMG_Load("res/batt60.png");
	else if (nBat >= 20)
		imageBatteryIcon = IMG_Load("res/batt40.png");
	else if (nBat >= 10)
		imageBatteryIcon = IMG_Load("res/batt20.png");
	else if (nBat >= 0)
		imageBatteryIcon = IMG_Load("res/batt0.png");
	
	SDL_Surface* imageMenuBar = IMG_Load("res/menuBar.png");
	
	SDL_Surface* imageTuto;
	
	int bShowBoot = 0;

	
	
	int	input_fd;
	input_fd = open("/dev/input/event0", O_RDONLY);
	struct input_event	ev;
	
	
	int run = 1;
	int firstPass = 1;
	int comboKey = 0;
	
	// SDL_Surface* video = SDL_SetVideoMode(640,480, 32, SDL_HWSURFACE);
	SDL_Surface* video = SDL_SetVideoMode(640,480, 32, SDL_HWSURFACE|SDL_DOUBLEBUF);     // activate double buffering to display the UI after MainUI
	SDL_Surface* screen = SDL_CreateRGBSurface(SDL_HWSURFACE, 640,480, 32, 0,0,0,0);

	
	SDL_Init(SDL_INIT_VIDEO);
	SDL_ShowCursor(SDL_DISABLE);
	TTF_Init();
	
	
	font = TTF_OpenFont("/customer/app/Exo-2-Bold-Italic.ttf", 30);
	
	imageBatt = TTF_RenderUTF8_Blended(font, currBat, color);

	

	free(currBat);
	
	
	SDL_Surface* imageBackgroundDefault = IMG_Load("res/bootScreen.png");
	SDL_Surface* imageBackgroundLowBat = IMG_Load("res/lowBat.png");
	SDL_Surface* imageBackgroundNoGame= IMG_Load("res/noGame.png");
	SDL_Surface* imageRemoveDialog= IMG_Load("res/removeDialog.png");
	SDL_Surface* imageBackgroundGame;
	SDL_Surface* imageFooterHelp = IMG_Load("res/footerHelp.png");	
	
	char currPicture[MAXHROMNAMESIZE+44];
	sprintf(currPicture, ROM_SCREENS_DIR "/%s%s", file_removeExtension(game_list[current_game].name), ".png");	 
	// Move screenshot to destination
	mkdirs(ROM_SCREENS_DIR);
	rename("screenshotGame.png", currPicture); 
/*
	if(file_exists("screenshotGame.bmp")==1){
		SDL_Surface* imageScreen = IMG_Load("res/screenshotGame.bmp");		
		IMG_SavePNG (imageScreen, currPicture);

	}	
	*/
	

	while(run) { 
		int bBrightChange = 0;	
		if (firstPass > 0){
			// First screen draw
			firstPass ++ ;
			
			// Needs another refresh 
			if (firstPass > 2)
			firstPass = 0;
			
    		sprintf(currPicture, ROM_SCREENS_DIR "/%s%s", file_removeExtension(game_list[current_game].name), ".png");
    		
 
    		if (file_exists(currPicture)){
				imageBackgroundGame = IMG_Load(currPicture);
			}
			
			if (game_list_len > 1){
				SDL_BlitSurface(surfaceArrowRight, NULL, screen, &rectArrowRight);
			}
		}
		else {
			if(read(input_fd, &ev, sizeof(ev)) == sizeof(ev) ){

				val = ev.value;
				
				if (( ev.code != HW_BTN_MENU )&&(val == 1)) {
					comboKey = 1;
				}
				
				if ( ev.code == HW_BTN_MENU ) {
						menu_pressed = val;
						if (menu_pressed == 1){
							comboKey = 0 ;
						}
						else {
							if (comboKey == 0){
								comboKey = 1 ;
								//super_short_pulse();
								break;

							}
						
						}
				}	
				
				
				if (ev.type != EV_KEY || val > 1) continue;
				
				switch (ev.code)
				{
				case HW_BTN_SELECT: select_pressed = val; break;
				case HW_BTN_A: 		a_pressed = val; break;
				case HW_BTN_B: 		b_pressed = val; break;
				case HW_BTN_LEFT: 	left_pressed = val; break;
				case HW_BTN_RIGHT: 	right_pressed = val; break;
				case HW_BTN_MENU: 	menu_pressed = val; break;
				case HW_BTN_X: 		x_pressed = val; break;
				case HW_BTN_Y: 		y_pressed = val; break;
				case HW_BTN_START: 	start_pressed = val; break;
				case HW_BTN_UP: 	up_pressed = val; break;
				case HW_BTN_DOWN: 	down_pressed = val; break;
				default: break;
				}
				
				
				if ( val == 0 ) continue;
				
				if (right_pressed) {
            		if (current_game<(game_list_len-1)){
                		current_game ++;
                		
                		sprintf(currPicture, ROM_SCREENS_DIR "/%s%s", file_removeExtension(game_list[current_game].name), ".png");
                		if (file_exists(currPicture)==1){
							if (imageBackgroundGame != NULL){
								SDL_FreeSurface(imageBackgroundGame); 
							}
							imageBackgroundGame = IMG_Load(currPicture);
						}
					}
				}
				
				if (left_pressed) {	
					if (current_game>0){
						current_game --;
						sprintf(currPicture, ROM_SCREENS_DIR "/%s%s", file_removeExtension(game_list[current_game].name), ".png");
						if (file_exists(currPicture)==1){
							if (imageBackgroundGame != NULL){
								SDL_FreeSurface(imageBackgroundGame); 
							}
							imageBackgroundGame = IMG_Load(currPicture);
						}
					}
				}
				
				// if (y_pressed) {	
				// 	int fd = creat(".menuActivity", 777);
				// 	close(fd); 
				// 	break;
				// }
		
				if (start_pressed) {	
					nExitToMiyoo = 1;
					break;
				}
				
				if ((a_pressed)||(b_pressed)) {	
					
					break;
				}
				
				
				if (up_pressed){
					// Change brightness
      				bBrightChange = 1;
      				int brightVal = getMiyooLum();
      				if (brightVal < 10) {
      					brightVal++;
      					SetBrightness(brightVal);
      				}	
				}	


				if (down_pressed){
					// Change brightness
					bBrightChange = 1;
      				int brightVal = getMiyooLum();
      				if (brightVal > 0) {
      					brightVal--;
      					SetBrightness(brightVal);
      				}	
				}		
								
			
				
			}
		}
			
		
		long lSize;
		FILE *fp;
		

		if (game_list_len==0){
			SDL_BlitSurface(imageBackgroundNoGame, NULL, screen, NULL);
		}
		else{
  			if (nBat > LOWBATRUMBLE){
  		
            	if (file_exists(currPicture)==1){
                		SDL_BlitSurface(imageBackgroundGame, NULL, screen, NULL);         
                	//	SDL_FreeSurface(imageBackgroundGame);         
            	}
            	else{
            	
                	SDL_BlitSurface(imageBackgroundDefault, NULL, screen, NULL);
            	}
        	}
        	else {	
        	
            	SDL_BlitSurface(imageBackgroundLowBat, NULL, screen, NULL); 
        	}
		}
		
		
		if (current_game!=0){
			SDL_BlitSurface(surfaceArrowLeft, NULL, screen, &rectArrowLeft);
		}
	
		if (game_list_len!=0){
			if (current_game!=game_list_len-1){
				SDL_BlitSurface(surfaceArrowRight, NULL, screen, &rectArrowRight);
			}
		
		}

		SDL_BlitSurface(imageMenuBar, NULL, screen, &rectMenuBar);
		if (game_list_len>0){
			imageGameName = TTF_RenderUTF8_Blended(font, file_removeExtension(game_list[current_game].name), color);
			SDL_BlitSurface(imageGameName, NULL, screen, &rectGameName);
		}
		
		SDL_BlitSurface(imageFooterHelp, NULL, screen, &rectFooterHelp);
	
		SDL_BlitSurface(imageBatt, NULL, screen, &rectBatt);
		
		if (bBrightChange == 1) {
			// Display luminosity slider
			int nLum = getMiyooLum();
			char imageLumName[10];
			sprintf(imageLumName, "lum%d.png", nLum);
			SDL_Surface* imageLum = IMG_Load(imageLumName);
			
			SDL_BlitSurface(imageLum, NULL, screen, &rectLum);
			SDL_FreeSurface(imageLum); 
		}
		
		imagePlay = TTF_RenderUTF8_Blended(font, game_list[current_game].totalTime, color);	
		SDL_BlitSurface(imagePlay, NULL, screen, &rectTime);
		SDL_BlitSurface(imageBatteryIcon, NULL, screen, &rectBatteryIcon);

		if (x_pressed) {		
			if (game_list_len != 0){
				x_pressed = 0;
				SDL_BlitSurface(imageRemoveDialog, NULL, screen, NULL);
				SDL_BlitSurface(screen, NULL, video, NULL); 
				SDL_Flip(video);
				
				while(read(input_fd, &ev, sizeof(ev)) == sizeof(ev) ){
					val = ev.value;
				
					if (( ev.code == HW_BTN_A)&&(val == 1)) {
						removeCurrentItem();
						readHistory();
						current_game = 0;
						firstPass = 1;
						break;
					}
					if (( ev.code == HW_BTN_B)&&(val == 1)) {
						
						firstPass = 1;
						break;
					}	
				}
			}
		}


		SDL_BlitSurface(screen, NULL, video, NULL); 
		SDL_Flip(video);
		
	}
	
	
	screen = SDL_CreateRGBSurface(SDL_HWSURFACE, 640,480, 32, 0,0,0,0);
	
	remove("/mnt/SDCARD/.tmp_update/cmd_to_run.sh");
	remove("/mnt/SDCARD/.tmp_update/romName.txt");
	if (nExitToMiyoo != 1){
		FILE *file = fopen("/mnt/SDCARD/.tmp_update/cmd_to_run.sh", "w");
		fputs(game_list[current_game].RACommand, file);
		fclose(file); 	

		FILE *file2 = fopen("/mnt/SDCARD/.tmp_update/romName.txt", "w");
		fputs(game_list[current_game].name, file2);
		fclose(file2); 			
	
	}
	
	 
	SDL_BlitSurface(screen, NULL, video, NULL); 
	SDL_Flip(video);
	SDL_BlitSurface(screen, NULL, video, NULL); // two times to manage double buffering from MainUI
	SDL_Flip(video);
	
   	SDL_FreeSurface(screen);
   	SDL_FreeSurface(video);
   	
   	
  
   	SDL_FreeSurface(imageBackgroundDefault);  
   	SDL_FreeSurface(imageBackgroundLowBat);  	
   	//SDL_FreeSurface(imageBackgroundGame);  
  
   		
   	SDL_FreeSurface(imageMenuBar);

   	SDL_FreeSurface(imagePlay);
   	SDL_FreeSurface(surfaceArrowLeft);
	SDL_FreeSurface(surfaceArrowRight);
  	SDL_FreeSurface(imageBatt);

    SDL_Quit();
		
    return EXIT_SUCCESS;
}
