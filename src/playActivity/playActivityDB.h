#ifndef PLAY_ACTIVITY_DB_H
#define PLAY_ACTIVITY_DB_H

#include <dirent.h>
#include <libgen.h>
#include <sqlite3/sqlite3.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#include "./cacheDB.h"

#define PLAY_ACTIVITY_DB_NEW_FILE "/mnt/SDCARD/Saves/CurrentProfile/play_activity/play_activity_db.sqlite"
#define ROMS_FOLDER "/mnt/SDCARD/Roms"

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
    bool play_activity_db_created = exists(PLAY_ACTIVITY_DB_NEW_FILE);
    mkdir("/mnt/SDCARD/Saves/CurrentProfile/play_activity/", 0777);
    int rc = sqlite3_open(PLAY_ACTIVITY_DB_NEW_FILE, &play_activity_db);
    if (rc != SQLITE_OK) {
        printf("%s\n", sqlite3_errmsg(play_activity_db));
        play_activity_db_close();
        return;
    }
    if (!play_activity_db_created) {
        sqlite3_exec(
            play_activity_db,
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

bool play_activity_db_execute_exists(char *sql)
{
    printf_debug("play_activity_db_execute_exists(%s)\n", sql);
    play_activity_db_open();

    sqlite3_stmt *stmt;
    bool result = false;

    if (sqlite3_prepare_v2(play_activity_db, sql, -1, &stmt, NULL) == SQLITE_OK) {
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            result = true;
        }
        sqlite3_finalize(stmt);
    }

    play_activity_db_close();
    return result;
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

ROM *rom_find_by_file_path(char *rom_file_path)
{
    // Game existence in the DB check
    printf("rom_find_by_file_path(%s)\n", rom_file_path);
    char *sql = sqlite3_mprintf("SELECT * FROM rom WHERE file_path LIKE '%%%q%%' LIMIT 1;", rom_file_path);
    sqlite3_stmt *stmt = play_activity_db_prepare(sql);
    sqlite3_free(sql);
    if (sqlite3_step(stmt) != SQLITE_ROW) {

        // Game not found
        // We try to add it in the DB
        // Using the cache infos
        printf("ROM not found in the DB\n");
        sqlite3_finalize(stmt);
        stmt = NULL;

        CacheDBItem *cache_db_item = cache_db_find(rom_file_path);

        if (cache_db_item == NULL) {
            printf("ROM not found in the database. File path: %s\n", rom_file_path);
            return NULL;
        }

        sql = sqlite3_mprintf("INSERT INTO rom(type, name, file_path, image_path) "
                              "VALUES('%q', '%q', '%q', '%q');",
                              cache_db_item->rom_type, cache_db_item->disp,
                              cache_db_item->path, cache_db_item->imgpath);

        free(cache_db_item);

        play_activity_db_execute(sql);
        sqlite3_free(sql);
        sql = sqlite3_mprintf("SELECT * FROM rom WHERE file_path LIKE '%%%q%%' LIMIT 1;", rom_file_path);
        stmt = play_activity_db_prepare(sql);
        sqlite3_step(stmt);
        sqlite3_free(sql);
    }
    else {
        printf("ROM already exists in the DB\n");
    }

    ROM *rom = (ROM *)malloc(sizeof(ROM));
    rom->id = sqlite3_column_int(stmt, 0);
    rom->type = strdup((const char *)sqlite3_column_text(stmt, 1));
    rom->name = strdup((const char *)sqlite3_column_text(stmt, 2));
    rom->file_path = strdup((const char *)sqlite3_column_text(stmt, 3));
    rom->image_path = strdup((const char *)sqlite3_column_text(stmt, 4));

    sqlite3_finalize(stmt);

    return rom;
}

void play_activity_start(char *rom_file_path)
{
    ROM *rom = rom_find_by_file_path(rom_file_path);
    if (rom == NULL) {
        exit(0);
    }
    printf("play_activity_stop(%s)\n", rom->name);
    int rom_id = rom->id;
    play_activity_db_execute(sqlite3_mprintf("INSERT INTO play_activity(rom_id) VALUES(%d);", rom_id));
    free(rom);
}

void play_activity_stop(char *rom_file_path)
{

    ROM *rom = rom_find_by_file_path(rom_file_path);
    if (rom == NULL) {
        exit(0);
    }
    printf("play_activity_stop(%s)\n", rom->name);
    int rom_id = rom->id;
    play_activity_db_execute(sqlite3_mprintf("UPDATE play_activity SET play_time = (strftime('%%s', 'now')) - created_at, updated_at = (strftime('%%s', 'now')) WHERE rom_id = %d AND play_time IS NULL;", rom_id));
    free(rom);
}

#endif // PLAY_ACTIVITY_DB_H
