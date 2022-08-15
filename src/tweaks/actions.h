#ifndef TWEAKS_ACTIONS_H__
#define TWEAKS_ACTIONS_H__

#include "utils/msleep.h"
#include "theme/resources.h"
#include "components/list.h"
#include "system/settings.h"
#include "system/rumble.h"

#include "./appstate.h"

void action_setStartupAutoResume(void *pt)
{
    settings.startup_auto_resume = ((ListItem*)pt)->value == 1;
}

void action_setStartupApplication(void *pt)
{
    settings.startup_application = ((ListItem*)pt)->value;
}

void action_setVibration(void *pt)
{
    settings.vibration = ((ListItem*)pt)->value;
    short_pulse();
}

void action_setLowBatteryAutoSave(void *pt)
{
    settings.low_battery_autosave = ((ListItem*)pt)->value == 1;
}

void action_setMenuButtonHaptics(void *pt)
{
    settings.menu_button_haptics = ((ListItem*)pt)->value == 1;
}

void action_setMenuButtonKeymap(void *pt)
{
    ListItem *item = (ListItem*)pt;
    static int *dests[] = {
        &settings.mainui_single_press,
        &settings.mainui_long_press,
        &settings.mainui_double_press,
        &settings.ingame_single_press,
        &settings.ingame_long_press,
        &settings.ingame_double_press
    };
    *(dests[item->action_id]) = item->value;
}

void action_batteryPercentageVisible(void *pt)
{
    static bool applied_values[] = {false, false, true};
    applied_values[0] = resources.theme_back.batteryPercentage.visible;
    int item_value = ((ListItem*)pt)->value;
    resources.theme.batteryPercentage.visible = applied_values[item_value];

    static int value_types[] = {cJSON_NULL, cJSON_False, cJSON_True};
    theme_changeOverride("batteryPercentage", "visible", NULL, value_types[item_value]);

    header_changed = true;
}

void action_batteryPercentagePosition(void *pt)
{
    static bool applied_values[] = {false, true, false};
    applied_values[0] = resources.theme_back.batteryPercentage.onleft;
    int item_value = ((ListItem*)pt)->value;    
    resources.theme.batteryPercentage.onleft = applied_values[((ListItem*)pt)->value];

    static int value_types[] = {cJSON_NULL, cJSON_True, cJSON_False};
    theme_changeOverride("batteryPercentage", "onleft", NULL, value_types[((ListItem*)pt)->value]);

    header_changed = true;
}

void action_batteryPercentageOffsetX(void *pt)
{
    int theme_value = resources.theme_back.batteryPercentage.offsetX;
    int item_value = ((ListItem*)pt)->value;
    int new_value = item_value == 0 ? theme_value : item_value - 11;
    resources.theme.batteryPercentage.offsetX = new_value;

    theme_changeOverride("batteryPercentage", "offsetX", &new_value, item_value == 0 ? cJSON_NULL : cJSON_Number);

    header_changed = true;
}

void action_batteryPercentageOffsetY(void *pt)
{
    int theme_value = resources.theme_back.batteryPercentage.offsetX;
    int item_value = ((ListItem*)pt)->value;
    int new_value = item_value == 0 ? theme_value : item_value - 11;
    resources.theme.batteryPercentage.offsetY = new_value;

    theme_changeOverride("batteryPercentage", "offsetY", &new_value, item_value == 0 ? cJSON_NULL : cJSON_Number);

    header_changed = true;
}

void action_hideLabelsIcons(void *pt)
{
    static bool applied_values[] = {false, false, true};
    applied_values[0] = resources.theme_back.hideLabels.icons;
    int item_value = ((ListItem*)pt)->value;
    resources.theme.hideLabels.icons = applied_values[item_value];

    static int value_types[] = {cJSON_NULL, cJSON_False, cJSON_True};
    theme_changeOverride("hideLabels", "icons", NULL, value_types[item_value]);

    footer_changed = true;
}

void action_hideLabelsHints(void *pt)
{
    static bool applied_values[] = {false, false, true};
    applied_values[0] = resources.theme_back.hideLabels.hints;
    int item_value = ((ListItem*)pt)->value;
    resources.theme.hideLabels.hints = applied_values[item_value];

    static int value_types[] = {cJSON_NULL, cJSON_False, cJSON_True};
    theme_changeOverride("hideLabels", "hints", NULL, value_types[item_value]);

    footer_changed = true;
}

void action_setShowRecents(void *pt)
{
    settings.show_recents = ((ListItem*)pt)->value == 1;
}

void action_setShowExpert(void *pt)
{
    settings.show_expert = ((ListItem*)pt)->value == 1;
}

void action_setLowBatteryWarnAt(void *pt)
{
    settings.low_battery_warn_at = ((ListItem*)pt)->value * 5;
    config_setNumber("battery/warnAt", settings.low_battery_warn_at);
}

#endif // TWEAKS_ACTIONS_H__
