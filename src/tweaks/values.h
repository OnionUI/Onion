#ifndef TWEAKS_VALUES_H__
#define TWEAKS_VALUES_H__

#include "utils/msleep.h"
#include "theme/resources.h"
#include "components/list.h"

#include "./appstate.h"

int value_batteryPercentageVisible(void)
{
    bool override_value = false;
    bool has_override = theme_getOverride("batteryPercentage", "visible", &override_value, cJSON_True);
    if (!has_override)
        return 0;
    return override_value ? 2 : 1;
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

#endif // TWEAKS_VALUES_H__


