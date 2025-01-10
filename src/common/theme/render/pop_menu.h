#ifndef THEME_RENDER_POP_MENU_H__
#define THEME_RENDER_POP_MENU_H__

#include "theme/config.h"
#include "theme/resources.h"

#include "components/list.h"
#include "utils/log.h"

#include "list.h"

void theme_renderPopMenu(SDL_Surface *screen, int y_pos, List *list)
{
    int size = list->scroll_height < 4 ? list->scroll_height : 4;
    SDL_Surface *bg = resource_getPopMenuBg(size);

    if (bg) {
        if (bg->h > g_display.height - y_pos) {
            y_pos = 0;
        }

        SDL_Rect bg_rect = {0, y_pos, g_display.width, g_display.height - y_pos};
        SDL_BlitSurface(bg, NULL, screen, &bg_rect);
    }

    ListRenderParams_s params = {
        .background = NULL,
        .dim = {0, 0, 320.0 * g_scale, (double)size * 60.0 * g_scale},
        .pos = {0, y_pos},
        .show_dividers = false,
        .stretch_y = true,
    };

    if (bg) {
        params.dim.w = bg->w;
        params.dim.h = bg->h;
    }

    theme_renderListCustom(screen, list, params);
}

#endif // THEME_RENDER_POP_MENU_H__
