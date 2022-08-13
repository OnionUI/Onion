#ifndef RENDER_HEADER_H__
#define RENDER_HEADER_H__

#include "theme/config.h"
#include "theme/resources.h"

void theme_renderHeader(SDL_Surface* screen, SDL_Surface* battery, const char *title_str, bool show_logo)
{    
	SDL_BlitSurface(resource_getSurface(BG_TITLE), NULL, screen, NULL);

    if (show_logo) {
        SDL_Surface *logo = resource_getSurface(LOGO);
        SDL_Rect logo_rect = {20, 30 - logo->h / 2};
        SDL_BlitSurface(logo, NULL, screen, &logo_rect);
    }

    if (title_str) {
        SDL_Surface *title = TTF_RenderUTF8_Blended(resource_getFont(TITLE), title_str, theme()->title.color);
        SDL_Rect title_rect = {320 - title->w / 2, 29 - title->h / 2};
        SDL_BlitSurface(title, NULL, screen, &title_rect);
        SDL_FreeSurface(title);
    }

    SDL_Rect battery_rect = {596 - battery->w / 2, 30 - battery->h / 2};
	SDL_BlitSurface(battery, NULL, screen, &battery_rect);
}

#endif // RENDER_HEADER_H__
