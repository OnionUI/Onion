#ifndef DISPLAY_H__
#define DISPLAY_H__

#include <linux/fb.h>
#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <sys/mman.h>

#include "mi_disp.h"
#include "mi_sys.h"
#include "system.h"
#include "utils/file.h"
#include "utils/log.h"

#define DISPLAY_WIDTH 640
#define DISPLAY_HEIGHT 480

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

void display_init(void)
{
    // Open and mmap FB
    fb_fd = open("/dev/fb0", O_RDWR);
    ioctl(fb_fd, FBIOGET_FSCREENINFO, &finfo);
    fb_addr = (uint32_t *)mmap(0, finfo.smem_len, PROT_READ | PROT_WRITE,
                               MAP_SHARED, fb_fd, 0);
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
        ofs[i + y * 640] = color;        // Top border
        ofs[i + (y + 14) * 640] = color; // Bottom border
    }
    for (j = y; j < y + 15; j++) {
        ofs[x + j * 640] = color;      // Left border
        ofs[x + 29 + j * 640] = color; // Right border
    }

    // Draw battery charge level
    int levelWidth = (level * 26) / 100;
    for (i = x + 3 + 26 - levelWidth; i < x + 1 + 26; i++) {
        for (j = y + 3; j < y + 12; j++) {
            ofs[i + j * 640] = fillColor;
        }
    }

    // Draw battery head wireframe
    for (i = x - 4; i < x; i++) {
        for (j = y + 2; j < y + 13; j++) {
            ofs[i + j * 640] = color;
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

void display_spawnMIDISP(MI_U16 width, MI_U16 height, MI_U32 luma, MI_U32 contrast, MI_U32 hue, MI_U32 saturation)
{
    /* 
        if we don't have a valid mi_disp0 endpoint the screen will look washed out. 
        we also can't push changes into it (luma/hue/sat/contrast, sharpness, etc)
        
        the below allows us to spawn an ephemeral mi_disp0 it will set the screen up correctly, but unfortunately when whatever process that called it ends, so will mi_disp0
        
        it's the circle of life.
        
        some maths ops happen, these were pulled from the l binary in ghidra
        
        this function will ONLY run it a mi_disp0 doesn't already exist. If it does you neeed to push CSC changes in via shell.
    */

    FILE *file = fopen("/proc/mi_modules/mi_disp/mi_disp0", "r");
    if (file == NULL) {
        MI_DISP_DEV DispDev = 0;
        MI_DISP_LAYER DispLayer = 0;
        MI_DISP_INPUTPORT DispInport = 0;
        MI_DISP_PubAttr_t stPubAttr;
        MI_DISP_VideoLayerAttr_t stLayerAttr;
        MI_DISP_InputPortAttr_t stInputPortAttr;
        MI_DISP_LcdParam_t lcdParam;
        MI_S32 s32Ret = MI_SUCCESS;

        MI_U32 lumaProcessed = luma + 17 * 2;
        MI_U32 satProcessed = saturation * 5;
        MI_U32 contProcessed = contrast + 40;
        MI_U32 hueProcessed = hue * 5;

        MI_SYS_Init();
        MI_DISP_Enable(DispDev);

        memset(&stPubAttr, 0, sizeof(MI_DISP_PubAttr_t));
        memset(&stLayerAttr, 0, sizeof(MI_DISP_VideoLayerAttr_t));
        memset(&stInputPortAttr, 0, sizeof(MI_DISP_InputPortAttr_t));

        stPubAttr.eIntfSync = E_MI_DISP_OUTPUT_USER;
        stPubAttr.eIntfType = E_MI_DISP_INTF_LCD;
        stPubAttr.u32BgColor = 0x800080;
        s32Ret = MI_DISP_SetPubAttr(DispDev, &stPubAttr);
        printf("MI_DISP_SetPubAttr returned %d\n", s32Ret);

        lcdParam.stCsc.eCscMatrix = E_MI_DISP_CSC_MATRIX_BT601_TO_RGB_PC;
        lcdParam.stCsc.u32Luma = lumaProcessed;
        lcdParam.stCsc.u32Contrast = contProcessed;
        lcdParam.stCsc.u32Hue = hueProcessed;
        lcdParam.stCsc.u32Saturation = satProcessed;
        s32Ret = MI_DISP_SetLcdParam(DispDev, &lcdParam);
        printf("MI_DISP_SetLcdParam returned %d\n", s32Ret);

        stLayerAttr.stVidLayerSize.u16Width = width;
        stLayerAttr.stVidLayerSize.u16Height = height;
        stLayerAttr.stVidLayerDispWin.u16X = 0;
        stLayerAttr.stVidLayerDispWin.u16Y = 0;
        stLayerAttr.stVidLayerDispWin.u16Width = width;
        stLayerAttr.stVidLayerDispWin.u16Height = height;
        s32Ret = MI_DISP_SetVideoLayerAttr(DispLayer, &stLayerAttr);
        printf("MI_DISP_SetVideoLayerAttr returned %d\n", s32Ret);

        s32Ret = MI_DISP_EnableVideoLayer(DispLayer);
        printf("MI_DISP_EnableVideoLayer returned %d\n", s32Ret);

        stInputPortAttr.u16SrcWidth = width;
        stInputPortAttr.u16SrcHeight = height;
        stInputPortAttr.stDispWin.u16X = 0;
        stInputPortAttr.stDispWin.u16Y = 0;
        stInputPortAttr.stDispWin.u16Width = width;
        stInputPortAttr.stDispWin.u16Height = height;
        s32Ret = MI_DISP_SetInputPortAttr(DispLayer, DispInport, &stInputPortAttr);
        printf("MI_DISP_SetInputPortAttr returned %d\n", s32Ret);

        s32Ret = MI_DISP_EnableInputPort(DispLayer, DispInport);
        printf("MI_DISP_EnableInputPort returned %d\n", s32Ret);
    }
    else {
        fclose(file);
    }
}

#endif // DISPLAY_H__
