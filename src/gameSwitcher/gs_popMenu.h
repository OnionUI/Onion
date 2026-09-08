#ifndef GAME_SWITCHER_POP_MENU_H__
#define GAME_SWITCHER_POP_MENU_H__

#include "components/list.h"
#include "system/lang.h"
#include "theme/theme.h"
#include "utils/file.h"
#include "utils/retroarch_cmd.h"

#include "gs_appState.h"
#include "gs_favorites.h"
#include "gs_model.h"
#include "gs_retroarch.h"

#define POP_MENU_ACTION_RESUME 0
#define POP_MENU_ACTION_SAVE 1
#define POP_MENU_ACTION_LOAD 2
#define POP_MENU_ACTION_EXIT 3
#define POP_MENU_ACTION_FAVORITE 4

typedef struct {
    int slots[10];
    int slot_count;
    int selected_slot;
} SaveStateInfo_s;

static SaveStateInfo_s g_save_state_info = {.slots = {0}, .slot_count = 0, .selected_slot = 0};

static pthread_t g_scan_thread_pt;
static bool g_scan_thread_started = false;
static int g_scan_game_index = -1;

static void setLoadPreview(void);
static void popMenu_finishScan(bool update_preview);

static bool g_save_thread_running = false;
static bool g_save_thread_success = false;

void popMenu_destroy(void)
{
    // The scan must be joined before the list can be freed. Older code let
    // _scan_thread() call setLoadPreview() after list_free(), which could
    // access freed ListItem storage and crash on a later popup open.
    popMenu_finishScan(false);
    list_free(&appState.pop_menu_list);
    appState.pop_menu_list = (List){0};
}

static bool _hasSaveStates(Game_s *game)
{
    char stateDirPath[4096];
    snprintf(stateDirPath, sizeof(stateDirPath), STATES_DIR "/%s", game->core_name);

    if (!exists(stateDirPath)) {
        return false;
    }

    DIR *dir = opendir(stateDirPath);
    if (dir == NULL) {
        return false;
    }

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_type == DT_REG && strncmp(entry->d_name, game->rom_name, strlen(game->rom_name)) == 0) {
            char *slotStr = entry->d_name + strlen(game->rom_name);
            if (strncmp(slotStr, ".state", 6) == 0) {
                if (strncmp(slotStr, ".state.auto", 11) == 0 || strncmp(slotStr + strlen(slotStr) - 4, ".png", 4) == 0) {
                    continue; // Skip auto save states and preview images
                }

                closedir(dir);
                return true;
            }
        }
    }

    closedir(dir);
    return false;
}

static bool _scanSaveStates(Game_s *game, SaveStateInfo_s *info)
{
    info->slot_count = 0;
    info->selected_slot = 0;

    char stateDirPath[4096];
    snprintf(stateDirPath, sizeof(stateDirPath), STATES_DIR "/%s", game->core_name);

    if (!exists(stateDirPath)) {
        return false;
    }

    DIR *dir = opendir(stateDirPath);
    if (dir == NULL) {
        return false;
    }

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_type == DT_REG && strncmp(entry->d_name, game->rom_name, strlen(game->rom_name)) == 0) {
            char *slotStr = entry->d_name + strlen(game->rom_name);
            if (strncmp(slotStr, ".state", 6) == 0) {
                int slot = 0;
                if (strncmp(slotStr, ".state.auto", 11) == 0 || strncmp(slotStr + strlen(slotStr) - 4, ".png", 4) == 0) {
                    continue; // Skip auto save states and preview images
                }
                else if (slotStr[6] != '\0') {
                    slot = atoi(slotStr + 6);
                    if (slot == 0) {
                        continue; // '.state0' is not a valid slot (should be '.state')
                    }
                }

                if (info->slot_count < 10) {
                    info->slots[info->slot_count++] = slot;
                }
                else {
                    // Find the minimum slot in the array
                    int min_index = 0;
                    for (int i = 1; i < 10; i++) {
                        if (info->slots[i] < info->slots[min_index]) {
                            min_index = i;
                        }
                    }
                    // Replace the minimum slot if the current slot is higher
                    if (slot > info->slots[min_index]) {
                        info->slots[min_index] = slot;
                    }
                }
            }
        }
    }

    closedir(dir);

    // Sort the slots in descending order
    for (int i = 0; i < info->slot_count - 1; i++) {
        for (int j = i + 1; j < info->slot_count; j++) {
            if (info->slots[j] > info->slots[i]) {
                int temp = info->slots[i];
                info->slots[i] = info->slots[j];
                info->slots[j] = temp;
            }
        }
    }

    return true;
}

