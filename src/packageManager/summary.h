#ifndef PACMAN_SUMMARY_H__
#define PACMAN_SUMMARY_H__

#include <SDL/SDL.h>
#include <SDL/SDL_ttf.h>

#include "utils/file.h"
#include "utils/str.h"
#include "utils/surfaceSetAlpha.h"

#include "./globals.h"

int renderSummaryLine(SDL_Surface *surfaceTemp, int pos_y, const char *line_str,
                      int alpha, SDL_Color color)
{
    SDL_Surface *surfaceLine = TTF_RenderUTF8_Blended(font18, line_str, color);
    SDL_Rect rectLine = {0, pos_y};
    int h = surfaceLine->h;

    SDL_SetAlpha(surfaceLine, 0, 0); /* important */
    surfaceSetAlpha(surfaceLine, alpha);

    SDL_BlitSurface(surfaceLine, NULL, surfaceTemp, &rectLine);
    SDL_FreeSurface(surfaceLine);

    return h;
}

void renderSummary()
{
    SDL_Rect rectSummaryFrame = {0, summaryScrollY, 593, 327};

    if (surfaceSummary == NULL) {
        SDL_Surface *surfaceTemp =
            SDL_CreateRGBSurface(0, 593, 4096, 32, 0x00FF0000, 0x0000FF00,
                                 0x000000FF, 0xFF000000); /* important */
        SDL_SetAlpha(surfaceTemp, 0, 0);                  /* important */

        int pos_y = 0;

        for (int nT = 0; nT < tab_count; nT++) {
            const char *data_path = layer_dirs[nT];

            if (strlen(data_path) == 0 || !exists(data_path))
                continue;

            if (changes_installs[nT] + changes_removals[nT] == 0)
                continue;

            pos_y += 10;

            char line_str[STR_MAX * 2];
            sprintf(line_str, "%s:", layer_names[nT]);

            if (changes_installs[nT] > 0)
                sprintf(line_str + strlen(line_str), " %d added",
                        changes_installs[nT]);

            if (changes_removals[nT] > 0) {
                int len = strlen(line_str);
                if (changes_installs[nT] > 0) {
                    strcpy(line_str + len, ",");
                    len += 1;
                }
                sprintf(line_str + len, " %d removed", changes_removals[nT]);
            }

            pos_y += renderSummaryLine(surfaceTemp, pos_y, line_str, 255,
                                       color_white) +
                     5;

            for (int i = 0; i < package_count[nT]; i++) {
                Package *package = &packages[nT][i];

                if (!package->changed &&
                    (!package->installed || package->complete))
                    continue;

                memset(line_str, 0, STR_MAX * 2);

                bool is_removed = package->changed && package->installed;
                sprintf(line_str, "[%s]  %s", is_removed ? "−" : "+",
                        package->name);

                pos_y +=
                    renderSummaryLine(surfaceTemp, pos_y, line_str, 120,
                                      is_removed ? color_red : color_green) +
                    5;
            }
        }

        surfaceSummary =
            SDL_CreateRGBSurface(0, 593, pos_y, 32, 0x00FF0000, 0x0000FF00,
                                 0x000000FF, 0xFF000000); /* important */
        SDL_FillRect(surfaceSummary, NULL, 0x000000FF);
        SDL_BlitSurface(surfaceTemp, NULL, surfaceSummary, NULL);
        SDL_FreeSurface(surfaceTemp);
    }

    SDL_BlitSurface(surfaceSummary, &rectSummaryFrame, screen, &rectSummaryPos);
}

void refreshSummary()
{
    if (surfaceSummary != NULL)
        SDL_FreeSurface(surfaceSummary);
    surfaceSummary = NULL;
    summaryScrollY = 0;
}

bool summaryScrollBy(int offset)
{
    bool changed = false;
    int y_max = surfaceSummary->h - 323;

    if ((offset > 0 && summaryScrollY < y_max) ||
        (offset < 0 && summaryScrollY > 0)) {
        summaryScrollY += offset;
        changed = true;
    }

    if (summaryScrollY > y_max)
        summaryScrollY = y_max;
    else if (summaryScrollY < 0)
        summaryScrollY = 0;

    return changed;
}

#endif // PACMAN_SUMMARY_H__