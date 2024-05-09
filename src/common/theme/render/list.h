#ifndef RENDER_MENU_H__
#define RENDER_MENU_H__

#include "components/list.h"
#include "theme/background.h"
#include "theme/config.h"
#include "theme/resources.h"

#define HIDDEN_ITEM_ALPHA 60

// static SDL_Color color_black = {0, 0, 0};

void theme_renderListLabel(SDL_Surface *screen, const char *label, SDL_Color fg,
                           int offset_x, int center_y, bool is_active,
                           int label_end, bool disabled)
{
    TTF_Font *list_font = resource_getFont(LIST);

    SDL_Surface *item_label = TTF_RenderUTF8_Blended(list_font, label, fg);
    SDL_Rect item_label_rect = {offset_x, center_y - item_label->h / 2};

    SDL_Rect label_crop = {0, 0, label_end - 30, item_label->h};

    /* Maybe shadows will be an option in the future
    SDL_Rect item_shadow_rect = {item_label_rect.x + 1, item_label_rect.y + 2};

    if (is_active) {
        SDL_Surface *item_shadow =
            TTF_RenderUTF8_Blended(list_font, label, color_black);
        SDL_BlitSurface(item_shadow, &label_crop, screen, &item_shadow_rect);
        SDL_FreeSurface(item_shadow);
    }*/

    if (disabled) {
        surfaceSetAlpha(item_label, HIDDEN_ITEM_ALPHA);
    }

    SDL_BlitSurface(item_label, &label_crop, screen, &item_label_rect);
    SDL_FreeSurface(item_label);
}

