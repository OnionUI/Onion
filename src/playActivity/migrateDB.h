#ifndef PLAY_ACTIVITY_MIGRATE_DB
#define PLAY_ACTIVITY_MIGRATE_DB

#include "utils/file.h"
#include "utils/log.h"
#include "utils/str.h"

#include "./cacheDB.h"
#include "./legacyDB.h"
#include "./playActivityDB.h"

#define MIGRATE_DB_MAX_FILES 200

typedef struct DBHandle {
    sqlite3 *db_handle;
    char db_type[100];
} DBHandle;
static DBHandle __migrate_cache_handles[MIGRATE_DB_MAX_FILES];
static int __migrate_cache_count = 0;

void _migrate_loadCacheDBs(void)
{
    // Optimisation function to ease with the SD multiple reads
    DIR *dir;
    struct dirent *entry;

    if (is_dir(ROMS_FOLDER) == 1) {
        // Scanning across all console roms for this specific game
        // If the rom is found, the miyoo cache db is retrieved to retrieve the displayed name + img path
        dir = opendir(ROMS_FOLDER);

        while ((entry = readdir(dir)) != NULL) {

            if (entry->d_type == DT_DIR && strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {
                // New folder found
                char romFolder[100];
                sprintf(romFolder, entry->d_name);

                char cache_db_file_path[STR_MAX];
                int cache_version = cache_get_path(cache_db_file_path, romFolder);

                if (cache_version != -1) {
                    printf("Cache found : %s\n", cache_db_file_path);

                    // Open the database file
                    sqlite3 *db;
                    int rc = sqlite3_open(cache_db_file_path, &db);
                    if (rc != SQLITE_OK) {
                        fprintf(stderr, "Cannot open database '%s': %s\n", cache_db_file_path, sqlite3_errmsg(db));
                        continue; // Skip to the next file
                    }

                    __migrate_cache_handles[__migrate_cache_count].db_handle = db;

                    strcpy(__migrate_cache_handles[__migrate_cache_count].db_type, romFolder);
                    __migrate_cache_count++;

                    if (__migrate_cache_count >= MIGRATE_DB_MAX_FILES) {
                        fprintf(stderr, "Maximum number of database files reached.\n");
                        break;
                    }
                }
            }
        }
        printf("\n%d cache files found\n", __migrate_cache_count);
        closedir(dir);
    }
    else
        printf("No Rom folder detected ... \n");
}

void migrateDB(void)
{
    printf_debug("%s\n", "play_activity_db_V3_upgrade()");
    remove("/tmp/initTimer");

    if (readLegacyDB() == -1) {
        return;
    }

    CacheDBItem *cache_db_item = NULL;

    _migrate_loadCacheDBs();

    //displayRomOldDB();
    printf("\n------- Migrating data to new database -------\n");

    int totalOldRecords = 0;
    int totalImported = 0;
    int totalAlreadyImported = 0;
    int totalSkipped = 0;
    int totalOrphan = 0;

    printf("\n%d games to migrate\n", rom_list_len);
    play_activity_db_open();

    for (int i = 0; i < LEGACY_DB_MAX; i++) {
        
        if ((strlen(rom_list[i].name) == 0) || (rom_list[i].playTime) == 0) {

            continue;
        }

        // Skip dummy entry
        if (strcmp(rom_list[i].name, "Dummy") == 0 && rom_list[i].playTime < 600) {
            continue;
        }

        // ************************ //
        // Rom search               //
        // ************************ //
        // search for the game in all cache
        // If found : search for its path in the new DB
        //              Add it if needed
        // Else : Search for its raw name in the new DB as an orphan

        printf("\n%s\n", rom_list[i].name);
        totalOldRecords++;
        int rom_id = -1;
        char *sql;
        sqlite3_stmt *stmt;
        bool bFound = false;
        int nDB = 0;
        
        while ((nDB < __migrate_cache_count) && (bFound == false)){
            
            DBHandle *handle = &__migrate_cache_handles[nDB];
            nDB ++;    
                
            if (handle->db_handle != NULL) {
              
                sql = sqlite3_mprintf("SELECT * FROM %q_roms WHERE path LIKE '%%%q%%';", handle->db_type, rom_list[i].name);

                //sql = sqlite3_mprintf("SELECT * FROM %q_roms WHERE basename(path) = '%q' OR basename(path) = '%q' LIMIT 1;", handle->db_type, rom_list[i].name);
                int rc = sqlite3_prepare_v2(handle->db_handle, sql, -1, &stmt, NULL);

                if (rc != SQLITE_OK) {
                    printf("%s: %s\n", sqlite3_errmsg(handle->db_handle), sqlite3_sql(stmt));
                }

                while ((sqlite3_step(stmt) == SQLITE_ROW) && (bFound == false)) {                    
/*                 
                 char *path_rom = (char *)sqlite3_column_text(stmt, 2);
                   char *path_rom_no_ext = file_removeExtension(path_rom);
                   
                   printf("Item found %s in cache %s\n",path_rom, handle->db_type);
                   printf("No ext %s\n",path_rom_no_ext);
  */               
                   if ((strcmp(basename((char *)sqlite3_column_text(stmt, 2)),rom_list[i].name) == 0)||(strcmp(basename(file_removeExtension((char *)sqlite3_column_text(stmt, 2))),rom_list[i].name) == 0)){
                       
                        bFound = true;
                                                      
                        cache_db_item = (CacheDBItem *)malloc(sizeof(CacheDBItem));
                        cache_db_item->id = sqlite3_column_int(stmt, 0);
        
                        cache_db_item->rom_type = handle->db_type;
                        cache_db_item->disp = strdup((const char *)sqlite3_column_text(stmt, 1));
                        cache_db_item->path = strdup((const char *)sqlite3_column_text(stmt, 2));
                        cache_db_item->imgpath = strdup((const char *)sqlite3_column_text(stmt, 3));
                        cache_db_item->type = sqlite3_column_int(stmt, 4);
                        cache_db_item->ppath = strdup((const char *)sqlite3_column_text(stmt, 5));
                        sqlite3_free(sql);
                        sqlite3_finalize(stmt);
                        
                        // Check if ROM does not already exist
                        sql = sqlite3_mprintf(
                            "SELECT * FROM rom WHERE file_path = '%q' LIMIT 1;",
                            cache_db_item->path);

                        rc = sqlite3_prepare_v2(play_activity_db, sql, -1, &stmt, NULL);

                        if (rc != SQLITE_OK) {
                            printf("%s: %s\n", sqlite3_errmsg(play_activity_db), sqlite3_sql(stmt));
                        }

                        if (sqlite3_step(stmt) != SQLITE_ROW) {

                            // Rom not already inserted

                            sql = sqlite3_mprintf("INSERT INTO rom(type, name, file_path, "
                                                  "image_path) VALUES('%q', '%q', '%q', '%q');",
                                                  cache_db_item->rom_type, cache_db_item->disp,
                                                  cache_db_item->path, cache_db_item->imgpath);

                            int rc = sqlite3_exec(play_activity_db, sql, NULL, NULL, NULL);
                            sqlite3_free(sql);

                            if (rc != SQLITE_OK) {
                                printf("%s: %s\n", sqlite3_errmsg(play_activity_db), sqlite3_sql(stmt));
                            }

                            rc = sqlite3_prepare_v2(play_activity_db, "SELECT last_insert_rowid()", -1, &stmt, NULL);

                            if (rc != SQLITE_OK) {
                                printf("%s: %s\n", sqlite3_errmsg(play_activity_db), sqlite3_sql(stmt));
                            }
                            if (sqlite3_step(stmt) == SQLITE_ROW) {
                                rom_id = sqlite3_column_int(stmt, 0);
                                printf("- added - ID %d\n", rom_id);
                            }
                          
                            sqlite3_finalize(stmt);
                             
                        }
                        else {
                            rom_id = sqlite3_column_int(stmt, 0);
                            printf("- already added - ID %d\n", rom_id);
                        
                        }
                        free(cache_db_item);
                        break;                        
                   } 
               }              
            }  
        }
        if (rom_id == -1) {
            /**
            * A rom is an orphan if:
            * - it hasn't been found on the SD
            * - cache file missing for roms folder
            * - rom is not in the cache
            * 
            * For orphan some missing information will be substituted:
            * 1. Type: ("GB", "GBA", etc) = "ORPHAN"
            * 2. Virtual rom name = file name
            * 3. Path = ""
            * 4. Img path = ""
            * 
            */
            // Game existence in the DB check
            sql = sqlite3_mprintf("SELECT * FROM rom WHERE name IS '%q' LIMIT 1;", rom_list[i].name);
            int rc = sqlite3_prepare_v2(play_activity_db, sql, -1, &stmt, NULL);

            if (rc != SQLITE_OK) {
                printf("%s: %s\n", sqlite3_errmsg(play_activity_db), sqlite3_sql(stmt));
            }

            if (sqlite3_step(stmt) != SQLITE_ROW) {
                // Game not found
               
                sql = sqlite3_mprintf("INSERT INTO rom(type, name, file_path, image_path) VALUES('ORPHAN', %Q, '', '');", rom_list[i].name);
                int rc = sqlite3_exec(play_activity_db, sql, NULL, NULL, NULL);
                sqlite3_free(sql);

                if (rc != SQLITE_OK) {
                    printf("%s: %s\n", sqlite3_errmsg(play_activity_db), sqlite3_sql(stmt));
                    continue;
                }
                // Retrieve ROM id by its name

                rc = sqlite3_prepare_v2(play_activity_db, "SELECT last_insert_rowid()", -1, &stmt, NULL);

                if (rc != SQLITE_OK) {
                    printf("%s: %s\n", sqlite3_errmsg(play_activity_db), sqlite3_sql(stmt));
                    continue;
                }
                if (sqlite3_step(stmt) == SQLITE_ROW) {
                    rom_id = sqlite3_column_int(stmt, 0);
                    printf("Orphan added - ID %d\n", rom_id);
                }
            }
            else {
                printf("Orphan already exists\n");
                rom_id = sqlite3_column_int(stmt, 0);
            }
            sqlite3_finalize(stmt);
            //sqlite3_free(sql);

            if (rom_id != -1)
                totalOrphan++;
        }

        if (rom_id == -1) {
            // Error adding the orphan rom idn the db
            printf("Skipped \n");
            totalSkipped++;

            continue;
        }

        // ******************* //
        // Play time migration //
        // ******************* //

        // The Rom is found or has been successfully inserted in the db
        // Search for a previous play time migration (Same rom_id + created_at = 0)
        sql = sqlite3_mprintf("SELECT rom_id FROM play_activity WHERE rom_id = %d AND created_at = 0;", rom_id);
        int rc = sqlite3_prepare_v2(play_activity_db, sql, -1, &stmt, NULL);

        if (rc != SQLITE_OK) {
            printf("%s\n", sqlite3_errmsg(play_activity_db));
            continue;
        }
        else {
            rc = sqlite3_step(stmt);
            sqlite3_finalize(stmt);
            sqlite3_free(sql);

            if (rc == SQLITE_ROW) {
                printf("Play time already imported\n");
                totalAlreadyImported++;
                continue;
            }
            else {
                printf("Importing play time: %d\n", rom_list[i].playTime);

                sql = sqlite3_mprintf("INSERT INTO play_activity(rom_id, play_time, created_at, updated_at) VALUES "
                                      "(%d,%d,0,0);", // Imported times have the particularity of having a "created_at" at 0.
                                      rom_id, rom_list[i].playTime);

                // printf("SQL query: %s\n", sql);

                if (sqlite3_exec(play_activity_db, sql, NULL, NULL, NULL) == SQLITE_OK) {
                    totalImported++;
                }
                else {
                    printf("%s: %s\n", sqlite3_errmsg(play_activity_db), sqlite3_sql(stmt));
                }
                sqlite3_free(sql);
            }
        }
    }

    // Close the database connections

    play_activity_db_close();
    for (int i = 0; i < __migrate_cache_count; i++) {
        sqlite3_close(__migrate_cache_handles[i].db_handle);
    }

    printf("\n********************************\n");
    printf("Summary:\n========\n");
    printf("Total of old records:        %d\n", totalOldRecords);
    printf("Total imported: %d - %d orphans\n", totalImported, totalOrphan);
    printf("Total already imported:      %d\n", totalAlreadyImported);
    printf("Total skipped:               %d\n", totalSkipped);
    printf("********************************\n");

    // rename(PLAY_ACTIVITY_DB_OLD_PATH, PLAY_ACTIVITY_DB_OLD_PATH_TMP);
}

#endif // PLAY_ACTIVITY_MIGRATE_DB
