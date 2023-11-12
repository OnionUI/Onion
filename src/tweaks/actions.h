#ifndef TWEAKS_ACTIONS_H__
#define TWEAKS_ACTIONS_H__

#include "components/list.h"
#include "system/axp.h"
#include "system/osd.h"
#include "system/rumble.h"
#include "system/settings.h"
#include "theme/resources.h"
#include "theme/sound.h"
#include "utils/apps.h"
#include "utils/config.h"
#include "utils/file.h"
#include "utils/msleep.h"

#include "./appstate.h"
#include "./diags.h"
#include "./values.h"

void action_setAppShortcut(void *pt)
{
    ListItem *item = (ListItem *)pt;
    int value = item->value;
    char *sett_pt = item->action_id == 0 ? settings.mainui_button_x
                                         : settings.mainui_button_y;
    InstalledApp *apps = getInstalledApps(true);

    memset(sett_pt, 0, JSON_STRING_LEN * sizeof(char));

    if (value == 0)
        return;

    value -= 1;

    if (value < installed_apps_count) {
        strcpy(sett_pt, "app:");
        strncat(sett_pt, apps[value].dirName, JSON_STRING_LEN - 5);
        return;
    }

    value -= installed_apps_count;

    if (value < NUM_TOOLS) {
        strcpy(sett_pt, "tool:");
        strncat(sett_pt, tools_short_names[value], JSON_STRING_LEN - 6);
        return;
    }

    if (item->action_id == 1) {
        strcpy(sett_pt, "glo");
    }
}

void action_meterWidth(void *pt)
{
    config_setNumber("display/meterWidth", ((ListItem *)pt)->value);
    osd_showBrightnessBar(settings.brightness);
}

static pthread_t last_thread;
static pthread_mutex_t thread_mutex = PTHREAD_MUTEX_INITIALIZER;
static int blf_changed = 0;
static int blf_fade_in = 0;

void setRGBValues(int value, int *endB, int *endG, int *endR)
{
    switch (value) {
    case 0:
        *endB = 128;
        *endG = 128;
        *endR = 128;
        break;
    case 1:
        *endB = 110;
        *endG = 125;
        *endR = 140;
        break;
    case 2:
        *endB = 100;
        *endG = 120;
        *endR = 140;
        break;
    case 3:
        *endB = 90;
        *endG = 115;
        *endR = 140;
        break;
    case 4:
        *endB = 80;
        *endG = 110;
        *endR = 140;
        break;
    case 5:
        *endB = 70;
        *endG = 105;
        *endR = 140;
        break;
    default:
        *endB = 0;
        *endG = 0;
        *endR = 0;
        break;
    }
}

void *action_blueLight_thread(void *arg)
{
    static int lastB, lastG, lastR;
    int combinedRGB;
    int value;

    // how is the function being called based on the arg
    if ((intptr_t)arg < 1024) {
        value = (intptr_t)arg;
        blf_changed = 1;
    }
    else if (arg != NULL) {
        ListItem *item = (ListItem *)arg;
        value = item->value;
    }
    else {
        value = 0;
    }

    // what is the current blf value
    config_get("display/blueLightRGB", CONFIG_INT, &combinedRGB);

    if ((intptr_t)arg == 0) {
        // we're turning blf off, fade out to default values (case 0)
        blf_changed = 1;
    }

    if (blf_changed) {
        lastR = (combinedRGB >> 16) & 0xFF;
        lastG = (combinedRGB >> 8) & 0xFF;
        lastB = combinedRGB & 0xFF;
        blf_changed = 0;

        if (lastR == 0 && lastG == 0 && lastB == 0) { // recalculate if they get lost
            setRGBValues(settings.blue_light_level, &lastB, &lastG, &lastR);
        }
    }

    if (blf_fade_in) {
        lastR = lastG = lastB = 128;
        blf_fade_in = 0;
    }

    int endB, endG, endR;
    setRGBValues(value, &endB, &endG, &endR);

    combinedRGB = (endR << 16) | (endG << 8) | endB;
    config_setNumber("display/blueLightRGB", combinedRGB);
    settings.blue_light_rgb = combinedRGB;

    char cmd[128];
    for (int i = 0; i <= 20; i++) {
        snprintf(cmd, sizeof(cmd), "echo 'colortemp 0 0 0 0 %d %d %d' > /proc/mi_modules/mi_disp/mi_disp0",
                 lastB + (endB - lastB) * i / 20,
                 lastG + (endG - lastG) * i / 20,
                 lastR + (endR - lastR) * i / 20);
        system(cmd);
        usleep(50000);
    }

    return NULL;
}

