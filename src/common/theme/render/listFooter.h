#ifndef RENDER_LIST_FOOTER_H__
#define RENDER_LIST_FOOTER_H__

#include "./footer.h"
#include "components/listSmall.h"

void theme_renderListFooter(Theme_s *theme, Resources_s *res, SDL_Surface *screen, int current_num, int total_num, char *label_open_str, char *label_back_str)
{
	Theme_Surfaces_s *s = &res->surfaces;
    
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

#endif // RENDER_LIST_FOOTER_H__
