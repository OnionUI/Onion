
//
//    Stock compatible custom keymon for miyoomini
//

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
#include <png.h>

#include "../common/settings.h"
#include "../common/keymap_hw.h"
#include "../common/utils.h"
#include "../common/rumble.h"
#include "../common/battery.h"
#include "../common/system.h"

#ifndef DT_DIR
#define DT_DIR 4
#endif

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
#define MI_AO_SETVOLUME    0x4008690b
#define MI_AO_GETVOLUME    0xc008690c
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
//    Search pid of running executable (forward match)
//
pid_t searchpid(const char *commname) {
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

//
//    Terminate retroarch before kill/shotdown processes to save progress
//
int terminate_retroarch(void) {
    char fname[16];
    pid_t pid = searchpid("retroarch");
    if (!pid) pid = searchpid("ra32");

    if (pid) {
        // send signal
        kill(pid, SIGCONT); usleep(100000); kill(pid, SIGTERM);
        // wait for terminate
        sprintf(fname, "/proc/%d", pid);

        uint32_t count = 20; // 4s
        while (--count && file_exists(fname))
            usleep(200000); // 0.2s

        return 1;
    }
    return 0;
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

    // reset FB when anything killed
    if ((mode == 2)&&(ret)) {
        ioctl(fb_fd, FBIOGET_VSCREENINFO, &vinfo);
        vinfo.yoffset = 0;
        memset(fb_addr, 0, finfo.smem_len);
        ioctl(fb_fd, FBIOPUT_VSCREENINFO, &vinfo);
    }

    return ret;
}

//
//    Resume
//
void resume(void) {
    // Send SIGCONT to suspended processes
    if (suspendpid[0]) {
        for (uint32_t i=1; i <= suspendpid[0]; i++) {
            kill(suspendpid[i], SIGCONT);
        }
        suspendpid[0] = 0;
        // super_short_pulse();
    }
}

//
//    [onion] get recent filename from content_history.lpl
//
char* getrecent_onion(char *filename) {
    FILE    *fp;
    char    key[256], val[256];
    char    *keyptr, *valptr, *strptr;
    int    f;

    *filename = 0;
    if ( (fp = fopen("/mnt/SDCARD/Saves/CurrentProfile/lists/content_history.lpl", "r")) ) {
        key[0] = 0; val[0] = 0;
        while ((f = fscanf(fp, "%255[^:]:%255[^\n]\n", key, val)) != EOF) {
            if (!f) { if (fscanf(fp, "%*[^\n]\n") == EOF) break; else continue; }
            if ( ((keyptr = trimstr(key, 0))) && ((valptr = trimstr(val, 1))) ) {
                if ( (!strcmp(keyptr, "\"path\"")) && ((valptr = strrchr(valptr, '/'))) ) {
                    valptr++;
                    if ((strptr = strrchr(valptr, '"'))) *strptr = 0;
                    strcpy(filename, valptr);
                    break;
                }
            }
            key[0] = 0; val[0] = 0;
        }
        fclose(fp);
    }
    if (*filename == 0) return NULL;
    return filename;
}

//
//    Get most recent file name for screenshot
//
char* getrecent_png(char *filename) {
    FILE        *fp;
    char        *fnptr,    *strptr;
    uint32_t    i;

    strcpy(filename, "/mnt/SDCARD/Screenshots/");
    if (!file_exists(filename)) mkdir(filename, 777);

    fnptr = filename + strlen(filename);

    if (file_exists("/tmp/cmd_to_run.sh")) {
        // for stock
        if ((fp = fopen("/mnt/SDCARD/Roms/recentlist.json", "r"))) {
            fscanf(fp, "%*255[^:]:\"%255[^\"]", fnptr);
            fclose(fp);
        }
    } else if (file_exists("/tmp/cmd_to_run_launcher.sh")) {
        // for gameSwitcher
        if (getrecent_onion(fnptr)) {
            if ((strptr = strrchr(fnptr, '.'))) *strptr = 0;
        }
    }

    if (!(*fnptr)) {
        if (searchpid("gameSwitcher")) strcat(filename, "gameSwitcher");
        else strcat(filename, "MainUI");
    }

    fnptr = filename + strlen(filename);
    for (i=0; i<1000; i++) {
        sprintf(fnptr, "_%03d.png", i);
        if (!file_exists(filename)) break;
    }
    if (i > 999)
        return NULL;
    return filename;
}

//
//    Screenshot (640x480x32bpp only, rotate180, png)
//
void screenshot(void) {
    char        screenshotname[512];
    uint32_t    *buffer;
    uint32_t    *src;
    uint32_t    linebuffer[640], x, y, pix;
    FILE        *fp;
    png_structp    png_ptr;
    png_infop    info_ptr;

    if (getrecent_png(screenshotname) == NULL) return;

    ioctl(fb_fd, FBIOGET_VSCREENINFO, &vinfo);
    buffer = fb_addr + 640*vinfo.yoffset;

    if ((fp = fopen(screenshotname, "wb"))) {
        png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, 0, 0, 0);
        info_ptr = png_create_info_struct(png_ptr);
        png_init_io(png_ptr, fp);
        png_set_IHDR(png_ptr, info_ptr, 640, 480, 8, PNG_COLOR_TYPE_RGBA,
            PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);
        png_write_info(png_ptr, info_ptr);
        src = buffer + 640*480;
        for (y=0; y<480; y++) {
            for (x=0; x<640; x++){
                pix = *--src;
                linebuffer[x] = 0xFF000000 | (pix & 0x0000FF00) | (pix & 0x00FF0000)>>16 | (pix & 0x000000FF)<<16;
            }
            png_write_row(png_ptr, (png_bytep)linebuffer);
            
        }
        png_write_end(png_ptr, info_ptr);
        png_destroy_write_struct(&png_ptr, &info_ptr);
        fflush(fp);
        fsync(fileno(fp));
        fclose(fp);
    }
}


