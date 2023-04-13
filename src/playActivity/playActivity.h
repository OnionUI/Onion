#ifndef PLAY_ACTIVITY_H
#define PLAY_ACTIVITY_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sqlite3/sqlite3.h>
#include <time.h>

#include "utils/log.h"
#include "utils/str.h"
#include "utils/file.h"

#define INIT_TIMER_PATH "/tmp/initTimer"
#define PLAY_ACTIVITY_SQLITE_PATH "/mnt/SDCARD/Saves/CurrentProfile/saves/play_activity.sqlite"
#define PLAY_ACTIVITY_DB_PATH "/mnt/SDCARD/Saves/CurrentProfile/saves/playActivity.db"

struct {
    char *name;
    char *file_path;
    int play_count;
    int play_time;
} PlayActivity;

PlayActivity * find_play_activities(const char *name);
#endif
