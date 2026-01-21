#include "sqldaemon.h"

void sql_shutdown();
int prepare_statements();

int init()
{

    bool play_activity_db_created = is_file(PLAY_ACTIVITY_DB_NEW_FILE);

    mkdir("/mnt/SDCARD/Saves/CurrentProfile/play_activity/", 0777);

    if (sqlite3_open_v2(PLAY_ACTIVITY_DB_NEW_FILE, &play_activity_db_writer, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL) != SQLITE_OK) {
        printf_debug("%s\n", sqlite3_errmsg(play_activity_db_writer));
        return 0;
    }

    if (!play_activity_db_created) {
        sqlite3_exec(play_activity_db_writer,
                     "DROP TABLE IF EXISTS rom;"
                     "CREATE TABLE rom(id INTEGER PRIMARY KEY, type TEXT, name TEXT, file_path TEXT, image_path TEXT, created_at INTEGER DEFAULT (strftime('%s', 'now')), updated_at INTEGER);"
                     "CREATE UNIQUE INDEX rom_id_index ON rom(id);",
                     NULL, NULL, NULL);

        sqlite3_exec(play_activity_db_writer,
                     "DROP TABLE IF EXISTS play_activity;"
                     "CREATE TABLE play_activity(rom_id INTEGER, play_time INTEGER, created_at INTEGER DEFAULT (strftime('%s', 'now')), updated_at INTEGER);"
                     "CREATE INDEX play_activity_rom_id_index ON play_activity(rom_id);",
                     NULL, NULL, NULL);
    }
    sqlite3_exec(play_activity_db_writer, "PRAGMA synchronous = FULL;", NULL, NULL, NULL);
    sqlite3_exec(play_activity_db_writer, "PRAGMA journal_mode=WAL;", NULL, NULL, NULL);
    sqlite3_busy_timeout(play_activity_db_writer, 5000);

    if (sqlite3_open_v2(PLAY_ACTIVITY_DB_NEW_FILE, &play_activity_db_reader, SQLITE_OPEN_READONLY, NULL) != SQLITE_OK) {
        printf_debug("%s\n", sqlite3_errmsg(play_activity_db_reader));
        play_activity_db_reader = NULL;
        return 0;
    }
    sqlite3_busy_timeout(play_activity_db_reader, 5000);
    return prepare_statements();
}

void sql_shutdown()
{

    int ret;
    do {
        ret = sqlite3_close(play_activity_db_writer);
        msleep(50);
    } while (ret == SQLITE_BUSY);
    play_activity_db_writer = NULL;

    do {
        ret = sqlite3_close(play_activity_db_reader);
        msleep(50);
    } while (ret == SQLITE_BUSY);
    play_activity_db_reader = NULL;
}

