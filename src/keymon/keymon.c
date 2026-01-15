#include <SDL/SDL.h>
#include <SDL/SDL_ttf.h>
#include <dirent.h>
#include <fcntl.h>
#include <poll.h>
#include <signal.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/reboot.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>

#include "system/axp.h"
#include "system/battery.h"
#include "system/device_model.h"
#include "system/display.h"
#include "system/keymap_hw.h"
#include "system/osd.h"
#include "system/rumble.h"
#include "system/screenshot.h"
#include "system/settings.h"
#include "system/settings_sync.h"
#include "system/state.h"
#include "system/system.h"
#include "system/system_utils.h"
#include "system/volume.h"
#include "theme/config.h"
#include "theme/render.h"
#include "theme/resources.h"
#include "utils/config.h"
#include "utils/file.h"
#include "utils/flags.h"
#include "utils/log.h"
#include "utils/msleep.h"
#include "utils/process.h"
#include "utils/str.h"

#include "./input_fd.h"
#include "./menuButtonAction.h"

#define FAVORITES_PATH "/mnt/SDCARD/Roms/favourite.json"

// for proc_stat flags
#define PF_KTHREAD 0x00200000

// for suspend / hibernate
#define CHECK_SEC 15   // Interval sec to check hibernate/adc
#define SHUTDOWN_MIN 1 // Minutes to power off after hibernate
#define REPEAT_SEC(val) ((val * 1000 - 250) / 50)
#define PIDMAX 32

uint32_t suspendpid[PIDMAX];

const int KONAMI_CODE[] = {HW_BTN_UP, HW_BTN_UP, HW_BTN_DOWN, HW_BTN_DOWN,
                           HW_BTN_LEFT, HW_BTN_RIGHT, HW_BTN_LEFT, HW_BTN_RIGHT,
                           HW_BTN_B, HW_BTN_A};
const int KONAMI_CODE_LENGTH = sizeof(KONAMI_CODE) / sizeof(KONAMI_CODE[0]);

void takeScreenshot(void)
{
    super_short_pulse();
    display_setBrightnessRaw(0);
    msleep(10);
    osd_hideBar();
    screenshot_recent();
    settings_setBrightness(settings.brightness, true, false);
}

