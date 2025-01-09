#ifndef THEME_RENDER_POP_MENU_H__
#define THEME_RENDER_POP_MENU_H__

#include "theme/config.h"
#include "theme/resources.h"

#include "utils/log.h"

void theme_renderPopMenu(SDL_Surface *screen, int y_pos, int size)
{
    SDL_Surface *bg = resource_getPopMenuBg(size);

    if (bg) {
        SDL_Rect bg_rect = {0, y_pos, g_display.width, g_display.height - y_pos};
        SDL_BlitSurface(bg, NULL, screen, &bg_rect);
    }
}

#endif // THEME_RENDER_POP_MENU_H__
