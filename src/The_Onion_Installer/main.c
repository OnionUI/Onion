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
#include <stdlib.h>

// Max number of records in the DB
#define NUMBER_OF_LAYERS 200
#define MAX_LAYER_NAME_SIZE 256
#define MAY_LAYER_DISPLAY 35

char layers[3][NUMBER_OF_LAYERS][MAX_LAYER_NAME_SIZE];
int bInstall[3][NUMBER_OF_LAYERS];
int bInstallChange[3][NUMBER_OF_LAYERS];
int nb_Layers[3];
int nSelection = 0; 
int nListPostion = 0;
int nTab = 0;
int allActivated = 0;

SDL_Surface* video = SDL_SetVideoMode(640,480, 32, SDL_HWSURFACE);
SDL_Surface* screen = SDL_CreateRGBSurface(SDL_HWSURFACE, 640,480, 32, 0,0,0,0);

SDL_Surface* surfaceBackground = IMG_Load("./ressources/bgApp.png");
SDL_Surface* surfaceSelection = IMG_Load("./ressources/selection.png");
SDL_Surface* surfaceTableau = IMG_Load("./ressources/tableau.png");
SDL_Surface* surfacesTabSelection = IMG_Load("./ressources/selectionTitle.png");
SDL_Surface* surfaceScroller = IMG_Load("./ressources/scroller.png");
SDL_Surface* surfaceCheck = IMG_Load("./ressources/checked.png");
SDL_Surface* surfaceCross = IMG_Load("./ressources/cross.png");

SDL_Color color_white={255,255,255,0};

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

void logMessage(char* Message) {
	FILE *file = fopen("log_OI_Message.txt", "a");

    char valLog[200]; 
    sprintf(valLog, "%s%s", Message, "\n");
    fputs(valLog, file);
	fclose(file); 
}


bool file_exists (char *filename) {
    int status;
    struct stat st_buf;
    
    status = stat (filename, &st_buf);
    if (status != 0) {
        return 0;
    }
  
  	if (S_ISREG (st_buf.st_mode)) {
  		return 1;
  	}
  	return 0;
  	
}

bool folder_exists (char *filename) {
    int status;
    struct stat st_buf;
    
    status = stat (filename, &st_buf);
    if (status != 0) {
        return 0;
    }
  
  	if (S_ISDIR (st_buf.st_mode)) {
  		return 1;
  	}
  	return 0;

}


bool file_folder_exists (char *filename) {
  struct stat   buffer;   
  return (stat (filename, &buffer) == 0);
}


void loadingScreen() {
	SDL_Surface* surfaceBackground = IMG_Load("./ressources/loading.png");
	SDL_BlitSurface(surfaceBackground, NULL, screen, NULL);
	SDL_BlitSurface(screen, NULL, video, NULL); 
	SDL_Flip(video);
	SDL_FreeSurface(surfaceBackground);

}

void setLayersInstall (int bInstallValue, int bInstallValueChange){
	for(int n = 0 ; n < 3 ; n++){
		for(int i = 0 ; i < NUMBER_OF_LAYERS ; i++){
			bInstall[n][i] = bInstallValue;
			bInstallChange[n][i] = bInstallValueChange;
		}	
	}
}



void appUninstall(char *basePath, int nT, int index, int strlenBase)
{
    char path[1000];
    char pathInstalledApp[1000];
    char *basePathDestination = "/mnt/SDCARD";
   	
    struct dirent *dp;
    DIR *dir = opendir(basePath);
	
	int run = 1;

    // Unable to open directory stream
    if (!dir)
        return;

    while (((dp = readdir(dir)) != NULL) && (run == 1))
    	
        {
        if (strcmp(dp->d_name, ".") != 0 && strcmp(dp->d_name, "..") != 0)
        {
        
        		
        	// Construct new path from our base path
        	strcpy(path, basePath);
        	strcat(path, "/");
        	strcat(path, dp->d_name);

			if (file_folder_exists(path)){
				char pathInstalledApp[1000];
		
				int nIndex = 0;
				
				strcpy(pathInstalledApp, basePathDestination);

				
				for (int i = strlenBase ; i < strlen(path)+strlen(basePathDestination) ; i++){
					pathInstalledApp[nIndex+strlen(basePathDestination)] = path[i];
					nIndex ++;
				}	
				pathInstalledApp[nIndex]= '\0';		
			
				if  (file_exists (pathInstalledApp)) {			
					remove(pathInstalledApp);
				}
				
				if  (folder_exists (pathInstalledApp)) {			
					rmdir(pathInstalledApp);
				}		
				
				
				
			}
			
			appUninstall(path, nT, index, strlenBase);
     	
        }
    }

    closedir(dir);
}