static bool createSaveStatePath(Game_s *game, int slot, char *out_path, size_t out_path_size)
{
    if (strlen(game->core_name) == 0) {
        return false;
    }

    if (slot == -1) {
        snprintf(out_path, out_path_size, STATES_DIR "/%s/%s.state.auto", game->core_name, game->rom_name);
    }
    else if (slot == 0) {
        snprintf(out_path, out_path_size, STATES_DIR "/%s/%s.state", game->core_name, game->rom_name);
    }
    else {
        snprintf(out_path, out_path_size, STATES_DIR "/%s/%s.state%d", game->core_name, game->rom_name, slot);
    }

    return true;
}

static void setLoadPreview(void)
{
    if (!appState.pop_menu_list._created ||
        appState.current_game < 0 ||
        appState.current_game >= game_list_len) {
        return;
    }

    ListItem *item = NULL;
    for (int i = 0; i < appState.pop_menu_list.item_count; i++) {
        if (appState.pop_menu_list.items[i].action_id == POP_MENU_ACTION_LOAD) {
            item = &appState.pop_menu_list.items[i];
            break;
        }
    }

    if (item != NULL) {
        item->preview_path[0] = '\0';

        if (g_save_state_info.selected_slot >= 0 && g_save_state_info.selected_slot < g_save_state_info.slot_count) {
            const int real_slot = g_save_state_info.slots[g_save_state_info.selected_slot];
            Game_s *game = &game_list[appState.current_game];
            char stateFilePath[2048];

            if (createSaveStatePath(game, real_slot, stateFilePath, sizeof(stateFilePath))) {
                snprintf(item->preview_path, sizeof(item->preview_path), "%s.png", stateFilePath);
            }
        }

        if (item->preview_ptr != NULL) {
            SDL_FreeSurface((SDL_Surface *)item->preview_ptr);
            item->preview_ptr = NULL;
        }
    }
}

static void *_save_thread(void *_)
{
    msleep(200);

    Game_s *game = &game_list[0];
    if (!_scanSaveStates(game, &g_save_state_info)) {
        g_save_thread_running = false;
        return NULL;
    }

    int slot = g_save_state_info.slot_count > 0 ? g_save_state_info.slots[0] + 1 : 1;
    printf_debug("Saving state to slot %d\n", slot);

    char stateFilePath[4096];
    time_t saveLastModified = 0;

    // Check if save state exists
    if (createSaveStatePath(game, slot, stateFilePath, sizeof(stateFilePath))) {
        printf_debug("Checking for save state: %s\n", stateFilePath);
        if (exists(stateFilePath)) {
            file_isModified(stateFilePath, &saveLastModified);
        }
    }

    const int start = SDL_GetTicks();

    retroarch_save(slot);

    // Wait for save state to be created
    bool saved = false;
    while (!saved && SDL_GetTicks() - start < 30000) {
        if (exists(stateFilePath)) {
            time_t newLastModified = 0;
            if (file_isModified(stateFilePath, &newLastModified) && newLastModified > saveLastModified) {
                saved = true;
                g_save_thread_success = true;
            }
        }
        msleep(100);
    }

    // Check if any process is using the save state file
    while (file_isLocked(stateFilePath) && SDL_GetTicks() - start < 30000) {
        msleep(100);
    }

    g_save_thread_running = false;
    return NULL;
}

static void *_scan_thread(void *_)
{
    const int game_index = g_scan_game_index;
    SaveStateInfo_s result = {.slots = {0}, .slot_count = 0, .selected_slot = 0};

    if (game_index >= 0 && game_index < game_list_len) {
        _scanSaveStates(&game_list[game_index], &result);
    }

    // Only publish plain scan data here. ListItem and SDL surface updates
    // must stay on the main thread.
    g_save_state_info = result;
    return NULL;
}

static void popMenu_finishScan(bool update_preview)
{
    if (g_scan_thread_started) {
        pthread_join(g_scan_thread_pt, NULL);
        g_scan_thread_started = false;
    }

    if (update_preview &&
        appState.pop_menu_list._created &&
        g_scan_game_index == appState.current_game) {
        setLoadPreview();
    }

    g_scan_game_index = -1;
}

static bool _isSaveEnabled(void)
{
    return currentGame()->is_running;
}

static bool _isLoadEnabled(void)
{
    Game_s *game = &game_list[appState.current_game];
    return _hasSaveStates(game);
}

