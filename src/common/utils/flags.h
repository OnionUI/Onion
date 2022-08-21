#ifndef FLAGS_H__
#define FLAGS_H__

#include <stdbool.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "file.h"
#include "str.h"

#define temp_flag_get(key) flag_get("/tmp/", key)
#define temp_flag_set(key, value) flag_set("/tmp/", key, value)

bool flag_get(const char *path, const char *key) {
    char filename[STR_MAX];
    concat(filename, path, key);
    return exists(filename);
}

void flag_set(const char *path, const char *key, bool value) {
    char filename[STR_MAX];
    concat(filename, path, key);
    
    if (value)
        close(creat(filename, 777));
    else
        remove(filename);
}

#endif // FLAGS_H__
