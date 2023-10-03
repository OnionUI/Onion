#ifndef TWEAKS_VALUES_H__
#define TWEAKS_VALUES_H__

#include <time.h>

#include "components/list.h"
#include "system/axp.h"
#include "theme/resources.h"
#include "utils/apps.h"
#include "utils/config.h"
#include "utils/file.h"
#include "utils/json.h"
#include "utils/msleep.h"

#include "./appstate.h"
#include "./formatters.h"
#include "./tools.h"

#define LCD_VOLT_CONFIG "/mnt/SDCARD/.tmp_update/config/.lcdvolt"

static int stored_value_frame_throttle = 0;
static bool stored_value_frame_throttle_changed = false;

static int stored_value_swap_triggers = 0;
static bool stored_value_swap_triggers_changed = false;

int value_timezone(void)
{

    time_t t = time(NULL);
    struct tm lt = *localtime(&t);
    struct tm gm = *gmtime(&t);
    double utc_offset = difftime(mktime(&lt), mktime(&gm)) / 3600.0;
    if (gm.tm_isdst)
        utc_offset += 1.0;
    return (int)((utc_offset + 12.0) * 2.0);
}

int value_appShortcut(int button)
{
    int i;
    char *saved_value =
        button == 0 ? settings.mainui_button_x : settings.mainui_button_y;
    InstalledApp *apps = getInstalledApps(true);

    if (strncmp(saved_value, "app:", 4) == 0) {
        for (i = 0; i < installed_apps_count; i++)
            if (strcmp(saved_value + 4, apps[i].dirName) == 0)
                return 1 + i;
    }
    else if (strncmp(saved_value, "tool:", 5) == 0) {
        for (i = 0; i < NUM_TOOLS; i++)
            if (strcmp(saved_value + 5, tools_short_names[i]) == 0)
                return 1 + installed_apps_count + i;
    }
    else if (button == 1 && strcmp(saved_value, "glo") == 0) {
        return 1 + installed_apps_count + NUM_TOOLS;
    }

    return 0;
}

int value_meterWidth(void)
{
    int meterWidth = 4;
    config_get("display/meterWidth", CONFIG_INT, &meterWidth);
    return meterWidth;
}

int value_batteryPercentageVisible(void)
{
    bool override_value = false;
    bool has_override = theme_getOverride("batteryPercentage", "visible",
                                          &override_value, cJSON_True);
    if (!has_override)
        return 0;
    return override_value ? 2 : 1;
}

int value_batteryPercentageFontFamily(void)
{
    char override_value[JSON_STRING_LEN];
    bool has_override = theme_getOverride("batteryPercentage", "font", &override_value, cJSON_String);

    if (!has_override)
        return 0;

    int value = 0;
    for (int i = 0; i < num_font_families; i++)
        if (str_endsWith(override_value, font_families[i])) {
            value = i + 1;
            break;
        }

    return value;
}

int value_batteryPercentageFontSize(void)
{
    int override_value = 0;
    bool has_override = theme_getOverride("batteryPercentage", "size", &override_value, cJSON_Number);
    if (!has_override)
        return 0;

    int value = 0;
    for (int i = 0; i < num_font_sizes; i++)
        if (override_value == font_sizes[i]) {
            value = i + 1;
            break;
        }

    return value;
}

int value_batteryPercentagePosition(void)
{
    char override_value[JSON_STRING_LEN];
    bool has_override = theme_getOverride("batteryPercentage", "textAlign", override_value, cJSON_String);
    if (!has_override)
        return 0;
    if (strcmp("center", override_value) == 0)
        return 2;
    if (strcmp("right", override_value) == 0)
        return 3;
    return 1;
}

int value_batteryPercentageFixed(void)
{
    bool override_value;
    bool has_override = theme_getOverride("batteryPercentage", "fixed", &override_value, cJSON_True);
    if (!has_override)
        return 0;
    return override_value ? 2 : 1;
}

int value_batteryPercentageOffsetX(void)
{
    int override_value = 0;
    bool has_override = theme_getOverride("batteryPercentage", "offsetX", &override_value, cJSON_Number);
    if (!has_override)
        return 0;
    return override_value + 1 + BATTPERC_MAX_OFFSET;
}

int value_batteryPercentageOffsetY(void)
{
    int override_value = 0;
    bool has_override = theme_getOverride("batteryPercentage", "offsetY", &override_value, cJSON_Number);
    if (!has_override)
        return 0;
    return override_value + 1 + BATTPERC_MAX_OFFSET;
}

