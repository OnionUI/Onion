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

#include "utils/utils.h"
#include "system/battery.h"
#include "utils/IMG_Save.h"
#include "system/settings.h"
#include "theme/theme.h"

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
    bool icon_exists = theme_getImagePath(theme, "power-full-icon", icon_path) == 1;
    bool backup_exists = theme_getImagePath(theme, "power-full-icon_back", icon_backup) == 1;

    // Restore regular battery display
    if (icon_exists && backup_exists) {
        char systemCommand[256*2+32*2+8];
        remove(icon_path);
        file_copy(icon_backup, icon_path);
        remove(icon_backup);
    }
}

void drawBatteryPercentage(Theme_s *theme)
{
    char icon_path[STR_MAX],
         icon_backup[STR_MAX];
    bool icon_location = theme_getImagePath(theme, "power-full-icon", icon_path);
    bool backup_location = theme_getImagePath(theme, "power-full-icon_back", icon_backup);

    // Backup old battery icon
    if (icon_location != backup_location) {
        sprintf(icon_backup, "%s_back.png", file_removeExtension(icon_path));
        file_copy(icon_path, icon_backup);
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
}

int main(int argc, char *argv[])
{
    settings_load();
    Theme_s theme = loadThemeFromPath(settings.theme);
    if (argc > 1 && strcmp(argv[1], "--restore") == 0)
        restoreRegularDisplay(&theme);
    else if (!battery_isCharging())
        drawBatteryPercentage(&theme);
    return 0;
}
