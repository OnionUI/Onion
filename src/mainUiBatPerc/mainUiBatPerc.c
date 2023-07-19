#include <SDL/SDL.h>
#include <SDL/SDL_image.h>
#include <SDL/SDL_ttf.h>
#include <fcntl.h>
#include <linux/input.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <unistd.h>

#include "system/battery.h"
#include "system/settings.h"
#include "theme/background.h"
#include "theme/theme.h"
#include "utils/IMG_Save.h"
#include "utils/file.h"
#include "utils/log.h"
#include "utils/str.h"

void restoreRegularDisplay(void)
{
    print_debug(":: restoreRegularDisplay");

    char icon_path[STR_MAX + 20],
        icon_backup[STR_MAX],
        theme_path[STR_MAX];
    theme_getPath(theme_path);

    printf_debug("theme_path: %s\n", theme_path);

    bool icon_exists = theme_getImagePath(theme_path, "power-full-icon", icon_path) == 1;
    bool backup_exists = theme_getImagePath(theme_path, "power-full-icon_back", icon_backup) == 1;

    // Restore regular battery display
    if (icon_exists && backup_exists) {
        remove(icon_path);
        file_copy(icon_backup, icon_path);
        remove(icon_backup);
    }

    printf_debug("icon_path: %s (exists: %d)\n", icon_path, icon_exists);
    printf_debug("icon_backup: %s (exists: %d)\n", icon_backup, backup_exists);
}

void drawBatteryPercentage(void)
{
    print_debug(":: drawBatteryPercentage");

    char theme_path[STR_MAX];
    theme_getPath(theme_path);

    printf_debug("theme_path: %s\n", theme_path);

    char icon_path[STR_MAX + 20];
    snprintf(icon_path, STR_MAX + 19, "%sskin/.batt-perc.png", theme_path);

    printf_debug("icon_path: %s\n", icon_path);

    TTF_Init();

    int percentage = battery_getPercentage();
    SDL_Surface *image = theme_batterySurfaceWithBg(percentage, theme_background());

    // Save custom battery icon
    if (image != NULL && percentage != 500)
        IMG_Save(image, icon_path);

    SDL_FreeSurface(image);
    resources_free();
    TTF_Quit();
}

int main(int argc, char *argv[])
{
    // Repair themes modified with the previous logic
    // and make sure the percentage resource exists
    restoreRegularDisplay();
    drawBatteryPercentage();
    return 0;
}
