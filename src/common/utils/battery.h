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
        else if (!process_isRunning("batmon")) {
            printf_debug("/tmp/percBat not found (%d)\n", retry);
            process_start("batmon", NULL, NULL, false);
        }
        else {
            printf_debug("bin/batmon not found (%d)\n", retry);
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
    return battery_getPercentage() == 500;
}

#endif // BATTERY_H__
