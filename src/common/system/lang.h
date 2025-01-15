#ifndef SYSTEM_LANG_H__
#define SYSTEM_LANG_H__

#include <stdbool.h>

#include "./settings.h"
#include "utils/file.h"
#include "utils/json.h"
#include "utils/log.h"
#include "utils/str.h"

#define LANG_MAX 400
#define LANG_DEFAULT "en.lang"
#define LANG_DIR "/mnt/SDCARD/miyoo/app/lang"
#define LANG_DIR_FALLBACK "/customer/app/lang"
#define LANG_DIR_BACKUP "/mnt/SDCARD/miyoo/app/lang_backup"

#define LANG_FALLBACK_SELECT "SELECT"
#define LANG_FALLBACK_BACK "BACK"
#define LANG_FALLBACK_OK "OK"
#define LANG_FALLBACK_CANCEL "CANCEL"
#define LANG_FALLBACK_NEXT "NEXT"
#define LANG_FALLBACK_EXIT "EXIT"
#define LANG_FALLBACK_RESUME_UC "RESUME"

#define LANG_FALLBACK_RECENTS_TAB "Recents"
#define LANG_FALLBACK_FAVORITES_TAB "Favorites"
#define LANG_FALLBACK_GAMES_TAB "Games"
#define LANG_FALLBACK_EXPERT_TAB "Expert"
#define LANG_FALLBACK_APPS_TAB "Apps"

#define LANG_FALLBACK_RESUME "Resume"
#define LANG_FALLBACK_SAVE "Save"
#define LANG_FALLBACK_LOAD "Load"
#define LANG_FALLBACK_EXIT_TO_MENU "Exit to menu"
#define LANG_FALLBACK_ADVANCED "Advanced"

static char **lang_list = NULL;

typedef enum {
    LANG_EXPERT_TAB = 0,
    LANG_FAVORITES_TAB = 1,
    LANG_GAMES_TAB = 2,
    LANG_SETTINGS_TAB = 15,
    LANG_RECENTS_TAB = 18,
    LANG_CHARGING = 40,
    LANG_CANCEL = 45,
    LANG_OK = 46,
    LANG_LOADING_SCREEN = 75,
    LANG_LOADING_TITLE = 76,
    LANG_SELECT = 88,
    LANG_BACK = 89,
    LANG_MENU = 91,
    LANG_RESUME = 92,
    LANG_SAVE = 93,
    LANG_LOAD = 94,
    LANG_EXIT_TO_MENU = 96,
    LANG_ADVANCED = 106,
    LANG_APPS_TAB = 107,
    LANG_EXIT = 111,
    LANG_SAVE_EXIT = 112,
    LANG_NEXT = 300,
    LANG_RESUME_UC = 301
} lang_hash;

void lang_removeIconLabels(bool remove_icon_labels, bool remove_hints)
{
    DIR *dp;
    struct dirent *ep;

    if ((dp = opendir(LANG_DIR)) == NULL)
        return;

    if (!remove_icon_labels || !remove_hints) {
        // restore original lang files
        if (exists(LANG_DIR_BACKUP)) {
            system("mv -f " LANG_DIR_BACKUP "/* " LANG_DIR "");
            remove(LANG_DIR_BACKUP);
        }
    }

    if (!remove_icon_labels && !remove_hints) {
        closedir(dp);
        return;
    }

    // backup lang files
    if (!exists(LANG_DIR_BACKUP))
        system("cp -R " LANG_DIR " " LANG_DIR_BACKUP "");

    while ((ep = readdir(dp))) {
        if (ep->d_type != DT_REG)
            continue; // skip non-regular files and folders
        if (strcmp("lang", file_getExtension(ep->d_name)) != 0)
            continue; // skip files not having the `.lang` extension

        char file_path[STR_MAX * 2];
        snprintf(file_path, STR_MAX * 2 - 1, LANG_DIR "/%s", ep->d_name);

        char *json_data = file_read(file_path);
        cJSON *root = cJSON_Parse(json_data);
        free(json_data);

        if (!root)
            continue;

        printf_debug("Lang: %s (%s)\n",
                     cJSON_GetStringValue(cJSON_GetObjectItem(root, "lang")),
                     ep->d_name);

        if (remove_icon_labels) {
            json_setString(root, "0", " ");   // Expert
            json_setString(root, "1", " ");   // Favorites
            json_setString(root, "2", " ");   // Games
            json_setString(root, "15", " ");  // Settings
            json_setString(root, "18", " ");  // Recents
            json_setString(root, "107", " "); // Apps
        }

        if (remove_hints) {
            json_setString(root, "45", " ");  // CANCEL
            json_setString(root, "46", " ");  // OK
            json_setString(root, "88", " ");  // SELECT
            json_setString(root, "89", " ");  // BACK
            json_setString(root, "111", " "); // EXIT
            json_setString(root, "112", " "); // SAVE AND EXIT
            json_setString(root, "300", " "); // NEXT
        }

        json_save(root, file_path);
        cJSON_Delete(root);
    }
    closedir(dp);
}

bool lang_getFilePath(const char *lang_name, char *lang_path)
{
    sprintf(lang_path, LANG_DIR_BACKUP "/%s", lang_name);
    if (exists(lang_path))
        return true;
    sprintf(lang_path, LANG_DIR "/%s", lang_name);
    if (exists(lang_path))
        return true;
    sprintf(lang_path, LANG_DIR_FALLBACK "/%s", lang_name);
    return exists(lang_path);
}

bool lang_load(void)
{
    if (!settings_loaded)
        settings_load();

    char lang_path[STR_MAX];

    if (!lang_getFilePath(settings.language, lang_path) &&
        !lang_getFilePath(LANG_DEFAULT, lang_path))
        return false;

    lang_list = (char **)malloc(LANG_MAX * sizeof(char *));
    memset(lang_list, 0, LANG_MAX * sizeof(char *));

    cJSON *lang_file = json_load(lang_path);

    char key[32];
    char value[STR_MAX];
    for (int i = 0; i < LANG_MAX; i++) {
        sprintf(key, "%d", i);
        if (json_getString(lang_file, key, value)) {
            lang_list[i] = (char *)malloc(STR_MAX * sizeof(char));
            strcpy(lang_list[i], value);
        }
    }

    cJSON_Delete(lang_file);

    return true;
}

void lang_free(void)
{
    for (int i = 0; i < LANG_MAX; i++) {
        if (lang_list[i] == NULL)
            continue;
        free(lang_list[i]);
    }
    free(lang_list);
}

const char *lang_get(lang_hash key, const char *fallback)
{
    if (lang_list && lang_list[key])
        return lang_list[key];
    return fallback;
}

#endif // SYSTEM_LANG_H__
