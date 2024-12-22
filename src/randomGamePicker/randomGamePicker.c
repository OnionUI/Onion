#include <dirent.h>
#include <libgen.h>
#include <sqlite3/sqlite3.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "cjson/cJSON.h"
#include "components/JsonGameEntry.h"
#include "utils/file.h"
#include "utils/flags.h"
#include "utils/log.h"

#define MAX_SYSTEMS 500
#define ERROR_CODE_NO_GAME_FOUND 99

#define PATH_FAVORITES "/mnt/SDCARD/Roms/favourite.json"
#define PATH_RECENTS "/mnt/SDCARD/Roms/recentlist.json"
#define PATH_EMU "/mnt/SDCARD/Emu/"
#define PATH_RAPP "/mnt/SDCARD/RApp/"

typedef struct game_entry_s {
    int id;
    int sum;
    int c_sum;
    char label[STR_MAX];
    char path[STR_MAX];
    char img_path[STR_MAX * 3 + 3];
    char emu_name[STR_MAX];
    char launch_path[STR_MAX * 2];
} GameEntry;

static GameEntry
    random_games[MAX_SYSTEMS]; // contains a random game for each system
static int system_count = 0;
static int total_games_count = 0;

void print_game(GameEntry *game)
{
    printf("emu=%s\n"
           "label=%s\n"
           "path=%s\n"
           "img=%s\n"
           "launch=%s\n",
           game->emu_name, game->label, game->path, game->img_path,
           game->launch_path);
}

bool loadEmuConfig(char *emupath, char *emuname_out, char *romsdir_out,
                   char *launch_out, char *imgsdir_out)
{
    char config_path[STR_MAX + 13];
    snprintf(config_path, STR_MAX + 12, "%s/config.json", emupath);

    if (!exists(config_path))
        return false;

    cJSON *json_root = json_load(config_path);

    if (romsdir_out != NULL) {
        char romsdir_rel[STR_MAX];
        if (!json_getString(json_root, "rompath", romsdir_rel)) {
            cJSON_Delete(json_root);
            return false;
        }

        // Ignore Search results
        if (strncmp(romsdir_rel, "../../App/", 10) == 0) {
            cJSON_Delete(json_root);
            return false;
        }

        snprintf(romsdir_out, STR_MAX * 2 + 1, "%s/%s", emupath, romsdir_rel);
    }

    if (launch_out != NULL) {
        char launch_rel[STR_MAX];
        if (!json_getString(json_root, "launch", launch_rel))
            return false;
        snprintf(launch_out, STR_MAX * 2 + 1, "%s/%s", emupath, launch_rel);
    }

    if (emuname_out != NULL) {
        char label_temp[STR_MAX];
        if (!json_getString(json_root, "label", label_temp))
            strcpy(emuname_out, basename(emupath));
        else
            str_trim(emuname_out, STR_MAX - 1, label_temp, false);
    }

    if (imgsdir_out != NULL) {
        char imgpath_rel[STR_MAX];
        if (json_getString(json_root, "imgpath", imgpath_rel))
            snprintf(imgsdir_out, STR_MAX * 2 + 1, "%s/%s", emupath,
                     imgpath_rel);
    }

    cJSON_Delete(json_root);
    return true;
}

int getTotalGamesCount(sqlite3 *db, const char *table_name)
{
    sqlite3_stmt *res;
    const char *sql = sqlite3_mprintf(
        "SELECT COUNT(id) FROM %q WHERE type=0 AND path NOT LIKE '%%.miyoocmd'",
        table_name);
    if (sqlite3_prepare_v2(db, sql, -1, &res, 0) != SQLITE_OK)
        return 0;
    if (sqlite3_step(res) != SQLITE_ROW) {
        sqlite3_finalize(res);
        return 0;
    }
    return sqlite3_column_int(res, 0);
}

