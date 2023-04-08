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

int get_play_time(const char* name);

#endif
