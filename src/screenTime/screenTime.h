#ifndef SCREEN_TIME_H
#define SCREEN_TIME_H

#include <stdbool.h>
#include <stdint.h>
#include <time.h>

#define SCREEN_TIME_DEFAULT_CONFIG_DIR "/mnt/SDCARD/.tmp_update/config/screenTime"
#define SCREEN_TIME_DEFAULT_ACTIVITY_DB "/mnt/SDCARD/Saves/CurrentProfile/play_activity/play_activity_db.sqlite"
#define SCREEN_TIME_DATE_LEN 11

typedef struct ScreenTimeSettings {
    bool enabled;
    int daily_limit_minutes;
    int extra_minutes;
    int warning_seconds;
    char extra_date[SCREEN_TIME_DATE_LEN];
} ScreenTimeSettings;

typedef struct ScreenTimeStatus {
    bool enabled;
    int64_t used_seconds;
    int64_t limit_seconds;
    int64_t extra_seconds;
    int64_t remaining_seconds;
} ScreenTimeStatus;

const char *screen_time_config_dir(void);
const char *screen_time_activity_db_path(void);
time_t screen_time_now(void);
void screen_time_today_bounds(time_t now, time_t *start_out, time_t *end_out);
void screen_time_date_string(time_t now, char *out, size_t out_size);

int screen_time_load_settings(ScreenTimeSettings *settings);
int screen_time_set_enabled(bool enabled);
int screen_time_set_daily_limit_minutes(int minutes);
int screen_time_set_extra_minutes(int minutes);
int screen_time_add_extra_minutes(int minutes);
int screen_time_clear_extra_minutes(void);
int screen_time_debug_set_remaining_seconds(int seconds);
int screen_time_debug_clear_remaining(void);
bool screen_time_pin_configured(void);
int screen_time_set_pin(const char *pin);
bool screen_time_verify_pin(const char *pin);
int screen_time_get_usage_seconds(const char *db_path, time_t now, int64_t *used_seconds);
int screen_time_get_status(ScreenTimeStatus *status);
bool screen_time_launch_allowed(ScreenTimeStatus *status);

#endif // SCREEN_TIME_H
