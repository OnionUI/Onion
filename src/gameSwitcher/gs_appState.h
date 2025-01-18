#ifndef GAME_SWITCHER_APP_STATE_H__
#define GAME_SWITCHER_APP_STATE_H__

#include <SDL/SDL.h>
#include <signal.h>

#include "gs_model.h"

#define VIEW_NORMAL 0
#define VIEW_MINIMAL 1
#define VIEW_FULLSCREEN -1

typedef struct {
    List pop_menu_list;
    bool quit;
    bool exit_to_menu;
    bool changed;
    bool current_game_changed;
    bool brightness_changed;
    bool pop_menu_open;
    bool show_time;
    bool show_total;
    bool show_legend;
    bool is_overlay;
    int view_mode;
    int view_restore;
    int pop_menu_game_index;
    uint32_t acc_ticks;
    uint32_t last_ticks;
    uint32_t time_step;
    uint32_t legend_start;
    uint32_t legend_timeout;
    uint32_t brightness_start;
    uint32_t brightness_timeout;
    SDL_Surface *custom_header;
    SDL_Surface *custom_footer;
    int header_height;
    int footer_height;
    SDL_Surface *current_bg;
    SDL_Surface *transparent_bg;
    bool first_render;
    int current_game;
    SDL_Surface *surfaceGameName;
    SDL_Rect game_name_size;
    int game_name_max_width;
    int gameNameScrollX;
    int gameNameScrollSpeed;
    int gameNameScrollStart;
    int gameNameScrollEnd;
} AppState;

static AppState appState = {
    .pop_menu_list = {{0}},
    .quit = false,
    .exit_to_menu = false,
    .changed = true,
    .current_game_changed = true,
    .brightness_changed = false,
    .pop_menu_open = false,
    .show_time = false,
    .show_total = true,
    .show_legend = true,
    .is_overlay = false,
    .view_mode = VIEW_NORMAL,
    .view_restore = VIEW_NORMAL,
    .pop_menu_game_index = 0,
    .acc_ticks = 0,
    .last_ticks = 0,
    .time_step = 1000 / 30,
    .legend_start = 0,
    .legend_timeout = 5000,
    .brightness_start = 0,
    .brightness_timeout = 2000,
    .custom_header = NULL,
    .custom_footer = NULL,
    .header_height = 0,
    .footer_height = 0,
    .current_bg = NULL,
    .transparent_bg = NULL,
    .first_render = true,
    .current_game = 0,
    .surfaceGameName = NULL,
    .game_name_size = {0, 0},
    .game_name_max_width = 0,
    .gameNameScrollX = 0,
    .gameNameScrollSpeed = 10,
    .gameNameScrollStart = 20,
    .gameNameScrollEnd = 20};

static void sigHandler(int sig)
{
    switch (sig) {
    case SIGINT:
    case SIGTERM:
        appState.exit_to_menu = true;
        appState.quit = true;
        break;
    default:
        break;
    }
}

static char sTotalTimePlayed[50] = "";

Game_s *currentGame(void)
{
    return &game_list[appState.current_game];
}

#endif // GAME_SWITCHER_APP_STATE_H__