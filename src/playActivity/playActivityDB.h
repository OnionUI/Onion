#ifndef PLAY_ACTIVITY_DB_H
#define PLAY_ACTIVITY_DB_H

#include <dirent.h>
#include <libgen.h>
#include <limits.h>
#include <sqlite3/sqlite3.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#include "utils/file.h"
#include "utils/log.h"

#include "./cacheDB.h"

#define PLAY_ACTIVITY_DB_NEW_FILE "/mnt/SDCARD/Saves/CurrentProfile/play_activity/play_activity_db.sqlite"
#define ROMS_FOLDER "/mnt/SDCARD/Roms"
#define CMD_TO_RUN "/mnt/SDCARD/.tmp_update/cmd_to_run.sh"

typedef struct ROM ROM;
typedef struct PlayActivity PlayActivity;
typedef struct PlayActivities PlayActivities;

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

sqlite3 *play_activity_db = NULL;

void play_activity_db_close()
{
    sqlite3_close(play_activity_db);
    play_activity_db = NULL;
}

void play_activity_db_open(void)
{
    if (play_activity_db != NULL)
        return;

    bool play_activity_db_created = is_file(PLAY_ACTIVITY_DB_NEW_FILE);

    mkdir("/mnt/SDCARD/Saves/CurrentProfile/play_activity/", 0777);

    if (sqlite3_open(PLAY_ACTIVITY_DB_NEW_FILE, &play_activity_db) != SQLITE_OK) {
        printf("%s\n", sqlite3_errmsg(play_activity_db));
        play_activity_db_close();
        return;
    }

    if (!play_activity_db_created) {
        sqlite3_exec(play_activity_db,
                     "DROP TABLE IF EXISTS rom;"
                     "CREATE TABLE rom(id INTEGER PRIMARY KEY, type TEXT, name TEXT, file_path TEXT, image_path TEXT, created_at INTEGER DEFAULT (strftime('%s', 'now')), updated_at INTEGER);"
                     "CREATE UNIQUE INDEX rom_id_index ON rom(id);",
                     NULL, NULL, NULL);
        sqlite3_exec(play_activity_db,
                     "DROP TABLE IF EXISTS play_activity;"
                     "CREATE TABLE play_activity(rom_id INTEGER, play_time INTEGER, created_at INTEGER DEFAULT (strftime('%s', 'now')), updated_at INTEGER);"
                     "CREATE INDEX play_activity_rom_id_index ON play_activity(rom_id);",
                     NULL, NULL, NULL);
    }
}

int play_activity_db_execute(char *sql)
{
    printf_debug("play_activity_db_execute(%s)\n", sql);
    play_activity_db_open();
    int rc = sqlite3_exec(play_activity_db, sql, NULL, NULL, NULL);
    play_activity_db_close();
    return rc;
}

sqlite3_stmt *play_activity_db_prepare(char *sql)
{
    printf_debug("play_activity_db_prepare(%s)\n", sql);
    play_activity_db_open();
    sqlite3_stmt *stmt = NULL;
    if (sqlite3_prepare_v2(play_activity_db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        printf("%s: %s\n", sqlite3_errmsg(play_activity_db), sqlite3_sql(stmt));
    }
    play_activity_db_close();
    return (stmt);
}

PlayActivities *play_activity_find_all(void)
{
    PlayActivities *play_activities = NULL;
    char *sql =
        "SELECT rom.id, rom.type, rom.name, rom.file_path, rom.image_path, "
        "COUNT(play_activity.ROWID) AS play_count_total, "
        "SUM(play_activity.play_time) AS play_time_total, "
        "SUM(play_activity.play_time)/COUNT(play_activity.ROWID) AS play_time_average, "
        "datetime(MIN(play_activity.created_at), 'unixepoch') AS first_played_at, "
        "datetime(MAX(play_activity.created_at), 'unixepoch') AS last_played_at "
        "FROM rom LEFT JOIN play_activity ON rom.id = play_activity.rom_id "
        "GROUP BY rom.id "
        "ORDER BY play_time_total DESC;";
    sqlite3_stmt *stmt = play_activity_db_prepare(sql);
    int play_activity_count = 0;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        play_activity_count++;
    }
    sqlite3_reset(stmt);

    play_activities = (PlayActivities *)malloc(sizeof(PlayActivities));
    play_activities->count = play_activity_count;
    play_activities->play_time_total = 0;
    play_activities->play_activity = (PlayActivity **)malloc(sizeof(PlayActivity *) * play_activities->count);

    for (int i = 0; i < play_activities->count; i++) {
        if (sqlite3_step(stmt) != SQLITE_ROW)
            continue;

        PlayActivity *entry = play_activities->play_activity[i] = (PlayActivity *)malloc(sizeof(PlayActivity));
        ROM *rom = play_activities->play_activity[i]->rom = (ROM *)malloc(sizeof(ROM));

        rom->id = sqlite3_column_int(stmt, 0);
        rom->type = strdup((const char *)sqlite3_column_text(stmt, 1));
        rom->name = strdup((const char *)sqlite3_column_text(stmt, 2));
        if (sqlite3_column_text(stmt, 3) != NULL) {
            rom->file_path = strdup((const char *)sqlite3_column_text(stmt, 3));
        }
        if (sqlite3_column_text(stmt, 4) != NULL) {
            rom->image_path = strdup((const char *)sqlite3_column_text(stmt, 4));
        }

        entry->play_count = sqlite3_column_int(stmt, 5);
        entry->play_time_total = sqlite3_column_int(stmt, 6);
        entry->play_time_average = sqlite3_column_int(stmt, 7);
        if (sqlite3_column_text(stmt, 8) != NULL) {
            entry->first_played_at = strdup((const char *)sqlite3_column_text(stmt, 8));
        }
        if (sqlite3_column_text(stmt, 9) != NULL) {
            entry->last_played_at = strdup((const char *)sqlite3_column_text(stmt, 9));
        }

        play_activities->play_time_total += entry->play_time_total;
    }

    sqlite3_finalize(stmt);

    return play_activities;
}

