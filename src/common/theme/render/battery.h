#ifndef RENDER_BATTERY_H__
#define RENDER_BATTERY_H__

#include "theme/config.h"
#include "theme/resources.h"

ThemeImages _getBatteryRequest(int percentage)
{
    if (percentage == 500)
        return BATTERY_CHARGING;
    if (percentage < 5)
        return BATTERY_0;
    if (percentage < 30)
        return BATTERY_20;
    if (percentage < 60)
        return BATTERY_50;
    if (percentage < 90)
        return BATTERY_80;
    return BATTERY_100;
}

SDL_Surface* theme_batterySurface(int percentage)
{
    BatteryPercentage_s* style = &theme()->batteryPercentage;
    bool visible = style->visible;

    // Currently charging, hide text
    if (percentage == 500)
        visible = false;

    TTF_Font* font = resource_getFont(BATTERY);
    int offsetY = style->offsetY;

    // Correct Exo 2 font offset
    if (strncmp(TTF_FontFaceFamilyName(font), "Exo 2", 5) == 0)
        offsetY -= 0.075 * TTF_FontHeight(font);

    // Battery percentage text
    char buffer[5];
    sprintf(buffer, "%d%%", percentage);
    SDL_Surface *text = TTF_RenderUTF8_Blended(font, buffer, style->color);
    SDL_SetAlpha(text, 0, 0); /* important */

    // Battery icon
    ThemeImages icon_request = _getBatteryRequest(percentage);
    SDL_Surface *icon = resource_getSurface(icon_request);
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
    SDL_Rect rect_text = {icon->w + SPACER + style->offsetX, (img_height - text->h) / 2 + offsetY};

    if (visible && style->onleft) {
        rect_text.x = style->offsetX;
        rect_icon.x = text->w + SPACER;
    }

    SDL_BlitSurface(icon, NULL, image, &rect_icon);

    if (visible)
        SDL_BlitSurface(text, NULL, image, &rect_text);

    SDL_FreeSurface(text);

    return image;
}

#endif // RENDER_BATTERY_H__
