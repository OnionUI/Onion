#include <fcntl.h>
#include <libgen.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>
#include <sqlite3/sqlite3.h>

#include "utils/file.h"
#include "utils/log.h"
#include "utils/str.h"

#define INIT_TIMER_PATH "/tmp/initTimer"
#define PLAY_ACTIVITY_SQLITE_PATH "/mnt/SDCARD/Saves/CurrentProfile/saves/playActivity.sqlite"
#define PLAY_ACTIVITY_DB_PATH "/mnt/SDCARD/Saves/CurrentProfile/saves/playActivity.db"

sqlite3 *db;
static struct rom_s {
    char *name;
    int playTime;
};

void closeDB(void) {
    printf_debug("%s\n", "closeDB()");
    sqlite3_close(db);
    printf_debug("%s\n", "closeDB() return");
}

void upgradeRomDB(void) {
    printf_debug("%s\n", "upgradeRomDB()");
    FILE *file = fopen(PLAY_ACTIVITY_DB_PATH, "rb");
    struct rom_s romList[1000];
    if (file != NULL) {
        fread(romList, sizeof(romList), 1, file);
        fclose(file);
    }
    char *err_msg = 0;
    int rc = sqlite3_open(PLAY_ACTIVITY_SQLITE_PATH, &db);
    if (rc != SQLITE_OK) {
        printf_debug("Cannot open database: %s\n", sqlite3_errmsg(db));
        closeDB();
        printf_debug("%s\n", "upgradeRomDB() return");
        return;
    }
    char *sql;
    strncpy(sql, "DROP TABLE IF EXISTS playActivity;", STR_MAX);
    strncat(sql, "CREATE TABLE playActivity(name TEXT, filePath Text, playCount INT, playTime INT);", STR_MAX);
    strncat(sql, "CREATE UNIQUE INDEX name_index ON playActivity(name);", STR_MAX);
    int i;
    char *insert;
    for (i = 0; i < sizeof(romList)/sizeof(romList[0]); i++) {
        if (strlen(romList[i].name) > 0) {
            snprintf(insert, STR_MAX, "INSERT OR REPLACE INTO playActivity VALUES ('%s', NULL, 1, %d);", romList[i].name, romList[i].playTime);
            strncat(sql, insert, STR_MAX);
        }
    }
    rc = sqlite3_exec(db, sql, 0, 0, &err_msg);
    if (rc != SQLITE_OK ) {
        printf_debug("SQL error: %s\n", sqlite3_errmsg(db));
    }
    printf_debug("%s\n", "upgradeRomDB() return");
}

