

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h> 
#include "cJSON.h"
#include <stdbool.h>  
#include <sys/stat.h>  
#include <fcntl.h>


void logMessage(char* Message) {
	FILE *file = fopen("/mnt/SDCARD/.tmp_update/log_lastMessage.txt", "a");
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
	
	const char *request_body = load_file("/mnt/SDCARD/RetroArch/content_history.lpl");
	
	request_json = cJSON_Parse(request_body);
	 
	items = cJSON_GetObjectItem(request_json, "items");
	
	cJSON * subitem = cJSON_GetArrayItem(items, 0);
    path = cJSON_GetObjectItem(subitem, "path");
    core_path = cJSON_GetObjectItem(subitem, "core_path");

	FILE *file = fopen("/mnt/SDCARD/.tmp_update/RACommand.txt", "w");

	fputs("./retroarch -v -L ", file);
	fputs(cJSON_Print(core_path), file);
	fputs(" ", file);
	fputs(cJSON_Print(path), file);
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
	//char *bnameWOExt = removeExt(bname);

	remove("/mnt/SDCARD/.tmp_update/romName.txt");
	FILE *fileRom = fopen("/mnt/SDCARD/.tmp_update/romName.txt", "w");
	fputs(bname, fileRom);
	fclose(fileRom); 


    return EXIT_SUCCESS;
}
