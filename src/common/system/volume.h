#ifndef VOLUME_H__
#define VOLUME_H__

#include <fcntl.h>
#include <math.h>
#include <sys/ioctl.h>

#include "utils/file.h"

#define MAX_VOLUME 20
#define MIN_RAW_VALUE -60
#define MAX_RAW_VALUE 30

#define MI_AO_SETVOLUME 0x4008690b
#define MI_AO_GETVOLUME 0xc008690c
#define MI_AO_SETMUTE 0x4008690d

// Set volume output between 0 and 60, use `add` for boosting
int setVolumeRaw(int value, int add)
{
    int fd;

    if ((fd = open("/dev/mi_ao", O_RDWR)) < 0)
        return 0;

    int buf2[] = {0, 0};
    uint64_t buf1[] = {sizeof(buf2), (uintptr_t)buf2};
    ioctl(fd, MI_AO_GETVOLUME, buf1);
    int prev_value = buf2[1];

    if (add)
        value = prev_value + add;
    else
        value += MIN_RAW_VALUE;

    if (value > MAX_RAW_VALUE)
        value = MAX_RAW_VALUE;
    else if (value < MIN_RAW_VALUE)
        value = MIN_RAW_VALUE;

    if (value == prev_value) {
        close(fd);
        return prev_value;
    }

    buf2[1] = value;
    ioctl(fd, MI_AO_SETVOLUME, buf1);
    printf_debug("raw volume: %d\n", buf2[1]);

    if (prev_value <= MIN_RAW_VALUE && value > MIN_RAW_VALUE) {
        buf2[1] = 0;
        ioctl(fd, MI_AO_SETMUTE, buf1);
        print_debug("mute: OFF");
    }
    else if (prev_value > MIN_RAW_VALUE && value <= MIN_RAW_VALUE) {
        buf2[1] = 1;
        ioctl(fd, MI_AO_SETMUTE, buf1);
        print_debug("mute: ON");
    }

    close(fd);
    return value;
}

// Increments between 0 and 20
int setVolume(int volume)
{
    int volume_raw = 0;

    if (volume > 20)
        volume = 20;
    else if (volume < 0)
        volume = 0;

    if (volume != 0)
        volume_raw = round(48 * log10(1 + volume)); // see volume curve below

    printf_debug("set volume: %d -> %d\n", volume, volume_raw);

    setVolumeRaw(volume_raw, 0);
    return volume;
}

/*
VOLUME CURVE (later corrected to rise to 63)

         60 :                    .                    .                    .       ..............
            .                                                             .............
            .                                                    ............
            .                                             ..........
            .                                      ...::....
            .                                 ....... .
            .                            .......
         40 :                    .  ..::...           .                    .                    .
            .                    .:...
    RAW     .                 .:...
   VALUE    .              .:..
            .           .::.
            .         .::
            .       .:.
         20 :      ::.           .                    .                    .                    .
            .     :.
            .   .:
            .  .:
            .  :
            ..:.
            ...
          0 :....................:....................:....................:....................:.
            0                    5                   10                   15                   20

                                                   VOLUME
*/

#endif // VOLUME_H__
