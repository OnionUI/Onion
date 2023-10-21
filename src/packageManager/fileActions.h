#ifndef PACMAN_FILE_ACTIONS_H__
#define PACMAN_FILE_ACTIONS_H__

#include <string.h>

#include "utils/apply_icons.h"
#include "utils/file.h"
#include "utils/json.h"
#include "utils/log.h"
#include "utils/str.h"

#include "./globals.h"

bool checkAppInstalled(const char *basePath, int base_len, int level, bool complete)
{
    char path[1000];
    char pathInstalledApp[1000];

    struct dirent *dp;
    DIR *dir = opendir(basePath);

    int run = 1;
    bool is_installed = true;

    // Unable to open directory stream
    if (!dir)
        return true;

    while ((dp = readdir(dir)) != NULL && run == 1) {
        if (strcmp(dp->d_name, ".") == 0 || strcmp(dp->d_name, "..") == 0)
            continue;

        // Ignore other dirs
        if ((level == 0 && strcmp(dp->d_name, "Emu") > 0 &&
             strcmp(dp->d_name, "App") > 0 && strcmp(dp->d_name, "RApp") > 0) ||
            (level == 1 && strcmp(dp->d_name, "romscripts") == 0))
            continue;

        // Construct new path from our base path
        sprintf(path, "%s/%s", basePath, dp->d_name);

        if (exists(path)) {
            sprintf(pathInstalledApp, "/mnt/SDCARD%s", path + base_len);

            if (!exists(pathInstalledApp))
                is_installed = false;
            else if (dp->d_type == DT_DIR)
                is_installed = checkAppInstalled(path, base_len, level + 1, complete);

            if (!complete && level >= 2 && exists(pathInstalledApp))
                return true;

            if ((complete || level < 2) && !is_installed)
                run = 0;
        }
    }

    closedir(dir);
    return is_installed;
}

bool getConfigPath(char *config_path, const char *data_path, const char *base_dir_name)
{
    char base_dir[STR_MAX];
    snprintf(base_dir, STR_MAX - 1, "%s/%s", data_path, base_dir_name);

    if (!is_dir(base_dir))
        return false;

    struct dirent *dp;
    DIR *dir = opendir(base_dir);

    // Unable to open directory stream
    if (!dir)
        return false;

    while ((dp = readdir(dir)) != NULL) {
        if (strcmp(dp->d_name, ".") == 0 || strcmp(dp->d_name, "..") == 0)
            continue;
        if (dp->d_type != DT_DIR)
            continue;
        sprintf(config_path, "%s/%s/config.json", base_dir, dp->d_name);
        if (!is_file(config_path)) {
            return false;
        }
        break;
    }

    return true;
}

bool hasExtension(const char *file_name, const char *extlist)
{
    const char *file_ext = file_getExtension(file_name);

    if (extlist == NULL || strlen(extlist) == 0)
        return true;

    if (strcasecmp(file_ext, "miyoocmd") == 0)
        return false;

    char extlist_dup[STR_MAX];
    strcpy(extlist_dup, extlist);

    char *token = strtok(extlist_dup, "|");

    while (token != NULL) {
        if (strcasecmp(file_ext, token) == 0)
            return true;
        token = strtok(NULL, "|");
    }

    return false;
}

bool checkRomDir(const char *rom_dir, const char *extlist, int level)
{
    struct dirent *dp;
    DIR *dir = opendir(rom_dir);

    // Unable to open directory stream
    if (!dir)
        return false;

    while ((dp = readdir(dir)) != NULL) {
        if (dp->d_name[0] == '.')
            continue;

        if (dp->d_type == DT_DIR) {
            if (level == 0) {
                char subdir[PATH_MAX];
                snprintf(subdir, PATH_MAX - 1, "%s/%s", rom_dir, dp->d_name);
                if (checkRomDir(subdir, extlist, level + 1)) {
                    return true;
                }
            }
            continue;
        }

        if (dp->d_type == DT_REG) {
            if (!hasExtension(dp->d_name, extlist))
                continue;

            return true;
        }
    }

    return false;
}

bool checkRoms(const char *data_path)
{
    char path_dup[PATH_MAX];
    strncpy(path_dup, data_path, PATH_MAX - 1);

    char *base_dir_name = basename(dirname(path_dup));
    char config_path[PATH_MAX];

    if (!getConfigPath(config_path, data_path, base_dir_name)) {
        return false;
    }

    cJSON *config = json_load(config_path);
    char roms_rel_path[STR_MAX];
    char extlist[STR_MAX] = {0};

    if (!json_getString(config, "rompath", roms_rel_path)) {
        free(config);
        return false;
    }

    json_getString(config, "extlist", extlist);

    free(config);

    if (strncmp(roms_rel_path, "../../", 6) != 0)
        return false;

    char rom_dir[PATH_MAX];
    snprintf(rom_dir, PATH_MAX - 1, "/mnt/SDCARD/%s", roms_rel_path + 6);

    return checkRomDir(rom_dir, extlist, 0);
}