int prepare_statements()
{
    int ret = sqlite3_prepare_v2(play_activity_db_reader,
                                 "SELECT SUM(play_time_total) FROM (SELECT SUM(play_time) AS play_time_total FROM play_activity GROUP BY rom_id) "
                                 "WHERE play_time_total > 60;",
                                 -1, &total_play_time, NULL);
    if (ret != SQLITE_OK) {
        printf_debug("Failed to prepare statement: %s\n",
                     sqlite3_errmsg(play_activity_db_reader));
        return 0;
    }

    ret = sqlite3_prepare_v2(play_activity_db_reader,
                             "SELECT * FROM ("
                             "    SELECT rom.id, rom.type, rom.name, rom.file_path, "
                             "           COUNT(play_activity.ROWID) AS play_count_total, "
                             "           SUM(play_activity.play_time) AS play_time_total, "
                             "           SUM(play_activity.play_time)/COUNT(play_activity.ROWID) AS play_time_average, "
                             "           datetime(MIN(play_activity.created_at), 'unixepoch') AS first_played_at, "
                             "           datetime(MAX(play_activity.created_at), 'unixepoch') AS last_played_at "
                             "    FROM rom LEFT JOIN play_activity ON rom.id = play_activity.rom_id "
                             "    GROUP BY rom.id) "
                             "WHERE play_time_total > 60 "
                             "ORDER BY play_time_total DESC;",
                             -1, &find_all, NULL);
    if (ret != SQLITE_OK) {
        printf_debug("Failed to prepare statement: %s\n",
                     sqlite3_errmsg(play_activity_db_reader));
        return 0;
    }

    ret = sqlite3_prepare_v2(play_activity_db_writer, "INSERT INTO rom(type, name, file_path, image_path) VALUES(?,?,?,?)", -1, &insert_rom, NULL);
    if (ret != SQLITE_OK) {
        printf_debug("Failed to prepare statement: %s\n",
                     sqlite3_errmsg(play_activity_db_writer));
        return 0;
    }

    // Here we use writer because its dependent on the last insert
    ret = sqlite3_prepare_v2(play_activity_db_writer, "SELECT id FROM rom WHERE ROWID = last_insert_rowid()", -1, &rom_id, NULL);
    if (ret != SQLITE_OK) {
        printf_debug("Failed to prepare statement: %s\n",
                     sqlite3_errmsg(play_activity_db_writer));
        return 0;
    }

    ret = sqlite3_prepare_v2(play_activity_db_writer, "UPDATE rom SET type = ?, name = ?, file_path = ?, image_path = ? WHERE id = ?;", -1, &update_rom, NULL);
    if (ret != SQLITE_OK) {
        printf_debug("Failed to prepare statement: %s\n",
                     sqlite3_errmsg(play_activity_db_writer));
        return 0;
    }

    ret = sqlite3_prepare_v2(play_activity_db_reader, "SELECT id FROM rom WHERE (name=? OR name=?) AND type='ORPHAN' LIMIT 1;", -1, &orphan_rom, NULL);
    if (ret != SQLITE_OK) {
        printf_debug("Failed to prepare statement: %s\n",
                     sqlite3_errmsg(play_activity_db_reader));
        return 0;
    }

    ret = sqlite3_prepare_v2(play_activity_db_reader, "SELECT id FROM rom WHERE file_path=? LIMIT 1;", -1, &rom_by_path, NULL);
    if (ret != SQLITE_OK) {
        printf_debug("Failed to prepare statement: %s\n",
                     sqlite3_errmsg(play_activity_db_reader));
        return 0;
    }

    ret = sqlite3_prepare_v2(play_activity_db_reader, "SELECT SUM(play_time) FROM play_activity WHERE rom_id = ?;",
                             -1, &rom_playtime, NULL);
    if (ret != SQLITE_OK) {
        printf_debug("Failed to prepare statement: %s\n",
                     sqlite3_errmsg(play_activity_db_reader));
        return 0;
    }

    ret = sqlite3_prepare_v2(play_activity_db_reader, "SELECT * FROM play_activity WHERE rom_id = ? AND play_time IS NULL;", -1, &active_closed_activity, NULL);
    if (ret != SQLITE_OK) {
        printf_debug("Failed to prepare statement: %s\n",
                     sqlite3_errmsg(play_activity_db_reader));
        return 0;
    }
    ret = sqlite3_prepare_v2(play_activity_db_writer, "INSERT INTO play_activity(rom_id) VALUES(?);", -1, &activity_start, NULL);
    if (ret != SQLITE_OK) {
        printf_debug("Failed to prepare statement: %s\n",
                     sqlite3_errmsg(play_activity_db_writer));
        return 0;
    }

    ret = sqlite3_prepare_v2(play_activity_db_writer, "UPDATE play_activity SET play_time = (strftime('%s', 'now')) - created_at, updated_at = (strftime('%s', 'now')) WHERE rom_id = ? AND play_time IS NULL;", -1, &activity_stop, NULL);

    if (ret != SQLITE_OK) {
        printf_debug("Failed to prepare statement: %s\n",
                     sqlite3_errmsg(play_activity_db_writer));
        return 0;
    }
    ret = sqlite3_prepare_v2(play_activity_db_writer, "UPDATE play_activity SET play_time = (strftime('%s', 'now')) - created_at, updated_at = (strftime('%s', 'now')) WHERE play_time IS NULL;", -1, &activity_stop_all, NULL);
    if (ret != SQLITE_OK) {
        printf_debug("Failed to prepare statement: %s\n",
                     sqlite3_errmsg(play_activity_db_writer));
        return 0;
    }

    ret = sqlite3_prepare_v2(play_activity_db_writer, "DELETE FROM play_activity WHERE play_time < 0;", -1, &delete_null_entry, NULL);
    if (ret != SQLITE_OK) {
        printf_debug("Failed to prepare statement: %s\n",
                     sqlite3_errmsg(play_activity_db_writer));
        return 0;
    }
    ret = sqlite3_prepare_v2(play_activity_db_reader, "SELECT id, file_path FROM rom WHERE file_path LIKE '/mnt/SDCARD/%';", -1, &get_file_path, NULL);
    if (ret != SQLITE_OK) {
        printf_debug("Failed to prepare statement: %s\n",
                     sqlite3_errmsg(play_activity_db_reader));
        return 0;
    }

    ret = sqlite3_prepare_v2(play_activity_db_writer, "UPDATE rom SET file_path = ? WHERE id = ?;", -1, &update_file_path, NULL);
    if (ret != SQLITE_OK) {
        printf_debug("Failed to prepare statement: %s\n",
                     sqlite3_errmsg(play_activity_db_writer));
        return 0;
    }
    ret = sqlite3_prepare_v2(play_activity_db_writer, "UPDATE rom SET file_path = ?, type = ? WHERE id = ?;", -1, &update_file_path_cache, NULL);
    if (ret != SQLITE_OK) {
        printf_debug("Failed to prepare statement: %s\n",
                     sqlite3_errmsg(play_activity_db_writer));
        return 0;
    }
    return 1;
}
void strip_newline(char *str)
{
    if (!str)
        return;
    size_t len = strlen(str);
    while (len > 0 && (str[len - 1] == '\n' || str[len - 1] == '\r')) {
        str[len - 1] = '\0';
        len--;
    }
}
sqlite3_stmt *clone_statement(sqlite3_stmt *src, sqlite3 *db)
{
    sqlite3_stmt *new_stmt = NULL;

    const char *sql = sqlite3_sql(src);

    if (sqlite3_prepare_v2(db, sql, -1, &new_stmt, NULL) != SQLITE_OK) {
        return NULL;
    }

    return new_stmt;
}

