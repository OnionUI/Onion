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
#include "cJSON.h"

#define MAXHISTORY 50
#define MAXHROMNAMESIZE 150
#define MAXHRACOMMAND 500
#define LOWBATRUMBLE 10

// Max number of records in the DB
#define MAXVALUES 1000


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

#define	BUTTON_UP	KEY_UP
#define	BUTTON_DOWN	KEY_DOWN

#define	BUTTON_LEFT		KEY_LEFT
#define	BUTTON_RIGHT	KEY_RIGHT

// Game history list
struct structGames {                 
             char name[MAXHROMNAMESIZE]   ;
             char RACommand[500] ;
           	 char totalTime[10];
           	 int jsonIndex;
           	 }
            gameList[MAXHISTORY];   
            
int taillestructGames = 0;
int currentGame = 0 ;

// Play activity database
struct structPlayActivity {                  
             char name[100]   ;
             int playTime ;
            }
            romList[MAXVALUES];        
int tailleStructPlayActivity = 0;

cJSON* request_json = NULL;
cJSON* items = cJSON_CreateArray();
	
	
void logMessage(char* Message) {
	FILE *file = fopen("/mnt/SDCARD/.tmp_update/log_turnMessage.txt", "a");
    char valLog[200];
    sprintf(valLog, "%s %s", Message, "\n");
    fputs(valLog, file);
	fclose(file); 
}

char *removeExt(char* myStr) {
    char *retStr;
    char *lastExt;
    if (myStr == NULL) return NULL;
    if ((retStr = (char*)malloc (strlen (myStr) + 1)) == NULL) return NULL;
    strcpy (retStr, myStr);
    lastExt = strrchr (retStr, '.');
    if (lastExt != NULL)
        *lastExt = '\0';
    return retStr;
}


bool file_exists (char *filename) {
  struct stat   buffer;   
  return (stat (filename, &buffer) == 0);
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

int bDisplayBoxArt = 0;

void SetRawBrightness(int val) {  // val = 0-100
    int fd = open("/sys/class/pwm/pwmchip0/pwm0/duty_cycle", O_WRONLY);
    if (fd>=0) {
        dprintf(fd,"%d",val);
        close(fd);
    }
}


int readRomDB(){

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
    			tailleStructPlayActivity++;
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

int getMiyooLum(void){

	cJSON* request_json = NULL;
	cJSON* itemBrightness;

	char sBrightness[20]; 
	
	const char *request_body = load_file("/appconfigs/system.json");
	request_json = cJSON_Parse(request_body);
	itemBrightness = cJSON_GetObjectItem(request_json, "brightness");

	int dBrightness = cJSON_GetNumberValue(itemBrightness);
		
	return dBrightness;
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


void readHistory(){
// History extraction
	taillestructGames = 0;
	const char *request_body = load_file("/mnt/SDCARD/Saves/CurrentProfile/lists/content_history.lpl");	
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
    						//logMessage(bname);
    						
    						if (nTimePosition >= 0){
    							
    							int nTime = romList[nTimePosition].playTime;
    							
    							if (nTime >=  0) {				
								int h, m;
								
								h = (nTime/3600); 
								
								m = (nTime -(3600*h))/60;	

								sprintf(gameList[nbGame].totalTime, "%d:%02d", h,m);
								} 							
    						}

    						char *bnameWOExt = removeExt(bname);
    						
    						strcpy(gameList[taillestructGames].name, bnameWOExt);
    						strcpy(gameList[taillestructGames].RACommand, RACommand);
							gameList[taillestructGames].jsonIndex = nbGame;
    						taillestructGames ++; 				
						}
					}
    			}
    			else {
    				break;
    			}
    				
			}	
	}
}


void removeCurrentItem(){
	
	cJSON_DeleteItemFromArray(items, gameList[currentGame].jsonIndex);
	
	FILE *file = fopen("/mnt/SDCARD/Saves/CurrentProfile/lists/content_history.lpl", "w");
	
	char *test = cJSON_Print(request_json);
	
	fputs(test, file);
	fclose(file); 

}

