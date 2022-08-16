#include <stdlib.h>
#include <stdint.h>
#include <dirent.h>
#include <fcntl.h>
#include <poll.h>
#include <pthread.h>
#include <signal.h>
#include <sys/mman.h>
#include <sys/reboot.h>
#include <sys/stat.h>
#include <linux/fb.h>
#include <linux/input.h>
#include <linux/rtc.h>

#include "utils/utils.h"
#include "utils/log.h"
#include "utils/process.h"
#include "utils/flags.h"
#include "utils/config.h"
#include "system/battery.h"
#include "system/settings.h"
#include "system/settings_sync.h"
#include "system/keymap_hw.h"
#include "system/rumble.h"
#include "system/system.h"
#include "system/screenshot.h"
#include "system/state.h"

#ifndef CLOCK_MONOTONIC_COARSE
#define CLOCK_MONOTONIC_COARSE 6
#endif

// for ev.value
#define RELEASED 0
#define PRESSED  1
#define REPEAT   2

// for button_flag
#define SELECT_BIT 0
#define START_BIT  1
#define L2_BIT     2
#define R2_BIT     3
#define SELECT     (1<<SELECT_BIT)
#define START      (1<<START_BIT)
#define L2         (1<<L2_BIT)
#define R2         (1<<R2_BIT)

// for proc_stat flags
#define PF_KTHREAD 0x00200000

// for suspend / hibernate
#define CHECK_SEC       15    // Interval sec to check hibernate/adc
#define SHUTDOWN_MIN    1    // Minutes to power off after hibernate
#define REPEAT_SEC(val) ((val * 1000 - 250) / 50)
#define PIDMAX          32
uint32_t    suspendpid[PIDMAX];

// Global Variables
int                 input_fd;
struct              input_event ev;
struct              pollfd fds[1];
uint32_t            clkthread_active;
pthread_t           clock_pt;
pthread_mutex_t     clock_mx;

//
//    Set Volume (Raw)
//
#define MI_AO_SETVOLUME 0x4008690b
#define MI_AO_GETVOLUME 0xc008690c

int setVolumeRaw(int volume, int add) {
    int recent_volume = 0;
    int fd = open("/dev/mi_ao", O_RDWR);
    if (fd >= 0) {
        int buf2[] = {0, 0};
        uint64_t buf1[] = {sizeof(buf2), (uintptr_t)buf2};
        ioctl(fd, MI_AO_GETVOLUME, buf1);
        recent_volume = buf2[1];
        if (add) {
            buf2[1] += add;
            if (buf2[1] > 30) buf2[1] = 30;
            else if (buf2[1] < -30) buf2[1] = -30;
        } else buf2[1] = volume;
        if (buf2[1] != recent_volume) ioctl(fd, MI_AO_SETVOLUME, buf1);
        close(fd);
    }
    return recent_volume;
}



//
//    Terminate retroarch before kill/shotdown processes to save progress
//
bool terminate_retroarch(void) {
    char fname[16];
    pid_t pid = process_searchpid("retroarch");
    if (!pid) pid = process_searchpid("ra32");

    if (pid) {
        screenshot_system();
        
        // send signal
        kill(pid, SIGCONT); usleep(100000); kill(pid, SIGTERM);
        // wait for terminate
        sprintf(fname, "/proc/%d", pid);

        uint32_t count = 20; // 4s
        while (--count && exists(fname))
            usleep(200000); // 0.2s

        return true;
    }

    return false;
}

