#ifndef THEME_RENDER_POP_MENU_H__
#define THEME_RENDER_POP_MENU_H__

#include "theme/config.h"
#include "theme/resources.h"

#include "components/list.h"
#include "utils/log.h"

#include "footer.h"
#include "list.h"

void theme_renderPopMenu(SDL_Surface *screen, int y_pos, List *list, SDL_Surface *transparent_bg)
{
    int size = list->scroll_height < 4 ? list->scroll_height : 4;
    SDL_Surface *bg = resource_getPopMenuBg(size);

    if (bg && bg->h > (g_display.height - y_pos - 60.0 * g_scale)) {
        y_pos = 0;
    }

    SDL_Rect bg_rect = {0, y_pos, g_display.width, g_display.height - y_pos};

    if (transparent_bg) {
        SDL_BlitSurface(transparent_bg, NULL, screen, &bg_rect);
    }

    theme_renderFooter(screen);
    theme_renderStandardHint(screen, lang_get(LANG_SELECT, LANG_FALLBACK_SELECT), lang_get(LANG_BACK, LANG_FALLBACK_BACK));

    if (bg) {
        SDL_BlitSurface(bg, NULL, screen, &bg_rect);
    }

    ListRenderParams_s params = {
        .background = NULL,
        .dim = {0, 0, 320.0 * g_scale, (double)size * 60.0 * g_scale},
        .pos = {0, y_pos},
        .show_dividers = false,
        .stretch_y = true,
        .preview_bg = false,
        .preview_stretch = true,
        .preview_width = 320,
        .preview_smoothing = false,
    };

    if (bg) {
        params.dim.w = bg->w;
        params.dim.h = bg->h;
    }

    theme_renderListCustom(screen, list, params);
}

#endif // THEME_RENDER_POP_MENU_H__
