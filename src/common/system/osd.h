#ifndef SYSTEM_OSD_H__
#define SYSTEM_OSD_H__

#include <SDL/SDL.h>
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

typedef struct {
    SDL_Surface *surface;
    int destX;
    int destY;
    int duration_ms;
    bool rotate;
    struct fb_var_screeninfo vinfo;
    struct fb_fix_screeninfo finfo;
    int fb_fd;
    unsigned char *fbmem;
    long screensize;
    int numBuffers;
    int fbWidth;
    int fbHeight;
    bool abort_requested;
    pthread_t thread_id;
} overlay_thread_data;

static overlay_thread_data *g_overlay_data = NULL;
static pthread_mutex_t g_overlay_data_lock = PTHREAD_MUTEX_INITIALIZER;

/**
 * @brief Cancels the overlay and cleans up resources.
 *
 * Called by overlay_surface() if there is an existing overlay, or manually by
 * the user to cancel an overlay. (future use - gameswitcher :))
 */
void cancel_overlay()
{
    // Mark abort
    g_overlay_data->abort_requested = true;

    // Unlock and wait for thread to exit
    pthread_mutex_unlock(&g_overlay_data_lock);
    pthread_join(g_overlay_data->thread_id, NULL);

    // Re-lock after join to safely free
    pthread_mutex_lock(&g_overlay_data_lock);

    if (g_overlay_data->fbmem && g_overlay_data->fbmem != MAP_FAILED) {
        munmap(g_overlay_data->fbmem, g_overlay_data->screensize);
    }
    if (g_overlay_data->surface) {
        SDL_FreeSurface(g_overlay_data->surface);
        g_overlay_data->surface = NULL;
    }
    free(g_overlay_data);
    g_overlay_data = NULL;
    pthread_mutex_unlock(&g_overlay_data_lock);
}

static void *_overlay_draw_thread(void *arg)
{
    overlay_thread_data *data = (overlay_thread_data *)arg;

    struct timespec start, now;
    clock_gettime(CLOCK_MONOTONIC, &start);
    long elapsed_ms = 0;
    int draw_count = 0;

    unsigned int *overlayData = (unsigned int *)data->surface->pixels;

    // Continuously blit to each buffer until abort
    while (true) {
        clock_gettime(CLOCK_MONOTONIC, &now);
        elapsed_ms =
            (long)(now.tv_sec - start.tv_sec) * 1000L +
            (long)(now.tv_nsec - start.tv_nsec) / 1000000L;

        // Timeout
        if (data->duration_ms > 0 && elapsed_ms >= data->duration_ms)
            break;

        // Abort
        pthread_mutex_lock(&g_overlay_data_lock);
        bool local_abort = data->abort_requested;
        pthread_mutex_unlock(&g_overlay_data_lock);
        if (local_abort)
            break;

        // Draw to each buffer
        for (int b = 0; b < data->numBuffers; b++) {
            draw_count++;
            int bufferTop = b * data->fbHeight;

            // Iterate over surface and draw in reverse
            for (int oy = 0; oy < data->surface->h; oy++) {
                for (int ox = 0; ox < data->surface->w; ox++) {
                    unsigned int color = overlayData[oy * data->surface->w + ox];

                    // Where we'd draw if no flipping
                    int logicalX = data->destX + ox;
                    int logicalY = bufferTop + data->destY + oy;

                    // localY in the buffer
                    int localY = logicalY - bufferTop;
                    if (localY < 0 || localY >= data->fbHeight)
                        continue;

                    // Flip Y (vertical)
                    int flippedLocalY = (data->fbHeight - 1) - localY;
                    int flippedY = bufferTop + flippedLocalY;

                    // Flip X (horizontal)
                    int flippedX = (data->fbWidth - 1) - logicalX;

                    // Range check
                    if (flippedX < 0 || flippedX >= data->fbWidth)
                        continue;
                    if (flippedY < 0 || flippedY >= (bufferTop + data->fbHeight))
                        continue;

                    // Calculate offset into fb and write
                    long offset = (long)flippedY * finfo.line_length +
                                  (long)flippedX * 4;
                    *((unsigned int *)(data->fbmem + offset)) = color;
                }
            }
        }

        // TODO: sleep or not?
        // usleep(4000);
    }

    printf_debug("Draw count: %d\n", draw_count);
    printf_debug("Draw speed: %f\n", (float)draw_count / (float)elapsed_ms * 1000.0f);

    // Cleanup
    if (data->surface) {
        SDL_FreeSurface(data->surface);
        data->surface = NULL;
    }

    pthread_exit(NULL);
}

