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

#define CLOCK_SAVE_FILE "/mnt/SDCARD/Saves/CurrentProfile/saves/currentTime.txt"

static struct tm clk;

//
//    Read Clock (system)
//
void read_clock(void) {
    struct timeval tod;
    gettimeofday(&tod, NULL);
    gmtime_r(&tod.tv_sec, &clk);
}

//
//    Read Clock (RTC)
//
void read_clock_rtc(void) {
    int cfd = open("/dev/rtc0", O_RDONLY);
    if (cfd > 0) {
        ioctl(cfd, RTC_RD_TIME, &clk);
        close(cfd);
    }
    else read_clock();
}

//
//    Write Clock (system)
//
void write_clock(void) {
    struct timeval tod;
    gettimeofday(&tod, NULL);
    tod.tv_sec = mktime(&clk);
    settimeofday(&tod, NULL);
}

//
//    Write Clock (RTC & system)
//
void write_clock_rtc(void) {
    int cfd = open("/dev/rtc0", O_WRONLY);
    if (cfd >= 0) { ioctl(cfd, RTC_SET_TIME, &clk); close(cfd); }
    write_clock();
}

//
//    Write clock.txt
//
void write_clockfile(void) {
    FILE* fp;

    if ( (fp = fopen(CLOCK_SAVE_FILE, "w")) ) {
        fprintf(fp, "%ld", mktime(&clk));
        fflush(fp);
        fsync(fileno(fp));
        fclose(fp);
    }
}

#endif // CLOCK_H__
