#ifndef PACMAN_APPLY_H__
#define PACMAN_APPLY_H__

#include "utils/file.h"
#include "utils/log.h"

#include "./fileActions.h"
#include "./globals.h"

void applyAllChanges(bool auto_update)
{
    // installation
    char cmd[STR_MAX * 2 + 100];

    SDL_Surface *surfaceBackground = IMG_Load("/mnt/SDCARD/.tmp_update/res/waitingBG.png");
    SDL_Surface *surfaceMessage;

    for (int nT = 0; nT < tab_count; nT++) {
        const char *data_path = layer_dirs[nT];

        if (strlen(data_path) == 0 || !exists(data_path))
            continue;

        SDL_Rect rectMessage = {10, 420, 603, 48};

        for (int nLayer = 0; nLayer < package_count[nT]; nLayer++) {
            Package *package = &packages[nT][nLayer];

            bool should_apply =
                auto_update || (package->installed && !package->complete) ||
                package->changed;
            bool should_install = package->installed != package->changed ||
                                  (package->installed &&
                                   !package->complete && !package->changed);

            if (!should_apply)
                continue;

            if (should_install) {
                printf_debug("Installing %s...\n", package->name);
                SDL_BlitSurface(surfaceBackground, NULL, screen, NULL);

                surfaceMessage = TTF_RenderUTF8_Blended(
                    font35, package->name, color_white);
                SDL_BlitSurface(surfaceMessage, NULL, screen, &rectMessage);
                SDL_FreeSurface(surfaceMessage);

                SDL_BlitSurface(screen, NULL, video, NULL);
                SDL_Flip(video);

                sprintf(cmd, "/mnt/SDCARD/.tmp_update/script/pacman_install.sh \"%s\" \"%s\"", data_path, package->name);
                system(cmd);
                sync();

                callPackageInstaller(data_path, package->name, true);
            }
            else if (package->installed) {
                printf_debug("Removing %s...\n", package->name);
                callPackageInstaller(data_path, package->name, false);

                // app removal
                char pathAppUninstall[1000];
                sprintf(pathAppUninstall, "%s/%s", data_path, package->name);
                appUninstall(pathAppUninstall, strlen(pathAppUninstall));
            }
        }
    }
}

#endif // PACMAN_APPLY_H__
