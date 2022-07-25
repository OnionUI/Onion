#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <linux/input.h>
#include <SDL/SDL.h>
#include <SDL/SDL_image.h>
#include <SDL/SDL_ttf.h>
  
#include <png.h>
#include "cJSON/cJSON.h"
  
#define SARADC_IOC_MAGIC			'a'
#define IOCTL_SAR_INIT				_IO(SARADC_IOC_MAGIC, 0)
#define IOCTL_SAR_SET_CHANNEL_READ_VALUE	_IO(SARADC_IOC_MAGIC, 1)

void logMessage(char* Message) {
	FILE *file = fopen("/mnt/SDCARD/.tmp_update/log_mainUiBatPerc.txt", "a");

	char valLog[200];
	sprintf(valLog, "%s%s", Message, "\n");
	fputs(valLog, file);
	fclose(file);
}

typedef struct {
	int channel_value;
	int adc_value;
} SAR_ADC_CONFIG_READ;

static SAR_ADC_CONFIG_READ adcCfg = {0,0};
static int sar_fd = 0;

//	libshmvar header
typedef enum {
	MONITOR_VOLUME,			// vol
	MONITOR_BRIGHTNESS,		// brightness
	MONITOR_KEYMAP,			// keymap (maybe unused)
	MONITOR_MUTE,			// mute
	MONITOR_VOLUME_CHANGED,		// volume change (internal use)
	MONITOR_BGM_VOLUME,		// bgmvol
	MONITOR_HIBERNATE_DELAY,	// hibernate
	MONITOR_ADC_VALUE,		// charging state (internal use)
	MONITOR_LUMINATION,		// lumination
	MONITOR_HUE,			// hue
	MONITOR_SATURATION,		// saturation
	MONITOR_CONTRAST,		// contrast
	MONITOR_UNUSED,			// (unused)
	MONITOR_AUDIOFIX,		// audiofix
	MONITOR_VALUE_MAX,
} MonitorValue;

typedef struct _KeyShmInfo {
	int id;
	void *addr;
} KeyShmInfo;

int	InitKeyShm(KeyShmInfo *);
int	SetKeyShm(KeyShmInfo* info, MonitorValue key, int value);
int	GetKeyShm(KeyShmInfo* info, MonitorValue key);
int	UninitKeyShm(KeyShmInfo *);

bool file_exists (char *filename) {
	struct stat buffer;
	return (stat(filename, &buffer) == 0);
}

char* load_file (char const* path)
{
	char* buffer = 0;
	long length = 0;
	FILE * f = fopen(path, "rb");

	if (f)
	{
		fseek(f, 0, SEEK_END);
		length = ftell(f);
		fseek(f, 0, SEEK_SET);
		buffer = (char*)malloc((length+1)*sizeof(char));
		if (buffer)
		{
			fread(buffer, sizeof(char), length, f);
		}
		fclose(f);
	}
	buffer[length] = '\0';

	return buffer; 
}

