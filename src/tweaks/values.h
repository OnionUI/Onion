#ifndef TWEAKS_VALUES_H__
#define TWEAKS_VALUES_H__

#include "utils/msleep.h"
#include "theme/resources.h"
#include "components/list.h"
#include "system/state.h"

#include "./appstate.h"
#include "./tools.h"

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


