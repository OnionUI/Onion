#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h> 
#include "cJSON.h"
#include <stdbool.h>  
#include <sys/stat.h>  
#include <fcntl.h>
#include <time.h>

// Max number of records in the DB
#define MAXVALUES 100

struct structom {                  /*struct called list*/
             char name[100]   ;
             int playTime ;
            }
            romList[MAXVALUES];     

int tailleStructure = 0;

void logMessage(char* Message) {
	FILE *file = fopen("/mnt/SDCARD/App/PlayActivity/log_lastMessage.txt", "a");

    char valLog[200];
    sprintf(valLog, "%s %s", Message, "\n");
    fputs(valLog, file);
	fclose(file); 
}

char *removeExt(char* myStr) {
    char *retStr;
    char *lastExt;
    if (myStr == NULL) return NULL;
    if ((retStr = malloc (strlen (myStr) + 1)) == NULL) return NULL;
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

void readRomDB(){
    FILE * file= fopen("/mnt/SDCARD/RetroArch/.retroarch/saves/playActivity.db", "rb");

	if (file != NULL) {

    	fread(romList, sizeof(romList), 1, file);
    	
     	tailleStructure = 0;
    	
    	for (int i=0; i<MAXVALUES; i++){
    		if (strlen(romList[i].name) == 0){
    			break;
    		}
    		tailleStructure++;
    	}
    	fclose(file);
	}

}

void writeRomDB(void){
	FILE * file= fopen("/mnt/SDCARD/RetroArch/.retroarch/saves/playActivity.db", "wb");
	if (file != NULL) {
    	fwrite(romList, sizeof(romList), 1, file);
    	fclose(file);
	}
}


void displayRomDB(void){
	logMessage("--------------------------------");
	for (int i = 0 ; i < tailleStructure ; i++) {	
			logMessage(romList[i].name);
			
			char cPlayTime[15];
			sprintf(cPlayTime, "%d", romList[i].playTime);
			logMessage(cPlayTime);
	 }
	logMessage("--------------------------------");

}
	
int searchRomDB(char* romName){
	int position = -1;
	
	for (int i = 0 ; i < tailleStructure ; i++) {
		if (strcmp(romList[i].name,romName) == 0){
			position = i;
			break;
		}
	}
	return position;
}

int main(int argc, char *argv[]) {
  	
	if (argc > 1){
		if (strcmp(argv[1],"init") == 0) {
			int epochTime = (int)time(NULL);
			char baseTime[15];
			sprintf(baseTime, "%d", epochTime);
			
			remove("initTimer");
			int init_fd = open("initTimer", O_CREAT | O_WRONLY);
			if (init_fd>0) {
        		write(init_fd, baseTime, strlen(baseTime));
        		close(init_fd);
        
			}			
		}
	
		else {
			FILE *fp;
			long lSize;
			char *baseTime;
			
			char *gameName = (char *)basename(argv[1]);
			gameName[99] = '\0';
			fp = fopen ( "initTimer" , "rb" );
			if( fp > 0 ) {
				fseek( fp , 0L , SEEK_END);
				lSize = ftell( fp );
				rewind( fp );
				baseTime = (char*)calloc( 1, lSize+1 );
				if( !baseTime ) fclose(fp),fputs("memory alloc fails",stderr),exit(1);
			
				if( 1!=fread( baseTime , lSize, 1 , fp) )
  				fclose(fp),free(baseTime),fputs("entire read fails",stderr),exit(1);
				fclose(fp);
	
				int iBaseTime = atoi(baseTime) ;
    			
    			int iEndEpochTime = (int)time(NULL);
				char cEndEpochTime[15];
				sprintf(cEndEpochTime, "%d", iEndEpochTime);

				char cTempsDeJeuSession[15];	
    			int iTempsDeJeuSession = iEndEpochTime - iBaseTime ;
    			sprintf(cTempsDeJeuSession, "%d", iTempsDeJeuSession);
				
	
				
				// Loading DB
				readRomDB();
    			 
    			//Addition of the new time
    			int totalPlayTime;
    			int searchPosition = searchRomDB(gameName);
    			if (searchPosition>=0){
    				// Game found
    				romList[searchPosition].playTime += iTempsDeJeuSession;
    				totalPlayTime = romList[searchPosition].playTime;
					}
    			else {
    				// Game inexistant, add to the DB	
    				if (tailleStructure<MAXVALUES-1){
    					romList[tailleStructure].playTime = iTempsDeJeuSession;
    					totalPlayTime = iTempsDeJeuSession ;
    					strcpy (romList[tailleStructure].name, gameName);	
    					tailleStructure ++;
    				}
    				else {
    					totalPlayTime = -1;
    				}	
    			}
    		
    			    			
    			// Write total current time for the onion launcher				 	
				char cTotalTimePlayed[50];	 	
				int totalTime_fd = open("currentTotalTime", O_CREAT | O_WRONLY);

				if (totalTime_fd > 0) {
				
					if (totalPlayTime >=  0) {				
						int h, m;
						
						h = (totalPlayTime/3600); 
						
						m = (totalPlayTime -(3600*h))/60;		

						//sprintf(cTotalTimePlayed, "%02d:%02d:%02d", h,m,s);
					 	sprintf(cTotalTimePlayed, "%dh%02dm", h,m);
					 	//logMessage(cTotalTimePlayed);
					 	
					}
					else {
						if (totalPlayTime == -1) {
							// DB full, needs to be cleaned
						strcpy(cTotalTimePlayed,"DB:FU");
						}
						else{
						strcpy(cTotalTimePlayed,"ERROR");
						
						}
					}
					write(totalTime_fd, cTotalTimePlayed, strlen(cTotalTimePlayed));
        			close(totalTime_fd);		
				}
				
	
   				// We save the DB

   				writeRomDB();		
    			//displayRomDB();
	
			}
		
		}		
		
	}
	
    return EXIT_SUCCESS;
}
