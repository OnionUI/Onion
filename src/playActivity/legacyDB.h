#ifndef PLAY_ACTIVITY_LEGACY_DB
#define PLAY_ACTIVITY_LEGACY_DB

#include "utils/file.h"
#include "utils/str.h"

#define PLAY_ACTIVITY_DB_OLD_PATH "/mnt/SDCARD/Saves/CurrentProfile/saves/playActivity.db"
#define PLAY_ACTIVITY_DB_OLD_PATH_TMP "/mnt/SDCARD/Saves/CurrentProfile/saves/playActivity_tmp.db"

#define LEGACY_DB_MAX 1000

typedef struct structRom { // for reading from old DB
    char name[100];
    int playTime;
} rom_list_s;

static rom_list_s rom_list[LEGACY_DB_MAX];
static int rom_list_len = 0;

int readLegacyDB()
{
    FILE *fp;

    if (is_file(PLAY_ACTIVITY_DB_OLD_PATH)) {
        if ((fp = fopen(PLAY_ACTIVITY_DB_OLD_PATH, "rb")) != NULL) {
            fread(rom_list, sizeof(rom_list), 1, fp);
            rom_list_len = 0;

            for (int i = 0; i < LEGACY_DB_MAX; i++) {
                if ((strlen(rom_list[i].name) != 0) && (rom_list[i].playTime) != 0)
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

void displayLegacyDB(void)
{
    printf("--------------- Old DB entries ---------------\n");
    for (int i = 0; i < rom_list_len; i++) {
        printf("rom_list name: %s\n", rom_list[i].name);

        char cPlayTime[15];
        sprintf(cPlayTime, "%d", rom_list[i].playTime);
        printf("playtime: %s\n", cPlayTime);
    }
}

#endif // PLAY_ACTIVITY_LEGACY_DB