/**
 * @brief Overlays an SDL_Surface onto the framebuffer at a specified position and duration.
 *
 * Flickering is minimized by drawing to all buffers in the framebuffer.
 * Should work regardless of the count of buffers (double, triple, etc. buffering).
 * TODO: How does Alpha work if at all? Performance hit on bigger surfaces?
 * 
 * @param surface The SDL_Surface to overlay. The surface will be freed by this function.
 * @param destX The X coordinate on the screen where the overlay should be placed.
 * @param destY The Y coordinate on the screen where the overlay should be placed.
 * @param duration_ms The duration in milliseconds for which the overlay should be displayed. 0 for indefinite.
 * @param rotate Should the overlay be rotated 180 degrees? 
 * @return int Returns 0 on success, -1 on failure.
 */
int overlay_surface(SDL_Surface *surface, int destX, int destY, int duration_ms, bool rotate)
{
    display_init();

    pthread_mutex_lock(&g_overlay_data_lock);

    // If there's an existing thread, abort it
    if (g_overlay_data != NULL) {
        cancel_overlay();
    }

    // Query FB info
    struct fb_var_screeninfo vinfo;
    if (ioctl(fb_fd, FBIOGET_VSCREENINFO, &vinfo) == -1) {
        perror("Error reading variable screen info");
        pthread_mutex_unlock(&g_overlay_data_lock);
        return -1;
    }

    if (vinfo.bits_per_pixel != 32) {
        perror("Only 32bpp is supported for now\n");
        pthread_mutex_unlock(&g_overlay_data_lock);
        return -1;
    }

    // Data for thread
    overlay_thread_data *data = (overlay_thread_data *)calloc(1, sizeof(overlay_thread_data));
    if (!data) {
        perror("Failed to allocate overlay_thread_data\n");
        pthread_mutex_unlock(&g_overlay_data_lock);
        return -1;
    }
    data->surface = surface;
    data->destX = destX;
    data->destY = destY;
    data->duration_ms = duration_ms;
    data->rotate = rotate;
    data->fb_fd = fb_fd;
    data->vinfo = vinfo;
    data->finfo = finfo;
    data->fbWidth = vinfo.xres;
    data->fbHeight = vinfo.yres;
    data->numBuffers = vinfo.yres_virtual / vinfo.yres;

    printf_debug("Detected Buffers: %d\n", data->numBuffers);

    // Need to map every time in case of buffer changes
    data->screensize = (long)finfo.line_length * (long)vinfo.yres_virtual;
    data->fbmem = mmap(NULL, data->screensize,
                       PROT_READ | PROT_WRITE,
                       MAP_SHARED,
                       fb_fd,
                       0);
    if (data->fbmem == MAP_FAILED) {
        perror("Failed to mmap framebuffer");
        free(data);
        pthread_mutex_unlock(&g_overlay_data_lock);
        return -1;
    }

    // Create the thread
    int rc = pthread_create(&data->thread_id, NULL, _overlay_draw_thread, data);
    if (rc != 0) {
        fprintf(stderr, "Failed to create overlay drawing thread\n");
        munmap(data->fbmem, data->screensize);
        free(data);
        pthread_mutex_unlock(&g_overlay_data_lock);
        return -1;
    }
    g_overlay_data = data;
    pthread_mutex_unlock(&g_overlay_data_lock);

    return 0;
}

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
        usleep(100);
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
