#ifndef CACHE_DB_H
#define CACHE_DB_H

#include <libgen.h>
#include <sqlite3/sqlite3.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

typedef struct CacheDBItem CacheDBItem;

struct CacheDBItem {
    int id;
    char *rom_type;
    char *disp;
    char *path;
    char *imgpath;
    int type;
    char *ppath;
};

sqlite3 *cache_db = NULL;

void cache_db_close(void)
{
    sqlite3_close(cache_db);
    cache_db = NULL;
}

void cache_db_open(char *cache_db_file_path)
{
    struct stat buffer;
    if (stat(cache_db_file_path, &buffer) == 0) {
        int rc = sqlite3_open(cache_db_file_path, &cache_db);
        if (rc != SQLITE_OK) {
            cache_db_close();
        }
    }
}

sqlite3_stmt *cache_db_prepare(char *cache_db_file_path, char *sql)
{
    printf_debug("cache_db_prepare(%s, %s)\n", cache_db_file_path, sql);
    sqlite3_stmt *stmt = NULL;
    cache_db_open(cache_db_file_path);
    if (cache_db != NULL) {
        int rc = sqlite3_prepare_v2(cache_db, sql, -1, &stmt, NULL);
        if (rc != SQLITE_OK) {
            printf_debug("%s: %s\n", sqlite3_errmsg(cache_db), sqlite3_sql(stmt));
        }
        cache_db_close();
    }
    return (stmt);
}

CacheDBItem *cache_db_find(char *cache_db_item_file_path)
{
    printf_debug("cache_db_find(%s)\n", cache_db_item_file_path);
    CacheDBItem *cache_db_item = NULL;
    char *type = basename(dirname(strdup((char *)cache_db_item_file_path)));
    char *cache_db_file_path = malloc(strlen(type) * 2 + 29);
    snprintf(cache_db_file_path, strlen(type) * 2 + 29, "/mnt/SDCARD/Roms/%s/%s_cache2.db", type, type);
    char *sql = sqlite3_mprintf("SELECT * FROM %q_roms WHERE path LIKE '%%%q%%' OR disp = '%q' LIMIT 1;", type, cache_db_item_file_path, cache_db_item_file_path);
    sqlite3_stmt *stmt = cache_db_prepare(cache_db_file_path, sql);
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        printf_debug("cache item found: %s\n", strdup((const char *)sqlite3_column_text(stmt, 1)));
        cache_db_item = (CacheDBItem *)malloc(sizeof(CacheDBItem));
        cache_db_item->id = sqlite3_column_int(stmt, 0);
        cache_db_item->rom_type = type;
        cache_db_item->disp = strdup((const char *)sqlite3_column_text(stmt, 1));
        cache_db_item->path = strdup((const char *)sqlite3_column_text(stmt, 2));
        cache_db_item->imgpath = strdup((const char *)sqlite3_column_text(stmt, 3));
        cache_db_item->type = sqlite3_column_int(stmt, 4);
        cache_db_item->ppath = strdup((const char *)sqlite3_column_text(stmt, 5));
    }
    sqlite3_finalize(stmt);
    return cache_db_item;
}
#endif
