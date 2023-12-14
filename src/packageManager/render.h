#ifndef PACMAN_RENDER_H__
#define PACMAN_RENDER_H__

#include <SDL/SDL.h>
#include <SDL/SDL_image.h>
#include <math.h>

#include "system/keymap_sw.h"
#include "utils/keystate.h"
#include "utils/surfaceSetAlpha.h"

#include "./globals.h"
#include "./summary.h"

SDL_Surface *createLabelSurface(Package *package)
{
    SDL_Surface *textbox =
        SDL_CreateRGBSurface(0, 593, 49, 32, 0x00FF0000, 0x0000FF00, 0x000000FF,
                             0xFF000000); /* important */
    SDL_FillRect(textbox, NULL, 0x000000FF);

    char label_text[STR_MAX], parens[STR_MAX] = "";
    strncpy(label_text, package->name, STR_MAX - 1);

    if (strchr(package->name, '(') != NULL) {
        parens[0] = '(';
        strncat(parens, str_split(label_text, "("), STR_MAX - 2);
    }

    SDL_Surface *label_surface =
        TTF_RenderUTF8_Blended(font25, label_text, color_white);
    SDL_SetAlpha(label_surface, 0, 0); /* important */
    SDL_Rect label_pos = {0, 0};
    SDL_BlitSurface(label_surface, NULL, textbox, &label_pos);

    if (package->installed && !package->complete)
        strcat(parens, "*");

    if (strlen(parens) > 0) {
        SDL_Surface *parens_surface =
            TTF_RenderUTF8_Blended(font25, parens, color_white);
        SDL_SetAlpha(parens_surface, 0, 0); /* important */
        surfaceSetAlpha(parens_surface, 120);
        SDL_Rect parens_pos = {label_surface->w, 0};
        SDL_BlitSurface(parens_surface, NULL, textbox, &parens_pos);
        SDL_FreeSurface(parens_surface);
    }

    SDL_FreeSurface(label_surface);

    return textbox;
}

void displayLayersNames(void)
{
    SDL_Rect rectResName = {30, 92, 80, 20};
    SDL_Surface *surfaceResName;
    for (int i = 0; i < 7; i++) {
        if ((i + nListPosition) < package_count[nTab]) {
            Package *package = &packages[nTab][i + nListPosition];
            surfaceResName = createLabelSurface(package);
            rectResName.y = 90 + i * 47;
            SDL_BlitSurface(surfaceResName, NULL, screen, &rectResName);
            SDL_FreeSurface(surfaceResName);
        }
    }
}

void renderFooter(const char *footer_str)
{
    SDL_Surface *footer =
        TTF_RenderUTF8_Blended(font18, footer_str, color_white);
    surfaceSetAlpha(footer, 120);
    SDL_Rect footer_rect = {320 - footer->w / 2, 414};
    SDL_BlitSurface(footer, NULL, screen, &footer_rect);
    SDL_FreeSurface(footer);
}

void displayLayersInstall(void)
{
    SDL_Rect rectInstall = {600 - surfaceCheck->w, 96};

    for (int i = 0; i < 7; i++) {
        if ((i + nListPosition) < package_count[nTab]) {
            Package *package = &packages[nTab][i + nListPosition];
            rectInstall.y = 107 - surfaceCheck->h / 2 + i * 47;
            if (package->installed != package->changed)
                SDL_BlitSurface(surfaceCheck, NULL, screen, &rectInstall);
            else
                SDL_BlitSurface(surfaceCross, NULL, screen, &rectInstall);

            if (package->has_roms) {
                SDL_Rect iconPos = {rectInstall.x - 34, rectInstall.y + 12};
                SDL_BlitSurface(surfaceIconHasRoms, NULL, screen, &iconPos);
            }

            if (package->changed) {
                SDL_Rect iconPos = {22, rectInstall.y + 8};
                SDL_BlitSurface(surfaceDotChanged, NULL, screen, &iconPos);
            }
        }
    }

    char footer_str[STR_MAX];
    sprintf(footer_str, "%d added  |  %d removed  |  %d installed  |  %d total",
            changes_installs[nTab], changes_removals[nTab],
            package_installed_count[nTab], package_count[nTab]);
    renderFooter(footer_str);
}

