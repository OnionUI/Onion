#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <fcntl.h>

#include "utils/file.h"

#define MI_AO_SETVOLUME 0x4008690b
#define MI_AO_GETVOLUME 0xc008690c

int setVolumeRaw(int volume, int add) {
    int recent_volume = 0;
    int fd = open("/dev/mi_ao", O_RDWR);
    if (fd >= 0) {
        int buf2[] = {0, 0};
        uint64_t buf1[] = {sizeof(buf2), (uintptr_t)buf2};
        ioctl(fd, MI_AO_GETVOLUME, buf1);
        recent_volume = buf2[1];
        if (add) {
            buf2[1] += add;
            if (buf2[1] > 30) buf2[1] = 30;
            else if (buf2[1] < -30) buf2[1] = -30;
        } else buf2[1] = volume;
        if (buf2[1] != recent_volume) ioctl(fd, MI_AO_SETVOLUME, buf1);
        close(fd);
    }
    return recent_volume;
}

int main(int argc, char *argv[])
{
	if (argc != 2) {
        printf("Usage: mute [N]\nValues: 0 - mute off (sound), 1 - mute on (no sound)\n");
        return 1;
    }

	bool mute_on = argv[1][0] == '1';

    FILE *fp;
    int recent_volume = 0;

    if (mute_on) {
        recent_volume = setVolumeRaw(-60, 0);
        file_put_sync(fp, "/tmp/recent_volume", "%d", recent_volume);
    }
    else {
        if (exists("/tmp/recent_volume"))
            file_get(fp, "/tmp/recent_volume", "%d", &recent_volume);
        setVolumeRaw(recent_volume, 0);
    }

	return 0;
}

