#ifndef TWEAKS_SCREEN_TIME_UI_H__
#define TWEAKS_SCREEN_TIME_UI_H__

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "components/kbinput_wrapper.h"
#include "components/list.h"

#include "../screenTime/screenTime.h"
#include "./appstate.h"
#include "./info_dialog.h"

static const int SCREEN_TIME_DAILY_LIMITS[] = {0, 15, 30, 45, 60, 90, 120, 180, 240, 300, 360};
static const int SCREEN_TIME_EXTRA_OPTIONS[] = {0, 5, 10, 15, 30, 45, 60};
#define SCREEN_TIME_DURATION_LABEL_LEN 32

static bool screen_time_settings_unlocked = false;

static int screen_time_value_for_minutes(const int *values, int count, int minutes)
{
    for (int i = 0; i < count; i++) {
        if (values[i] == minutes)
            return i;
    }

    return 0;
}

static void screen_time_format_minutes(int64_t minutes, char *out_label, size_t out_label_size)
{
    if (minutes <= 0) {
        snprintf(out_label, out_label_size, "Off");
        return;
    }

    int64_t hours = minutes / 60;
    int64_t mins = minutes % 60;
    if (hours == 0)
        snprintf(out_label, out_label_size, "%lldm", (long long)mins);
    else if (mins == 0)
        snprintf(out_label, out_label_size, "%lldh", (long long)hours);
    else
        snprintf(out_label, out_label_size, "%lldh %lldm", (long long)hours, (long long)mins);
}

static void screen_time_format_duration(int64_t seconds, char *out_label, size_t out_label_size)
{
    if (seconds <= 0) {
        snprintf(out_label, out_label_size, "0m");
        return;
    }

    int64_t minutes = (seconds + 59) / 60;
    screen_time_format_minutes(minutes, out_label, out_label_size);
}

static void screen_time_status_labels(char *used_label, char *remaining_label)
{
    ScreenTimeStatus status;
    if (screen_time_get_status(&status) != 0) {
        strcpy(used_label, "Today used: unavailable");
        strcpy(remaining_label, "Time remaining: unavailable");
        return;
    }

    char duration[SCREEN_TIME_DURATION_LABEL_LEN];
    screen_time_format_duration(status.used_seconds, duration, sizeof(duration));
    snprintf(used_label, STR_MAX, "Today used: %s", duration);

    if (!status.enabled) {
        strcpy(remaining_label, "Time remaining: unlimited");
        return;
    }

    screen_time_format_duration(status.remaining_seconds, duration, sizeof(duration));
    snprintf(remaining_label, STR_MAX, "Time remaining: %s", duration);
}

static void screen_time_refresh_menu(void)
{
    if (!_menu_screen_time._created)
        return;

    ScreenTimeSettings settings;
    if (screen_time_load_settings(&settings) != 0)
        memset(&settings, 0, sizeof(settings));

    char today[SCREEN_TIME_DATE_LEN];
    screen_time_date_string(screen_time_now(), today, sizeof(today));
    int extra_minutes = strncmp(settings.extra_date, today, sizeof(settings.extra_date)) == 0 ? settings.extra_minutes : 0;

    bool pin_configured = screen_time_pin_configured();
    screen_time_settings_unlocked = !pin_configured || screen_time_settings_unlocked;

    strcpy(_menu_screen_time.items[0].label, screen_time_settings_unlocked ? "Settings unlocked" : "Unlock settings...");
    _menu_screen_time.items[0].disabled = !pin_configured;
    if (_menu_screen_time.items[0].disabled && _menu_screen_time.active_pos == 0)
        list_scrollTo(&_menu_screen_time, 1);

    _menu_screen_time.items[1].value = settings.enabled ? 1 : 0;
    _menu_screen_time.items[2].value = screen_time_value_for_minutes(
        SCREEN_TIME_DAILY_LIMITS,
        sizeof(SCREEN_TIME_DAILY_LIMITS) / sizeof(SCREEN_TIME_DAILY_LIMITS[0]),
        settings.daily_limit_minutes);
    _menu_screen_time.items[3].value = screen_time_value_for_minutes(
        SCREEN_TIME_EXTRA_OPTIONS,
        sizeof(SCREEN_TIME_EXTRA_OPTIONS) / sizeof(SCREEN_TIME_EXTRA_OPTIONS[0]),
        extra_minutes);

    for (int i = 1; i < 4; i++)
        _menu_screen_time.items[i]._reset_value = _menu_screen_time.items[i].value;

    strcpy(_menu_screen_time.items[4].label, pin_configured ? "PIN: Change..." : "PIN: Set...");
    screen_time_status_labels(_menu_screen_time.items[5].label, _menu_screen_time.items[6].label);
    list_changed = true;
}

