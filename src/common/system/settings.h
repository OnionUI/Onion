#ifndef SETTINGS_H__
#define SETTINGS_H__

#include <stdbool.h>

#include "utils/file.h"
#include "utils/json.h"
#include "utils/config.h"
#include "display.h"

#define MAX_BRIGHTNESS 10
#define MAIN_UI_SETTINGS "/appconfigs/system.json"

static struct settings_s
{
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
    char theme[JSON_STRING_LEN];
    int fontsize;
    int audiofix;
    bool vibration;
    bool menu_haptics;
    bool switcher_enabled;
    bool menu_inverted;
    bool low_battery_warning;
    bool low_battery_autosave;
}
settings = {
    // MainUI settings
    .volume = 20,
    .keymap = "L2,L,R2,R,X,A,B,Y",
    .mute = 1,
    .bgm_volume = 20,
    .brightness = 7,
    .language = "en.lang",
    .sleep_timer = 5,
    .lumination = 7,
    .hue = 10,
    .saturation = 10,
    .contrast = 10,
    .theme = "/mnt/SDCARD/Themes/Silky by DiMo/",
    .fontsize = 24,
    .audiofix = 1,
    // Onion settings
    .vibration = true,
    .menu_haptics = false,
    .switcher_enabled = true,
    .menu_inverted = false,
    .low_battery_warning = true,
    .low_battery_autosave = true
};

static bool settings_loaded = false;

void settings_load(void)
{
    const char *json_str = NULL;

    settings.vibration = !config_flag_get(".noVibration");
    settings.menu_haptics = config_flag_get(".menuHaptics");
    settings.menu_inverted = config_flag_get(".menuInverted");
    settings.switcher_enabled = !config_flag_get(".noGameSwitcher");
    settings.low_battery_warning = !config_flag_get(".noBatteryWarning");
    settings.low_battery_autosave = !config_flag_get(".noLowBatteryAutoSave");

	if (!(json_str = file_read(MAIN_UI_SETTINGS)))
		return;

	cJSON* json_root = cJSON_Parse(json_str);

    json_getInt(json_root, "vol", &settings.volume);
    json_getInt(json_root, "mute", &settings.mute);
    json_getInt(json_root, "bgmvol", &settings.bgm_volume);
    json_getInt(json_root, "brightness",&settings.brightness);
    json_getInt(json_root, "hibernate", &settings.sleep_timer);
    json_getInt(json_root, "lumination", &settings.lumination);
    json_getInt(json_root, "hue", &settings.hue);
    json_getInt(json_root, "saturation", &settings.saturation);
    json_getInt(json_root, "contrast", &settings.contrast);
    json_getInt(json_root, "fontsize", &settings.fontsize);
    json_getInt(json_root, "audiofix", &settings.audiofix);

    json_getString(json_root, "keymap", settings.keymap);
    json_getString(json_root, "language", settings.language);
    json_getString(json_root, "theme", settings.theme);

    cJSON_free(json_root);

    settings_loaded = true;
}

void settings_save(void)
{
    FILE *fp;

    config_flag_set(".noVibration", !settings.vibration);
    config_flag_set(".menuHaptics", settings.menu_haptics);
    config_flag_set(".menuInverted", settings.menu_inverted);
    config_flag_set(".noGameSwitcher", !settings.switcher_enabled);
    config_flag_set(".noBatteryWarning", !settings.low_battery_warning);
    config_flag_set(".noLowBatteryAutoSave", !settings.low_battery_autosave);

    if ((fp = fopen(MAIN_UI_SETTINGS, "w+")) == 0)
        return;

    fprintf(fp, "{\n");
    fprintf(fp, JSON_FORMAT_NUMBER, "vol", settings.volume);
    fprintf(fp, JSON_FORMAT_STRING, "keymap", settings.keymap);
    fprintf(fp, JSON_FORMAT_NUMBER, "mute", settings.mute);
    fprintf(fp, JSON_FORMAT_NUMBER, "bgmvol", settings.bgm_volume);
    fprintf(fp, JSON_FORMAT_NUMBER, "brightness", settings.brightness);
    fprintf(fp, JSON_FORMAT_STRING, "language", settings.language);
    fprintf(fp, JSON_FORMAT_NUMBER, "hibernate", settings.sleep_timer);
    fprintf(fp, JSON_FORMAT_NUMBER, "lumination", settings.lumination);
    fprintf(fp, JSON_FORMAT_NUMBER, "hue", settings.hue);
    fprintf(fp, JSON_FORMAT_NUMBER, "saturation", settings.saturation);
    fprintf(fp, JSON_FORMAT_NUMBER, "contrast", settings.contrast);
    fprintf(fp, JSON_FORMAT_STRING, "theme", settings.theme);
    fprintf(fp, JSON_FORMAT_NUMBER, "fontsize", settings.fontsize);
    fprintf(fp, JSON_FORMAT_NUMBER_NC, "audiofix", settings.audiofix);
    fprintf(fp, "}\n");

    fflush(fp);
    fsync(fileno(fp));
    fclose(fp);
}

void settings_setBrightness(uint32_t value, bool apply)
{
    settings.brightness = value;

    if (apply)
        display_setBrightness(value);
}


#endif // SETTINGS_H__
