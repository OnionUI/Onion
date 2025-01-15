#ifndef GAME_SWITCHER_RENDER_H__
#define GAME_SWITCHER_RENDER_H__

#include <SDL/SDL.h>

#include "system/display.h"
#include "system/lang.h"
#include "theme/theme.h"

#include "../playActivity/playActivityDB.h"

#include "gs_appState.h"
#include "gs_model.h"
#include "gs_popMenu.h"

void renderCentered(SDL_Surface *image, int view_mode, SDL_Rect *overrideSrcRect, SDL_Rect *overrideDestRect)
{
    int offSetX = (int)(g_display.width - image->w) / 2;
    int offSetY = (int)(g_display.height - image->h) / 2;

    SDL_Rect image_size = {0, 0, g_display.width, g_display.height};
    SDL_Rect image_pos = {offSetX, offSetY};

    if (view_mode == VIEW_NORMAL) {
        image_size.x = theme()->frame.border_left;
        image_size.w -= theme()->frame.border_left + theme()->frame.border_right;
        image_pos.x += theme()->frame.border_left;
    }

    if (overrideSrcRect != NULL) {
        image_size = *overrideSrcRect;
        image_size.x -= offSetX;
        image_size.y -= offSetY;
    }

    if (overrideDestRect != NULL) {
        image_pos = *overrideDestRect;
    }

    SDL_BlitSurface(image, &image_size, screen, &image_pos);
}

void render_showFullscreenMessage(const char *message, bool draw_bg)
{
    if (draw_bg && appState.current_bg != NULL) {
        SDL_FillRect(screen, NULL, 0);
        renderCentered(appState.current_bg, VIEW_FULLSCREEN, NULL, NULL);
    }
    theme_renderInfoPanel(screen, NULL, message, true);
    render();
}

SDL_Surface *loadOptionalImage(const char *resourceName)
{
    char image_path[STR_MAX];
    if (theme_getImagePath(theme()->path, resourceName, image_path)) {
        return theme_loadImage(theme()->path, resourceName);
    }
    return NULL;
}

int getHeightOrDefault(SDL_Surface *surface, int default_height)
{
    int height = surface ? surface->h : default_height;
    return height > 1 ? height : 0;
}

void renderGameName(AppState *state)
{
    SDL_Color color_white = {255, 255, 255};
    SDL_Surface *arrow_left = resource_getSurface(LEFT_ARROW_WB);
    SDL_Surface *arrow_right = resource_getSurface(RIGHT_ARROW_WB);
    int game_name_padding = arrow_left->w + 20;
    state->game_name_max_width = g_display.width - 2 * game_name_padding;

    Game_s *game = &game_list[state->current_game];
    SDL_Rect game_name_bg_size = theme_scaleRect((SDL_Rect){0, 0, 640, 60});
    SDL_Rect game_name_bg_pos = {0, 0};

    if (state->view_mode == VIEW_NORMAL) {
        game_name_bg_size.x = game_name_bg_pos.x = theme()->frame.border_left;
        game_name_bg_size.w -= theme()->frame.border_left + theme()->frame.border_right;
    }

    int name_pos = g_display.height - game_name_bg_size.h;
    if (state->view_mode == VIEW_NORMAL) {
        name_pos -= state->footer_height;
    }
    game_name_bg_size.y = game_name_bg_pos.y = name_pos;

    game_name_bg_pos.w = game_name_bg_size.w;
    game_name_bg_pos.h = game_name_bg_size.h;

    SDL_FillRect(screen, &game_name_bg_pos, 0);

    if (state->current_bg != NULL) {
        renderCentered(state->current_bg, state->view_mode, &game_name_bg_size, &game_name_bg_pos);
    }

    SDL_BlitSurface(state->transparent_bg, &game_name_bg_size, screen, &game_name_bg_pos);

    if (state->current_game > 0) {
        SDL_Rect arrow_left_rect = {(double)(theme()->frame.border_left + 10) * g_scale, 30.0 * g_scale - arrow_left->h / 2};
        arrow_left_rect.y += game_name_bg_pos.y;
        SDL_BlitSurface(arrow_left, NULL, screen, &arrow_left_rect);
    }

    if (state->current_game < game_list_len - 1) {
        SDL_Rect arrow_right_rect = {
            (double)(630 - theme()->frame.border_right) * g_scale - arrow_right->w,
            30.0 * g_scale - arrow_right->h / 2};
        arrow_right_rect.y += game_name_bg_pos.y;
        SDL_BlitSurface(arrow_right, NULL, screen, &arrow_right_rect);
    }

    char game_name_str[STR_MAX * 2 + 4];
    strcpy(game_name_str, game->shortname);

    if (state->current_game_changed) {
        if (state->surfaceGameName != NULL)
            SDL_FreeSurface(state->surfaceGameName);

        state->surfaceGameName = TTF_RenderUTF8_Blended(resource_getFont(TITLE), game_name_str, color_white);

        state->game_name_size.w = state->surfaceGameName->w < state->game_name_max_width ? state->surfaceGameName->w : state->game_name_max_width;
        state->game_name_size.h = state->surfaceGameName->h;

        state->gameNameScrollX = -state->gameNameScrollStart * state->gameNameScrollSpeed;

        state->current_game_changed = false;
    }

    SDL_Rect game_name_rect = {(g_display.width - state->surfaceGameName->w) / 2,
                               game_name_bg_pos.y + 30.0 * g_scale - state->surfaceGameName->h / 2};
    if (game_name_rect.x < game_name_padding)
        game_name_rect.x = game_name_padding;

    state->game_name_size.x =
        state->gameNameScrollX < (state->surfaceGameName->w - state->game_name_size.w)
            ? (state->gameNameScrollX > 0 ? state->gameNameScrollX : 0)
            : state->surfaceGameName->w - state->game_name_size.w;

    SDL_BlitSurface(state->surfaceGameName, &state->game_name_size, screen, &game_name_rect);

    if (state->surfaceGameName->w > state->game_name_max_width) {
        state->gameNameScrollX += state->gameNameScrollSpeed;

        if (state->gameNameScrollX > (state->surfaceGameName->w - state->game_name_size.w + state->gameNameScrollEnd * state->gameNameScrollSpeed))
            state->gameNameScrollX = -state->gameNameScrollStart * state->gameNameScrollSpeed;
    }
}

