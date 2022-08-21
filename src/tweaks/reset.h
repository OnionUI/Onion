#ifndef TWEAKS_RESET_H__
#define TWEAKS_RESET_H__

#include <stdio.h>
#include <SDL/SDL.h>
#include "system/keymap_sw.h"
#include "theme/sound.h"
#include "theme/render/dialog.h"

#include "./appstate.h"

#define RESET_CONFIGS_PAK "/mnt/SDCARD/.tmp_update/config/configs.pak"

static bool _disable_confirm = false;

void reset_tweaksMenu(SDL_Surface * screen)
{
    int current_state[10][2];
    int current_level = menu_level;
    for (int i = 0; i <= current_level; i++) {
        current_state[i][0] = menu_stack[i]->active_pos;
        current_state[i][1] = menu_stack[i]->scroll_pos;
    }
    menu_free_all();
    menu_main();
    for (int i = 0; i <= current_level; i++) {
        menu_stack[i]->active_pos = current_state[i][0];
        menu_stack[i]->scroll_pos = current_state[i][1];
        if (i < current_level)
            list_activateItem(menu_stack[i]);
    }
    reset_menus = false;
}

bool _confirmReset(SDL_Surface *video, SDL_Surface *screen, const char *title_str, const char *message_str)
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

void action_resetTweaks(SDL_Surface *video, SDL_Surface *screen, void *pt)
{
    if (!_disable_confirm && !_confirmReset(video, screen, "Reset tweaks to default", "Are you sure you want to\nreset tweaks to default?"))
        return;
    rename(RESET_CONFIGS_PAK, "/mnt/SDCARD/.tmp_update/temp");
    system("rm -rf /mnt/SDCARD/.tmp_update/config && mkdir -p /mnt/SDCARD/.tmp_update/config");
    system("unzip /mnt/SDCARD/.tmp_update/temp \".tmp_update/config/*\" -d /mnt/SDCARD/");
    rename("/mnt/SDCARD/.tmp_update/temp", RESET_CONFIGS_PAK);
    reset_menus = true;
	settings_load();
}

void action_resetThemeOverrides(SDL_Surface *video, SDL_Surface *screen, void *pt)
{
    if (!_disable_confirm && _confirmReset(video, screen, "Reset theme overrides", "Are you sure you want to\nreset theme overrides?"))
        system("rm -rf /mnt/SDCARD/Saves/CurrentProfile/theme/*");
}

void action_resetMainUI(SDL_Surface *video, SDL_Surface *screen, void *pt)
{
    if (!_disable_confirm && !_confirmReset(video, screen, "Reset MainUI settings", "Are you sure you want to\nreset MainUI settings?"))
        return;
    system("rm -f /appconfigs/system.json");
    system("unzip -o " RESET_CONFIGS_PAK " \".tmp_update/config/system.json\" -d /mnt/SDCARD/");
    system("cp /mnt/SDCARD/.tmp_update/config/system.json /appconfigs/system.json");
    reset_menus = true;
	settings_load();
}

void action_resetRAMain(SDL_Surface *video, SDL_Surface *screen, void *pt)
{
    if (!_disable_confirm && !_confirmReset(video, screen, "Reset RA main configuration", "Are you sure you want to reset\nRetroArch main configuration?"))
        return;
    system("unzip -o " RESET_CONFIGS_PAK " \"RetroArch/*\" -d /mnt/SDCARD/");
    reset_menus = true;
}

void action_resetRACores(SDL_Surface *video, SDL_Surface *screen, void *pt)
{
    if (!_disable_confirm && !_confirmReset(video, screen, "Reset all RA core overrides", "Are you sure you want to reset\nall RetroArch core overrides?"))
        return;
    system("rm -rf /mnt/SDCARD/Saves/CurrentProfile/config/*");
    system("unzip -o " RESET_CONFIGS_PAK " \"Saves/CurrentProfile/config/*\" -d /mnt/SDCARD/");
    reset_menus = true;
}

void action_resetAll(SDL_Surface *video, SDL_Surface *screen, void *pt)
{
    if (!_confirmReset(video, screen, "Reset all to default", "Are you sure you want to\nreset everything to default?"))
        return;
    _disable_confirm = true;
    action_resetTweaks(video, screen, pt);
    action_resetThemeOverrides(video, screen, pt);
    action_resetRAMain(video, screen, pt);
    action_resetRACores(video, screen, pt);
    _disable_confirm = false;
    reset_menus = true;
	settings_load();
}

#endif // TWEAKS_RESET_H__