void free_play_activities(PlayActivities *pa_ptr)
{
    for (int i = 0; i < pa_ptr->count; i++) {
        free(pa_ptr->play_activity[i]->rom);
        free(pa_ptr->play_activity[i]);
    }
    free(pa_ptr->play_activity);
    free(pa_ptr);
}

void play_activity_fix_paths(void)
{
    play_activity_db_open();
    sqlite3_stmt *stmt = NULL;

    if (sqlite3_prepare_v2(play_activity_db, "SELECT id, file_path FROM rom WHERE file_path LIKE '/mnt/SDCARD/%%';", -1, &stmt, NULL) != SQLITE_OK) {
        printf("%s: %s\n", sqlite3_errmsg(play_activity_db), sqlite3_sql(stmt));
    }

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        int rom_id = sqlite3_column_int(stmt, 0);
        char file_path[PATH_MAX];
        strcpy(file_path, (const char *)sqlite3_column_text(stmt, 1));

        if (strlen(file_path) == 0) {
            continue;
        }

        char cache_path[PATH_MAX];
        char cache_name[STR_MAX];
        int cache_version = cache_get_path(cache_path, cache_name, file_path);

        char rel_path[PATH_MAX];
        if (!file_path_relative_to(rel_path, ROMS_FOLDER, file_path)) {
            strcpy(rel_path, str_split(file_path, "../../Roms/"));
        }

        char *sql;
        if (cache_version == -1) {
            sql = sqlite3_mprintf("UPDATE rom SET file_path = %Q WHERE id = %d;", rel_path, rom_id);
        }
        else {
            sql = sqlite3_mprintf("UPDATE rom SET file_path = %Q, type = %Q WHERE id = %d;", rel_path, cache_path, rom_id);
        }
        printf_debug("%s\n", sql);
        sqlite3_exec(play_activity_db, sql, NULL, NULL, NULL);
        sqlite3_free(sql);
    }

    sqlite3_finalize(stmt);
    play_activity_db_close();
}

void play_activity_insert_rom(const char *rom_type, const char *rom_name, const char *file_path, const char *image_path)
{
    char rel_path[PATH_MAX];
    if (!file_path_relative_to(rel_path, ROMS_FOLDER, file_path)) {
        printf_debug("Error in file path: %s\n", file_path);
        return;
    }
    char *sql = sqlite3_mprintf("INSERT INTO rom(type, name, file_path, image_path) VALUES(%Q, %Q, %Q, %Q);",
                                rom_type, rom_name, rel_path, image_path);
    play_activity_db_execute(sql);
    sqlite3_free(sql);
}

void play_activity_insert_rom_from_cache(CacheDBItem *cache_db_item)
{
    play_activity_insert_rom(cache_db_item->cache_path, cache_db_item->name, cache_db_item->path, cache_db_item->imgpath);
}

int _get_rom_id(const char *rom_path)
{
    int rom_id = -1;

    char *sql = sqlite3_mprintf("SELECT id FROM rom WHERE file_path=%Q LIMIT 1;", rom_path);
    sqlite3_stmt *stmt = play_activity_db_prepare(sql);

    if (sqlite3_step(stmt) == SQLITE_ROW) {
        rom_id = sqlite3_column_int(stmt, 0);
    }

    sqlite3_finalize(stmt);

    return rom_id;
}