int value_hideLabelsIcons(void)
{
    bool override_value = false;
    bool has_override =
        theme_getOverride("hideLabels", "icons", &override_value, cJSON_True);
    if (!has_override)
        return 0;
    return override_value ? 2 : 1;
}

int value_hideLabelsHints(void)
{
    bool override_value = false;
    bool has_override =
        theme_getOverride("hideLabels", "hints", &override_value, cJSON_True);
    if (!has_override)
        return 0;
    return override_value ? 2 : 1;
}

int value_getFrameThrottle(void)
{
    char value[STR_MAX];
    if (file_parseKeyValue(RETROARCH_CONFIG, "fastforward_ratio", value, '=',
                           0) != NULL)
        stored_value_frame_throttle = atoi(value);
    return stored_value_frame_throttle;
}

void value_setFrameThrottle(void)
{
    if (!stored_value_frame_throttle_changed)
        return;
    char new_value[STR_MAX];
    sprintf(new_value, "fastforward_ratio = \"%d.000000\"",
            stored_value_frame_throttle);
    file_changeKeyValue(RETROARCH_CONFIG, "fastforward_ratio =", new_value);
}

int value_getSwapTriggers(void)
{
    int l_btn, r_btn, l2_btn, r2_btn;
    char value[STR_MAX];

    if (file_parseKeyValue(RETROARCH_CONFIG, "input_player1_l_btn", value, '=',
                           0) != NULL)
        l_btn = atoi(value);
    if (file_parseKeyValue(RETROARCH_CONFIG, "input_player1_r_btn", value, '=',
                           0) != NULL)
        r_btn = atoi(value);
    if (file_parseKeyValue(RETROARCH_CONFIG, "input_player1_l2_btn", value, '=',
                           0) != NULL)
        l2_btn = atoi(value);
    if (file_parseKeyValue(RETROARCH_CONFIG, "input_player1_r2_btn", value, '=',
                           0) != NULL)
        r2_btn = atoi(value);

    printf_debug("l: %d, r: %d, l2: %d, r2: %d\n", l_btn, r_btn, l2_btn,
                 r2_btn);
    bool is_normal = l_btn == 10 && r_btn == 11 && l2_btn == 12 && r2_btn == 13;

    return is_normal ? 0 : 1;
}

void value_setSwapTriggers(void)
{
    if (!stored_value_swap_triggers_changed)
        return;

    char value[STR_MAX];
    int l_btn = 10, r_btn = 11, l2_btn = 12, r2_btn = 13;

    if (stored_value_swap_triggers == 1)
        l_btn = 12, r_btn = 13, l2_btn = 10, r2_btn = 11;

    sprintf(value, "input_player1_l_btn = \"%d\"", l_btn);
    file_changeKeyValue(RETROARCH_CONFIG, "input_player1_l_btn =", value);

    sprintf(value, "input_player1_r_btn = \"%d\"", r_btn);
    file_changeKeyValue(RETROARCH_CONFIG, "input_player1_r_btn =", value);

    sprintf(value, "input_player1_l2_btn = \"%d\"", l2_btn);
    file_changeKeyValue(RETROARCH_CONFIG, "input_player1_l2_btn =", value);

    sprintf(value, "input_player1_r2_btn = \"%d\"", r2_btn);
    file_changeKeyValue(RETROARCH_CONFIG, "input_player1_r2_btn =", value);

    printf_debug("Saved triggers = l: %d, r: %d, l2: %d, r2: %d\n", l_btn,
                 r_btn, l2_btn, r2_btn);
}

int value_getPWMFrequency(void)
{
    int pwmfrequency = 0;
    config_get(".pwmfrequency", CONFIG_INT, &pwmfrequency);
    return pwmfrequency;
}

int value_getLcdVoltage(void)
{
    int value = 0x0;

    if (!is_file(LCD_VOLT_CONFIG))
        return 0;

    value = axp_lcd_get();

    if (value < 0x09 || value > 0x0e) {
        config_flag_set(".lcdvolt", false);
        return 0;
    }

    return 0x0e - value;
}

void value_setLcdVoltage(void)
{
    FILE *fp;
    int value;

    if (!is_file(LCD_VOLT_CONFIG))
        return;

    value = axp_lcd_get();

    if (value < 0x09 || value > 0x0e) {
        config_flag_set(".lcdvolt", false);
        return;
    }

    file_put(fp, LCD_VOLT_CONFIG, "%x", value);
}

#endif // TWEAKS_VALUES_H__
