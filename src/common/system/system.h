#ifndef SYSTEM_H__
#define SYSTEM_H__

#include <fcntl.h>
#include <stdbool.h>

#include "clock.h"
#include "utils/flags.h"

#ifndef DT_DIR
#define DT_DIR 4
#endif

// system directories
#define GPIO_DIR1 "/sys/class/gpio/"
#define GPIO_DIR2 "/sys/devices/gpiochip0/gpio/"
#define PWM_DIR   "/sys/devices/soc0/soc/1f003400.pwm/pwm/pwmchip0/"
#define CPU_DIR   "/sys/devices/system/cpu/cpufreq/policy0/"

#define CPU_SCALING_MIN_FREQ CPU_DIR "scaling_min_freq"
#define CPU_SCALING_GOVERNOR CPU_DIR "scaling_governor"

#define system_powersave_on() system_powersave(true)
#define system_powersave_off() system_powersave(false)

/**
 * @brief Creates the file `/mnt/SDCARD/.tmp_update/.offOrder`,
 * which is used by `runtime.sh` for knowing when to show shutdown screen.
 * 
 */
void system_shutdown(void) {
    settings_flag_set(".offOrder", true);
    system_clock_get();
    system_clock_save();
    sync();
}

/**
 * @brief Toggle powersave mode
 * 
 * @param enabled true: turn on powersave, false: turn off powersave
 */
void system_powersave(bool enabled) {
    static uint32_t saved_min_freq;
    static char saved_governor[16];
    FILE* fp;

    if (enabled) {
        // save values for restoring later
        file_get(fp, CPU_SCALING_MIN_FREQ, "%u", &saved_min_freq);
        file_get(fp, CPU_SCALING_GOVERNOR, "%15s", saved_governor);
        // set powersaving values
        file_put(fp, CPU_SCALING_MIN_FREQ, "%u", 0);
        file_put(fp, CPU_SCALING_GOVERNOR, "%s", "powersave");
    }
    else {
        // restore
        file_put(fp, CPU_SCALING_MIN_FREQ, "%u", saved_min_freq);
        file_put(fp, CPU_SCALING_GOVERNOR, "%s", saved_governor);
    }
}

//
//    Search pid of running executable (forward match)
//
pid_t system_searchpid(const char *commname) {
    DIR *procdp;
    struct dirent *dir;
    char fname[24];
    char comm[128];
    pid_t pid;
    pid_t ret = 0;
    size_t commlen = strlen(commname);

    procdp = opendir("/proc");
    while ((dir = readdir(procdp))) {
        if (dir->d_type == DT_DIR) {
            pid = atoi(dir->d_name);
            if ( pid > 2 ) {
                sprintf(fname, "/proc/%d/comm", pid);
                FILE *fp = fopen(fname, "r");
                if (fp) {
                    fscanf(fp, "%127s", comm);
                    fclose(fp);
                    if (!strncmp(comm, commname, commlen)) { ret = pid; break; }
                }
            }
        }
    }
    closedir(procdp);
    return ret;
}

#endif // SYSTEM_H__