//
//    Screenshot (640x480x32bpp only, rotate180, png)
//
void screenshot_onion(void) {
    char        screenshotname[512]="/mnt/SDCARD/.tmp_update/screenshotGame.png";
    uint32_t    *buffer;
    uint32_t    *src;
    uint32_t    linebuffer[640], x, y, pix;
    FILE        *fp;
    png_structp    png_ptr;
    png_infop    info_ptr;

    ioctl(fb_fd, FBIOGET_VSCREENINFO, &vinfo);
    buffer = fb_addr + 640*vinfo.yoffset;

    if ((fp = fopen(screenshotname, "wb"))) {
        png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, 0, 0, 0);
        info_ptr = png_create_info_struct(png_ptr);
        png_init_io(png_ptr, fp);
        png_set_IHDR(png_ptr, info_ptr, 640, 480, 8, PNG_COLOR_TYPE_RGBA,
            PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);
        png_write_info(png_ptr, info_ptr);
        src = buffer + 640*480;
        for (y=0; y<480; y++) {
            for (x=0; x<640; x++){
                pix = *--src;
                linebuffer[x] = 0xFF000000 | (pix & 0x0000FF00) | (pix & 0x00FF0000)>>16 | (pix & 0x000000FF)<<16;
            }
            png_write_row(png_ptr, (png_bytep)linebuffer);
            
        }
        png_write_end(png_ptr, info_ptr);
        png_destroy_write_struct(&png_ptr, &info_ptr);
        fflush(fp);
        fsync(fileno(fp));
        fclose(fp);
    }
}

//
//    set CPU governor
//        mode: 0: save 1: restore
//
void setCPUsave(uint32_t mode) {
    static uint32_t minfreq;
    static char gov[16];
    char fn_min_freq[128];
    char fn_governor[128];
    const char powersave[] = "powersave";
    FILE* fp;
    concat(fn_min_freq, CPU_DIR, "scaling_min_freq");
    concat(fn_governor, CPU_DIR, "scaling_governor");

    if (!mode) {
        /* save min_freq */
        fp = fopen(fn_min_freq, "r");
        if (fp) { fscanf(fp, "%u", &minfreq); fclose(fp); }
        /* set min_freq to lowest */
        fp = fopen(fn_min_freq, "w");
        if (fp) { fprintf(fp, "%u", 0); fclose(fp); }
        /* save governor */
        fp = fopen(fn_governor, "r");
        if (fp) { fscanf(fp, "%15s", gov); fclose(fp); }
        /* set governor to powersave */
        fp = fopen(fn_governor, "w");
        if (fp) { fprintf(fp, "%s", powersave); fclose(fp); }
    } else {
        /* restore min_freq */
        fp = fopen(fn_min_freq, "w");
        if (fp) { fprintf(fp, "%u", minfreq); fclose(fp); }
        /* restore governor */
        fp = fopen(fn_governor, "w");
        if (fp) { fprintf(fp, "%s", gov); fclose(fp); }
    }
}

//
//    Quit
//
void quit(int exitcode) {
    battery_hideWarning();
    display_free();
    if (input_fd > 0) close(input_fd);
    battery_free();
    read_clock();
    write_clock_rtc();
    write_clockfile();
    exit(exitcode);
}

//
//    Shutdown
//
void shutdown(void) {
    system_shutdown();
    terminate_retroarch();
    read_clock();
    write_clockfile();
    sync();
    reboot(RB_AUTOBOOT);
    while(1) pause();
    exit(0);
}

