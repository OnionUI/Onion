#ifndef CONFIG_H__
#define CONFIG_H__

#include <stdio.h>
#include <stdbool.h>
#include <libgen.h>
#include <sys/stat.h>

#include "file.h"
#include "str.h"
#include "flags.h"
#include "log.h"

#define CONFIG_PATH "/mnt/SDCARD/.tmp_update/config/"

bool config_flag_get(const char *key)
{
    return flag_get(CONFIG_PATH, key);
}

void config_flag_set(const char *key, bool value)
{
    char hidden_flag[STR_MAX];
    concat(hidden_flag, key, "_");
    flag_set(CONFIG_PATH, key, value);
    flag_set(CONFIG_PATH, hidden_flag, !value);
}

bool config_get(const char *key, const char *format, void *dest) {
    FILE *fp;
    
    char filename[STR_MAX];
    concat(filename, CONFIG_PATH, key);
    
    if (exists(filename)) {
        file_get(fp, filename, format, dest);
        return true;
    }

    return false;
}

void _config_prepare(const char *key, char *filename)
{
    concat(filename, CONFIG_PATH, key);

    char dir_path[STR_MAX];
    strcpy(dir_path, filename);
    dirname(dir_path);

    if (!exists(dir_path)) {
        char dir_cmd[512];
        sprintf(dir_cmd, "mkdir -p \"%s\"", dir_path);
        system(dir_cmd);
    }
}

void config_setNumber(const char *key, int value) {
    FILE *fp;
    char filename[STR_MAX];
    _config_prepare(key, filename);
    file_put_sync(fp, filename, "%d", value);
}

void config_setString(const char *key, char *value) {
    FILE *fp;
    char filename[STR_MAX];
    _config_prepare(key, filename);
    file_put_sync(fp, filename, "%s", value);
}

#endif // CONFIG_H__