int main(void) {

	SDL_Color	color={255,255,255,0};
	TTF_Font*	font; 
	SDL_Surface*	imageBatt;
	SDL_Surface*	imageLum;
	SDL_Surface*	imagePlay;
	SDL_Surface*	imageGameName;
	SDL_Surface* 	surfaceArrowLeft = IMG_Load("arrowLeft.png");
	SDL_Surface* 	surfaceArrowRight = IMG_Load("arrowRight.png");

	SDL_Rect	rectBatt;
	SDL_Rect	rectLum;
	SDL_Rect	rectWhiteLigne;
	SDL_Rect	rectMenuBar;
	SDL_Rect	rectTime;
	SDL_Rect	rectBatteryIcon;	
	SDL_Rect	rectTuto;
	SDL_Rect	rectFooter;
	SDL_Rect	rectFooterHelp;
	SDL_Rect	rectGameName;	

	
	SDL_Rect 	rectArrowLeft = { 6, 222, 28, 32};
	SDL_Rect 	rectArrowRight = { 603, 222, 28, 32};
	
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
	
	// Check current brightness value
	/*
	F
	
	
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
	*/
	char currLum[20];
	long lSize;
	FILE *fp;
	int nLum = getMiyooLum();
	sprintf(currLum, "%d", nLum);
	
	char tempLum[] = "/10";
    strcat(currLum,tempLum);
    
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
	
	SDL_Surface* imageMenuBar = IMG_Load("menuBar.png");
	
	SDL_Surface* imageTuto;
	
	rectTime = { 244, 0, 150, 39};
	rectMenuBar = {0,0,640,480};
	rectLum = { 419, 0, 113, 39};
	rectBatteryIcon = {541,7,13,27};
	rectBatt = { 566, 0, 113, 29};
	rectFooterHelp = { 420, 441, 220, 39};

	rectGameName = { 9, 439, 640, 39};
	
	int bShowBoot = 0;

	
	
	int			input_fd;
	input_fd = open("/dev/input/event0", O_RDONLY);
	struct input_event	ev;
	
	
	int run = 1;
	int firstPass = 1;
	int comboKey = 0;
	
	SDL_Surface* video = SDL_SetVideoMode(640,480, 32, SDL_HWSURFACE);
	SDL_Surface* screen = SDL_CreateRGBSurface(SDL_HWSURFACE, 640,480, 32, 0,0,0,0);

	
	SDL_Init(SDL_INIT_VIDEO);
	SDL_ShowCursor(SDL_DISABLE);
	TTF_Init();
	
	
	font = TTF_OpenFont("/customer/app/Exo-2-Bold-Italic.ttf", 30);
	
	imageLum = TTF_RenderUTF8_Blended(font, currLum, color);
	imageBatt = TTF_RenderUTF8_Blended(font, currBat, color);

	

	free(currBat);
	
	
	SDL_Surface* imageBackgroundDefault = IMG_Load("bootScreen.png");
	SDL_Surface* imageBackgroundLowBat = IMG_Load("lowBat.png");
	SDL_Surface* imageBackgroundNoGame= IMG_Load("noGame.png");
	SDL_Surface* imageRemoveDialog= IMG_Load("removeDialog.png");
	SDL_Surface* imageBackgroundGame;
	SDL_Surface* imageFooterHelp = IMG_Load("footerHelp.png");	
	char currPicture[MAXHROMNAMESIZE+44];
	
	//if (file_exists(".RALaunched")==0) {
	// The screenshot is ready 
	// Printscreen save for specific game
	sprintf(currPicture,"/mnt/SDCARD/.tmp_update/romScreens/%s%s",gameList[currentGame].name,".png");	 	
	// Move screenshot to destination
	rename("screenshotGame.bmp", currPicture);  
	//}

	while(run) { 
	logMessage("a");
		if (firstPass > 0){
			// First screen draw
			firstPass ++ ;
			
			// Needs another refresh 
			if (firstPass > 2)
			firstPass = 0;
			
			
    		sprintf(currPicture,"/mnt/SDCARD/.tmp_update/romScreens/%s%s",gameList[currentGame].name,".png");
    		if (file_exists(currPicture)==1){
				imageBackgroundGame = IMG_Load(currPicture);
			}
			
			if (taillestructGames > 1){
				SDL_BlitSurface(surfaceArrowRight, NULL, screen, &rectArrowRight);
			}
		}
		else {
			if(read(input_fd, &ev, sizeof(ev)) == sizeof(ev) ){

				val = ev.value;
				
				if (( ev.code != BUTTON_MENU )&&(val == 1)) {
					comboKey = 1;
				}
				
				if (( ev.type != EV_KEY ) || ( val > 1 )) continue;
				
				
				if ( ev.code == BUTTON_SELECT ) {
						select_pressed = val;
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
				if ( ev.code == BUTTON_LEFT ) {
						left_pressed = val;
				}
				else
					if ( ev.code == BUTTON_RIGHT ) {
							right_pressed = val;
				}
				else
					if ( ev.code == BUTTON_MENU ) {
							menu_pressed = val;
				}
				else
					if ( ev.code == BUTTON_X ) {
							x_pressed = val;
				}
				else
					if ( ev.code == BUTTON_Y ) {
							y_pressed = val;
				}
				else 
					if ( ev.code == BUTTON_START ) {
							start_pressed = val;
				}
				
				logMessage("b");
				
				if (right_pressed) {	
				
            		if (currentGame<(taillestructGames-1)){
                		currentGame ++;
                		sprintf(currPicture,"/mnt/SDCARD/.tmp_update/romScreens/%s%s",gameList[currentGame].name,".png");
                		if (file_exists(currPicture)==1){
							imageBackgroundGame = IMG_Load(currPicture);
						}
					}
				}
				
				if (left_pressed) {	
								if (currentGame>0){
                     				currentGame --;
                     				sprintf(currPicture,"/mnt/SDCARD/.tmp_update/romScreens/%s%s",gameList[currentGame].name,".png");
                     				if (file_exists(currPicture)==1){
										imageBackgroundGame = IMG_Load(currPicture);
									}
                     			}
				}
				
				if (y_pressed) {	
					int fd = creat(".menuActivity", 777);
					close(fd); 
					break;
				}
				/*
				if (a_pressed) {	
					break;
				}		
				*/
				logMessage("c");
				if (start_pressed) {	
					nExitToMiyoo = 1;
					break;
				}
								
				if ( ev.code == BUTTON_MENU ) {
						menu_pressed = val;
						if (menu_pressed == 1){
							comboKey = 0 ;
						}
						else {
							if (comboKey == 0){
								comboKey = 1 ;
								
								break;

							}
						
						}
				}				
				
			}
		}
			
		rectWhiteLigne = { 0, 0, 1280, 39};
			
		// Brightness update

		char currLum[20];
		long lSize;
		FILE *fp;
		int nLum = getMiyooLum();
		sprintf(currLum, "%d", nLum);
	
		char tempLum[] = "/10";
    	strcat(currLum,tempLum);
    	
		imageLum = TTF_RenderUTF8_Blended(font, currLum, color);

		if (taillestructGames==0){
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
	
		
		if (currentGame!=0){
			SDL_BlitSurface(surfaceArrowLeft, NULL, screen, &rectArrowLeft);
		}
	
		if (taillestructGames!=0){
			if (currentGame!=taillestructGames-1){
				SDL_BlitSurface(surfaceArrowRight, NULL, screen, &rectArrowRight);
			}
		
		}


	
		SDL_BlitSurface(imageMenuBar, NULL, screen, &rectMenuBar);
		if (taillestructGames>0){
			imageGameName = TTF_RenderUTF8_Blended(font, gameList[currentGame].name, color);
			SDL_BlitSurface(imageGameName, NULL, screen, &rectGameName);
		}
		
		SDL_BlitSurface(imageFooterHelp, NULL, screen, &rectFooterHelp);
	
			
		SDL_BlitSurface(imageBatt, NULL, screen, &rectBatt);
		SDL_BlitSurface(imageLum, NULL, screen, &rectLum);
		imagePlay = TTF_RenderUTF8_Blended(font, gameList[currentGame].totalTime, color);	
		SDL_BlitSurface(imagePlay, NULL, screen, &rectTime);
		SDL_BlitSurface(imageBatteryIcon, NULL, screen, &rectBatteryIcon);

		if (x_pressed) {		
			if (taillestructGames != 0){
				x_pressed = 0;
				SDL_BlitSurface(imageRemoveDialog, NULL, screen, NULL);
				SDL_BlitSurface(screen, NULL, video, NULL); 
				SDL_Flip(video);
				
				while(read(input_fd, &ev, sizeof(ev)) == sizeof(ev) ){
				val = ev.value;
				
					if (( ev.code == BUTTON_A)&&(val == 1)) {
						removeCurrentItem();
						readHistory();
						currentGame = 0;
						firstPass = 1;
						break;
					}
					if (( ev.code == BUTTON_B)&&(val == 1)) {
						
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
	
	if (nExitToMiyoo == 0){
		remove("/tmp/cmd_to_run_launcher.sh");
		FILE *file = fopen("/tmp/cmd_to_run_launcher.sh", "w");
		fputs(gameList[currentGame].RACommand, file);
		fclose(file); 	
		remove("/tmp/romName.txt");		
	}
	 
	SDL_BlitSurface(screen, NULL, video, NULL); 
	SDL_Flip(video);
	
   	SDL_FreeSurface(screen);
   	SDL_FreeSurface(video);
   		
  
   	SDL_FreeSurface(imageBackgroundDefault);  
   	SDL_FreeSurface(imageBackgroundLowBat);  	
   	SDL_FreeSurface(imageBackgroundGame);  
   	SDL_FreeSurface(imageBackgroundNoGame); 
   		
   	SDL_FreeSurface(imageMenuBar);

   	SDL_FreeSurface(imagePlay);
   	SDL_FreeSurface(surfaceArrowLeft);
	SDL_FreeSurface(surfaceArrowRight);
  	SDL_FreeSurface(imageBatt);
  	SDL_FreeSurface(imageLum);
    SDL_Quit();
		
    return EXIT_SUCCESS;
}
