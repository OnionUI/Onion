#ifndef RUMBLE_H__
#define RUMBLE_H__

#include <fcntl.h>
#include <stdbool.h>
#include <stdint.h>
#include <unistd.h>

#include "settings.h"
#include "utils/file.h"
#include "utils/msleep.h"

#define SHORT_PULSE_MS 100
#define SUPER_SHORT_PULSE_MS 50

static int super_short_timings[] = {0, 25, 50, 75};
static int short_timings[] = {0, 50, 100, 150};

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
void short_pulse(void)
{
    if (settings.vibration == 0)
        return;
    rumble(true);
    msleep(short_timings[settings.vibration]);
    rumble(false);
}

/**
 * @brief Turns on vibration for 50ms
 *
 */
void super_short_pulse(void)
{
    if (settings.vibration == 0)
        return;
    rumble(true);
    msleep(super_short_timings[settings.vibration]);
    rumble(false);
}

/**
 * @brief Turns on vibration for 50ms
 *
 */
void menu_short_pulse(void)
{
    if (settings.vibration == 0 || !settings.menu_button_haptics)
        return;
    rumble(true);
    msleep(short_timings[settings.vibration]);
    rumble(false);
}

/**
 * @brief Turns on vibration for 50ms
 *
 */
void menu_super_short_pulse(void)
{
    if (settings.vibration == 0 || !settings.menu_button_haptics)
        return;
    rumble(true);
    msleep(super_short_timings[settings.vibration]);
    rumble(false);
}

#endif // RUMBLE_H__
