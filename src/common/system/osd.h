#ifndef SYSTEM_OSD_H__
#define SYSTEM_OSD_H__

#include <pthread.h>

#include "utils/config.h"
#include "utils/log.h"
#include "utils/msleep.h"

#include "./clock.h"
#include "./display.h"

#define CHR_WIDTH (3 * 4 + 4)
#define CHR_HEIGHT (5 * 4)

#define OSD_COLOR_WHITE 0x00FFFFFF
#define OSD_COLOR_RED 0x00F80355
#define OSD_COLOR_GREEN 0x001CD577
#define OSD_COLOR_CYAN 0x0000ffD7
#define OSD_COLOR_YELLOW 0x00DCFF62

#define OSD_BRIGHTNESS_COLOR OSD_COLOR_WHITE
#define OSD_VOLUME_COLOR OSD_COLOR_GREEN
#define OSD_MUTE_ON_COLOR OSD_COLOR_RED

static bool osd_thread_active = false;
static pthread_t osd_pt;

//
//	Print digit
//
void print_digit(uint8_t num, uint32_t x, uint32_t color)
{
    const uint16_t pix[13] = {0b000000000000000,  // space
                              0b000001010100000,  // /
                              0b111101101101111,  // 0
                              0b001001001001001,  // 1
                              0b111001111100111,  // 2
                              0b111001111001111,  // 3
                              0b101101111001001,  // 4
                              0b111100111001111,  // 5
                              0b111100111101111,  // 6
                              0b111001001001001,  // 7
                              0b111101111101111,  // 8
                              0b111101111001111,  // 9
                              0b000010000010000}; // :
    uint32_t c32, i, y;
    uint16_t number_pixel, c16;
    uint8_t *ofs;
    uint16_t *ofs16;
    uint32_t *ofs32;
    uint32_t s16 = stride / 2;
    uint32_t s32 = stride / 4;

    if (num == ' ')
        num = 0;
    else
        num -= 0x2e;
    if ((num > 12) || (x > 18))
        return;
    number_pixel = pix[num];
    ofs = fbofs + ((18 - x) * CHR_WIDTH * bpp);

    printf_debug("printing %d\n", num);

    for (y = 5; y > 0; y--, ofs += stride * 4) {
        if (bpp == 4) {
            ofs32 = (uint32_t *)ofs;
            for (i = 3; i > 0; i--, number_pixel >>= 1, ofs32 += 4) {
                c32 = (number_pixel & 1) ? color : 0;
                ofs32[0] = c32;
                ofs32[1] = c32;
                ofs32[2] = c32;
                ofs32[3] = c32;
                ofs32[s32 + 0] = c32;
                ofs32[s32 + 1] = c32;
                ofs32[s32 + 2] = c32;
                ofs32[s32 + 3] = c32;
                ofs32[s32 * 2 + 0] = c32;
                ofs32[s32 * 2 + 1] = c32;
                ofs32[s32 * 2 + 2] = c32;
                ofs32[s32 * 2 + 3] = c32;
                ofs32[s32 * 3 + 0] = c32;
                ofs32[s32 * 3 + 1] = c32;
                ofs32[s32 * 3 + 2] = c32;
                ofs32[s32 * 3 + 3] = c32;
            }
        }
        else {
            ofs16 = (uint16_t *)ofs;
            for (i = 3; i > 0; i--, number_pixel >>= 1, ofs16 += 4) {
                c16 = (number_pixel & 1) ? (color & 0xffff) : 0;
                ofs16[0] = c16;
                ofs16[1] = c16;
                ofs16[2] = c16;
                ofs16[3] = c16;
                ofs16[s16 + 0] = c16;
                ofs16[s16 + 1] = c16;
                ofs16[s16 + 2] = c16;
                ofs16[s16 + 3] = c16;
                ofs16[s16 * 2 + 0] = c16;
                ofs16[s16 * 2 + 1] = c16;
                ofs16[s16 * 2 + 2] = c16;
                ofs16[s16 * 2 + 3] = c16;
                ofs16[s16 * 3 + 0] = c16;
                ofs16[s16 * 3 + 1] = c16;
                ofs16[s16 * 3 + 2] = c16;
                ofs16[s16 * 3 + 3] = c16;
            }
        }
    }
}

