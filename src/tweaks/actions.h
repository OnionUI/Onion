#ifndef TWEAKS_ACTIONS_H__
#define TWEAKS_ACTIONS_H__

#include "components/list.h"
#include "system/axp.h"
#include "system/rumble.h"
#include "system/settings.h"
#include "theme/resources.h"
#include "theme/sound.h"
#include "utils/apps.h"
#include "utils/config.h"
#include "utils/file.h"
#include "utils/msleep.h"

#include "./appstate.h"
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
    settings.low_battery_autosave = ((ListItem *)pt)->value == 1;
}

void action_sethttpstate(void *pt)
{
    settings.http_state = ((ListItem *)pt)->value == 1;
}

void action_setsshstate(void *pt)
{
    settings.ssh_state = ((ListItem *)pt)->value == 1;
}

void action_setftpstate(void *pt)
{
    settings.ftp_state = ((ListItem *)pt)->value == 1;
}

void action_settelnetstate(void *pt)
{
    settings.telnet_state = ((ListItem *)pt)->value == 1;
}

void action_sethotspotstate(void *pt)
{
    settings.hotspot_state = ((ListItem *)pt)->value == 1;
}

void action_setntpstate(void *pt)
{
    settings.ntp_state = ((ListItem *)pt)->value == 1;
}

void action_settzselectstate(void *pt)
{
    settings.tzselect_state = ((ListItem *)pt)->value;
}

void action_settelnetauthstate(void *pt)
{
    settings.auth_telnet_state = ((ListItem *)pt)->value == 1;
}

void action_setftpauthstate(void *pt)
{
    settings.auth_ftp_state = ((ListItem *)pt)->value == 1;
}

void action_sethttpsignupstate(void *pt)
{
    settings.signup_http_state = ((ListItem *)pt)->value == 1;
}

void action_sethttpauthstate(void *pt)
{
    settings.auth_http_state = ((ListItem *)pt)->value == 1;
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
        &settings.ingame_long_press,   &settings.ingame_double_press};
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
    static bool applied_values[] = {false, true, false};
    applied_values[0] = resources.theme_back.batteryPercentage.onleft;
    int item_value = ((ListItem *)pt)->value;
    resources.theme.batteryPercentage.onleft = applied_values[item_value];

    static int value_types[] = {cJSON_NULL, cJSON_True, cJSON_False};
    theme_changeOverride("batteryPercentage", "onleft", NULL,
                         value_types[item_value]);

    battery_changed = true;
}

void action_batteryPercentageOffsetX(void *pt)
{
    int theme_value = resources.theme_back.batteryPercentage.offsetX;
    int item_value = ((ListItem *)pt)->value;
    int new_value = item_value == 0 ? theme_value : item_value - 11;
    resources.theme.batteryPercentage.offsetX = new_value;

    theme_changeOverride("batteryPercentage", "offsetX", &new_value,
                         item_value == 0 ? cJSON_NULL : cJSON_Number);

    battery_changed = true;
}

void action_batteryPercentageOffsetY(void *pt)
{
    int theme_value = resources.theme_back.batteryPercentage.offsetX;
    int item_value = ((ListItem *)pt)->value;
    int new_value = item_value == 0 ? theme_value : item_value - 11;
    resources.theme.batteryPercentage.offsetY = new_value;

    theme_changeOverride("batteryPercentage", "offsetY", &new_value,
                         item_value == 0 ? cJSON_NULL : cJSON_Number);

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
