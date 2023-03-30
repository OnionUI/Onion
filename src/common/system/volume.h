#ifndef VOLUME_H__
#define VOLUME_H__

#include "utils/file.h"

#define MAX_VOLUME 20
#define VOLUME_INCREMENTS 1

#define MI_AO_SETVOLUME 0x4008690b
#define MI_AO_GETVOLUME 0xc008690c
#define MI_AO_SETMUTE 0x4008690d

int setVolumeRaw(int volume, int add)
{
    int recent_volume = 0;
    int fd = open("/dev/mi_ao", O_RDWR);
    if (fd >= 0) {
        int buf2[] = {0, 0};
        uint64_t buf1[] = {sizeof(buf2), (uintptr_t)buf2};
        ioctl(fd, MI_AO_GETVOLUME, buf1);
        recent_volume = buf2[1];
        if (add) {
            buf2[1] += add;
            if (buf2[1] > 0)
                buf2[1] = 0;
            else if (buf2[1] < -60)
                buf2[1] = -60;
        }
        else
            buf2[1] = volume;
        if (buf2[1] != recent_volume)
            ioctl(fd, MI_AO_SETVOLUME, buf1);
        close(fd);
    }
    return recent_volume;
}

// Increments between 0 and 20
int setVolume(int volume, int add)
{
    int recent_volume = 0;
    int rawVolumeValue = 0;
    int rawAdd = 0;

    rawVolumeValue = (volume * 3) - 60;
    rawAdd = (add * 3);

    recent_volume = setVolumeRaw(rawVolumeValue, rawAdd);
    return (int)((recent_volume / 3) + 20);
}

void setMute(bool mute)
{
    int fd = open("/dev/mi_ao", O_RDWR);
    if (fd >= 0) {
        int buf2[] = {0, mute};
        uint64_t buf1[] = {sizeof(buf2), (uintptr_t)buf2};

        ioctl(fd, MI_AO_SETMUTE, buf1);
        close(fd);
    }
}
#endif // VOLUME_H__