void showScroller(void)
{
    SDL_Rect rectMarker = {614, 87};
    SDL_Rect rectScroller = {611, 87};
    int scrollerHeight = 323;

    if (nTab == summary_tab) {
        if (surfaceSummary == NULL || surfaceSummary->h <= 323)
            return;

        scrollerHeight = round(323.0 * 323.0 / (float)(surfaceSummary->h));
        float scroll = (float)summaryScrollY / (float)(surfaceSummary->h - 323);
        rectScroller.y += round(scroll * (float)(323 - scrollerHeight));
    }
    else if (package_count[nTab] - 7 > 0) {
        scrollerHeight = round(323.0 * 7.0 / (float)package_count[nTab]);

        if (scrollerHeight < 10)
            scrollerHeight = 10;

        float scroll = (float)nListPosition / (float)(package_count[nTab] - 7);
        rectScroller.y += round(scroll * (float)(323 - scrollerHeight));
    }

    float markerSlice = 323.0 / (float)(package_count[nTab]);
    float markerOffset = 0.5 * markerSlice - (float)surfaceMarker->h / 2;

    for (int i = 0; i < package_count[nTab]; i++) {
        Package *package = &packages[nTab][i];

        if (package->installed == package->changed)
            continue;

        rectMarker.y = 87 + round((float)i * markerSlice + markerOffset);
        SDL_BlitSurface(surfaceMarker, NULL, screen, &rectMarker);
    }

    static SDL_Rect rectScrollerTop = {0, 0, 10, 5};
    static SDL_Rect rectScrollerMiddle = {0, 5, 10, 10};
    static SDL_Rect rectScrollerBottom = {0, 15, 10, 5};

    SDL_BlitSurface(surfaceScroller, &rectScrollerTop, screen, &rectScroller);
    rectScroller.y += 5;

    int scrollerMiddle = scrollerHeight - 10;

    while (scrollerMiddle > 0) {
        int h = scrollerMiddle > 10 ? 10 : scrollerMiddle;
        rectScrollerMiddle.h = h;
        SDL_BlitSurface(surfaceScroller, &rectScrollerMiddle, screen,
                        &rectScroller);
        rectScroller.y += h;
        scrollerMiddle -= h;
    }

    SDL_BlitSurface(surfaceScroller, &rectScrollerBottom, screen,
                    &rectScroller);
}

bool confirmDoNothing(KeyState *keystate)
{
    bool quit = false;
    SDL_Surface *image = IMG_Load("res/confirmDoNothing.png");

    SDL_BlitSurface(image, NULL, screen, NULL);
    SDL_BlitSurface(screen, NULL, video, NULL);
    SDL_Flip(video);

    bool confirm = false;

    while (!quit) {
        if (updateKeystate(keystate, &quit, true, NULL)) {
            if (keystate[SW_BTN_A] == PRESSED)
                confirm = true;
            else if (keystate[SW_BTN_A] == RELEASED && confirm)
                quit = true;
            else if (keystate[SW_BTN_B] == PRESSED)
                quit = true;
        }
    }

    SDL_FreeSurface(image);

    return confirm;
}

typedef enum Alignment { ALIGN_START,
                         ALIGN_CENTER,
                         ALIGN_END } alignment_e;

int alignCoord(int n, int size, alignment_e alignment)
{
    switch (alignment) {
    case ALIGN_START:
        return n;
    case ALIGN_CENTER:
        return n - size / 2;
    case ALIGN_END:
        return n - size;
    default:
        return n;
    }
}