void get_rom_image_path(char *rom_file, char *out_image_path)
{
    if (str_endsWith(rom_file, ".p8") || str_endsWith(rom_file, ".png")) {
        snprintf(out_image_path, STR_MAX - 1, "/mnt/SDCARD/Roms/%s", rom_file);
    }

    char *clean_rom_name = file_removeExtension(basename(rom_file));
    char *rom_folder = strtok(rom_file, "/");

    snprintf(out_image_path, STR_MAX - 1, "/mnt/SDCARD/Roms/%s/Imgs/%s.png", rom_folder, clean_rom_name);
    free(clean_rom_name);
}

int play_activity_db_transaction(int (*exec_transaction)(void))
{

    int retval;
    retval = exec_transaction();
    return retval;
}

// int play_activity_db_read(sqlite3_stmt *sql)
// {
//     printf_debug("play_activity_db_read(%s)\n", sqlite3_sql(sql));
//
//     int rc = sqlite3_step(stmt);
//
//     if (rc != SQLITE_ROW && rc != SQLITE_DONE)
//         printf_debug( "Error stepping statement: %s\n", sqlite3_errmsg(play_activity_db_reader));
//
//     return rc;
// }

int play_activity_db_execute(sqlite3_stmt *sql)
{
    printf_debug("play_activity_db_execute(%s)\n", sqlite3_sql(sql));

    int rc = sqlite3_exec(play_activity_db_writer, "BEGIN EXCLUSIVE;", NULL, NULL, NULL);
    if (rc != SQLITE_OK)
        return rc;
    rc = sqlite3_step(sql);
    if (rc != SQLITE_DONE) {
        sqlite3_exec(play_activity_db_writer, "ROLLBACK;", 0, 0, 0);
        return rc;
    }
    rc = sqlite3_exec(play_activity_db_writer, "COMMIT;", 0, 0, 0);
    if (rc != SQLITE_OK)
        sqlite3_exec(play_activity_db_writer, "ROLLBACK;", 0, 0, 0);

    return rc;
}

int play_activity_get_total_play_time(void)
{
    int total_play_time_int = 0;
    sqlite3_stmt *stmt = total_play_time;
    int ret = sqlite3_step(stmt);

    if (ret == SQLITE_ROW) {
        total_play_time_int = sqlite3_column_int(stmt, 0);
    }
    sqlite3_step(stmt); // Flush statement
    printf_debug("Play time for User is %d\n", total_play_time_int);

    return total_play_time_int;
}

