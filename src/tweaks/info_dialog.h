#ifndef TWEAKS_INFO_DIALOG_H__
#define TWEAKS_INFO_DIALOG_H__

#include "system/keymap_sw.h"
#include "theme/render/dialog.h"
#include "theme/sound.h"

#include "./appstate.h"

void __showInfoDialog(const char *title, const char *message)
{
    bool confirm_quit = false;
    SDLKey changed_key = SDLK_UNKNOWN;

    keys_enabled = false;

    background_cache = SDL_CreateRGBSurface(SDL_HWSURFACE, 640, 480, 32, 0, 0, 0, 0);
    SDL_BlitSurface(screen, NULL, background_cache, NULL);

    theme_renderDialog(screen, title, message, false);
    SDL_BlitSurface(screen, NULL, video, NULL);
    SDL_Flip(video);

    while (!confirm_quit) {
        if (updateKeystate(keystate, &confirm_quit, true, &changed_key)) {
            if ((changed_key == SW_BTN_A || changed_key == SW_BTN_B || changed_key == SW_BTN_SELECT) && keystate[changed_key] == PRESSED) {
                confirm_quit = true;
                sound_change();
            }
        }
    }

    keys_enabled = true;
    all_changed = true;
}

void showInfoDialog(List *list)
{
    ListItem *item = list_currentItem(list);
    __showInfoDialog(item->label, item->info_note);
}

#endif // TWEAKS_INFO_DIALOG_H__