void loadPackages(bool auto_update)
{
    DIR *dp;
    struct dirent *ep;
    char basePath[1000];

    for (int nT = 0; nT < tab_count; nT++) {
        const char *data_path = layer_dirs[nT];
        const bool check_roms = layer_check_roms[nT];
        package_count[nT] = 0;

        if (strlen(data_path) == 0 || !exists(data_path) ||
            (dp = opendir(data_path)) == NULL)
            continue;

        while ((ep = readdir(dp)) && package_count[nT] < LAYER_ITEM_COUNT) {
            char cShort[MAX_LAYER_NAME_SIZE];
            strcpy(cShort, ep->d_name);

            const char *file_name = ep->d_name;
            if (file_name[0] != '.') {
                // Installation check
                sprintf(basePath, "%s/%s", data_path, file_name);

                bool is_installed = checkAppInstalled(basePath, strlen(basePath), 0, false);
                bool is_complete = !auto_update && is_installed
                                       ? checkAppInstalled(basePath, strlen(basePath), 0, true)
                                       : false;

                Package package = {.installed = is_installed,
                                   .changed = false,
                                   .complete = is_complete,
                                   .has_roms = check_roms ? checkRoms(basePath) : false};

                if (is_installed) {
                    package_installed_count[nT]++;

                    if (!is_complete)
                        changes_installs[nT]++;
                }

                strcpy(package.name, file_name);

                packages[nT][package_count[nT]] = package;
                package_count[nT]++;
            }
        }

        closedir(dp);
    }

    Package temp;

    // Sort function
    for (int nT = 0; nT < tab_count; nT++) {
        bool found = true;

        while (found) {
            found = false;

            for (int i = 0; i < package_count[nT] - 1; i++) {
                Package *package_a = &packages[nT][i];
                Package *package_b = &packages[nT][i + 1];

                if (strcmp(package_a->name, package_b->name) > 0) {
                    temp = *package_a;
                    *package_a = *package_b;
                    *package_b = temp;
                    found = true;
                }
            }
        }
    }
}

bool getPackageMainPath(char *out_path, const char *data_path,
                        const char *package_name)
{
    const char *base_dir = basename((char *)data_path);
    sprintf(out_path, "%s/%s/%s/", data_path, package_name, base_dir);

    if (!is_dir(out_path))
        return false;

    struct dirent *dp;
    DIR *dir = opendir(out_path);

    // Unable to open directory stream
    if (!dir)
        return false;

    while ((dp = readdir(dir)) != NULL) {
        if (strcmp(dp->d_name, ".") == 0 || strcmp(dp->d_name, "..") == 0)
            continue;
        if (dp->d_type != DT_DIR)
            continue;
        sprintf(out_path, "/mnt/SDCARD/%s/%s", base_dir, dp->d_name);
        return is_dir(out_path);
    }

    return false;
}

void callPackageInstaller(const char *data_path, const char *package_name,
                          bool install)
{
    char main_path[STR_MAX], cmd[STR_MAX];

    if (getPackageMainPath(main_path, data_path, package_name)) {
        char config_path[STR_MAX + 32];
        snprintf(config_path, STR_MAX + 32 - 1, "%s/config.json", main_path);

        if (install && is_file(config_path))
            apply_singleIcon(config_path);

        char installer_path[STR_MAX + 32];
        concat(installer_path, main_path,
               install ? "/install.sh" : "/uninstall.sh");
        if (is_file(installer_path)) {
            sprintf(cmd,
                    install
                        ? "cd \"%s\"; chmod a+x ./install.sh; ./install.sh"
                        : "cd \"%s\"; chmod a+x ./uninstall.sh; ./uninstall.sh",
                    main_path);
            system(cmd);
        }
    }
}

void appUninstall(char *basePath, int strlenBase)
{
    char path[1000];
    char pathInstalledApp[1000];

    struct dirent *dp;
    DIR *dir = opendir(basePath);

    // Unable to open directory stream
    if (!dir)
        return;

    while ((dp = readdir(dir)) != NULL) {
        if (strcmp(dp->d_name, ".") != 0 && strcmp(dp->d_name, "..") != 0) {
            // Construct new path from our base path
            sprintf(path, "%s/%s", basePath, dp->d_name);

            if (exists(path)) {
                sprintf(pathInstalledApp, "/mnt/SDCARD%s", path + strlenBase);

                if (is_file(pathInstalledApp))
                    remove(pathInstalledApp);

                if (is_dir(path))
                    appUninstall(path, strlenBase);

                if (is_dir(pathInstalledApp))
                    rmdir(pathInstalledApp);
            }
        }
    }

    closedir(dir);
}

#endif // PACMAN_FILE_ACTIONS_H__