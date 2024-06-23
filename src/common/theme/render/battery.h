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

SDL_Surface *theme_batterySurfaceWithBg(int percentage, SDL_Surface *background)
{
    BatteryPercentage_s *style = &theme()->batteryPercentage;
    bool visible = style->visible;

    // Currently charging, hide text
    if (percentage == 500)
        visible = false;

    TTF_Font *font = resource_getFont(BATTERY);
    int offsetY = style->offsetY;

    // Correct Exo 2 font offset
    if (strncmp(TTF_FontFaceFamilyName(font), "Exo 2", 5) == 0)
        offsetY -= 0.075 * TTF_FontHeight(font);

    // Battery percentage text
    char buffer[5];
    sprintf(buffer, "%d%%", percentage);

    // Battery icon
    ThemeImages icon_request = _getBatteryRequest(percentage);
    SDL_Surface *icon = resource_getSurface(icon_request);

    SDL_Surface *image = NULL;
    SDL_Surface *text = TTF_RenderUTF8_Blended(font, buffer, style->color);

    if (!text) {
        text = SDL_CreateRGBSurface(0, 1, 1, 32, 0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000);
    }

    SDL_SetAlpha(text, 0, SDL_ALPHA_TRANSPARENT); /* important */

    if (icon->w > 640)
        visible = false;

    const int SPACER = 5;
    int img_width = 2 * (text->w + SPACER) + icon->w;
    int img_height = text->h > icon->h ? text->h : icon->w;

    if (!visible) {
        img_width = icon->w;
        img_height = icon->h;
    }
    else if (style->fixed || style->textAlign == CENTER) {
        img_width = icon->w > text->w ? icon->w : text->w;
    }

    if (img_width % 2 != 0)
        img_width += 1;
    if (img_height % 2 != 0)
        img_height += 1;

    if (img_width < 48)
        img_width = 48;
    if (img_height < 48)
        img_height = 48;

    image = SDL_CreateRGBSurface(0, img_width, img_height, 32, 0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000);

    SDL_Rect rect_icon = {0, (img_height - icon->h) / 2};
    SDL_Rect rect_text = {0, (img_height - text->h) / 2 + offsetY};

    if (visible) {
        if (style->fixed) {
            switch (style->textAlign) {
            case RIGHT:
                rect_text.x = icon->w - text->w + style->offsetX;
                break;
            case CENTER:
                rect_text.x = (icon->w - text->w) / 2 + style->offsetX;
                break;
            case LEFT:
            default:
                rect_text.x = style->offsetX;
                break;
            }
        }
        else {
            switch (style->textAlign) {
            case RIGHT:
                rect_icon.x = text->w + SPACER;
                rect_text.x = style->offsetX;
                break;
            case CENTER:
                rect_text.x = (icon->w - text->w) / 2 + style->offsetX;
                break;
            case LEFT:
            default:
                rect_text.x = icon->w + SPACER + style->offsetX;
                break;
            }
        }
    }

    icon = SDL_ConvertSurface(icon, image->format, 0);
    SDL_SetAlpha(icon, 0, SDL_ALPHA_TRANSPARENT); /* important */
    SDL_BlitSurface(icon, NULL, image, &rect_icon);

    if (visible)
        SDL_BlitSurface(text, NULL, image, &rect_text);

    if (background != NULL) {
        SDL_Surface *bg = SDL_ConvertSurface(background, image->format, 0);
        SDL_SetAlpha(bg, 0, SDL_ALPHA_TRANSPARENT);

        SDL_Surface *bg_title =
            SDL_ConvertSurface(resource_getSurface(BG_TITLE), image->format, 0);
        SDL_SetAlpha(bg_title, SDL_SRCALPHA, SDL_ALPHA_TRANSPARENT);
        SDL_BlitSurface(bg_title, NULL, bg, NULL);

        SDL_Rect bg_crop = {572, 6, 48, 48};
        SDL_Rect bg_pos = {(img_width - 48) / 2, (img_height - 48) / 2};

        rect_icon.x += bg_crop.x - bg_pos.x;
        rect_icon.y += bg_crop.y - bg_pos.y;
        rect_text.x += bg_crop.x - bg_pos.x;
        rect_text.y += bg_crop.y - bg_pos.y;

        SDL_SetAlpha(icon, SDL_SRCALPHA, SDL_ALPHA_TRANSPARENT);
        SDL_SetAlpha(text, SDL_SRCALPHA, SDL_ALPHA_TRANSPARENT);

        SDL_BlitSurface(icon, NULL, bg, &rect_icon);
        if (visible)
            SDL_BlitSurface(text, NULL, bg, &rect_text);

        SDL_BlitSurface(bg, &bg_crop, image, &bg_pos);

        SDL_FreeSurface(bg);
        SDL_FreeSurface(bg_title);
    }

    SDL_FreeSurface(text);
    SDL_FreeSurface(icon);

    return image;
}

SDL_Surface *theme_batterySurface(int percentage)
{
    return theme_batterySurfaceWithBg(percentage, NULL);
}

#endif // RENDER_BATTERY_H__
