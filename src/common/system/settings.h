#ifndef SETTINGS_H__
#define SETTINGS_H__

#include <stdbool.h>

#include "display.h"
#include "system/volume.h"
#include "utils/config.h"
#include "utils/file.h"
#include "utils/json.h"

#define MAX_BRIGHTNESS 10
#define MAIN_UI_SETTINGS "/appconfigs/system.json"
#define CMD_TO_RUN_PATH "/mnt/SDCARD/.tmp_update/cmd_to_run.sh"
#define RETROARCH_CONFIG "/mnt/SDCARD/RetroArch/.retroarch/retroarch.cfg"
#define HISTORY_PATH \
    "/mnt/SDCARD/Saves/CurrentProfile/lists/content_history.lpl"
#define DEFAULT_THEME_PATH "/mnt/SDCARD/Themes/Silky by DiMo/"

static struct settings_s {
    int volume;
    char keymap[JSON_STRING_LEN];
    int mute;
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
    bool low_battery_autosave;
    bool low_battery_warning;
    bool smbd_state;
    bool http_state;
    bool ssh_state;
    bool ftp_state;
    bool telnet_state;
    bool hotspot_state;
    bool ntp_state;
    bool ntp_wait;
    bool auth_telnet_state;
    bool auth_ftp_state;
    bool auth_http_state;
    bool auth_ssh_state;
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

    char mainui_button_x[JSON_STRING_LEN];
    char mainui_button_y[JSON_STRING_LEN];
} settings;

static bool settings_loaded = false;

void _settings_reset(void)
{
    // MainUI settings
    settings.volume = 20;
    strcpy(settings.keymap, "L2,L,R2,R,X,A,B,Y");
    settings.mute = 0;
    settings.bgm_volume = 20;
    settings.brightness = 7;
    strcpy(settings.language, "en.lang");
    settings.sleep_timer = 5;
    settings.lumination = 7;
    settings.hue = 10;
    settings.saturation = 10;
    settings.contrast = 10;
    strcpy(settings.theme, DEFAULT_THEME_PATH);
    settings.fontsize = 24;
    settings.audiofix = 1;
    settings.wifi_on = 0;
    // Onion settings
    settings.show_recents = false;
    settings.show_expert = false;
    settings.startup_auto_resume = true;
    settings.menu_button_haptics = false;
    settings.low_battery_autosave = true;
    settings.low_battery_warning = true;
    settings.low_battery_warn_at = 10;
    settings.time_skip = 4;
    settings.vibration = 2;
    settings.startup_tab = 0;
    settings.startup_application = 0;
    settings.smbd_state = false;
    settings.http_state = false;
    settings.auth_http_state = false;
    settings.ssh_state = false;
    settings.ftp_state = false;
    settings.telnet_state = false;
    settings.hotspot_state = false;
    settings.ntp_state = false;
    settings.ntp_wait = false;
    settings.auth_ftp_state = false;
    settings.auth_ssh_state = false;
    settings.auth_telnet_state = false;
    // Menu button actions
    settings.mainui_single_press = 1;
    settings.mainui_long_press = 0;
    settings.mainui_double_press = 2;
    settings.ingame_single_press = 1;
    settings.ingame_long_press = 2;
    settings.ingame_double_press = 3;
    settings.disable_standby = false;
    memset(settings.mainui_button_x, 0, JSON_STRING_LEN);
    memset(settings.mainui_button_y, 0, JSON_STRING_LEN);
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
    cJSON_free(keymap);
}

void _settings_load_mainui(void)
{
    const char *json_str = NULL;

    if (!(json_str = file_read(MAIN_UI_SETTINGS)))
        return;

    cJSON *json_root = cJSON_Parse(json_str);

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

    cJSON_free(json_root);
}

void settings_load(void)
{
    _settings_reset();

    settings.startup_auto_resume = !config_flag_get(".noAutoStart");
    settings.menu_button_haptics = !config_flag_get(".noMenuHaptics");
    settings.show_recents = config_flag_get(".showRecents");
    settings.show_expert = config_flag_get(".showExpert");
    settings.low_battery_autosave = !config_flag_get(".noLowBatteryAutoSave");
    settings.smbd_state = config_flag_get(".smbdState");
    settings.http_state = config_flag_get(".httpState");
    settings.ssh_state = config_flag_get(".sshState");
    settings.telnet_state = config_flag_get(".telnetState");
    settings.ftp_state = config_flag_get(".ftpState");
    settings.hotspot_state = config_flag_get(".hotspotState");
    settings.ntp_state = config_flag_get(".ntpState");
    settings.ntp_wait = config_flag_get(".ntpWait");
    settings.auth_telnet_state = config_flag_get(".authtelnetState");
    settings.auth_ftp_state = config_flag_get(".authftpState");
    settings.auth_http_state = config_flag_get(".authhttpState");
    settings.auth_ssh_state = config_flag_get(".authsshState");
    settings.mute = config_flag_get(".muteVolume");
    settings.disable_standby = config_flag_get(".disableStandby");

    if (config_flag_get(
            ".noBatteryWarning")) // flag is deprecated, but keep compatibility
        settings.low_battery_warn_at = 0;

    if (config_flag_get(
            ".noVibration")) // flag is deprecated, but keep compatibility
        settings.vibration = 0;

    config_get("battery/warnAt", "%d", &settings.low_battery_warn_at);
    config_get("startup/app", "%d", &settings.startup_application);
    config_get("startup/addHours", "%d", &settings.time_skip);
    config_get("vibration", "%d", &settings.vibration);
    config_get("startup/tab", "%d", &settings.startup_tab);

    if (config_flag_get(
            ".menuInverted")) { // flag is deprecated, but keep compatibility
        settings.ingame_single_press = 2;
        settings.ingame_long_press = 1;
    }

    if (config_flag_get(
            ".noGameSwitcher")) { // flag is deprecated, but keep compatibility
        settings.mainui_single_press = 0;
        settings.ingame_single_press = 2;
        settings.ingame_long_press = 0;
    }

    _settings_load_keymap();
    _settings_load_mainui();

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

void _settings_save_mainui(void)
{
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
    config_flag_set(".showRecents", settings.show_recents);
    config_flag_set(".showExpert", settings.show_expert);
    config_flag_set(".noLowBatteryAutoSave", !settings.low_battery_autosave);
    config_flag_set(".smbdState", settings.smbd_state);
    config_flag_set(".httpState", settings.http_state);
    config_flag_set(".sshState", settings.ssh_state);
    config_flag_set(".ftpState", settings.ftp_state);
    config_flag_set(".telnetState", settings.telnet_state);
    config_flag_set(".hotspotState", settings.hotspot_state);
    config_flag_set(".ntpState", settings.ntp_state);
    config_flag_set(".ntpWait", settings.ntp_wait);
    config_flag_set(".authtelnetState", settings.auth_telnet_state);
    config_flag_set(".authftpState", settings.auth_ftp_state);
    config_flag_set(".authhttpState", settings.auth_http_state);
    config_flag_set(".authsshState", settings.auth_ssh_state);
    config_flag_set(".muteVolume", settings.mute);
    config_flag_set(".disableStandby", settings.disable_standby);
    config_setNumber("battery/warnAt", settings.low_battery_warn_at);
    config_setNumber("startup/app", settings.startup_application);
    config_setNumber("startup/addHours", settings.time_skip);
    config_setNumber("vibration", settings.vibration);
    config_setNumber("startup/tab", settings.startup_tab);

    // remove deprecated flags
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
    cJSON_free(json_root);
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
