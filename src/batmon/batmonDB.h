#ifndef BATMON_DB_H
#define BATMON_DB_H
#define BATTERY_LOG_FILE "/mnt/SDCARD/Saves/Common/battery_logs/battery_logs.sqlite"

sqlite3 *bat_log_db = NULL;

int open_battery_log_db(void)
{
    bool bat_log_db_created = is_file(BATTERY_LOG_FILE);

    if (!bat_log_db_created) {
        mkdir("/mnt/SDCARD/Saves/Common", 0777);
        mkdir("/mnt/SDCARD/Saves/Common/battery_logs", 0777);
    }

    if (sqlite3_open(BATTERY_LOG_FILE, &bat_log_db) != SQLITE_OK) {
        printf("%s\n", sqlite3_errmsg(bat_log_db));
        sqlite3_close(bat_log_db);
        bat_log_db = NULL;
        return -1;
    }

    if (!bat_log_db_created) {
        sqlite3_exec(bat_log_db,
                     "DROP TABLE IF EXISTS bat_activity;"
                     "CREATE TABLE bat_activity(id INTEGER PRIMARY KEY, device_serial TEXT, bat_level INTEGER, duration INTEGER, is_charging INTEGER);"
                     "CREATE INDEX bat_activity_device_SN_index ON bat_activity(device_serial);",
                     NULL, NULL, NULL);
        sqlite3_exec(bat_log_db,
                     "DROP TABLE IF EXISTS device_specifics;"
                     "CREATE TABLE device_specifics(id INTEGER PRIMARY KEY, device_serial TEXT, best_session INTEGER);"
                     "CREATE INDEX device_specifics_index ON device_specifics(device_serial);",
                     NULL, NULL, NULL);
    }

    return 1;
}

void close_battery_log_db(void)
{
    sqlite3_close(bat_log_db);
    bat_log_db = NULL;
}

int get_best_session_time(void)
{
    int best_time = 0;
    if (open_battery_log_db() == 1) {
        if (bat_log_db != NULL) {
            const char *sql = "SELECT * FROM device_specifics WHERE device_serial = ? ORDER BY id LIMIT 1;";
            sqlite3_stmt *stmt;

            int rc = sqlite3_prepare_v2(bat_log_db, sql, -1, &stmt, 0);
            if (rc == SQLITE_OK) {
                sqlite3_bind_text(stmt, 1, DEVICE_SN, -1, SQLITE_STATIC);
                rc = sqlite3_step(stmt);
                if (rc == SQLITE_ROW) {
                    best_time = sqlite3_column_int(stmt, 2);
                }
                else {
                    char *sql2 = sqlite3_mprintf("INSERT INTO device_specifics(device_serial, best_session) VALUES(%Q, %d);", DEVICE_SN, 0);
                    sqlite3_exec(bat_log_db, sql2, NULL, NULL, NULL);
                    sqlite3_free(sql2);
                }
            }
            sqlite3_finalize(stmt);
        }
    }
    return best_time;
}

#endif // BATMON_DB_H