//
//    Suspend / Kill processes
//        mode: 0:STOP 1:TERM 2:KILL
//
int suspend(uint32_t mode)
{
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
    if (mode == 2) {
        screenshot_system();
        terminate_retroarch();
        terminate_drastic();
    }

    sync();
    procdp = opendir("/proc");

    // Pick active processes to suspend and send SIGSTOP
    // Cond:1. PID is greater than 2(kthreadd) and not myself
    //    2. PPID is greater than 2(kthreadd)
    //    3. state is "R" or "S" or "D"
    //    4. comm is not "(sh)" / "(MainUI)" when AudioFix:OFF
    //    5. flags does not contain PF_KTHREAD (0x200000) (just in case)
    //    6. comm is not "(updater)" "(MainUI)" "(tee)" "(audioserver*" when
    //    kill mode
    if (!mode)
        suspendpid[0] = 0;
    while ((dir = readdir(procdp))) {
        if (dir->d_type == DT_DIR) {
            pid = atoi(dir->d_name);
            if ((pid > 2) && (pid != suspend_pid)) {
                sprintf(fname, "/proc/%d/stat", pid);
                FILE *fp = fopen(fname, "r");
                if (fp) {
                    fscanf(fp, "%*d %127s %c %d %*d %*d %*d %*d %u",
                           (char *)&comm, &state, &ppid, &flags);
                    fclose(fp);
                }
                if ((ppid > 2) &&
                    ((state == 'R') || (state == 'S') || (state == 'D')) &&
                    (strcmp(comm, "(sh)")) && (!(flags & PF_KTHREAD))) {
                    if (mode) {
                        if ((strcmp(comm, "(runtime.sh)")) &&
                            (strcmp(comm, "(updater)")) &&
                            (strcmp(comm, "(MainUI)")) &&
                            (strcmp(comm, "(tee)")) &&
                            (strncmp(comm, "(audioserver", 12)) &&
                            (strcmp(comm, "(batmon)"))) {
                            kill(pid, (mode == 1) ? SIGTERM : SIGKILL);
                            ret++;
                        }
                    }
                    else {
                        if (suspendpid[0] < PIDMAX) {
                            suspendpid[++suspendpid[0]] = pid;
                            kill(pid, SIGSTOP);
                            ret++;
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
void resume(void)
{
    // Send SIGCONT to suspended processes
    if (suspendpid[0]) {
        for (uint32_t i = 1; i <= suspendpid[0]; i++)
            kill(suspendpid[i], SIGCONT);
        suspendpid[0] = 0;
    }
}

//
//    Quit
//
void quit(int exitcode)
{
    display_close();
    if (input_fd > 0)
        close(input_fd);
    system_clock_get();
    system_rtc_set();
    system_clock_save();
    exit(exitcode);
}

//
//    Shutdown
//
void force_shutdown(void)
{
    set_system_shutdown();
    screenshot_system();
    terminate_retroarch();
    terminate_drastic();
    system_clock_get();
    system_clock_save();
    sync();
    system("shutdown");
    while (1)
        pause();
    exit(0);
}

void wait(int seconds)
{
    time_t t = time(NULL);
    while ((time(NULL) - t) < seconds) {
        sleep(1);
    }
}

void showBootScreen(const char *type)
{
    char cmd[256];
    sprintf(cmd, "bootScreen \"%s\" &", type);
    system(cmd);
}

//
//    [onion] deepsleep if MainUI/gameSwitcher/retroarch is running
//
void deepsleep(void)
{
    system_state_update();

    if (system_state == MODE_GAME && !check_autosave()) {
        return;
    }

    short_pulse();
    set_system_shutdown();

    if (system_state == MODE_MAIN_UI) {
        kill_mainUI();
    }
    else if (system_state == MODE_SWITCHER) {
        kill(system_state_pid, SIGTERM);
    }
    else if (system_state == MODE_GAME) {
        screenshot_system();
        retroarch_pause();
        showBootScreen("End_Save");
        terminate_retroarch();
    }
    else if (system_state == MODE_ADVMENU) {
        kill(system_state_pid, SIGQUIT);
    }
    else if (system_state == MODE_APPS) {
        remove(CMD_TO_RUN_PATH);
        suspend(1);
    }
    else if (system_state == MODE_DRASTIC) {
        screenshot_system();
        terminate_drastic();
    }

    // Wait 30s before forcing a shutdown
    wait(30);
    if (!temp_flag_get("shutting_down")) {
        force_shutdown();
    }
}

//
//    Suspend interface
//
void suspend_exec(int timeout)
{
    bool stay_awake = timeout == -1;
    keyinput_disable();

    // pause playActivity
    system("playActivity stop_all");

    // suspend
    if (!stay_awake) {
        suspend(0);
        setVolume(0);
    }
    rumble(0);
    display_setBrightnessRaw(0);
    display_off();
    system_powersave_on();

    uint32_t repeat_power = 0;
    uint32_t killexit = 0;

    while (1) {
        int ready = poll(fds, 1, timeout);

        if (ready > 0) {
            read(input_fd, &ev, sizeof(ev));
            if ((ev.type != EV_KEY) || (ev.value > REPEAT))
                continue;
            if (ev.code == HW_BTN_POWER) {
                if (ev.value == RELEASED)
                    break;
                else if (ev.value == PRESSED)
                    repeat_power = 0;
                else if (ev.value == REPEAT) {
                    if (++repeat_power >= REPEAT_SEC(5)) {
                        short_pulse();
                        killexit = 1;
                        break;
                    }
                }
            }
            else if (ev.value == RELEASED) {
                if (ev.code == HW_BTN_MENU) {
                    // screenshot
                    system_powersave_off();
                    display_on();
                    takeScreenshot();
                    break;
                }
            }
        }
        else if (!ready && !battery_isCharging()) {
            // shutdown
            system_powersave_off();
            resume();
            usleep(150000);
            deepsleep();
        }
    }

    // resume
    system_powersave_off();
    if (killexit) {
        resume();
        usleep(150000);
        suspend(2);
        usleep(400000);
    }
    display_on();
    display_setBrightness(settings.brightness);
    setVolume(settings.mute ? 0 : settings.volume);
    if (!killexit) {
        // resume processes
        resume();
        // resume playActivity
        system("playActivity resume");
    }

    keyinput_enable();
}

void turnOffScreen(void)
{
    int timeout = (settings.sleep_timer + SHUTDOWN_MIN) * 60000;
    bool stay_awake = settings.sleep_timer == 0 || temp_flag_get("stay_awake");
    suspend_exec(stay_awake ? -1 : timeout);
}

/**
 * @brief Adjusts the CPU clock speed based on the given adjustment value and triggers an OSD message
 *
 * This function reads the current CPU clock speed, adjusts it by the specified
 * amount, and sets the new CPU clock speed if it falls within the allowed range.
 * It then triggers an OSD message to display the new CPU clock speed.
 * If the new CPU clock speed is outside the allowed range, a short pulse is triggered
 * and the function returns without making any changes.
 *
 * @param adjust The amount to adjust the CPU clock speed by (in MHz).
 *
 * @return void
 */
void cpuClockHotkey(int adjust)
{
    if (config_flag_get(".cpuClockHotkey") == 0) {
        return;
    }
    printf_debug("cpuClockHotkey: %d\n", adjust);
    int min_cpu_clock = 500; // ?
    int max_cpu_clock;
    switch (DEVICE_ID) {
    case MIYOO354:
        max_cpu_clock = 1800;
        break;
    case MIYOO283:
        max_cpu_clock = 1600;
        break;
    default:
        // Unknown device
        return;
    }
    char cpuclockstr[5];

    // Read current CPU clock
    int ret = process_start_read_return("cpuclock", cpuclockstr);
    int cpuclock = atoi(cpuclockstr);
    printf_debug("Current CPU clock: %d\n", cpuclock);
    cpuclock += adjust;
    printf_debug("Desired CPU clock: %d\n", cpuclock);
    // Bounds check
    if (cpuclock < min_cpu_clock || cpuclock > max_cpu_clock) {
        printf_debug("Desired CPU clock %d out of range (%d, %d)\n", cpuclock, min_cpu_clock, max_cpu_clock);
        SDL_Surface *surface = theme_createTextOverlay("CPU clock out of range", (SDL_Color){255, 255, 255}, (SDL_Color){0, 0, 0}, 1.0, 10);
        if (surface && overlay_surface(surface, 10, 10, 1000, true) != 0) {
            SDL_FreeSurface(surface);
        }
        short_pulse();
        return;
    }

    // Set new CPU clock
    char cmd[STR_MAX];
    snprintf(cmd, STR_MAX, "cpuclock %d", cpuclock);
    ret = process_start_read_return(cmd, cpuclockstr);
    if (ret == 0) {
        printf_debug("Updated CPU clock: %s\n", cpuclockstr);
        char osd_txt[STR_MAX];
        snprintf(osd_txt, STR_MAX, "CPU clock set to %s MHz", cpuclockstr);
        SDL_Surface *surface = theme_createTextOverlay(osd_txt, (SDL_Color){255, 255, 255}, (SDL_Color){0, 0, 0}, 1.0, 10);
        if (surface && overlay_surface(surface, 10, 10, 1000, true) != 0) {
            SDL_FreeSurface(surface);
        }
    }
}

static void signal_refresh(int sig)
{
    display_getRenderResolution();
}

//
//    Main
//
int main(void)
{
    // Initialize
    signal(SIGTERM, quit);
    signal(SIGSEGV, quit);
    signal(SIGUSR1, signal_refresh);
    log_setName("keymon");

    getDeviceModel();

    if (DEVICE_ID == MIYOO354) {
        // set hardware poweroff time to 10s
        axp_write(0x36, axp_read(0x36) | 3);
    }

    settings_init();

    // Set Initial Volume / Brightness
    setVolume(settings.mute ? 0 : settings.volume);
    display_setBrightness(settings.brightness);
    printf_debug("Settings loaded. Brightness set to: %d\n",
                 settings.brightness);

    display_init(true);

    // Prepare for Poll button input
    input_fd = open("/dev/input/event0", O_RDONLY);
    memset(&fds, 0, sizeof(fds));
    fds[0].fd = input_fd;
    fds[0].events = POLLIN;

    // Main Loop
    uint32_t button_flag = 0;
    uint32_t repeat_LR = 0;
    uint32_t repeat_power = 0;
    uint32_t val;
    int konamiCodeIndex = 0;
    bool b_BTN_Not_Menu_Pressed = false;
    bool b_BTN_Menu_Pressed = false;
    bool power_pressed = false;
    bool volUp_state = false;
    bool volUp_active = false;
    bool volDown_state = false;
    bool volDown_active = false;
    bool comboKey_volume = false;
    bool comboKey_menu = false;
    bool comboKey_select = false;
    bool menuAndAPressed = false; // screen recorder
    bool menuAndBPressed = false; // blue light filter
    int menuAndAPressedTime = 0;
    int menuAndBPressedTime = 0;

    int ticks = getMilliseconds();
    int hibernate_start = ticks;
    int hibernate_time;
    int elapsed_sec = 0;

    bool delete_flag = false;
    bool settings_changed = false;

    int save_settings_timestamp = 0;
    volatile bool needWriteSettings = false;

    time_t fav_last_modified = time(NULL);

    while (1) {
        if (poll(fds, 1, (CHECK_SEC - elapsed_sec) * 1000) > 0) {
            if (!keyinput_isValid())
                continue;
            val = ev.value;

            printf_debug("Keymon input: code=%d, value=%d\n", ev.code,
                         ev.value);

            if (exists("/tmp/settings_changed")) {
                settings_load();
                remove("/tmp/settings_changed");
                sync();
            }

            if (exists("/tmp/state_changed")) {
                system_state_update();

                if (delete_flag) {
                    system_state_update();
                    remove("/tmp/state_changed");
                    sync();
                    delete_flag = false;
                }

                if (system_state == MODE_MAIN_UI)
                    display_setBrightness(settings.brightness);
            }

            settings_changed = false;
            osd_bar_activated = false;

            if (val != REPEAT) {

                if (ev.code == HW_BTN_MENU)
                    b_BTN_Menu_Pressed = val == PRESSED;
                else
                    b_BTN_Not_Menu_Pressed = val == PRESSED;

                if (b_BTN_Menu_Pressed && b_BTN_Not_Menu_Pressed)
                    comboKey_menu = true;
            }

            if (system_state == MODE_MAIN_UI && (ev.code == HW_BTN_B || ev.code == HW_BTN_X) && val == RELEASED) {
                // Check if favorite file changed
                if (file_isModified(FAVORITES_PATH, &fav_last_modified)) {
                    system("tools favfix");
                    sync();
                }
            }

            switch (ev.code) {
            case HW_BTN_POWER:
                if (val == PRESSED)
                    power_pressed = true;
                if (!comboKey_menu && val == REPEAT) {
                    repeat_power++;
                    if (repeat_power == 7) {
                        deepsleep(); // 0.5sec deepsleep
                    }
                    else if (repeat_power >= REPEAT_SEC(5)) {
                        short_pulse();
                        remove(CMD_TO_RUN_PATH);
                        force_shutdown(); // 10sec force shutdown
                    }
                    break;
                }
                if (val == RELEASED) {
                    // suspend
                    if (power_pressed && repeat_power < 7) {
                        if (comboKey_menu) {
                            takeScreenshot();
                        }
                        else {
                            if (settings.disable_standby) {
                                deepsleep();
                            }
                            else if (settings.sleep_timer == 0) {
                                suspend_exec(SHUTDOWN_MIN * 60000);
                            }
                            else {
                                turnOffScreen();
                            }
                        }
                    }
                    power_pressed = false;
                }
                repeat_power = 0;
                break;
            case HW_BTN_SELECT:
                if (!comboKey_select && val == RELEASED) {
                    if (system_state == MODE_MAIN_UI) {
                        keyinput_send(HW_BTN_MENU, PRESSED);
                        keyinput_send(HW_BTN_MENU, RELEASED);
                    }
                }
                if (val != REPEAT)
                    button_flag =
                        (button_flag & (~SELECT)) | (val << SELECT_BIT);
                if (val == RELEASED)
                    comboKey_select = false;
                break;
            case HW_BTN_START:
                if (val != REPEAT)
                    button_flag = (button_flag & (~START)) | (val << START_BIT);
                break;
            case HW_BTN_R1:
                if (val == PRESSED && (button_flag & (SELECT | START)) == (SELECT | START)) {
                    cpuClockHotkey(100);
                }
                break;
            case HW_BTN_L1:
                if (val == PRESSED && (button_flag & (SELECT | START)) == (SELECT | START)) {
                    cpuClockHotkey(-100);
                }
                break;
            case HW_BTN_L2:
                if (val == REPEAT) {
                    // Adjust repeat speed to 1/2
                    val = repeat_LR;
                    repeat_LR ^= PRESSED;
                }
                else {
                    button_flag = (button_flag & (~L2)) | (val << L2_BIT);
                    repeat_LR = 0;
                }
                if (val == PRESSED) {
                    switch (button_flag & (SELECT | START)) {
                    case START:
                        // START + L2 : audio boost down / + R2 : reset volume
                        if (button_flag & R2)
                            setVolume(settings.volume);
                        else
                            setVolumeRaw(0, -3);
                        break;
                    case SELECT:
                        if (DEVICE_ID == MIYOO354)
                            break; // disable this shortcut for MMP
                        // SELECT + L2 : brightness down
                        if (config_flag_get(".altBrightness"))
                            break;
                        if (settings.brightness > 0) {
                            settings_setBrightness(settings.brightness - 1,
                                                   true, false);
                            settings_changed = true;
                        }
                        osd_showBrightnessBar(settings.brightness);
                        comboKey_select = true;
                        break;
                    default:
                        break;
                    }
                }
                break;
            case HW_BTN_R2:
                if (val == REPEAT) {
                    // Adjust repeat speed to 1/2
                    val = repeat_LR;
                    repeat_LR ^= PRESSED;
                }
                else {
                    button_flag = (button_flag & (~R2)) | (val << R2_BIT);
                    repeat_LR = 0;
                }
                if (val == PRESSED) {
                    switch (button_flag & (SELECT | START)) {
                    case START:
                        // START + R2 : audio boost up / + L2 : reset volume
                        if (button_flag & L2)
                            setVolume(settings.volume);
                        else
                            setVolumeRaw(0, +3);
                        break;
                    case SELECT:
                        if (DEVICE_ID == MIYOO354)
                            break; // disable this shortcut for MMP
                        // SELECT + R2 : brightness up
                        if (config_flag_get(".altBrightness"))
                            break;
                        if (settings.brightness < MAX_BRIGHTNESS) {
                            settings_setBrightness(settings.brightness + 1,
                                                   true, false);
                            settings_changed = true;
                        }
                        osd_showBrightnessBar(settings.brightness);
                        comboKey_select = true;
                        break;
                    default:
                        break;
                    }
                }
                break;
            case HW_BTN_MENU:

                if (!temp_flag_get("disable_menu_button")) {
                    system_state_update();
                    comboKey_menu = menuButtonAction(val, comboKey_menu);
                }

                break;
            case HW_BTN_X:
                if (val == PRESSED && system_state == MODE_MAIN_UI)
                    temp_flag_set("launch_alt", false);
                if (val == PRESSED)
                    applyExtraButtonShortcut(0);
                break;
            case HW_BTN_Y:
                if (val == PRESSED)
                    applyExtraButtonShortcut(1);
                break;
            case HW_BTN_A:
                if (val == PRESSED) {
                    if (b_BTN_Menu_Pressed) {
                        menuAndAPressed = true;
                        menuAndAPressedTime = getMilliseconds();
                    }
                }
                else if (val == RELEASED) {
                    menuAndAPressed = false;
                }
                break;
            case HW_BTN_B:
                if (val == PRESSED) {
                    if (b_BTN_Menu_Pressed) {
                        menuAndBPressed = true;
                        menuAndBPressedTime = getMilliseconds();
                    }
                }
                else if (val == RELEASED) {
                    menuAndBPressed = false;
                }
                if (val == PRESSED && system_state == MODE_MAIN_UI)
                    temp_flag_set("launch_alt", false);
                break;
            case HW_BTN_VOLUME_DOWN:
                if (comboKey_menu) {
                    // MENU + VOL DOWN : brightness down
                    if (val != RELEASED && settings.brightness > 0) {
                        settings_setBrightness(settings.brightness - 1, true,
                                               false);
                        settings_changed = true;
                    }
                    osd_showBrightnessBar(settings.brightness);
                    break;
                }
                if (val == PRESSED)
                    volDown_active = true;
                volDown_state = val;
                if (!volDown_active || comboKey_volume)
                    break;
                if (val == RELEASED || val == REPEAT) {
                    if (settings_setVolume(settings.volume - 1, true))
                        settings_changed = true;
                }
                if (val == RELEASED)
                    volDown_active = false;
                osd_showVolumeBar(settings.volume, settings.mute);
                break;
            case HW_BTN_DOWN:
                if (DEVICE_ID == MIYOO283) {
                    if (comboKey_menu) {
                        if (config_flag_get(".altBrightness")) {
                            // MENU + BTN DOWN : brightness down
                            if (val != RELEASED && settings.brightness > 0) {
                                settings_setBrightness(settings.brightness - 1, true,
                                                       false);
                                settings_changed = true;
                            }
                            osd_showBrightnessBar(settings.brightness);
                        }
                    }
                }
                break;
            case HW_BTN_VOLUME_UP:
                if (comboKey_menu) {
                    // MENU + VOL UP : brightness up
                    if (val != RELEASED &&
                        settings.brightness < MAX_BRIGHTNESS) {
                        settings_setBrightness(settings.brightness + 1, true,
                                               false);
                        settings_changed = true;
                    }
                    osd_showBrightnessBar(settings.brightness);
                    break;
                }
                if (val == PRESSED)
                    volUp_active = true;
                volUp_state = val;
                if (!volUp_active || comboKey_volume)
                    break;
                if (val == RELEASED || val == REPEAT) {
                    if (settings_setMute(0, true)) {
                        settings_changed = true;
                    }
                    else if (settings_setVolume(settings.volume + 1, true))
                        settings_changed = true;
                }
                if (val == RELEASED)
                    volUp_active = false;
                osd_showVolumeBar(settings.volume, settings.mute);
                break;
            case HW_BTN_UP:
                if (DEVICE_ID == MIYOO283) {
                    if (comboKey_menu) {
                        if (config_flag_get(".altBrightness")) {
                            // MENU + BTN UP : brightness up
                            if (val != RELEASED &&
                                settings.brightness < MAX_BRIGHTNESS) {
                                settings_setBrightness(settings.brightness + 1, true,
                                                       false);
                                settings_changed = true;
                            }
                            osd_showBrightnessBar(settings.brightness);
                        }
                    }
                }
                break;
            default:
                break;
            }

            // start screen recording after holding for >2secs
            if (menuAndAPressed && (getMilliseconds() - menuAndAPressedTime >= 2000)) {
                if (access("/mnt/SDCARD/.tmp_update/config/.recHotkey", F_OK) != -1) {
                    system("/mnt/SDCARD/.tmp_update/script/screen_recorder.sh toggle &");
                }

                menuAndAPressed = false;
                menuAndAPressedTime = 0;
            }

            // toggle blue light filter
            if (menuAndBPressed && (getMilliseconds() - menuAndBPressedTime >= 2000)) {
                if (access("/tmp/.blfOn", F_OK) != -1) {
                    system("/mnt/SDCARD/.tmp_update/script/blue_light.sh disable &");
                    system("touch /tmp/.blfIgnoreSchedule");
                }
                else {
                    system("/mnt/SDCARD/.tmp_update/script/blue_light.sh enable &");
                    system("touch /tmp/.blfIgnoreSchedule");
                }

                menuAndBPressed = false;
                menuAndBPressedTime = 0;
            }

            if (val == PRESSED && !osd_bar_activated) {
                osd_hideBar();
            }

            // Mute toggle
            if (volDown_state != RELEASED && volUp_state != RELEASED &&
                !comboKey_volume) {
                comboKey_volume = true;
                if (settings_setMute(!settings.mute, true))
                    settings_changed = true;
                osd_showVolumeBar(settings.volume, settings.mute);
            }
            else if (volDown_state == RELEASED && volUp_state == RELEASED)
                comboKey_volume = false;

            if (settings_changed) {
                settings_shm_write();
                needWriteSettings = true;
                save_settings_timestamp = ticks;
            }

            if (needWriteSettings && (ticks - save_settings_timestamp) > 150) {
                settings_save();
                needWriteSettings = false;
            }

            if ((val == PRESSED) && (system_state == MODE_MAIN_UI)) {
                // Check for Konami code
                if (ev.code == KONAMI_CODE[konamiCodeIndex]) {
                    ++konamiCodeIndex;
                    if (konamiCodeIndex == KONAMI_CODE_LENGTH) {
                        // The entire Konami code was entered!
                        FILE *file =
                            fopen("/mnt/SDCARD/.tmp_update/cmd_to_run.sh", "w");
                        fputs("cd /mnt/SDCARD/.tmp_update/bin; ./easter", file);
                        fclose(file);

                        konamiCodeIndex = 0;
                        kill_mainUI();
                    }
                }
                else {
                    konamiCodeIndex = (ev.code == HW_BTN_UP);
                }
            }

            if (system_state == MODE_MAIN_UI) {
                settings_shm_read();
                display_setBrightness(settings.brightness);
            }

            hibernate_start = getMilliseconds();
            elapsed_sec = (hibernate_start - ticks) / 1000;
            if (elapsed_sec < CHECK_SEC)
                continue;
        }

        // Comes here every CHECK_SEC(def:15) seconds interval
        if (delete_flag) {
            if (exists("/tmp/state_changed")) {
                system_state_update();
                remove("/tmp/state_changed");
                sync();
            }
            delete_flag = false;
        }
        else {
            delete_flag = true;
        }

        // Update ticks
        ticks = getMilliseconds();

        // Check Hibernate
        if (battery_isCharging())
            hibernate_time = 0;
        else
            hibernate_time = settings.sleep_timer;

        if (hibernate_time && !temp_flag_get("stay_awake")) {
            if (ticks - hibernate_start > hibernate_time * 60 * 1000) {
                suspend_exec(SHUTDOWN_MIN * 60000);
                hibernate_start = ticks;
            }
        }

        // Check bluelight filter
        if (settings.blue_light_schedule) {
            system("/mnt/SDCARD/.tmp_update/script/blue_light.sh check");
        }

        // Quit RetroArch / auto-save when battery too low
        if (settings.low_battery_autosave_at && battery_getPercentage() <= settings.low_battery_autosave_at && check_autosave()) {
            temp_flag_set(".lowBat", true);
            screenshot_system();
            terminate_retroarch();
            terminate_drastic();
        }

        elapsed_sec = 0;
    }
}