void IMG_SavePNG (SDL_Surface *SurfaceImage, char* pathImage){
	int width = SurfaceImage->w;
	int height = SurfaceImage->h;

	png_structp	png_ptr;
	png_infop	info_ptr;
	FILE		*fp;

	fp = fopen(pathImage, "wb");

	if (fp) {
		Uint32* linebuffer = malloc(SurfaceImage->pitch);
		Uint32* src = SurfaceImage->pixels;

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

int main (void) {

	int percBat =0 ;
	char val[5];
	cJSON* themePath;

	char cThemePath[256];
	char cBatIconPath[256+32];
	char cBatIconPathBack[256+32];

	int nRed = 255;
	int nGreen = 255;
	int nBlue = 255;

	cJSON* request_json = NULL;
	int nBatteryPercentagesDisplay = 0;
	// Search for the percfentage option
	if (file_exists("/appconfigs/system.json") == 1){

		char *request_body = load_file("/appconfigs/system.json");

		if (request_body != NULL){
			char cConfigThemePath[256+16];

			request_json = cJSON_Parse(request_body);
			themePath = cJSON_GetObjectItem(request_json, "theme");

			strcpy(cThemePath, cJSON_GetStringValue(themePath));
			sprintf(cConfigThemePath, "%sconfig.json", cThemePath);

			sprintf(cBatIconPath, "%sskin/power-full-icon.png", cThemePath);

			sprintf(cBatIconPathBack, "%sskin/power-full-icon_back.png", cThemePath);

			if (file_exists(cConfigThemePath) == 1){
				char *request_body_Theme = load_file(cConfigThemePath);
				cJSON* request_json_theme = NULL;
				cJSON* batteryPercentage;

				cJSON* batVisible;
				cJSON* batRed;
				cJSON* batGreen;
				cJSON* batBlue;

				if (request_body_Theme != NULL){

					request_json_theme = cJSON_Parse(request_body_Theme);

					batteryPercentage = cJSON_GetObjectItem(request_json_theme, "batteryPercentage");

					batVisible = cJSON_GetObjectItem(batteryPercentage, "visible");

					batRed = cJSON_GetObjectItem(batteryPercentage, "red");

					batGreen = cJSON_GetObjectItem(batteryPercentage, "green");
					batBlue = cJSON_GetObjectItem(batteryPercentage, "blue");

					nRed = cJSON_GetNumberValue(batRed);
					nGreen = cJSON_GetNumberValue(batGreen);
					nBlue = cJSON_GetNumberValue(batBlue);

					if(cJSON_IsTrue(batVisible)){
						nBatteryPercentagesDisplay = 1;
					}
					else {
						nBatteryPercentagesDisplay = 0;
					}
					free(request_body_Theme);
				}

			}
			free(request_body);
		}

	}

	if (nBatteryPercentagesDisplay == 0) {
		// Regular battery display
		if (file_exists(cBatIconPathBack) == 1) {
			char systemCommand[256*2+32*2+8];
			remove(cBatIconPath);
			sprintf(systemCommand, "cp \"%s\" \"%s\"", cBatIconPathBack, cBatIconPath);
			system(systemCommand);
			remove(cBatIconPathBack);
		}

		sar_fd = open("/dev/sar", O_WRONLY);
		ioctl(sar_fd, IOCTL_SAR_INIT, NULL);
		ioctl(sar_fd, IOCTL_SAR_SET_CHANNEL_READ_VALUE, &adcCfg);
		KeyShmInfo info;
		InitKeyShm(&info);
		SetKeyShm(&info, MONITOR_ADC_VALUE, adcCfg.adc_value);
		UninitKeyShm(&info);

	} else {
		// Draw percentage for MainUI
		if (file_exists("/tmp/percBat") == 1) {
			char *cPercBat = load_file("/tmp/percBat");
			strcpy(val, cPercBat);
			free(cPercBat);
			percBat = atoi(val);
		}
		if (percBat != 500) {
			// Not charging
			TTF_Init();
			TTF_Font* font = TTF_OpenFont("/customer/app/Exo-2-Bold-Italic.ttf", 25);
			SDL_Color color = {nRed, nGreen, nBlue, 0};
			strcat(val, "%");
			SDL_Surface *imagePerc = TTF_RenderUTF8_Blended(font, val, color);
			SDL_SetAlpha(imagePerc, 0, 0); /* important */

			if(file_exists(cBatIconPathBack) == 0) {
				char systemCommand[256*2+32*2+8];
				sprintf(systemCommand, "cp \"%s\" \"%s\"", cBatIconPath, cBatIconPathBack);
				system(systemCommand);
			}

			if (percBat <= 10) {
				sprintf(cBatIconPath, "%sskin/power-0%%-icon.png", cThemePath);
			} else if (percBat <= 20) {
				sprintf(cBatIconPath, "%sskin/power-20%%-icon.png", cThemePath);
			} else if (percBat <= 50) {
				sprintf(cBatIconPath, "%sskin/power-50%%-icon.png", cThemePath);
			} else if (percBat <= 80) {
				sprintf(cBatIconPath, "%sskin/power-80%%-icon.png", cThemePath);
			} else {
				sprintf(cBatIconPath, "%sskin/power-full-icon_back.png", cThemePath);
			}

			SDL_Surface *imageBatteryIcon = imageBatteryIcon = IMG_Load(cBatIconPath);
			SDL_SetAlpha(imageBatteryIcon, 0, 0); /* important */

			SDL_Surface *imageBatFinal = SDL_CreateRGBSurface(0, imagePerc->w+80, imagePerc->h, 32,
				0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000); /* important */

			SDL_Rect rectPercBat = { 55, 0,imagePerc->w, imagePerc->h};
			SDL_Rect rectBatIcon = { 0, -7, 48, 48};

			SDL_BlitSurface(imagePerc, NULL, imageBatFinal, &rectPercBat);
			SDL_BlitSurface(imageBatteryIcon, NULL, imageBatFinal, &rectBatIcon);

			char cMiyooBatIconPath[256+24];
			sprintf(cMiyooBatIconPath, "%sskin/power-full-icon.png", cThemePath);

			IMG_SavePNG(imageBatFinal, cMiyooBatIconPath);

			SDL_FreeSurface(imagePerc);
			SDL_FreeSurface(imageBatFinal);
			SDL_FreeSurface(imageBatteryIcon);

			KeyShmInfo info;
			InitKeyShm(&info);
			SetKeyShm(&info, MONITOR_ADC_VALUE, 640);
			UninitKeyShm(&info);

		}

	}

}