void renderHeader(AppState *state, int battery_percentage)
{
    char title_str[STR_MAX] = "GameSwitcher";
    Game_s *game = &game_list[state->current_game];

    if (state->show_time && game_list_len > 0) {
        if (strlen(game->totalTime) == 0) {
            str_serializeTime(game->totalTime, play_activity_get_play_time(game->recentItem.rompath));
        }
        strcpy(title_str, game->totalTime);

        if (state->show_total) {
            if (strlen(sTotalTimePlayed) == 0) {
                str_serializeTime(sTotalTimePlayed, play_activity_get_total_play_time());
            }
            sprintf(title_str + strlen(title_str), " / %s", sTotalTimePlayed);
        }
    }

    if (state->custom_header) {
        if (state->header_height > 0) {
            SDL_BlitSurface(state->custom_header, NULL, screen, NULL);
            SDL_Surface *title = TTF_RenderUTF8_Blended(
                resource_getFont(TITLE), title_str,
                theme()->title.color);
            if (title) {
                SDL_Rect title_rect = {(g_display.width - title->w) / 2,
                                       (state->header_height - title->h) / 2};
                SDL_BlitSurface(title, NULL, screen, &title_rect);
                SDL_FreeSurface(title);
            }
            theme_renderHeaderBatteryCustom(screen, battery_percentage, state->header_height);
        }
    }
    else {
        theme_renderHeader(screen, title_str, false);
        theme_renderHeaderBattery(screen, battery_percentage);
    }
}

void renderFooter(AppState *state)
{
    if (state->custom_footer) {
        if (state->footer_height > 0) {
            SDL_Rect footer_rect = {0, g_display.height - state->custom_footer->h};
            SDL_BlitSurface(state->custom_footer, NULL, screen, &footer_rect);
        }
    }
    else {
        theme_renderFooter(screen);
        theme_renderStandardHint(screen,
                                 lang_get(LANG_RESUME_UC, LANG_FALLBACK_RESUME_UC),
                                 lang_get(LANG_BACK, LANG_FALLBACK_BACK));
        if (!state->first_render) {
            theme_renderFooterStatus(screen,
                                     game_list_len > 0 ? state->current_game + 1 : 0,
                                     game_list_len);
        }
    }
}

void renderLegend(AppState *state)
{
    if (state->show_legend && state->view_mode != VIEW_FULLSCREEN) {
        SDL_Surface *legend = resource_getSurface(LEGEND_GAMESWITCHER);
        if (legend) {
            SDL_Rect legend_rect = {g_display.width - legend->w,
                                    state->view_mode == VIEW_NORMAL ? state->header_height : 0};
            SDL_BlitSurface(legend, NULL, screen, &legend_rect);
        }
    }
}

void renderBrightness(AppState *state)
{
    if (state->brightness_changed) {
        // Display luminosity slider
        SDL_Surface *brightness = resource_getBrightness(settings.brightness);
        bool vertical = brightness->h > brightness->w;
        SDL_Rect brightness_rect = {0, (double)(state->view_mode == VIEW_NORMAL ? 240 : 210) * g_scale - brightness->h / 2};
        if (!vertical) {
            brightness_rect.x = (g_display.width - brightness->w) / 2;
            brightness_rect.y = state->view_mode == VIEW_NORMAL ? state->header_height : 0;
        }
        SDL_BlitSurface(brightness, NULL, screen, &brightness_rect);
    }
}

#endif // GAME_SWITCHER_RENDER_H__