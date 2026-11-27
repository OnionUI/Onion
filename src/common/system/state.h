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

#define RECENT_LINE_MAX (STR_MAX * 6)
#define RECENT_PATH_MAX (STR_MAX * 2)

//
//    [onion] parse one line of the miyoo recent list
//    Returns false if the line is not valid JSON, is not a game entry
//    (type 5 or 17), or if a path does not fit the given buffers.
//
static bool history_parseRecentLine(const char *line, char *launch, size_t launch_size,
                                    char *rompath, size_t rompath_size)
{
    bool ok = false;
    cJSON *root = cJSON_Parse(line);
    if (root == NULL)
        return false;

    cJSON *type = cJSON_GetObjectItemCaseSensitive(root, "type");
    cJSON *rp = cJSON_GetObjectItemCaseSensitive(root, "rompath");
    cJSON *ln = cJSON_GetObjectItemCaseSensitive(root, "launch");

    if (!cJSON_IsNumber(type) || (type->valueint != 5 && type->valueint != 17))
        goto done;
    if (!cJSON_IsString(rp) || rp->valuestring == NULL)
        goto done;

    const char *rompath_str = rp->valuestring;
    const char *colon = strchr(rompath_str, ':');

    if (colon != NULL) {
        // Game launched with the search panel: "launch:rompath"
        size_t launch_len = (size_t)(colon - rompath_str);
        if (launch_len >= launch_size || strlen(colon + 1) >= rompath_size)
            goto done;
        memcpy(launch, rompath_str, launch_len);
        launch[launch_len] = '\0';
        strcpy(rompath, colon + 1);
    }
    else {
        if (strlen(rompath_str) >= rompath_size)
            goto done;
        strcpy(rompath, rompath_str);

        launch[0] = '\0';
        if (cJSON_IsString(ln) && ln->valuestring != NULL) {
            if (strlen(ln->valuestring) >= launch_size)
                goto done;
            strcpy(launch, ln->valuestring);
        }
    }

    ok = true;

done:
    cJSON_Delete(root);
    return ok;
}

//
//    [onion] get recent rom path from miyoo recent list
//    rom_path must hold at least rom_path_size bytes
//
char *history_getRecentPath(char *rom_path, size_t rom_path_size)
{
    FILE *file;
    char line[RECENT_LINE_MAX];
    char launch[RECENT_PATH_MAX];
    char romPathSearch[RECENT_PATH_MAX];
    char *result = NULL;

    file = fopen(getMiyooRecentFilePath(), "r");

    if (file == NULL) {
        return NULL;
    }

    // Only the most recent entry is relevant
    if (fgets(line, sizeof(line), file) != NULL &&
        history_parseRecentLine(line, launch, sizeof(launch),
                                romPathSearch, sizeof(romPathSearch))) {
        printf_debug("romPathSearch : %s\n", romPathSearch);

        if (exists(romPathSearch) && strlen(romPathSearch) < rom_path_size) {
            strcpy(rom_path, romPathSearch);
            result = rom_path;
        }
    }

    fclose(file);
    return result;
}

bool history_getRomscreenPath(char *path_out)
{
    char file_path[RECENT_PATH_MAX];

    if (history_getRecentPath(file_path, sizeof(file_path)) == NULL) {
        return false;
    }

    print_debug(file_path);
    sprintf(path_out, "/mnt/SDCARD/Saves/CurrentProfile/romScreens/%" PRIu32 ".png",
            FNV1A_Pippip_Yurii(file_path, strlen(file_path)));
    return true;
}

void resumeGame(int index)
{
    const char *recentPath = getMiyooRecentFilePath();
    FILE *file = fopen(recentPath, "r");

    if (!file) {
        fprintf(stderr, "Can't open file %s\n", recentPath);
        return;
    }

    // getline() so that lineCount matches the line numbers used by
    // file_read_lineN() and file_delete_line(), whatever the line length
    char *jsonContent = NULL;
    size_t jsonContentSize = 0;
    int validGameCount = -1;
    int lineCount = 0;

    while (getline(&jsonContent, &jsonContentSize, file) != -1) {
        char rompath[RECENT_PATH_MAX];
        char launch[RECENT_PATH_MAX];
        lineCount++;

        if (!history_parseRecentLine(jsonContent, launch, sizeof(launch),
                                     rompath, sizeof(rompath)))
            continue;

        printf_debug("launch: %s\n", launch);
        printf_debug("rompath: %s\n", rompath);

        if (!exists(rompath) || !exists(launch))
            continue;

        ++validGameCount;

        if (validGameCount == index) {

            FILE *fp;
            char LaunchCommand[RECENT_PATH_MAX * 2 + 64];

            fclose(file);
            file = NULL;
            snprintf(LaunchCommand, sizeof(LaunchCommand), "LD_PRELOAD=/mnt/SDCARD/miyoo/app/../lib/libpadsp.so \"%s\" \"%s\"", launch, rompath);

            remove("/mnt/SDCARD/.tmp_update/.runGameSwitcher");

            // move selected rom to top of recent list for quick switch
            if (lineCount > 1) {
                temp_flag_set("quick_switch", true);

                char *line_n = file_read_lineN(recentPath, lineCount);
                if (line_n != NULL) {
                    file_add_line_to_beginning(recentPath, line_n);
                    file_delete_line(recentPath, lineCount + 1);
                    free(line_n);
                }
            }

            file_put_sync(fp, CMD_TO_RUN_PATH, "%s", LaunchCommand);
            printf_debug("resume game: %s\n", LaunchCommand);

            temp_flag_set("force_auto_load_state", true);

            sync();
            break;
        }
    }

    free(jsonContent);

    if (file != NULL) {
        fclose(file);
    }
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
