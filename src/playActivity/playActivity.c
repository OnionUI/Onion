#include <fcntl.h>
#include <libgen.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>

#include "cjson/cJSON.h"
#include "utils/file.h"
#include "utils/log.h"
#include "utils/str.h"

// Max number of records in the DB
#define MAXVALUES 1000
#define MAXBACKUPFILES 80
#define NEW_GAME_MINIMAL_PLAYTIME 60
#define INIT_TIMER_PATH "/tmp/initTimer"
#define PLAY_ACTIVITY_DB_PATH                                                  \
    "/mnt/SDCARD/Saves/CurrentProfile/saves/playActivity.db"
#define PLAY_ACTIVITY_DB_TMP_PATH                                              \
    "/mnt/SDCARD/Saves/CurrentProfile/saves/playActivity_tmp.db"
#define PLAY_ACTIVITY_BACKUP_DIR                                               \
    "/mnt/SDCARD/Saves/CurrentProfile/saves/PlayActivityBackup"
#define PLAY_ACTIVITY_BACKUP_NUM(num)                                          \
    "/mnt/SDCARD/Saves/CurrentProfile/saves/PlayActivityBackup/"               \
    "playActivityBackup" num ".db"

static struct rom_s {
    char name[100];
    int playTime;
} rom_list[MAXVALUES];
static int rom_list_len = 0;
static int total_time_played = 0;

int readRomDB(const char *filePath)
{
    total_time_played = 0;

    // Check to avoid corruption
    if (!exists(filePath))
        return -1;

    FILE *file = fopen(filePath, "rb");

    if (file == NULL)
        // The file exists but could not be opened
        // Something went wrong, the program is terminated
        return -1;

    fread(rom_list, sizeof(rom_list), 1, file);
    rom_list_len = 0;
    int i;

    for (i = 0; i < MAXVALUES; i++) {
        if (strlen(rom_list[i].name) > 0) {
            rom_list_len = i + 1;
            total_time_played += rom_list[i].playTime;
        }
    }
    if (total_time_played == 0)
        return -1;

    fclose(file);
    return 0;
}

void writeRomDB(void)
{
    FILE *fp;
    char command[250];
    if (rom_list_len == 0)
        return;

    // Write db in a temporary file
    remove(PLAY_ACTIVITY_DB_TMP_PATH);

    if ((fp = fopen(PLAY_ACTIVITY_DB_TMP_PATH, "wb")) != NULL) {
        fwrite(rom_list, sizeof(rom_list), 1, fp);
        fclose(fp);
        system("sync");
    }

    // Read the new database
    int total_time_played_tmp = total_time_played;
    int nReadSuccess = readRomDB(PLAY_ACTIVITY_DB_TMP_PATH);

    // Check for file corruption
    if ((nReadSuccess != -1) && (total_time_played_tmp <= total_time_played)) {
        // The test passed, the db seems to show valid times
        remove(PLAY_ACTIVITY_DB_PATH);
        sprintf(command, "mv " PLAY_ACTIVITY_DB_TMP_PATH " %s",
                PLAY_ACTIVITY_DB_PATH);
        system(command);
        system("sync");
    }
}

void displayRomDB(void)
{
    printf("--------------------------------\n");
    for (int i = 0; i < rom_list_len; i++) {
        printf("romlist name: %s\n", rom_list[i].name);

        char cPlayTime[15];
        sprintf(cPlayTime, "%d", rom_list[i].playTime);
        printf("playtime: %s\n", cPlayTime);
    }
    printf("--------------------------------\n");
}

int searchRomDB(const char *romName)
{
    int position = -1;

    for (int i = 0; i < rom_list_len; i++) {
        if (strcmp(rom_list[i].name, romName) == 0 ||
            strcmp(file_removeExtension(rom_list[i].name), romName) == 0) {
            position = i;
            break;
        }
    }

    return position;
}

void backupDB(void)
{
    char fileNameToBackup[120];
    char fileNameNextSlot[120];
    char command[250];
    int i;

    mkdir(PLAY_ACTIVITY_BACKUP_DIR, 0700);

    for (i = 0; i < MAXBACKUPFILES; i++) {
        snprintf(fileNameToBackup, sizeof(fileNameToBackup),
                 PLAY_ACTIVITY_BACKUP_NUM("%02d"), i);
        if (!is_file(fileNameToBackup))
            break;
    }

    // Backup
    if (i < MAXBACKUPFILES)
        snprintf(fileNameNextSlot, sizeof(fileNameNextSlot),
                 PLAY_ACTIVITY_BACKUP_NUM("%02d"), i + 1);
    else {
        snprintf(fileNameToBackup, sizeof(fileNameToBackup),
                 PLAY_ACTIVITY_BACKUP_NUM("00"));
        snprintf(fileNameNextSlot, sizeof(fileNameNextSlot),
                 PLAY_ACTIVITY_BACKUP_NUM("01"));
    }
    // Next slot for backup
    remove(fileNameToBackup);
    remove(fileNameNextSlot);

    sprintf(command, "cp " PLAY_ACTIVITY_DB_PATH " %s", fileNameToBackup);
    system(command);
}