//
//    [onion] suspend/resume PlayActivity timer
// 
void onion_pa_suspend(int mode) {
    if (mode) {
        // The current time is resumed
        chdir("cd /mnt/SDCARD/.tmp_update/");
        system("./loadTime.sh; sync");
    }
    else {
        // The current time is saved
        chdir("cd /mnt/SDCARD/.tmp_update/");
        system("./saveTime.sh; sync");
    }
}

//
//    Suspend interface
//
void suspend_exec(int timeout) {
    // stop input event for other processes
    while (ioctl(input_fd, EVIOCGRAB, 1) < 0) { usleep(100000); }

    // suspend
    battery_hideWarning();
    onion_pa_suspend(0);
    suspend(0);
    rumble(0);
    int recent_volume = setVolumeRaw(-60,0);
    _setBrightnessRaw(0);
    display_setScreen(0);
    setCPUsave(0);

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
                    setCPUsave(1);
                    display_setScreen(1);
                    screenshot();
                    // display_setScreen(0);
                    // setCPUsave(0);
                    break;   //  avoid bad screen state after the screen shot
                }
            }
        }
        else if ((!ready)&&(battery_readADC() != 100)) {
            // shutdown
            setCPUsave(1); resume(); usleep(100000); shutdown();
        }
    }

    // resume
    setCPUsave(1);
    if (killexit) { resume(); usleep(100000); suspend(2); usleep(400000); }
    display_setScreen(1);
    display_setBrightness(settings.brightness);
    setVolumeRaw(recent_volume, 0);
    if (!killexit) {
        resume();
        onion_pa_suspend(1);
    }
    battery_updateADC(true);

    // restart input event for other processes
    while (ioctl(input_fd, EVIOCGRAB, 0) < 0) { usleep(100000); }
}

//
//    [onion] Check retroarch running & savestate_auto_save in retroarch.cfg is true
//
int check_autosave(void) {
    int    f, ret = 0;
    pid_t    ra_pid;
    char    key[256] = {0};
    char    value[256] = {0};
    char    *keyptr, *valptr, *strptr;
    char    cfgpath[64];
    const char cfg_ext[] = ".cfg";

    char ra_name[12] = "retroarch";
    if ( (ra_pid = searchpid(ra_name)) ) {
        strptr = cfgpath + sprintf(cfgpath, "/proc/%d/cwd/", ra_pid);
        // standard retroarch ( ./.retroarch/retroarch.cfg )
        sprintf(strptr, ".retroarch/%s%s", ra_name, cfg_ext);
        if (access(cfgpath, R_OK)) {
            // stock retroarch ( ./retroarch.cfg )
            sprintf(strptr, "%s%s", ra_name, cfg_ext);
            if (access(cfgpath, R_OK)) return 0;
        }
    } else {
        strcpy(ra_name, "ra32");
        if ( (ra_pid = searchpid(ra_name)) ) {
            // stock ra32.ss ( ./ra32.cfg )
            sprintf(cfgpath, "/proc/%d/cwd/%s%s", ra_pid, ra_name, cfg_ext);
            if (access(cfgpath, R_OK)) return 0;
        } else return 0;
    }

    FILE* fp = fopen(cfgpath, "r");
    if (!fp) return 0;
    while ((f = fscanf(fp, "%255[^=]=%255[^\n]\n", key, value)) != EOF) {
        if (!f) { if (fscanf(fp, "%*[^\n]\n") == EOF) break; else continue; }
        if ((keyptr = trimstr(key, 0))) {
            if (!strcmp(keyptr, "savestate_auto_save")) {
                if (valptr = trimstr(value, 1) && !strcmp(valptr, "\"true\""))
                    ret = 1;
                break;
            }
        }
    }
    fclose(fp);
    return ret;
}

//
//    [onion] Write percBat to /tmp/percBat
//
void write_percBat(void) {
    FILE*    fp;
    int percBat = battery_readPercentage();

    if (settings.low_battery_shutdown && percBat <= 4) {
        if (check_autosave())
            terminate_retroarch();
    }

    if ((fp = fopen("/tmp/percBat", "w"))) {
        fprintf(fp, "%d", percBat);
        fclose(fp);
    }
}



