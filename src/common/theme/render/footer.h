#ifndef RENDER_FOOTER_H__
#define RENDER_FOOTER_H__

#include "theme/config.h"
#include "theme/resources.h"

void theme_renderStandardHint(Resources_s *res, SDL_Surface *screen, const char *label_open_str, const char *label_back_str)
{
    SDL_Surface *label_open, *label_back;

    SDL_Surface *button_a = resource_getSurface(res, BUTTON_A);
    SDL_Rect btn_a_rect = {20, 450 - button_a->h / 2};

    SDL_BlitSurface(button_a, NULL, screen, &btn_a_rect);

    TTF_Font *font_hint = resource_getFont(res, HINT);
    label_open = TTF_RenderUTF8_Blended(font_hint, label_open_str, res->theme->hint.color);

    SDL_Rect label_open_rect = {btn_a_rect.x + button_a->w + 5, 449 - label_open->h / 2};
    SDL_BlitSurface(label_open, NULL, screen, &label_open_rect);

    if (label_back_str) {
        SDL_Surface *button_b = resource_getSurface(res, BUTTON_B);
        SDL_Rect btn_b_rect = {label_open_rect.x + label_open->w + 30, 450 - button_b->h / 2};
        SDL_BlitSurface(button_b, NULL, screen, &btn_b_rect);

        label_back = TTF_RenderUTF8_Blended(font_hint, label_back_str, res->theme->hint.color);
        
        SDL_Rect label_back_rect = {btn_b_rect.x + button_b->w + 5, 449 - label_back->h / 2};
        SDL_BlitSurface(label_back, NULL, screen, &label_back_rect);
        SDL_FreeSurface(label_back);
    }

    SDL_FreeSurface(label_open);
}

void theme_renderFooter(Resources_s *res, SDL_Surface *screen)
{
    SDL_Rect footer_rect = {0, 420};
	SDL_BlitSurface(resource_getSurface(res, BG_FOOTER), NULL, screen, &footer_rect);
}

void theme_renderListFooter(Resources_s *res, SDL_Surface *screen, int current_num, int total_num, const char *label_open_str, const char *label_back_str)
{    
    theme_renderFooter(res, screen);
    theme_renderStandardHint(res, screen, label_open_str, label_back_str);

    TTF_Font *font_hint = resource_getFont(res, HINT);

    char current_str[16];
    sprintf(current_str, "%d/", current_num);
    SDL_Surface *current = TTF_RenderUTF8_Blended(font_hint, current_str, res->theme->currentpage.color);

    char total_str[16];
    sprintf(total_str, "%d", total_num);
    SDL_Surface *total = TTF_RenderUTF8_Blended(font_hint, total_str, res->theme->total.color);

    SDL_Rect total_rect = {620 - total->w, 449 - total->h / 2};
    SDL_BlitSurface(total, NULL, screen, &total_rect);

    SDL_Rect current_rect = {total_rect.x - current->w, 449 - current->h / 2};
    SDL_BlitSurface(current, NULL, screen, &current_rect);

    SDL_FreeSurface(current);
    SDL_FreeSurface(total);
}

#endif // RENDER_FOOTER_H__
