#ifndef GAME_SWITCHER_POP_MENU_H__
#define GAME_SWITCHER_POP_MENU_H__

#include "components/list.h"
#include "system/lang.h"
#include "theme/theme.h"

#include "gs_appState.h"
#include "gs_model.h"

void action_resumeGame(void *_)
{
    appState.exit_to_menu = false;
    appState.quit = true;
    appState.pop_menu_open = false;
}

void action_saveGame(void *_)
{
    // not implemented
}

void action_loadGame(void *_)
{
    // not implemented
}

void action_exitToMenu(void *_)
{
    appState.exit_to_menu = true;
    appState.quit = true;
    appState.pop_menu_open = false;
}

void popMenu_create(void)
{
    if (!appState.pop_menu_list._created) {
        printf_debug("Creating pop menu for game %i\n", appState.current_game);

        bool save_enabled = appState.is_overlay && appState.current_game == 0;
        bool load_enabled = false;

        appState.pop_menu_list = list_create(4, LIST_SMALL);
        appState.pop_menu_list.scroll_height = (save_enabled && load_enabled) ? 4 : (save_enabled || load_enabled ? 3 : 2);

        list_addItemWithLang(&appState.pop_menu_list,
                             (ListItem){
                                 .label = LANG_FALLBACK_RESUME,
                                 .action = action_resumeGame},
                             LANG_RESUME);

        if (save_enabled) {
            list_addItemWithLang(&appState.pop_menu_list,
                                 (ListItem){
                                     .label = LANG_FALLBACK_SAVE,
                                     .action = action_saveGame},
                                 LANG_SAVE);
        }

        if (load_enabled) {
            list_addItemWithLang(&appState.pop_menu_list,
                                 (ListItem){
                                     .label = LANG_FALLBACK_LOAD,
                                     .action = action_loadGame},
                                 LANG_LOAD);
        }

        list_addItemWithLang(&appState.pop_menu_list,
                             (ListItem){
                                 .label = LANG_FALLBACK_EXIT_TO_MENU,
                                 .action = action_exitToMenu},
                             LANG_EXIT_TO_MENU);
    }
}

void popMenu_destroy(void)
{
    list_free(&appState.pop_menu_list);
}

void renderPopMenu(AppState *state)
{
    if (state->pop_menu_open) {
        if (!appState.pop_menu_list._created) {
            popMenu_create();
        }
        theme_renderFooter(screen);
        theme_renderStandardHint(screen,
                                 lang_get(LANG_SELECT, LANG_FALLBACK_SELECT),
                                 lang_get(LANG_BACK, LANG_FALLBACK_BACK));
        theme_renderPopMenu(screen, state->view_mode == VIEW_NORMAL ? state->header_height : 0, &appState.pop_menu_list);
    }
}

#endif // GAME_SWITCHER_POP_MENU_H__
