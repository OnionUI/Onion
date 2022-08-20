#ifndef TWEAKS_TOOLS_H__
#define TWEAKS_TOOLS_H__

#include <stdio.h>
#include <sys/wait.h>
#include <SDL/SDL.h>
#include <SDL/SDL_ttf.h>

#include "utils/msleep.h"
#include "components/list.h"
#include "theme/theme.h"
#include "./appstate.h"

#define NUM_TOOLS 5

static char tools_short_names[NUM_TOOLS][STR_MAX] = {
    "favsort-az",
    "favsort-sys",
    "favfix",
    "recents",
    "dot_clean"
};
static tool_success = false;

bool _runCommand(const char *cmd)
{
    int ret = system(cmd);
    if (WEXITSTATUS(ret) == 0)
        return true;
    else
        return false;
}

void _blackScreenWithText(const char *text_str)
{
    if (video == NULL) {
        printf("%s\n", text_str);
        return;
    }
    
    int popup_width = 640,
        popup_height = 160;

    SDL_Surface *text = theme_textboxSurface(text_str, resource_getFont(TITLE), (SDL_Color){255, 255, 255}, ALIGN_CENTER);

    SDL_Rect popup_rect = {320 - popup_width / 2, 240 - popup_height / 2, popup_width, popup_height};
    SDL_FillRect(screen, &popup_rect, 0);

    SDL_Rect text_rect = {320 - text->w / 2, 240 - text->h / 2};
    SDL_BlitSurface(text, NULL, screen, &text_rect);
    SDL_FreeSurface(text);

    SDL_BlitSurface(screen, NULL, video, NULL);
    SDL_Flip(video);
}

void _runCommandPopup(const char *tool_name, const char *cmd)
{
    keys_enabled = false;
    char msg[STR_MAX];
    sprintf(msg, "Applying tool...\n<%s>", tool_name);
    _blackScreenWithText(msg);
    tool_success = _runCommand(cmd);
    if (video != NULL) msleep(300);
    _blackScreenWithText(tool_success ? "Done" : "Tool failed");
    if (video != NULL) msleep(300);
    keys_enabled = true;
    all_changed = true;
}

void tool_favoritesSortAlpha(void *pt)
{
    _runCommandPopup(tools_short_names[0], "./bin/tools favsort");
}

void tool_favoritesSortSystem(void *pt)
{
    _runCommandPopup(tools_short_names[1], "./bin/tools favsort2");
}

void tool_favoritesFix(void *pt)
{
    _runCommandPopup(tools_short_names[2], "./bin/tools favfix");
}

void tool_recentsRemoveApps(void *pt)
{
    _runCommandPopup(tools_short_names[3], "./bin/tools recents --clean_all");
}

void tool_removeMacFiles(void *pt)
{
    _runCommandPopup(tools_short_names[4], 
        "find /mnt/SDCARD/ -depth -type f \\( -name \"._*\" -o -name \".DS_Store\" \\) -delete; "
        "find /mnt/SDCARD/ -depth -type d -name \"__MACOSX\" -exec rm -rf {} \\;"
    );
}

static void (*tools_pt[NUM_TOOLS])(void*) = {
    tool_favoritesSortAlpha,
    tool_favoritesSortSystem,
    tool_favoritesFix,
    tool_recentsRemoveApps,
    tool_removeMacFiles
};

#endif // TWEAKS_TOOLS_H__
