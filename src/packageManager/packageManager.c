#include "utils/log.h"
#include "utils/msleep.h"

#include "./apply.h"
#include "./changes.h"
#include "./fileActions.h"
#include "./globals.h"
#include "./keystateHandler.h"
#include "./listActions.h"
#include "./render.h"
#include "./summary.h"

int main(int argc, char *argv[])
{
    log_setName("packageManager");

    bool show_confirm = false;
    bool auto_update = false;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--confirm") == 0)
            show_confirm = true;
        else if (strcmp(argv[i], "--auto_update") == 0)
            auto_update = true;
        else {
            printf("unknown argument: %s\n", argv[i]);
            return EXIT_FAILURE;
        }
    }

    initResources();

    if (!auto_update) {
        SDL_Surface *loadingScreen = IMG_Load("res/loading.png");
        SDL_BlitSurface(loadingScreen, NULL, screen, NULL);
        SDL_BlitSurface(screen, NULL, video, NULL);
        SDL_Flip(video);
        SDL_FreeSurface(loadingScreen);
    }

    loadPackages(auto_update);

    if (show_confirm) {
        layerToggleAll(0, true);
    }

    bool quit = false;
    bool state_changed = true;
    bool apply_changes = false;

    if (auto_update) {
        apply_singleIcon("/mnt/SDCARD/App/PackageManager/config.json");
        quit = true;
        apply_changes = true;
    }

    while (!quit) {
        state_changed |= keystateHandler(&quit, &apply_changes, show_confirm, auto_update);

        if (quit)
            break;

        if (state_changed) {
            renderApplication();
            state_changed = false;
        }
    }

    if (apply_changes) {
        applyAllChanges(auto_update);
    }

#ifndef PLATFORM_MIYOOMINI
    msleep(200);
#endif

    freeResources();

    return EXIT_SUCCESS;
}