void checkAppInstalled(char *basePath, int nT, int index, int strlenBase)
{
    char path[1000];
    char pathInstalledApp[1000];
    char *basePathDestination = "/mnt/SDCARD";
   	
   	
  //  char file_path[1000];
    struct dirent *dp;
    DIR *dir = opendir(basePath);
	
	int run = 1;

    // Unable to open directory stream
    if (!dir)
        return;
	
    while (((dp = readdir(dir)) != NULL) && (run == 1))
    	
        {
        if (strcmp(dp->d_name, ".") != 0 && strcmp(dp->d_name, "..") != 0)
        {
        

        	// Construct new path from our base path
        	strcpy(path, basePath);
        	strcat(path, "/");
        	strcat(path, dp->d_name);
		
			
			
			if (file_exists(path)){
				char pathInstalledApp[1000];
				int nIndex = 0;	
				strcpy(pathInstalledApp, basePathDestination);
				
				for (int i = strlenBase ; i < strlen(path)+strlen(basePathDestination) ; i++){
				
				pathInstalledApp[nIndex+strlen(basePathDestination)] = path[i];
				nIndex ++;
				
				}	
				pathInstalledApp[nIndex]= '\0';		

				//logMessage(path);
				//logMessage(pathInstalledApp);
				
				if  (file_exists (pathInstalledApp)) {			
			
					bInstall[nT][index] = 1;						
					run = 0;
				}
			}
			
			// Recursive call
			checkAppInstalled(path, nT, index, strlenBase);

        }
    }

    closedir(dir);
}



void loadRessources(){
	
	int nT = 0;	
	char cCommand[500];
	for (int nT = 0 ; nT < 3 ; nT ++){
			DIR *dp;
			struct dirent *ep;  
		
			char cSingleRessource[250]; 
			
			char ressourcesPath[200] ;
			nb_Layers[nT]=0;
			
			switch(nT){
				case 0 :
					sprintf(ressourcesPath,"%s","./data/Layer1");
					break;
				case 1 :
					sprintf(ressourcesPath,"%s","./data/Layer2");
					break;
				case 2 :
					sprintf(ressourcesPath,"%s","./data/Layer3");
					break;
			}
		
			dp = opendir (ressourcesPath);
	
  			if (dp != NULL)
  			{
    			while ((ep = readdir (dp)) && (nb_Layers[nT]<NUMBER_OF_LAYERS)){
		
					char cShort[MAX_LAYER_NAME_SIZE];
					strcpy(cShort, ep->d_name);	
					cShort[MAY_LAYER_DISPLAY] = '\0';
					size_t len = strlen(cShort);
					if ((len > 2)||(cShort[0]!='.')){
					
				
					//logMessage(ressourcesPath);
					//logMessage(cShort);
					
					
					// Installation check
    				char basePath[1000];
    				sprintf(basePath,"%s%s%s",ressourcesPath,"/",cShort);
					//logMessage(basePath);
					//logMessage("");
    				
    				checkAppInstalled(basePath, nT , nb_Layers[nT], strlen(basePath));
					
					//sprintf(cCommand, "cd /mnt/SDCARD/App/The_Onion_Installer/ressources ; ./searchInstall.sh \"%s\" \"%s\"",ressourcesPath, cShort);
					//logMessage(cCommand);
					//system(cCommand);
				
					strcpy(layers[nT][nb_Layers[nT]],cShort);
					nb_Layers[nT] ++;   		
    				}
    		   				
    			}    
    			
    			(void) closedir (dp);
  			}
  			
  			else{
  				perror ("Couldn't open the directory");
  			}
  		}																						
}

void displayLayersNames(){
	SDL_Rect rectRessName;	
	SDL_Surface* surfaceRessName;
	TTF_Font* font25 = TTF_OpenFont("/customer/app/Exo-2-Bold-Italic.ttf", 25);	
	
	for (int i = 0 ; i < 7 ; i++){
		if ((i + nListPostion) < nb_Layers[nTab]){
			surfaceRessName = TTF_RenderUTF8_Blended(font25, layers[nTab][i + nListPostion], color_white);
			rectRessName = { 35, 92 + (i*47) , 80, 20};
			SDL_BlitSurface(surfaceRessName, NULL, screen, &rectRessName);
		}
	}
	TTF_CloseFont(font25);	
	SDL_FreeSurface(surfaceRessName);
}

