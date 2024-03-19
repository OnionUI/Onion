#ifndef DISPLAY_H__
#define DISPLAY_H__

#include <linux/fb.h>
#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <sys/mman.h>

#include "system.h"
#include "utils/file.h"
#include "utils/log.h"

#define display_on() display_setScreen(true)
#define display_off() display_setScreen(false)

static uint32_t *fb_addr;
static int fb_fd;
static uint8_t *fbofs;
static struct fb_fix_screeninfo finfo;
static struct fb_var_screeninfo vinfo;
static uint32_t stride, bpp;
static uint8_t *savebuf;
static bool display_enabled = true;
static int DISPLAY_WIDTH = 640; // physical screen resolution
static int DISPLAY_HEIGHT = 480;
int RENDER_WIDTH = 640; // render resolution
int RENDER_HEIGHT = 480;
struct timeval start_time, end_time;

//
//    Get render resolution
//
void display_getRenderResolution()
{
    print_debug("Getting render resolution\n");
    if (fb_fd < 0)
        fb_fd = open("/dev/fb0", O_RDWR);
    ioctl(fb_fd, FBIOGET_VSCREENINFO, &vinfo);
    RENDER_WIDTH = vinfo.xres;
    RENDER_HEIGHT = vinfo.yres;
}

//
//    Get physical screen resolution
//
void display_getResolution()
{
    FILE *file = fopen("/tmp/screen_resolution", "r");
    if (file == NULL) {
        printf("Failed to open screen resolution file\n");
        return;
    }
    if (fscanf(file, "%dx%d", &DISPLAY_WIDTH, &DISPLAY_HEIGHT) != 2)
        printf("Failed to get screen resolution\n");
    fclose(file);
}

void display_init(void)
{
    // Open and mmap FB
    fb_fd = open("/dev/fb0", O_RDWR);
    ioctl(fb_fd, FBIOGET_FSCREENINFO, &finfo);
    fb_addr = (uint32_t *)mmap(0, finfo.smem_len, PROT_READ | PROT_WRITE,
                               MAP_SHARED, fb_fd, 0);
    display_getResolution();
    display_getRenderResolution();
}

//
//    Save/Clear Display area
//
void display_save(void)
{
    stride = finfo.line_length;
    ioctl(fb_fd, FBIOGET_VSCREENINFO, &vinfo);
    bpp = vinfo.bits_per_pixel / 8; // byte per pixel
    fbofs = (uint8_t *)fb_addr + (vinfo.yoffset * stride);

    // Save display area and clear
    if ((savebuf = (uint8_t *)malloc(DISPLAY_WIDTH * bpp * DISPLAY_HEIGHT))) {
        uint32_t i, ofss, ofsd;
        ofss = ofsd = 0;
        for (i = DISPLAY_HEIGHT; i > 0;
             i--, ofss += stride, ofsd += DISPLAY_WIDTH * bpp) {
            memcpy(savebuf + ofsd, fbofs + ofss, DISPLAY_WIDTH * bpp);
            memset(fbofs + ofss, 0, DISPLAY_WIDTH * bpp);
        }
    }
}

//
//    Restore Display area
//
void display_restore(void)
{
    // Restore display area
    if (savebuf) {
        uint32_t i, ofss, ofsd;
        ofss = ofsd = 0;
        for (i = DISPLAY_HEIGHT; i > 0;
             i--, ofsd += stride, ofss += DISPLAY_WIDTH * bpp) {
            memcpy(fbofs + ofsd, savebuf + ofss, DISPLAY_WIDTH * bpp);
        }
        free(savebuf);
        savebuf = NULL;
    }
}

void display_reset(void)
{
    ioctl(fb_fd, FBIOGET_VSCREENINFO, &vinfo);
    vinfo.yoffset = 0;
    memset(fb_addr, 0, finfo.smem_len);
    ioctl(fb_fd, FBIOPUT_VSCREENINFO, &vinfo);
}

