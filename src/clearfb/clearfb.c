/**
 * clearfb - Clear framebuffer to black
 * 
 * This utility clears the Linux framebuffer (/dev/fb0) to solid black.
 * Used to eliminate visual glitches during screen transitions on Miyoo Mini.
 * 
 * The Miyoo Mini uses triple buffering, so we must clear all 3 buffers
 * and optionally pan through each one to ensure the display controller
 * has fully updated.
 * 
 * Usage: clearfb [options]
 *   -a, --all     Clear all buffers and pan through each (default)
 *   -c, --current Clear only the current buffer
 *   -s, --sync    Add sync delays between buffer switches
 */

#include <fcntl.h>
#include <linux/fb.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <unistd.h>

int main(int argc, char *argv[])
{
    int fb_fd;
    struct fb_fix_screeninfo finfo;
    struct fb_var_screeninfo vinfo;
    uint32_t *fb_addr;
    long fb_size;
    int clear_all = 1;
    int with_sync = 0;

    // Parse arguments
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-c") == 0 || strcmp(argv[i], "--current") == 0) {
            clear_all = 0;
        } else if (strcmp(argv[i], "-a") == 0 || strcmp(argv[i], "--all") == 0) {
            clear_all = 1;
        } else if (strcmp(argv[i], "-s") == 0 || strcmp(argv[i], "--sync") == 0) {
            with_sync = 1;
        }
    }

    // Open framebuffer device
    fb_fd = open("/dev/fb0", O_RDWR);
    if (fb_fd < 0) {
        perror("clearfb: Failed to open /dev/fb0");
        return EXIT_FAILURE;
    }

    // Get fixed screen info
    if (ioctl(fb_fd, FBIOGET_FSCREENINFO, &finfo) < 0) {
        perror("clearfb: Failed to get fixed screen info");
        close(fb_fd);
        return EXIT_FAILURE;
    }

    // Get variable screen info
    if (ioctl(fb_fd, FBIOGET_VSCREENINFO, &vinfo) < 0) {
        perror("clearfb: Failed to get variable screen info");
        close(fb_fd);
        return EXIT_FAILURE;
    }

    fb_size = finfo.smem_len;

    // Memory map the framebuffer
    fb_addr = (uint32_t *)mmap(NULL, fb_size, PROT_READ | PROT_WRITE, MAP_SHARED, fb_fd, 0);
    if (fb_addr == MAP_FAILED) {
        perror("clearfb: Failed to mmap framebuffer");
        close(fb_fd);
        return EXIT_FAILURE;
    }

    // Calculate number of buffers (triple buffering = 3)
    int num_buffers = vinfo.yres_virtual / vinfo.yres;
    if (num_buffers < 1) num_buffers = 1;
    if (num_buffers > 3) num_buffers = 3;

    // Size of one buffer
    size_t buffer_size = vinfo.xres * vinfo.yres * (vinfo.bits_per_pixel / 8);

    if (clear_all) {
        // Clear ALL framebuffer memory (all buffers at once)
        memset(fb_addr, 0, fb_size);
        
        // Pan through each buffer to ensure display controller updates
        for (int i = 0; i < num_buffers; i++) {
            vinfo.yoffset = i * vinfo.yres;
            ioctl(fb_fd, FBIOPAN_DISPLAY, &vinfo);
            
            if (with_sync) {
                // Delay to let the display controller catch up
                // Use 33ms (~2 frames at 60Hz) for more reliable sync
                usleep(33333);
            }
        }
        
        // Reset to first buffer
        vinfo.yoffset = 0;
        ioctl(fb_fd, FBIOPUT_VSCREENINFO, &vinfo);
    } else {
        // Clear only the current buffer
        size_t offset = vinfo.yoffset * vinfo.xres * (vinfo.bits_per_pixel / 8);
        memset((uint8_t *)fb_addr + offset, 0, buffer_size);
    }

    // Cleanup
    munmap(fb_addr, fb_size);
    close(fb_fd);

    return EXIT_SUCCESS;
}