void displayLayersInstall(){
	SDL_Rect rectInstall;
		
	for (int i = 0 ; i < 7 ; i++){
		if ((i + nListPostion) < nb_Layers[nTab]){
		
			rectInstall = { 567, 96 + (i*47) , 27, 27};
			if (bInstall[nTab][i + nListPostion] == 1){
				SDL_BlitSurface(surfaceCheck, NULL, screen, &rectInstall);			
			} 
			else {
				SDL_BlitSurface(surfaceCross, NULL, screen, &rectInstall);	
			}
			
		}
	}

}


void showScroller(){

	int shiftY = (int)((nListPostion * 311) / (nb_Layers[nTab]-7));
	SDL_Rect rectSroller = { 608, 86+(shiftY), 16, 16};
	SDL_BlitSurface(surfaceScroller, NULL, screen, &rectSroller);

}



void refreshScreen(){

	SDL_Color color_pink={136,97,252,0};

	SDL_Rect rectSelection = { 15, 84+(nSelection*47), 593, 49};	
	SDL_Rect rectTitle = { 457, 9, 200, 50};	
	SDL_Rect rectTabSelection = { 15 + (199 * nTab), 59, 199, 26};	
	
	SDL_BlitSurface(surfaceBackground, NULL, screen, NULL);
	SDL_BlitSurface(surfacesTabSelection, NULL, screen, &rectTabSelection);
	SDL_BlitSurface(surfaceTableau, NULL, screen, NULL);
	SDL_BlitSurface(surfaceSelection, NULL, screen, &rectSelection);
	
	if (nb_Layers[nTab] > 0){
		displayLayersNames();
		showScroller();
		displayLayersInstall();
	}

	SDL_BlitSurface(screen, NULL, video, NULL); 
	SDL_Flip(video);
}