void action_blueLight()
{
    pthread_mutex_lock(&thread_mutex);
    if (last_thread) {
        pthread_cancel(last_thread);
        pthread_join(last_thread, NULL);
    }

    pthread_create(&last_thread, NULL, action_blueLight_thread, (void *)(intptr_t)settings.blue_light_level);
    pthread_detach(last_thread);
    pthread_mutex_unlock(&thread_mutex);

    reset_menus = true;
    all_changed = true;
}

void action_blueLightLevel(void *pt)
{
    int value;
    ListItem *item = (ListItem *)pt;

    value = item->value;
    settings.blue_light_level = value;
    config_setNumber("display/blueLightLevel", value);

    remove("/tmp/blueLightOn");
    blf_changed = 1;
}

void action_blueLightState(void *pt)
{
    ListItem *item = (ListItem *)pt;
    settings.blue_light_state = item->value == 1;
    config_flag_set(".blf", settings.blue_light_state);

    if (item->value == 0) {
        pthread_create(&last_thread, NULL, action_blueLight_thread, 0);
        pthread_detach(last_thread);
    }
    else {
        // blf being enabled, get the default value. set the fade_in flag
        int savedStrength = value_blueLightLevel();
        action_blueLight((void *)(intptr_t)savedStrength);
        // blf_fade_in = 1;
    }

    reset_menus = true;
    all_changed = true;
}

void action_blueLightTimeOn(void *pt)
{
    char time_str[10];
    formatter_Time(pt, time_str);
    strcpy(settings.blue_light_time, time_str);
    config_setString("display/blueLightTime", time_str);
}

void action_blueLightTimeOff(void *pt)
{
    char time_str[10];
    formatter_Time(pt, time_str);
    strcpy(settings.blue_light_time_off, time_str);
    config_setString("display/blueLightTimeOff", time_str);
}

void action_setStartupAutoResume(void *pt)
{
    settings.startup_auto_resume = ((ListItem *)pt)->value == 1;
}

void action_setStartupApplication(void *pt)
{
    settings.startup_application = ((ListItem *)pt)->value;
}

void action_setVibration(void *pt)
{
    sound_change();
    skip_next_change = true;
    settings.vibration = ((ListItem *)pt)->value;
    short_pulse();
}

void action_setLowBatteryAutoSave(void *pt)
{
    settings.low_battery_autosave_at = ((ListItem *)pt)->value;
}

void action_setDisableStandby(void *pt)
{
    settings.disable_standby = ((ListItem *)pt)->value == 1;
}

void action_setEnableLogging(void *pt)
{
    settings.enable_logging = ((ListItem *)pt)->value == 1;
    char new_value[22];
    sprintf(new_value, "log_to_file = %s", settings.enable_logging ? "\"true\"" : "\"false\"");
    file_changeKeyValue(RETROARCH_CONFIG, "log_to_file =", new_value);
}

void action_runDiagnosticScript(void *payload_ptr)
{ // run the script based on what the payload_ptr gives us
    ListItem *item = (ListItem *)payload_ptr;
    char *filename = (char *)item->payload_ptr;
    char script_path[DIAG_MAX_PATH_LENGTH + 1];
    snprintf(script_path, sizeof(script_path), "%s/%s", DIAG_SCRIPT_PATH, filename);

    pthread_t thread;
    if (pthread_create(&thread, NULL, diags_runScript, payload_ptr) != 0) {
        list_updateStickyNote(item, "Failed to run script..."); // threading issues
        list_changed = true;
    }

    pthread_detach(thread);
}

