#ifndef SYSTEM_STATE_H__
#define SYSTEM_STATE_H__

#include <unistd.h>

#include "utils/file.h"
#include "utils/flags.h"
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
    if (!exists(CMD_TO_RUN_PATH))
        return false;
    const char *cmd = file_read(CMD_TO_RUN_PATH);
    if (strstr(cmd, "retroarch") != NULL ||
        strstr(cmd, "/mnt/SDCARD/Emu/") != NULL ||
        strstr(cmd, "/mnt/SDCARD/RApp/") != NULL) {
        pid_t pid;
        if ((pid = process_searchpid("retroarch")) != 0 ||
            (pid = process_searchpid("ra32")) != 0) {
            system_state_pid = pid;
            return true;
        }
    }
    return false;
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

    if (exists(RECENTLIST_PATH))
        sprintf(filename, "%s", RECENTLIST_PATH);
    else
        sprintf(filename, "%s", RECENTLIST_HIDDEN_PATH);

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
        perror("Erreur lors de l'ouverture du fichier");
        return NULL; // Renvoie NULL en cas d'erreur d'ouverture du fichier
    }

    while (fgets(line, STR_MAX * 3, file) != NULL) {
        char *jsonContent = (char *)malloc(strlen(line) + 1);
        char romPathSearch[STR_MAX];
        int type;

        strcpy(jsonContent, line);
        sscanf(strstr(jsonContent, "\"type\":") + 7, "%d", &type);

        if (type != 5) {
            free(jsonContent);
            continue;
        }

        const char *rompathStart = strstr(jsonContent, "\"rompath\":\"") + 11;
        const char *rompathEnd = strchr(rompathStart, '\"');

        strncpy(romPathSearch, rompathStart, rompathEnd - rompathStart);
        romPathSearch[rompathEnd - rompathStart] = '\0';

        free(jsonContent);

        if (!exists(romPathSearch))
            continue;

        strcpy(rom_path, romPathSearch);

        fclose(file);
        return rom_path;
    }

    fclose(file);
    return NULL;
}

void resumeGame(int n)
{
    FILE *file = fopen(getMiyooRecentFilePath(), "r");
    struct JsonData {
        char label[256];
        char rompath[256];
        char imgpath[256];
        char launch[256];
        int type;
    };

    if (!file) {
        fprintf(stderr, "Can't open file %s\n", getMiyooRecentFilePath());
        return;
    }

    char buffer[STR_MAX * 4];
    int type5Count = 0;
    int lineCount = 0;

    while (fgets(buffer, sizeof(buffer), file) != NULL) {
        lineCount++;
        struct JsonData jsonData;

        if (sscanf(buffer, "{\"label\":\"%[^\"]\",\"rompath\":\"%[^\"]\",\"imgpath\":\"%[^\"]\",\"launch\":\"%[^\"]\",\"type\":%d}",
                   jsonData.label, jsonData.rompath, jsonData.imgpath, jsonData.launch, &jsonData.type) == 5) {
            if (jsonData.type == 5) {
                type5Count++;
                if (type5Count == n) {
                    FILE *fp;
                    char RACommand[STR_MAX * 3];

                    fclose(file);
                    sprintf(RACommand, "LD_PRELOAD=/mnt/SDCARD/miyoo/app/../lib/libpadsp.so  \"%s\" \"%s\"", jsonData.launch, jsonData.rompath);

                    remove("/mnt/SDCARD/.tmp_update/.runGameSwitcher");
                    printf_debug("resume game: %s\n", RACommand);

                    if (lineCount > 1) {
                        temp_flag_set("quick_switch", true);

                        file_add_line_to_beginning(getMiyooRecentFilePath(), file_read_lineN(getMiyooRecentFilePath(), lineCount));
                        file_delete_line(getMiyooRecentFilePath(), lineCount + 1);
                    }

                    file_put_sync(fp, CMD_TO_RUN_PATH, "%s", RACommand);

                    sync();
                    return;
                }
            }
        }
    }
    fclose(file);
}

void set_resumeGame(void)
{
    resumeGame(1);
}

void set_quickSwitch(void)
{
    resumeGame(2);
}

#endif // SYSTEM_STATE_H__