//
//    [onion] deepsleep if MainUI/gameSwitcher/retroarch is running
//
void deepsleep(void) {

    pid_t pid;
    if ((pid = searchpid("MainUI"))) {
        short_pulse();
        system_shutdown(); kill(pid, SIGKILL); 
        
    }
    else if ((pid = searchpid("gameSwitcher"))) {
        short_pulse();
        system_shutdown();
        kill(pid, SIGKILL);
    }
    else if ((pid = searchpid("retroarch"))) {
         if (check_autosave()){
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
    uint32_t comboKey = 0;
    
    struct timespec recent;
    clock_gettime(CLOCK_MONOTONIC_COARSE, &recent);
    struct timespec hibernate_start = recent;
    int hibernate_time;
    int elapsed_sec = 0;

    // The next calls are in 15s
    // Update ADC Value
    battery_updateADC(false);
    write_percBat();
    // Update recent time
    clock_gettime(CLOCK_MONOTONIC_COARSE, &recent);
    

    while(1) {
        if (poll(fds, 1, (CHECK_SEC - elapsed_sec) * 1000) > 0) {

            settings_load();
            
            read(input_fd, &ev, sizeof(ev));
            val = ev.value;
            
            if (( ev.type != EV_KEY ) || ( val > REPEAT )) continue;
            
            if (val!=REPEAT){

                if (ev.code == HW_BTN_MENU){
                    b_BTN_Menu_Pressed = val;
                }

                else {
                    b_BTN_Not_Menu_Pressed = val;
                }
                    
    
                if ((b_BTN_Menu_Pressed == 1)&&(b_BTN_Not_Menu_Pressed == 1)){
                    comboKey = 1;
                }
                
                if ((b_BTN_Menu_Pressed == 1)&&(ev.code == HW_BTN_POWER)){
                    // screenshot
                    
                }
                
                if (ev.code == HW_BTN_MENU && val == 0) {
                    if (comboKey == 0 && check_autosave()) {
                        if (settings.launcher && !settings.menu_inverted) {
                            temp_flag_set(".trimUIMenu", true);
                            screenshot_onion();
                            terminate_retroarch();
                        }
                        else {
                            temp_flag_set(".trimUIMenu", false);
                            terminate_retroarch();
                        }
                    }
                    comboKey = 0;                    
                }    
                
            }
            
            switch (ev.code) {
            case HW_BTN_POWER:
                if ( val == REPEAT ) {
                    repeat_power++;
                    if (repeat_power == 7)
                        deepsleep(); // 0.5sec deepsleep
                     
                    else if (repeat_power == REPEAT_SEC(5)) {
                        short_pulse();
                        remove("/mnt/SDCARD/.tmp_update/cmd_to_run.sh");
                        suspend(2); // 5sec kill processes
                        
                    }
                    else if (repeat_power >= REPEAT_SEC(10)) {
                        short_pulse();
                        shutdown(); // 10sec force shutdown
                    }
                    break;
                }
                if (val == RELEASED && repeat_power < 7 && !file_exists("/tmp/stay_awake")) {
                    // suspend
                    suspend_exec(settings.sleep_timer == 0 ? -1 : (settings.sleep_timer + SHUTDOWN_MIN) * 60000);
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
                                display_setBrightness(--settings.brightness);
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
                            display_setBrightness(++settings.brightness);
                            settings_save();
                        }
                        break;
                    default:
                        break;
                    }
                } 
                break;
            case HW_BTN_MENU:
                if (val == REPEAT) {
                    repeat_menu++;
                    if (repeat_menu == REPEAT_SEC(1) && !button_flag) {    // long press on menu
                        if (!settings.menu_inverted) {
                            if (check_autosave()) {
                                remove("/tmp/.trimUIMenu");
                                short_pulse();
                                terminate_retroarch();
                            }
                        }
                        else if (settings.launcher) {
                            if (check_autosave()) {
                                short_pulse();
                                close(creat("/tmp/.trimUIMenu", 777));
                                screenshot_onion();
                                terminate_retroarch();    
                            }
                        }
                        else {
                            comboKey = 1;  
                        }
                        repeat_menu = 0;
                    }
                }
                break;
            default:
                break;
            }
            
            clock_gettime(CLOCK_MONOTONIC_COARSE, &hibernate_start);
            elapsed_sec = hibernate_start.tv_sec - recent.tv_sec;
            if ( elapsed_sec < CHECK_SEC ) continue;
        }

        // Comes here every CHECK_SEC(def:15) seconds interval

        // Check Hibernate
        if (temp_flag_get("battery_charging"))
            hibernate_time = 1;
        else
            hibernate_time = settings.sleep_timer;

        if (hibernate_time && !temp_flag_get("stay_awake")) {
            clock_gettime(CLOCK_MONOTONIC_COARSE, &recent);
            if (recent.tv_sec - hibernate_start.tv_sec >= hibernate_time * 60) {
                suspend_exec(SHUTDOWN_MIN * 60000);
                clock_gettime(CLOCK_MONOTONIC_COARSE, &hibernate_start);
            }
        }

        // Update ADC Value
        battery_updateADC(false);
        write_percBat();
        // Update recent time
        clock_gettime(CLOCK_MONOTONIC_COARSE, &recent);
        elapsed_sec = 0;
    }
}