void action_setMenuButtonHaptics(void *pt)
{
    settings.menu_button_haptics = ((ListItem *)pt)->value == 1;
}

void action_setMenuButtonKeymap(void *pt)
{
    ListItem *item = (ListItem *)pt;
    static int *dests[] = {
        &settings.mainui_single_press, &settings.mainui_long_press,
        &settings.mainui_double_press, &settings.ingame_single_press,
        &settings.ingame_long_press, &settings.ingame_double_press};
    *(dests[item->action_id]) = item->value;
}

void action_batteryPercentageVisible(void *pt)
{
    static bool applied_values[] = {false, false, true};
    applied_values[0] = resources.theme_back.batteryPercentage.visible;
    int item_value = ((ListItem *)pt)->value;
    resources.theme.batteryPercentage.visible = applied_values[item_value];

    static int value_types[] = {cJSON_NULL, cJSON_False, cJSON_True};
    theme_changeOverride("batteryPercentage", "visible", NULL,
                         value_types[item_value]);

    battery_changed = true;
}

void action_batteryPercentageFontFamily(void *pt)
{
    int item_value = ((ListItem *)pt)->value;
    char theme_value[JSON_STRING_LEN];
    strcpy(theme_value, resources.theme_back.batteryPercentage.font);

    if (item_value == 0) {
        strcpy(resources.theme.batteryPercentage.font, theme_value);
    }
    else {
        char font_path[JSON_STRING_LEN] = "/mnt/SDCARD/miyoo/app/";
        strcat(font_path, font_families[item_value - 1]);
        strcpy(resources.theme.batteryPercentage.font, font_path);
    }

    theme_changeOverride("batteryPercentage", "font",
                         resources.theme.batteryPercentage.font,
                         item_value == 0 ? cJSON_NULL : cJSON_String);
    resource_reloadFont(BATTERY);
    battery_changed = true;
}

void action_batteryPercentageFontSize(void *pt)
{
    int theme_value = resources.theme_back.batteryPercentage.size;
    int item_value = ((ListItem *)pt)->value;
    int new_value = item_value == 0 ? theme_value : font_sizes[item_value - 1];
    resources.theme.batteryPercentage.size = new_value;

    theme_changeOverride("batteryPercentage", "size", &new_value,
                         item_value == 0 ? cJSON_NULL : cJSON_Number);
    resource_reloadFont(BATTERY);
    battery_changed = true;
}

void action_batteryPercentagePosition(void *pt)
{
    static Theme_TextAlign applied_values[] = {LEFT, LEFT, CENTER, RIGHT};
    applied_values[0] = resources.theme_back.batteryPercentage.textAlign;
    int item_value = ((ListItem *)pt)->value;
    resources.theme.batteryPercentage.textAlign = applied_values[item_value];

    theme_changeOverride("batteryPercentage", "onleft", NULL, cJSON_NULL);

    static char *value_types[] = {"", "left", "center", "right"};
    if (item_value != 0)
        theme_changeOverride("batteryPercentage", "textAlign", value_types[item_value], cJSON_String);
    else
        theme_changeOverride("batteryPercentage", "textAlign", NULL, cJSON_NULL);

    battery_changed = true;
}

void action_batteryPercentageFixed(void *pt)
{
    static bool applied_values[] = {false, false, true};
    applied_values[0] = resources.theme_back.batteryPercentage.fixed;
    int item_value = ((ListItem *)pt)->value;
    resources.theme.batteryPercentage.fixed = applied_values[item_value];

    static int value_types[] = {cJSON_NULL, cJSON_False, cJSON_True};
    theme_changeOverride("batteryPercentage", "fixed", NULL, value_types[item_value]);

    battery_changed = true;
}

