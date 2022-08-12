#ifndef RENDER_FOOTER_H__
#define RENDER_FOOTER_H__

#include "theme/config.h"
#include "theme/resources.h"

void theme_renderStandardHint(Theme_s *theme, Resources_s *res, SDL_Surface *screen, const char *label_open_str, const char *label_back_str)
{
	Theme_Surfaces_s *s = &res->surfaces;

    SDL_Surface *label_open, *label_back;
    SDL_Rect btn_a_rect = {20, 450 - s->button_a->h / 2};

    SDL_BlitSurface(s->button_a, NULL, screen, &btn_a_rect);

    label_open = TTF_RenderUTF8_Blended(res->fonts.hint, label_open_str, theme->hint.color);

    SDL_Rect label_open_rect = {btn_a_rect.x + s->button_a->w + 5, 449 - label_open->h / 2};
    SDL_BlitSurface(label_open, NULL, screen, &label_open_rect);

    if (label_back_str) {
        SDL_Rect btn_b_rect = {label_open_rect.x + label_open->w + 30, 450 - s->button_b->h / 2};
        SDL_BlitSurface(s->button_b, NULL, screen, &btn_b_rect);

        label_back = TTF_RenderUTF8_Blended(res->fonts.hint, label_back_str, theme->hint.color);
        
        SDL_Rect label_back_rect = {btn_b_rect.x + s->button_b->w + 5, 449 - label_back->h / 2};
        SDL_BlitSurface(label_back, NULL, screen, &label_back_rect);
        SDL_FreeSurface(label_back);
    }

    SDL_FreeSurface(label_open);
}

void theme_renderFooter(Theme_s *theme, Resources_s *res, SDL_Surface *screen)
{
	Theme_Surfaces_s *s = &res->surfaces;
    
    SDL_Rect footer_rect = {0, 420};
	SDL_BlitSurface(s->bg_footer, NULL, screen, &footer_rect);
}

void theme_renderListFooter(Theme_s *theme, Resources_s *res, SDL_Surface *screen, int current_num, int total_num, const char *label_open_str, const char *label_back_str)
{    
    theme_renderFooter(theme, res, screen);
    theme_renderStandardHint(theme, res, screen, label_open_str, label_back_str);

    char current_str[16];
    sprintf(current_str, "%d/", current_num);
    SDL_Surface *current = TTF_RenderUTF8_Blended(res->fonts.hint, current_str, theme->currentpage.color);

    char total_str[16];
    sprintf(total_str, "%d", total_num);
    SDL_Surface *total = TTF_RenderUTF8_Blended(res->fonts.hint, total_str, theme->total.color);

    SDL_Rect total_rect = {620 - total->w, 449 - total->h / 2};
    SDL_BlitSurface(total, NULL, screen, &total_rect);

    SDL_Rect current_rect = {total_rect.x - current->w, 449 - current->h / 2};
    SDL_BlitSurface(current, NULL, screen, &current_rect);

    SDL_FreeSurface(current);
    SDL_FreeSurface(total);
}

#endif // RENDER_FOOTER_H__
