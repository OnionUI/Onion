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
#include "utils/log.h"
#include "utils/IMG_Save.h"
#include "system/battery.h"
#include "system/settings.h"
#include "theme/theme.h"
#include "theme/background.h"

void restoreRegularDisplay(void)
{
    char theme_path[STR_MAX];
    theme_getPath(theme_path);

    char icon_path[STR_MAX],
         icon_backup[STR_MAX];
    bool icon_exists = theme_getImagePath(theme_path, "power-full-icon", icon_path) == 1;
    bool backup_exists = theme_getImagePath(theme_path, "power-full-icon_back", icon_backup) == 1;

    // Restore regular battery display
    if (icon_exists && backup_exists) {
        remove(icon_path);
        file_copy(icon_backup, icon_path);
        remove(icon_backup);
    }
}

void drawBatteryPercentage(void)
{
    char theme_path[STR_MAX];
    theme_getPath(theme_path);

    char icon_path[STR_MAX],
         icon_backup[STR_MAX];
    bool icon_location = theme_getImagePath(theme_path, "power-full-icon", icon_path);
    bool backup_location = theme_getImagePath(theme_path, "power-full-icon_back", icon_backup);

    // Backup old battery icon
    if (icon_location != backup_location) {
        sprintf(icon_backup, "%s_back.png", file_removeExtension(icon_path));
        file_copy(icon_path, icon_backup);
    }

    TTF_Init();

    int percentage = battery_getPercentage();
    SDL_Surface *image = theme_batterySurfaceWithBg(percentage, theme_background());

    // Save custom battery icon
    if (percentage != 500)
        IMG_Save(image, icon_path);

    SDL_FreeSurface(image);
	resources_free();
    TTF_Quit();
}

int main(int argc, char *argv[])
{
    if (argc > 1 && strcmp(argv[1], "--restore") == 0)
        restoreRegularDisplay();
    else if (!battery_isCharging())
        drawBatteryPercentage();
    return 0;
}
