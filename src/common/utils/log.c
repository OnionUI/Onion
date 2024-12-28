#include "log.h"

#include <stdarg.h>
#include <stdio.h>

#include "./file.h"
#include "./str.h"

static char _log_path[64] = "";

void log_setName(const char *log_name)
{
    snprintf(_log_path, 63, "/mnt/SDCARD/.tmp_update/logs/%s.log", log_name);
    mkdirs("/mnt/SDCARD/.tmp_update/logs");
}

void log_debug(const char *file_path, int line, const char *format_str, ...)
{
    char log_message[1024];

    va_list valist;
    va_start(valist, format_str);
    sprintf(log_message, "%s:%d>\t", file_path, line);
    vsprintf(log_message + strlen(log_message), format_str, valist);
    va_end(valist);

    fprintf(stderr, "%s", log_message);

    if (strlen(_log_path) == 0)
        return;

    FILE *fp;
    if ((fp = fopen(_log_path, "a+")) != NULL) {
        fwrite(log_message, sizeof(char), strlen(log_message), fp);
        fflush(fp);
        fsync(fileno(fp));
        fclose(fp);
    }
}