int rom_find_by_file_path(char *rom_file_path, bool create_not_found)
{
    // Game existence in the DB check
    char rel_path[PATH_MAX];
    if (!file_path_relative_to(rel_path, ROMS_FOLDER, rom_file_path)) {
        printf_debug("rom_find_by_file_path(%s) - path not found\n", rom_file_path);
        return -1;
    }

    printf_debug("rom_find_by_file_path('%s') -> '%s'\n", rom_file_path, rel_path);

    int rom_id = _get_rom_id(rel_path);

    if (rom_id == -1 && create_not_found) {
        CacheDBItem *cache_db_item = cache_db_find(rom_file_path);

        if (cache_db_item == NULL) {
            printf_debug("ROM not found in the database. File path: %s\n", rom_file_path);
            return -1;
        }

        play_activity_insert_rom_from_cache(cache_db_item);
        free(cache_db_item);

        sqlite3_stmt *stmt = play_activity_db_prepare("SELECT id FROM rom WHERE ROWID = last_insert_rowid()");

        if (sqlite3_step(stmt) == SQLITE_ROW) {
            rom_id = sqlite3_column_int(stmt, 0);
        }

        sqlite3_finalize(stmt);
    }

    return rom_id;
}

bool _get_active_rom_path(char *rom_path_out)
{
    char *ptr;
    char cmd[STR_MAX] = "";

    FILE *fp;
    file_get(fp, CMD_TO_RUN, CONTENT_STR, cmd);

    if (strlen(cmd) == 0) {
        return false;
    }

    if ((ptr = strrchr(cmd, '"')) != NULL) {
        *ptr = '\0';
    }

    if ((ptr = strrchr(cmd, '"')) != NULL) {
        strncpy(rom_path_out, ptr + 1, STR_MAX);
        return true;
    }

    return false;
}

int _get_active_closed_activity(void)
{
    int rom_id = -1;

    char rom_path[STR_MAX];
    bool rom_active = _get_active_rom_path(rom_path);

    if (!rom_active) {
        return -1;
    }

    printf_debug("Last closed active rom: %s\n", rom_path);

    int active_rom_id = rom_find_by_file_path(rom_path, false);
    if (active_rom_id == -1) {
        return -1;
    }

    char *sql = sqlite3_mprintf("SELECT * FROM play_activity WHERE rom_id = %d AND play_time IS NULL;", rom_id);
    sqlite3_stmt *stmt = play_activity_db_prepare(sql);

    if (sqlite3_step(stmt) == SQLITE_ROW) {
        // Activity is not closed
        return -1;
    }

    sqlite3_free(sql);
    sqlite3_finalize(stmt);

    return rom_id;
}

void play_activity_start(char *rom_file_path)
{
    int rom_id = rom_find_by_file_path(rom_file_path, true);
    if (rom_id == -1) {
        exit(1);
    }
    printf_debug("play_activity_start(%s)\n", rom_file_path);
    play_activity_db_execute(sqlite3_mprintf("INSERT INTO play_activity(rom_id) VALUES(%d);", rom_id));
}

void play_activity_resume(void)
{
    int rom_id = _get_active_closed_activity();
    if (rom_id == -1) {
        printf("Error: no active rom\n");
        exit(1);
    }
    printf_debug("play_activity_resume() - rom_id: %d\n", rom_id);
    play_activity_db_execute(sqlite3_mprintf("INSERT INTO play_activity(rom_id) VALUES(%d);", rom_id));
}

void play_activity_stop(char *rom_file_path)
{
    int rom_id = rom_find_by_file_path(rom_file_path, false);
    if (rom_id == -1) {
        exit(1);
    }
    printf_debug("play_activity_stop(%s)\n", rom_file_path);
    play_activity_db_execute(sqlite3_mprintf("UPDATE play_activity SET play_time = (strftime('%%s', 'now')) - created_at, updated_at = (strftime('%%s', 'now')) WHERE rom_id = %d AND play_time IS NULL;", rom_id));
}

void play_activity_stop_all(void)
{
    print_debug("play_activity_stop_all()");
    play_activity_db_execute(
        "UPDATE play_activity SET play_time = (strftime('%s', 'now')) - created_at, updated_at = (strftime('%s', 'now')) WHERE play_time IS NULL;"
        "DELETE FROM play_activity WHERE play_time < 0;");
}

#endif // PLAY_ACTIVITY_DB_H
