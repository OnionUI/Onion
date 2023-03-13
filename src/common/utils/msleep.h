#ifndef MSLEEP_H__
#define MSLEEP_H__

#include <errno.h>
#include <time.h>

static int msleep_interrupt = 0;

/* msleep(): Sleep for the requested number of milliseconds. */
int msleep(long msec)
{
    struct timespec ts;
    int res;

    if (msec < 0) {
        errno = EINVAL;
        return -1;
    }

    ts.tv_sec = msec / 1000;
    ts.tv_nsec = (msec % 1000) * 1000000;

    do {
        res = nanosleep(&ts, &ts);
    } while (res && errno == EINTR && msleep_interrupt == 0);

    msleep_interrupt = 0;

    return res;
}

#endif // MSLEEP_H__
