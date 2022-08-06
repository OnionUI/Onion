#ifndef LOG_H__
#define LOG_H__

#include <stdio.h>
#include <stdarg.h>

#include "str.h"

#define LOG_INIT "Initialized %s\n"
#define LOG_SUCCESS "Successfully %s\n"
#define LOG_MESSAGE "%s\n"
#define LOG_TERM "Terminated %s with values %d and %d\n"

#ifdef LOG_DEBUG
#define print_debug(message) log_debug(__FILE__, __LINE__, LOG_MESSAGE, message)
#define printf_debug(format_str, ...) log_debug(__FILE__, __LINE__, format_str, __VA_ARGS__)
#else
#define print_debug(message)
#define printf_debug(format_str, ...)
#endif

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

#endif // LOG_H__
