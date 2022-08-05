#ifndef FLAGS_H__
#define FLAGS_H__

#include <stdbool.h>

#include "utils.h"

#define DOTFILE_PATH "/mnt/SDCARD/.tmp_update/config/"

#define settings_flag_get(key) flag_get(DOTFILE_PATH, key)
#define settings_flag_set(key, value) flag_set(DOTFILE_PATH, key, value)
#define temp_flag_get(key) flag_get("/tmp/", key)
#define temp_flag_set(key, value) flag_set("/tmp/", key, value)

bool flag_get(const char *path, const char *key) {
    char filename[MAX_LEN];
    concat(filename, path, key);
    return file_exists(filename);
}

void flag_set(const char *path, const char *key, bool value) {
    char filename[MAX_LEN];
    concat(filename, path, key);
    
    if (value)
        close(creat(filename, 777));
    else
        remove(filename);
}

#endif // FLAGS_H__
