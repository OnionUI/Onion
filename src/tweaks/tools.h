#ifndef TWEAKS_TOOLS_H__
#define TWEAKS_TOOLS_H__

#include <stdio.h>
#include <sys/wait.h>
#include <pthread.h>
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
static pthread_t thread_pt;
static bool thread_active = false;
static bool thread_success = false;
static SDL_Surface *_tool_bg_cache = NULL;

static void* _runCommandThread(void *cmd)
{
    int ret = system((char*)cmd);
    thread_success = WEXITSTATUS(ret) == 0;
    thread_active = false;
    return 0;
}

void _toolDialog(const char *title_str, const char *message_str, bool show_progress)
{
    if (video == NULL) {
        printf("%s: %s\n", title_str, message_str);
        return;
    }

    if (_tool_bg_cache == NULL) {
        _tool_bg_cache = SDL_CreateRGBSurface(SDL_HWSURFACE, 640, 480, 32, 0, 0, 0, 0);
        SDL_BlitSurface(screen, NULL, _tool_bg_cache, NULL);
    }

    SDL_BlitSurface(_tool_bg_cache, NULL, screen, NULL);

    if (show_progress)
        theme_renderDialogProgress(screen, title_str, message_str, false);
    else
        theme_renderDialog(screen, title_str, message_str, false);

    SDL_BlitSurface(screen, NULL, video, NULL);
    SDL_Flip(video);
}

void _runCommandPopup(const char *tool_name, const char *_cmd)
{
    static char msg_apply[] = "Applying tool...\n \n \n ";

    keys_enabled = false;
    char full_title[STR_MAX];
    sprintf(full_title, "Tool: %s", tool_name);
    _toolDialog(full_title, msg_apply, false);

    char cmd[STR_MAX];
    strncpy(cmd, _cmd, STR_MAX - 1);
    thread_active = true;
    pthread_create(&thread_pt, NULL, _runCommandThread, cmd);

    theme_clearDialogProgress();

    while (thread_active) {
        if (video == NULL) {
            msleep(15);
            continue;
        }
        msleep(300);
        if (!thread_active)
            break;
        _toolDialog(full_title, msg_apply, true);
    }

    if (video != NULL) msleep(300);
    _toolDialog(full_title, thread_success ? "Done" : "Tool failed", false);
    
    if (video != NULL) msleep(300);
    
    SDL_FreeSurface(_tool_bg_cache);
    _tool_bg_cache = NULL;

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
