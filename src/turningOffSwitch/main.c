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

#define MAXHISTORY 30
#define MAXHROMNAMESIZE 150
#define MAXHRACOMMAND 500
#define LOWBATRUMBLE 10

// Max number of records in the DB
#define MAXVALUES 1000

// Game history list
struct structGames {                 
             char name[MAXHROMNAMESIZE]   ;
             char RACommand[500] ;
           	 char totalTime[10];
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


int readRomDB(){

  	// Check to avoid corruption
  	if (file_exists("/mnt/SDCARD/RetroArch/.retroarch/saves/playActivity.db") == 1){
  	
    	FILE * file= fopen("/mnt/SDCARD/RetroArch/.retroarch/saves/playActivity.db", "rb");

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
	
	SDL_Rect 	rectArrowLeft = { 0, 222, 28, 32};
	SDL_Rect 	rectArrowRight = { 612, 222, 28, 32};
	
	int bTuto = file_exists(".modeTuto");
	
	if ( file_exists(".modeTuto2") == 1) {
		remove(".modeTuto2");
		bTuto = 5;
	}

	readRomDB();

	// History extraction
	cJSON* request_json = NULL;
	cJSON* items = cJSON_CreateArray();
	
	const char *request_body = load_file("/mnt/SDCARD/RetroArch/content_history.lpl");	
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
						
							sprintf(RACommand, "./retroarch -v -L %s %s", cJSON_Print(core_path), cJSON_Print(path));
												
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
    							//logMessage("Trouvé");
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
		
    						taillestructGames ++; 				
    						
						}
					}
    			
    			}
    			else {
    				break;
    			}
    			
		
			}
		
	
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
	
	SDL_Init(SDL_INIT_VIDEO);
	SDL_ShowCursor(SDL_DISABLE);
	TTF_Init();
	   
	SDL_Surface* video = SDL_SetVideoMode(640,480, 32, SDL_HWSURFACE);
	SDL_Surface* screen = SDL_CreateRGBSurface(SDL_HWSURFACE, 640,480, 32, 0,0,0,0);
		
	font = TTF_OpenFont("/customer/app/Exo-2-Bold-Italic.ttf", 30);
	
	imageLum = TTF_RenderUTF8_Blended(font, currLum, color);
	imageBatt = TTF_RenderUTF8_Blended(font, currBat, color);

	
	free(currLum);
	free(currBat);
	
	
	SDL_Surface* imageBackgroundDefault = IMG_Load("bootScr.png");
	SDL_Surface* imageBackgroundLowBat = IMG_Load("lowBat.png");
	SDL_Surface* imageBackgroundNoGame= IMG_Load("noGame.png");
	SDL_Surface* imageBackgroundGame;
	
	for (int i = 1; i < 150; ++i){
		if (file_exists(".menuStart")) {
			// The screenshot is ready 
			remove(".menuStart");
			
			imageBackgroundGame = IMG_Load("screenshotGame.bmp");
	 	
	 		// Printscreen save for specific game
			char currPicture[MAXHROMNAMESIZE+44];
	 		sprintf(currPicture,"/mnt/SDCARD/.tmp_update/romScreens/%s%s",gameList[0].name,".png");	 		

			// Move screenshot to destination
			rename("screenshotGame.bmp", currPicture);
	
	 		break;    
	 	}
		usleep(100);

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
	SDL_Surface* imageFooter = IMG_Load("footer.png");
	SDL_Surface* imageFooterHelp = IMG_Load("footerHelp.png");
	SDL_Surface* imagewhiteLigne = IMG_Load("whiteLigne.png");
	
	SDL_Surface* imageTuto;
	
	rectTime = { 244, 0, 150, 39};
	rectMenuBar = {0,0,640,39};
	rectLum = { 419, 0, 113, 39};
	rectBatteryIcon = {541,7,13,27};
	rectBatt = { 566, 0, 113, 29};
	rectFooter = { 0, 441, 637, 39};
	rectFooterHelp = { 413, 403, 227, 39};
	rectGameName = { 5, 439, 640, 39};
	
	int bShowBoot = 0;



	if (taillestructGames > 1){
		SDL_BlitSurface(surfaceArrowRight, NULL, screen, &rectArrowRight);
	}
	
	SDL_BlitSurface(imageFooter, NULL, screen, &rectFooter);
	
	if (taillestructGames>0){
		
		imageGameName = TTF_RenderUTF8_Blended(font, gameList[currentGame].name, color);
		SDL_BlitSurface(imageGameName, NULL, screen, &rectGameName);
	}
	
	SDL_BlitSurface(imageFooterHelp, NULL, screen, &rectFooterHelp);
	
	int nSlower = 0;
	for (int i = 1; i < 67; ++i){
	
		if (bTuto>=4){
			rectWhiteLigne = { 0, 0, 1280, 39};
		}
		else {
			rectWhiteLigne = { -((i-1)*10), 0, 1280, 39};
		}
		
				
		if ((i == 1) || (i == 33)){
			if (bTuto > 0) {
				char sImageTuto[20];
				sprintf(sImageTuto,"./Tuto/tuto0%d.png",bTuto);
				imageTuto = IMG_Load(sImageTuto);
				if(bTuto<4){
					bTuto ++;	
				}
					
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
			else{
				bShowBoot = 1;
			}		
			
		}	
		

		
		SDL_Event event;
		while (SDL_PollEvent(&event)) {
			if ((event.type==SDL_KEYDOWN)&&(file_exists("./RACommand.txt")==1)) {
				switch( event.key.keysym.sym ){
					    case SDLK_RIGHT:
					    
                     		if ((bTuto==0)&&(currentGame<(taillestructGames-1))){
                     			currentGame ++;
                     			nSlower = 1;
                     			i=0;
 
                     		}
							break;
							
						case SDLK_LEFT:
							if ((bTuto==0)&&(currentGame>0)){
                     			currentGame --;
                     			i = 0;
                     		}
							break;
						default:
                       		break;
				}
			
			}
		}
		char currPicture[MAXHROMNAMESIZE+44];
		sprintf(currPicture,"/mnt/SDCARD/.tmp_update/romScreens/%s%s",gameList[currentGame].name,".png");	

		if (i==0){
			// Loading game screens	hot 
	 	    if (file_exists(currPicture)==1){
				imageBackgroundGame = IMG_Load(currPicture);
			}
		}

      	if (bTuto > 0) {
			SDL_BlitSurface(imageBackgroundDefault, NULL, screen, NULL);
		}	
		else {
			if (file_exists("./RACommand.txt")!=1){
				SDL_BlitSurface(imageBackgroundNoGame, NULL, screen, NULL);
			}
			else{
  				if (nBat > LOWBATRUMBLE){
            		if (file_exists(currPicture)==1){
                		SDL_BlitSurface(imageBackgroundGame, NULL, screen, NULL);
            		}
            		else{
                		SDL_BlitSurface(imageBackgroundDefault, NULL, screen, NULL);
            		}
        		}
        		else {		
            		SDL_BlitSurface(imageBackgroundLowBat, NULL, screen, NULL); 
        		}
			}
		}
		
		if (currentGame!=0){
			SDL_BlitSurface(surfaceArrowLeft, NULL, screen, &rectArrowLeft);
		}
	
		if (currentGame!=taillestructGames-1){
			SDL_BlitSurface(surfaceArrowRight, NULL, screen, &rectArrowRight);
		}


		SDL_BlitSurface(imageFooter, NULL, screen, &rectFooter);
		imageGameName = TTF_RenderUTF8_Blended(font, gameList[currentGame].name, color);
		SDL_BlitSurface(imageGameName, NULL, screen, &rectGameName);
		SDL_BlitSurface(imageFooterHelp, NULL, screen, &rectFooterHelp);
		SDL_BlitSurface(imagewhiteLigne, NULL, screen, &rectWhiteLigne);	
		SDL_BlitSurface(imageMenuBar, NULL, screen, &rectMenuBar);	
		SDL_BlitSurface(imageBatt, NULL, screen, &rectBatt);
		SDL_BlitSurface(imageLum, NULL, screen, &rectLum);
		imagePlay = TTF_RenderUTF8_Blended(font, gameList[currentGame].totalTime, color);	
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
		
		
		
		SDL_BlitSurface(screen, NULL, video, NULL); 
		SDL_Flip(video);
		
		usleep(50000);
		if (nSlower == 1){
			usleep(80000);
		}
	}
	screen = SDL_CreateRGBSurface(SDL_HWSURFACE, 640,480, 32, 0,0,0,0);
	if (bShowBoot == 1){
		SDL_BlitSurface(imageBackgroundDefault, NULL, screen, NULL);
	}
	else{
		if (currentGame != 0){
			// A new game has been chosen
			remove("/mnt/SDCARD/.tmp_update/RACommand.txt");
			FILE *file = fopen("/mnt/SDCARD/.tmp_update/RACommand.txt", "w");
			fputs(gameList[currentGame].RACommand, file);
			fclose(file); 	
			
			remove("/mnt/SDCARD/.tmp_update/romName.txt");			
		}
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
   	SDL_FreeSurface(imagewhiteLigne);
   	SDL_FreeSurface(imagePlay);
   	SDL_FreeSurface(surfaceArrowLeft);
	SDL_FreeSurface(surfaceArrowRight);
  	SDL_FreeSurface(imageBatt);
  	SDL_FreeSurface(imageLum);
    SDL_Quit();
		
    return EXIT_SUCCESS;
}