int main(void) {
	
	uint32_t		val;
	uint32_t		l2_pressed = 0;
	uint32_t		r2_pressed = 0;
	uint32_t		menu_pressed = 0;

	uint32_t		a_pressed = 0;
	uint32_t		b_pressed = 0;
	uint32_t		x_pressed = 0;
	uint32_t		y_pressed = 0;
	uint32_t		start_pressed = 0;
	uint32_t		select_pressed = 0;	

	uint32_t		left_pressed = 0;
	uint32_t		right_pressed = 0;		
	uint32_t		up_pressed = 0;	
	uint32_t		down_pressed = 0;	

	SDL_Init(SDL_INIT_VIDEO);
	SDL_ShowCursor(SDL_DISABLE);
	TTF_Init();
	
	loadingScreen();
	
	// Prepare for Poll button input
	int			input_fd;
	input_fd = open("/dev/input/event0", O_RDONLY);
	setLayersInstall (0,0);
	loadRessources();
	
	
//	system("reboot");
	
	refreshScreen();
	
	struct input_event	ev;

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
										else
										if ( ev.code == KEY_UP ) {
												up_pressed = val;
										}
											else
												if ( ev.code == KEY_DOWN ) {
														down_pressed = val;
												}
												else
													if ( ev.code == BUTTON_X){
														x_pressed = val;
													}
													else
														if ( ev.code == BUTTON_Y){
															y_pressed = val;
														}											
		if (y_pressed) {			
			if (allActivated == 0){
				allActivated = 1; 
				setLayersInstall(1,1);
			}
			else {
				allActivated = 0; 
				setLayersInstall(0,1);
			}
			

			refreshScreen();
		}	 										
		if (right_pressed) {			
			if (nTab < 2){
				nTab ++;

				
				nSelection = 0; 
				nListPostion = 0;
				refreshScreen();
			}
		}	 
		if (left_pressed) {			
			if (nTab > 0){
				nTab --;

				nSelection = 0; 
				nListPostion = 0;
				refreshScreen();
			}
		}	
		
		
		if ((r2_pressed)&&(nb_Layers[nTab] > 0)) {			
			if ((nListPostion + 14) <nb_Layers[nTab]){
				nListPostion += 7;
			}
			else if ((nListPostion + 7) <nb_Layers[nTab]){
				nListPostion = nb_Layers[nTab] - 7;
				nSelection = 6 ;
			}
			refreshScreen();
		}	 
		
		if ((l2_pressed)&&(nb_Layers[nTab] > 0)) {			
			if ((nListPostion - 7) > 0) {
				nListPostion -= 7;
			}
			else {
				nListPostion = 0;
				nSelection = 0;
			
			}
			refreshScreen();
		}	
		
		if ((down_pressed)&&(nb_Layers[nTab] > 0)) {			
			if (nSelection < 6){
				nSelection ++;
			}
			else if ((nSelection+nListPostion) < nb_Layers[nTab]-1){
				nListPostion++;				
			}
			refreshScreen();
		}
		if ((up_pressed)&&(nb_Layers[nTab] > 0)) {			
			if (nSelection > 0){
				nSelection --;
			}
			else if (nListPostion > 0){
				nListPostion--;				
			}
			refreshScreen();
		}
	 
		if (b_pressed) {			
			break;
		}
		if ((a_pressed)&&(nb_Layers[nTab] > 0)) {		
			if (nListPostion+nSelection<nb_Layers[nTab]){
				if (bInstall[nTab][nListPostion+nSelection] == 1){
					bInstall[nTab][nListPostion+nSelection] = 0;
					bInstallChange[nTab][nListPostion+nSelection] = 1;
				}
				else{
					bInstall[nTab][nListPostion+nSelection] = 1;
					bInstallChange[nTab][nListPostion+nSelection] = 1;
		 		}
				if (nSelection < 6){
					nSelection ++;
				}
				else if ((nSelection+nListPostion) < nb_Layers[nTab]-1){
					nListPostion++;				
				}
				refreshScreen();
			}	
		}
		if (start_pressed) {	
		// installation
		char param1[250];
		char param2[60];
		char ressourcesPath[250];
		char cCommand[500];
		
		SDL_Surface* surfaceBackground = IMG_Load("./ressources/waitingBG.png");
		SDL_Surface* surfaceMessage;
		TTF_Font* font35 = TTF_OpenFont("/customer/app/Exo-2-Bold-Italic.ttf", 35);
		
		for (int nT = 0 ; nT < 3 ; nT ++){
			switch(nT){
				case 0 :
					sprintf(ressourcesPath,"%s","./data/Layer1");
					break;
				case 1 :
					sprintf(ressourcesPath,"%s","./data/Layer2");
					break;
				case 2 :
					sprintf(ressourcesPath,"%s","./data/Layer3");
					break;
			}
			
			sprintf(param1,"%s%d","/mnt/SDCARD/App/The_Onion_Installer/data/Layer",(nT+1));

			SDL_Rect rectMessage = { 10, 420 , 603, 48};
			
	
			
			for (int nLayer = 0 ; nLayer < nb_Layers[nT] ; nLayer++){
				if (bInstallChange[nT][nLayer] == 1){
					if (bInstall[nT][nLayer] == 1){
						surfaceMessage = TTF_RenderUTF8_Blended(font35, layers[nT][nLayer], color_white);
						SDL_BlitSurface(surfaceBackground, NULL, screen, NULL);
						SDL_BlitSurface(surfaceMessage, NULL, screen, &rectMessage); 
						SDL_FreeSurface(surfaceMessage);
						SDL_BlitSurface(screen, NULL, video, NULL); 
						SDL_Flip(video);
						
						sprintf(cCommand, "cd /mnt/SDCARD/App/The_Onion_Installer/ressources ; ./install.sh \"%s\" \"%s\"",param1, layers[nT][nLayer]);
						
						//logMessage(cCommand);
						system(cCommand);
					}
					else {
					// app uninstallation
					char pathAppUninstal[1000];
					pathAppUninstal[0]='\0';
					strcat(pathAppUninstal,param1);
					strcat(pathAppUninstal,"/");
					strcat(pathAppUninstal,layers[nT][nLayer]);
					//logMessage(pathAppUninstal);
					appUninstall(pathAppUninstal, nT, nLayer, strlen(pathAppUninstal));		
					// A second pass for deleting the empty folders
					appUninstall(pathAppUninstal, nT, nLayer, strlen(pathAppUninstal));		
					}				
				}
		
			}		
			
		}
			TTF_CloseFont(font35);	
			break;

		}
	
	}
	
	
	TTF_Quit();
	SDL_FreeSurface(surfaceCheck);
	SDL_FreeSurface(surfaceCross);
	SDL_FreeSurface(surfaceBackground);
	SDL_FreeSurface(surfaceBackground);
	SDL_FreeSurface(surfaceTableau);
	SDL_FreeSurface(surfaceSelection);
	SDL_FreeSurface(surfaceScroller);
	SDL_FreeSurface(surfacesTabSelection);

    return EXIT_SUCCESS;
}
