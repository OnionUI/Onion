#ifndef SETTINGS_H__
#define SETTINGS_H__

#include <stdbool.h>

#ifdef PLATFORM_MIYOOMINI
#include "shmvar/shmvar.h"
static KeyShmInfo shminfo;
#endif

#include "../utils/file.h"
#include "../utils/json.h"
#include "../utils/flags.h"

#define MAX_BRIGHTNESS 10

#define MAIN_UI_SETTINGS "/appconfigs/system.json"


static struct settings_s
{
    uint32_t volume;
    char *keymap;
    uint32_t mute;
    uint32_t bgm_volume;
    uint32_t brightness;
    char *language;
    uint32_t sleep_timer;
    uint32_t lumination;
    uint32_t hue;
    uint32_t saturation;
    uint32_t contrast;
    char *theme;
    uint32_t fontsize;
    uint32_t audiofix;
    bool vibration;
    bool launcher;
    bool menu_inverted;
    bool low_battery_warning;
    bool low_battery_shutdown;
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
    .theme = "/mnt/SDCARD/Themes/Silky [Onion Default] by DiMo/",
    .fontsize = 24,
    .audiofix = 1,
    // Onion settings
    .vibration = true,
    .launcher = true,
    .menu_inverted = false,
    .low_battery_warning = true,
    .low_battery_shutdown = true
};

void settings_load(void)
{
    const char *json_str = NULL;

    settings.vibration = !settings_flag_get(".noVibration");
    settings.menu_inverted = settings_flag_get(".menuInverted");
    settings.launcher = !settings_flag_get(".noGameSwitcher");
    settings.low_battery_warning = !settings_flag_get(".noBatteryWarning");
    settings.low_battery_shutdown = !settings_flag_get(".noLowBatteryOff");

	if (!(json_str = file_readAll(MAIN_UI_SETTINGS)))
		return;

	cJSON* json_root = cJSON_Parse(json_str);

    json_getNumberValue(json_root, "vol", (int*)&settings.volume);
    json_getStringValue(json_root, "keymap", settings.keymap);
    json_getNumberValue(json_root, "mute", (int*)&settings.mute);
    json_getNumberValue(json_root, "bgmvol", (int*)&settings.bgm_volume);
    json_getNumberValue(json_root, "brightness",(int*)&settings.brightness);
    json_getStringValue(json_root, "language", settings.language);
    json_getNumberValue(json_root, "hibernate", (int*)&settings.sleep_timer);
    json_getNumberValue(json_root, "lumination", (int*)&settings.lumination);
    json_getNumberValue(json_root, "hue", (int*)&settings.hue);
    json_getNumberValue(json_root, "saturation", (int*)&settings.saturation);
    json_getNumberValue(json_root, "contrast", (int*)&settings.contrast);
    json_getStringValue(json_root, "theme", settings.theme);
    json_getNumberValue(json_root, "fontsize", (int*)&settings.fontsize);
    json_getNumberValue(json_root, "audiofix", (int*)&settings.audiofix);

    cJSON_free(json_root);

    #ifdef PLATFORM_MIYOOMINI
    SetKeyShm(&shminfo, MONITOR_VOLUME, settings.volume);
    SetKeyShm(&shminfo, MONITOR_BRIGHTNESS, settings.brightness);
    SetKeyShm(&shminfo, MONITOR_KEYMAP, 0);
    SetKeyShm(&shminfo, MONITOR_MUTE, settings.mute);
    SetKeyShm(&shminfo, MONITOR_VOLUME_CHANGED, 0);
    SetKeyShm(&shminfo, MONITOR_BGM_VOLUME, settings.bgm_volume);
    SetKeyShm(&shminfo, MONITOR_HIBERNATE_DELAY, settings.sleep_timer);
    SetKeyShm(&shminfo, MONITOR_LUMINATION, settings.lumination);
    SetKeyShm(&shminfo, MONITOR_HUE, settings.hue);
    SetKeyShm(&shminfo, MONITOR_SATURATION, settings.saturation);
    SetKeyShm(&shminfo, MONITOR_CONTRAST, settings.contrast);
    SetKeyShm(&shminfo, MONITOR_AUDIOFIX, settings.audiofix);
    #endif
}

void settings_save(void)
{
    FILE *fp;

    settings_flag_set(".noVibration", !settings.vibration);
    settings_flag_set(".menuInverted", settings.menu_inverted);
    settings_flag_set(".noGameSwitcher", !settings.launcher);
    settings_flag_set(".noBatteryWarning", !settings.low_battery_warning);
    settings_flag_set(".noLowBatteryOff", !settings.low_battery_shutdown);

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

void settings_init(void)
{
    #ifdef PLATFORM_MIYOOMINI
    InitKeyShm(&shminfo);

    // Disable MainUI battery monitor
    SetKeyShm(&shminfo, MONITOR_ADC_VALUE, 640);
    #endif

    settings_load();
}

void settings_setBrightness(uint32_t value)
{
    #ifdef PLATFORM_MIYOOMINI
    SetKeyShm(&shminfo, MONITOR_BRIGHTNESS, value);
    #endif
    settings.brightness = value;
}


#endif // SETTINGS_H__
