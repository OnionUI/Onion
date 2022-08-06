#ifndef BATTERY_H__
#define BATTERY_H__

#include "utils/file.h"
#include "utils/process.h"

/**
 * @brief Retrieve the current battery percentage as reported by percBat
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
        else if (!process_searchpid("percBat")) {
            printf_debug("percBat not found (%d)\n", retry);
            process_start("percBat", NULL, NULL, false);
        }
        else {
            printf_debug("bin/percBat not found (%d)\n", retry);
        }

        retry--;
        msleep(100);
    }

    if (percentage == -1)
        percentage = 500;

    return percentage;
}

bool battery_isCharging(void)
{
    return battery_getPercentage() == 500;
}

#endif // BATTERY_H__
