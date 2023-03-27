#include "log.h"

#include <stdarg.h>
#include <stdio.h>

#include "./str.h"

void log_debug(const char *filename, int line, const char *format_str, ...)
{
    char log_message[512], cmd[1024];

    va_list valist;
    va_start(valist, format_str);
    sprintf(log_message, "%s:%d>\t", filename, line);
    vsprintf(log_message + strlen(log_message), format_str, valist);
    va_end(valist);

    snprintf(cmd, 1023, "echo -n \"%s\" >/dev/stderr",
             str_replace(log_message, "\"", "\\\""));
    system(cmd);
}
