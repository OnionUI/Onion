#ifndef BATTERY_H__
#define BATTERY_H__

#include "utils/file.h"
#include "utils/process.h"
#include "system/system.h"

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
        if (exists("/tmp/percBat")) {
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
        percentage = 500;

    return percentage;
}

bool battery_isCharging(void)
{
    #ifdef PLATFORM_MIYOOMINI
    char charging = 0;
    int fd = open(GPIO_DIR2 "gpio59/value", O_RDONLY);

    if (fd < 0) {
        // export gpio59, direction: in
        file_write(GPIO_DIR1 "export", "59", 2);
        file_write(GPIO_DIR2 "gpio59/direction", "in", 2);
        fd = open(GPIO_DIR2 "gpio59/value", O_RDONLY);
    }

    if (fd >= 0) {
        read(fd, &charging, 1);
        close(fd);        
    }

    return charging == '1';
    #else
    return true;
    #endif
}

#endif // BATTERY_H__
