#ifndef TWEAKS_ACTIONS_H__
#define TWEAKS_ACTIONS_H__

#include "utils/msleep.h"
#include "theme/resources.h"
#include "components/list.h"

#include "./appstate.h"

void applyBatteryPercentage(void *pt)
{
    static int values[] = {cJSON_NULL, cJSON_False, cJSON_True};
    theme_changeOverrideFile("batteryPercentage", "visible", NULL, values[((ListItem*)pt)->value], true, &resources.theme);
    header_changed = true;
    system("./bin/mainUiBatPerc"); // TODO: add a delay when actions are repeated! (which should cancel out each other!)
}

#endif // TWEAKS_ACTIONS_H__


