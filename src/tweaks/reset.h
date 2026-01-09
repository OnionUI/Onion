#ifndef TWEAKS_RESET_H__
#define TWEAKS_RESET_H__

#include <stdio.h>

#include "system/device_model.h"
#include "system/keymap_sw.h"
#include "theme/render/dialog.h"
#include "theme/sound.h"

#include "./appstate.h"

#define RESET_CONFIGS_PAK "/mnt/SDCARD/.tmp_update/config/configs.pak"

bool _confirmReset(const char *title_str, const char *message_str)
{
    bool retval = false;
    bool confirm_quit = false;
    SDLKey changed_key = SDLK_UNKNOWN;

    keys_enabled = false;

    background_cache = SDL_CreateRGBSurface(SDL_HWSURFACE, 640, 480, 32, 0, 0, 0, 0);
    SDL_BlitSurface(screen, NULL, background_cache, NULL);

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

    if (changed_key != SDLK_UNKNOWN)
        sound_change();

    if (retval) {
        SDL_BlitSurface(screen, NULL, background_cache, NULL);
        theme_renderDialog(screen, title_str, "Resetting...", false);
        SDL_BlitSurface(screen, NULL, video, NULL);
        SDL_Flip(video);
    }
    else {
        keys_enabled = true;
        all_changed = true;
    }

    return retval;
}

void _notifyResetDone(const char *title_str)
{
    SDL_BlitSurface(background_cache, NULL, screen, NULL);
    theme_renderDialog(screen, title_str, "Done", false);
    SDL_BlitSurface(screen, NULL, video, NULL);
    SDL_Flip(video);
    msleep(300);

    SDL_FreeSurface(background_cache);
    keys_enabled = true;
    all_changed = true;

    sync();
}

void action_resetTweaks(void *pt)
{
    const char title_str[] = "Reset system tweaks";
    if (!_disable_confirm && !_confirmReset(title_str, "Are you sure you want to\nreset system tweaks?"))
        return;
    rename(RESET_CONFIGS_PAK, "/mnt/SDCARD/.tmp_update/temp");
    system("rm -rf /mnt/SDCARD/.tmp_update/config && mkdir -p /mnt/SDCARD/.tmp_update/config");
    system("7z x /mnt/SDCARD/.tmp_update/temp -o/mnt/SDCARD/ -ir!.tmp_update/config/*");
    rename("/mnt/SDCARD/.tmp_update/temp", RESET_CONFIGS_PAK);
    reset_menus = true;
    settings_load();
    if (!_disable_confirm)
        _notifyResetDone(title_str);
}

void action_resetThemeOverrides(void *pt)
{
    const char title_str[] = "Reset theme overrides";
    if (!_disable_confirm && !_confirmReset(title_str, "Are you sure you want to\nreset theme overrides?"))
        return;
    system("rm -rf /mnt/SDCARD/Saves/CurrentProfile/theme/*");
    if (!_disable_confirm)
        _notifyResetDone(title_str);
}

void action_resetMainUI(void *pt)
{
    const char title_str[] = "Reset MainUI settings";

    if (!_disable_confirm && !_confirmReset(title_str, "Are you sure you want to\nreset MainUI settings?"))
        return;

    system("rm -f /mnt/SDCARD/system.json");

    char cmd_str[80];
    sprintf(cmd_str, "cp /mnt/SDCARD/.tmp_update/res/miyoo%d_system.json /mnt/SDCARD/system.json", DEVICE_ID);
    system(cmd_str);

    if (IS_MIYOO_PLUS_OR_FLIP()) {
        system("rm -f /appconfigs/wpa_supplicant.conf");
        system("cp /mnt/SDCARD/.tmp_update/res/wpa_supplicant.reset /appconfigs/wpa_supplicant.conf");
    }

    reset_menus = true;
    settings_load();
    if (!_disable_confirm)
        _notifyResetDone(title_str);
}

void action_resetRAMain(void *pt)
{
    const char title_str[] = "Reset RetroArch configuration";
    if (!_disable_confirm && !_confirmReset(title_str, "Are you sure you want to reset\nRetroArch main configuration?"))
        return;
    system("7z x -aoa " RESET_CONFIGS_PAK " -o/mnt/SDCARD/ -ir!RetroArch/*");
    reset_menus = true;
    if (!_disable_confirm)
        _notifyResetDone(title_str);
}

void action_resetRACores(void *pt)
{
    const char title_str[] = "Reset all RA core overrides";
    if (!_disable_confirm && !_confirmReset(title_str, "Are you sure you want to reset\nall RetroArch core overrides?"))
        return;
    system("rm -rf /mnt/SDCARD/Saves/CurrentProfile/config/*");
    system("7z x " RESET_CONFIGS_PAK " -o/mnt/SDCARD/ -ir!Saves/CurrentProfile/config/*");
    reset_menus = true;
    if (!_disable_confirm)
        _notifyResetDone(title_str);
}

void action_resetAdvanceMENU(void *pt)
{
    const char title_str[] = "Reset AdvanceMENU/MAME/MESS";
    if (!_disable_confirm && !_confirmReset(title_str, "Are you sure you want to\nreset AdvanceMENU/MAME/MESS?"))
        return;
    system("7z x -aoa " RESET_CONFIGS_PAK " -o/mnt/SDCARD/ -ir!BIOS/.advance/*");
    reset_menus = true;
    if (!_disable_confirm)
        _notifyResetDone(title_str);
}

void action_resetAll(void *pt)
{
    const char title_str[] = "Reset everything";
    if (!_confirmReset(title_str, "Are you sure you want to\nreset everything?"))
        return;
    _disable_confirm = true;
    action_resetTweaks(pt);
    action_resetThemeOverrides(pt);
    action_resetMainUI(pt);
    action_resetRAMain(pt);
    action_resetRACores(pt);
    action_resetAdvanceMENU(pt);
    _disable_confirm = false;
    reset_menus = true;
    settings_load();
    _notifyResetDone(title_str);
}

#endif // TWEAKS_RESET_H__
