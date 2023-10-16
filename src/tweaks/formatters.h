#ifndef TWEAKS_FORMATTERS_H__
#define TWEAKS_FORMATTERS_H__

#include <stdio.h>
#include <sys/statvfs.h>
#include "components/list.h"
#include "utils/apps.h"
#include "utils/str.h"

#include "./tools.h"

#define BATTPERC_MAX_OFFSET 48

#define BUTTON_MAINUI_LABELS                          \
    {                                                 \
        "Context menu", "GameSwitcher", "Resume game" \
    }
#define BUTTON_INGAME_LABELS                                  \
    {                                                         \
        "Off", "GameSwitcher", "Exit to menu", "Quick switch" \
    }

#define THEME_TOGGLE_LABELS \
    {                       \
        "-", "Off", "On"    \
    }

#define SLEEP_TIMER_LABELS                 \
    {                                      \
        "Off", "5 min", "15 min", "30 min" \
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

void formatter_Serial(void *pt, char *out_label)
{

    static char serial[32];
    if (strlen(serial) == 0)
        process_start_read_return("serial", serial);
    strcpy(out_label, serial);
}

void formatter_Language(void *pt, char *out_label)
{
    ListItem *item = (ListItem *)pt;
    strcpy(out_label, all_language_names[item->value]);
}

void formatter_OnionVersion(void *pt, char *out_label)
{
    FILE *fp;
    static char onion_version[32] = "";
    if (strlen(onion_version) == 0)
        file_get(fp, "/mnt/SDCARD/.tmp_update/onionVersion/version.txt", "%s", onion_version);
    if (strlen(onion_version) > 0)
        strcpy(out_label, onion_version);
    else
        strcpy(out_label, "0.0.0");
}

void formatter_FirmwareVersion(void *pt, char *out_label)
{
    static char firmware_version[16] = "";
    if (strlen(firmware_version) == 0)
        process_start_read_return("/etc/fw_printenv miyoo_version | sed 's/[^0-9]//g'", firmware_version);
    if (strlen(firmware_version) > 0)
        sprintf(out_label, "%s", firmware_version);
    else
        strcpy(out_label, "XXXXXXXXXXXX");
}

void formatter_CPUFrequency(void *pt, char *out_label)
{
    static char cpu_frequency[32] = "";
    if (strlen(cpu_frequency) == 0)
        process_start_read_return("cat /sys/devices/system/cpu/cpu0/cpufreq/cpuinfo_max_freq", cpu_frequency);

    if (strlen(cpu_frequency) > 0)
        sprintf(out_label, "%.4s MHz", cpu_frequency);
    else
        strcpy(out_label, "0 MHz");
}

void formatter_Memory(void *pt, char *out_label)
{
    static char memory[8] = "";
    if (strlen(memory) == 0)
        process_start_read_return("cat /proc/meminfo | awk '/MemTotal/ {print $2}'", memory);

    int mem = atoi(memory);

    // this is how MainUI calculates it so don't @ me
    mem += 1023; // round up
    mem >>= 10;  // convert to MB (divide by 1024)
    mem += 32;   // add 32 MB because 128 looks better than 96
    mem >>= 5;   // divide by 32
    mem <<= 5;   // multiply by 32 - this rounds to the nearest 32 MB

    if (strlen(memory) > 0)
        sprintf(out_label, "%d MB", mem);
    else
        strcpy(out_label, "0 MB");
}

void formatter_Storage(void *pt, char *out_label)
{
    struct statvfs stat;
    static char storage[32] = "";
    if (strlen(storage) == 0) {
        if (statvfs("/mnt/SDCARD", &stat) == 0) {
            unsigned long long block_size = stat.f_frsize;
            unsigned long long total_blocks = stat.f_blocks;
            unsigned long long free_blocks = stat.f_bfree;

            unsigned long long total_space = block_size * total_blocks;
            unsigned long long free_space = block_size * free_blocks;
            unsigned long long used_space = total_space - free_space;

            double total_gb = (double)total_space / (1 << 30);
            double used_gb = (double)used_space / (1 << 30);

            sprintf(storage, "%.2lf/%.2lf GB", used_gb, total_gb);
        }
        else {
            strcpy(storage, "failed");
        }
    }
    strcpy(out_label, storage);
}
#endif // TWEAKS_FORMATTERS_H__
