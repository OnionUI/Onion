#ifndef SETTINGS_H__
#define SETTINGS_H__

#include <stdbool.h>

#include "display.h"
#include "system/volume.h"
#include "utils/config.h"
#include "utils/file.h"
#include "utils/json.h"

#define MAX_BRIGHTNESS 10
#define MAIN_UI_SETTINGS "/mnt/SDCARD/system.json"
#define CMD_TO_RUN_PATH "/mnt/SDCARD/.tmp_update/cmd_to_run.sh"
#define RETROARCH_CONFIG "/mnt/SDCARD/RetroArch/.retroarch/retroarch.cfg"
#define HISTORY_PATH \
    "/mnt/SDCARD/Saves/CurrentProfile/lists/content_history.lpl"
#define RECENTLIST_PATH "/mnt/SDCARD/Roms/recentlist.json"
#define RECENTLIST_HIDDEN_PATH "/mnt/SDCARD/Roms/recentlist-hidden.json"
#define RECENTLISTMIGRATED "/mnt/SDCARD/Saves/CurrentProfile/config/.recentListMigrated"
#define DEFAULT_THEME_PATH "/mnt/SDCARD/Themes/Silky by DiMo/"
#define RECORDED_DIR "/mnt/SDCARD/Media/Videos/Recorded"

typedef struct settings_s {
    int volume;
    char keymap[JSON_STRING_LEN];
    int mute;
    bool bgm_mute;
    int bgm_volume;
    int brightness;
    char language[JSON_STRING_LEN];
    int sleep_timer;
    int lumination;
    int hue;
    int saturation;
    int contrast;
    int wifi_on;
    char theme[JSON_STRING_LEN];
    int fontsize;
    int audiofix;
    bool show_recents;
    bool show_expert;
    bool startup_auto_resume;
    bool menu_button_haptics;
    int low_battery_autosave_at;
    int low_battery_warn_at;
    int time_skip;
    int vibration;
    int startup_tab;
    int startup_application;
    int mainui_single_press;
    int mainui_long_press;
    int mainui_double_press;
    int ingame_single_press;
    int ingame_long_press;
    int ingame_double_press;
    bool disable_standby;
    int pwmfrequency;
    bool enable_logging;
    bool rec_indicator;
    bool rec_hotkey;
    int rec_countdown;
    int blue_light_state;
    int blue_light_schedule;
    int blue_light_level;
    int blue_light_rgb;
    char blue_light_time[16];
    char blue_light_time_off[16];
    bool rtc_available;
    int lid_close_action;

    char mainui_button_x[JSON_STRING_LEN];
    char mainui_button_y[JSON_STRING_LEN];
} settings_s;

static bool settings_loaded = false;
static settings_s settings;
static settings_s __settings;
static settings_s __default_settings = (settings_s){
    // MainUI settings
    .volume = 20,
    .keymap = "L2,L,R2,R,X,A,B,Y",
    .mute = 0,
    .bgm_volume = 20,
    .brightness = 7,
    .language = "en.lang",
    .sleep_timer = 5,
    .lumination = 7,
    .hue = 10,
    .saturation = 10,
    .contrast = 10,
    .theme = DEFAULT_THEME_PATH,
    .fontsize = 24,
    .audiofix = 1,
    .wifi_on = 0,
    // Onion settings
    .bgm_mute = false,
    .show_recents = false,
    .show_expert = false,
    .startup_auto_resume = true,
    .menu_button_haptics = false,
    .low_battery_autosave_at = 4,
    .low_battery_warn_at = 10,
    .time_skip = 4,
    .vibration = 2,
    .startup_tab = 0,
    .startup_application = 0,
    .rtc_available = false,
    // Menu button actions
    .mainui_single_press = 1,
    .mainui_long_press = 0,
    .mainui_double_press = 2,
    .ingame_single_press = 1,
    .ingame_long_press = 2,
    .ingame_double_press = 3,
    .disable_standby = false,
    .enable_logging = false,
    .blue_light_state = false,
    .blue_light_schedule = false,
    .blue_light_level = 0,
    .blue_light_rgb = 8421504,
    .blue_light_time = "20:00",
    .blue_light_time_off = "08:00",
    .pwmfrequency = 7,
    .lid_close_action = 0,
    .mainui_button_x = "",
    .mainui_button_y = "",
    //utility
    .rec_countdown = false,
    .rec_indicator = false,
    .rec_hotkey = false};