void action_batteryPercentageOffsetX(void *pt)
{
    int theme_value = resources.theme_back.batteryPercentage.offsetX;
    int item_value = ((ListItem *)pt)->value;
    int new_value = item_value == 0 ? theme_value : item_value - 1 - BATTPERC_MAX_OFFSET;
    resources.theme.batteryPercentage.offsetX = new_value;

    theme_changeOverride("batteryPercentage", "offsetX", &new_value, item_value == 0 ? cJSON_NULL : cJSON_Number);

    battery_changed = true;
}

void action_batteryPercentageOffsetY(void *pt)
{
    int theme_value = resources.theme_back.batteryPercentage.offsetY;
    int item_value = ((ListItem *)pt)->value;
    int new_value = item_value == 0 ? theme_value : item_value - 1 - BATTPERC_MAX_OFFSET;
    resources.theme.batteryPercentage.offsetY = new_value;

    theme_changeOverride("batteryPercentage", "offsetY", &new_value, item_value == 0 ? cJSON_NULL : cJSON_Number);

    battery_changed = true;
}

void action_hideLabelsIcons(void *pt)
{
    static bool applied_values[] = {false, false, true};
    applied_values[0] = resources.theme_back.hideLabels.icons;
    int item_value = ((ListItem *)pt)->value;
    resources.theme.hideLabels.icons = applied_values[item_value];

    static int value_types[] = {cJSON_NULL, cJSON_False, cJSON_True};
    theme_changeOverride("hideLabels", "icons", NULL, value_types[item_value]);
}

void action_hideLabelsHints(void *pt)
{
    static bool applied_values[] = {false, false, true};
    applied_values[0] = resources.theme_back.hideLabels.hints;
    int item_value = ((ListItem *)pt)->value;
    resources.theme.hideLabels.hints = applied_values[item_value];

    static int value_types[] = {cJSON_NULL, cJSON_False, cJSON_True};
    theme_changeOverride("hideLabels", "hints", NULL, value_types[item_value]);

    footer_changed = true;
}

void action_setShowRecents(void *pt)
{
    settings.show_recents = ((ListItem *)pt)->value == 1;
}

void action_setShowExpert(void *pt)
{
    settings.show_expert = ((ListItem *)pt)->value == 1;
}

void action_setLowBatteryWarnAt(void *pt)
{
    settings.low_battery_warn_at = ((ListItem *)pt)->value * 5;
    config_setNumber("battery/warnAt", settings.low_battery_warn_at);
}

void action_setStartupTab(void *pt)
{
    settings.startup_tab = ((ListItem *)pt)->value;
}

void action_setTimeSkip(void *pt)
{
    settings.time_skip = ((ListItem *)pt)->value;
}

void action_advancedSetFrameThrottle(void *pt)
{
    int item_value = ((ListItem *)pt)->value;
    stored_value_frame_throttle = item_value;
    stored_value_frame_throttle_changed = true;
}

void action_advancedSetSwapTriggers(void *pt)
{
    int item_value = ((ListItem *)pt)->value;
    stored_value_swap_triggers = item_value;
    stored_value_swap_triggers_changed = true;
}

void action_setAltBrightness(void *pt)
{
    config_flag_set(".altBrightness", ((ListItem *)pt)->value);
}

void action_advancedSetLcdVoltage(void *pt)
{
    int value = 0x0e - ((ListItem *)pt)->value;

    if (value == 0x0e) {
        axp_lcd_set(0x0e);
        config_flag_set(".lcdvolt", false);
        return;
    }

    if (value < 0x09 || value > 0x0e) {
        config_flag_set(".lcdvolt", false);
        return;
    }

    int res = axp_lcd_set(value);

    if (res != 0) {
        printf_debug("Error: Failed to set LCD voltage: %d\n",
                     1600 + value * 100);
        config_flag_set(".lcdvolt", false);
        msleep(200);
        return;
    }

    printf_debug("LCD voltage set to: %d\n", 1600 + value * 100);

    config_flag_set(".lcdvolt", true);

    FILE *fp;
    file_put(fp, LCD_VOLT_CONFIG, "%x", 0x0e);
}

#endif // TWEAKS_ACTIONS_H__
