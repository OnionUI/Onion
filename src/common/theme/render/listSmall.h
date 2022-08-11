#ifndef RENDER_MENU_H__
#define RENDER_MENU_H__

#include "theme/config.h"
#include "theme/resources.h"
#include "components/listSmall.h"

void theme_renderListSmall(Theme_s* theme, Resources_s* res, SDL_Surface* screen, ListSmall* list)
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
    int menu_pos_y = 420 - list->scroll_height * 60;
    int last_item = list->scroll_pos + list->scroll_height;
    
    if (last_item > list->item_count)
        last_item = list->item_count;

    for (i = list->scroll_pos; i < last_item; i++) {
        ListSmallItem* item = &list->items[i];
        item_bg_rect.y = menu_pos_y + (i - list->scroll_pos) * 60;
        
        SDL_BlitSurface(s->horizontal_divider, &item_div_size, screen, &item_bg_rect);
        item_bg_rect.y += 4;

        if (i == list->active_pos)
            SDL_BlitSurface(s->bg_list_small, &item_bg_size, screen, &item_bg_rect);

        item_label = TTF_RenderUTF8_Blended(res->fonts.list, item->label, theme->list.color);
        item_label_rect.x = 20;
        item_label_rect.y = item_bg_rect.y + 27 - item_label->h / 2;

        if (i == list->active_pos) {
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

#endif // RENDER_MENU_H__