void renderTabName(const char *name, int x, alignment_e alignment, bool active,
                   bool has_changes)
{
    char name_str[STR_MAX];
    snprintf(name_str, STR_MAX - 1, has_changes ? "%s*" : "%s", name);

    SDL_Surface *tab_name =
        TTF_RenderUTF8_Blended(active ? font25 : font18, name_str, color_white);
    SDL_Rect tab_rect = {alignCoord(x, tab_name->w, alignment),
                         (active ? 53 : 56) - 2 - tab_name->h / 2};

    SDL_BlitSurface(tab_name, NULL, screen, &tab_rect);
    SDL_FreeSurface(tab_name);
}

void renderCurrentTab(void)
{
    int lTab = nTab == 0 ? tab_count - 1 : nTab - 1,
        rTab = nTab == tab_count - 1 ? 0 : nTab + 1;

    renderTabName(layer_names[lTab], 50, ALIGN_START, false,
                  changes_installs[lTab] > 0 || changes_removals[lTab] > 0);
    renderTabName(layer_names[rTab], 590, ALIGN_END, false,
                  changes_installs[rTab] > 0 || changes_removals[rTab] > 0);
    renderTabName(layer_names[nTab], 320, ALIGN_CENTER, true,
                  changes_installs[nTab] > 0 || changes_removals[nTab] > 0);

    int tab_dots_width =
        (tab_count - 1) * surfaceDotNeutral->w + surfaceDotActive->w;
    SDL_Rect rectTabDot = {320 - tab_dots_width / 2, 14};

    for (int i = 0; i < tab_count; i++) {
        SDL_Surface *current_dot;

        if (i != summary_tab)
            current_dot = i == nTab ? surfaceDotActive : surfaceDotNeutral;
        else
            current_dot =
                i == nTab ? surfaceDotApplyActive : surfaceDotApplyNeutral;

        rectTabDot.y = 14 - current_dot->h / 2;

        SDL_BlitSurface(current_dot, NULL, screen, &rectTabDot);
        rectTabDot.x += current_dot->w;
    }
}

static SDL_Rect rectSelection = {14, 84, 593, 49};

void renderApplication(void)
{
    rectSelection.y = 83 + nSelection * 47;

    SDL_FillRect(screen, NULL, 0);
    SDL_BlitSurface(surfaceBackground, NULL, screen, NULL);
    SDL_BlitSurface(surfaceTableau, NULL, screen, NULL);

    int changes_total = changesTotal();

    if (changes_total > 0) {
        int installs_count = changesInstalls();
        int removals_count = changesRemovals();
        char status_str[STR_MAX] = "";

        if (installs_count > 0)
            sprintf(status_str, "+%d", installs_count);

        if (removals_count > 0) {
            int len = strlen(status_str);
            if (len > 0) {
                strcpy(status_str + len, "  ");
                len += 2;
            }
            sprintf(status_str + len, " −%d", removals_count);
        }

        SDL_Surface *status =
            TTF_RenderUTF8_Blended(font18, status_str, color_white);
        SDL_Rect status_rect = {620 - status->w, 16 - status->h / 2};
        SDL_BlitSurface(status, NULL, screen, &status_rect);
        SDL_FreeSurface(status);
    }

    renderCurrentTab();

    if (nTab == summary_tab) {
        if (changes_total > 0) {
            renderSummary();
            showScroller();
            renderFooter("Press A or START to apply changes");
        }
        else {
            SDL_Surface *status = TTF_RenderUTF8_Blended(
                font35, "NO CHANGES", color_white);
            SDL_Rect status_rect = {
                alignCoord(320, status->w, ALIGN_CENTER),
                alignCoord(247, status->h, ALIGN_CENTER)};
            SDL_BlitSurface(status, NULL, screen, &status_rect);
            SDL_FreeSurface(status);
            renderFooter("Press A or START to exit");
        }
    }
    else if (package_count[nTab] > 0) {
        if (surfaceSelection == NULL)
            SDL_FillRect(screen, &rectSelection, 0xFF333333);
        else
            SDL_BlitSurface(surfaceSelection, NULL, screen,
                            &rectSelection);

        displayLayersNames();
        showScroller();
        displayLayersInstall();
    }

    SDL_BlitSurface(screen, NULL, video, NULL);
    SDL_Flip(video);
}

#endif // PACMAN_RENDER_H__