//
//    Screen On/Off
//
void display_setScreen(bool enabled)
{
    // export gpio4, direction: out
    file_write(GPIO_DIR1 "export", "4", 1);
    file_write(GPIO_DIR2 "gpio4/direction", "out", 3);

    // screen on/off
    file_write(GPIO_DIR2 "gpio4/value", enabled ? "1" : "0", 1);

    // unexport gpio4
    file_write(GPIO_DIR1 "unexport", "4", 1);

    if (enabled) {
        // re-enable brightness control
        file_write(PWM_DIR "export", "0", 1);
        file_write(PWM_DIR "pwm0/enable", "0", 1);
        file_write(PWM_DIR "pwm0/enable", "1", 1);
        display_restore();
    }
    else {
        display_save();
    }
    display_enabled = enabled;
}

void display_toggle(void) { display_setScreen(!display_enabled); }

uint32_t display_getBrightnessRaw()
{
    uint32_t duty_cycle = 0;
    FILE *fp;

    if (exists(PWM_DIR "pwm0/duty_cycle")) {
        file_get(fp, PWM_DIR "pwm0/duty_cycle", "%u", &duty_cycle);
    }
    return duty_cycle;
}

//
//    Set Brightness (Raw)
//
void display_setBrightnessRaw(uint32_t value)
{
    FILE *fp;
    file_put_sync(fp, PWM_DIR "pwm0/duty_cycle", "%u", value);
    printf_debug("Raw brightness: %d\n", value);
}

// Set display brightness (0 - 10)
void display_setBrightness(uint32_t value)
{
    // Linear curve
    // int value_raw = (value == 0) ? 3 : (value * 10);

    // Exponential curve
    int value_raw = round(3.0 * exp(0.350656 * value));

    display_setBrightnessRaw(value_raw);
}

//
//    Draw frame, fixed 640x480x32bpp for now
//
void display_drawFrame(uint32_t color)
{
    uint32_t *ofs = fb_addr;
    uint32_t i;
    for (i = 0; i < 640; i++) {
        ofs[i] = color;
    }
    ofs += 640 * 479;
    for (i = 0; i < 640 * 2; i++) {
        ofs[i] = color;
    }
    ofs += 640 * 480;
    for (i = 0; i < 640 * 2; i++) {
        ofs[i] = color;
    }
    ofs += 640 * 480;
    for (i = 0; i < 640; i++) {
        ofs[i] = color;
    }
    ofs = fb_addr + 639;
    for (i = 0; i < 480 * 3 - 1; i++, ofs += 640) {
        ofs[0] = color;
        ofs[1] = color;
    }
}

//
//    Draw a battery icon
//
void display_drawBatteryIcon(uint32_t color, int x, int y, int level,
                             uint32_t fillColor)
{
    uint32_t *ofs = fb_addr;
    int i, j;

    // Draw battery body wireframe
    for (i = x; i < x + 30; i++) {
        ofs[i + y * RENDER_WIDTH] = color;        // Top border
        ofs[i + (y + 14) * RENDER_WIDTH] = color; // Bottom border
    }
    for (j = y; j < y + 15; j++) {
        ofs[x + j * RENDER_WIDTH] = color;      // Left border
        ofs[x + 29 + j * RENDER_WIDTH] = color; // Right border
    }

    // Draw battery charge level
    int levelWidth = (level * 26) / 100;
    for (i = x + 3 + 26 - levelWidth; i < x + 1 + 26; i++) {
        for (j = y + 3; j < y + 12; j++) {
            ofs[i + j * RENDER_WIDTH] = fillColor;
        }
    }

    // Draw battery head wireframe
    for (i = x - 4; i < x; i++) {
        for (j = y + 2; j < y + 13; j++) {
            ofs[i + j * RENDER_WIDTH] = color;
        }
    }
}

void display_free(void)
{
    if (savebuf)
        free(savebuf);
    if (fb_addr)
        munmap(fb_addr, finfo.smem_len);
    if (fb_fd > 0)
        close(fb_fd);
}

#endif // DISPLAY_H__
