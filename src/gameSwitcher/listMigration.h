// *********************************************************************
// Migrate retroarch's content_history.lpl entries (old logic)
// into Miyoo's recentlist.json (New logic that incorporate standalones)
// *********************************************************************

#include "system/settings.h"
#include "system/state.h"
#include "utils/file.h"

#define ROM_SCREENS_DIR "/mnt/SDCARD/Saves/CurrentProfile/romScreens"
#define MIGRATION_TEMP_LIST "/mnt/SDCARD/Roms/recentlist_temp.json"

#define MAXHISTORY 100
#define MAXHROMNAMESIZE 250

// Game history list
typedef struct {
    uint32_t hash;
    char name[MAXHROMNAMESIZE];
    char shortname[STR_MAX];
    int is_duplicate;
    char path[PATH_MAX];
    char core[PATH_MAX];
    char picture[PATH_MAX];
} Game_s_migration;

static Game_s_migration game_list_migration[MAXHISTORY];
static int game_list_len_migration = 0;

static cJSON *json_root_migration = NULL;
static cJSON *json_items_migration = NULL;

void getGameName_migration(char *name_out, const char *rom_path)
{
    CacheDBItem *cache_item = cache_db_find(rom_path);
    if (cache_item != NULL) {
        strcpy(name_out, cache_item->name);
        free(cache_item);
    }
    else {
        strcpy(name_out, file_removeExtension(basename(strdup(rom_path))));
    }
}

char *extractConsoleNameFromRomPath(const char *path)
{
    const char *base = "/mnt/SDCARD/Roms/";
    const char *startOfDir = strstr(path, base);

    if (startOfDir != NULL) {
        startOfDir += strlen(base);

        // Find the end of the directory
        const char *endOfDir = strchr(startOfDir, '/');

        // Allocate memory for the result
        size_t dirLength = endOfDir != NULL ? (size_t)(endOfDir - startOfDir) : strlen(startOfDir);
        char *directory = (char *)malloc(dirLength + 1);

        // Copy the directory into the new string
        strncpy(directory, startOfDir, dirLength);
        directory[dirLength] = '\0';

        return directory;
    }
    else {
        return NULL;
    }
}

// Function to extract all subfolders to the right of "/mnt/SDCARD/Roms"
char *extractAllSubfoldersFromRomPath(const char *path) {
    const char *base = "/mnt/SDCARD/Roms/";
    const char *startOfDir = strstr(path, base);

    if (startOfDir != NULL) {
        startOfDir += strlen(base);

        // Find the end of the path
        const char *endOfPath = strrchr(startOfDir, '/');

        // Allocate memory for the result
        size_t pathLength = endOfPath != NULL ? (size_t)(endOfPath - startOfDir) : strlen(startOfDir);
        char *subfolders = (char *)malloc(pathLength + 1);

        // Copy the subfolders into the new string
        strncpy(subfolders, startOfDir, pathLength);
        subfolders[pathLength] = '\0';

        return subfolders;
    } else {
        return NULL;
    }
}

void readHistoryRA()
{
    game_list_len_migration = 0;

    if (!exists(HISTORY_PATH)) {
        print_debug("RA history file missing");
        return;
    }

    char rom_path[STR_MAX];
    char core_path[STR_MAX];

    if (json_items_migration == NULL) {
        json_root_migration = json_load(HISTORY_PATH);
        json_items_migration = cJSON_GetObjectItem(json_root_migration, "items");
    }

    for (int nbGame = 0; nbGame < MAXHISTORY; nbGame++) {
        cJSON *subitem = cJSON_GetArrayItem(json_items_migration, nbGame);

        if (subitem == NULL)
            break;

        if (!json_getString(subitem, "path", rom_path) ||
            !json_getString(subitem, "core_path", core_path))
            continue;

        if (strncmp("/mnt/SDCARD/App", rom_path, 15) == 0)
            continue;

        Game_s_migration *game = &game_list_migration[game_list_len_migration];
        game->hash = FNV1A_Pippip_Yurii(rom_path, strlen(rom_path));
        game->is_duplicate = 0;
        getGameName_migration(game->name, rom_path);
        strcpy(game->path, rom_path);
        file_cleanName(game->shortname, game->name);

        strcpy(game->core, basename(core_path));
        str_split(game->core, "_libretro");

        printf_debug("Game loaded:\n"
                     "\tname: '%s' (%s)\n"
                     "\thash: %" PRIu32 "\n"
                     "\tpath: '%s'\n"
                     "\n",
                     game->name, game->shortname,
                     game->hash,
                     game->path);

        // Check for duplicates
        for (int i = 0; i < game_list_len_migration; i++) {
            Game_s_migration *other = &game_list_migration[i];
            if (other->hash == game->hash) {
                other->is_duplicate += 1;
                game->is_duplicate = other->is_duplicate;
            }
        }

        game_list_len_migration++;
    }
}