bool pickRandomGameFromCache(char *emuname, char *romsdir,
                             const char *launch_path)
{
    sqlite3 *db;
    sqlite3_stmt *res;

    char cache_path[STR_MAX * 3];
    snprintf(cache_path, STR_MAX * 3 - 1, "%s/%s_cache6.db", romsdir,
             basename(romsdir));
    printf_debug("cache: %s\n", cache_path);

    if (!is_file(cache_path))
        return false;

    char table_name[STR_MAX];
    snprintf(table_name, STR_MAX, "%s_roms", basename(romsdir));

    if (sqlite3_open(cache_path, &db) != SQLITE_OK) {
        fprintf(stderr, "Cannot open database: %s (%s)\n", sqlite3_errmsg(db),
                cache_path);
        sqlite3_close(db);
        return false;
    }

    int count = getTotalGamesCount(db, table_name);

    const char *sql = sqlite3_mprintf("SELECT id, pinyin, path, imgpath FROM "
                                      "%q WHERE type=0 AND path NOT LIKE "
                                      "'%%.miyoocmd' ORDER BY RANDOM() LIMIT 1",
                                      table_name);

    if (sqlite3_prepare_v2(db, sql, -1, &res, 0) != SQLITE_OK) {
        sqlite3_close(db);
        return false;
    }

    if (sqlite3_step(res) == SQLITE_ROW) {
        GameEntry *game = &random_games[system_count];
        system_count++;
        total_games_count += count;

        game->id = sqlite3_column_int(res, 0);
        game->sum = count;
        game->c_sum = total_games_count;

        strncpy(game->label, (const char *)sqlite3_column_text(res, 1),
                STR_MAX - 1);
        strncpy(game->path, (const char *)sqlite3_column_text(res, 2),
                STR_MAX - 1);
        strncpy(game->img_path, (const char *)sqlite3_column_text(res, 3),
                STR_MAX - 1);

        strncpy(game->launch_path, launch_path, STR_MAX * 2 - 1);
        strncpy(game->emu_name, emuname, STR_MAX - 1);
    }

    sqlite3_finalize(res);
    sqlite3_close(db);

    return true;
}

bool addRandomFromEmu(char *emupath)
{
    char emuname[STR_MAX];
    char romsdir[STR_MAX * 2 + 2];
    char launch_path[STR_MAX * 2 + 2];

    if (!loadEmuConfig(emupath, emuname, romsdir, launch_path, NULL))
        return false;

    printf_debug("\nemuname: %s\nromsdir: %s\nlaunch: %s\n", emuname, romsdir,
                 launch_path);

    return pickRandomGameFromCache(emuname, romsdir, launch_path);
}

typedef enum {
    TYPE_UNKNOWN,
    TYPE_APP = 3,
    TYPE_GAME = 5,
    TYPE_EXPERT = 17
} JsonEntryType_e;

bool extractEmuPath(char *emupath, char *comp_path)
{
    if (strncmp(comp_path, emupath, strlen(comp_path)) == 0) {
        char *str_p = strstr(emupath + strlen(comp_path), "/");
        *str_p = 0;
        return true;
    }

    return false;
}

bool addRandomFromJson(char *json_path)
{
    int count = 0;

    FILE *fp;
    char line[STR_MAX * 4];
    char path_a[STR_MAX];
    char path_b[STR_MAX];
    cJSON *json_root;
    JsonEntryType_e type;

    if ((fp = fopen(json_path, "r")) == NULL)
        return false;

    while (fgets(line, sizeof(line), fp)) {
        json_root = cJSON_Parse(line);
        if(!json_getInt(json_root, "type", (int *)&type)) {
            print_debug("Malformed json; Skipping\n");
            continue;
        }

        if (type == TYPE_GAME || type == TYPE_EXPERT) {
            GameEntry *game = &random_games[count];
            memset(game->label, 0, strlen(game->label));
            memset(game->path, 0, strlen(game->path));
            memset(game->img_path, 0, strlen(game->img_path));
            memset(game->launch_path, 0, strlen(game->launch_path));

            game->id = type;
            game->sum = 1;
            game->c_sum = count + 1;
            json_getString(json_root, "label", game->label);
            json_getString(json_root, "rompath", game->path);
            json_getString(json_root, "imgpath", game->img_path);
            json_getString(json_root, "launch", game->launch_path);

            if (!is_file(game->path))
                continue;

            if (strcmp("miyoocmd", file_getExtension(game->path)) == 0)
                continue;

            realpath(game->path, path_b);
            bool is_duplicate = false;

            for (int i = 0; i < count; i++) {
                GameEntry *other = &random_games[i];
                realpath(other->path, path_a);
                if (other->id == game->id && strcmp(path_a, path_b) == 0) {
                    is_duplicate = true;
                    break;
                }
            }

            if (is_duplicate)
                continue;

            char emupath[STR_MAX];
            strcpy(emupath, game->launch_path);

            if (!extractEmuPath(emupath, PATH_EMU))
                extractEmuPath(emupath, PATH_RAPP);

            char imgsdir[STR_MAX * 2 + 2];

            if (!is_dir(emupath) ||
                !loadEmuConfig(emupath, game->emu_name, NULL, game->launch_path,
                               imgsdir) ||
                !is_file(game->launch_path))
                continue;

            if (strlen(game->img_path) == 0) {
                char *no_extension = file_removeExtension(basename(game->path));
                snprintf(game->img_path, STR_MAX * 3 + 2, "%s/%s.png", imgsdir,
                         no_extension);
                free(no_extension);
            }

            count++;
        }

        cJSON_Delete(json_root);
    }

    total_games_count = system_count = count;

    fclose(fp);
    return true;
}

