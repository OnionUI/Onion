#ifndef SYSTEM_H__
#define SYSTEM_H__

#include <fcntl.h>

#include "clock.h"
#include "flags.h"

// system directories
#define    GPIO_DIR1    "/sys/class/gpio/"
#define    GPIO_DIR2    "/sys/devices/gpiochip0/gpio/"
#define    PWM_DIR      "/sys/devices/soc0/soc/1f003400.pwm/pwm/pwmchip0/"
#define    CPU_DIR      "/sys/devices/system/cpu/cpufreq/policy0/"

/**
 * @brief Creates the file `/mnt/SDCARD/.tmp_update/.offOrder`,
 * which is used by `runtime.sh` for knowing when to show shutdown screen.
 * 
 */
void system_shutdown(void) {
    settings_flag_set(".offOrder", true);
    read_clock();
    write_clockfile();
    sync();
}

#endif // SYSTEM_H__