void _settings_clone(settings_s *dst, settings_s *src)
{
    *dst = *src;
    strcpy(dst->keymap, src->keymap);
    strcpy(dst->language, src->language);
    strcpy(dst->theme, src->theme);
    strcpy(dst->mainui_button_x, src->mainui_button_x);
    strcpy(dst->mainui_button_y, src->mainui_button_y);
}

void _settings_reset(settings_s *_settings)
{
    _settings_clone(_settings, &__default_settings);
}

void _settings_load_keymap(void)
{
    if (!exists(CONFIG_PATH "keymap.json"))
        return;

    cJSON *keymap = json_load(CONFIG_PATH "keymap.json");
    json_getInt(keymap, "mainui_single_press", &settings.mainui_single_press);
    json_getInt(keymap, "mainui_long_press", &settings.mainui_long_press);
    json_getInt(keymap, "mainui_double_press", &settings.mainui_double_press);
    json_getInt(keymap, "ingame_single_press", &settings.ingame_single_press);
    json_getInt(keymap, "ingame_long_press", &settings.ingame_long_press);
    json_getInt(keymap, "ingame_double_press", &settings.ingame_double_press);
    json_getString(keymap, "mainui_button_x", settings.mainui_button_x);
    json_getString(keymap, "mainui_button_y", settings.mainui_button_y);
    cJSON_Delete(keymap);
}

void _settings_load_mainui(void)
{
    char *json_str = NULL;

    if (!(json_str = file_read(MAIN_UI_SETTINGS)))
        return;

    cJSON *json_root = cJSON_Parse(json_str);
    free(json_str);

    json_getInt(json_root, "vol", &settings.volume);
    json_getInt(json_root, "bgmvol", &settings.bgm_volume);
    json_getInt(json_root, "brightness", &settings.brightness);
    json_getInt(json_root, "hibernate", &settings.sleep_timer);
    json_getInt(json_root, "lumination", &settings.lumination);
    json_getInt(json_root, "hue", &settings.hue);
    json_getInt(json_root, "saturation", &settings.saturation);
    json_getInt(json_root, "contrast", &settings.contrast);
    json_getInt(json_root, "fontsize", &settings.fontsize);
    json_getInt(json_root, "audiofix", &settings.audiofix);
    json_getInt(json_root, "wifi", &settings.wifi_on);

    json_getString(json_root, "keymap", settings.keymap);
    json_getString(json_root, "language", settings.language);
    json_getString(json_root, "theme", settings.theme);

    if (strcmp(settings.theme, "./") == 0) {
        strcpy(settings.theme, DEFAULT_THEME_PATH);
    }

    cJSON_Delete(json_root);
}

void settings_load(void)
{
    _settings_reset(&settings);

    settings.startup_auto_resume = !config_flag_get(".noAutoStart");
    settings.menu_button_haptics = !config_flag_get(".noMenuHaptics");
    settings.bgm_mute = config_flag_get(".bgmMute");
    settings.show_recents = config_flag_get(".showRecents");
    settings.show_expert = config_flag_get(".showExpert");
    settings.mute = config_flag_get(".muteVolume");
    settings.disable_standby = config_flag_get(".disableStandby");
    settings.enable_logging = config_flag_get(".logging");
    settings.blue_light_state = config_flag_get(".blfOn");
    settings.blue_light_schedule = config_flag_get(".blf");
    settings.rec_indicator = config_flag_get(".recIndicator");
    settings.rec_hotkey = config_flag_get(".recHotkey");
    settings.rtc_available = temp_flag_get("rtc_available");

    if (config_flag_get(".noLowBatteryAutoSave")) // flag is deprecated, but keep compatibility
        settings.low_battery_autosave_at = 0;

    if (config_flag_get(".noBatteryWarning")) // flag is deprecated, but keep compatibility
        settings.low_battery_warn_at = 0;

    if (config_flag_get(".noVibration")) // flag is deprecated, but keep compatibility
        settings.vibration = 0;

    config_get("battery/warnAt", CONFIG_INT, &settings.low_battery_warn_at);
    config_get("battery/exitAt", CONFIG_INT, &settings.low_battery_autosave_at);
    config_get("startup/app", CONFIG_INT, &settings.startup_application);
    config_get("startup/addHours", CONFIG_INT, &settings.time_skip);
    config_get("vibration", CONFIG_INT, &settings.vibration);
    config_get("startup/tab", CONFIG_INT, &settings.startup_tab);
    config_get("display/blueLightLevel", CONFIG_INT, &settings.blue_light_level);
    config_get("display/blueLightTime", CONFIG_STR, &settings.blue_light_time);
    config_get("display/blueLightTimeOff", CONFIG_STR, &settings.blue_light_time_off);
    config_get("display/blueLightRGB", CONFIG_INT, &settings.blue_light_rgb);
    config_get("pwmfrequency", CONFIG_INT, &settings.pwmfrequency);
    config_get("recCountdown", CONFIG_INT, &settings.rec_countdown);
    config_get("flip/lidCloseAction", CONFIG_INT, &settings.lid_close_action);

    if (config_flag_get(".menuInverted")) { // flag is deprecated, but keep compatibility
        settings.ingame_single_press = 2;
        settings.ingame_long_press = 1;
    }

    if (config_flag_get(".noGameSwitcher")) { // flag is deprecated, but keep compatibility
        settings.mainui_single_press = 0;
        settings.ingame_single_press = 2;
        settings.ingame_long_press = 0;
    }

    _settings_load_keymap();
    _settings_load_mainui();

    _settings_clone(&__settings, &settings);

    settings_loaded = true;
}