PlayActivities *play_activity_find_all(void)
{
    PlayActivities *play_activities = NULL;
    sqlite3_stmt *stmt = find_all;

    int play_activity_count = 0;
    int ret = sqlite3_step(stmt);

    while (ret == SQLITE_ROW) {
        play_activity_count++;
        ret = sqlite3_step(stmt);
    }
    sqlite3_reset(stmt);

    play_activities = (PlayActivities *)malloc(sizeof(PlayActivities));
    play_activities->count = play_activity_count;
    play_activities->play_time_total = 0;
    play_activities->play_activity = (PlayActivity **)malloc(sizeof(PlayActivity *) * play_activities->count);

    for (int i = 0; i < play_activities->count; i++) {
        ret = sqlite3_step(stmt);

        if (ret != SQLITE_ROW)
            break;

        PlayActivity *entry = play_activities->play_activity[i] = (PlayActivity *)malloc(sizeof(PlayActivity));
        ROM *rom = play_activities->play_activity[i]->rom = (ROM *)malloc(sizeof(ROM));
        entry->first_played_at = NULL;
        entry->last_played_at = NULL;

        rom->id = sqlite3_column_int(stmt, 0);
        rom->type = strdup((const char *)sqlite3_column_text(stmt, 1));
        rom->name = strdup((const char *)sqlite3_column_text(stmt, 2));
        if (sqlite3_column_text(stmt, 3) != NULL) {
            rom->file_path = strdup((const char *)sqlite3_column_text(stmt, 3));
            rom->image_path = malloc(STR_MAX * sizeof(char));
            memset(rom->image_path, 0, STR_MAX);
            get_rom_image_path(rom->file_path, rom->image_path);
        }

        entry->play_count = sqlite3_column_int(stmt, 4);
        entry->play_time_total = sqlite3_column_int(stmt, 5);
        entry->play_time_average = sqlite3_column_int(stmt, 6);
        if (sqlite3_column_text(stmt, 8) != NULL) {
            entry->first_played_at = strdup((const char *)sqlite3_column_text(stmt, 7));
        }
        if (sqlite3_column_text(stmt, 9) != NULL) {
            entry->last_played_at = strdup((const char *)sqlite3_column_text(stmt, 8));
        }

        play_activities->play_time_total += entry->play_time_total;
    }

    sqlite3_reset(stmt);
    sqlite3_clear_bindings(stmt);

    return play_activities;
}

