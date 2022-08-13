#ifndef SCREENSHOT_H__
#define SCREENSHOT_H__

#include <stdio.h>
#include <png/png.h>

#include "utils/utils.h"
#include "utils/process.h"
#include "utils/hash.h"
#include "utils/log.h"
#include "./display.h"
#include "./state.h"

//
//    [onion] get recent filename from content_history.lpl
//
char* getrecent_onion(char *filename) {
    file_parseKeyValue("/mnt/SDCARD/Saves/CurrentProfile/lists/content_history.lpl", "path", filename, ':');
    printf_debug("recent path: '%s'\n", filename);
    if (*filename == 0) return NULL;
    sprintf(filename, "%"PRIu32, FNV1A_Pippip_Yurii(filename, strlen(filename)));
    return filename;
}

//
//    Get most recent file name for screenshot
//
char* getrecent_png(char *filename) {
    char *fnptr;
    uint32_t i;

    strcpy(filename, "/mnt/SDCARD/Screenshots/");
    fnptr = filename + strlen(filename);

    MenuMode mode = state_getMenuMode();

    if (mode == MODE_GAME)
        strcat(filename, getrecent_onion(fnptr));
    else if (mode == MODE_SWITCHER)
        strcat(filename, "GameSwitcher");
    else if (mode == MODE_MAIN_UI)
        strcat(filename, "MainUI");
    else if (mode == MODE_APPS && exists(CMD_TO_RUN_PATH)) {
        const char *cmd = file_read(CMD_TO_RUN_PATH);
        char app_name[STR_MAX];
        state_getAppName(app_name, cmd);
        strcat(filename, app_name);
    }

    if (!(*fnptr))
        strcat(filename, "Screenshot");

    fnptr = filename + strlen(filename);
    for (i=0; i<1000; i++) {
        sprintf(fnptr, "_%03d.png", i);
        if (!exists(filename)) break;
    }
    if (i > 999)
        return NULL;
    return filename;
}

//
//    Screenshot (640x480x32bpp only, rotate180, png)
//
void screenshot(const char *screenshot_path) {
    #ifdef PLATFORM_MIYOOMINI
    uint32_t    *buffer;
    uint32_t    *src;
    uint32_t    linebuffer[DISPLAY_WIDTH], x, y, pix;
    FILE        *fp;
    png_structp png_ptr;
    png_infop   info_ptr;

    ioctl(fb_fd, FBIOGET_VSCREENINFO, &vinfo);
    buffer = fb_addr + DISPLAY_WIDTH*vinfo.yoffset;
    #endif

    char screenshot_dir[STR_MAX];
    strcpy(screenshot_dir, screenshot_path);
    mkdirs(dirname(screenshot_dir));

    #ifdef PLATFORM_MIYOOMINI
    if ((fp = fopen(screenshot_path, "wb"))) {
        png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, 0, 0, 0);
        info_ptr = png_create_info_struct(png_ptr);
        png_init_io(png_ptr, fp);
        png_set_IHDR(png_ptr, info_ptr, DISPLAY_WIDTH, DISPLAY_HEIGHT, 8,
            PNG_COLOR_TYPE_RGBA,
            PNG_INTERLACE_NONE,
            PNG_COMPRESSION_TYPE_DEFAULT,
            PNG_FILTER_TYPE_DEFAULT);
        png_write_info(png_ptr, info_ptr);
        src = buffer + DISPLAY_WIDTH * DISPLAY_HEIGHT;
        for (y = 0; y < DISPLAY_HEIGHT; y++) {
            for (x = 0; x < DISPLAY_WIDTH; x++){
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
    #endif
}

void screenshot_recent(void) {
    char screenshot_path[512];
    if (getrecent_png(screenshot_path) == NULL) return;
    screenshot(screenshot_path);
}

void screenshot_system(void) {
    char screenshot_path[512],
         screenshot_name[256];
    if (getrecent_onion(screenshot_name) == NULL) return;
    sprintf(screenshot_path, "/mnt/SDCARD/Saves/CurrentProfile/romScreens/%s.png", screenshot_name);
    screenshot(screenshot_path);
}

#endif // SCREENSHOT_H__
