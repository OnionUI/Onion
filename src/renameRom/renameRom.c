#include <libgen.h>
#include <sqlite3/sqlite3.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "components/JsonGameEntry.h"
#include "utils/file.h"
#include "utils/log.h"
#include "utils/str.h"

void _path(char *dest, const char *dir_path, const char *file_name,
           const char *file_ext)
{
    snprintf(dest, STR_MAX * 3 - 1, "%s/%s.%s", dir_path, file_name, file_ext);
}

bool renameFile(const char *dir_path, const char *file_ext,
                const char *old_name, const char *new_name)
{
    char old_path[STR_MAX * 3], new_path[STR_MAX * 3];
    _path(old_path, dir_path, old_name, file_ext);
    _path(new_path, dir_path, new_name, file_ext);

    if (is_file(old_path)) {
        printf_debug("\nrename: '%s'\n     -> '%s'\n", old_path, new_path);
#ifdef PLATFORM_MIYOOMINI
        if (rename(old_path, new_path) == 0) {
            print_debug("File renamed successfully.");
        }
#endif
        return true;
    }

    return false;
}

bool renameCache(const char *cache_path, const char *rom_dirname,
                 const char *rom_path, const char *new_rompath,
                 const char *new_imgpath, const char *new_name)
{
    sqlite3 *db;
    sqlite3_stmt *res;

    int rc = sqlite3_open(cache_path, &db);

    if (rc != SQLITE_OK) {
        fprintf(stderr, "Cannot open database: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return false;
    }

    const char *sql = sqlite3_mprintf(
        "UPDATE %q_roms SET path = %Q, imgpath = %Q, disp = %Q WHERE path = %Q",
        rom_dirname, new_rompath, new_imgpath, new_name, rom_path);
    printf_debug("query: %s\n", sql);

    rc = sqlite3_prepare_v2(db, sql, -1, &res, 0);

    if (rc != SQLITE_OK) {
        fprintf(stderr, "Failed to fetch data: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return false;
    }

    rc = sqlite3_step(res);

    if (rc == SQLITE_ROW) {
        printf("%s\n", sqlite3_column_text(res, 0));
    }

    sqlite3_finalize(res);
    sqlite3_close(db);

    return true;
}

int main(int argc, char *argv[])
{
    if (argc < 2) {
        printf("Usage: renameRom [ROMPATH] [NEWNAME]\n");
        return 1;
    }

    char rompath[STR_MAX], new_name[STR_MAX];

    strncpy(rompath, argv[1], STR_MAX - 1);
    strncpy(new_name, argv[2], STR_MAX - 1);

    printf_debug("rompath: %s\n", rompath);
    printf_debug("rename to: %s\n", new_name);

    if (!is_file(rompath))
        return 1;

    char romdir[STR_MAX];
    strncpy(romdir, rompath, STR_MAX - 1);
    dirname(romdir);

    const char *romext = file_getExtension(rompath);
    char *old_name = file_removeExtension(basename(rompath));
    printf_debug("old name: %s\n", old_name);

    // Rename rom file
    renameFile(romdir, romext, old_name, new_name);

    char emupath[STR_MAX];
    strncpy(emupath, rompath, STR_MAX - 1);
    str_split(emupath, "/../../");

    printf_debug("emupath: %s\n", emupath);

    char config_path[STR_MAX];
    concat(config_path, emupath, "/config.json");

    if (!is_file(config_path))
        return 0;

    char *config_str = file_read(config_path);
    JsonGameEntry config = JsonGameEntry_fromJson(config_str);
    free(config_str);

    // Rename box art

    char imgdir[STR_MAX * 2 + 1];
    snprintf(imgdir, STR_MAX * 2, "%s/%s", emupath, config.imgpath);

    if (!renameFile(imgdir, "png", old_name, new_name)) {
        print_debug("No box art found");
    }
    free(old_name);

    // Rename cache entry

    char cache_path[STR_MAX * 3];
    snprintf(cache_path, STR_MAX * 3 - 1, "%s/%s/%s_cache6.db", emupath,
             config.rompath, basename(config.rompath));

    char new_rompath[STR_MAX * 3], new_imgpath[STR_MAX * 3];
    _path(new_rompath, romdir, new_name, romext);
    _path(new_imgpath, imgdir, new_name, "png");

    printf_debug("cache path: %s\n", cache_path);
    renameCache(cache_path, basename(config.rompath), rompath, new_rompath,
                new_imgpath, new_name);

    return 0;
}
