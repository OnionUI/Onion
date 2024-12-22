#ifndef UTILS_APPLY_ICONS_H__
#define UTILS_APPLY_ICONS_H__

#include <dirent.h>
#include <libgen.h>
#include <sys/file.h>
#include <sys/types.h>

#include "utils/file.h"
#include "utils/json.h"
#include "utils/log.h"
#include "utils/str.h"

#define ICON_PACK_DEFAULT "/mnt/SDCARD/Icons/Default"

#define CONFIG_EMU_PATH "/mnt/SDCARD/Emu"
#define CONFIG_APP_PATH "/mnt/SDCARD/App"
#define CONFIG_RAPP_PATH "/mnt/SDCARD/RApp"
#define ACTIVE_ICON_PACK "/mnt/SDCARD/.tmp_update/config/active_icon_pack"
#define SEARCH_CONFIG_SRC "/mnt/SDCARD/.tmp_update/res/search_config.json"
#define SEARCH_CONFIG "/mnt/SDCARD/Emu/SEARCH/config.json"

#define GUEST_DIR "/mnt/SDCARD/Saves/GuestProfile"
#define GUEST_CONFIG "/mnt/SDCARD/App/Guest_Mode/config.json"
#define GUEST_ON_CONFIG "/mnt/SDCARD/App/Guest_Mode/data/configON.json"
#define GUEST_OFF_CONFIG "/mnt/SDCARD/App/Guest_Mode/data/configOFF.json"

typedef enum IconMode {
    ICON_MODE_EMU,
    ICON_MODE_APP,
    ICON_MODE_RAPP
} IconMode_e;

IconMode_e icons_getIconMode(const char *config_path)
{
    if (strncmp(CONFIG_APP_PATH, config_path, strlen(CONFIG_APP_PATH)) == 0)
        return ICON_MODE_APP;
    if (strncmp(CONFIG_RAPP_PATH, config_path, strlen(CONFIG_RAPP_PATH)) == 0)
        return ICON_MODE_RAPP;
    return ICON_MODE_EMU;
}

void _saveConfigFile(const char *config_path, const char *content)
{
    FILE *config_file = fopen(config_path, "w+");
    fprintf(config_file, "%s", content);
    fflush(config_file);
    fclose(config_file);

    if (strcmp(SEARCH_CONFIG_SRC, config_path) == 0 && is_file(SEARCH_CONFIG))
        system("cp \"" SEARCH_CONFIG_SRC "\" \"" SEARCH_CONFIG "\"");
    else if (strcmp(GUEST_OFF_CONFIG, config_path) == 0) {
        if (is_dir(GUEST_DIR)) // main profile
            system("cp \"" GUEST_OFF_CONFIG "\" \"" GUEST_CONFIG "\"");
    }
    else if (strcmp(GUEST_ON_CONFIG, config_path) == 0) {
        if (!is_dir(GUEST_DIR)) // guest profile
            system("cp \"" GUEST_ON_CONFIG "\" \"" GUEST_CONFIG "\"");
    }
}

bool apply_singleIconByFullPath(const char *config_path, const char *icon_path)
{
    if (!is_file(config_path) || !is_file(icon_path))
        return false;

    char temp_path[STR_MAX];
    strncpy(temp_path, icon_path, STR_MAX - 1);

    const char *file_name = basename(temp_path);
    const char *dir_path = dirname(temp_path);

    cJSON *config = json_load(config_path);

    char sel_path[STR_MAX];
    sprintf(sel_path, "%s/sel/%s", dir_path, file_name);

    if (!is_file(sel_path))
        strcpy(sel_path, icon_path);

    json_forceSetString(config, "icon", icon_path);
    json_forceSetString(config, "iconsel", sel_path);

    _saveConfigFile(config_path, cJSON_Print(config));
    cJSON_Delete(config);

    return true;
}

const char *icons_getIconNameFormat(IconMode_e mode)
{
    switch (mode) {
    case ICON_MODE_APP:
        return "app/%s";
    case ICON_MODE_RAPP:
        return "rapp/%s";
    default:
        break;
    }

    return "%s";
}

const char *icons_getIconPathFormat(IconMode_e mode)
{
    switch (mode) {
    case ICON_MODE_APP:
        return "%s/app/%s.png";
    case ICON_MODE_RAPP:
        return "%s/rapp/%s.png";
    default:
        break;
    }

    return "%s/%s.png";
}

const char *icons_getSelectedIconPathFormat(IconMode_e mode)
{
    switch (mode) {
    case ICON_MODE_APP:
        return "%s/app/sel/%s.png";
    case ICON_MODE_RAPP:
        return "%s/rapp/sel/%s.png";
    default:
        break;
    }

    return "%s/sel/%s.png";
}

