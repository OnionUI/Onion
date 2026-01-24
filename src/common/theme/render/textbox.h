#ifndef RENDER_TEXTBOX_H__
#define RENDER_TEXTBOX_H__

#include "theme/config.h"
#include "theme/resources.h"

#include "header.h"

typedef enum { ALIGN_LEFT,
               ALIGN_CENTER,
               ALIGN_RIGHT } text_alignment_e;

/**
 * @brief Creates an SDL_Surface containing a textbox with the given message.
 *        Newlines in the message are handled appropriately, creating multiple lines.
 *        Double newlines create paragraph spacing.
 *        The textbox will have a transparent background and the text will be
 *        rendered with the given font and color.
 * @param message The message to be displayed in the textbox.
 * @param font The TTF_Font to be used for rendering the text.
 * @param fg The foreground color (text color).
 * @param align The text alignment within the textbox.
 * @return An SDL_Surface containing the rendered textbox, or NULL if nothing could be rendered.
 */
SDL_Surface *theme_textboxSurface(const char *message, TTF_Font *font,
                                  SDL_Color fg, text_alignment_e align)
{
    char s[STR_MAX];
    strncpy(s, message, sizeof(s) - 1);
    s[sizeof(s) - 1] = '\0';

    int max_lines = 8; // start small, grow if needed
    int line_count = 0, max_width = 0, empty_lines = 0;
    int *line_widths = malloc(max_lines * sizeof(int));
    char **lines = malloc(max_lines * sizeof(char *));

    TTF_Font *cur_font = font;
    int font_lineskip = TTF_FontLineSkip(cur_font);
    int line_height = 1.2 * font_lineskip; // default height
    int paragraph_spacing = (int)(0.5 * font_lineskip);

    // --- First pass: measure each line
    size_t msglen = strlen(s);
    size_t start = 0;
    for (size_t i = 0; i <= msglen; i++) {
        if (s[i] != '\n' && s[i] != '\0') {
            continue;
        }
        size_t len = i - start;
        if (line_count >= max_lines) {
            max_lines *= 2;
            lines = realloc(lines, max_lines * sizeof(char *));
            line_widths = realloc(line_widths, max_lines * sizeof(int));
        }
        char *linebuf = malloc(len + 1);
        memcpy(linebuf, &s[start], len);
        linebuf[len] = 0;
        lines[line_count] = linebuf;

        int w = 0, h = 0;
        if (linebuf[0] == '\0') {
            empty_lines++;
            line_widths[line_count] = 0;
        }
        else {
            if (TTF_SizeUTF8(cur_font, linebuf, &w, &h) == 0) {
                if (h > line_height)
                    line_height = h;
            }
            else {
                fprintf(stderr, "TTF_SizeUTF8 failed: %s\n", TTF_GetError());
            }
            line_widths[line_count] = w;
            if (w > max_width)
                max_width = w;
        }
        line_count++;
        start = i + 1;
    }

    if (line_count == 0) {
        free(lines);
        free(line_widths);
        return NULL;
    }

    // --- Calculate textbox height
    int textbox_height = (line_count - empty_lines) * line_height +
                         empty_lines * paragraph_spacing;

    // --- Create textbox surface
    SDL_Surface *textbox = SDL_CreateRGBSurface(
        0, max_width, textbox_height, 32, 0x00FF0000, 0x0000FF00,
        0x000000FF, 0xFF000000);
    SDL_FillRect(textbox, NULL, 0x000000FF);

    // --- Second pass: render each line if not empty
    int y = 0;
    for (int i = 0; i < line_count; ++i) {
        if (lines[i][0] == '\0') {
            y += paragraph_spacing; // empty line, just add spacing
        }
        else {
            SDL_Surface *surf = TTF_RenderUTF8_Blended(cur_font, lines[i], fg);
            if (surf) {
                SDL_SetAlpha(surf, 0, 0);
                SDL_Rect r = {0, y};
                // horizontal alignment
                if (align == ALIGN_CENTER)
                    r.x = (max_width - surf->w) / 2;
                else if (align == ALIGN_RIGHT)
                    r.x = max_width - surf->w;
                r.y = y + (line_height - surf->h) / 2;

                SDL_BlitSurface(surf, NULL, textbox, &r);
                SDL_FreeSurface(surf);
            }
            else {
                fprintf(stderr, "TTF_RenderUTF8_Blended failed: %s\n", TTF_GetError());
            }
            y += line_height;
        }
        free(lines[i]);
    }

    free(lines);
    free(line_widths);

    return textbox;
}

/**
 * @brief Creates an SDL_Surface containing text with a solid background color
 *
 * @param text The text to be rendered.
 * @param fg The foreground color (text color).
 * @param bg The background color.
 * @param padding The padding around the text relative to background.
 * @return The created surface containing the rendered text with margin.
 *         Returns NULL if the text rendering fails.
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