void openDB(void) {
    printf_debug("%s\n", "openDB()");
    if (!exists(PLAY_ACTIVITY_SQLITE_PATH)) {
        upgradeRomDB();
    }
    int rc = sqlite3_open(PLAY_ACTIVITY_SQLITE_PATH, &db);
    if (rc != SQLITE_OK) {
        printf_debug("Cannot open database: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
    }
    printf_debug("%s\n", "openDB() return");
}

void addPlayTime(const char* name, const char* filePath, int playTime) {
    printf_debug("addPlayTime(%s, %s)\n", name, filePath);
    sqlite3_stmt *res;
    char *updateSQL = "INSERT OR REPLACE INTO playActivity VALUES(?, ?, COALESCE((SELECT playCount FROM playActivity WHERE name=?), 0) + 1, COALESCE((SELECT playTime FROM playActivity WHERE name=?), 0) + ?);";
    int rc = sqlite3_prepare_v2(db, updateSQL, -1, &res, NULL);
    if (rc == SQLITE_OK) {
        sqlite3_bind_text(res, 1, name, -1, NULL);
        sqlite3_bind_text(res, 2, filePath, -1, NULL);
        sqlite3_bind_text(res, 3, name, -1, NULL);
        sqlite3_bind_text(res, 4, name, -1, NULL);
        sqlite3_bind_int(res, 5, playTime);
    } else {
        printf_debug("Failed to execute statement: %s\n", sqlite3_errmsg(db));
    }
    printf_debug("%s\n", sqlite3_expanded_sql(res));
    rc = sqlite3_step(res);
    if (rc == SQLITE_ROW) {
        printf_debug("Success: %s\n", sqlite3_column_text(res, 0));
    }
    sqlite3_finalize(res);
    printf_debug("%s\n", "addPlayTime() return");
}

void registerTimerEnd(const char *gameName, const char *filePath)
{
    printf_debug("registerTimerEnd(%s, %s)\n", gameName, filePath);
    FILE *fp;
    long lSize;
    char *baseTime;
    if ((fp = fopen(INIT_TIMER_PATH, "rb")) == 0) {
        return;
    }
    fseek(fp, 0L, SEEK_END);
    lSize = ftell(fp);
    rewind(fp);
    baseTime = (char *)calloc(1, lSize + 1);
    if (!baseTime) {
        fclose(fp);
        fputs("memory alloc fails", stderr);
        exit(1);
    }
    if (1 != fread(baseTime, lSize, 1, fp)) {
        fclose(fp);
        free(baseTime);
        fputs("entire read fails", stderr);
        exit(1);
    }
    fclose(fp);
    int iBaseTime = atoi(baseTime);
    int iEndEpochTime = (int)time(NULL);
    int playTime = iEndEpochTime - iBaseTime;
    addPlayTime(gameName, filePath, playTime);
    remove(INIT_TIMER_PATH);
    free(baseTime);
    printf_debug("%s\n", "registerTimerEnd() return");
}

int main(int argc, char *argv[])
{
    log_setName("playActivity");
    printf_debug("%s\n", "main()");
    if (argc <= 1) {
        printf_debug("%s\n", "main() argc <= 1");
        return 1;
    }
    openDB();
    if (db == NULL) {
        printf_debug("%s\n", "db == NULL");
        return 1;
    }
    if (strcmp(argv[1], "init") == 0) {
        printf_debug("%s\n", "main() argv[1] = 'init'");
        int epochTime = (int)time(NULL);
        char baseTime[15];
        sprintf(baseTime, "%d", epochTime);
        printf_debug("%s\n", "main() init remove init timer");
        remove(INIT_TIMER_PATH);
        int init_fd;
        if ((init_fd = open(INIT_TIMER_PATH, O_CREAT | O_WRONLY)) > 0) {
            printf_debug("%s\n", "main() init write init fd");
            write(init_fd, baseTime, strlen(baseTime));
            close(init_fd);
            system("sync");
            printf_debug("%s\n", "main() init saved init fd");
        }
        printf_debug("main() init Timer initiated @ %d\n", epochTime);
        printf_debug("main() init return %d\n", EXIT_SUCCESS);
        return EXIT_SUCCESS;
    }
    char *cmd;
    strncpy(cmd, argv[1], STR_MAX);
    char *gameName;
    char *filePath;
    char *relativePath;
    if (strstr(cmd, "../../Roms/") != NULL) {
        printf_debug("%s", "main() cmd includes '../../Roms/'\n");
        relativePath = str_split(cmd, "../../Roms/");
        if (relativePath != NULL) {
            relativePath[strlen(relativePath) - 1] = 0;
            snprintf(filePath, STR_MAX, "/mnt/SDCARD/Roms/./%s", relativePath);
            char *name_path;
            sprintf(name_path, "%s/.game_config/%s.name", dirname(filePath), basename(filePath));
            if (is_file(name_path)) {
                FILE *fp;
                file_get(fp, name_path, "%[^\n]", gameName);
            }
            if (strlen(gameName) == 0) {
                strncpy(gameName, file_removeExtension(basename(argv[1])), STR_MAX);
            }
            printf_debug("main() cmd = '%s'\n", cmd);
            printf_debug("main() gameName = '%s'\n", gameName);
            printf_debug("main() filePath = '%s'\n", filePath);
            printf_debug("main() relativePath = '%s'\n", relativePath);
            registerTimerEnd(gameName, relativePath);
        }
    }
    closeDB();
    printf_debug("main() return %d\n", EXIT_SUCCESS);
    return EXIT_SUCCESS;
}
