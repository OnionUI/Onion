#include <stdio.h>
#include <unistd.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>  
#include <sys/stat.h>
#include <dirent.h>
#include <SDL/SDL.h>
#include <SDL/SDL_image.h>
#include <SDL/SDL_ttf.h>

#define MAX_LEN 33

bool file_exists(char *filename)
{
	struct stat buffer;   
	return (stat (filename, &buffer) == 0);
}

void read_last_line(const char* filename, char* out_str)
{
	FILE *fd;
	char buff[MAX_LEN + 1];
	char* pch = NULL;

	if ((fd = fopen(filename, "rb")) != NULL) {
		fseek(fd, -MAX_LEN, SEEK_END);
		fread(buff, MAX_LEN-1, 1, fd);
		fclose(fd);
		buff[MAX_LEN-1] = '\0';

		pch = strtok(buff, "\n");
		while (pch != NULL) {
			if (strlen(pch) > 0)
				sprintf(out_str, "%s", pch);
			pch = strtok(NULL, "\n");
		}
	}
}

int main(void)
{
	SDL_Init(SDL_INIT_VIDEO);
	SDL_ShowCursor(SDL_DISABLE);
	TTF_Init();

	SDL_Surface* video = SDL_SetVideoMode(640, 480, 32, SDL_HWSURFACE);
	SDL_Surface* screen = SDL_CreateRGBSurface(SDL_HWSURFACE, 640, 480, 32, 0, 0, 0, 0);
	
	SDL_Surface* image = IMG_Load("waitingBG.png");

	TTF_Font* font = TTF_OpenFont("/mnt/SDCARD/miyoo/app/Exo-2-Bold-Italic.ttf", 36);
	SDL_Color color_white = {255, 255, 255, 0};
	SDL_Rect rectMessage = {10, 420 , 603, 48};
	SDL_Surface* surfaceMessage;
	bool exit = false;

	while (!exit) {
		char msg[255];
		SDL_BlitSurface(image, NULL, screen, NULL);
		
		if (file_exists(".installed")) {
			sprintf(msg, "Finishing...");
			exit = true;
		}
		else if (file_exists(".installFailed")) {
			sprintf(msg, "Installation failed!");
			exit = true;
		}
		else if (file_exists(".update_msg")) {
			read_last_line(".update_msg", msg);
		}
		else {
			sprintf(msg, "Preparing...");
		}
		
		surfaceMessage = TTF_RenderUTF8_Blended(font, msg, color_white);		
		SDL_BlitSurface(surfaceMessage, NULL, screen, &rectMessage);
	
		SDL_BlitSurface(screen, NULL, video, NULL); 
		SDL_Flip(video);
		
		sleep(0.5);
	}
	
   	SDL_FreeSurface(screen);
   	SDL_FreeSurface(surfaceMessage);
   	SDL_FreeSurface(video);
    SDL_Quit();
	
    return EXIT_SUCCESS;
}
