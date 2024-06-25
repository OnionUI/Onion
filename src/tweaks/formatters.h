#ifndef TWEAKS_FORMATTERS_H__
#define TWEAKS_FORMATTERS_H__

#include <stdio.h>

#include "components/list.h"
#include "utils/apps.h"
#include "utils/str.h"

#include "./tools.h"

#define BATTPERC_MAX_OFFSET 48

#define BUTTON_MAINUI_LABELS                          \
    {                                                 \
        "Context menu", "GameSwitcher", "Resume game" \
    }
#define BUTTON_INGAME_LABELS                                                \
    {                                                                       \
        "Off", "GameSwitcher", "Exit to menu", "Quick switch", "Quick menu" \
    }

#define THEME_TOGGLE_LABELS \
    {                       \
        "-", "Off", "On"    \
    }

#define BLUELIGHT_LABELS                                                          \
    {                                                                             \
        "Subtle 1/5", "Moderate 2/5", "Balanced 3/5", "Strong 4/5", "Intense 5/5" \
    }

#define PWM_FREQUENCIES                                                                                                \
    {                                                                                                                  \
        "100 Hz", "200 Hz", "300 Hz", "400 Hz", "500 Hz", "600 Hz", "700 Hz", "800  Hz (Default)", "900 Hz", "1000 Hz" \
    }

void formatter_timezone(void *pt, char *out_label)
{
    ListItem *item = (ListItem *)pt;
    int value = item->value;
    double utc_value = ((double)value / 2.0) - 12.0;
    bool half_past = round(utc_value) != utc_value;
    if (utc_value == 0.0) {
        strcpy(out_label, "UTC");
    }
    else {
        sprintf(out_label, utc_value > 0.0 ? "UTC+%02d:%02d" : "UTC-%02d:%02d", (int)floor(abs(utc_value)), half_past ? 30 : 0);
    }
}

void formatter_Time(void *pt, char *out_label)
{
    ListItem *item = (ListItem *)pt;
    int value = item->value;
    int hours = value / 4;
    int minutes = (value % 4) * 15;
    sprintf(out_label, "%02d:%02d", hours, minutes);
}

int formatter_timeStringToID(const char *time_str)
{
    int hours, minutes;
    sscanf(time_str, "%02d:%02d", &hours, &minutes);
    int intervalsFromHours = hours * 4;
    int intervalsFromMinutes = minutes / 15;
    return intervalsFromHours + intervalsFromMinutes;
}

void formatter_appShortcut(void *pt, char *out_label)
{
    ListItem *item = (ListItem *)pt;
    int value = item->value;
    InstalledApp *apps = getInstalledApps(true);
    int max_value = installed_apps_count + NUM_TOOLS + item->action_id;

    if (value <= 0 || value > max_value) {
        strcpy(out_label, item->action_id == 0 ? "B button" : "A button");
        return;
    }

    // apps
    value -= 1;
    if (value < installed_apps_count) {
        InstalledApp *app = &apps[value];
        strcpy(out_label, app->is_duplicate ? app->dirName : app->label);
        return;
    }

    // tools
    value -= installed_apps_count;
    if (value < NUM_TOOLS) {
        sprintf(out_label, "Tool: %s", tools_short_names[value]);
        return;
    }

    if (item->action_id == 1) {
        strcpy(out_label, "GLO");
    }
}

void formatter_battWarn(void *pt, char *out_label)
{
    ListItem *item = (ListItem *)pt;
    if (item->value == 0)
        strcpy(out_label, "Off");
    else
        sprintf(out_label, "< %d%%", item->value * 5);
}

void formatter_battExit(void *pt, char *out_label)
{
    ListItem *item = (ListItem *)pt;
    if (item->value == 0)
        strcpy(out_label, "Off");
    else
        sprintf(out_label, "< %d%%", item->value);
}

static const int num_font_families = 5;
static const char font_families[][STR_MAX] = {
    "BPreplayBold.otf", "Exo-2-Bold-Italic_Universal.ttf",
    "Helvetica-Neue-2.ttf", "HENB.TTF", "wqy-microhei.ttc"};
void formatter_fontFamily(void *pt, char *out_label)
{
    ListItem *item = (ListItem *)pt;
    if (item->value == 0)
        strcpy(out_label, "-");
    else
        strcpy(out_label, font_families[item->value - 1]);
}

static const int num_font_sizes = 5;
static const int font_sizes[] = {13, 18, 24, 32, 40};
void formatter_fontSize(void *pt, char *out_label)
{
    ListItem *item = (ListItem *)pt;
    if (item->value == 0)
        strcpy(out_label, "-");
    else
        sprintf(out_label, "%d px", font_sizes[item->value - 1]);
}

void formatter_fastForward(void *pt, char *out_label)
{
    ListItem *item = (ListItem *)pt;
    if (item->value == 0)
        strcpy(out_label, "Unlimited");
    else
        sprintf(out_label, "%d.0x", item->value);
}

void formatter_positionOffset(void *pt, char *out_label)
{
    ListItem *item = (ListItem *)pt;
    if (item->value == 0)
        strcpy(out_label, "-");
    else
        sprintf(out_label, "%d px", item->value - 1 - BATTPERC_MAX_OFFSET);
}

void formatter_meterWidth(void *pt, char *out_label)
{
    ListItem *item = (ListItem *)pt;
    sprintf(out_label, "%d px", item->value);
}

void formatter_startupTab(void *pt, char *out_label)
{
    ListItem *item = (ListItem *)pt;
    switch (item->value) {
    case 0:
        strcpy(out_label, "Main menu");
        break;
    case 1:
        strncpy(out_label,
                lang_get(LANG_RECENTS_TAB, LANG_FALLBACK_RECENTS_TAB),
                STR_MAX - 1);
        break;
    case 2:
        strncpy(out_label,
                lang_get(LANG_FAVORITES_TAB, LANG_FALLBACK_FAVORITES_TAB),
                STR_MAX - 1);
        break;
    case 3:
        strncpy(out_label, lang_get(LANG_GAMES_TAB, LANG_FALLBACK_GAMES_TAB),
                STR_MAX - 1);
        break;
    case 4:
        strncpy(out_label, lang_get(LANG_EXPERT_TAB, LANG_FALLBACK_EXPERT_TAB),
                STR_MAX - 1);
        break;
    case 5:
        strncpy(out_label, lang_get(LANG_APPS_TAB, LANG_FALLBACK_APPS_TAB),
                STR_MAX - 1);
        break;
    default:
        break;
    }
}

void formatter_timeSkip(void *pt, char *out_label)
{
    ListItem *item = (ListItem *)pt;
    if (item->value == 0)
        strcpy(out_label, "Off");
    else
        sprintf(out_label, "+ %dh", item->value);
}

#endif // TWEAKS_FORMATTERS_H__
