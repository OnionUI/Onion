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

void start_timer(const char *name, const char *file_path)
{
    printf_debug("start_timer(%s, %s)\n", name, file_path);
    FILE *file;
    long file_size;
    char *base_time;
    if ((file = fopen(INIT_TIMER_PATH, "rb")) == 0) {
        return;
    }
    fseek(file, 0L, SEEK_END);
    file_size = ftell(file);
    rewind(file);
    base_time = (char *)calloc(1, file_size + 1);
    if (!base_time) {
        fclose(file);
        printf_debug("%s\n", "base_time memory alloc fails");
        return;
    }
    if (1 != fread(base_time, file_size, 1, file)) {
        fclose(file);
        free(base_time);
        printf_debug("%s\n", "base_time memory alloc fails");
        return;
    }
    fclose(file);
    int timer_time = atoi(base_time);
    int current_time = (int)time(NULL);
    insert_data(name, file_path, 1, current_time - timer_time);
    remove(INIT_TIMER_PATH);
    free(base_time);
    printf_debug("%s\n", "start_timer() return");
}

void usage(void) {
    printf_debug("%s\n", "main() argc <= 1");
}

int main(int argc, char *argv[])
{
    log_setName("playActivity");
    printf_debug("%s\n", "main()");
    if (argc <= 1) {
        usage();
        return 1;
    }
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
    if (strcmp(argv[1], "init") == 0) {
        printf_debug("%s\n", "main() argv[1] = 'init'");
        int current_time = (int)time(NULL);
        char base_time[15];
        sprintf(base_time, "%d", current_time);
        printf_debug("%s\n", "main() init remove init timer");
        remove(INIT_TIMER_PATH);
        int init_fd;
        if ((init_fd = open(INIT_TIMER_PATH, O_CREAT | O_WRONLY)) > 0) {
            printf_debug("%s\n", "main() init write init fd");
            write(init_fd, base_time, strlen(base_time));
            close(init_fd);
            system("sync");
            printf_debug("%s\n", "main() init saved init fd");
        }
        printf_debug("main() init Timer initiated @ %d\n", current_time);
        printf_debug("main() init return %d\n", EXIT_SUCCESS);
        return EXIT_SUCCESS;
    }
    char *cmd = argv[1];
    if (strstr(cmd, "../../Roms/") != NULL) {
        printf_debug("%s\n", "main() cmd includes '../../Roms/'");
        char *relative_path = NULL;
        snprintf(relative_path, strlen(str_split(cmd, "../../Roms/")), "%s", str_split(cmd, "../../Roms/"));
        if (relative_path != NULL) {
            relative_path[strlen(relative_path) - 1] = 0;
            char *file_path = NULL;
            snprintf(file_path, strlen(relative_path)+19, "/mnt/SDCARD/Roms/./%s", relative_path);
            char *name_path = NULL;
            sprintf(name_path, "%s/.game_config/%s.name", dirname(file_path), basename(file_path));
            char *name = NULL;
            if (is_file(name_path)) {
                FILE *file;
                file_get(file, name_path, "%[^\n]", name);
            }
            if (strlen(name) == 0) {
                strncpy(name, file_removeExtension(basename(argv[1])), STR_MAX);
            }
            printf_debug("main() cmd = '%s'\n", cmd);
            printf_debug("main() name = '%s'\n", name);
            printf_debug("main() file_path = '%s'\n", file_path);
            printf_debug("main() relative_path = '%s'\n", relative_path);
            start_timer(name, relative_path);
        }
    }
    close_db();
    printf_debug("main() return %d\n", EXIT_SUCCESS);
    return EXIT_SUCCESS;
}
