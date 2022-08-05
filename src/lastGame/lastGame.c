#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h> 
#include <stdbool.h>  
#include <sys/stat.h>  
#include <fcntl.h>

#include "cjson/cJSON.h"


void logMessage(char* Message) {
	FILE *file = fopen("/mnt/SDCARD/.tmp_update/log_lastMessage.txt", "a");
	/*char tempMess[] = "\r\n";
    strcat(Message,tempMess);
    */
    char valLog[200];
    sprintf(valLog, "%s%s", Message, "\n");
    fputs(valLog, file);
	fclose(file); 
}

char *file_removeExtension(char* myStr) {
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
    FILE * f = fopen (path, "rb"); //was "rb"

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

int main(void) {
  	
	//logMessage(" ");

	cJSON* request_json = NULL;
	cJSON* items = cJSON_CreateArray();
	cJSON* path = NULL;
	cJSON* core_path = NULL;
	
	remove ("/tmp/cmd_to_run_launcher.sh");
	remove ("/tmp/romName.txt");
	
	const char *request_body = load_file("/mnt/SDCARD/Saves/CurrentProfile/lists/content_history.lpl");
	
	request_json = cJSON_Parse(request_body);
	
	items = cJSON_GetObjectItem(request_json, "items");
	
	cJSON * subitem = cJSON_GetArrayItem(items, 0);
    path = cJSON_GetObjectItem(subitem, "path");
    core_path = cJSON_GetObjectItem(subitem, "core_path");

	//logMessage(cJSON_Print(core_path));
	//logMessage(cJSON_Print(path));

	
	char *cPath = cJSON_Print(path) ;
	char *cCore_path = cJSON_Print(core_path) ;
	
	if ((strlen(cCore_path)>1) && (strlen(cPath)>1)) {
		// Quote character removal
		
		char *cCore_path1 = cCore_path+1;
		char *cPath1 = cPath+1;
		
		cCore_path1[strlen(cCore_path1)-1] = '\0';
		cPath1[strlen(cPath1)-1] = '\0';

		//logMessage(cCore_path1);
		//logMessage(cPath1);


		if ((file_exists(cCore_path1) == 1) && (file_exists(cPath1) == 1)){
		
			FILE *file = fopen("/tmp/cmd_to_run_launcher.sh", "w");
			 if (file_exists("RADirectLaunch.enable") == 1){
				fputs("LD_PRELOAD=/mnt/SDCARD/miyoo/lib/libpadsp.so ./retroarch", file);
				}
			else {
				fputs("LD_PRELOAD=/mnt/SDCARD/miyoo/lib/libpadsp.so ./retroarch -v -L ", file);
				fputs(cJSON_Print(core_path), file);
				fputs(" ", file);
				fputs(cJSON_Print(path), file);
			}
			fclose(file); 

		
			// Rom name copy for timers
			char *bname;
    		char *path2 = strdup(cJSON_Print(path));
       		
			//  Complete path  
   			//logMessage(path2);
    		
    		// File name
    		bname = (char *)basename(path2);
    		
    		// Cut the last " character
    		if (strlen( bname ) > 0){
    			bname[strlen( bname )-1] = '\0';
    		}
  			bname = (char *)basename(path2);  
    		
    		//File name without ext
			//char *bnameWOExt = file_removeExtension(bname);
		
			remove("/tmp/romName.txt");
			FILE *fileRom = fopen("/tmp/romName.txt", "w");
			fputs(bname, fileRom);
			fclose(fileRom); 
		}
		else {
			remove ("/tmp/cmd_to_run_launcher.sh");
			remove ("/tmp/romName.txt");
		}

	}
	else {
	remove ("/tmp/cmd_to_run_launcher.sh");
	remove ("/tmp/romName.txt");
	}
	

    return EXIT_SUCCESS;
}
