#ifndef SYSTEM_OSD_H__
#define SYSTEM_OSD_H__

#include <SDL/SDL.h>
#include <pthread.h>

#include "utils/config.h"
#include "utils/log.h"
#include "utils/msleep.h"
#include "utils/timer.h"

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

#ifndef CLOCK_MONOTONIC
#define CLOCK_MONOTONIC 1
#endif

static bool osd_thread_active = false;
static pthread_t osd_pt;

typedef struct {
    SDL_Surface *surface;
    int destX;
    int destY;
    int duration_ms;
    bool rotate;
    bool useMask;
    display_t display;
} overlay_thread_data;

typedef struct {
    bool cancelled;
    pthread_t thread_id;
    pthread_mutex_t lock;
} task_t;

static task_t g_overlay_task = {true, 0, PTHREAD_MUTEX_INITIALIZER};

/**
 * @brief Cancels the overlay and cleans up resources.
 *
 * Called by overlay_surface() if there is an existing overlay, or manually by
 * the user to cancel an overlay. (future use - gameswitcher :))
 */
void cancel_overlay()
{
    if (g_overlay_task.thread_id == 0) {
        print_debug("No overlay to cancel\n");
        return;
    }
    // Mark abort
    pthread_mutex_lock(&g_overlay_task.lock);
    pthread_t thread_to_join = g_overlay_task.thread_id;
    printf_debug("Cancelling overlay thread ID %ld\n", thread_to_join);
    g_overlay_task.cancelled = true;
    pthread_mutex_unlock(&g_overlay_task.lock);

    pthread_join(thread_to_join, NULL);

    printf_debug("Cancelled overlay thread ID %ld\n", thread_to_join);
}

static void free_overlay_data(overlay_thread_data *data)
{
    if (data->surface) {
        SDL_FreeSurface(data->surface);
        data->surface = NULL;
    }
    display_free(&data->display);
    free(data);
}

static void *_overlay_draw_thread(void *arg)
{
    overlay_thread_data *data = (overlay_thread_data *)arg;

    // Backup original fb content
    START_TIMER(framebuffer_backup);
    int numBuffers = data->display.vinfo.yres_virtual / data->display.vinfo.yres;
    unsigned int **originalPixels = malloc(numBuffers * sizeof(unsigned int *));
    if (!originalPixels) {
        return NULL;
    }
    for (int b = 0; b < numBuffers; b++) {
        originalPixels[b] = malloc(data->surface->w * data->surface->h * sizeof(unsigned int));
        if (!originalPixels[b]) {
            for (int i = 0; i < b; i++) {
                free(originalPixels[i]);
            }
            free(originalPixels);
            return NULL;
        }
    }

    rect_t rect = {data->destX, data->destY, data->surface->w, data->surface->h};

    display_readBuffers(&data->display, originalPixels, rect, data->rotate, data->useMask);
    END_TIMER(framebuffer_backup);
    printf_debug("Backup buffer total size: %d KiB\n",
                 (numBuffers * data->surface->w * data->surface->h * sizeof(unsigned int)) / 1024);

    struct timespec start, now;
    clock_gettime(CLOCK_MONOTONIC, &start);
    long elapsed_ms = -1;
    int draw_count = 0;

    // Continuously blit the overlay to each buffer
    while (true) {
        clock_gettime(CLOCK_MONOTONIC, &now);
        elapsed_ms =
            (long)(now.tv_sec - start.tv_sec) * 1000L +
            (long)(now.tv_nsec - start.tv_nsec) / 1000000L;

        // Timeout
        if (data->duration_ms > 0 && elapsed_ms >= data->duration_ms)
            break;

        // Abort
        pthread_mutex_lock(&g_overlay_task.lock);
        bool local_abort = g_overlay_task.cancelled;
        pthread_mutex_unlock(&g_overlay_task.lock);
        if (local_abort)
            break;

        // Draw to each buffer
        numBuffers = data->display.vinfo.yres_virtual / data->display.vinfo.yres;
        for (int b = 0; b < numBuffers; b++) {
            draw_count++;
            display_writeBuffer(b, &data->display, data->surface->pixels, rect, data->rotate, false);
        }

        // TODO: sleep or not? atm i'd say no
        // usleep(4000);
    }

    printf("Draw count: %d\n", draw_count);
    printf("Draw speed: %f\n", (float)draw_count / (float)elapsed_ms * 1000.0f);

    // Restore original framebuffer content after overlay
    // TODO: If the content "behind" the overlay has changed, this will not restore it correctly, causing a 1 frame glitch. How to fix? Only backup if the overlay is (partially) outside the game screen?
    // TODO: Theoretically the backup/restore is only needed if the position of the overlay is outside the game screen because that part of the screen is not updated by the game.

    START_TIMER(framebuffer_restore);
    display_writeBuffers(&data->display, originalPixels, rect, data->rotate, data->useMask);

    // Free buffer backups
    numBuffers = data->display.vinfo.yres_virtual / data->display.vinfo.yres;
    for (int b = 0; b < numBuffers; b++) {
        free(originalPixels[b]);
    }
    free(originalPixels);
    END_TIMER(framebuffer_restore);

    free_overlay_data(data);

    // Clean up thread data
    pthread_mutex_lock(&g_overlay_task.lock);
    g_overlay_task.cancelled = true;
    g_overlay_task.thread_id = 0;
    pthread_mutex_unlock(&g_overlay_task.lock);

    pthread_exit(NULL);
}

