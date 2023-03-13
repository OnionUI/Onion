#ifndef CLOCK_H__
#define CLOCK_H__

#include <dirent.h>
#include <fcntl.h>
#include <linux/fb.h>
#include <linux/rtc.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>

#include "utils/file.h"
#include "utils/str.h"

#define CLOCK_SAVE_FILE "/mnt/SDCARD/Saves/CurrentProfile/saves/currentTime.txt"

static struct tm clk;
static bool clock_paused = false;

//
//    Read Clock (system)
//
void system_clock_get(void)
{
    struct timeval tod;
    gettimeofday(&tod, NULL);
    gmtime_r(&tod.tv_sec, &clk);
}

//
//    Read Clock (RTC)
//
void system_rtc_get(void)
{
    int cfd;
    if ((cfd = open("/dev/rtc0", O_RDONLY)) > 0) {
        ioctl(cfd, RTC_RD_TIME, &clk);
        close(cfd);
    }
    else
        system_clock_get();
}

//
//    Write Clock (system)
//
void system_clock_set(void)
{
    struct timeval tod;
    gettimeofday(&tod, NULL);
    tod.tv_sec = mktime(&clk);
    settimeofday(&tod, NULL);
}

//
//    Write Clock (RTC & system)
//
void system_rtc_set(void)
{
    int cfd;
    if ((cfd = open("/dev/rtc0", O_WRONLY)) >= 0) {
        ioctl(cfd, RTC_SET_TIME, &clk);
        close(cfd);
    }
    system_clock_set();
}

//
//    Write currentTime.txt
//
void system_clock_save(void)
{
    FILE *fp;
    if (clock_paused)
        return;
    system_clock_get();
    file_put_sync(fp, CLOCK_SAVE_FILE, "%ld", mktime(&clk));
}

//
//    Read currentTime.txt
//
void system_clock_load(void)
{
    FILE *fp;
    struct timeval tod;
    gettimeofday(&tod, NULL);
    file_get(fp, CLOCK_SAVE_FILE, "%ld", &tod.tv_sec);
    settimeofday(&tod, NULL);
}

//
//    [onion] suspend/resume PlayActivity timer
//
void system_clock_pause(bool enabled)
{
    if (enabled)
        // The current time is saved
        system_clock_save();
    else
        // The current time is resumed
        system_clock_load();

    clock_paused = enabled;
}

/**
 * @brief Get current time in milliseconds (uses CLOCK_MONOTONIC_RAW)
 *
 * @return int Milliseconds
 */
int getMilliseconds(void)
{
    struct timespec te;
    clock_gettime(CLOCK_MONOTONIC_RAW, &te);
    int ms = (int)(te.tv_sec * 1000 + te.tv_nsec / 1000000);
    return ms;
}

/**
 * @brief Get current time in seconds (uses CLOCK_MONOTONIC_COARSE)
 *
 * @return int Seconds
 */
int getSeconds(void)
{
    struct timespec te;
    clock_gettime(CLOCK_MONOTONIC_COARSE, &te);
    return (int)te.tv_sec;
}

#endif // CLOCK_H__
