#ifndef RENDER_FOOTER_H__
#define RENDER_FOOTER_H__

#include "theme/config.h"
#include "theme/resources.h"

void theme_renderStandardHint(Theme_s *theme, Resources_s *res, SDL_Surface *screen, char *label_open_str, char *label_back_str)
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

    if (label_open_str)
        SDL_FreeSurface(label_open);
}

void theme_renderFooter(Theme_s *theme, Resources_s *res, SDL_Surface *screen)
{
	Theme_Surfaces_s *s = &res->surfaces;
    
    SDL_Rect footer_rect = {0, 420};
	SDL_BlitSurface(s->bg_footer, NULL, screen, &footer_rect);
}

#endif // RENDER_FOOTER_H__