void action_resumeGame(void *_)
{
    appState.exit_to_menu = false;
    appState.quit = true;
    appState.pop_menu_open = false;
}

void action_saveGame(void *_)
{
    if (!appState.is_overlay || appState.current_game != 0) {
        return;
    }

    if (g_save_thread_running) {
        print_debug("Save thread already running");
        return;
    }

    pthread_t save_thread;
    g_save_thread_running = true;
    g_save_thread_success = false;

    pthread_create(&save_thread, NULL, _save_thread, NULL);

    SDL_Surface *bg = SDL_CreateRGBSurface(SDL_SWSURFACE, g_display.width, g_display.height, 32, 0, 0, 0, 0);
    SDL_BlitSurface(screen, NULL, bg, NULL);

    theme_clearDialogProgress();

    while (g_save_thread_running) {
        SDL_BlitSurface(bg, NULL, screen, NULL);
        theme_renderDialogProgress(screen, "Saving", " ", false);
        render();
    }

    SDL_BlitSurface(bg, NULL, screen, NULL);
    theme_renderDialog(screen, "Saving", g_save_thread_success ? "State saved" : "Save failed", false);
    render();

    SDL_FreeSurface(bg);
    msleep(1000);

    popMenu_destroy();

    appState.changed = true;
}

void action_loadGame(void *_)
{
    popMenu_finishScan(true);

    if (g_save_state_info.selected_slot < 0 ||
        g_save_state_info.selected_slot >= g_save_state_info.slot_count) {
        return;
    }

    const int real_slot = g_save_state_info.slots[g_save_state_info.selected_slot];

    if (currentGame()->is_running) {
        retroarch_load(real_slot);
    }
    else {
        // Copy the save state to the auto state path
        Game_s *game = &game_list[appState.current_game];
        char stateFilePath[2048];
        char autoStateFilePath[2048];

        if (createSaveStatePath(game, real_slot, stateFilePath, sizeof(stateFilePath)) &&
            createSaveStatePath(game, -1, autoStateFilePath, sizeof(autoStateFilePath))) {
            file_copy(stateFilePath, autoStateFilePath);
        }
    }

    appState.exit_to_menu = false;
    appState.quit = true;
    appState.pop_menu_open = false;
}

void popMenu_deleteSaveState(void)
{
    popMenu_finishScan(true);

    int selected_slot = g_save_state_info.selected_slot;

    if (selected_slot < 0 || selected_slot >= g_save_state_info.slot_count) {
        return;
    }

    const int real_slot = g_save_state_info.slots[selected_slot];
    Game_s *game = &game_list[appState.current_game];
    char stateFilePath[2048];
    char imageFilePath[2056];

    if (createSaveStatePath(game, real_slot, stateFilePath, sizeof(stateFilePath))) {
        theme_renderDialog(
            screen, "Delete save state",
            "Are you sure you want to\ndelete this save state?",
            true);
        render();

        KeyState keystate[320] = {0};

        while (!appState.quit) {
            if (_updateKeystate(keystate, &appState.quit, true, NULL)) {
                if (keystate[SW_BTN_B] == PRESSED) {
                    break;
                }
                else if (keystate[SW_BTN_A] == PRESSED) {
                    printf_debug("Deleting save state %s\n", stateFilePath);
                    remove(stateFilePath);
                    snprintf(imageFilePath, sizeof(imageFilePath), "%s.png", stateFilePath);
                    printf_debug("Deleting save state image %s\n", imageFilePath);
                    remove(imageFilePath);
                    sync();
                    _scanSaveStates(game, &g_save_state_info);
                    if (g_save_state_info.slot_count > 0) {
                        g_save_state_info.selected_slot = selected_slot < g_save_state_info.slot_count
                                                              ? selected_slot
                                                              : (g_save_state_info.slot_count - 1);
                        setLoadPreview();
                    }
                    else {
                        popMenu_destroy();
                    }
                    break;
                }
            }
        }

        appState.changed = true;
    }
}

// Both labels are Onion additions, registered next to LANG_NEXT and
// LANG_RESUME_UC. Stock string 55 ("Add to favorites") would have come
// pre-translated, but runs to 24 characters in some languages, which
// overflows the popup, so these are deliberately shorter.
static void _setFavoriteLabel(ListItem *item, bool is_favorite)
{
    snprintf(item->label, sizeof(item->label), "%s",
             is_favorite
                 ? lang_get(LANG_REMOVE_FAVORITE, LANG_FALLBACK_REMOVE_FAVORITE)
                 : lang_get(LANG_ADD_FAVORITE, LANG_FALLBACK_ADD_FAVORITE));
}

