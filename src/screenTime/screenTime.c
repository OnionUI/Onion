#include "screenTime.h"

#include <errno.h>
#include <inttypes.h>
#include <limits.h>
#include <sqlite3/sqlite3.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#define SCREEN_TIME_ENV_CONFIG_DIR "SCREEN_TIME_CONFIG_DIR"
#define SCREEN_TIME_ENV_ACTIVITY_DB "SCREEN_TIME_ACTIVITY_DB"
#define SCREEN_TIME_ENV_NOW "SCREEN_TIME_NOW"
#define SCREEN_TIME_ENV_BOOT_TIME "SCREEN_TIME_BOOT_TIME"
#define SCREEN_TIME_PIN_HASH_PREFIX "v1:"
#define SCREEN_TIME_PIN_HASH_SALT "OnionOS-screen-time"

static void screen_time_config_path(char *out, size_t out_size, const char *name)
{
    snprintf(out, out_size, "%s/%s", screen_time_config_dir(), name);
}

static bool read_int_file(const char *path, int *value)
{
    FILE *fp = fopen(path, "r");
    if (fp == NULL)
        return false;

    int read_count = fscanf(fp, "%d", value);
    fclose(fp);
    return read_count == 1;
}

static bool read_string_file(const char *path, char *out, size_t out_size)
{
    FILE *fp = fopen(path, "r");
    if (fp == NULL)
        return false;

    if (fgets(out, (int)out_size, fp) == NULL) {
        fclose(fp);
        return false;
    }

    out[strcspn(out, "\r\n")] = '\0';
    fclose(fp);
    return true;
}

static bool read_int64_file(const char *path, int64_t *value)
{
    char buffer[64];
    if (value == NULL || !read_string_file(path, buffer, sizeof(buffer)))
        return false;

    char *end = NULL;
    errno = 0;
    long long parsed = strtoll(buffer, &end, 10);
    if (errno != 0 || end == buffer)
        return false;

    *value = parsed;
    return true;
}

static bool is_regular_file(const char *path)
{
    struct stat buffer;
    return stat(path, &buffer) == 0 && S_ISREG(buffer.st_mode);
}

static time_t screen_time_boot_time(void)
{
    const char *boot_time_str = getenv(SCREEN_TIME_ENV_BOOT_TIME);
    if (boot_time_str != NULL && boot_time_str[0] != '\0') {
        char *end = NULL;
        errno = 0;
        long long parsed = strtoll(boot_time_str, &end, 10);
        if (errno == 0 && end != boot_time_str)
            return (time_t)parsed;
    }

    FILE *fp = fopen("/proc/stat", "r");
    if (fp == NULL)
        return 0;

    char line[128];
    long long value = 0;
    while (fgets(line, sizeof(line), fp) != NULL) {
        if (sscanf(line, "btime %lld", &value) == 1) {
            fclose(fp);
            return (time_t)value;
        }
    }

    fclose(fp);
    return 0;
}

static int ensure_dir(const char *dir_path)
{
    if (dir_path == NULL || dir_path[0] == '\0')
        return -1;

    char path[PATH_MAX];
    snprintf(path, sizeof(path), "%s", dir_path);

    size_t len = strlen(path);
    if (len == 0)
        return -1;

    if (path[len - 1] == '/')
        path[len - 1] = '\0';

    for (char *p = path + 1; *p != '\0'; p++) {
        if (*p != '/')
            continue;

        *p = '\0';
        if (mkdir(path, 0777) != 0 && errno != EEXIST)
            return -1;
        *p = '/';
    }

    if (mkdir(path, 0777) != 0 && errno != EEXIST)
        return -1;

    return 0;
}

static int write_int_file(const char *path, int value)
{
    if (ensure_dir(screen_time_config_dir()) != 0)
        return -1;

    FILE *fp = fopen(path, "w+");
    if (fp == NULL)
        return -1;

    fprintf(fp, "%d", value);
    fflush(fp);
    fsync(fileno(fp));
    fclose(fp);
    return 0;
}

