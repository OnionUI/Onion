
#ifndef PLAY_ACTIVITY_COMMON_H
#define PLAY_ACTIVITY_COMMON_H

#define PLAY_ACTIVITY_DB_NEW_FILE "/mnt/SDCARD/Saves/CurrentProfile/play_activity/play_activity_db.sqlite"
#define ROMS_FOLDER "/mnt/SDCARD/Roms"
#define CMD_TO_RUN "/mnt/SDCARD/.tmp_update/cmd_to_run.sh"
#define ROM_NOT_FOUND -1

#define BUF_SIZE 8192
#define PORT 25565

typedef struct ROM ROM;
typedef struct PlayActivity PlayActivity;
typedef struct PlayActivityUI PlayActivityUI;
typedef struct PlayActivities PlayActivities;
typedef struct PlayActivitiesUI PlayActivitiesUI;
#include "utils/file.h"
#include "utils/str.h"
#include <string.h>

#define GET_PLAY_TIME "GET_PLAY_TIME"
#define GET_PLAY_TIME_TOTAL "GET_PLAY_TIME_TOTAL"
#define START_PLAY_ACTIVITY "START_PLAY_ACTIVITY"
#define RESUME_PLAY_ACTIVITY "RESUME_PLAY_ACTIVITY"
#define STOP_PLAY_ACTIVITY "STOP_PLAY_ACTIVITY"
#define STOP_ALL_PLAY_ACTIVITY "STOP_ALL_PLAY_ACTIVITY"
#define CLOSE_DB "CLOSE_DB"
int asprintf(char **strp, const char *fmt, ...);

struct ROM {
    int id;
    char *type;
    char *name;
    char *file_path;
    char *image_path;
};
struct PlayActivity {
    ROM *rom;
    int play_count;
    int play_time_total;
    int play_time_average;
    char *first_played_at;
    char *last_played_at;
};
struct PlayActivities {
    PlayActivity **play_activity;
    int count;
    int play_time_total;
};

struct PlayActivityUI {
    char play_time_total[25];
    char play_time_average[25];
    char play_count[25];
    char rom_name[STR_MAX];
    char image_path[STR_MAX];
};
struct PlayActivitiesUI {
    PlayActivityUI **play_activity;
    int count;
    int play_time_total;
};

void __ensure_rel_path(char *rel_path, const char *rom_path)
{
    if (!file_path_relative_to(rel_path, ROMS_FOLDER, rom_path)) {
        if (strstr(rom_path, "../../Roms/") != NULL) {
            strcpy(rel_path, str_split(strdup((const char *)rom_path), "../../Roms/"));
        }
        else {
            strcpy(rel_path, str_replace(strdup((const char *)rom_path), "/mnt/SDCARD/Roms/", ""));
        }
    }
}
void free_play_activities_ui(PlayActivitiesUI *pa_ptr)
{
    for (int i = 0; i < pa_ptr->count; i++)
        free(pa_ptr->play_activity[i]);

    free(pa_ptr->play_activity);
    free(pa_ptr);
}

#endif
