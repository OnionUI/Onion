#ifndef TWEAKS_FORMATTERS_H__
#define TWEAKS_FORMATTERS_H__

#include <stdio.h>

#include "utils/str.h"
#include "components/list.h"

#define BUTTON_MAINUI_LABELS {"Context menu", "GameSwitcher", "Resume game"}
#define BUTTON_INGAME_LABELS {"Off", "GameSwitcher", "Exit to menu", "Quick switch"}

#define THEME_TOGGLE_LABELS {"-", "Off", "On"}

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
	sprintf(out_label, "%d.0x", ((ListItem*)pt)->value);
}

void formatter_positionOffset(void *pt, char *out_label)
{
	ListItem *item = (ListItem*)pt;
	if (item->value == 0)
		strcpy(out_label, "-");
	else sprintf(out_label, "%d px", item->value - 11);
}

#endif // TWEAKS_FORMATTERS_H__
