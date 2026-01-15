#ifndef GAME_SWITCHER_KEY_STATE_H__
#define GAME_SWITCHER_KEY_STATE_H__

#include "components/list.h"
#include "system/keymap_sw.h"
#include "system/settings.h"
#include "theme/render/dialog.h"
#include "utils/config.h"
#include "utils/keystate.h"

#include "gs_appState.h"
#include "gs_model.h"
#include "gs_popMenu.h"
#include "gs_romscreen.h"

typedef struct {
    KeyState keystate[320];
    bool btn_a_pressed;
    bool menu_pressed;
    bool combo_key;
    bool select_pressed;
    bool select_combo_key;
    SDLKey changed_key;
    int button_y_repeat;
} AppKeyState_s;

static AppKeyState_s _gs_keystate = {
    .keystate = {RELEASED},
    .btn_a_pressed = false,
    .menu_pressed = false,
    .combo_key = false,
    .select_pressed = false,
    .select_combo_key = false,
    .changed_key = SDLK_UNKNOWN,
    .button_y_repeat = 0,
};

void removeTemporaryFile(char *game_name)
{

    DIR *temp_rom_dir = opendir(ROM_TEMP_DIR);
    if (temp_rom_dir) {
        struct dirent *entry;
        while ((entry = readdir(temp_rom_dir)) != NULL) {
            if (strstr(entry->d_name, game_name)) {
                char *temp_file;
                if (asprintf(&temp_file, "%s/%s", ROM_TEMP_DIR, entry->d_name) < 0) {
                    print_debug("Unable to allocate string for the path of the temporary rom\n");
                }
                else {
                    remove(temp_file);
                    free(temp_file);
                }
                break;
            }
        }
        closedir(temp_rom_dir);
    }
    else {
        print_debug("Unable to open temporary rom folder\n");
    }
}

void removeCurrentItem()
{
    Game_s *game = &game_list[appState.current_game];

    printf_debug("removing: %s\n", game->name);
    printf_debug("linenumber: %i\n", game->recentItem.lineNo);

    if (game->romScreen != NULL) {
        SDL_FreeSurface(game->romScreen);
        game->romScreen = NULL;
    }

    file_delete_line(getMiyooRecentFilePath(), game->recentItem.lineNo);

    if (strlen(game->recentItem.imgpath) > 0 && is_file(game->recentItem.imgpath)) {
        if (strncmp(game->recentItem.imgpath, ROM_SCREENS_DIR, strlen(ROM_SCREENS_DIR)) == 0) {
            remove(game->recentItem.imgpath);
        }
    }

    const char *ext = file_getExtension(game->recentItem.rompath);
    if (!strcmp(ext, "zip") || !strcmp(ext, "7z")) {
        removeTemporaryFile(game->name);
    }

    // Copy next element value to current element
    for (int i = appState.current_game; i < game_list_len - 1; i++) {
        game_list[i] = game_list[i + 1];
        game_list[i].recentItem.lineNo -= 1;
        game_list[i].index -= 1;
    }

    game_list_len--;
}

int checkQuitAction(void)
{
    FILE *fp;
    char prev_state[10];
    file_get(fp, "/tmp/prev_state", "%s", prev_state);
    if (strncmp(prev_state, "mainui", 6) == 0)
        return 1;
    return 0;
}

void action_confirmRemove(AppState *state)
{
    theme_renderDialog(
        screen, "Remove from history",
        "Are you sure you want to\nremove game from history?",
        true);
    render();

    KeyState *keystate = _gs_keystate.keystate;

    while (!state->quit) {
        if (_updateKeystate(keystate, &state->quit, true, NULL)) {
            if (keystate[SW_BTN_A] == PRESSED) {
                removeCurrentItem();
                if (state->current_game > 0)
                    state->current_game--;
                state->current_game_changed = true;
                loadRomScreen(state->current_game);
                state->changed = true;
                break;
            }
            if (keystate[SW_BTN_B] == PRESSED) {
                state->changed = true;
                break;
            }
        }
    }
}

void action_toggleHeader(AppState *state)
{
    state->show_legend = true;
    state->legend_start = state->last_ticks;

    if (!state->show_time && !state->show_total)
        state->show_time = true, state->show_total = false;
    else if (state->show_time && !state->show_total)
        state->show_time = true, state->show_total = true;
    else
        state->show_time = false, state->show_total = false;

    config_flag_set("gameSwitcher/showTime", state->show_time);
    config_flag_set("gameSwitcher/hideTotal", !state->show_total);

    state->changed = true;
}

