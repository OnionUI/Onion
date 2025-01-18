#ifndef SYSTEM_STATE_H__
#define SYSTEM_STATE_H__

#include <unistd.h>

#include "utils/file.h"
#include "utils/flags.h"
#include "utils/hash.h"
#include "utils/log.h"
#include "utils/process.h"
#include "utils/str.h"

#include "./display.h"
#include "./settings.h"

typedef enum system_state_e {
    MODE_UNKNOWN,
    MODE_MAIN_UI,
    MODE_SWITCHER,
    MODE_GAME,
    MODE_APPS,
    MODE_ADVMENU,
    MODE_DRASTIC
} SystemState;
static SystemState system_state = MODE_UNKNOWN;
static pid_t system_state_pid = 0;

bool check_isRetroArch(void)
{
    bool rc = false;
    if (!exists(CMD_TO_RUN_PATH))
        return false;
    char *cmd = file_read(CMD_TO_RUN_PATH);
    if (strstr(cmd, "retroarch") != NULL ||
        strstr(cmd, "/mnt/SDCARD/Emu/") != NULL ||
        strstr(cmd, "/mnt/SDCARD/RApp/") != NULL) {
        pid_t pid;
        if ((pid = process_searchpid("retroarch")) != 0 ||
            (pid = process_searchpid("ra32")) != 0) {
            system_state_pid = pid;
            rc = true;
        }
    }
    free(cmd);
    return rc;
}

bool check_isMainUI(void)
{
    pid_t pid;
    if (!exists(CMD_TO_RUN_PATH) && (pid = process_searchpid("MainUI")) != 0) {
        system_state_pid = pid;
        return true;
    }
    return false;
}

bool check_isAdvMenu(void)
{
    pid_t pid;
    if (!exists(CMD_TO_RUN_PATH) && (pid = process_searchpid("advmenu")) != 0) {
        system_state_pid = pid;
        return true;
    }
    return false;
}

bool check_isGameSwitcher(void)
{
    pid_t pid;
    if (exists("/mnt/SDCARD/.tmp_update/.runGameSwitcher") &&
        (pid = process_searchpid("gameSwitcher")) != 0) {
        system_state_pid = pid;
        return true;
    }
    return false;
}

bool check_isDrastic(void)
{
    pid_t pid;
    if (exists(CMD_TO_RUN_PATH) && (pid = process_searchpid("drastic")) != 0) {
        system_state_pid = pid;
        return true;
    }
    return false;
}

void system_state_update(void)
{
    if (check_isGameSwitcher())
        system_state = MODE_SWITCHER;
    else if (check_isRetroArch())
        system_state = MODE_GAME;
    else if (check_isMainUI())
        system_state = MODE_MAIN_UI;
    else if (check_isAdvMenu())
        system_state = MODE_ADVMENU;
    else if (check_isDrastic())
        system_state = MODE_DRASTIC;
    else
        system_state = MODE_APPS;

#ifdef LOG_DEBUG
    switch (system_state) {
    case MODE_MAIN_UI:
        print_debug("System state: Main UI");
        break;
    case MODE_SWITCHER:
        print_debug("System state: Game Switcher");
        break;
    case MODE_GAME:
        print_debug("System state: RetroArch");
        break;
    case MODE_APPS:
        print_debug("System state: Apps");
        break;
    default:
        print_debug("System state: Unknown");
        break;
    }
#endif
}

size_t state_getAppName(char *out, const char *str)
{
    char *end;
    size_t out_size;

    str += 19;
    end = (char *)strchr(str, ';');

    out_size = (end - str) < STR_MAX - 1 ? (end - str) : STR_MAX - 1;
    memcpy(out, str, out_size);
    out[out_size] = 0;

    return out_size;
}

typedef enum mainui_states {
    MAIN_MENU,
    RECENTS,
    FAVORITES,
    GAMES,
    EXPERT,
    APPS
} MainUIState;

