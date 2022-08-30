#ifndef RENDER_HEADER_H__
#define RENDER_HEADER_H__

#include "utils/surfaceSetAlpha.h"
#include "theme/config.h"
#include "theme/resources.h"
#include "theme/background.h"

void theme_renderHeaderBackground(SDL_Surface *screen)
{
    SDL_Rect header_size = {0, 0, 640, 60};
    SDL_BlitSurface(theme_background(), &header_size, screen, &header_size);
	SDL_BlitSurface(resource_getSurface(BG_TITLE), &header_size, screen, &header_size);
}

void theme_renderHeaderBattery(SDL_Surface *screen, int battery_percentage)
{
    SDL_Surface *battery = theme_batterySurface(battery_percentage);
    SDL_Rect battery_rect = {596 - battery->w / 2, 30 - battery->h / 2};
	SDL_BlitSurface(battery, NULL, screen, &battery_rect);
    SDL_FreeSurface(battery);
}

void theme_renderHeaderBatteryCustom(SDL_Surface *screen, int battery_percentage, int header_height)
{
    SDL_Surface *battery = theme_batterySurface(battery_percentage);
    SDL_Rect battery_rect = {596 - battery->w / 2, header_height / 2 - battery->h / 2};
	SDL_BlitSurface(battery, NULL, screen, &battery_rect);
    SDL_FreeSurface(battery);
}

void theme_renderHeader(SDL_Surface* screen, const char *title_str, bool show_logo)
{
    theme_renderHeaderBackground(screen);

    if (show_logo) {
        SDL_Surface *logo = resource_getSurface(LOGO);
        SDL_Rect logo_rect = {20, 30 - logo->h / 2};
        SDL_BlitSurface(logo, NULL, screen, &logo_rect);
    }

    if (title_str) {
        SDL_Surface *title = TTF_RenderUTF8_Blended(resource_getFont(TITLE), title_str, theme()->title.color);
        if (title) {
            SDL_Rect title_rect = {320 - title->w / 2, 29 - title->h / 2};
            SDL_Rect title_bg = {title_rect.x - 10, 0, title->w + 20, 60};
            SDL_BlitSurface(theme_background(), &title_bg, screen, &title_bg);
            SDL_BlitSurface(resource_getSurface(BG_TITLE), &title_bg, screen, &title_bg);
            SDL_BlitSurface(title, NULL, screen, &title_rect);
            SDL_FreeSurface(title);
        }
    }
}

void theme_renderHeaderExtra(SDL_Surface* screen, const char *title_str, const char *prev_title_str)
{
    theme_renderHeaderBackground(screen);

    SDL_Surface *title = TTF_RenderUTF8_Blended(resource_getFont(TITLE), title_str, theme()->title.color);
    SDL_Rect title_rect = {320 - title->w / 2, 29 - title->h / 2};
    SDL_BlitSurface(title, NULL, screen, &title_rect);
    SDL_FreeSurface(title);
}

#endif // RENDER_HEADER_H__