void theme_renderList(SDL_Surface *screen, List *list)
{
    SDL_Rect bg_size = {0, 60, 640, 360}, bg_pos = {0, 60};
    SDL_BlitSurface(theme_background(), &bg_size, screen, &bg_pos);

    bool list_small = list->list_type == LIST_SMALL;

    TTF_Font *list_font = resource_getFont(LIST);

    SDL_Rect item_bg_rect = {0, 60}, item_div_size = {0, 0, 640, 4},
             item_bg_size = {0, 0, 640, list_small ? 56 : 90},
             toggle_rect = {0, 0};

    int item_padding = list_small ? 4 : 0;
    int item_height = list_small ? 60 : 90;
    int label_y = list_small ? 27 : 37;
    SDL_Surface *item_bg = resource_getSurface(list_small ? BG_LIST_S : BG_LIST_L);

    int menu_pos_y = 420 - list->scroll_height * item_height;
    int last_item = list->scroll_pos + list->scroll_height;

    if (last_item > list->item_count)
        last_item = list->item_count;

    ListItem *active_preview = NULL;

    SDL_Surface *hidden_toggle_off = resource_getSurfaceCopy(TOGGLE_OFF);
    SDL_Surface *hidden_toggle_on = resource_getSurfaceCopy(TOGGLE_ON);
    SDL_Surface *hidden_arrow_left = resource_getSurfaceCopy(LEFT_ARROW);
    SDL_Surface *hidden_arrow_right = resource_getSurfaceCopy(RIGHT_ARROW);
    surfaceSetAlpha(hidden_toggle_off, HIDDEN_ITEM_ALPHA);
    surfaceSetAlpha(hidden_toggle_on, HIDDEN_ITEM_ALPHA);
    surfaceSetAlpha(hidden_arrow_left, HIDDEN_ITEM_ALPHA);
    surfaceSetAlpha(hidden_arrow_right, HIDDEN_ITEM_ALPHA);

    if (list->has_sticky) {
        ListItem *active_item = &list->items[list->active_pos];
        SDL_BlitSurface(resource_getSurface(HORIZONTAL_DIVIDER), &item_div_size, screen, &item_bg_rect);
        theme_renderListLabel(screen, active_item->sticky_note, theme()->list.color, 20, item_bg_rect.y + label_y, false, 640, true);
    }

    for (int i = list->scroll_pos; i < last_item; i++) {
        ListItem *item = &list->items[i];
        bool show_disabled = item->disabled && !item->show_opaque;

        item_bg_rect.y = menu_pos_y + (i - list->scroll_pos) * item_height;

        SDL_BlitSurface(resource_getSurface(HORIZONTAL_DIVIDER), &item_div_size, screen, &item_bg_rect);
        item_bg_rect.y += item_padding;

        if (i == list->active_pos) {
            SDL_BlitSurface(item_bg, &item_bg_size, screen, &item_bg_rect);

            if (item->preview_ptr == NULL && strlen(item->preview_path) > 0 &&
                is_file(item->preview_path)) {
                item->preview_ptr = (void *)IMG_Load(item->preview_path);
            }

            if (item->preview_ptr != NULL)
                active_preview = item;
        }

        int item_center_y = item_bg_rect.y + item_bg_size.h / 2;
        static int multivalue_width = 226;
        int label_end = 640;
        int offset_x = 20;

        if (item->icon_ptr != NULL) {
            SDL_Surface *icon = (SDL_Surface *)item->icon_ptr;
            if (icon->w > 1) {
                SDL_Rect icon_rect = {offset_x, item_center_y - icon->h / 2};
                SDL_BlitSurface(icon, NULL, screen, &icon_rect);
                offset_x += icon->w + 17;
            }
        }

        if (item->item_type == TOGGLE) {
            SDL_Surface *toggle = show_disabled ? (item->value == 1 ? hidden_toggle_on : hidden_toggle_off) : (resource_getSurface(item->value == 1 ? TOGGLE_ON : TOGGLE_OFF));
            toggle_rect.x = 620 - toggle->w;
            toggle_rect.y = item_center_y - toggle->h / 2;
            label_end = toggle_rect.x;
            SDL_BlitSurface(toggle, NULL, screen, &toggle_rect);
        }
        else if (item->item_type == MULTIVALUE) {
            SDL_Surface *arrow_left = show_disabled ? hidden_arrow_left : resource_getSurface(LEFT_ARROW);
            SDL_Surface *arrow_right = show_disabled ? hidden_arrow_right : resource_getSurface(RIGHT_ARROW);
            SDL_Rect arrow_left_pos = {
                640 - 20 - arrow_right->w - multivalue_width - arrow_left->w,
                item_center_y - arrow_left->h / 2};
            SDL_Rect arrow_right_pos = {
                640 - 20 - arrow_right->w,
                item_center_y - arrow_right->h / 2};
            SDL_BlitSurface(arrow_left, NULL, screen, &arrow_left_pos);
            SDL_BlitSurface(arrow_right, NULL, screen, &arrow_right_pos);
            label_end = arrow_left_pos.x;

            char value_str[STR_MAX];
            list_getItemValueLabel(item, value_str);
            SDL_Surface *value_label = TTF_RenderUTF8_Blended(list_font, value_str, theme()->list.color);
            if (show_disabled) {
                surfaceSetAlpha(value_label, HIDDEN_ITEM_ALPHA);
            }
            SDL_Rect value_size = {0, 0, multivalue_width, value_label->h};
            int label_width = value_label->w > value_size.w ? value_size.w : value_label->w;
            SDL_Rect value_pos = {
                640 - 20 - arrow_right->w - multivalue_width / 2 - label_width / 2,
                item_center_y - value_size.h / 2};
            SDL_BlitSurface(value_label, &value_size, screen, &value_pos);
        }

        theme_renderListLabel(screen, item->label, theme()->list.color,
                              offset_x, item_bg_rect.y + label_y,
                              list->active_pos == i, label_end, show_disabled);

        if (!list_small && strlen(item->description)) {
            theme_renderListLabel(
                screen, item->description, theme()->grid.color, offset_x,
                item_bg_rect.y + 62, list->active_pos == i, label_end, show_disabled);
        }
    }

    SDL_FreeSurface(hidden_toggle_off);
    SDL_FreeSurface(hidden_toggle_on);
    SDL_FreeSurface(hidden_arrow_left);
    SDL_FreeSurface(hidden_arrow_right);

    if (active_preview != NULL) {
        SDL_Surface *preview_bg = resource_getSurface(PREVIEW_BG);
        SDL_Rect preview_bg_rect = {640 - preview_bg->w, 60};
        SDL_BlitSurface(preview_bg, NULL, screen, &preview_bg_rect);

        SDL_Surface *preview = (SDL_Surface *)active_preview->preview_ptr;
        bool free_after = false;

        if (preview->w > 250) {
            preview =
                rotozoomSurface(preview, 0.0, 250.0 / (double)preview->w, 0);
            free_after = true;
        }

        SDL_Rect preview_rect = {640 - preview_bg->w + 125 - preview->w / 2,
                                 240 - preview->h / 2};
        SDL_BlitSurface(preview, NULL, screen, &preview_rect);

        if (free_after)
            SDL_FreeSurface(preview);
    }
}

#endif // RENDER_MENU_H__
