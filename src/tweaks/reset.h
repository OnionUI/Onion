#ifndef TWEAKS_RESET_H__
#define TWEAKS_RESET_H__

#include <stdio.h>
#include "system/keymap_sw.h"
#include "theme/sound.h"
#include "theme/render/dialog.h"

#include "./appstate.h"

#define RESET_CONFIGS_PAK "/mnt/SDCARD/.tmp_update/config/configs.pak"

static bool _disable_confirm = false;

bool _confirmReset(const char *title_str, const char *message_str)
{
    bool retval = false;
    bool confirm_quit = false;
    SDLKey changed_key;

    keys_enabled = false;

    theme_renderDialog(screen, title_str, message_str, true);
    SDL_BlitSurface(screen, NULL, video, NULL);
    SDL_Flip(video);

    while (!confirm_quit) {
        if (updateKeystate(keystate, &confirm_quit, true, &changed_key)) {
            if (changed_key == SW_BTN_B && keystate[SW_BTN_B] == PRESSED)
                confirm_quit = true;
            else if (changed_key == SW_BTN_A && keystate[SW_BTN_A] == PRESSED) {
                retval = true;
                confirm_quit = true;
            }
        }
    }

    sound_change();

    if (retval) {
        msleep(300);
        theme_renderDialog(screen, title_str, "Done", true);
        SDL_BlitSurface(screen, NULL, video, NULL);
        SDL_Flip(video);
        msleep(300);
    }
    
    theme_clearDialog();
    keys_enabled = true;
    all_changed = true;

    return retval;
}

void action_resetTweaks(void *pt)
{
    if (!_disable_confirm && !_confirmReset("Reset tweaks", "Are you sure you want to\nreset tweaks?"))
        return;
    rename(RESET_CONFIGS_PAK, "/mnt/SDCARD/.tmp_update/temp");
    system("rm -rf /mnt/SDCARD/.tmp_update/config && mkdir -p /mnt/SDCARD/.tmp_update/config");
    system("unzip /mnt/SDCARD/.tmp_update/temp \".tmp_update/config/*\" -d /mnt/SDCARD/");
    rename("/mnt/SDCARD/.tmp_update/temp", RESET_CONFIGS_PAK);
    reset_menus = true;
	settings_load();
}

void action_resetThemeOverrides(void *pt)
{
    if (!_disable_confirm && _confirmReset("Reset theme overrides", "Are you sure you want to\nreset theme overrides?"))
        system("rm -rf /mnt/SDCARD/Saves/CurrentProfile/theme/*");
}

void action_resetMainUI(void *pt)
{
    if (!_disable_confirm && !_confirmReset("Reset MainUI settings", "Are you sure you want to\nreset MainUI settings?"))
        return;
    system("rm -f /appconfigs/system.json");
    system("unzip -o " RESET_CONFIGS_PAK " \".tmp_update/config/system.json\" -d /mnt/SDCARD/");
    system("cp /mnt/SDCARD/.tmp_update/config/system.json /appconfigs/system.json");
    reset_menus = true;
	settings_load();
}

void action_resetRAMain(void *pt)
{
    if (!_disable_confirm && !_confirmReset("Reset RA main configuration", "Are you sure you want to reset\nRetroArch main configuration?"))
        return;
    system("unzip -o " RESET_CONFIGS_PAK " \"RetroArch/*\" -d /mnt/SDCARD/");
    reset_menus = true;
}

void action_resetRACores(void *pt)
{
    if (!_disable_confirm && !_confirmReset("Reset all RA core overrides", "Are you sure you want to reset\nall RetroArch core overrides?"))
        return;
    system("rm -rf /mnt/SDCARD/Saves/CurrentProfile/config/*");
    system("unzip -o " RESET_CONFIGS_PAK " \"Saves/CurrentProfile/config/*\" -d /mnt/SDCARD/");
    reset_menus = true;
}

void action_resetAll(void *pt)
{
    if (!_confirmReset("Reset everything", "Are you sure you want to\nreset everything?"))
        return;
    _disable_confirm = true;
    action_resetTweaks(pt);
    action_resetThemeOverrides(pt);
    action_resetRAMain(pt);
    action_resetRACores(pt);
    _disable_confirm = false;
    reset_menus = true;
	settings_load();
}

#endif // TWEAKS_RESET_H__
