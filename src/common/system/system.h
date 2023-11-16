#ifndef SYSTEM_H__
#define SYSTEM_H__

#include <fcntl.h>
#include <stdbool.h>

#include "clock.h"
#include "utils/flags.h"

// system directories
#define GPIO_DIR1 "/sys/class/gpio/"
#define GPIO_DIR2 "/sys/devices/gpiochip0/gpio/"
#define PWM_DIR "/sys/devices/soc0/soc/1f003400.pwm/pwm/pwmchip0/"
#define CPU_DIR "/sys/devices/system/cpu/cpufreq/policy0/"

#define CPU_SCALING_MIN_FREQ CPU_DIR "scaling_min_freq"
#define CPU_SCALING_GOVERNOR CPU_DIR "scaling_governor"

#define system_powersave_on() system_powersave(true)
#define system_powersave_off() system_powersave(false)

/**
 * @brief Creates the file `/tmp/.offOrder`,
 * which is used by `runtime.sh` for knowing when to show shutdown screen.
 *
 */
void set_system_shutdown(void)
{
    temp_flag_set(".offOrder", true);
    system_clock_get();
    system_clock_save();
    sync();
}

/**
 * @brief Toggle powersave mode
 *
 * @param enabled true: turn on powersave, false: turn off powersave
 */
void system_powersave(bool enabled)
{
    static uint32_t saved_min_freq;
    static char saved_governor[16];
    FILE *fp;

    if (enabled) {
        char buffer[128];
        FILE *pipe = popen("cpuclock", "r");

        if (pipe) {
            while (fgets(buffer, sizeof(buffer), pipe) != NULL) {
                saved_min_freq = atoi(buffer);
            }
        }

        pclose(pipe);

        // save values for restoring later
        file_get(fp, CPU_SCALING_GOVERNOR, "%15s", saved_governor);
        // set powersaving values
        file_put(fp, CPU_SCALING_MIN_FREQ, "%u", 0);
        file_put(fp, CPU_SCALING_GOVERNOR, "%s", "powersave");
    }
    else {
        // restore
        char sCommand[15];
        sprintf(sCommand, "cpuclock %u", saved_min_freq);
        system(sCommand);
        file_put(fp, CPU_SCALING_GOVERNOR, "%s", saved_governor);
    }
}

#endif // SYSTEM_H__
