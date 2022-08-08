#ifndef RUMBLE_H__
#define RUMBLE_H__

#include <unistd.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdbool.h>

#include "utils/file.h"
#include "settings.h"


#define rumbleOn() rumble(true)
#define rumbleOff() rumble(false)


void rumble(bool enabled)
{
    file_write("/sys/class/gpio/export", "48", 2);
    file_write("/sys/class/gpio/gpio48/direction", "out", 3);
    file_write("/sys/class/gpio/gpio48/value", enabled ? "0" : "1", 1);
}

/**
 * @brief Turns on vibration for 100ms
 * 
 */
void short_pulse(void) {
    if (!settings.vibration) return;
    rumbleOn();
    usleep(100000); // 0.1s
    rumbleOff();
}

/**
 * @brief Turns on vibration for 50ms
 * 
 */
void super_short_pulse(void) {
    if (!settings.vibration) return;
    rumbleOn();
    usleep(40000); // 0.05s
    rumbleOff();
}

#endif // RUMBLE_H__