static int write_string_file(const char *path, const char *value)
{
    if (ensure_dir(screen_time_config_dir()) != 0)
        return -1;

    FILE *fp = fopen(path, "w+");
    if (fp == NULL)
        return -1;

    fprintf(fp, "%s", value);
    fflush(fp);
    fsync(fileno(fp));
    fclose(fp);
    return 0;
}

static uint64_t fnv1a64_update(uint64_t hash, const char *value)
{
    const unsigned char *p = (const unsigned char *)value;
    while (*p != '\0') {
        hash ^= *p++;
        hash *= 1099511628211ULL;
    }
    return hash;
}

static void hash_pin(const char *pin, char *out, size_t out_size)
{
    uint64_t hash = 14695981039346656037ULL;
    hash = fnv1a64_update(hash, SCREEN_TIME_PIN_HASH_SALT);
    hash = fnv1a64_update(hash, ":");
    hash = fnv1a64_update(hash, pin == NULL ? "" : pin);
    snprintf(out, out_size, SCREEN_TIME_PIN_HASH_PREFIX "%016llx", (unsigned long long)hash);
}

const char *screen_time_config_dir(void)
{
    const char *config_dir = getenv(SCREEN_TIME_ENV_CONFIG_DIR);
    return config_dir != NULL && config_dir[0] != '\0' ? config_dir : SCREEN_TIME_DEFAULT_CONFIG_DIR;
}

const char *screen_time_activity_db_path(void)
{
    const char *db_path = getenv(SCREEN_TIME_ENV_ACTIVITY_DB);
    return db_path != NULL && db_path[0] != '\0' ? db_path : SCREEN_TIME_DEFAULT_ACTIVITY_DB;
}

time_t screen_time_now(void)
{
    const char *now_str = getenv(SCREEN_TIME_ENV_NOW);
    if (now_str != NULL && now_str[0] != '\0') {
        char *end = NULL;
        errno = 0;
        long long parsed = strtoll(now_str, &end, 10);
        if (errno == 0 && end != now_str)
            return (time_t)parsed;
    }

    return time(NULL);
}

void screen_time_today_bounds(time_t now, time_t *start_out, time_t *end_out)
{
    struct tm local_now;
    localtime_r(&now, &local_now);

    local_now.tm_hour = 0;
    local_now.tm_min = 0;
    local_now.tm_sec = 0;
    local_now.tm_isdst = -1;
    time_t start = mktime(&local_now);

    local_now.tm_mday += 1;
    local_now.tm_isdst = -1;
    time_t end = mktime(&local_now);

    if (start_out != NULL)
        *start_out = start;
    if (end_out != NULL)
        *end_out = end;
}

void screen_time_date_string(time_t now, char *out, size_t out_size)
{
    struct tm local_now;
    localtime_r(&now, &local_now);
    strftime(out, out_size, "%Y-%m-%d", &local_now);
}

int screen_time_load_settings(ScreenTimeSettings *settings)
{
    if (settings == NULL)
        return -1;

    memset(settings, 0, sizeof(ScreenTimeSettings));

    char path[PATH_MAX];
    int int_value = 0;

    screen_time_config_path(path, sizeof(path), "enabled");
    settings->enabled = read_int_file(path, &int_value) && int_value != 0;

    screen_time_config_path(path, sizeof(path), "dailyLimitMinutes");
    if (read_int_file(path, &settings->daily_limit_minutes) && settings->daily_limit_minutes < 0)
        settings->daily_limit_minutes = 0;

    screen_time_config_path(path, sizeof(path), "extraDate");
    read_string_file(path, settings->extra_date, sizeof(settings->extra_date));

    screen_time_config_path(path, sizeof(path), "extraMinutes");
    if (read_int_file(path, &settings->extra_minutes) && settings->extra_minutes < 0)
        settings->extra_minutes = 0;

    screen_time_config_path(path, sizeof(path), "warningSeconds");
    if (read_int_file(path, &settings->warning_seconds) && settings->warning_seconds < 0)
        settings->warning_seconds = 0;

    return 0;
}

