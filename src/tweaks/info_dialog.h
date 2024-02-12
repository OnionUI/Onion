#ifndef TWEAKS_INFO_DIALOG_H__
#define TWEAKS_INFO_DIALOG_H__

#include "system/keymap_sw.h"
#include "theme/render/dialog.h"
#include "theme/sound.h"

#include "./appstate.h"

int __showInfoDialog(const char *title, const char *message, bool waitinput, int timeout)
{
    bool confirm_quit = false;
    SDLKey changed_key = SDLK_UNKNOWN;

    keys_enabled = false;
    int ret_val = 0;
    background_cache = SDL_CreateRGBSurface(SDL_HWSURFACE, 640, 480, 32, 0, 0, 0, 0);
    SDL_BlitSurface(screen, NULL, background_cache, NULL);

    theme_renderDialog(screen, title, message, false);
    SDL_BlitSurface(screen, NULL, video, NULL);
    SDL_Flip(video);
    if (waitinput) {
        while (!confirm_quit) {
            if (updateKeystate(keystate, &confirm_quit, true, &changed_key)) {
                if ((changed_key == SW_BTN_A || changed_key == SW_BTN_B || changed_key == SW_BTN_SELECT) && keystate[changed_key] == PRESSED) {
                    confirm_quit = true;
                    ret_val = changed_key;
                    sound_change();
                }
            }
        }
    }
    if (timeout != NULL && timeout > 0) {
        SDL_Delay(timeout);
    }
    keys_enabled = true;
    all_changed = true;
    return ret_val;
}

int showInfoDialogGeneric(const char *title, const char *message, bool waitinput, int timeout)
{
    return __showInfoDialog(title, message, waitinput, timeout);
}

void showInfoDialog(List *list, bool waitinput, int timeout)
{
    ListItem *item = list_currentItem(list);
    __showInfoDialog(item->label, item->info_note, waitinput, timeout);
}

#endif // TWEAKS_INFO_DIALOG_H__