void _settings_save_keymap(void)
{
    FILE *fp;

    if ((fp = fopen(CONFIG_PATH "keymap.json", "w+")) == 0)
        return;

    fprintf(fp, "{\n");
    fprintf(fp, JSON_FORMAT_NUMBER, "mainui_single_press",
            settings.mainui_single_press);
    fprintf(fp, JSON_FORMAT_NUMBER, "mainui_long_press",
            settings.mainui_long_press);
    fprintf(fp, JSON_FORMAT_NUMBER, "mainui_double_press",
            settings.mainui_double_press);
    fprintf(fp, JSON_FORMAT_NUMBER, "ingame_single_press",
            settings.ingame_single_press);
    fprintf(fp, JSON_FORMAT_NUMBER, "ingame_long_press",
            settings.ingame_long_press);
    fprintf(fp, JSON_FORMAT_NUMBER, "ingame_double_press",
            settings.ingame_double_press);
    fprintf(fp, JSON_FORMAT_STRING, "mainui_button_x",
            settings.mainui_button_x);
    fprintf(fp, JSON_FORMAT_STRING_NC, "mainui_button_y",
            settings.mainui_button_y);
    fprintf(fp, "}\n");

    fflush(fp);
    fsync(fileno(fp));
    fclose(fp);
}

bool _settings_dirty_mainui(void)
{
    return settings.volume != __settings.volume ||
           strcmp(settings.keymap, __settings.keymap) != 0 ||
           settings.mute != __settings.mute ||
           settings.bgm_volume != __settings.bgm_volume ||
           settings.brightness != __settings.brightness ||
           strcmp(settings.language, __settings.language) != 0 ||
           settings.sleep_timer != __settings.sleep_timer ||
           settings.lumination != __settings.lumination ||
           settings.hue != __settings.hue ||
           settings.saturation != __settings.saturation ||
           settings.contrast != __settings.contrast ||
           strcmp(settings.theme, __settings.theme) != 0 ||
           settings.fontsize != __settings.fontsize ||
           settings.audiofix != __settings.audiofix ||
           settings.wifi_on != __settings.wifi_on;
}

void _settings_save_mainui(void)
{
    if (!_settings_dirty_mainui()) {
        print_debug("Skipped saving system.json (not dirty)");
        return;
    }

    FILE *fp;

    if ((fp = fopen(MAIN_UI_SETTINGS, "w+")) == NULL)
        return;

    fprintf(fp, "{\n");
    fprintf(fp, JSON_FORMAT_TAB_NUMBER, "vol", settings.volume);
    fprintf(fp, JSON_FORMAT_TAB_STRING, "keymap", settings.keymap);
    fprintf(fp, JSON_FORMAT_TAB_NUMBER, "mute", settings.mute);
    fprintf(fp, JSON_FORMAT_TAB_NUMBER, "bgmvol", settings.bgm_volume);
    fprintf(fp, JSON_FORMAT_TAB_NUMBER, "brightness", settings.brightness);
    fprintf(fp, JSON_FORMAT_TAB_STRING, "language", settings.language);
    fprintf(fp, JSON_FORMAT_TAB_NUMBER, "hibernate", settings.sleep_timer);
    fprintf(fp, JSON_FORMAT_TAB_NUMBER, "lumination", settings.lumination);
    fprintf(fp, JSON_FORMAT_TAB_NUMBER, "hue", settings.hue);
    fprintf(fp, JSON_FORMAT_TAB_NUMBER, "saturation", settings.saturation);
    fprintf(fp, JSON_FORMAT_TAB_NUMBER, "contrast", settings.contrast);
    fprintf(fp, JSON_FORMAT_TAB_STRING, "theme", settings.theme);
    fprintf(fp, JSON_FORMAT_TAB_NUMBER, "fontsize", settings.fontsize);
    fprintf(fp, JSON_FORMAT_TAB_NUMBER, "audiofix", settings.audiofix);
    fprintf(fp, JSON_FORMAT_TAB_NUMBER_NC, "wifi", settings.wifi_on);
    fprintf(fp, "}");

    fflush(fp);
    fsync(fileno(fp));
    fclose(fp);
}

