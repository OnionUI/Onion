#include <SDL/SDL.h>
#include <SDL/SDL_image.h>
#include <SDL/SDL_ttf.h>
#include <stdbool.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h> 

#include </usr/local/include/mi_sys.h>

bool file_exists (char *filename) {
  struct stat   buffer;   
  return (stat (filename, &buffer) == 0);
}
/*
void freeMMA(void){
	int		fd_fb;
	struct		fb_fix_screeninfo finfo;
	FILE		*fp;
	const char	*heapinfoname = "/proc/mi_modules/mi_sys_mma/mma_heap_name0";
	char		str[256];
	uint32_t	baseaddr, offset, length, usedflag;

	// print MMA information (before)
	fprintf(stdout, "------------------------------------------ before\n");
	fflush(stdout); system("cat /proc/mi_modules/mi_sys_mma/mma_heap_name0"); fflush(stdout);
	fprintf(stdout, "------------------------------------------ \n");

	// open FB to get physical address of FB (finfo.smem_start)
	fd_fb = open("/dev/fb0", O_RDONLY);
	ioctl(fd_fb, FBIOGET_FSCREENINFO, &finfo);
	baseaddr = finfo.smem_start - 0x021000;	// default baseaddr (tmp)
	fprintf(stdout, "MMA baseaddr(tmp) : %08X\n", baseaddr);
	// clear entire FB (in addition)
	MI_SYS_MemsetPa(finfo.smem_start, 0, finfo.smem_len);
	close(fd_fb);

	// open heap information file
	fp = fopen(heapinfoname, "r");
	if (fp) {
		// skip reading until chunk information
		do { if (fscanf(fp, "%255s", str) == EOF) { fclose(fp); return 0; } } while (strcmp(str,"sys-logConfig"));
		// get MMA each chunk information and release
		while(fscanf(fp, "%x %x %x %255s", &offset, &length, &usedflag, str) != EOF) {
			if (!usedflag) continue; // NA
			if (!strcmp(str,"fb_device")) { // FB .. fix baseaddr
				baseaddr = finfo.smem_start - offset;
				fprintf(stdout, "MMA baseaddr(fix) : %08X\n", baseaddr);
				continue;
			}
			if (!strcmp(str,"ao-Dev0-tmp")) continue; // ao .. Audio buffer, skip
			// For daemon program authors, MMA allocated as "daemon" will not be released (read daemon/main.c)
			if (strncmp(str,"daemon",6)) { // others except "daemon" .. release
				fprintf(stdout, "Released %s offset : %08X length : %08X\n", str, offset, length);
				MI_SYS_MMA_Free(baseaddr + offset);
			}
		}
		fclose(fp);


}
*/
int main(int argc, char *argv[]) {
	
	// Boot 

	// Boot : Loading screen
	// End_Save : Ending screen with save
	// End : Ending screen without save

    SDL_Init(SDL_INIT_VIDEO);

	TTF_Init();
	
	TTF_Font*	font; 
	SDL_Color	color={255,255,255,0};
	font = TTF_OpenFont("/customer/app/Exo-2-Bold-Italic.ttf", 30);
	SDL_Rect 	rectVersion = { 5, 445, 120, 40};

	
	SDL_Surface* video = SDL_SetVideoMode(640,480, 32, SDL_HWSURFACE);
	SDL_Surface* screen = SDL_CreateRGBSurface(SDL_HWSURFACE, 640,480, 32, 0,0,0,0);
	
	SDL_Surface* image;
	
	if (argc > 1){
		if (strcmp(argv[1],"Boot") == 0) {
			image = IMG_Load("bootScreen.png");
		}
		else if (strcmp(argv[1],"End_Save") == 0) {
			image = IMG_Load("Screen_Off_Save.png");
		}
		else if (strcmp(argv[1],"End") == 0) {
			image = IMG_Load("Screen_Off.png");
		}
		
	}
	else{
		image = IMG_Load("bootScreen.png");
	}
	
	char *cVersion;
	long lSize;
	FILE *fp;
	fp = fopen ( "/mnt/SDCARD/.tmp_update/onionVersion/version.txt" , "rb" );
	if( fp > 0 ) {
		fseek( fp , 0L , SEEK_END);
		lSize = ftell( fp );
		rewind( fp );
		cVersion = (char*)calloc( 1, lSize+1 );
		if( !cVersion ) fclose(fp),fputs("memory alloc fails",stderr),exit(1);
		
		if( 1!=fread( cVersion , lSize, 1 , fp) )
  		fclose(fp),free(cVersion),fputs("entire read fails",stderr),exit(1);
		fclose(fp);
	}
 
		
	SDL_Surface* imageVer = TTF_RenderUTF8_Blended(font, cVersion, color);
	
	SDL_BlitSurface(image, NULL, screen, NULL);
	SDL_BlitSurface(imageVer, NULL, screen, &rectVersion);
	
	SDL_BlitSurface(screen, NULL, video, NULL); 
	SDL_Flip(video);
	
	if (argc > 1){
		if (strcmp(argv[1],"Boot") != 0) {
				remove(".offOrder");
				system("sync");
				system("reboot");
				system("sleep 10");
		}	
	}

		
   	SDL_FreeSurface(screen);
   	SDL_FreeSurface(video);
    SDL_Quit();
	
    return EXIT_SUCCESS;
}