void write_mainui_state(MainUIState state, int currpos, int total)
{
    FILE *fp;
    char state_str[STR_MAX];
    int title_num = 0, page_type = 0, page_size = 6, page_start = 0, page_end,
        main_currpos = 0, main_page_start = 0, main_page_end;

    switch (state) {
    case MAIN_MENU:
        remove("/tmp/state.json");
        return;
    case RECENTS:
        title_num = 18;
        page_type = 10;
        main_currpos = 0;
        break;
    case FAVORITES:
        title_num = 1;
        page_type = 2;
        main_currpos = 1;
        break;
    case GAMES:
        title_num = 2;
        page_type = 1;
        page_size = 8;
        main_currpos = 2;
        break;
    case EXPERT:
        title_num = 0;
        page_type = 16;
        page_size = 9;
        main_currpos = 3;
        break;
    case APPS:
        title_num = 107;
        page_type = 3;
        page_size = 4;
        main_currpos = 4;
        break;
    default:
        return;
    }

    int main_total = 6;
    if (!settings.show_recents) {
        if (main_currpos > 0)
            main_currpos--;
        main_total--;
    }
    if (!settings.show_expert) {
        if (state == APPS)
            main_currpos--;
        main_total--;
    }

    if (main_currpos + 4 > main_total)
        main_page_start = main_total - 4;
    main_page_end = main_page_start + 3;

    if (currpos + page_size > total)
        page_start = total - page_size;
    else
        page_start = currpos;
    page_end = page_start + page_size - 1;

    sprintf(state_str,
            "{\"list\":[{\"title\":157,\"type\":0,\"currpos\":%d,\"pagestart\":"
            "%d,\"pageend\":%d},{\"title\":%d,\"type\":%d,\"currpos\":%d,"
            "\"pagestart\":%d,\"pageend\":%d}]}",
            main_currpos, main_page_start, main_page_end, title_num, page_type,
            currpos, page_start, page_end);

    file_put_sync(fp, "/tmp/state.json", "%s", state_str);
}

//
//    [onion] get miyoo recent file path
//

char *getMiyooRecentFilePath()
{
    static char filename[STR_MAX];

    if (exists(RECENTLIST_HIDDEN_PATH))
        sprintf(filename, "%s", RECENTLIST_HIDDEN_PATH);
    else
        sprintf(filename, "%s", RECENTLIST_PATH);

    return filename;
}

//
//    [onion] get recent rom path from miyoo recent list
//
char *history_getRecentPath(char *rom_path)
{
    FILE *file;
    char line[STR_MAX * 3];

    file = fopen(getMiyooRecentFilePath(), "r");

    if (file == NULL) {
        return NULL;
    }

    while (fgets(line, STR_MAX * 3, file) != NULL) {
        char *jsonContent = (char *)malloc(strlen(line) + 1);
        char romPathSearch[STR_MAX];
        int type;

        strcpy(jsonContent, line);
        sscanf(strstr(jsonContent, "\"type\":") + 7, "%d", &type);

        if ((type != 5) && (type != 17)) {
            free(jsonContent);
            fclose(file);
            return NULL;
        }

        const char *rompathStart = strstr(jsonContent, "\"rompath\":\"") + 11;
        const char *rompathEnd = strchr(rompathStart, '\"');

        strncpy(romPathSearch, rompathStart, rompathEnd - rompathStart);
        romPathSearch[rompathEnd - rompathStart] = '\0';

        free(jsonContent);

        // Game launched with the search panel
        char *colonPosition = strchr(romPathSearch, ':');
        if (colonPosition != NULL) {

            int position = (int)(colonPosition - romPathSearch);
            char secondPart[strlen(romPathSearch) - position];
            strcpy(secondPart, colonPosition + 1);
            strcpy(romPathSearch, secondPart);
        }

        printf_debug("romPathSearch : %s\n", romPathSearch);

        if (!exists(romPathSearch)) {
            fclose(file);
            return NULL;
        }

        strcpy(rom_path, romPathSearch);

        fclose(file);
        return rom_path;
    }

    fclose(file);
    return NULL;
}

bool history_getRomscreenPath(char *path_out)
{
    char filename[STR_MAX];
    char file_path[STR_MAX];

    if (history_getRecentPath(file_path) != NULL) {
        sprintf(filename, "%" PRIu32, FNV1A_Pippip_Yurii(file_path, strlen(file_path)));
    }
    print_debug(file_path);
    if (strlen(filename) > 0) {
        sprintf(path_out, "/mnt/SDCARD/Saves/CurrentProfile/romScreens/%s.png", filename);
        return true;
    }

    return false;
}