//
//    Suspend / Kill processes
//        mode: 0:STOP 1:TERM 2:KILL
//
int suspend(uint32_t mode) {
    DIR *procdp;
    struct dirent *dir;
    char fname[32];
    pid_t suspend_pid = getpid();
    pid_t pid;
    pid_t ppid;
    char state;
    uint32_t flags;
    char comm[128];
    int ret = 0;

    // terminate retroarch before kill
    if (mode == 2) ret = terminate_retroarch();

    sync();
    procdp = opendir("/proc");

    // Pick active processes to suspend and send SIGSTOP
    // Cond:1. PID is greater than 2(kthreadd) and not myself
    //    2. PPID is greater than 2(kthreadd)
    //    3. state is "R" or "S" or "D"
    //    4. comm is not "(sh)" / "(MainUI)" when AudioFix:OFF
    //    5. flags does not contain PF_KTHREAD (0x200000) (just in case)
    //    6. comm is not "(updater)" "(MainUI)" "(tee)" "(audioserver*" when kill mode
    if (!mode) suspendpid[0] = 0;
    while ((dir = readdir(procdp))) {
        if (dir->d_type == DT_DIR) {
            pid = atoi(dir->d_name);
            if (( pid > 2 )&&( pid != suspend_pid )) {
                sprintf(fname, "/proc/%d/stat", pid);
                FILE *fp = fopen(fname, "r");
                if (fp) {
                    fscanf(fp, "%*d %127s %c %d %*d %*d %*d %*d %u", (char*)&comm, &state, &ppid, &flags);
                    fclose(fp);
                }
                if ( (ppid > 2) && ((state == 'R')||(state == 'S')||(state == 'D')) &&
                     (strcmp(comm,"(sh)")) && (!(flags & PF_KTHREAD)) ) {
                    if (mode) {
                        if ( (strcmp(comm,"(updater)")) && (strcmp(comm,"(MainUI)"))
                          && (strcmp(comm,"(tee)")) && (strncmp(comm,"(audioserver",12)) ) {
                            kill(pid, (mode == 1) ? SIGTERM : SIGKILL); ret++;
                        }
                    } else {
                        if ( suspendpid[0] < PIDMAX ) {
                            suspendpid[++suspendpid[0]] = pid;
                            kill(pid, SIGSTOP); ret++;
                        }
                    }
                }
            }
        }
    }
    closedir(procdp);

    // reset display when anything killed
    if (mode == 2 && ret)
        display_reset();

    return ret;
}

//
//    Resume
//
void resume(void) {
    // Send SIGCONT to suspended processes
    if (suspendpid[0]) {
        for (uint32_t i=1; i <= suspendpid[0]; i++)
            kill(suspendpid[i], SIGCONT);
        suspendpid[0] = 0;
    }
}

//
//    Quit
//
void quit(int exitcode) {
    process_kill("batmon");
    display_free();
    if (input_fd > 0) close(input_fd);
    system_clock_get();
    system_rtc_set();
    system_clock_save();
    exit(exitcode);
}

//
//    Shutdown
//
void shutdown(void) {
    system_shutdown();
    terminate_retroarch();
    system_clock_get();
    system_clock_save();
    sync();
    reboot(RB_AUTOBOOT);
    while(1) pause();
    exit(0);
}

/**
 * @brief stop input event for other processes
 * 
 */
void keyinput_disable(void)
{
    while (ioctl(input_fd, EVIOCGRAB, 1) < 0) { usleep(100000); }
}

/**
 * @brief restart input event for other processes
 * 
 */
void keyinput_enable(void)
{
    while (ioctl(input_fd, EVIOCGRAB, 0) < 0) { usleep(100000); }
}

//
//    Suspend interface
//
void suspend_exec(int timeout) {
    keyinput_disable();

    // suspend
    system_clock_pause(true);
    suspend(0);
    rumble(0);
    int recent_volume = setVolumeRaw(-60,0);
    display_setBrightnessRaw(0);
    display_off();
    system_powersave_on();

    uint32_t repeat_power = 0;
    uint32_t killexit = 0;

    while(1) {
        int ready = poll(fds, 1, timeout);

        if (ready > 0) {
            read(input_fd, &ev, sizeof(ev));
            if (( ev.type != EV_KEY ) || ( ev.value > REPEAT )) continue;
            if ( ev.code == HW_BTN_POWER ) {
                if ( ev.value == RELEASED ) break;
                else if ( ev.value == PRESSED ) repeat_power = 0;
                else if ( ev.value == REPEAT ) {
                    if ( ++repeat_power >= REPEAT_SEC(5) ) {
                        short_pulse();
                        killexit = 1; break;
                    }
                }
            }
            else if ( ev.value == RELEASED ) {
                if ( ev.code == HW_BTN_MENU ) {
                    // screenshot
                    super_short_pulse();
                    system_powersave_off();
                    display_on();
                    screenshot_recent();
                    // display_off();
                    // system_powersave_on();
                    break;
                }
            }
        }
        else if (!ready && !battery_isCharging()) {
            // shutdown
            system_powersave_off(); resume(); usleep(100000); shutdown();
        }
    }

    // resume
    system_powersave_off();
    if (killexit) { resume(); usleep(100000); suspend(2); usleep(400000); }
    display_on();
    display_setBrightness(settings.brightness);
    setVolumeRaw(recent_volume, 0);
    if (!killexit) {
        resume();
        system_clock_pause(false);
    }

    keyinput_enable();
}