void buildRecentFile()
{

    if (is_file(MIGRATION_TEMP_LIST)) {
        remove(MIGRATION_TEMP_LIST);
    }

    FILE *temp_file;
    temp_file = fopen(MIGRATION_TEMP_LIST, "a");

    for (int i = 0; i < game_list_len_migration; i++) {

        Game_s_migration *game = &game_list_migration[i];
        if (strncmp("/mnt/SDCARD/Roms", game->path, 16) == 0) {
            if (game->is_duplicate == 0) {

                printf_debug("Game '%s'\n", game->shortname);

                // old screenshot path
                char oldPicturePath[PATH_MAX * 2];
                sprintf(oldPicturePath, "%s/%" PRIu32 "_%s.png", ROM_SCREENS_DIR, game->hash,
                        game->core);
                printf_debug("oldPicturePath : %s\n", oldPicturePath);

                char imgPath[STR_MAX*2];
                char console_folder[10];
                sprintf(console_folder, extractConsoleNameFromRomPath(game->path));

                char all_subs[STR_MAX];
                sprintf(all_subs, extractAllSubfoldersFromRomPath(game->path));

                // miyoo boxart png path
                sprintf(imgPath, "/mnt/SDCARD/Emu/%s/../../Roms/%s/Imgs/%s.png", console_folder, all_subs, file_removeExtension(basename(game->path)));

                char tempString[STR_MAX*2];
                sprintf(tempString, "/mnt/SDCARD/Emu/%s/../../Roms/%s/%s", console_folder, all_subs, basename(game->path));
                strcpy(game->path, tempString);

                // new screenshot path
                printf_debug("new rom path : %s\n", game->path);
                game->hash = FNV1A_Pippip_Yurii(game->path, strlen(game->path));
                char currPicturePath[PATH_MAX];
                sprintf(currPicturePath, ROM_SCREENS_DIR "/%" PRIu32 ".png", game->hash);
                printf_debug("currPicturePath : %s\n", currPicturePath);

                // screenshot renaming
                if (is_file(oldPicturePath))
                    rename(oldPicturePath, currPicturePath);

                printf_debug("png path'%s':\n", imgPath);
                if (!is_file(imgPath)) {
                    imgPath[0] = '\0';
                }

                if (!is_file(imgPath))
                   strcpy(imgPath, "");

                char launchPath[STR_MAX];

                sprintf(launchPath, "/mnt/SDCARD/Emu/%s/launch.sh", console_folder);
                printf_debug("{\"label\":\"%s\",\"rompath\":\"%s\",\"imgpath\":\"%s\",\"launch\":\"%s\",\"type\":5}\n", game->name,
                             game->path, imgPath, launchPath);

                fprintf(temp_file, "{\"label\":\"%s\",\"rompath\":\"%s\",\"imgpath\":\"%s\",\"launch\":\"%s\",\"type\":5}\n", game->name,
                        game->path, imgPath, launchPath);
            }
        }
    }
    fclose(temp_file);
    char sCommand[STR_MAX * 2];
    /*
    sprintf(sCommand, "cat %s >> %s", sRecentListPath, MIGRATION_TEMP_LIST);
    system(sCommand);

    remove(sRecentListPath);
*/
    sprintf(sCommand, "mv %s %s", MIGRATION_TEMP_LIST, getMiyooRecentFilePath());
    system(sCommand);

    system("sync");
}

int migrateGameSwitcherList()
{

    readHistoryRA();

    if (game_list_len_migration > 0) {
        buildRecentFile();
    }
    return 1;
}