void settings_save(void)
{
    config_flag_set(".noAutoStart", !settings.startup_auto_resume);
    config_flag_set(".noMenuHaptics", !settings.menu_button_haptics);
    config_flag_set(".bgmMute", settings.bgm_mute);
    config_flag_set(".showRecents", settings.show_recents);
    config_flag_set(".showExpert", settings.show_expert);
    config_flag_set(".muteVolume", settings.mute);
    config_flag_set(".disableStandby", settings.disable_standby);
    config_flag_set(".logging", settings.enable_logging);
    config_flag_set(".blfOn", settings.blue_light_state);
    config_flag_set(".blf", settings.blue_light_schedule);
    config_flag_set(".recIndicator", settings.rec_indicator);
    config_flag_set(".recHotkey", settings.rec_hotkey);
    config_setNumber("battery/warnAt", settings.low_battery_warn_at);
    config_setNumber("battery/exitAt", settings.low_battery_autosave_at);
    config_setNumber("startup/app", settings.startup_application);
    config_setNumber("startup/addHours", settings.time_skip);
    config_setNumber("vibration", settings.vibration);
    config_setNumber("startup/tab", settings.startup_tab);
    config_setNumber("recCountdown", settings.rec_countdown);
    config_setNumber("display/blueLightLevel", settings.blue_light_level);
    config_setNumber("display/blueLightRGB", settings.blue_light_rgb);
    config_setString("display/blueLightTime", settings.blue_light_time);
    config_setString("display/blueLightTimeOff", settings.blue_light_time_off);
    config_setNumber("flip/lidCloseAction", settings.lid_close_action);

    config_setNumber("pwmfrequency", settings.pwmfrequency);
    // remove deprecated flags
    remove(CONFIG_PATH ".noLowBatteryAutoSave");
    remove(CONFIG_PATH ".noBatteryWarning");
    remove(CONFIG_PATH ".noVibration");
    remove(CONFIG_PATH ".menuInverted");
    remove(CONFIG_PATH ".noGameSwitcher");

    _settings_save_keymap();
    _settings_save_mainui();

    temp_flag_set("settings_changed", true);
}

bool settings_saveSystemProperty(const char *prop_name, int value)
{
    cJSON *json_root = json_load(MAIN_UI_SETTINGS);
    cJSON *prop = cJSON_GetObjectItem(json_root, prop_name);

    if (cJSON_GetNumberValue(prop) == value)
        return false;

    cJSON_SetNumberValue(prop, value);
    json_save(json_root, MAIN_UI_SETTINGS);
    cJSON_Delete(json_root);
    temp_flag_set("settings_changed", true);

    return true;
}

void settings_setBrightness(uint32_t value, bool apply, bool save)
{
    settings.brightness = value;

    if (apply)
        display_setBrightness(settings.brightness);

    if (save)
        settings_saveSystemProperty("brightness", settings.brightness);
}

bool settings_setVolume(int value, bool apply)
{
    bool changed = false;

    if (value > 20)
        value = 20;
    else if (value < 0)
        value = 0;

    if (settings.volume != value) {
        settings.volume = value;
        changed = true;
    }

    if (apply)
        setVolume(settings.mute ? 0 : settings.volume);

    return changed;
}

bool settings_setMute(uint32_t value, bool apply)
{
    bool changed = false;

    if (settings.mute != value) {
        settings.mute = value;
        changed = true;
    }

    if (apply)
        setVolume(settings.mute ? 0 : settings.volume);

    return changed;
}

#endif // SETTINGS_H__
