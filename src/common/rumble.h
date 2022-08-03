#ifndef RUMBLE_H__
#define RUMBLE_H__

#include <unistd.h>
#include <fcntl.h>
#include <stdint.h>

void rumble(uint32_t val) {
    int fd;
    const char str_export[] = "48";
    const char str_direction[] = "out";
    char value[1];
    value[0] = ((val&1)^1) + 0x30;

    if ((fd = open("/sys/class/gpio/export", O_WRONLY)) > 0) {
        write(fd, str_export, 2);
        close(fd);
    }
    if ((fd = open("/sys/class/gpio/gpio48/direction", O_WRONLY)) > 0) {
        write(fd, str_direction, 3);
        close(fd);
    }
    if ((fd = open("/sys/class/gpio/gpio48/value", O_WRONLY)) > 0) {
        write(fd, value, 1);
        close(fd);
    }
}

#endif // RUMBLE_H__
