#ifndef CONFIG_H__
#define CONFIG_H__

#include <stdio.h>
#include <stdbool.h>
#include <libgen.h>
#include <sys/stat.h>

#include "file.h"
#include "str.h"
#include "flags.h"

#define CONFIG_PATH "/mnt/SDCARD/.tmp_update/config/"

#define config_flag_get(key) flag_get(CONFIG_PATH, key)
#define config_flag_set(key, value) flag_set(CONFIG_PATH, key, value)

bool config_get(const char *key, const char *format, void *dest) {
    FILE *fp;
    
    char filename[STR_MAX];
    concat(filename, CONFIG_PATH, key);
    
    if (file_exists(filename)) {
        file_get(fp, filename, format, dest);
        return true;
    }

    return false;
}

void config_prepare(const char *key, char *filename)
{
    concat(filename, CONFIG_PATH, key);

    char dir_path[STR_MAX];
    strcpy(dir_path, filename);
    dirname(dir_path);

    if (!file_exists(dir_path)) {
        char dir_cmd[512];
        sprintf(dir_cmd, "mkdir -p \"%s\"", dir_path);
        system(dir_cmd);
    }
}

void config_setNumber(const char *key, int value) {
    FILE *fp;
    char filename[STR_MAX];
    config_prepare(key, filename);
    file_put_sync(fp, filename, "%d", value);
    printf_debug("config set: <%s> = %d\n", key, value);
}

void config_setString(const char *key, char *value) {
    FILE *fp;
    char filename[STR_MAX];
    config_prepare(key, filename);
    file_put_sync(fp, filename, "%s", value);
    printf_debug("config set: <%s> = '%s'\n", key, value);
}

#endif // CONFIG_H__
