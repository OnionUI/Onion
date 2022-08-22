#ifndef SYSTEM_LANG_H__
#define SYSTEM_LANG_H__

#include <stdbool.h>

#include "utils/file.h"
#include "utils/str.h"
#include "utils/json.h"
#include "utils/log.h"
#include "./settings.h"

#ifndef DT_REG
#define DT_REG 8
#endif

#define LANG_MAX 150
#define LANG_DEFAULT "en.lang"
#define LANG_DIR "/mnt/SDCARD/miyoo/app/lang"
#define LANG_DIR_FALLBACK "/customer/app/lang"
#define LANG_DIR_BACKUP "/mnt/SDCARD/miyoo/app/lang_backup"

void lang_removeIconLabels(bool remove_icon_labels, bool remove_hints)
{
    DIR *dp;
    struct dirent *ep;

	if ((dp = opendir(LANG_DIR)) == NULL)
        return;

    if (!remove_icon_labels && !remove_hints) {
        // restore original lang files
        if (exists(LANG_DIR_BACKUP)) {
            system("mv -f " LANG_DIR_BACKUP "/* " LANG_DIR "");
            remove(LANG_DIR_BACKUP);
        }
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

        const char *json_data = file_read(file_path);
        cJSON *root = cJSON_Parse(json_data);

        if (!root)
            continue;

        printf_debug("Lang: %s (%s)\n", cJSON_GetStringValue(cJSON_GetObjectItem(root, "lang")), ep->d_name);

        if (remove_icon_labels) {
            json_setString(root, "0", " "); // Expert
            json_setString(root, "1", " "); // Favorites
            json_setString(root, "2", " "); // Games
            json_setString(root, "15", " "); // Settings
            json_setString(root, "18", " "); // Recents
            json_setString(root, "107", " "); // Apps
        }

        if (remove_hints) {
            json_setString(root, "45", " "); // CANCEL
            json_setString(root, "46", " "); // OK
            json_setString(root, "88", " "); // SELECT
            json_setString(root, "89", " "); // BACK
            json_setString(root, "111", " "); // EXIT
            json_setString(root, "112", " "); // SAVE AND EXIT
        }

        json_save(root, file_path);
        cJSON_free(root);
    }
    closedir(dp);
}

static char **lang_list = NULL;

typedef enum
{
    LANG_EXPERT_TAB,
    LANG_FAVORITES_TAB,
    LANG_GAMES_TAB,
    LANG_SETTINGS_TAB = 15,
    LANG_RECENTS_TAB = 18,
    LANG_CHARGING = 40,
    LANG_CANCEL = 45,
    LANG_OK,
    LANG_LOADING_SCREEN = 75,
    LANG_LOADING_TITLE,
    LANG_SELECT = 88,
    LANG_BACK,
    LANG_MENU = 91,
    LANG_APPS_TAB = 107,
    LANG_EXIT = 111,
    LANG_SAVE_EXIT
} lang_hash;

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

    if (!lang_getFilePath(settings.language, lang_path) && !lang_getFilePath(LANG_DEFAULT, lang_path))
        return false;

    lang_list = (char**)malloc(LANG_MAX * sizeof(char*));

    cJSON *lang_file = json_load(lang_path);

    char key[32];
    char value[STR_MAX];
    for (int i = 0; i < LANG_MAX; i++) {
        sprintf(key, "%d", i);
        if (json_getString(lang_file, key, value)) {
            lang_list[i] = (char*)malloc(STR_MAX * sizeof(char));
            strcpy(lang_list[i], value);
        }
    }

    cJSON_free(lang_file);
    
    return true;
}

void lang_free(void)
{
    free(lang_list);
}

char* lang_get(lang_hash key)
{
    if (lang_list[key])
        return lang_list[key];
    return "";
}

#endif // SYSTEM_LANG_H__