int screen_time_set_enabled(bool enabled)
{
    char path[PATH_MAX];
    screen_time_config_path(path, sizeof(path), "enabled");
    return write_int_file(path, enabled ? 1 : 0);
}

int screen_time_set_daily_limit_minutes(int minutes)
{
    if (minutes < 0)
        return -1;

    char path[PATH_MAX];
    screen_time_config_path(path, sizeof(path), "dailyLimitMinutes");
    return write_int_file(path, minutes);
}

int screen_time_set_extra_minutes(int minutes)
{
    if (minutes < 0)
        return -1;

    char today[SCREEN_TIME_DATE_LEN];
    screen_time_date_string(screen_time_now(), today, sizeof(today));

    char path[PATH_MAX];
    screen_time_config_path(path, sizeof(path), "extraDate");
    if (write_string_file(path, today) != 0)
        return -1;

    screen_time_config_path(path, sizeof(path), "extraMinutes");
    return write_int_file(path, minutes);
}

int screen_time_add_extra_minutes(int minutes)
{
    if (minutes < 0)
        return -1;

    ScreenTimeSettings settings;
    if (screen_time_load_settings(&settings) != 0)
        return -1;

    char today[SCREEN_TIME_DATE_LEN];
    screen_time_date_string(screen_time_now(), today, sizeof(today));

    int extra_minutes = settings.extra_minutes;
    if (strncmp(settings.extra_date, today, sizeof(settings.extra_date)) != 0)
        extra_minutes = 0;
    extra_minutes += minutes;

    char path[PATH_MAX];
    screen_time_config_path(path, sizeof(path), "extraDate");
    if (write_string_file(path, today) != 0)
        return -1;

    screen_time_config_path(path, sizeof(path), "extraMinutes");
    return write_int_file(path, extra_minutes);
}

int screen_time_clear_extra_minutes(void)
{
    char today[SCREEN_TIME_DATE_LEN];
    screen_time_date_string(screen_time_now(), today, sizeof(today));

    char path[PATH_MAX];
    screen_time_config_path(path, sizeof(path), "extraDate");
    if (write_string_file(path, today) != 0)
        return -1;

    screen_time_config_path(path, sizeof(path), "extraMinutes");
    return write_int_file(path, 0);
}

int screen_time_debug_set_remaining_seconds(int seconds)
{
    if (seconds < 0)
        return -1;

    char path[PATH_MAX];
    char value[64];
    screen_time_config_path(path, sizeof(path), "debugExpireAt");
    snprintf(value, sizeof(value), "%lld", (long long)screen_time_now() + seconds);
    return write_string_file(path, value);
}

int screen_time_debug_clear_remaining(void)
{
    char path[PATH_MAX];
    screen_time_config_path(path, sizeof(path), "debugExpireAt");
    unlink(path);
    return 0;
}

bool screen_time_pin_configured(void)
{
    char path[PATH_MAX];
    char stored_hash[64];
    screen_time_config_path(path, sizeof(path), "pinHash");
    return read_string_file(path, stored_hash, sizeof(stored_hash)) && stored_hash[0] != '\0';
}

int screen_time_set_pin(const char *pin)
{
    char path[PATH_MAX];
    screen_time_config_path(path, sizeof(path), "pinHash");

    if (pin == NULL || pin[0] == '\0') {
        unlink(path);
        return 0;
    }

    char hash[64];
    hash_pin(pin, hash, sizeof(hash));
    return write_string_file(path, hash);
}

bool screen_time_verify_pin(const char *pin)
{
    char path[PATH_MAX];
    char stored_hash[64];
    screen_time_config_path(path, sizeof(path), "pinHash");
    if (!read_string_file(path, stored_hash, sizeof(stored_hash)) || stored_hash[0] == '\0')
        return true;

    char candidate_hash[64];
    hash_pin(pin, candidate_hash, sizeof(candidate_hash));
    return strcmp(stored_hash, candidate_hash) == 0;
}