#define THICK_BAR \
    "================================================================="
#define THIN_BAR \
    "-----------------------------------------------------------------"

void logWeights()
{
    GameEntry *g;
    FILE *fp;

    if ((fp = fopen("/mnt/SDCARD/Emu/random_weights.log", "w+")) == NULL)
        return;

    print_debug("generating weights file: Emu/random_weights.log");

    float system_chance = 1.0 / system_count;
    float equal_chance = 1.0 / total_games_count;

    fprintf(fp, THICK_BAR "\n");
    fprintf(fp, " %-23s  %s  %s\n", "", "      METHOD A    ", "      METHOD B");
    fprintf(fp, " %-15s  %6s    %7s  %7s    %7s  %7s\n", "EMU", "COUNT",
            "WEIGHT", "CHANCE", "WEIGHT", "CHANCE");
    fprintf(fp, THIN_BAR "\n");

    for (int i = 0; i < system_count; i++) {
        g = &random_games[i];

        float weight = (float)g->sum / total_games_count;
        float chance = 1.0 / g->sum;

        fprintf(fp, " %-15s  %6d    %6.2f%%  %6.2f%%    %6.2f%%  %6.2f%%\n",
                g->emu_name, g->sum, weight * 100, equal_chance * 100,
                system_chance * 100, chance * system_chance * 100);
    }

    fprintf(fp, THIN_BAR "\n");
    fprintf(fp, " TOTAL: %-4d  %10d\n", system_count, total_games_count);
    fprintf(fp, THICK_BAR "\n\n");

    fprintf(fp, "Generated by RandomGamePicker\n");

    fclose(fp);
}

typedef enum {
    MODE_ALL,
    MODE_SINGLE_SYSTEM,
    MODE_FAVORITES,
    MODE_RECENTS
} Mode_e;

int main(int argc, char *argv[])
{
    int random_number = 0;
    srand(time(NULL));

    DIR *dp;
    struct dirent *ep;

    char emupath[STR_MAX + 17];
    Mode_e mode = MODE_ALL;

    if (argc > 1) {
        if (strcmp("--favorites", argv[1]) == 0)
            mode = MODE_FAVORITES;
        else if (strcmp("--recents", argv[1]) == 0)
            mode = MODE_RECENTS;
        else {
            strncpy(emupath, argv[1], STR_MAX + 16);
            mode = MODE_SINGLE_SYSTEM;
        }
    }

    switch (mode) {
    case MODE_FAVORITES:
    case MODE_RECENTS:
        addRandomFromJson(mode == MODE_FAVORITES ? PATH_FAVORITES
                                                 : PATH_RECENTS);
        random_number = rand() % total_games_count;
        break;
    case MODE_SINGLE_SYSTEM:
        printf_debug("mode: single, emupath: %s\n", emupath);
        addRandomFromEmu(emupath);
        break;
    case MODE_ALL:
    default:
        if ((dp = opendir("/mnt/SDCARD/Emu")) != NULL) {

            // choose a random game from all emus
            while ((ep = readdir(dp))) {
                if (ep->d_type != DT_DIR)
                    continue;
                snprintf(emupath, STR_MAX + 16, "/mnt/SDCARD/Emu/%s",
                         ep->d_name);
                addRandomFromEmu(emupath);
            }

            closedir(dp);
        }
        else {
            perror("Emu folder does not exists");
        }

        int random_weighted_index = rand() % total_games_count;
        printf_debug("total: %d\n", total_games_count);
        printf_debug("rwi: %d\n", random_weighted_index);

        for (int i = 0; i < system_count; i++) {
            if (random_weighted_index < random_games[i].c_sum) {
                random_number = i;
                break;
            }
        }

        logWeights();
        break;
    }

    if (system_count == 0)
        return ERROR_CODE_NO_GAME_FOUND;

    GameEntry *chosen_game = &random_games[random_number];

    char cmd_to_run[STR_MAX * 3 + 65];
    snprintf(
        cmd_to_run, STR_MAX * 3 + 64,
        "LD_PRELOAD=/mnt/SDCARD/miyoo/app/../lib/libpadsp.so \"%s\" \"%s\"",
        chosen_game->launch_path, chosen_game->path);

    FILE *fp;
    file_put_sync(fp, "/mnt/SDCARD/.tmp_update/cmd_to_run.sh", "%s",
                  cmd_to_run);

    printf_debug("cmd_to_run: %s\n", cmd_to_run);

    print_game(chosen_game);

    fflush(stdin);
    return EXIT_SUCCESS;
}