void registerTimerEnd(const char *gameName)
{
    FILE *fp;
    long lSize;
    char *baseTime;

    if ((fp = fopen(INIT_TIMER_PATH, "rb")) == 0)
        return;

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
    char cEndEpochTime[15];
    sprintf(cEndEpochTime, "%d", iEndEpochTime);

    char cTempsDeJeuSession[15];
    int iTempsDeJeuSession = iEndEpochTime - iBaseTime;
    sprintf(cTempsDeJeuSession, "%d", iTempsDeJeuSession);

    // Loading DB
    if (readRomDB(PLAY_ACTIVITY_DB_PATH) == -1) {
        free(baseTime);
        // To avoid a DB overwrite
        return;
    }

    // Addition of the new time
    int searchPosition = searchRomDB(gameName);
    if (searchPosition >= 0) {
        // Game found
        rom_list[searchPosition].playTime += iTempsDeJeuSession;
    }
    else {
        // A new game must be used more than
        if (iTempsDeJeuSession > NEW_GAME_MINIMAL_PLAYTIME) {
            // Game inexistant, add to the DB
            if (rom_list_len < MAXVALUES - 1) {
                rom_list[rom_list_len].playTime = iTempsDeJeuSession;
                strcpy(rom_list[rom_list_len].name, gameName);
                rom_list_len++;
            }
        }
    }

    printf("Timer ended (%s): session = %d\n", gameName, iTempsDeJeuSession);

    // DB Backup
    backupDB();

    // We save the DB
    writeRomDB();

    remove(INIT_TIMER_PATH);
    free(baseTime);
}

int main(int argc, char *argv[])
{
    int init_fd;

    if (argc <= 1)
        return 1;

    if (strcmp(argv[1], "init") == 0) {
        int epochTime = (int)time(NULL);
        char baseTime[15];
        sprintf(baseTime, "%d", epochTime);

        remove(INIT_TIMER_PATH);

        if ((init_fd = open(INIT_TIMER_PATH, O_CREAT | O_WRONLY)) > 0) {
            write(init_fd, baseTime, strlen(baseTime));
            close(init_fd);
            system("sync");
        }

        printf("Timer initiated: %d\n", epochTime);

        return EXIT_SUCCESS;
    }

    char cmd[STR_MAX];
    strncpy(cmd, argv[1], STR_MAX - 1);

    char gameName[STR_MAX];
    memset(gameName, 0, STR_MAX);

    if (strstr(cmd, "Roms/PORTS/Shortcuts") != NULL) {
        char *path =
            str_split(cmd, "/mnt/SDCARD/Emu/PORTS/../../Roms/PORTS/Shortcuts");

        if (path != NULL) {
            path[strlen(path) - 1] = 0;
            char full_path[STR_MAX];
            snprintf(full_path, STR_MAX - 1,
                     "/mnt/SDCARD/Roms/PORTS/Shortcuts%s", path);

            if (is_file(full_path)) {
                file_parseKeyValue(full_path, "GameName", gameName, '=', 0);
            }
        }
    }
    else if (strstr(cmd, "/mnt/SDCARD/Roms") != NULL) {
        char *path = str_split(cmd, "/mnt/SDCARD/Roms");

        if (path != NULL) {
            path[strlen(path) - 1] = 0;
            char full_path[STR_MAX];
            snprintf(full_path, STR_MAX - 1, "/mnt/SDCARD/Roms%s", path);

            char name_path[STR_MAX];
            sprintf(name_path, "%s/.game_config/%s.name", dirname(full_path),
                    basename(full_path));

            if (is_file(name_path)) {
                FILE *fp;
                file_get(fp, name_path, "%[^\n]", gameName);
            }
        }
    }

    if (strlen(gameName) == 0) {
        strcpy(gameName, file_removeExtension(basename(argv[1])));
    }

    printf_debug("register end: '%s'\n", gameName);
    registerTimerEnd(gameName);

    return EXIT_SUCCESS;
}