int screen_time_get_usage_seconds(const char *db_path, time_t now, int64_t *used_seconds)
{
    if (used_seconds == NULL)
        return -1;

    *used_seconds = 0;

    if (db_path == NULL || !is_regular_file(db_path))
        return 0;

    time_t day_start;
    time_t day_end;
    screen_time_today_bounds(now, &day_start, &day_end);
    time_t boot_time = screen_time_boot_time();

    sqlite3 *db = NULL;
    int rc = sqlite3_open(db_path, &db);
    if (rc != SQLITE_OK) {
        sqlite3_close(db);
        return -1;
    }

    const char *sql =
        "SELECT created_at, "
        "       CASE WHEN play_time IS NULL THEN ?1 - created_at ELSE play_time END AS duration "
        "FROM play_activity "
        "WHERE created_at < ?2 "
        "  AND (created_at + play_time > ?3 "
        "       OR (play_time IS NULL AND (?4 = 0 OR created_at >= ?4)));";

    sqlite3_stmt *stmt = NULL;
    rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        sqlite3_close(db);
        return -1;
    }

    sqlite3_bind_int64(stmt, 1, (sqlite3_int64)now);
    sqlite3_bind_int64(stmt, 2, (sqlite3_int64)day_end);
    sqlite3_bind_int64(stmt, 3, (sqlite3_int64)day_start);
    sqlite3_bind_int64(stmt, 4, (sqlite3_int64)boot_time);

    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
        int64_t session_start = sqlite3_column_int64(stmt, 0);
        int64_t duration = sqlite3_column_int64(stmt, 1);
        if (duration <= 0)
            continue;

        int64_t session_end = session_start + duration;
        int64_t overlap_start = session_start > (int64_t)day_start ? session_start : (int64_t)day_start;
        int64_t overlap_end = session_end < (int64_t)day_end ? session_end : (int64_t)day_end;
        if (overlap_end > overlap_start)
            *used_seconds += overlap_end - overlap_start;
    }

    sqlite3_finalize(stmt);
    sqlite3_close(db);

    return rc == SQLITE_DONE ? 0 : -1;
}

int screen_time_get_status(ScreenTimeStatus *status)
{
    if (status == NULL)
        return -1;

    memset(status, 0, sizeof(ScreenTimeStatus));

    time_t now = screen_time_now();
    ScreenTimeSettings settings;
    if (screen_time_load_settings(&settings) != 0)
        return -1;

    int64_t used_seconds = 0;
    if (screen_time_get_usage_seconds(screen_time_activity_db_path(), now, &used_seconds) != 0)
        return -1;

    char today[SCREEN_TIME_DATE_LEN];
    screen_time_date_string(now, today, sizeof(today));

    int extra_minutes = 0;
    if (strncmp(settings.extra_date, today, sizeof(settings.extra_date)) == 0)
        extra_minutes = settings.extra_minutes;

    status->enabled = settings.enabled && settings.daily_limit_minutes > 0;
    status->used_seconds = used_seconds;
    status->limit_seconds = (int64_t)settings.daily_limit_minutes * 60;
    status->extra_seconds = (int64_t)extra_minutes * 60;
    status->remaining_seconds = status->limit_seconds + status->extra_seconds - used_seconds;
    if (!status->enabled)
        status->remaining_seconds = INT64_MAX;
    else {
        char path[PATH_MAX];
        int64_t debug_expire_at = 0;
        screen_time_config_path(path, sizeof(path), "debugExpireAt");
        if (read_int64_file(path, &debug_expire_at)) {
            status->remaining_seconds = debug_expire_at - (int64_t)now;
            if (status->remaining_seconds <= 0)
                screen_time_debug_clear_remaining();
        }
    }

    return 0;
}

bool screen_time_launch_allowed(ScreenTimeStatus *status)
{
    if (status == NULL)
        return true;
    return !status->enabled || status->remaining_seconds > 0;
}
