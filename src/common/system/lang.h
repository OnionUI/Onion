#ifndef SYSTEM_LANG_H__
#define SYSTEM_LANG_H__

#include <stdbool.h>

#include "utils/file.h"
#include "utils/str.h"
#include "utils/json.h"
#include "utils/log.h"
#include "./settings.h"

#define LANG_MAX 150
#define LANG_DEFAULT "en.lang"
#define LANG_DIR "/mnt/SDCARD/miyoo/app/lang"
#define LANG_DIR_FALLBACK "/customer/app/lang"

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
    LANG_EXIT = 111,
    LANG_SAVE_EXIT
} lang_hash;

bool lang_getFilePath(const char *lang_name, char *lang_path)
{
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

    lang_list = malloc(LANG_MAX * sizeof(char*));

    printf_debug("Loading lang file: %s\n", lang_path);

    cJSON *lang_file = json_load(lang_path);
    cJSON *lang_name = cJSON_GetObjectItem(lang_file, "0");

    char key[32];
    char value[STR_MAX];
    for (int i = 0; i < LANG_MAX; i++) {
        sprintf(key, "%d", i);
        if (json_getString(lang_file, key, value)) {
            lang_list[i] = malloc(STR_MAX * sizeof(char));
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
