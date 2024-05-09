#ifndef TIMER_H__
#define TIMER_H__

#include <stdio.h>
#include <sys/time.h>

#ifdef LOG_DEBUG

#define START_TIMER(struct_name)                                                    \
    struct timeval struct_name##_before, struct_name##_after, struct_name##_result; \
    gettimeofday(&struct_name##_before, NULL);

#define END_TIMER(struct_name)                                                                                  \
    gettimeofday(&struct_name##_after, NULL);                                                                   \
    timersub(&struct_name##_after, &struct_name##_before, &struct_name##_result);                               \
    long struct_name##_milliseconds = struct_name##_result.tv_sec * 1000 + struct_name##_result.tv_usec / 1000; \
    printf("\033[1;33m%s: %ld milliseconds\033[0m\n", #struct_name, struct_name##_milliseconds);

#else

#define START_TIMER(struct_name)
#define END_TIMER(struct_name)

#endif

#endif

/*
 * Usage:
 * 
 * #include "timer.h"
 * 
 * int main() {
 *     START_TIMER(loading);
 *     some_loading_function();
 *     END_TIMER(loading);
 *     return 0;
 * }
 * 
 * Output:
 * 
 * loading: 123 milliseconds
 */