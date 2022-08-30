#ifndef TWEAKS_FORMATTERS_H__
#define TWEAKS_FORMATTERS_H__

#include <stdio.h>

#include "utils/str.h"
#include "components/list.h"
#include "system/state.h"

#include "./tools.h"

#define BUTTON_MAINUI_LABELS {"Context menu", "GameSwitcher", "Resume game"}
#define BUTTON_INGAME_LABELS {"Off", "GameSwitcher", "Exit to menu", "Quick switch"}

#define THEME_TOGGLE_LABELS {"-", "Off", "On"}

void formatter_appShortcut(void *pt, char *out_label)
{
	ListItem *item = (ListItem*)pt;
	int value = item->value;
	char ***apps = getInstalledApps();

	if (value <= 0 || value > installed_apps_count + NUM_TOOLS) {
		strcpy(out_label, "Off");
		return;
	}

	// apps
	value -= 1;
	if (value < installed_apps_count) {
		sprintf(out_label, "App: %s", apps[value][1]);
		return;
	}

	// tools
	value -= installed_apps_count;
	if (value < NUM_TOOLS) {
		sprintf(out_label, "Tool: %s", tools_short_names[value]);
		return;
	}
}

void formatter_battWarn(void *pt, char *out_label)
{
	ListItem *item = (ListItem*)pt;
	if (item->value == 0)
		strcpy(out_label, "Off");
	else sprintf(out_label, "< %d%%", item->value * 5);
}

static const int num_font_families = 5;
static const char font_families[][STR_MAX] = {
	"BPreplayBold.otf",
	"Exo-2-Bold-Italic_Universal.ttf",
	"Helvetica-Neue-2.ttf",
	"HENB.TTF",
	"wqy-microhei.ttc"
};
void formatter_fontFamily(void *pt, char *out_label)
{
	ListItem *item = (ListItem*)pt;
	if (item->value == 0)
		strcpy(out_label, "-");
	else strcpy(out_label, font_families[item->value - 1]);
}

static const int num_font_sizes = 5;
static const int font_sizes[] = {13, 18, 24, 32, 40};
void formatter_fontSize(void *pt, char *out_label)
{
	ListItem *item = (ListItem*)pt;
	if (item->value == 0)
		strcpy(out_label, "-");
	else sprintf(out_label, "%d px", font_sizes[item->value - 1]);
}

void formatter_fastForward(void *pt, char *out_label)
{
	ListItem *item = (ListItem*)pt;
	if (item->value == 0)
		strcpy(out_label, "Unlimited");
	else sprintf(out_label, "%d.0x", item->value);
}

void formatter_positionOffset(void *pt, char *out_label)
{
	ListItem *item = (ListItem*)pt;
	if (item->value == 0)
		strcpy(out_label, "-");
	else sprintf(out_label, "%d px", item->value - 11);
}

void formatter_startupTab(void *pt, char *out_label)
{
	ListItem *item = (ListItem*)pt;
	switch (item->value) {
		case 0: strcpy(out_label, "Main menu"); break;
		case 1: strncpy(out_label, lang_get(LANG_RECENTS_TAB), STR_MAX - 1); break;
		case 2: strncpy(out_label, lang_get(LANG_FAVORITES_TAB), STR_MAX - 1); break;
		case 3: strncpy(out_label, lang_get(LANG_GAMES_TAB), STR_MAX - 1); break;
		case 4: strncpy(out_label, lang_get(LANG_EXPERT_TAB), STR_MAX - 1); break;
		case 5: strncpy(out_label, lang_get(LANG_APPS_TAB), STR_MAX - 1); break;
		default: break;
	}
}

void formatter_timeSkip(void *pt, char *out_label)
{
	ListItem *item = (ListItem*)pt;
	if (item->value == 0)
		strcpy(out_label, "Off");
	else sprintf(out_label, "+ %dh", item->value);
}

#endif // TWEAKS_FORMATTERS_H__
