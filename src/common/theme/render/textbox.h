#ifndef RENDER_TEXTBOX_H__
#define RENDER_TEXTBOX_H__

#include "theme/config.h"
#include "theme/resources.h"

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

#endif // RENDER_TEXTBOX_H__