//
//	Print number value
//
//  Color: white=0x00FFFFFF, red=0x00FF0000
//
void print_value(uint32_t value, uint32_t color)
{
    char str[20];
    sprintf(str, "%d", value);

    printf_debug("osd: %s\n", str);

    for (uint32_t x = 0; x < 19; x++) {
        print_digit(str[x], x, color);
    }
}

static int meterWidth = 4;
static bool osd_bar_activated = false;
static int _bar_timer = 0;
static int _bar_value = 0;
static int _bar_max = 0;
static uint32_t _bar_color = 0x00FFFFFF;
#ifdef PLATFORM_MIYOOMINI
static uint32_t *_bar_savebuf;
#endif

void _print_bar(void)
{
#ifdef PLATFORM_MIYOOMINI
    uint32_t *ofs = fb_addr;
    uint32_t i, j, curr, percentage = _bar_max > 0 ? _bar_value * RENDER_HEIGHT / _bar_max : 0;

    ofs += RENDER_WIDTH - meterWidth;
    for (i = 0; i < RENDER_HEIGHT * 3; i++, ofs += RENDER_WIDTH) {
        curr = (i % RENDER_HEIGHT) < percentage ? _bar_color : 0;
        for (j = 0; j < meterWidth; j++)
            ofs[j] = curr;
    }
#endif
}

void _bar_restoreBufferBehind(void)
{
#ifdef PLATFORM_MIYOOMINI
    _bar_value = 0;
    _bar_max = 0;
    _bar_color = 0;
    _print_bar();
    if (_bar_savebuf) {
        uint32_t i, j, *ofs = fb_addr, *ofss = _bar_savebuf;
        ofs += RENDER_WIDTH - meterWidth;
        ofss += RENDER_WIDTH - meterWidth;
        for (i = 0; i < RENDER_HEIGHT; i++, ofs += RENDER_WIDTH, ofss += RENDER_WIDTH) {
            for (j = 0; j < meterWidth; j++)
                ofs[j] = ofss[j];
        }
        free(_bar_savebuf);
        _bar_savebuf = NULL;
    }
#endif
}

void _bar_saveBufferBehind(void)
{
#ifdef PLATFORM_MIYOOMINI
    // Save display area and clear
    if ((_bar_savebuf = (uint32_t *)malloc(RENDER_WIDTH * RENDER_HEIGHT *
                                           sizeof(uint32_t)))) {
        uint32_t i, j, *ofs = fb_addr, *ofss = _bar_savebuf;
        ofs += RENDER_WIDTH - meterWidth;
        ofss += RENDER_WIDTH - meterWidth;
        for (i = 0; i < RENDER_HEIGHT; i++, ofs += RENDER_WIDTH, ofss += RENDER_WIDTH) {
            for (j = 0; j < meterWidth; j++)
                ofss[j] = ofs[j];
        }
    }
#endif
}

//
//    OSD draw thread
//
static void *_osd_thread(void *_)
{
    while (getMilliseconds() - _bar_timer < 2000) {
        _print_bar();
        msleep(1);
    }
    _bar_restoreBufferBehind();
    osd_thread_active = false;
    return 0;
}

/**
 * @brief Show OSD percentage bar
 * 
 * @param value 
 * @param value_max 
 * @param color 
 */
void osd_showBar(int value, int value_max, uint32_t color)
{
    _bar_timer = getMilliseconds();
    _bar_value = value;
    _bar_max = value_max;
    _bar_color = color;
    osd_bar_activated = true;

    config_get("display/meterWidth", CONFIG_INT, &meterWidth);

    if (osd_thread_active)
        return;

    _bar_saveBufferBehind();
    pthread_create(&osd_pt, NULL, _osd_thread, _print_bar);
    osd_thread_active = true;
}

void osd_hideBar(void)
{
    osd_bar_activated = false;
    if (!osd_thread_active)
        return;
    pthread_cancel(osd_pt);
    pthread_join(osd_pt, NULL);
    _bar_restoreBufferBehind();
    osd_thread_active = false;
}

void osd_showVolumeBar(int volume, bool mute)
{
    osd_showBar(volume, 20, mute ? OSD_MUTE_ON_COLOR : OSD_VOLUME_COLOR);
}

void osd_showBrightnessBar(int brightness)
{
    osd_showBar(brightness, 10, OSD_BRIGHTNESS_COLOR);
}

#endif // SYSTEM_OSD_H__
