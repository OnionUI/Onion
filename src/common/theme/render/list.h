#ifndef RENDER_MENU_H__
#define RENDER_MENU_H__

#include "theme/config.h"
#include "theme/resources.h"
#include "components/list.h"

static SDL_Color color_black = {0, 0, 0};

void theme_renderListLabel(Theme_s* theme, Resources_s* res, SDL_Surface* screen, const char *label, SDL_Color fg, int center_y, bool is_active)
{
    SDL_Surface *item_label = TTF_RenderUTF8_Blended(res->fonts.list, label, fg);
    SDL_Rect item_label_rect = {20, center_y - item_label->h / 2};
    SDL_Rect item_shadow_rect = {item_label_rect.x + 1, item_label_rect.y + 2};

    if (is_active) {
        SDL_Surface *item_shadow = TTF_RenderUTF8_Blended(res->fonts.list, label, color_black);
        SDL_BlitSurface(item_shadow, NULL, screen, &item_shadow_rect);
        SDL_FreeSurface(item_shadow);
    }

    SDL_BlitSurface(item_label, NULL, screen, &item_label_rect);
    SDL_FreeSurface(item_label);
}

void theme_renderList(Theme_s* theme, Resources_s* res, SDL_Surface* screen, List* list)
{
	Theme_Surfaces_s* s = &res->surfaces;

    bool list_small = list->list_type == LIST_SMALL;

    SDL_Rect item_bg_rect = {0, 60},
             item_div_size = {0, 0, 640, 4},
             item_bg_size = {0, 0, 640, list_small ? 56 : 90},
             toggle_rect = {0, 0};

    int item_padding = list_small ? 4 : 0;
    int item_height = list_small ? 60 : 90;
    int label_y = list_small ? 27 : 37;
    SDL_Surface *item_bg = list_small ? s->bg_list_small : s->bg_list_large;

    TTF_SetFontStyle(res->fonts.list, TTF_STYLE_BOLD);
    int menu_pos_y = 420 - list->scroll_height * item_height;
    int last_item = list->scroll_pos + list->scroll_height;
    
    if (last_item > list->item_count)
        last_item = list->item_count;

    for (int i = list->scroll_pos; i < last_item; i++) {
        ListItem* item = &list->items[i];
        item_bg_rect.y = menu_pos_y + (i - list->scroll_pos) * item_height;
        
        SDL_BlitSurface(s->horizontal_divider, &item_div_size, screen, &item_bg_rect);
        item_bg_rect.y += item_padding;

        if (i == list->active_pos)
            SDL_BlitSurface(item_bg, &item_bg_size, screen, &item_bg_rect);

        theme_renderListLabel(theme, res, screen, item->label, theme->list.color, item_bg_rect.y + label_y, list->active_pos == i);

        if (!list_small && strlen(item->description)) {
            theme_renderListLabel(theme, res, screen, item->description, theme->grid.color, item_bg_rect.y + 62, list->active_pos == i);
        }

        if (item->is_toggle) {
            SDL_Surface *toggle = item->state ? s->toggle_on : s->toggle_off;
            toggle_rect.x = 620 - toggle->w;
            toggle_rect.y = item_bg_rect.y + item_bg_size.h / 2 - toggle->h / 2;
            SDL_BlitSurface(toggle, NULL, screen, &toggle_rect);
        }
    }
}

#endif // RENDER_MENU_H__