/**
 * @brief Overlays an SDL_Surface onto the framebuffer at a specified position and duration.
 *
 * Flickering is minimized by drawing to all buffers in the framebuffer.
 * Should work regardless of the count of buffers (double, triple, etc. buffering).
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
    printf_debug("Overlaying surface at %d, %d, size %dx%d, duration %d\n, rotate %d\n",
                 destX, destY, surface->w, surface->h, duration_ms, rotate);
    display_init(false);

    // If there's an existing thread, abort it
    if (g_overlay_task.thread_id != 0) {
        print_debug("Cancelling existing overlay\n");
        cancel_overlay();
    }

    // Data for thread
    overlay_thread_data *data = (overlay_thread_data *)calloc(1, sizeof(overlay_thread_data));
    if (!data) {
        perror("Failed to allocate overlay_thread_data\n");
        return -1;
    }

    display_t *display = &data->display;
    display->finfo = g_display.finfo;

    // Query FB info
    if (ioctl(fb_fd, FBIOGET_VSCREENINFO, &display->vinfo) == -1) {
        perror("Error reading variable screen info");
        return -1;
    }

    if (display->vinfo.bits_per_pixel != 32) {
        perror("Only 32bpp is supported for now\n");
        return -1;
    }

    data->surface = surface;
    data->destX = destX;
    data->destY = destY;
    data->duration_ms = duration_ms;
    data->rotate = rotate;
    data->useMask = false; // TODO: Use mask dependent on running application

    // Need to map every time in case of buffer changes
    display->fb_size = (long)g_display.finfo.line_length * (long)display->vinfo.yres_virtual;
    display->fb_addr = mmap(NULL, display->fb_size, PROT_READ | PROT_WRITE, MAP_SHARED, fb_fd, 0);
    if (display->fb_addr == MAP_FAILED) {
        perror("Failed to mmap framebuffer");
        free_overlay_data(data);
        return -1;
    }

    // Create the thread
    print_debug("Creating overlay drawing thread\n");
    pthread_mutex_lock(&g_overlay_task.lock);
    g_overlay_task.cancelled = false;
    int rc = pthread_create(&g_overlay_task.thread_id, NULL, _overlay_draw_thread, data);
    if (rc != 0) {
        fprintf(stderr, "Failed to create overlay drawing thread\n");
        free_overlay_data(data);
        pthread_mutex_unlock(&g_overlay_task.lock);
        return -1;
    }

    printf_debug("Thread ID %ld created\n", g_overlay_task.thread_id);
    pthread_mutex_unlock(&g_overlay_task.lock);

    return 0;
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
    uint32_t *ofs = g_display.fb_addr;
    uint32_t i, j, curr, percentage = _bar_max > 0 ? _bar_value * g_display.height / _bar_max : 0;

    ofs += g_display.width - meterWidth;
    for (i = 0; i < g_display.height * 3; i++, ofs += g_display.width) {
        curr = (i % g_display.height) < percentage ? _bar_color : 0;
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
        uint32_t i, j, *ofs = g_display.fb_addr, *ofss = _bar_savebuf;
        ofs += g_display.width - meterWidth;
        ofss += g_display.width - meterWidth;
        for (i = 0; i < g_display.height; i++, ofs += g_display.width, ofss += g_display.width) {
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
    if ((_bar_savebuf = (uint32_t *)malloc(g_display.width * g_display.height *
                                           sizeof(uint32_t)))) {
        uint32_t i, j, *ofs = g_display.fb_addr, *ofss = _bar_savebuf;
        ofs += g_display.width - meterWidth;
        ofss += g_display.width - meterWidth;
        for (i = 0; i < g_display.height; i++, ofs += g_display.width, ofss += g_display.width) {
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
