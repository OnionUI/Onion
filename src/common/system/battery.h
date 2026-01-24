#ifndef BATTERY_H__
#define BATTERY_H__

#include "system/device_model.h"
#include "system/system.h"
#include "utils/file.h"
#include "utils/log.h"
#include "utils/msleep.h"
#include "utils/process.h"

static time_t battery_last_modified = 0;
static bool battery_is_charging = false;

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

#ifndef PLATFORM_MIYOOMINI
#ifdef LOG_DEBUG
    return 78;
#endif
#endif

    if (percentage == -1)
        percentage = 0; // show zero when percBat not found

    return percentage;
}

bool battery_isCharging(void)
{
#ifdef PLATFORM_MIYOOMINI
    if (DEVICE_ID == MIYOO283) {
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
    }
    else if (IS_MIYOO_PLUS_OR_FLIP()) {
        char *cmd = "cd /customer/app/ ; ./axp_test";
        int batJsonSize = 100;
        char buf[batJsonSize];
        int charge_number;

        FILE *fp;
        fp = popen(cmd, "r");
        if (fgets(buf, batJsonSize, fp) != NULL) {
            sscanf(buf, "{\"battery\":%*d, \"voltage\":%*d, \"charging\":%d}",
                   &charge_number);
        }
        pclose(fp);
        return charge_number == 3;
    }
    return false;
#else
    return false;
#endif
}

bool battery_hasChanged(int ticks, int *out_percentage)
{
    bool changed = false;

    if (battery_isCharging()) {
        if (!battery_is_charging) {
            *out_percentage = 500;
            battery_is_charging = true;
            return true;
        }
        return false;
    }
    else if (battery_is_charging) {
        battery_is_charging = false;
    }

    if (file_isModified("/tmp/percBat", &battery_last_modified)) {
        int current_percentage = battery_getPercentage();

        if (current_percentage != *out_percentage) {
            *out_percentage = current_percentage;
            changed = true;
        }
    }

    return changed;
}

#endif // BATTERY_H__
