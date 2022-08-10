#ifndef BATTERY_H__
#define BATTERY_H__

#include "utils/file.h"
#include "utils/process.h"

/**
 * @brief Retrieve the current battery percentage as reported by batmon
 * 
 * @return int : Battery percentage (0-100) or 500 if charging
 */
int battery_getPercentage(void)
{
    FILE *fp;
    int percentage = -1;
    int retry = 3;

    while (percentage == -1 && retry > 0) {
        if (file_exists("/tmp/percBat")) {
            file_get(fp, "/tmp/percBat", "%d", &percentage);
            break;
        }
        else {
            printf_debug("/tmp/percBat not found (%d)\n", retry);

            if (!process_isRunning("batmon")) {
                printf_debug("bin/batmon not running (%d)\n", retry);
                break;
            }
        }

        retry--;
        msleep(100);
    }

    if (percentage == -1)
        percentage = 0;

    return percentage;
}

bool battery_isCharging(void)
{
    #ifdef PLATFORM_MIYOOMINI
    int value = 0;
    FILE *fp;
    file_get(fp, "/sys/devices/gpiochip0/gpio/gpio59/value", "%d", &value);
    return value == 1;
    #else
    return true;
    #endif
}

#endif // BATTERY_H__
