#ifndef LOG_H__
#define LOG_H__

#include <stdio.h>
#include <stdarg.h>

#define LOG_INIT "Initialized %s\n"
#define LOG_SUCCESS "Successfully %s\n"
#define LOG_MESSAGE "%s\n"
#define LOG_TERM "Terminated %s with values %d and %d\n"

#ifdef LOG_DEBUG
#define print_debug(message) log_debug(__FILE__, __LINE__, LOG_MESSAGE, message)
#define printf_debug(format_str, ...) log_debug(__FILE__, __LINE__, format_str, __VA_ARGS__)

void log_debug(const char *filename, int line, const char *format_str,...) {
    va_list valist;
    va_start(valist, format_str);
    printf("%s:%d>\t", filename, line);
    vprintf(format_str, valist);
    va_end(valist);
}
#else
#define log_debug(message)
#define logf_debug(format_str, ...)
#endif

#endif // LOG_H__
