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

#include "../common/battery.h"
#include "../common/utils.h"
#include "../common/IMG_Save.h"
#include "../common/theme.h"
#include "../common/theme_resources.h"
#include "../common/theme_render.h"

#define RESOURCES { \
	TR_BATTERY_0, \
	TR_BATTERY_20, \
	TR_BATTERY_50, \
	TR_BATTERY_80, \
	TR_BATTERY_100, \
	TR_BATTERY_CHARGING \
}
#define NUM_RESOURCES 6


void logMessage(char* message) {
    FILE *file = fopen("/mnt/SDCARD/.tmp_update/log_mainUiBatPerc.txt", "a");
    fprintf(file, "%s%s", message, "\n");
    fclose(file);
}

void restoreRegularDisplay(Theme_s* theme)
{
    char icon_path[STR_MAX],
         icon_backup[STR_MAX];
    bool icon_exists = theme_getImagePath(theme, "power-full-icon", icon_path);
    bool backup_exists = theme_getImagePath(theme, "power-full-icon_back", icon_backup);

    // Restore regular battery display
    if (icon_exists && backup_exists) {
        char systemCommand[256*2+32*2+8];
        remove(icon_path);
        sprintf(systemCommand, "cp \"%s\" \"%s\"", icon_backup, icon_path);
        system(systemCommand);
        remove(icon_backup);
    }

    battery_monitorADC_on();
}

void drawBatteryPercentage(Theme_s *theme)
{
    char icon_path[STR_MAX],
         icon_backup[STR_MAX];
    bool icon_exists = theme_getImagePath(theme, "power-full-icon", icon_path);
    bool backup_exists = theme_getImagePath(theme, "power-full-icon_back", icon_backup);

    // Backup old battery icon
    if (icon_exists && !backup_exists || !icon_exists && !file_exists(icon_backup)) {
        char systemCommand[256*2+32*2+8];
        sprintf(systemCommand, "cp \"%s\" \"%s\"", icon_path, icon_backup);
        system(systemCommand);
    }

    TTF_Init();
    enum theme_Images res_requests[NUM_RESOURCES] = RESOURCES;
	Resources_s res = theme_loadResources(theme, res_requests, NUM_RESOURCES);

    int percentage = battery_getPercentage();
    SDL_Surface* image = theme_batterySurface(theme, &res, percentage);

    // Save custom battery icon
    IMG_Save(image, icon_path);

    SDL_FreeSurface(image);
	theme_freeResources(&res);
    TTF_Quit();

    battery_monitorADC_off();
}

int main(void)
{
    Theme_s theme = loadTheme();
    drawBatteryPercentage(&theme);
    return 0;
}