void resumeGame(int index)
{
    FILE *file = fopen(getMiyooRecentFilePath(), "r");

    int type;

    if (!file) {
        fprintf(stderr, "Can't open file %s\n", getMiyooRecentFilePath());
        return;
    }

    char jsonContent[STR_MAX * 4];
    int validGameCount = -1;
    int lineCount = 0;

    while (fgets(jsonContent, sizeof(jsonContent), file) != NULL) {
        char label[256];
        char rompath[256];
        char imgpath[256];
        char launch[256];
        lineCount++;

        sscanf(strstr(jsonContent, "\"type\":") + 7, "%d", &type);

        if ((type != 5) && (type != 17))
            continue;

        const char *labelStart = strstr(jsonContent, "\"label\":\"");
        if (labelStart != NULL) {
            labelStart += 9;
            const char *labelEnd = strchr(labelStart, '\"');
            strncpy(label, labelStart, labelEnd - labelStart);
            label[labelEnd - labelStart] = '\0';
        }
        printf_debug("label: %s\n", label);
        const char *rompathStart = strstr(jsonContent, "\"rompath\":\"");
        if (rompathStart != NULL) {
            rompathStart += 11;
            const char *rompathEnd = strchr(rompathStart, '\"');
            strncpy(rompath, rompathStart, rompathEnd - rompathStart);
            rompath[rompathEnd - rompathStart] = '\0';
        }
        printf_debug("rompath: %s\n", rompath);
        const char *imgpathStart = strstr(jsonContent, "\"imgpath\":\"");
        if (imgpathStart != NULL) {
            imgpathStart += 11;
            const char *imgpathEnd = strchr(imgpathStart, '\"');
            strncpy(imgpath, imgpathStart, imgpathEnd - imgpathStart);
            imgpath[imgpathEnd - imgpathStart] = '\0';
        }

        char *colonPosition = strchr(rompath, ':');
        if (colonPosition != NULL) {

            int position = (int)(colonPosition - rompath);

            char firstPart[position + 1];
            strncpy(firstPart, rompath, position);
            firstPart[position] = '\0';

            char secondPart[strlen(rompath) - position];
            strcpy(secondPart, colonPosition + 1);

            strcpy(launch, firstPart);
            strcpy(rompath, secondPart);
            printf_debug("launch cutted: %s\n", launch);
            printf_debug("rompath cutted: %s\n", rompath);
        }
        else {
            const char *launchStart = strstr(jsonContent, "\"launch\":\"");
            if (launchStart != NULL) {
                launchStart += 10;
                const char *launchEnd = strchr(launchStart, '\"');
                strncpy(launch, launchStart, launchEnd - launchStart);
                launch[launchEnd - launchStart] = '\0';
            }
        }

        if (!exists(rompath) || !exists(launch))
            continue;

        ++validGameCount;

        if (validGameCount == index) {

            FILE *fp;
            char LaunchCommand[STR_MAX * 3];

            fclose(file);
            sprintf(LaunchCommand, "LD_PRELOAD=/mnt/SDCARD/miyoo/app/../lib/libpadsp.so \"%s\" \"%s\"", launch, rompath);

            remove("/mnt/SDCARD/.tmp_update/.runGameSwitcher");
            printf_debug("resume game: %s\n", LaunchCommand);

            if (lineCount > 1) {
                temp_flag_set("quick_switch", true);

                char *line_n = file_read_lineN(getMiyooRecentFilePath(), lineCount);
                file_add_line_to_beginning(getMiyooRecentFilePath(), line_n);
                file_delete_line(getMiyooRecentFilePath(), lineCount + 1);
                free(line_n);
            }

            file_put_sync(fp, CMD_TO_RUN_PATH, "%s", LaunchCommand);

            temp_flag_set("force_auto_load_state", true);

            sync();
            return;
        }
    }
    fclose(file);
}

void set_resumeGame(void)
{
    resumeGame(0);
}

void set_quickSwitch(void)
{
    resumeGame(1);
}

#endif // SYSTEM_STATE_H__
