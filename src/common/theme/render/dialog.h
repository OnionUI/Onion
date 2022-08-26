#ifndef THEME_RENDER_DIALOG_H__
#define THEME_RENDER_DIALOG_H__

#include "theme/config.h"
#include "theme/resources.h"
#include "utils/surfaceSetAlpha.h"
#include "./textbox.h"

static int dialog_progress = 0;

void theme_renderDialog(SDL_Surface *screen, const char *title_str, const char *message_str, bool show_hint)
{
    SDL_Surface *pop_bg = resource_getSurface(POP_BG);
    SDL_Rect center_rect = {320 - pop_bg->w / 2, 240 - pop_bg->h / 2};

    SDL_BlitSurface(pop_bg, NULL, screen, &center_rect);

    SDL_Surface *title = TTF_RenderUTF8_Blended(resource_getFont(TITLE), title_str, theme()->grid.selectedcolor);
    if (title) {
        SDL_Rect title_rect = {320 - title->w / 2, center_rect.y + 30 - title->h / 2};
        SDL_BlitSurface(title, NULL, screen, &title_rect);
        SDL_FreeSurface(title);
    }

    SDL_Surface *textbox = theme_textboxSurface(message_str, resource_getFont(TITLE), theme()->grid.color, ALIGN_CENTER);
    if (textbox) {
        SDL_Rect textbox_rect = {320 - textbox->w / 2, (show_hint ? 210 : 240) - textbox->h / 2};
        SDL_BlitSurface(textbox, NULL, screen, &textbox_rect);
        SDL_FreeSurface(textbox);
    }

    if (show_hint) {
        SDL_Rect hint_rect = {center_rect.x + 60, center_rect.y + pop_bg->h - 60};

        SDL_Surface *button_a = resource_getSurface(BUTTON_A);
        SDL_Rect button_a_rect = {hint_rect.x, hint_rect.y - button_a->h / 2};
        SDL_BlitSurface(button_a, NULL, screen, &button_a_rect);
        hint_rect.x += button_a->w + 5;

        SDL_Surface *label_ok = TTF_RenderUTF8_Blended(resource_getFont(HINT), lang_get(LANG_OK), theme()->hint.color);
        if (label_ok) {
            SDL_Rect label_ok_rect = {hint_rect.x, hint_rect.y - label_ok->h / 2};
            SDL_BlitSurface(label_ok, NULL, screen, &label_ok_rect);
            hint_rect.x += label_ok->w + 30;
            SDL_FreeSurface(label_ok);
        }

        SDL_Surface *button_b = resource_getSurface(BUTTON_B);
        SDL_Rect button_b_rect = {hint_rect.x, hint_rect.y - button_b->h / 2};
        SDL_BlitSurface(button_b, NULL, screen, &button_b_rect);
        hint_rect.x += button_b->w + 5;

        SDL_Surface *label_cancel = TTF_RenderUTF8_Blended(resource_getFont(HINT), lang_get(LANG_CANCEL), theme()->hint.color);
        if (label_cancel) {
            SDL_Rect label_cancel_rect = {hint_rect.x, hint_rect.y - label_cancel->h / 2};
            SDL_BlitSurface(label_cancel, NULL, screen, &label_cancel_rect);
            hint_rect.x += label_cancel->w + 30;
            SDL_FreeSurface(label_cancel);
        }
    }
}

void theme_renderDialogProgress(SDL_Surface *screen, const char *title_str, const char *message_str, bool show_hint)
{
    theme_renderDialog(screen, title_str, message_str, show_hint);

    SDL_Surface *dot = resource_getSurface(PROGRESS_DOT);
    SDL_Rect dot_rect = {320 - 32 - dot->w / 2, 225 - dot->h / 2};

    if (dialog_progress >= 1) SDL_BlitSurface(dot, NULL, screen, &dot_rect);
    dot_rect.x += 32;
    if (dialog_progress >= 2) SDL_BlitSurface(dot, NULL, screen, &dot_rect);
    dot_rect.x += 32;
    if (dialog_progress >= 3) SDL_BlitSurface(dot, NULL, screen, &dot_rect);

    dialog_progress = (dialog_progress + 1) % 4;
}

void theme_clearDialogProgress(void) {
    dialog_progress = 0;
}

#endif // THEME_RENDER_DIALOG_H__