bool _apply_singleIconFromPack(const char *config_path,
                               const char *icon_pack_path, bool reset_default)
{
    if (!is_file(config_path))
        return false;

    cJSON *config = json_load(config_path);
    char temp_path[STR_MAX];
    if (!json_getString(config, "icon", temp_path))
        return false;

    char *icon_name = file_removeExtension(basename(temp_path));
    str_split(icon_name, "-");

    IconMode_e mode = icons_getIconMode(config_path);

    char icon_path[STR_MAX];
    sprintf(icon_path, icons_getIconPathFormat(mode), icon_pack_path,
            icon_name);

    if (!is_file(icon_path)) {
        if (reset_default) {
            sprintf(icon_path, icons_getIconPathFormat(mode), ICON_PACK_DEFAULT,
                    icon_name);
        }

        if (!is_file(icon_path)) {
            free(icon_name);
            return false;
        }
    }

    char sel_path[STR_MAX];
    sprintf(sel_path, icons_getSelectedIconPathFormat(mode), icon_pack_path,
            icon_name);
    free(icon_name);

    if (is_file(sel_path))
        json_forceSetString(config, "iconsel", sel_path);
    else
        cJSON_DeleteItemFromObject(config, "iconsel");

    json_forceSetString(config, "icon", icon_path);

    char* config_str = cJSON_Print(config);
    _saveConfigFile(config_path, config_str);
    cJSON_free(config_str);
    cJSON_Delete(config);

    printf_debug("Applied icon to %s\nicon:    %s\niconsel: %s\n", config_path,
                 icon_path, sel_path);

    return true;
}

bool apply_singleIcon(const char *config_path)
{
    char icon_pack_path[STR_MAX];
    char *active_icon_pack = file_read(ACTIVE_ICON_PACK);

    if (active_icon_pack != NULL && is_dir(active_icon_pack))
        strncpy(icon_pack_path, active_icon_pack, STR_MAX - 1);
    else {
        strcpy(icon_pack_path, "/mnt/SDCARD/Icons/Default");
    }

    if (!is_dir(icon_pack_path))
        return false;

    if (strcmp(GUEST_CONFIG, config_path) == 0) {
        _apply_singleIconFromPack(GUEST_OFF_CONFIG, icon_pack_path, false);
        _apply_singleIconFromPack(GUEST_ON_CONFIG, icon_pack_path, false);
    }

    free(active_icon_pack);
    return _apply_singleIconFromPack(config_path, icon_pack_path, false);
}

int _apply_iconPackOnConfigs(const char *path, const char *icon_pack_path,
                             bool reset_default)
{
    DIR *dp;
    struct dirent *ep;
    char config_path[STR_MAX * 2];
    int count = 0;

    if ((dp = opendir(path)) != NULL) {
        while ((ep = readdir(dp))) {
            if (ep->d_type != DT_DIR)
                continue;
            if (ep->d_name[0] == '.')
                continue;
            if (strcmp("romscripts", ep->d_name) == 0)
                continue;

            snprintf(config_path, STR_MAX * 2 - 1, "%s/%s/config.json", path,
                     ep->d_name);

            if (strcmp(SEARCH_CONFIG, config_path) == 0)
                continue;
            if (strcmp(GUEST_CONFIG, config_path) == 0)
                continue;

            if (!is_file(config_path))
                continue;

            if (_apply_singleIconFromPack(config_path, icon_pack_path,
                                          reset_default))
                count++;
        }
        closedir(dp);
    }

    return count;
}

int apply_iconPack(const char *icon_pack_path, bool reset_default)
{
    FILE *fp;
    file_put_sync(fp, ACTIVE_ICON_PACK, "%s", icon_pack_path);

    int count = 0;

    count += _apply_iconPackOnConfigs(CONFIG_EMU_PATH, icon_pack_path,
                                      reset_default);
    count += _apply_iconPackOnConfigs(CONFIG_APP_PATH, icon_pack_path,
                                      reset_default);
    count += _apply_iconPackOnConfigs(CONFIG_RAPP_PATH, icon_pack_path,
                                      reset_default);

    if (_apply_singleIconFromPack(SEARCH_CONFIG_SRC, icon_pack_path,
                                  reset_default))
        count++;
    if (_apply_singleIconFromPack(GUEST_ON_CONFIG, icon_pack_path,
                                  reset_default))
        count++;
    if (_apply_singleIconFromPack(GUEST_OFF_CONFIG, icon_pack_path,
                                  reset_default))
        count++;

    return count;
}

#endif // UTILS_APPLY_ICONS_H__
