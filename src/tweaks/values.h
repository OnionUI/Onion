#ifndef TWEAKS_VALUES_H__
#define TWEAKS_VALUES_H__

#include "utils/msleep.h"
#include "utils/json.h"
#include "theme/resources.h"
#include "components/list.h"
#include "system/state.h"

#include "./appstate.h"
#include "./tools.h"

static int stored_value_frame_throttle = 0;
static bool stored_value_frame_throttle_changed = false;

static int stored_value_swap_triggers = 0;
static bool stored_value_swap_triggers_changed = false;

int value_appShortcut(int button)
{
    int i;
    char *saved_value = button == 0 ? settings.mainui_button_x : settings.mainui_button_y;
    char ***apps = getInstalledApps();

    if (strncmp(saved_value, "app:", 4) == 0) {
        for (i = 0; i < installed_apps_count; i++)
            if (strcmp(saved_value + 4, apps[i][0]) == 0)
                return 1 + i;
    }
    else if (strncmp(saved_value, "tool:", 5) == 0) {
        for (i = 0; i < NUM_TOOLS; i++)
            if (strcmp(saved_value + 5, tools_short_names[i]) == 0)
                return 1 + installed_apps_count + i;
    }

    return 0;
}

int value_batteryPercentageVisible(void)
{
    bool override_value = false;
    bool has_override = theme_getOverride("batteryPercentage", "visible", &override_value, cJSON_True);
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
    bool override_value = false;
    bool has_override = theme_getOverride("batteryPercentage", "onleft", &override_value, cJSON_True);
    if (!has_override)
        return 0;
    return override_value ? 1 : 2;
}

int value_batteryPercentageOffsetX(void)
{
    int override_value = 0;
    bool has_override = theme_getOverride("batteryPercentage", "offsetX", &override_value, cJSON_Number);
    if (!has_override)
        return 0;
    return override_value + 11;
}

int value_batteryPercentageOffsetY(void)
{
    int override_value = 0;
    bool has_override = theme_getOverride("batteryPercentage", "offsetY", &override_value, cJSON_Number);
    if (!has_override)
        return 0;
    return override_value + 11;
}

int value_hideLabelsIcons(void)
{
    bool override_value = false;
    bool has_override = theme_getOverride("hideLabels", "icons", &override_value, cJSON_True);
    if (!has_override)
        return 0;
    return override_value ? 2 : 1;
}

int value_hideLabelsHints(void)
{
    bool override_value = false;
    bool has_override = theme_getOverride("hideLabels", "hints", &override_value, cJSON_True);
    if (!has_override)
        return 0;
    return override_value ? 2 : 1;
}

int value_getFrameThrottle(void)
{
    char value[STR_MAX];
    if (file_parseKeyValue(RETROARCH_CONFIG, "fastforward_ratio", value, '=', 0) != NULL)
        stored_value_frame_throttle = atoi(value);
    return stored_value_frame_throttle;
}

void value_setFrameThrottle(void)
{
    if (!stored_value_frame_throttle_changed)
        return;
    char new_value[STR_MAX];
    sprintf(new_value, "fastforward_ratio = \"%d.000000\"", stored_value_frame_throttle);
    file_changeKeyValue(RETROARCH_CONFIG, "fastforward_ratio =", new_value);
}

int value_getSwapTriggers(void)
{
    int l_btn, r_btn, l2_btn, r2_btn;
    char value[STR_MAX];

    if (file_parseKeyValue(RETROARCH_CONFIG, "input_player1_l_btn", value, '=', 0) != NULL)
        l_btn = atoi(value);
    if (file_parseKeyValue(RETROARCH_CONFIG, "input_player1_r_btn", value, '=', 0) != NULL)
        r_btn = atoi(value);
    if (file_parseKeyValue(RETROARCH_CONFIG, "input_player1_l2_btn", value, '=', 0) != NULL)
        l2_btn = atoi(value);
    if (file_parseKeyValue(RETROARCH_CONFIG, "input_player1_r2_btn", value, '=', 0) != NULL)
        r2_btn = atoi(value);

    printf_debug("l: %d, r: %d, l2: %d, r2: %d\n", l_btn, r_btn, l2_btn, r2_btn);
    bool is_normal = l_btn == 10 &&
                     r_btn == 11 &&
                     l2_btn == 12 && 
                     r2_btn == 13;

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

    printf_debug("Saved triggers = l: %d, r: %d, l2: %d, r2: %d\n", l_btn, r_btn, l2_btn, r2_btn);
}

#endif // TWEAKS_VALUES_H__


