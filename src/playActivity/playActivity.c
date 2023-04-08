#include "./playActivity.h"

sqlite3 *db;

void close_db(void) {
    printf_debug("%s\n", "close_db()");
    sqlite3_close(db);
    printf_debug("%s\n", "close_db() return");
}

void open_db(void) {
    printf_debug("%s\n", "open_db()");
    int rc = sqlite3_open(PLAY_ACTIVITY_SQLITE_PATH, &db);
    if (rc != SQLITE_OK) {
        printf_debug("Cannot open database: %s\n", sqlite3_errmsg(db));
        close_db();
    }
    printf_debug("%s\n", "open_db() return");
}

void create_table(void) {
    printf_debug("%s\n", "create_table()");
    char *sql = "DROP TABLE IF EXISTS play_activity;CREATE TABLE play_activity(name TEXT PRIMARY KEY, relative_path TEXT, play_count INTEGER, play_time INTEGER);CREATE UNIQUE INDEX name_index ON play_activity(name);";
    int rc = sqlite3_exec(db, sql, NULL, NULL, NULL);
    if (rc != SQLITE_OK) {
        printf_debug("Error: could not create table: %s\n", sqlite3_errmsg(db));
    }
    printf_debug("%s\n", "create_table() return");
}

void insert_data(const char *name, const char *relative_path, int play_count, int play_time) {
    printf_debug("insert_date(%s, %s, %d, %d)", name, relative_path, play_count, play_time);
    char *sql = sqlite3_mprintf("INSERT OR REPLACE INTO play_activities (name, relative_path, play_count, play_time) VALUES ('%q', '%q', COALESCE((SELECT play_count FROM playActivity WHERE name='%q'), 0) + %d, COALESCE((SELECT play_time FROM playActivity WHERE name='%q'), 0) + %d);", name, relative_path, name, play_count, name, play_time);
    int rc = sqlite3_exec(db, sql, NULL, NULL, NULL);
    if (rc != SQLITE_OK) {
        printf_debug("Error: could not insert play_activity: %s\n", sqlite3_errmsg(db));
        sqlite3_free(sql);
    }
    sqlite3_free(sql);
    printf_debug("%s\n", "insert_data() return");
}

void upgrade_rom_db(void) {
    printf_debug("%s\n", "upgrade_rom_db()");
    typedef struct {
        char *name;
        int play_time;
    } PlayActivity;
    FILE *file = fopen(PLAY_ACTIVITY_DB_PATH, "rb");
    if (file != NULL) {
        PlayActivity play_activity;
        while (fread(&play_activity, sizeof(PlayActivity), 1, file) == 1) {
            insert_data(play_activity.name, NULL, 1, play_activity.play_time);
        }
        fclose(file);
    }
    printf_debug("%s\n", "upgrade_rom_db() return");
}

void update_play_activity(const char *name, const char *relative_path)
{
    printf_debug("update_play_activity(%s, %s)\n", name, relative_path);
    FILE *file;
    time_t current_time;
    int play_time;
    file = fopen(INIT_TIMER_PATH, "rb");
    if (file == NULL) {
        file = fopen(INIT_TIMER_PATH, "wb");
        time(&current_time);
        play_time = (int)current_time;
        fwrite(&play_time, sizeof(int), 1, file);
        fclose(file);
    } else {
        fread(&play_time, sizeof(int), 1, file);
        time(&current_time);
        insert_data(name, relative_path, 1, play_time);
        play_time = (int)current_time;
        fseek(file, 0, SEEK_SET);
        fwrite(&play_time, sizeof(int), 1, file);
        fclose(file);
        remove(INIT_TIMER_PATH);
    }
    printf_debug("%s\n", "start_timer() return");
}

void usage(void) {
    printf_debug("%s\n", "main() argc <= 1");
}

int main(int argc, char *argv[])
{
    log_setName("playActivity");
    printf_debug("%s\n", "main()");
    if (!exists(PLAY_ACTIVITY_SQLITE_PATH)) {
        open_db();
        create_table();
        if (exists(PLAY_ACTIVITY_DB_PATH)) {
            upgrade_rom_db();
        }
    } else {
        open_db();
    }
    if (db == NULL) {
        printf_debug("%s\n", "db == NULL");
        return 1;
    }
    char file_path[256] = argv[1];
    char* roms_path = "../../ROMS/";
    char* file_name = strrchr(file_path, '/') + 1;
    char* extension = strrchr(file_path, '.');
    char relative_path[256];
    sprintf(relative_path, "%.*s", (int)(file_name - file_path - 1), file_path);

    if (strncmp(relative_path, roms_path, strlen(roms_path)) != 0) {
        printf_debug("%s must be within %s.\n", relative_path, roms_path);
        return 1;
    }
    update_play_activity(file_name, relative_path);
    close_db();
    printf_debug("main() return %d\n", EXIT_SUCCESS);
    return EXIT_SUCCESS;
}