void action_toggleFavorite(void *self)
{
    (void)self;

    if (appState.current_game < 0 || appState.current_game >= game_list_len) {
        return;
    }

    bool is_favorite = false;
    if (!favorites_toggle(currentGame(), &is_favorite)) {
        print_debug("Failed to update favorites");
        return;
    }

    // Reacquire the item after file I/O instead of retaining the callback
    // pointer. This stays valid even if popup ownership changes later.
    if (appState.pop_menu_list._created) {
        for (int i = 0; i < appState.pop_menu_list.item_count; i++) {
            ListItem *item = &appState.pop_menu_list.items[i];
            if (item->action_id == POP_MENU_ACTION_FAVORITE) {
                _setFavoriteLabel(item, is_favorite);
                break;
            }
        }
    }

    appState.changed = true;
}

void action_exitToMenu(void *_)
{
    appState.exit_to_menu = true;
    appState.quit = true;
    appState.pop_menu_open = false;
}

void popMenu_create(void)
{
    if (appState.pop_menu_list._created ||
        game_list_len <= 0 ||
        appState.current_game < 0 ||
        appState.current_game >= game_list_len) {
        return;
    }

    printf_debug("Creating pop menu for game %i\n", appState.current_game);

    const bool has_save = _isSaveEnabled();
    const bool has_load = _isLoadEnabled();

    // Five items are currently possible. Keep one spare slot so a future
    // conditional item cannot silently write past the allocation.
    appState.pop_menu_list = list_create(6, LIST_SMALL);

    list_addItemWithLang(&appState.pop_menu_list,
                         (ListItem){.label = LANG_FALLBACK_RESUME, .action = action_resumeGame, .action_id = POP_MENU_ACTION_RESUME},
                         LANG_RESUME);

    if (has_save) {
        list_addItemWithLang(&appState.pop_menu_list,
                             (ListItem){.label = LANG_FALLBACK_SAVE, .action = action_saveGame, .action_id = POP_MENU_ACTION_SAVE},
                             LANG_SAVE);
    }

    if (has_load) {
        list_addItemWithLang(&appState.pop_menu_list,
                             (ListItem){.label = LANG_FALLBACK_LOAD, .action = action_loadGame, .action_id = POP_MENU_ACTION_LOAD},
                             LANG_LOAD);
    }

    ListItem *favorite_item = list_addItem(
        &appState.pop_menu_list,
        (ListItem){.action = action_toggleFavorite, .action_id = POP_MENU_ACTION_FAVORITE});
    if (favorite_item != NULL) {
        _setFavoriteLabel(favorite_item, favorites_contains(currentGame()));
    }

    list_addItemWithLang(&appState.pop_menu_list,
                         (ListItem){.label = LANG_FALLBACK_EXIT_TO_MENU, .action = action_exitToMenu, .action_id = POP_MENU_ACTION_EXIT},
                         LANG_EXIT_TO_MENU);

    appState.pop_menu_list.scroll_height = appState.pop_menu_list.item_count;

    if (has_load) {
        g_save_state_info = (SaveStateInfo_s){.slots = {0}, .slot_count = 0, .selected_slot = 0};
        g_scan_game_index = appState.current_game;

        if (pthread_create(&g_scan_thread_pt, NULL, _scan_thread, NULL) == 0) {
            g_scan_thread_started = true;
        }
        else {
            _scanSaveStates(currentGame(), &g_save_state_info);
            setLoadPreview();
            g_scan_game_index = -1;
        }
    }
}

void renderPopMenu(AppState *state)
{
    if (state->pop_menu_open) {
        if (!appState.pop_menu_list._created) {
            popMenu_create();
        }
        if (!appState.pop_menu_list._created) {
            state->pop_menu_open = false;
            return;
        }

        ListItem *item = list_currentItem(&appState.pop_menu_list);
        SDL_Surface *transparent_bg = NULL;

        if (item != NULL && item->action_id == POP_MENU_ACTION_LOAD) {
            transparent_bg = appState.transparent_bg;
            popMenu_finishScan(true);
        }

        theme_renderPopMenu(screen, state->view_mode == VIEW_NORMAL ? state->header_height : 0, &appState.pop_menu_list, transparent_bg);

        if (item != NULL && item->action_id == POP_MENU_ACTION_LOAD) {
            theme_renderFooterStatus(screen, g_save_state_info.selected_slot + 1, g_save_state_info.slot_count);
        }
    }
}

#endif // GAME_SWITCHER_POP_MENU_H__
