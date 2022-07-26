#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <linux/input.h>
#include <SDL/SDL.h>
#include <SDL/SDL_image.h>
#include <SDL/SDL_ttf.h>

#include "shmvar/shmvar.h"

#include "../common/utils.h"
#include "../common/IMG_Save.h"
#include "../common/theme.h"
  
#define SARADC_IOC_MAGIC 'a'
#define IOCTL_SAR_INIT _IO(SARADC_IOC_MAGIC, 0)
#define IOCTL_SAR_SET_CHANNEL_READ_VALUE _IO(SARADC_IOC_MAGIC, 1)

void logMessage(char* message) {
	FILE *file = fopen("/mnt/SDCARD/.tmp_update/log_mainUiBatPerc.txt", "a");
	fprintf(file, "%s%s", message, "\n");
	fclose(file);
}

typedef struct {
	int channel_value;
	int adc_value;
} SAR_ADC_CONFIG_READ;

static SAR_ADC_CONFIG_READ adcCfg = {0,0};
static int sar_fd = 0;

const char* getBatteryIcon(int percentage)
{
	if (percentage <= 10)
		return THEME_POWER_0;
	if (percentage <= 25)
		return THEME_POWER_20;
	if (percentage <= 60)
		return THEME_POWER_50;
	if (percentage <= 90)
		return THEME_POWER_80;
	return THEME_POWER_100 "_back";
}

void restoreRegularDisplay(Theme_s* theme)
{
	char icon_path[STR_MAX],
		 icon_backup[STR_MAX];
	theme_getImagePath(theme, THEME_POWER_100, icon_path);
	theme_getImagePath(theme, THEME_POWER_100 "_back", icon_backup);

	// Regular battery display
	if (file_exists(icon_backup)) {
		char systemCommand[256*2+32*2+8];
		remove(icon_path);
		sprintf(systemCommand, "cp \"%s\" \"%s\"", icon_backup, icon_path);
		system(systemCommand);
		remove(icon_backup);
	}

	sar_fd = open("/dev/sar", O_WRONLY);
	ioctl(sar_fd, IOCTL_SAR_INIT, NULL);
	ioctl(sar_fd, IOCTL_SAR_SET_CHANNEL_READ_VALUE, &adcCfg);
	KeyShmInfo info;
	InitKeyShm(&info);
	SetKeyShm(&info, MONITOR_ADC_VALUE, adcCfg.adc_value);
	UninitKeyShm(&info);
}

int getBatteryPercentage()
{
	int percentage = 0;

	if (file_exists("/tmp/percBat")) {
		char val[5];
		const char *cPercBat = loadFile("/tmp/percBat");
		strcpy(val, cPercBat);
		percentage = atoi(val);
	}

	return percentage;
}

int drawBatteryPercentage(Theme_s *theme)
{
	BatteryPercentage_s* style = &theme->batteryPercentage;
	int percentage = getBatteryPercentage();

	// Cancel if currently charging
	if (percentage == 500)
		return 1;

	TTF_Init();
	TTF_Font* font = theme_loadFont(theme, style->font, style->size);

	// Correct Exo 2 font offset
	if (strncmp(TTF_FontFaceFamilyName(font), "Exo 2", 5) == 0)
		style->offset -= 0.075 * TTF_FontHeight(font);
	
	char icon_path[STR_MAX],
		 icon_backup[STR_MAX];
	theme_getImagePath(theme, THEME_POWER_100, icon_path);
	theme_getImagePath(theme, THEME_POWER_100 "_back", icon_backup);

	// Backup old icon
	if (!file_exists(icon_backup)) {
		char systemCommand[256*2+32*2+8];
		sprintf(systemCommand, "cp \"%s\" \"%s\"", icon_path, icon_backup);
		system(systemCommand);
	}

	// Battery percentage text
	char buffer[5];
	sprintf(buffer, "%d%%", percentage);
	SDL_Surface *text = TTF_RenderUTF8_Blended(font, buffer, style->color);
	SDL_SetAlpha(text, 0, 0); /* important */

	// Battery icon
	char real_path[STR_MAX];
	theme_getImagePath(theme, getBatteryIcon(percentage), real_path);
	SDL_Surface *icon = IMG_Load(real_path);
	SDL_SetAlpha(icon, 0, 0); /* important */

	const int SPACER = 5;
	int img_width = 2 * (text->w + SPACER) + icon->w;
	int img_height = text->h > icon->h ? text->h : icon->w;

	if (!style->visible) {
		img_width = icon->w;
		img_height = icon->h;
	}

	SDL_Surface *image = SDL_CreateRGBSurface(0, img_width, img_height, 32,
		0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000); /* important */

	SDL_Rect rect_icon = {0, (img_height - icon->h) / 2};
	SDL_Rect rect_text = {icon->w + SPACER, (img_height - text->h) / 2 + style->offset};

	if (style->visible && style->onleft) {
		rect_text.x = 0;
		rect_icon.x = text->w + SPACER;
	}

	SDL_BlitSurface(icon, NULL, image, &rect_icon);

	if (style->visible)
		SDL_BlitSurface(text, NULL, image, &rect_text);

	// Save custom battery icon
	IMG_Save(image, icon_path);

	SDL_FreeSurface(text);
	SDL_FreeSurface(image);
	SDL_FreeSurface(icon);

	// Free font resources
	TTF_CloseFont(font);
	TTF_Quit();

	KeyShmInfo info;
	InitKeyShm(&info);
	SetKeyShm(&info, MONITOR_ADC_VALUE, 640);
	UninitKeyShm(&info);

	return 0;
}

int main(void)
{
	Theme_s theme = loadTheme();
	drawBatteryPercentage(&theme);
	logMessage("mainUiBatPerc ran successfully");
	return 0;
}
