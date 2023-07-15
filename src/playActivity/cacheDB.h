#ifndef CACHE_DB_H
#define CACHE_DB_H

#include <libgen.h>
#include <sqlite3/sqlite3.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#include "utils/str.h"

typedef struct CacheDBItem {
    char cache_path[PATH_MAX];
    char name[STR_MAX];
    char path[PATH_MAX];
    char imgpath[PATH_MAX];
} CacheDBItem;

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

int cache_get_path_and_version(char *cache_db_file_path, const char *cache_dir, const char *dir_name)
{
    // Check if "_cache6.db" file exists
    snprintf(cache_db_file_path, PATH_MAX - 1, "%s/%s_cache6.db", cache_dir, dir_name);
    if (is_file(cache_db_file_path) == 1) {
        return 6;
    }

    // Check if "_cache2.db" file exists
    snprintf(cache_db_file_path, PATH_MAX - 1, "%s/%s_cache2.db", cache_dir, dir_name);
    if (is_file(cache_db_file_path) == 1) {
        return 2;
    }

    printf_debug("No cache found at: '%s'\n", cache_db_file_path);
    return -1;
}

int cache_get_path(char *cache_path_out, char *cache_name_out, const char *rom_path)
{
    cache_path_out[0] = '\0';

    int cache_version = -1;
    char *cache_dir = dirname(strdup((char *)rom_path));

    while (strlen(cache_dir) > 16 && strcmp("Roms", cache_name_out) != 0) {
        strcpy(cache_name_out, basename(cache_dir));
        cache_version = cache_get_path_and_version(cache_path_out, cache_dir, cache_name_out);

        if (cache_version != -1) {
            break;
        }

        cache_dir = dirname(cache_dir);
    }

    return cache_version;
}

CacheDBItem *cache_db_find(const char *path_or_name)
{
    printf_debug("cache_db_find('%s', '%s')\n", path_or_name);

    CacheDBItem *cache_db_item = NULL;
    char cache_db_file_path[STR_MAX];
    char cache_type[STR_MAX];

    char *sql;
    int cache_version = cache_get_path(cache_db_file_path, cache_type, path_or_name);

    if (cache_version == 2) {
        sql = sqlite3_mprintf("SELECT disp, path, imgpath FROM %q_roms WHERE path LIKE '%%%q' OR disp = %Q LIMIT 1;", cache_type, path_or_name, path_or_name);
    }
    else if (cache_version == 6) {
        sql = sqlite3_mprintf("SELECT pinyin, path, imgpath FROM %q_roms WHERE path LIKE '%%%q' OR disp = %Q LIMIT 1;", cache_type, path_or_name, path_or_name);
    }
    else {
        printf("No cache db found\n");
        return NULL;
    }

    sqlite3_stmt *stmt = cache_db_prepare(cache_db_file_path, sql);
    sqlite3_free(sql);

    if (sqlite3_step(stmt) == SQLITE_ROW) {
        cache_db_item = (CacheDBItem *)malloc(sizeof(CacheDBItem));
        strcpy(cache_db_item->cache_path, cache_db_file_path);
        strcpy(cache_db_item->name, (const char *)sqlite3_column_text(stmt, 0));
        strcpy(cache_db_item->path, (const char *)sqlite3_column_text(stmt, 1));
        strcpy(cache_db_item->imgpath, (const char *)sqlite3_column_text(stmt, 2));
        printf_debug("cache item found: %s\n", cache_db_item->name);
    }
    else {
        printf("Game not found in this cache db\n");
    }

    sqlite3_finalize(stmt);

    return cache_db_item;
}

#endif
