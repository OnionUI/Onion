#define _GNU_SOURCE
#include "utils/file.h"
#include "utils/process.h"
#include <dlfcn.h>
#include <signal.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

typedef int (*printf_t)(const char *format, ...);
static printf_t original_printf = NULL;
typedef FILE *(*fopen_t)(const char *path, const char *mode);
static fopen_t original_fopen = NULL;

static bool rip_mainui = false;

int printf(const char *format, ...)
{
    if (!original_printf)
        original_printf = (printf_t)dlsym(RTLD_NEXT, "printf");

    if (strcmp(format, "createMenu %d\n") == 0) {
        va_list args;
        va_start(args, format);
        int arg = va_arg(args, int);
        va_end(args);

        if (arg == 7) { // 7 = settings menu
            FILE *fp;
            file_put_sync(fp, "/mnt/SDCARD/.tmp_update/cmd_to_run.sh", "%s", "/mnt/SDCARD/App/Tweaks/launch.sh");
            kill(getpid(), SIGTERM);
            rip_mainui = true;
        }
    }

    va_list args;
    va_start(args, format);
    int res = vprintf(format, args);
    va_end(args);

    return res;
}

FILE *fopen(const char *path, const char *mode)
{
    if (!original_fopen)
        original_fopen = (fopen_t)dlsym(RTLD_NEXT, "fopen");

    // don't let MainUI write state.json if we're transitioning to tweaks
    if (rip_mainui && strcmp(path, "/tmp/state.json") == 0)
        return NULL;

    return original_fopen(path, mode);
}