static void screen_time_lock_settings(void)
{
    screen_time_settings_unlocked = false;
}

static const char *screen_time_request_pin(const char *title)
{
    const char *pin = launch_keyboard("", title);
    all_changed = true;
    return pin;
}

static bool screen_time_unlock_settings(void)
{
    if (!screen_time_pin_configured())
        return true;

    const char *pin = screen_time_request_pin("Screen time PIN");
    if (pin != NULL && screen_time_verify_pin(pin)) {
        screen_time_settings_unlocked = true;
        return true;
    }

    __showInfoDialog("Screen time", "Incorrect PIN.");
    return false;
}

static bool screen_time_require_unlocked(void)
{
    if (!screen_time_pin_configured() || screen_time_settings_unlocked)
        return true;

    __showInfoDialog("Screen time", "Unlock settings first.");
    return false;
}

static void action_screenTimeUnlock(void *pt)
{
    if (screen_time_settings_unlocked) {
        __showInfoDialog("Screen time", "Settings already unlocked.");
        screen_time_refresh_menu();
        return;
    }

    if (screen_time_unlock_settings())
        __showInfoDialog("Screen time", "Settings unlocked.");

    screen_time_refresh_menu();
}

static void formatter_screenTimeDailyLimit(void *pt, char *out_label)
{
    ListItem *item = (ListItem *)pt;
    int count = sizeof(SCREEN_TIME_DAILY_LIMITS) / sizeof(SCREEN_TIME_DAILY_LIMITS[0]);
    int value = item->value < count ? item->value : 0;
    screen_time_format_minutes(SCREEN_TIME_DAILY_LIMITS[value], out_label, STR_MAX);
}

static void formatter_screenTimeExtra(void *pt, char *out_label)
{
    ListItem *item = (ListItem *)pt;
    int count = sizeof(SCREEN_TIME_EXTRA_OPTIONS) / sizeof(SCREEN_TIME_EXTRA_OPTIONS[0]);
    int value = item->value < count ? item->value : 0;
    int minutes = SCREEN_TIME_EXTRA_OPTIONS[value];
    if (minutes == 0)
        strcpy(out_label, "Off");
    else
        sprintf(out_label, "+%dm", minutes);
}

static void action_screenTimeEnabled(void *pt)
{
    ListItem *item = (ListItem *)pt;
    ScreenTimeSettings settings;
    if (screen_time_load_settings(&settings) != 0)
        memset(&settings, 0, sizeof(settings));
    if (settings.enabled && item->value == 0 && !screen_time_require_unlocked()) {
        screen_time_refresh_menu();
        return;
    }

    screen_time_set_enabled(item->value == 1);
    screen_time_refresh_menu();
}

static void action_screenTimeDailyLimit(void *pt)
{
    ListItem *item = (ListItem *)pt;
    int count = sizeof(SCREEN_TIME_DAILY_LIMITS) / sizeof(SCREEN_TIME_DAILY_LIMITS[0]);
    int value = item->value < count ? item->value : 0;
    int minutes = SCREEN_TIME_DAILY_LIMITS[value];

    ScreenTimeSettings settings;
    if (screen_time_load_settings(&settings) != 0)
        memset(&settings, 0, sizeof(settings));
    bool disables_limit = settings.daily_limit_minutes > 0 && minutes == 0;
    bool increases_limit = minutes > settings.daily_limit_minutes;
    if ((disables_limit || increases_limit) && !screen_time_require_unlocked()) {
        screen_time_refresh_menu();
        return;
    }

    screen_time_set_daily_limit_minutes(minutes);
    screen_time_refresh_menu();
}

static void action_screenTimeExtra(void *pt)
{
    ListItem *item = (ListItem *)pt;
    int count = sizeof(SCREEN_TIME_EXTRA_OPTIONS) / sizeof(SCREEN_TIME_EXTRA_OPTIONS[0]);
    int value = item->value < count ? item->value : 0;
    int minutes = SCREEN_TIME_EXTRA_OPTIONS[value];

    ScreenTimeSettings settings;
    if (screen_time_load_settings(&settings) != 0)
        memset(&settings, 0, sizeof(settings));
    char today[SCREEN_TIME_DATE_LEN];
    screen_time_date_string(screen_time_now(), today, sizeof(today));
    int extra_minutes = strncmp(settings.extra_date, today, sizeof(settings.extra_date)) == 0 ? settings.extra_minutes : 0;
    if (minutes > extra_minutes && !screen_time_require_unlocked()) {
        screen_time_refresh_menu();
        return;
    }

    screen_time_set_extra_minutes(minutes);
    screen_time_refresh_menu();
}

