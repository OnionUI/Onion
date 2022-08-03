#ifndef THEME_RENDER_H__
#define THEME_RENDER_H__

#include <SDL/SDL.h>
#include "./theme.h"
#include "./theme_resources.h"
#include "./menu.h"


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

SDL_Surface* theme_textboxSurface(Theme_s* theme, Resources_s* res, char *message, TTF_Font *font, SDL_Color fg)
{
    SDL_Surface **lines = malloc(6 * sizeof(SDL_Surface*));
    int line_count = 0;
    int line_width = 0;
    int line_height = 1.2 * TTF_FontLineSkip(font);

    char *token = NULL;
    char *delim = "\n";
    
    token = strtok(message, delim);
    while (token != NULL) {
        lines[line_count] = TTF_RenderUTF8_Blended(font, token, fg);
        SDL_SetAlpha(lines[line_count], 0, 0); /* important */
        if (lines[line_count]->w > line_width)
            line_width = lines[line_count]->w;
        line_count++;
        token = strtok(NULL, delim);
    }

    SDL_Surface *textbox = SDL_CreateRGBSurface(0, line_width, line_height * line_count, 32,
        0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000); /* important */
    SDL_FillRect(textbox, NULL, 0x000000FF);

    SDL_Rect line_rect = {0, 0};
    
    int i;
    for (i = 0; i < line_count; i++) {
        line_rect.y = line_height * i + (line_height - lines[i]->h) / 2;
        SDL_BlitSurface(lines[i], NULL, textbox, &line_rect);
        SDL_FreeSurface(lines[i]);
    }

    return textbox;
}

void theme_renderBackground(Theme_s* theme, Resources_s* res, SDL_Surface* screen)
{
	SDL_BlitSurface(res->surfaces.background, NULL, screen, NULL);
}

void theme_renderHeader(Theme_s* theme, Resources_s* res, SDL_Surface* screen, SDL_Surface* battery, const char *title_str, bool show_logo)
{
	Theme_Surfaces_s* s = &res->surfaces;
    
	SDL_BlitSurface(s->bg_title, NULL, screen, NULL);

    if (show_logo) {
        SDL_Rect logo_rect = {20, 30 - s->logo->h / 2};
        SDL_BlitSurface(s->logo, NULL, screen, &logo_rect);
    }

    if (title_str) {
        SDL_Surface *title = TTF_RenderUTF8_Blended(res->fonts.title, title_str, theme->title.color);
        SDL_Rect title_rect = {320 - title->w / 2, 29 - title->h / 2};
        SDL_BlitSurface(title, NULL, screen, &title_rect);
        SDL_FreeSurface(title);
    }

    SDL_Rect battery_rect = {596 - battery->w / 2, 30 - battery->h / 2};
	SDL_BlitSurface(battery, NULL, screen, &battery_rect);
}

void theme_renderMenu(Theme_s* theme, Resources_s* res, SDL_Surface* screen, Menu_s* menu)
{
	Theme_Surfaces_s* s = &res->surfaces;

    int i;
    SDL_Rect item_bg_rect = {0, 64},
             item_div_size = {0, 0, 640, 4},
             item_bg_size = {0, 0, 640, 56},
             item_label_rect,
             item_shadow_rect,
             toggle_rect;
    SDL_Surface *item_label, *item_shadow;
    SDL_Color color_black = {0, 0, 0};

    TTF_SetFontStyle(res->fonts.list, TTF_STYLE_BOLD);
    int menu_pos_y = 420 - menu->scroll_height * 60;
    int last_item = menu->scroll_pos + menu->scroll_height;
    
    if (last_item > menu->item_count)
        last_item = menu->item_count;

    for (i = menu->scroll_pos; i < last_item; i++) {
        MenuItem_s* item = &menu->items[i];
        item_bg_rect.y = menu_pos_y + (i - menu->scroll_pos) * 60;
        
        SDL_BlitSurface(s->horizontal_divider, &item_div_size, screen, &item_bg_rect);
        item_bg_rect.y += 4;

        if (i == menu->active_pos)
            SDL_BlitSurface(s->bg_list_small, &item_bg_size, screen, &item_bg_rect);

        item_label = TTF_RenderUTF8_Blended(res->fonts.list, item->label, theme->list.color);
        item_label_rect.x = 20;
        item_label_rect.y = item_bg_rect.y + 27 - item_label->h / 2;

        if (i == menu->active_pos) {
            item_shadow = TTF_RenderUTF8_Blended(res->fonts.list, item->label, color_black);
            item_shadow_rect.x = item_label_rect.x + 1;
            item_shadow_rect.y = item_label_rect.y + 2;
            SDL_BlitSurface(item_shadow, NULL, screen, &item_shadow_rect);
            SDL_FreeSurface(item_shadow);
        }

        SDL_BlitSurface(item_label, NULL, screen, &item_label_rect);
        SDL_FreeSurface(item_label);

        if (item->is_toggle) {
            SDL_Surface *toggle = item->state ? s->toggle_on : s->toggle_off;
            toggle_rect.x = 620 - toggle->w;
            toggle_rect.y = item_bg_rect.y + 28 - toggle->h / 2;
            SDL_BlitSurface(toggle, NULL, screen, &toggle_rect);
        }
    }
}

void theme_renderStandardHint(Theme_s* theme, Resources_s* res, SDL_Surface* screen, char *label_open_str, char *label_back_str)
{
	Theme_Surfaces_s* s = &res->surfaces;

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

void theme_renderListFooter(Theme_s* theme, Resources_s* res, SDL_Surface* screen, Menu_s* menu, char *label_open_str, char *label_back_str)
{
	Theme_Surfaces_s* s = &res->surfaces;
    
    SDL_Rect footer_rect = {0, 420};
	SDL_BlitSurface(s->bg_footer, NULL, screen, &footer_rect);

    theme_renderStandardHint(theme, res, screen, label_open_str, label_back_str);

    char current_str[16];
    sprintf(current_str, "%d/", menu->active_pos + 1);
    SDL_Surface *current = TTF_RenderUTF8_Blended(res->fonts.hint, current_str, theme->currentpage.color);

    char total_str[16];
    sprintf(total_str, "%d", menu->item_count);
    SDL_Surface *total = TTF_RenderUTF8_Blended(res->fonts.hint, total_str, theme->total.color);

    SDL_Rect total_rect = {620 - total->w, 449 - total->h / 2};
    SDL_BlitSurface(total, NULL, screen, &total_rect);

    SDL_Rect current_rect = {total_rect.x - current->w, 449 - current->h / 2};
    SDL_BlitSurface(current, NULL, screen, &current_rect);

    SDL_FreeSurface(current);
    SDL_FreeSurface(total);
}


#endif // THEME_RENDER_H__
