#ifndef TWEAKS_TOOLS_H__
#define TWEAKS_TOOLS_H__

#include <SDL/SDL.h>
#include <SDL/SDL_ttf.h>
#include <pthread.h>
#include <stdio.h>
#include <sys/wait.h>

#include "components/list.h"
#include "system/settings.h"
#include "theme/theme.h"
#include "utils/msleep.h"

#include "./appstate.h"
#include "./tools_defs.h"

static pthread_t thread_pt;
static bool thread_active = false;
static bool thread_success = false;
static SDL_Surface *_tool_bg_cache = NULL;

static void *_runCommandThread(void *cmd)
{
    int ret = system((char *)cmd);
    thread_success = WEXITSTATUS(ret) == 0;
    thread_active = false;
    return 0;
}

void _toolDialog(const char *title_str, const char *message_str,
                 bool show_progress)
{
    if (video == NULL) {
        printf("%s: %s\n", title_str, message_str);
        return;
    }

    if (_tool_bg_cache == NULL) {
        _tool_bg_cache =
            SDL_CreateRGBSurface(SDL_HWSURFACE, 640, 480, 32, 0, 0, 0, 0);
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

    if (video != NULL)
        msleep(300);
    _toolDialog(full_title, thread_success ? "Done" : "Tool failed", false);

    if (video != NULL)
        msleep(1200);

    SDL_FreeSurface(_tool_bg_cache);
    _tool_bg_cache = NULL;

    keys_enabled = true;
    all_changed = true;
}

void _displayM3uTotal()
{
    FILE *file = fopen("/tmp/count_m3u", "r");
    if (file == NULL) {
        perror("Error opening file");
        return;
    }
    int value;
    if (fscanf(file, "%d", &value) != 1) {
        perror("Error reading from file");
        fclose(file);
        return;
    }
    fclose(file);

    if (remove("/tmp/count_m3u") != 0) {
        perror("Error deleting file");
    }

    char message[28];
    snprintf(message, sizeof(message), "%d playlist files created.", value);

    _toolDialog("M3u Generator", message, false);
    if (video != NULL)
        msleep(1200);
}

void tool_generateCueFiles(void *pt)
{
    _runCommandPopup(tools_short_names[0], "/mnt/SDCARD/.tmp_update/script/cue_gen.sh");
}

void tool_generateM3uFiles_sd(void *pt)
{
    _runCommandPopup(tools_short_names[1], "/mnt/SDCARD/.tmp_update/script/m3u_gen.sh -sd");
    _displayM3uTotal();
}

void tool_generateM3uFiles_md(void *pt)
{
    _runCommandPopup(tools_short_names[2], "/mnt/SDCARD/.tmp_update/script/m3u_gen.sh -md");
    _displayM3uTotal();
}

void tool_buildShortRomGameList(void *pt)
{
    _runCommandPopup(tools_short_names[3], "./bin/gameNameList /mnt/SDCARD /mnt/SDCARD/BIOS/arcade_lists");
}

void tool_generateMiyoogamelists(void *pt)
{
    _runCommandPopup(tools_short_names[4], "/mnt/SDCARD/.tmp_update/script/miyoogamelist_gen.sh");
}

void tool_sortAppsAZ(void *pt)
{
    _runCommandPopup(tools_short_names[5], "/mnt/SDCARD/.tmp_update/script/app_sorter.sh");
}

void tool_sortAppsZA(void *pt)
{
    _runCommandPopup(tools_short_names[6], "/mnt/SDCARD/.tmp_update/script/app_sorter.sh desc");
}
void tool_restorePlayActivityHiddenEntries(void *pt)
{
    _runCommandPopup(tools_short_names[7], "/mnt/SDCARD/.tmp_update/script/restore_deleted_play_activity.sh");
}

void tool_screenRecorder(void *pt)
{
    ListItem *item = (ListItem *)pt;
    char cmd[STR_MAX];
    char newestFile[STR_MAX / 2];
    snprintf(cmd, sizeof(cmd), "/mnt/SDCARD/.tmp_update/script/screen_recorder.sh toggle &");
    int fileCheck;
    fileCheck = exists("/tmp/recorder_active");

    if (!fileCheck) {
        list_updateStickyNote(item, "Status: Now recording...");
        system(cmd);
    }
    else {
        if (file_findNewest(RECORDED_DIR, newestFile, sizeof(newestFile))) {
            char note[STR_MAX];
            system(cmd);
            snprintf(note, sizeof(note), "Stopped, saved as: %s", newestFile);
            list_updateStickyNote(item, note);
        }
        else {
            list_updateStickyNote(item, "Status: Recording ended, no new file found.");
            snprintf(cmd, sizeof(cmd), "/mnt/SDCARD/.tmp_update/script/screen_recorder.sh hardkill &");
        }
    }
    list_changed = true;
}

static void (*tools_pt[NUM_TOOLS])(void *) = {
    tool_generateCueFiles,
    tool_generateM3uFiles_sd,
    tool_generateM3uFiles_md,
    tool_buildShortRomGameList,
    tool_generateMiyoogamelists,
    tool_sortAppsAZ,
    tool_sortAppsZA};

#endif // TWEAKS_TOOLS_H__