static void action_screenTimePin(void *pt)
{
    if (screen_time_pin_configured() && !screen_time_require_unlocked()) {
        screen_time_refresh_menu();
        return;
    }

    bool pin_was_configured = screen_time_pin_configured();
    const char *pin = screen_time_request_pin(pin_was_configured ? "New screen time PIN" : "Set screen time PIN");
    if (pin == NULL) {
        screen_time_refresh_menu();
        return;
    }

    char new_pin[STR_MAX];
    strncpy(new_pin, pin, STR_MAX - 1);
    new_pin[STR_MAX - 1] = '\0';

    if (new_pin[0] != '\0') {
        const char *confirmed_pin = screen_time_request_pin("Confirm screen time PIN");
        if (confirmed_pin == NULL) {
            screen_time_refresh_menu();
            return;
        }

        if (strcmp(new_pin, confirmed_pin) != 0) {
            __showInfoDialog("Screen time", "PINs do not match.");
            screen_time_refresh_menu();
            return;
        }
    }
    else if (!pin_was_configured) {
        __showInfoDialog("Screen time", "PIN not set.");
        screen_time_refresh_menu();
        return;
    }

    screen_time_set_pin(new_pin);
    screen_time_settings_unlocked = new_pin[0] != '\0';
    __showInfoDialog("Screen time", new_pin[0] == '\0' ? "PIN cleared." : "PIN saved.");
    screen_time_refresh_menu();
}

void menu_screenTime(void *_)
{
    screen_time_lock_settings();

    if (!_menu_screen_time._created) {
        _menu_screen_time = list_createWithTitle(7, LIST_SMALL, "Screen time");
        list_addItemWithInfoNote(&_menu_screen_time,
                                 (ListItem){
                                     .label = "Unlock settings...",
                                     .action = action_screenTimeUnlock},
                                 "Unlock protected settings until you\n"
                                 "leave this Screen Time menu.");
        list_addItemWithInfoNote(&_menu_screen_time,
                                 (ListItem){
                                     .label = "State",
                                     .item_type = TOGGLE,
                                     .value = 0,
                                     .action = action_screenTimeEnabled},
                                 "Enable or disable daily play time limits.");
        list_addItemWithInfoNote(&_menu_screen_time,
                                 (ListItem){
                                     .label = "Daily limit",
                                     .item_type = MULTIVALUE,
                                     .value_max = sizeof(SCREEN_TIME_DAILY_LIMITS) / sizeof(SCREEN_TIME_DAILY_LIMITS[0]) - 1,
                                     .value_formatter = formatter_screenTimeDailyLimit,
                                     .action = action_screenTimeDailyLimit},
                                 "Set the total play time allowed today.");
        list_addItemWithInfoNote(&_menu_screen_time,
                                 (ListItem){
                                     .label = "Extra time today",
                                     .item_type = MULTIVALUE,
                                     .value_max = sizeof(SCREEN_TIME_EXTRA_OPTIONS) / sizeof(SCREEN_TIME_EXTRA_OPTIONS[0]) - 1,
                                     .value_formatter = formatter_screenTimeExtra,
                                     .action = action_screenTimeExtra},
                                 "Grant temporary extra time for today.");
        list_addItemWithInfoNote(&_menu_screen_time,
                                 (ListItem){
                                     .label = "PIN: Set...",
                                     .action = action_screenTimePin},
                                 "Set, change, or clear the PIN used\n"
                                 "to protect screen time changes.");
        list_addItem(&_menu_screen_time,
                     (ListItem){
                         .label = "Today used: ...",
                         .disabled = 1,
                         .show_opaque = 1,
                         .action = NULL});
        list_addItem(&_menu_screen_time,
                     (ListItem){
                         .label = "Time remaining: ...",
                         .disabled = 1,
                         .show_opaque = 1,
                         .action = NULL});
    }

    screen_time_refresh_menu();
    menu_stack[++menu_level] = &_menu_screen_time;
    header_changed = true;
}

#endif // TWEAKS_SCREEN_TIME_UI_H__
