
#ifndef SQLMON_H
#define SQLMON_H

#include "../playActivity/cacheDB.h"
#include "../playActivity/playActivityDBCommon.h"
#include <arpa/inet.h>
#include <dirent.h>
#include <libgen.h>
#include <limits.h>
#include <netinet/in.h>
#include <pthread.h>
#include <signal.h>
#include <sqlite3/sqlite3.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/un.h>

#include "utils/log.h"
#include "utils/msleep.h"

sqlite3_stmt *total_play_time;
sqlite3_stmt *find_all;
sqlite3_stmt *insert_rom;
sqlite3_stmt *rom_id;
sqlite3_stmt *update_rom;
sqlite3_stmt *orphan_rom;
sqlite3_stmt *rom_by_path;
sqlite3_stmt *rom_playtime;
sqlite3_stmt *active_closed_activity;
sqlite3_stmt *activity_start;
sqlite3_stmt *activity_stop;
sqlite3_stmt *activity_stop_all;
sqlite3_stmt *delete_null_entry;
sqlite3_stmt *get_file_path;
sqlite3_stmt *update_file_path;
sqlite3_stmt *update_file_path_cache;

static sqlite3 *play_activity_db_reader = NULL;
static sqlite3 *play_activity_db_writer = NULL;

static int quit = 0;

static void sigHandler(int sig)
{
    switch (sig) {
    case SIGINT:
    case SIGTERM:
        quit = 1;
        break;
    default:
        break;
    }
}
#endif
