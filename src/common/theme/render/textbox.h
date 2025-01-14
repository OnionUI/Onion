#ifndef RENDER_TEXTBOX_H__
#define RENDER_TEXTBOX_H__

#include "theme/config.h"
#include "theme/resources.h"

#include "header.h"

typedef enum { ALIGN_LEFT,
               ALIGN_CENTER,
               ALIGN_RIGHT } text_alignment_e;

SDL_Surface *theme_textboxSurface(const char *message, TTF_Font *font,
                                  SDL_Color fg, text_alignment_e align)
{
    SDL_Surface *lines[6];
    int line_count = 0;
    int line_width = 0;
    int line_height = 1.2 * TTF_FontLineSkip(font);

    char *token = NULL;
    char *delim = "\n";
    char s[STR_MAX];
    strcpy(s, message);

    token = strtok(s, delim);
    while (token != NULL) {
        lines[line_count] = TTF_RenderUTF8_Blended(font, token, fg);
        if (lines[line_count]) {
            SDL_SetAlpha(lines[line_count], 0, 0); /* important */
            if (lines[line_count]->w > line_width)
                line_width = lines[line_count]->w;
            line_count++;
        }
        token = strtok(NULL, delim);
    }

    SDL_Surface *textbox = SDL_CreateRGBSurface(
        0, line_width, line_height * line_count, 32, 0x00FF0000, 0x0000FF00,
        0x000000FF, 0xFF000000); /* important */
    SDL_FillRect(textbox, NULL, 0x000000FF);

    SDL_Rect line_rect = {0, 0};

    int i;
    for (i = 0; i < line_count; i++) {
        if (!lines[i]) {
            continue;
        }
        if (align == ALIGN_CENTER)
            line_rect.x = (line_width - lines[i]->w) / 2;
        else if (align == ALIGN_RIGHT)
            line_rect.x = line_width - lines[i]->w;

        line_rect.y = line_height * i + (line_height - lines[i]->h) / 2;

        SDL_BlitSurface(lines[i], NULL, textbox, &line_rect);
        SDL_FreeSurface(lines[i]);
    }

    return textbox;
}

/**
 * @brief Creates an SDL_Surface containing text with a solid background color
 *
 * @param text The text to be rendered.
 * @param fg The foreground color (text color).
 * @param bg The background color.
 * @param padding The padding around the text relative to background.
 * @return SDL_Surface* The created surface containing the rendered text with margin.
 *                       Returns NULL if the text rendering fails.
 */
SDL_Surface *theme_createTextOverlay(const char *text, SDL_Color fg, SDL_Color bg, double bgAlpha, int padding)
{
    TTF_Init();
    TTF_Font *font = theme_loadFont(theme()->path, theme()->list.font, (int)(18.0 * g_scale));
    TTF_SetFontStyle(font, TTF_STYLE_BOLD);

    SDL_Surface *text_surface = TTF_RenderUTF8_Blended(font, text, fg);
    if (!text_surface) {
        fprintf(stderr, "TTF_RenderUTF8 failed: %s\n", TTF_GetError());
        return 0;
    }

    TTF_CloseFont(font);

    padding = (int)(padding * g_scale);

    int overlayW = text_surface->w + 2 * padding;
    int overlayH = text_surface->h + 2 * padding;

    SDL_Surface *overlay_surface = SDL_CreateRGBSurface(SDL_SWSURFACE, overlayW, overlayH, 32, 0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000);
    SDL_FillRect(overlay_surface, NULL, SDL_MapRGBA(overlay_surface->format, bg.r, bg.g, bg.b, (Uint8)(bgAlpha * 255.0)));

    SDL_Rect dest = {padding, padding};
    SDL_BlitSurface(text_surface, NULL, overlay_surface, &dest);

    SDL_FreeSurface(text_surface);
    return overlay_surface;
}

#endif // RENDER_TEXTBOX_H__
