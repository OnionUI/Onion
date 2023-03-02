#include "log.h"

#include <stdio.h>
#include <stdarg.h>

#include "./str.h"

void log_debug(const char *filename, int line, const char *format_str,...) {
    char log_message[512], cmd[512];
    
    va_list valist;
    va_start(valist, format_str);
    sprintf(log_message, "%s:%d>\t", filename, line);
    vsprintf(log_message + strlen(log_message), format_str, valist);
    va_end(valist);

    concat(cmd, "echo -n \"", str_replace(log_message, "\"", "\\\""));
    int i = strlen(cmd);
    cmd[i++] = '"';
    cmd[i++] = '\0';
    system(cmd);
}
