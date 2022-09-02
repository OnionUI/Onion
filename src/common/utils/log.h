#ifndef LOG_H__
#define LOG_H__

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

void log_debug(const char *filename, int line, const char *format_str,...);

#endif // LOG_H__
