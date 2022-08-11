#ifndef RENDER_BATTERY_H__
#define RENDER_BATTERY_H__

#include "theme/config.h"
#include "theme/resources.h"

SDL_Surface* _getBatterySurface(Theme_Surfaces_s* s, int percentage)
{
    if (percentage == 500)
        return s->battery_charging;
    if (percentage <= 10)
        return s->battery_0;
    if (percentage <= 25)
        return s->battery_20;
    if (percentage <= 60)
        return s->battery_50;
    if (percentage <= 90)
        return s->battery_80;
    return s->battery_100;
}

SDL_Surface* theme_batterySurface(Theme_s* theme, Resources_s* res, int percentage)
{
    BatteryPercentage_s* style = &theme->batteryPercentage;
    bool visible = style->visible;

    // Currently charging, hide text
    if (percentage == 500)
        visible = false;

    TTF_Font* font = res->fonts.battery;

    // Correct Exo 2 font offset
    if (strncmp(TTF_FontFaceFamilyName(font), "Exo 2", 5) == 0)
        style->offsetY -= 0.075 * TTF_FontHeight(font);

    // Battery percentage text
    char buffer[5];
    sprintf(buffer, "%d%%", percentage);
    SDL_Surface *text = TTF_RenderUTF8_Blended(font, buffer, style->color);
    SDL_SetAlpha(text, 0, 0); /* important */

    // Battery icon
    SDL_Surface *icon = _getBatterySurface(&res->surfaces, percentage);
    SDL_SetAlpha(icon, 0, 0); /* important */

    const int SPACER = 5;
    int img_width = 2 * (text->w + SPACER) + icon->w;
    int img_height = text->h > icon->h ? text->h : icon->w;

    if (!visible) {
        img_width = icon->w;
        img_height = icon->h;
    }

    SDL_Surface *image = SDL_CreateRGBSurface(0, img_width, img_height, 32,
        0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000); /* important */

    SDL_Rect rect_icon = {0, (img_height - icon->h) / 2};
    SDL_Rect rect_text = {icon->w + SPACER + style->offsetX, (img_height - text->h) / 2 + style->offsetY};

    if (visible && style->onleft) {
        rect_text.x = 0;
        rect_icon.x = text->w + SPACER;
    }

    SDL_BlitSurface(icon, NULL, image, &rect_icon);

    if (visible)
        SDL_BlitSurface(text, NULL, image, &rect_text);

    SDL_FreeSurface(text);

    return image;
}

#endif // RENDER_BATTERY_H__