//
//    [onion] deepsleep if MainUI/gameSwitcher/retroarch is running
//
void deepsleep(void)
{
    pid_t pid;
    if (check_isMainUI() && (pid = process_searchpid("MainUI"))) {
        short_pulse();
        system_shutdown();
        kill(pid, SIGKILL);
    }
    else if (check_isGameSwitcher() && (pid = process_searchpid("gameSwitcher"))) {
        short_pulse();
        system_shutdown();
        kill(pid, SIGTERM);
    }
    else if (check_gameActive()) {
        if (check_autosave()) {
            short_pulse();
            system_shutdown();
            terminate_retroarch();
        }
    }
}

//
//    Main
//
int main(void) {
    // Initialize
    signal(SIGTERM, quit);
    signal(SIGSEGV, quit);

    settings_init();
    printf_debug("Settings loaded. Brightness set to: %d\n", settings.brightness);

    // Set Initial Volume / Brightness
    setVolumeRaw(0,0);
    display_setBrightness(settings.brightness);

    display_init();

    // Prepare for Poll button input
    input_fd = open("/dev/input/event0", O_RDONLY);
    memset(&fds, 0, sizeof(fds));
    fds[0].fd = input_fd;
    fds[0].events = POLLIN;

    // Main Loop
    uint32_t button_flag = 0;
    uint32_t repeat_LR = 0;
    uint32_t repeat_power = 0;
    uint32_t repeat_menu = 0;
    uint32_t val;
    uint32_t b_BTN_Not_Menu_Pressed = 0;
    uint32_t b_BTN_Menu_Pressed = 0;
    uint32_t power_pressed = 0;
    uint32_t comboKey = 0;

    struct timespec recent;
    struct timespec hibernate_start = recent;
    int hibernate_time;
    int elapsed_sec = 0;

    MenuMode menu_mode = MODE_UNKNOWN;

    // Update recent time
    clock_gettime(CLOCK_MONOTONIC_COARSE, &recent);

    while(1) {
        if (poll(fds, 1, (CHECK_SEC - elapsed_sec) * 1000) > 0) {

            settings_load();

            read(input_fd, &ev, sizeof(ev));
            val = ev.value;

            if (ev.type != EV_KEY || val > REPEAT) continue;

            if (val != REPEAT) {
                if (ev.code == HW_BTN_MENU)
                    b_BTN_Menu_Pressed = val;
                else
                    b_BTN_Not_Menu_Pressed = val;

                if (b_BTN_Menu_Pressed == 1 && b_BTN_Not_Menu_Pressed == 1)
                    comboKey = 1;
            }

            switch (ev.code) {
                case HW_BTN_POWER:
                    if (val == PRESSED)
                        power_pressed = 1;
                    if (val == REPEAT) {
                        repeat_power++;
                        if (repeat_power == 7)
                            deepsleep(); // 0.5sec deepsleep
                        else if (repeat_power == REPEAT_SEC(5)) {
                            short_pulse();
                            remove(CMD_TO_RUN_PATH);
                            suspend(2); // 5sec kill processes
                        }
                        else if (repeat_power >= REPEAT_SEC(10)) {
                            short_pulse();
                            shutdown(); // 10sec force shutdown
                        }
                        break;
                    }
                    if (val == RELEASED) {
                        // suspend
                        if (power_pressed && repeat_power < 7 && !exists("/tmp/stay_awake"))
                            suspend_exec(settings.sleep_timer == 0 ? -1 : (settings.sleep_timer + SHUTDOWN_MIN) * 60000);
                        power_pressed = 0;
                    }
                    repeat_power = 0;
                    break;
                case HW_BTN_SELECT:
                    if (val != REPEAT)
                        button_flag = (button_flag & (~SELECT)) | (val<<SELECT_BIT);
                    break;
                case HW_BTN_START:
                    if (val != REPEAT)
                        button_flag = (button_flag & (~START)) | (val<<START_BIT);
                    break;
                case HW_BTN_L2:
                    if (val == REPEAT) {
                        // Adjust repeat speed to 1/2
                        val = repeat_LR;
                        repeat_LR ^= PRESSED;
                    }
                    else {
                        button_flag = (button_flag & (~L2)) | (val<<L2_BIT);
                        repeat_LR = 0;
                    }
                    if (val == PRESSED) {
                        switch (button_flag & (SELECT|START)) {
                            case START:
                                // SELECT + L2 : volume down / + R2 : reset
                                setVolumeRaw(0, (button_flag & R2) ? 0 : -3);
                                break;
                            case SELECT:
                                // START + L2 : brightness down
                                settings_load();
                                if (settings.brightness > 0) {
                                    settings_setBrightness(settings.brightness - 1, true);
                                    settings_sync();
                                    settings_save();
                                }
                                break;
                            default: break;
                        }
                    }
                    break;
                case HW_BTN_R2:
                    if ( val == REPEAT ) {
                        // Adjust repeat speed to 1/2
                        val = repeat_LR;
                        repeat_LR ^= PRESSED;
                    } else {
                        button_flag = (button_flag & (~R2)) | (val<<R2_BIT);
                        repeat_LR = 0;
                    }
                    if ( val == PRESSED ) {
                        switch (button_flag & (SELECT|START)) {
                        case START:
                            // SELECT + R2 : volume up / + L2 : reset
                            setVolumeRaw(0, (button_flag & L2) ? 0 : +3);
                            break;
                        case SELECT:
                            // START + R2 : brightness up
                            settings_load();
                            if (settings.brightness < MAX_BRIGHTNESS) {
                                settings_setBrightness(settings.brightness + 1, true);
                                settings_sync();
                                settings_save();
                            }
                            break;
                        default:
                            break;
                        }
                    }
                    break;
                case HW_BTN_MENU:
                    if (val == PRESSED) {
                        menu_mode = state_getMenuMode();
                        switch (menu_mode) {
                            case MODE_MAIN_UI: print_debug("Mode is: Main UI"); break;
                            case MODE_SWITCHER: print_debug("Mode is: Game Switcher"); break;
                            case MODE_GAME: print_debug("Mode is: RetroArch"); break;
                            case MODE_APPS: print_debug("Mode is: Apps"); break;
                            default: print_debug("Mode is: Unknown"); break;
                        }
                    }
                    if (val == RELEASED) {
                        if (comboKey == 0) { // short press on menu
                            switch (menu_mode) {
                                case MODE_GAME:
                                    if (check_autosave()) {
                                        menu_super_short_pulse();
                                        run_gameSwitcher(settings.switcher_enabled && !settings.menu_inverted);
                                        terminate_retroarch();
                                    }
                                    break;
                                case MODE_MAIN_UI:
                                    if (settings.switcher_enabled && !settings.menu_inverted) {
                                        menu_super_short_pulse();
                                        force_gameSwitcher();
                                    }
                                    break;
                                default: break;
                            }
                        }
                        comboKey = 0;
                    }
                    if (val == REPEAT) {
                        repeat_menu++;
                        if (repeat_menu == REPEAT_SEC(1) && !button_flag) { // long press on menu
                            switch (menu_mode) {
                                case MODE_GAME:
                                    short_pulse();
                                    run_gameSwitcher(settings.switcher_enabled && settings.menu_inverted);
                                    terminate_retroarch();
                                    break;
                                case MODE_MAIN_UI:
                                    if (settings.menu_inverted) {
                                        if (settings.switcher_enabled) {
                                            super_short_pulse();
                                            force_gameSwitcher();
                                        }
                                    }
                                    else {
                                        // rumble when holding for popup menu (refresh roms)
                                        super_short_pulse();
                                    }
                                    break;
                                default: break;
                            }
                            repeat_menu = 0;
                            comboKey = 1;  // this will avoid to trigger short press action
                        }
                    }
                    break;
                default:
                    break;
            }

            clock_gettime(CLOCK_MONOTONIC_COARSE, &hibernate_start);
            elapsed_sec = hibernate_start.tv_sec - recent.tv_sec;
            if (elapsed_sec < CHECK_SEC) continue;
        }

        // Comes here every CHECK_SEC(def:15) seconds interval

        // Update recent time
        clock_gettime(CLOCK_MONOTONIC_COARSE, &recent);

        // Check Hibernate
        if (temp_flag_get("battery_charging"))
            hibernate_time = 1;
        else
            hibernate_time = settings.sleep_timer;

        if (hibernate_time && !temp_flag_get("stay_awake")) {
            if (recent.tv_sec - hibernate_start.tv_sec >= hibernate_time * 60) {
                suspend_exec(SHUTDOWN_MIN * 60000);
                clock_gettime(CLOCK_MONOTONIC_COARSE, &hibernate_start);
            }
        }

        // Quit RetroArch / auto-save when battery too low
        if (battery_getPercentage() <= 4 && settings.low_battery_autosave && check_autosave())
            terminate_retroarch();

        elapsed_sec = 0;
    }
}
