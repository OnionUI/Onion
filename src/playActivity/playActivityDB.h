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

#define PLAY_ACTIVITY_DB_PATH "/mnt/SDCARD/Saves/CurrentProfile/saves/playActivity.db"
#define PLAY_ACTIVITY_DB_TMP_PATH "/mnt/SDCARD/Saves/CurrentProfile/saves/playActivity_tmp.db"
#define MAXVALUES 1000

typedef struct structRom { // for reading from old DB
    char name[100];
    int playTime;
} rom_list_s;

static rom_list_s rom_list[MAXVALUES];
static int rom_list_len = 0;
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

void play_activity_db_close(void)
{
    sqlite3_close(play_activity_db);
    play_activity_db = NULL;
}

void play_activity_db_open(void)
{
    bool play_activity_db_created = exists("/mnt/SDCARD/Saves/CurrentProfile/play_activity/play_activity_db.sqlite");
    mkdir("/mnt/SDCARD/Saves/CurrentProfile/play_activity/", 0777);
    int rc = sqlite3_open("/mnt/SDCARD/Saves/CurrentProfile/play_activity/play_activity_db.sqlite", &play_activity_db);
    if (rc != SQLITE_OK) {
        printf("%s\n", sqlite3_errmsg(play_activity_db));
        play_activity_db_close();
        return;
    }
    if (!play_activity_db_created) {
        sqlite3_exec(
            play_activity_db,
            "DROP TABLE IF EXISTS rom;CREATE TABLE rom(id INTEGER PRIMARY KEY, "
            "type TEXT, name TEXT, file_path TEXT, image_path TEXT, created_at "
            "INTEGER DEFAULT (strftime('%s', 'now')), updated_at "
            "INTEGER);CREATE UNIQUE INDEX rom_id_index ON rom(id);",
            NULL, NULL, NULL);
        sqlite3_exec(play_activity_db,
                     "DROP TABLE IF EXISTS play_activity;CREATE TABLE "
                     "play_activity(rom_id INTEGER, play_time INTEGER, "
                     "created_at INTEGER DEFAULT (strftime('%s', 'now')), "
                     "updated_at INTEGER);CREATE INDEX "
                     "play_activity_rom_id_index ON play_activity(rom_id);",
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

int play_activity_db_execute_select(char *sql)
{
    printf_debug("play_activity_db_execute_select(%s)\n", sql);
    play_activity_db_open();

    sqlite3_stmt *stmt;
    int result = 0;

    if (sqlite3_prepare_v2(play_activity_db, sql, -1, &stmt, NULL) == SQLITE_OK) {
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            result = sqlite3_column_int(stmt, 0);
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
    if (sqlite3_prepare_v2(play_activity_db, sql, -1, &stmt, NULL) !=
        SQLITE_OK) {
        printf("%s: %s\n", sqlite3_errmsg(play_activity_db),
               sqlite3_sql(stmt));
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
        "SUM(play_activity.play_time)/COUNT(play_activity.ROWID) AS "
        "play_time_average, datetime(MIN(play_activity.created_at), "
        "'unixepoch') AS first_played_at, "
        "datetime(MAX(play_activity.created_at), 'unixepoch') AS "
        "last_played_at FROM rom LEFT JOIN play_activity ON rom.id = "
        "play_activity.rom_id GROUP BY rom.id ORDER BY play_time_total DESC;";
    sqlite3_stmt *stmt = play_activity_db_prepare(sql);
    int play_activity_count = 0;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        play_activity_count++;
    }
    sqlite3_reset(stmt);
    play_activities = (PlayActivities *)malloc(sizeof(PlayActivities));
    play_activities->count = play_activity_count;
    play_activities->play_time_total = 0;
    play_activities->play_activity = (PlayActivity **)malloc(
        sizeof(PlayActivity *) * play_activities->count);
    for (int i = 0; i < play_activities->count; i++) {
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            play_activities->play_time_total += sqlite3_column_int(stmt, 6);
            play_activities->play_activity[i] =
                (PlayActivity *)malloc(sizeof(PlayActivity));
            play_activities->play_activity[i]->rom = (ROM *)malloc(sizeof(ROM));
            play_activities->play_activity[i]->rom->id =
                sqlite3_column_int(stmt, 0);
            play_activities->play_activity[i]->rom->type =
                strdup((const char *)sqlite3_column_text(stmt, 1));
            play_activities->play_activity[i]->rom->name =
                strdup((const char *)sqlite3_column_text(stmt, 2));
            play_activities->play_activity[i]->rom->file_path =
                strdup((const char *)sqlite3_column_text(stmt, 3));
            play_activities->play_activity[i]->rom->image_path =
                strdup((const char *)sqlite3_column_text(stmt, 4));
            play_activities->play_activity[i]->play_count =
                sqlite3_column_int(stmt, 5);
            play_activities->play_activity[i]->play_time_total =
                sqlite3_column_int(stmt, 6);
            play_activities->play_activity[i]->play_time_average =
                sqlite3_column_int(stmt, 7);
            play_activities->play_activity[i]->first_played_at =
                strdup((const char *)sqlite3_column_text(stmt, 8));
            play_activities->play_activity[i]->last_played_at =
                strdup((const char *)sqlite3_column_text(stmt, 9));
        }
    }
    sqlite3_finalize(stmt);
    return play_activities;
}

ROM *rom_find_by_file_path(char *rom_file_path)
{
    // Game existence in the DB check
    printf("rom_find_by_file_path(%s)\n", rom_file_path);
    char *sql = sqlite3_mprintf(
        "SELECT * FROM rom WHERE file_path LIKE '%%%q%%' LIMIT 1;",
        rom_file_path);
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
        play_activity_db_execute(sql);
        sqlite3_free(sql);
        sql = sqlite3_mprintf(
            "SELECT * FROM rom WHERE file_path LIKE '%%%q%%' LIMIT 1;",
            rom_file_path);
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

ROM *add_orphan_rom(char *rom_file_name)
{     
    // The Rom is an orphan, 
    // It has not been found in the SD 
    // or
    // The cache is not present for this console
    // or
    // The game is not present in the cache
    
    // We are missing 4 information that will be inserted like this :
    // 1) The Type ("GB", "GBA", etc) is now ""
    // 2) The viual rom name is now the file name 
    // 3) The path is ""
    // 4) The image path is ""
        // Game existence in the DB check
    printf("add_orphan_rom(%s)\n", rom_file_name);
    char *sql = sqlite3_mprintf(
        "SELECT * FROM rom WHERE name LIKE '%%%q%%' LIMIT 1;",
        rom_file_name);
    sqlite3_stmt *stmt = play_activity_db_prepare(sql);
    sqlite3_free(sql);
    if (sqlite3_step(stmt) != SQLITE_ROW) {
        // Game not found 
        printf("Adding orphan ROM for %s\n", rom_file_name);
        
        char *sql = sqlite3_mprintf("INSERT INTO rom(type, name, file_path, image_path) "
                              "VALUES('%q', '%q', '%q', '%q');",
                              "", rom_file_name, "", "");
        play_activity_db_execute(sql);
        sqlite3_free(sql);
        // Retrieve ROM id by its name 
        sql = sqlite3_mprintf(
            "SELECT * FROM rom WHERE name LIKE '%%%q%%' LIMIT 1;",
            rom_file_name);
            
        stmt = play_activity_db_prepare(sql);
        sqlite3_step(stmt);
        sqlite3_free(sql);
    }
    else {
         printf("Orphan game already exists\n");
    }    
       

    ROM *rom = (ROM *)malloc(sizeof(ROM));
    rom->id = sqlite3_column_int(stmt, 0);
    rom->name = rom_file_name;
/*    rom->type = strdup((const char *)sqlite3_column_text(stmt, 1));
    rom->name = strdup((const char *)sqlite3_column_text(stmt, 2));
    rom->file_path = strdup((const char *)sqlite3_column_text(stmt, 3));
    rom->image_path = strdup((const char *)sqlite3_column_text(stmt, 4));
*/
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
    play_activity_db_execute(sqlite3_mprintf(
        "INSERT INTO play_activity(rom_id) VALUES(%d);", rom_id));
}

void play_activity_stop(char *rom_file_path)
{

    ROM *rom = rom_find_by_file_path(rom_file_path);
    if (rom == NULL) {
        exit(0);
    }
    printf("play_activity_stop(%s)\n", rom->name);
    int rom_id = rom->id;
    play_activity_db_execute(
        sqlite3_mprintf("UPDATE play_activity SET play_time = (strftime('%%s', "
                        "'now')) - created_at, updated_at = (strftime('%%s', "
                        "'now')) WHERE rom_id = %d AND play_time IS NULL;",
                        rom_id));
}

int readRomOldDB()
{
    FILE *fp;

    // Check to avoid corruption
    if (exists(PLAY_ACTIVITY_DB_PATH)) {
        if ((fp = fopen(PLAY_ACTIVITY_DB_PATH, "rb")) != NULL) {
            fread(rom_list, sizeof(rom_list), 1, fp);
            rom_list_len = 0;

            for (int i = 0; i < MAXVALUES; i++) {
                if (strlen(rom_list[i].name) != 0)
                    rom_list_len++;
            }

            fclose(fp);
        }
        else {
            // The file exists but could not be opened
            // Something went wrong, the program is terminated
            return -1;
        }
    }

    return 1;
}

void displayRomOldDB(void)
{

    printf("--------------- Old DB entries ---------------\n");
    for (int i = 0; i < rom_list_len; i++) {
        printf("rom_list name: %s\n", rom_list[i].name);

        char cPlayTime[15];
        sprintf(cPlayTime, "%d", rom_list[i].playTime);
        printf("playtime: %s\n", cPlayTime);
    }
}

void play_activity_db_V3_upgrade(void)
{
    printf_debug("%s\n", "play_activity_db_V3_upgrade()");
    remove("/tmp/initTimer");

    if (readRomOldDB() == -1) {
        return;
    }

    //displayRomOldDB();
    printf("\n------- Migrating data to new database -------\n");

    int totalOldRecords = 0;
    int totalImported = 0;
    int totalAlreadyImported = 0;
    int totalSkipped = 0;
    
    printf("\n------- rom number : %d -------\n", rom_list_len);
    for (int i = 0; i < rom_list_len; i++) {
        
        totalOldRecords++;
        printf("rom name : %s\n", rom_list[i].name);
        
        // ************************ //
        // Rom file + cache search  //
        // ************************ //        
        
        if (strlen(rom_list[i].name) > 0) {
            CacheDBItem *cache_db_item = NULL;
            DIR *dir;
            struct dirent *entry;
            
            if (is_dir("/mnt/SDCARD/Roms")==1){
                
                // Scanning for accross all console roms for this specific game
                // If the rom is found, the miyoo cache db is retrieved to retrieve the displayed name + img path
                dir = opendir("/mnt/SDCARD/Roms");
                
                while ((cache_db_item == NULL) && ((entry = readdir(dir)) != NULL)) {
                    if (entry->d_type == DT_DIR && strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {
                        char file_path[strlen(entry->d_name) + strlen(rom_list[i].name) + 7];
                        snprintf(file_path, strlen(entry->d_name) + strlen(rom_list[i].name) + 8, "/Roms/%s/%s", entry->d_name, rom_list[i].name);
             
                        cache_db_item = cache_db_find(file_path);
                    }
                }
                closedir(dir);
            }    
  
            // ************** //
            // Rom migration  //
            // ************** //
            ROM *rom = NULL;
            if (cache_db_item != NULL){
                // cache found for the game console
               rom = rom_find_by_file_path(cache_db_item->path);
            }
            else {
             printf("Cache empty\n");
            }    
                       
            if (rom == NULL){
                // Orphan rom
                rom = add_orphan_rom(rom_list[i].name);
            }
            
            
            if (rom == NULL) {
                // Error adding the orphan rom idn the db
                totalSkipped++;
                continue;
            }  

                     
            // ******************* //
            // Play time migration //
            // ******************* //
            
            // The Rom is found or has been successfully inserted in the db 
            // Search for a previous play time migration (Same rom_id + created_at = 0)
            char *selectSql = sqlite3_mprintf("SELECT COUNT(*) FROM play_activity WHERE rom_id = %d AND created_at = 0;", rom->id);
            int result = play_activity_db_execute_select(selectSql);
            printf("SQL query result : %d\n", result);
            sqlite3_free(selectSql);

            if (result > 0) {
                printf("%s already imported!\n", rom_list[i].name);
                totalAlreadyImported++;
                continue;
            }
            
            printf("Importing %s (time: %d)\n", rom_list[i].name, rom_list[i].playTime);
            char *sql = sqlite3_mprintf("INSERT INTO play_activity(rom_id, play_time, created_at, updated_at) VALUES "
                                        "('%d','%d',0,0);", // Imported times have the particularity of having a "created_at" at 0.
                                        rom->id, rom_list[i].playTime);
            printf("SQL query: %s\n", sql);
            if (play_activity_db_execute(sql) == 0) {
                totalImported++;
            }               
        
        }
 
    }
    printf("\n**************************\n");
    printf("Summary:\n========\n");
    printf("Total of old records : %d\n", totalOldRecords);
    printf("Total successfuly imported: %d\n", totalImported);
    printf("Total already imported: %d\n", totalAlreadyImported);
    printf("Total skipped: %d\n", totalSkipped);
    printf("**************************\n");
    
   // rename(PLAY_ACTIVITY_DB_PATH, PLAY_ACTIVITY_DB_TMP_PATH);
    
}

#endif // PLAY_ACTIVITY_DB_H
