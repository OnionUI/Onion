#ifndef TWEAKS_TOOLS_H__
#define TWEAKS_TOOLS_H__

#include <stdbool.h>

#include "utils/str.h"

#define NUM_TOOLS 5

typedef struct SDL_Surface SDL_Surface;

static tool_success = false;

static char tools_short_names[NUM_TOOLS][STR_MAX] = {
    "favsort-az",
    "favsort-sys",
    "favfix",
    "recents",
    "dot_clean"
};

void tool_favoritesSortAlpha(SDL_Surface *video, SDL_Surface *screen, void *pt);
void tool_favoritesSortSystem(SDL_Surface *video, SDL_Surface *screen, void *pt);
void tool_favoritesFix(SDL_Surface *video, SDL_Surface *screen, void *pt);
void tool_recentsRemoveApps(SDL_Surface *video, SDL_Surface *screen, void *pt);
void tool_removeMacFiles(SDL_Surface *video, SDL_Surface *screen, void *pt);

void blackScreenWithText(SDL_Surface *video, SDL_Surface *screen, const char *title_str, const char *message_str);

static void (*tools_pt[NUM_TOOLS])(void*) = {
    tool_favoritesSortAlpha,
    tool_favoritesSortSystem,
    tool_favoritesFix,
    tool_recentsRemoveApps,
    tool_removeMacFiles
};

#endif // TWEAKS_TOOLS_H__
