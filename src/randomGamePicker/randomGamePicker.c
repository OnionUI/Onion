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

#define MAXGAMES 100

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
    
    const char *sql = sqlite3_mprintf("SELECT * FROM %q", table_name);
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
        printf("sqlite3_column_text(res, 3): %s\n", sqlite3_column_text(res, 2));

        cJSON_AddItemToObject(games, "rompath", cJSON_CreateString(sqlite3_column_text(res, 2)));
        out = cJSON_Print(games);
        
        printf("%s\n", out);

        JsonGameEntry jsonGameEntry = JsonGameEntry_fromJson(out);

        all_games[total_games] = jsonGameEntry;
        total_games++;
    }

    sqlite3_finalize(res);
    sqlite3_close(db);
    
    return true;
}

int main(int argc, char *argv[])
{
    DIR *dp;
    struct dirent *ep;
    char config_path[512];
    
    if ((dp = opendir("/mnt/SDCARD/Emu")) != NULL) {
        
        while ((ep = readdir(dp))) {
            if (ep->d_type != DT_DIR) continue;

            sprintf(config_path, "/mnt/SDCARD/Emu/%s/config.json", ep->d_name);
            
            if (exists(config_path)) {
                const char *config_str = file_read(config_path);
                printf("config_str: %s\n", config_str);
                
                JsonGameEntry config = JsonGameEntry_fromJson(config_str);
                printf("rompath: %s\n", config.rompath);
                
                char cache_path[STR_MAX * 3];
                snprintf(cache_path, STR_MAX * 3 - 1, "%s/%s/%s/%s_cache2.db", "/mnt/SDCARD/Emu", ep->d_name, config.rompath, basename(config.rompath));

                char table_name[STR_MAX];

                printf("cache_path: %s\n", cache_path);
                char *last = strrchr(config.rompath, '/');
                if (last != NULL) {                    
                    snprintf(table_name, STR_MAX, "%s_roms", last+1);
                    printf("table_name: '%s'\n", table_name);
                }       
                
                printf("cache_path: %s\n", cache_path);

                readDatabase(cache_path, table_name);

            }
        }
        closedir(dp);
	}
	else {
		perror("Emu folder does not exists");
	}

    int random_number;
    srand(time(NULL));
    random_number = rand() % total_games;

    printf("random_number: %d\n", random_number);

    char emu_path[STR_MAX];
    concat(emu_path, all_games[random_number].emupath, "/launch.sh");

    char cmd_to_run[STR_MAX];
    snprintf(cmd_to_run, STR_MAX, "%s \"%s\" \"%s\"", "LD_PRELOAD=/mnt/SDCARD/miyoo/app/../lib/libpadsp.so", emu_path, all_games[random_number].rompath);
    
    //LD_PRELOAD=/mnt/SDCARD/miyoo/app/../lib/libpadsp.so "/mnt/SDCARD/Emu/GBA/launch.sh" "/mnt/SDCARD/Emu/GBA/../../Roms/GBA/dragon.zip"
    FILE *fp;
    file_put_sync(fp, "/mnt/SDCARD/.tmp_update/cmd_to_run.sh", "%s", cmd_to_run);

    printf("cmd_to_run: %s\n", cmd_to_run);

    temp_flag_set("quick_switch", true);

    fflush(stdin);
    return EXIT_SUCCESS;
}