void handleUpdateKeystateMain(AppState *state)
{
    KeyState *keystate = _gs_keystate.keystate;

    if (keystate[SW_BTN_RIGHT] >= PRESSED) {
        if (state->current_game < game_list_len - 1) {
            state->current_game++;
            state->current_game_changed = true;
            state->changed = true;
        }
    }

    if (keystate[SW_BTN_LEFT] >= PRESSED) {
        if (state->current_game > 0) {
            state->current_game--;
            state->current_game_changed = true;
            state->changed = true;
        }
    }

    if (keystate[SW_BTN_A] == PRESSED) {
        _gs_keystate.btn_a_pressed = true;
    }
    else if (keystate[SW_BTN_A] == RELEASED && _gs_keystate.btn_a_pressed) {
        _gs_keystate.btn_a_pressed = false;
        state->quit = true;
        return;
    }

    if (keystate[SW_BTN_B] == PRESSED) {
        state->exit_to_menu = true;
        state->quit = true;
        return;
    }

    if (keystate[_gs_keystate.changed_key] == PRESSED && _gs_keystate.changed_key != SW_BTN_UP && _gs_keystate.changed_key != SW_BTN_DOWN)
        state->brightness_changed = false;

    if (_gs_keystate.keystate[SW_BTN_UP] >= PRESSED) {
        // Change brightness
        if (settings.brightness < 10) {
            settings_setBrightness(settings.brightness + 1, true, true);
        }
        state->brightness_changed = true;
        state->brightness_start = state->last_ticks;
        state->changed = true;
    }

    if (_gs_keystate.keystate[SW_BTN_DOWN] >= PRESSED) {
        // Change brightness
        if (settings.brightness > 0) {
            settings_setBrightness(settings.brightness - 1, true, true);
        }
        state->brightness_changed = true;
        state->brightness_start = state->last_ticks;
        state->changed = true;
    }

    if (_gs_keystate.combo_key ||
        (_gs_keystate.select_pressed && ((_gs_keystate.changed_key == SW_BTN_L2 &&
                                          keystate[SW_BTN_L2] == RELEASED) ||
                                         (_gs_keystate.changed_key == SW_BTN_R2 &&
                                          keystate[SW_BTN_R2] == RELEASED)))) {
        settings_load();
        state->brightness_changed = false;
        state->changed = true;
    }

    if (_gs_keystate.changed_key == SW_BTN_Y && keystate[SW_BTN_Y] == RELEASED) {
        if (_gs_keystate.button_y_repeat < 75) {
            state->view_mode = state->view_mode == VIEW_FULLSCREEN ? state->view_restore : !state->view_mode;
            config_flag_set("gameSwitcher/minimal", state->view_mode == VIEW_MINIMAL);
            state->changed = true;
        }
        _gs_keystate.button_y_repeat = 0;
    }

    if (keystate[SW_BTN_X] == PRESSED) {
        if (game_list_len != 0) {
            action_confirmRemove(state);
        }
    }

    if (state->current_game_changed) {
        popMenu_destroy();
    }
}

void handleUpdateKeystatePopMenu(AppState *state)
{
    KeyState *keystate = _gs_keystate.keystate;
    ListItem *item = list_currentItem(&state->pop_menu_list);

    if (keystate[SW_BTN_B] == PRESSED) {
        state->pop_menu_open = false;
        state->changed = true;
    }

    if (keystate[SW_BTN_A] == PRESSED) {
        _gs_keystate.btn_a_pressed = true;
    }
    else if (keystate[SW_BTN_A] == RELEASED && _gs_keystate.btn_a_pressed) {
        _gs_keystate.btn_a_pressed = false;
        list_activateItem(&state->pop_menu_list);
    }

    if (keystate[SW_BTN_DOWN] >= PRESSED) {
        if (list_keyDown(&state->pop_menu_list, keystate[SW_BTN_DOWN] == REPEATING))
            state->changed = true;
    }
    else if (keystate[SW_BTN_UP] >= PRESSED) {
        if (list_keyUp(&state->pop_menu_list, keystate[SW_BTN_UP] == REPEATING))
            state->changed = true;
    }

    if (item != NULL && item->action_id == POP_MENU_ACTION_LOAD) {
        if (keystate[SW_BTN_LEFT] >= PRESSED) {
            if (g_save_state_info.selected_slot > 0) {
                g_save_state_info.selected_slot--;
                setLoadPreview();
                state->changed = true;
            }
        }
        else if (keystate[SW_BTN_RIGHT] >= PRESSED) {
            if (g_save_state_info.selected_slot < g_save_state_info.slot_count - 1) {
                g_save_state_info.selected_slot++;
                setLoadPreview();
                state->changed = true;
            }
        }
        else if (keystate[SW_BTN_X] == PRESSED) {
            popMenu_deleteSaveState();
            keystate[SW_BTN_X] = RELEASED;
        }
    }
}

void handleKeystate(AppState *state)
{
    KeyState *keystate = _gs_keystate.keystate;

    if (_updateKeystate(keystate, &state->quit, true, &_gs_keystate.changed_key)) {
        if (_gs_keystate.menu_pressed && _gs_keystate.changed_key != SW_BTN_MENU)
            _gs_keystate.combo_key = true;
        if (_gs_keystate.select_pressed && _gs_keystate.changed_key != SW_BTN_SELECT)
            _gs_keystate.select_combo_key = true;

        if (keystate[SW_BTN_MENU] == PRESSED)
            _gs_keystate.menu_pressed = true;

        if (_gs_keystate.menu_pressed && keystate[SW_BTN_MENU] == RELEASED) {
            if (!_gs_keystate.combo_key) {
                state->quit = true;
                return;
            }
            _gs_keystate.menu_pressed = false;
            _gs_keystate.combo_key = false;
        }

        if (_gs_keystate.changed_key == SW_BTN_SELECT) {
            if (keystate[SW_BTN_SELECT] == PRESSED)
                _gs_keystate.select_pressed = true;
            if (keystate[SW_BTN_SELECT] == RELEASED) {
                if (!_gs_keystate.select_combo_key) {
                    action_toggleHeader(state);
                }
                _gs_keystate.select_pressed = false;
                _gs_keystate.select_combo_key = false;
            }
        }

        if (keystate[SW_BTN_START] == PRESSED) {
            state->pop_menu_open = !state->pop_menu_open;
            state->changed = true;
        }

        if (state->pop_menu_open) {
            handleUpdateKeystatePopMenu(state);
        }
        else {
            handleUpdateKeystateMain(state);
        }
    }

    if (keystate[SW_BTN_Y] == PRESSED && state->view_mode != VIEW_FULLSCREEN && !state->pop_menu_open) {
        _gs_keystate.button_y_repeat++;
        if (_gs_keystate.button_y_repeat >= 75) {
            state->view_restore = state->view_mode;
            state->view_mode = VIEW_FULLSCREEN;
            state->changed = true;
        }
    }
}

#endif // GAME_SWITCHER_KEY_STATE_H__
