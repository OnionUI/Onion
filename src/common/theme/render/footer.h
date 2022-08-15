#ifndef RENDER_FOOTER_H__
#define RENDER_FOOTER_H__

#include "theme/config.h"
#include "theme/resources.h"
#include "theme/background.h"

void theme_renderStandardHint(SDL_Surface *screen, const char *btn_a_str, const char *btn_b_str)
{
    SDL_Surface *label_open, *label_back;

    SDL_Surface *button_a = resource_getSurface(BUTTON_A);
    SDL_Rect btn_a_rect = {20, 450 - button_a->h / 2};

    SDL_BlitSurface(button_a, NULL, screen, &btn_a_rect);

    char label_a_str[STR_MAX] = " ", label_b_str[STR_MAX] = " ";

    if (!theme()->hideLabels.hints) {
        strcpy(label_a_str, btn_a_str);
        strcpy(label_b_str, btn_b_str);
    }

    TTF_Font *font_hint = resource_getFont(HINT);
    label_open = TTF_RenderUTF8_Blended(font_hint, label_a_str, theme()->hint.color);

    SDL_Rect label_open_rect = {btn_a_rect.x + button_a->w + 5, 449 - label_open->h / 2};
    SDL_BlitSurface(label_open, NULL, screen, &label_open_rect);

    if (label_b_str) {
        SDL_Surface *button_b = resource_getSurface(BUTTON_B);
        SDL_Rect btn_b_rect = {label_open_rect.x + label_open->w + 30, 450 - button_b->h / 2};
        SDL_BlitSurface(button_b, NULL, screen, &btn_b_rect);

        label_back = TTF_RenderUTF8_Blended(font_hint, label_b_str, theme()->hint.color);
        
        SDL_Rect label_back_rect = {btn_b_rect.x + button_b->w + 5, 449 - label_back->h / 2};
        SDL_BlitSurface(label_back, NULL, screen, &label_back_rect);
        SDL_FreeSurface(label_back);
    }

    SDL_FreeSurface(label_open);
}

void theme_renderFooter(SDL_Surface *screen)
{
    SDL_Rect footer_rect = {0, 420};
	SDL_BlitSurface(resource_getSurface(BG_FOOTER), NULL, screen, &footer_rect);
}

static int old_status_width = 200;

void theme_renderFooterStatus(SDL_Surface *screen, int current_num, int total_num)
{
    SDL_Rect status_pos = {620 - old_status_width, 420, old_status_width, 60};
    SDL_Rect status_size = {status_pos.x, 0, old_status_width, 60};

    SDL_BlitSurface(theme_background(), &status_pos, screen, &status_pos);
    SDL_BlitSurface(resource_getSurface(BG_FOOTER), &status_size, screen, &status_pos);

    TTF_Font *font_hint = resource_getFont(HINT);

    if (total_num == 0)
        current_num = 0;
    
    char current_str[16];
    sprintf(current_str, "%d/", current_num);
    SDL_Surface *current = TTF_RenderUTF8_Blended(font_hint, current_str, theme()->currentpage.color);

    char total_str[16];
    sprintf(total_str, "%d", total_num);
    SDL_Surface *total = TTF_RenderUTF8_Blended(font_hint, total_str, theme()->total.color);

    SDL_Rect total_rect = {620 - total->w, 449 - total->h / 2};
    SDL_Rect current_rect = {total_rect.x - current->w, 449 - current->h / 2};
    old_status_width = total->w + current->w;
    
    SDL_BlitSurface(total, NULL, screen, &total_rect);
    SDL_BlitSurface(current, NULL, screen, &current_rect);

    SDL_FreeSurface(current);
    SDL_FreeSurface(total);
}

void theme_renderListFooter(SDL_Surface *screen, int current_num, int total_num, const char *label_a_str, const char *label_b_str)
{    
    theme_renderFooter(screen);
    theme_renderStandardHint(screen, label_a_str, label_b_str);
    theme_renderFooterStatus(screen, current_num, total_num);
}

#endif // RENDER_FOOTER_H__
