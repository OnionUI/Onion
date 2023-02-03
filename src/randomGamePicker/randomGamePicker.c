#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <dirent.h> 
#include <libgen.h>
#include <sqlite3/sqlite3.h>

#include "utils/flags.h"
#include "utils/file.h"
#include "utils/log.h"
#include "components/JsonGameEntry.h"
#include "cjson/cJSON.h"

#define MAXGAMES 90000
#define ERROR_CODE_NO_GAME_FOUND 99

int total_games = 0;
JsonGameEntry all_games[MAXGAMES];

bool readDatabase(const char *cache_path, const char *table_name) {
    sqlite3 *db;
    sqlite3_stmt *res;

    int rc = sqlite3_open(cache_path, &db);
    
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Cannot open database: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return false;
    }

    const char *sql = sqlite3_mprintf("SELECT * FROM %q WHERE type=0 ORDER BY RANDOM() LIMIT 1", table_name);
    printf_debug("query: %s\n", sql);

    rc = sqlite3_prepare_v2(db, sql, -1, &res, 0);    
    
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Failed to fetch data: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return false;
    }    

    while(sqlite3_step(res) == SQLITE_ROW)
    {   
        cJSON *games = cJSON_CreateObject();
        char *out;

        cJSON_AddItemToObject(games, "rompath", cJSON_CreateString((const char *)sqlite3_column_text(res, 2)));
        out = cJSON_Print(games);

        JsonGameEntry jsonGameEntry = JsonGameEntry_fromJson(out);

        all_games[total_games] = jsonGameEntry;
        total_games++;
    }

    sqlite3_finalize(res);
    sqlite3_close(db);
    
    return true;
}

void readConfigFile(const char *emuname) {

    char config_path[512];    
    sprintf(config_path, "/mnt/SDCARD/Emu/%s/config.json", emuname);

    if (exists(config_path)) {
        const char *config_str = file_read(config_path);
        
        JsonGameEntry config = JsonGameEntry_fromJson(config_str);
        
        if (strncmp(config.rompath, "../../App/", 10) != 0) {
            char cache_path[STR_MAX * 3];
            snprintf(cache_path, STR_MAX * 3 - 1, "%s/%s/%s/%s_cache2.db", "/mnt/SDCARD/Emu", emuname, config.rompath, basename(config.rompath));

            char table_name[STR_MAX];
            char *last = strrchr(config.rompath, '/');
            if (last != NULL) {                    
                snprintf(table_name, STR_MAX, "%s_roms", last+1);
                printf_debug("table_name: '%s'\n", table_name);
            }       
            
            printf_debug("cache_path: %s\n", cache_path);

            readDatabase(cache_path, table_name);
        }
    }
}

int main(int argc, char *argv[])
{
    DIR *dp;
    struct dirent *ep;
    
    if ((dp = opendir("/mnt/SDCARD/Emu")) != NULL) {

        if (argc > 1) {
            // read database from a specific emu
            readConfigFile(argv[1]);
        } 
        else {
            // read database from all cache dbs
            while ((ep = readdir(dp))) {
                if (ep->d_type != DT_DIR) continue;
                readConfigFile(ep->d_name);
            }
        }
        closedir(dp);
	}
	else {
		perror("Emu folder does not exists");
	}
    
    if (total_games <= 0) return ERROR_CODE_NO_GAME_FOUND;

    int random_number;
    srand(time(NULL));
    random_number = rand() % total_games;

    char emu_path[STR_MAX];
    concat(emu_path, all_games[random_number].emupath, "/launch.sh");

    char cmd_to_run[STR_MAX];
    snprintf(cmd_to_run, STR_MAX * 3, "%s \"%s\" \"%s\"", "LD_PRELOAD=/mnt/SDCARD/miyoo/app/../lib/libpadsp.so", emu_path, all_games[random_number].rompath);
    
    FILE *fp;
    file_put_sync(fp, "/mnt/SDCARD/.tmp_update/cmd_to_run.sh", "%s", cmd_to_run);

    printf_debug("cmd_to_run: %s\n", cmd_to_run);

    fflush(stdin);
    return EXIT_SUCCESS;
}