#ifndef SCREENSHOT_H__
#define SCREENSHOT_H__

#include <png/png.h>
#include <signal.h>
#include <stdio.h>
#include <sys/types.h>

#include "./display.h"
#include "./state.h"
#include "utils/file.h"
#include "utils/hash.h"
#include "utils/log.h"
#include "utils/process.h"
#include "utils/str.h"

bool __get_path_romscreen(char *path_out)
{
    char filename[STR_MAX] = "";
    char file_path[STR_MAX];
    char core_path[STR_MAX];

    if (history_getRecentPath(file_path) != NULL) {
        sprintf(filename, "%" PRIu32, FNV1A_Pippip_Yurii(file_path, strlen(file_path)));
    }

    if (history_getRecentCorePath(core_path) != NULL) {
        strcat(filename, "_");
        strcat(filename, basename(core_path));
        str_split(filename, "_libretro");
    }

    if (strlen(filename) > 0) {
        sprintf(path_out, "/mnt/SDCARD/Saves/CurrentProfile/romScreens/%s.png", filename);
        return true;
    }

    return false;
}

bool __get_path_recent(char *path_out)
{
    char *fnptr;
    uint32_t i;

    strcpy(path_out, "/mnt/SDCARD/Screenshots/");
    fnptr = path_out + strlen(path_out);

    system_state_update();

    if (system_state == MODE_GAME && (process_searchpid("retroarch") != 0 || process_searchpid("ra32") != 0)) {
        char file_path[STR_MAX];
        if (history_getRecentPath(file_path) != NULL)
            strcat(path_out, file_removeExtension(basename(file_path)));
    }
    else if (system_state == MODE_SWITCHER)
        strcat(path_out, "GameSwitcher");
    else if (system_state == MODE_MAIN_UI)
        strcat(path_out, "MainUI");
    else if ((system_state == MODE_GAME || system_state == MODE_APPS) && exists(CMD_TO_RUN_PATH)) {
        FILE *fp;
        char cmd[STR_MAX];
        file_get(fp, CMD_TO_RUN_PATH, "%[^\n]", cmd);
        printf_debug("cmd: '%s'\n", cmd);

        char app_name[STR_MAX];

        if (strstr(cmd, "; chmod") != NULL)
            state_getAppName(app_name, cmd);
        else {
            strcpy(app_name, file_removeExtension(basename(cmd)));
        }
        printf_debug("app: '%s'\n", app_name);

        strcat(path_out, app_name);
    }

    if (!(*fnptr))
        strcat(path_out, "Screenshot");

    fnptr = path_out + strlen(path_out);
    for (i = 0; i < 1000; i++) {
        sprintf(fnptr, "_%03d.png", i);
        if (!exists(path_out))
            break;
    }

    return i <= 999;
}

uint32_t *__screenshot_buffer(void)
{
    size_t buffer_size = DISPLAY_WIDTH * DISPLAY_HEIGHT * sizeof(uint32_t);
    uint32_t *buffer = (uint32_t *)malloc(buffer_size);

    ioctl(fb_fd, FBIOGET_VSCREENINFO, &vinfo);
    memcpy(buffer, fb_addr + DISPLAY_WIDTH * vinfo.yoffset, buffer_size);

    return buffer;
}

/**
 * Scales a framebuffer to given size using nearest neighbor interpolation.
 * @param input Pointer to the input framebuffer.
 * @param in_width Width of the input framebuffer.
 * @param in_height Height of the input framebuffer.
 * @param out_width Width of the output framebuffer.
 * @param out_height Height of the output framebuffer.
 * @return Pointer to the scaled output framebuffer.
 *         Returns NULL if memory allocation fails.
 */
uint32_t *__scale_fb(const uint32_t *input, int in_width, int in_height, int out_width, int out_height)
{
    printf_debug("Scaling framebuffer from %dx%d to %dx%d\n", in_width, in_height, out_width, out_height);
    uint32_t *out = (uint32_t *)malloc(out_width * out_height * sizeof(uint32_t));
    if (!out)
        return NULL;

    float x_ratio = (float)in_width / out_width;
    float y_ratio = (float)in_height / out_height;

    for (int y = 0; y < out_height; ++y) {
        for (int x = 0; x < out_width; ++x) {
            int px = (int)(x * x_ratio);
            int py = (int)(y * y_ratio);
            out[y * out_width + x] = input[py * in_width + px];
        }
    }
    return out;
}

/**
 * @brief Screenshot (640x480x32bpp only, rotate180, png)
 * 
 * @param buffer pointer to the frame buffer
 * @param screenshot_path image file save path
 * @return true Screenshot was saved
 * @return false Screenshot was not saved
 */
bool __screenshot_save(const uint32_t *buffer, const char *screenshot_path)
{
    uint32_t *src;
    int screenshot_x = 640;
    int screenshot_y = 480;

    uint32_t line_buffer[screenshot_x], x, y, pix;

    FILE *fp;
    png_structp png_ptr;
    png_infop info_ptr;

    // make sure render resolution is up to date
    display_getRenderResolution();

    if (RENDER_HEIGHT != screenshot_y || RENDER_WIDTH != screenshot_x)
        buffer = __scale_fb(buffer, RENDER_WIDTH, RENDER_HEIGHT, screenshot_x, screenshot_y);

    if (!(fp = file_open_ensure_path(screenshot_path, "wb"))) {
        return false;
    }

    png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, 0, 0, 0);
    info_ptr = png_create_info_struct(png_ptr);

    png_init_io(png_ptr, fp);
    png_set_IHDR(png_ptr, info_ptr, screenshot_x, screenshot_y, 8,
                 PNG_COLOR_TYPE_RGBA, PNG_INTERLACE_NONE,
                 PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);
    png_write_info(png_ptr, info_ptr);

    src = (uint32_t *)buffer + screenshot_x * screenshot_y;

    for (y = 0; y < screenshot_y; y++) {
        for (x = 0; x < screenshot_x; x++) {
            pix = *--src;
            line_buffer[x] = 0xFF000000 | (pix & 0x0000FF00) | (pix & 0x00FF0000) >> 16 | (pix & 0x000000FF) << 16;
        }
        png_write_row(png_ptr, (png_bytep)line_buffer);
    }

    png_write_end(png_ptr, info_ptr);
    png_destroy_write_struct(&png_ptr, &info_ptr);

    fflush(fp);
    fsync(fileno(fp));
    fclose(fp);

    return true;
}

bool __screenshot_perform(bool(get_path)(char *))
{
    bool retval = false;
    char path[512];
    uint32_t *buffer;
    pid_t ra_pid;

    if ((ra_pid = process_searchpid("retroarch")) != 0) {
        kill(ra_pid, SIGSTOP);
    }

    buffer = __screenshot_buffer();

    if (ra_pid != 0) {
        kill(ra_pid, SIGCONT);
    }

    if (get_path(path)) {
        retval = __screenshot_save(buffer, path);
    }

    free(buffer);

    return retval;
}

bool screenshot_recent(void)
{
    return __screenshot_perform(__get_path_recent);
}

bool screenshot_system(void)
{
    return __screenshot_perform(__get_path_romscreen);
}

#endif // SCREENSHOT_H__