int __db_insert_rom(const char *rom_type, const char *rom_name, const char *file_path, const char *image_path)
{
    int rom_id = ROM_NOT_FOUND;

    char rel_path[PATH_MAX];
    __ensure_rel_path(rel_path, file_path);

    sqlite3_bind_text(insert_rom, 1, rom_type, -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(insert_rom, 2, rom_name, -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(insert_rom, 3, file_path, -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(insert_rom, 4, image_path, -1, SQLITE_TRANSIENT);

    int ret = play_activity_db_execute(insert_rom);

    sqlite3_stmt *stmt = insert_rom;
    ret = sqlite3_step(stmt);

    if (ret == SQLITE_ROW) {
        rom_id = sqlite3_column_int(stmt, 0);
    }
    sqlite3_reset(insert_rom);
    sqlite3_clear_bindings(insert_rom);

    return rom_id;
}

int __db_insert_rom_from_cache(CacheDBItem *cache_db_item)
{
    return __db_insert_rom(cache_db_item->cache_path, cache_db_item->name, cache_db_item->path, cache_db_item->imgpath);
}
void __db_update_rom(int rom_id, const char *rom_type, const char *rom_name, const char *file_path, const char *image_path)
{
    char rel_path[PATH_MAX];
    __ensure_rel_path(rel_path, file_path);
    sqlite3_stmt *stmt = clone_statement(update_rom, play_activity_db_writer);
    sqlite3_bind_text(stmt, 1, rom_type, -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, rom_name, -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, file_path, -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 4, image_path, -1, SQLITE_TRANSIENT);

    play_activity_db_execute(stmt);
    sqlite3_finalize(stmt);
}

void __db_update_rom_from_cache(int rom_id, CacheDBItem *cache_db_item)
{
    __db_update_rom(rom_id, cache_db_item->cache_path, cache_db_item->name, cache_db_item->path, cache_db_item->imgpath);
}

int __db_get_orphan_rom_id(const char *rom_path)
{
    int rom_id = ROM_NOT_FOUND;
    char *_file_name = strdup(rom_path);
    char *file_name = basename(_file_name);
    char *rom_name = file_removeExtension(file_name);

    sqlite3_stmt *stmt = clone_statement(orphan_rom, play_activity_db_reader);
    sqlite3_bind_text(stmt, 1, rom_name, -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, file_name, -1, SQLITE_TRANSIENT);

    free(rom_name);
    free(_file_name);
    int ret = sqlite3_step(stmt);

    if (ret == SQLITE_ROW) {
        rom_id = sqlite3_column_int(stmt, 0);
    }
    sqlite3_finalize(stmt);

    return rom_id;
}

int __db_get_rom_id_by_path(const char *rom_path)
{
    int rom_id = ROM_NOT_FOUND;

    char rel_path[PATH_MAX];
    __ensure_rel_path(rel_path, rom_path);

    sqlite3_stmt *stmt = clone_statement(rom_by_path, play_activity_db_reader);
    sqlite3_bind_text(stmt, 1, rel_path, -1, SQLITE_TRANSIENT);

    int ret = sqlite3_step(stmt);

    if (ret == SQLITE_ROW) {
        rom_id = sqlite3_column_int(stmt, 0);
    }
    sqlite3_finalize(stmt);

    return rom_id;
}

int __db_rom_find_by_file_path(const char *rom_path, bool create_or_update)
{
    printf_debug("rom_find_by_file_path('%s')\n", rom_path);

    bool update_orphan = false;
    int rom_id = __db_get_rom_id_by_path(rom_path);

    if (rom_id == ROM_NOT_FOUND) {
        rom_id = __db_get_orphan_rom_id(rom_path);
        if (rom_id != ROM_NOT_FOUND) {
            update_orphan = true;
        }
    }
    else if (create_or_update) {
        CacheDBItem *cache_db_item = cache_db_find(rom_path);
        if (cache_db_item != NULL) {
            __db_update_rom_from_cache(rom_id, cache_db_item);
            free(cache_db_item);
        }
    }

    if (update_orphan) {
        CacheDBItem *cache_db_item = cache_db_find(rom_path);

        if (cache_db_item != NULL) {
            __db_update_rom_from_cache(rom_id, cache_db_item);
            free(cache_db_item);
        }
        else {
            char *rom_name = file_removeExtension(file_basename(rom_path));
            __db_update_rom(rom_id, "", rom_name, rom_path, "");
            free(rom_name);
        }
    }
    else if (rom_id == ROM_NOT_FOUND && create_or_update) {
        CacheDBItem *cache_db_item = cache_db_find(rom_path);

        if (cache_db_item != NULL) {
            rom_id = __db_insert_rom_from_cache(cache_db_item);
            free(cache_db_item);
        }
        else {
            char *rom_name = file_removeExtension(file_basename(rom_path));
            rom_id = __db_insert_rom("", rom_name, rom_path, "");
            free(rom_name);
        }
    }

    return rom_id;
}
int play_activity_transaction_rom_find_by_file_path(const char *rom_path, bool create_or_update)
{
    int retval;

    retval = __db_rom_find_by_file_path(rom_path, create_or_update);
    return retval;
}

int play_activity_get_play_time(const char *rom_path)
{

    int play_time = 0;
    int rom_id = __db_rom_find_by_file_path(rom_path, false);
    if (rom_id != ROM_NOT_FOUND) {
        sqlite3_stmt *stmt = clone_statement(rom_playtime, play_activity_db_reader);
        sqlite3_bind_int(stmt, 1, rom_id);
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            play_time = sqlite3_column_int(stmt, 0);
        }
        sqlite3_finalize(stmt);
    }

    printf_debug("Play time for %s is %d\n", rom_path, play_time);
    return play_time;
}

bool _get_active_rom_path(char *rom_path_out)
{
    char *ptr;
    char cmd[STR_MAX] = "";

    FILE *fp;
    file_get(fp, CMD_TO_RUN, CONTENT_STR, cmd);

    if (strlen(cmd) == 0) {
        return false;
    }

    if ((ptr = strrchr(cmd, '"')) != NULL) {
        *ptr = '\0';
    }

    if ((ptr = strrchr(cmd, '"')) != NULL) {
        strncpy(rom_path_out, ptr + 1, STR_MAX);
        return true;
    }

    return false;
}

int __db_get_active_closed_activity(void)
{
    int rom_id = ROM_NOT_FOUND;

    char rom_path[STR_MAX];
    if (!_get_active_rom_path(rom_path)) {
        return ROM_NOT_FOUND;
    }

    printf_debug("Last closed active rom: %s\n", rom_path);

    if ((rom_id = __db_rom_find_by_file_path(rom_path, false)) == ROM_NOT_FOUND) {
        return ROM_NOT_FOUND;
    }

    sqlite3_bind_int(active_closed_activity, 1, rom_id);
    sqlite3_stmt *stmt = active_closed_activity;
    int ret = sqlite3_step(stmt);

    if (ret == SQLITE_ROW) { // Activity is not closed
        rom_id = ROM_NOT_FOUND;
    }
    sqlite3_reset(active_closed_activity);
    sqlite3_clear_bindings(active_closed_activity);

    return rom_id;
}

bool play_activity_start(char *rom_file_path)
{
    printf_debug("\n:: play_activity_start(%s)\n", rom_file_path);
    int rom_id = play_activity_transaction_rom_find_by_file_path(rom_file_path, true);
    if (rom_id == ROM_NOT_FOUND) {
        return false;
    }

    sqlite3_bind_int(activity_start, 1, rom_id);

    int ret = play_activity_db_execute(activity_start);
    sqlite3_reset(activity_start);
    sqlite3_clear_bindings(activity_start);
    if (ret != SQLITE_OK)
        return false;
    return true;
}

bool play_activity_resume(void)
{
    print_debug("\n:: play_activity_resume()\n");

    int rom_id = play_activity_db_transaction(__db_get_active_closed_activity);
    if (rom_id == ROM_NOT_FOUND) {
        print_debug("Error: no active rom\n");
        return false;
    }
    sqlite3_bind_int(activity_start, 1, rom_id);

    int ret = play_activity_db_execute(activity_start);
    sqlite3_reset(activity_start);
    sqlite3_clear_bindings(activity_start);
    if (ret != SQLITE_OK)
        return false;
    return true;
}

bool play_activity_stop(char *rom_file_path)
{
    printf_debug("\n:: play_activity_stop(%s)\n", rom_file_path);
    int rom_id = play_activity_transaction_rom_find_by_file_path(rom_file_path, false);
    if (rom_id == ROM_NOT_FOUND) {
        return false;
    }

    sqlite3_bind_int(activity_stop, 1, rom_id);
    int ret = play_activity_db_execute(activity_stop);
    sqlite3_reset(activity_stop);
    sqlite3_clear_bindings(activity_stop);
    if (ret != SQLITE_OK)
        return false;
    return true;
}
void play_activity_stop_all(void)
{
    print_debug("\n:: play_activity_stop_all()\n");
    play_activity_db_execute(activity_stop_all);
}

void play_activity_fix_paths(void)
{
    print_debug("\n:: play_activity_fix_paths()\n");

    sqlite3_stmt *stmt = get_file_path;
    int ret = sqlite3_step(stmt);

    while (ret == SQLITE_ROW) {
        int rom_id = sqlite3_column_int(stmt, 0);
        char file_path[PATH_MAX];
        strcpy(file_path, (const char *)sqlite3_column_text(stmt, 1));

        if (strlen(file_path) == 0) {
            ret = sqlite3_step(stmt);
            continue;
        }

        char cache_path[PATH_MAX];
        char cache_name[STR_MAX];
        int cache_version = cache_get_path(cache_path, cache_name, file_path);

        char rel_path[PATH_MAX];
        __ensure_rel_path(rel_path, file_path);

        sqlite3_stmt *sql;
        if (cache_version == CACHE_NOT_FOUND) {
            sql = update_file_path;
            sqlite3_bind_text(sql, 1, rel_path, -1, SQLITE_TRANSIENT);
            sqlite3_bind_int(sql, 2, rom_id);
        }
        else {
            sql = update_file_path_cache;
            sqlite3_bind_text(sql, 1, rel_path, -1, SQLITE_TRANSIENT);
            sqlite3_bind_text(sql, 2, cache_path, -1, SQLITE_TRANSIENT);
            sqlite3_bind_int(sql, 3, rom_id);
        }
        play_activity_db_execute(sql);
        sqlite3_reset(sql);
        sqlite3_clear_bindings(sql);
        ret = sqlite3_step(stmt);
    }
}

void free_play_activities(PlayActivities *pa_ptr)
{
    for (int i = 0; i < pa_ptr->count; i++) {
        free(pa_ptr->play_activity[i]->first_played_at);
        free(pa_ptr->play_activity[i]->last_played_at);
        free(pa_ptr->play_activity[i]->rom);
        free(pa_ptr->play_activity[i]);
    }
    free(pa_ptr->play_activity);
    free(pa_ptr);
}
void play_activity_list_all(void)
{
    print_debug("\n:: play_activity_list_all()");

    int total_play_time = play_activity_get_total_play_time();
    PlayActivities *pas = play_activity_find_all();

    print_debug("\n");

    for (int i = 0; i < pas->count; i++) {
        PlayActivity *entry = pas->play_activity[i];
        ROM *rom = entry->rom;
        char rom_name[STR_MAX];
        file_cleanName(rom_name, rom->name);
        char play_time[STR_MAX];
        str_serializeTime(play_time, entry->play_time_total);
        printf_debug("%03d: %s (%s) [%s]\n", i + 1, rom_name, play_time, rom->type);
    }

    char total_str[25];
    str_serializeTime(total_str, total_play_time);
    printf_debug("\nTotal: %s\n", total_str);

    free_play_activities(pas);
}

void write_fd(const char *msg, int fd)
{
    if (write(fd, msg, strlen(msg)) < 0)
        print_debug("Unable to write message to socket\n");

    fsync(fd);
}

static void handle_client(void *fd_ptr)
{
    int client_fd = *(int *)fd_ptr;
    free(fd_ptr);
    char buffer[BUF_SIZE];
    int bytes = read(client_fd, buffer, BUF_SIZE - 1);
    if (bytes <= 0) {
        close(client_fd);
        return;
    }
    printf_debug("Command received from client %s\n", buffer);
    buffer[bytes] = '\0';
    strip_newline(buffer);
    char *msg;

    if (strncmp(buffer, GET_PLAY_TIME_TOTAL, strlen(GET_PLAY_TIME_TOTAL)) == 0) {
        int play_time = play_activity_get_total_play_time();
        msg = (char *)(malloc(sizeof(char) * 25));
        if (!msg)
            print_debug("Unable to alloc string for play time\n");
        else if (snprintf(msg, 25, "%d", play_time) < 0)
            print_debug("Unable to transform int into string\n");
    }
    else if (strncmp(buffer, GET_PLAY_TIME, strlen(GET_PLAY_TIME)) == 0) {
        int play_time = play_activity_get_play_time(&buffer[strlen(GET_PLAY_TIME) + 1]);
        msg = (char *)(malloc(sizeof(char) * 25));
        if (!msg)
            print_debug("Unable to alloc string for play time\n");
        else if (snprintf(msg, 25, "%d", play_time) < 0) {
            print_debug("Unable to transform int into string\n");
        }
    }
    else if (strncmp(buffer, CLOSE_DB, strlen(CLOSE_DB)) == 0) {
        // execute_write(buffer + 6);
        msg = strdup("OK");
        quit = 1;
    }
    else
        msg = "Unknown command\n";

    printf_debug("Command sent to client %s\n", msg);
    write_fd(msg, client_fd);
    free(msg);

    close(client_fd);
}

int main(int argc, char *argv[])
{

    log_setName("sqldaemon");
    print_debug("\n\nDebug logging enabled");

    signal(SIGINT, sigHandler);
    signal(SIGTERM, sigHandler);
    quit = init();

    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == -1) {
        print_debug("Unable to alloc a socket for the play activity daemon\n");
        sql_shutdown();
        return EXIT_FAILURE;
    }

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT);
    addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(server_fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {

        print_debug("Unable to start bind socket of the play activity daemon\n");
        return EXIT_FAILURE;
    }
    if (listen(server_fd, 5) == -1) {
        print_debug("Unable to start listening\n");
        sql_shutdown();
        return EXIT_FAILURE;
    }

    printf_debug("PlayActivity SQLite daemon running on port %d\n", PORT);

    while (quit) {
        int client_fd = accept(server_fd, NULL, NULL);
        if (client_fd < 0)
            continue;

        pthread_t thread_id;
        int *fd_ptr = malloc(sizeof(int));
        if (!fd_ptr) {
            print_debug("Unable to alloc int for new thread");
            close(client_fd);
            continue;
        }
        *fd_ptr = client_fd;
        pthread_create(&thread_id, NULL, (void *)handle_client, (void *)fd_ptr);
        pthread_detach(thread_id);
    }

    if (server_fd >= 0) {
        close(server_fd);
    }
    sql_shutdown();
    return EXIT_SUCCESS;
}
