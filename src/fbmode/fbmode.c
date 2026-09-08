/**
 * fbmode - verified framebuffer mode change for the SigmaStar mi_fb driver
 *
 * Replaces `fbset` around resolution transitions on 752x560 panels
 * (Miyoo Mini Flip / Mini Plus v4).
 *
 * The problem
 * -----------
 * The framebuffer is one fixed memory region and page boundaries are derived
 * from the current geometry, so they MOVE when the resolution changes:
 *
 *     752x560 pages:  0x6721000  0x68BC400  0x6A57800   (page = 0x19B400)
 *     640x480 pages:  0x6721000  0x684D000  0x6979000   (page = 0x12C000)
 *
 * 640 page 1 sits inside 752 page 0. After a 752->640 switch it can therefore
 * contain pixels written at a 3008-byte stride which are then scanned out at
 * 2560 bytes per row. If a controller or process flips onto that page while
 * the incoming owner is still filling buffers, alternate frames appear
 * sheared. This is the observed "dizzy lines" artifact.
 *
 * Stock `fbset -g $x $y $x $((y*2)) 32` also leaves two important details to
 * chance: it always requests two virtual pages even when the current owner is
 * using three, and it does not clear framebuffer memory whose page boundaries
 * have just moved.
 *
 * What this does
 * --------------
 * The whole transition is kept on one framebuffer descriptor:
 *
 *   open -> GET var/fix info
 *        -> optional preclear of the currently mapped framebuffer region
 *        -> pan to page 0
 *        -> PUT(FB_ACTIVATE_NOW|FB_ACTIVATE_FORCE)
 *        -> poll GET until visible AND virtual geometry match the request
 *        -> re-read fixed info to get the real post-switch line_length
 *        -> clear all requested pages using that real pitch
 *        -> pan to page 0 -> optional linger -> close
 *
 * --preclear is important. Clearing only after the mode switch leaves a short
 * window where old pixels, written using the previous row pitch, can be scanned
 * using the new pitch. Clearing first removes that window by construction.
 * The post-switch clear is still useful because page boundaries and virtual
 * layout have changed; it guarantees all pages in the new layout start clean.
 *
 * --preclear and --no-clear are independent and combine meaningfully:
 *
 *   (neither)               clear the new page layout after the latch
 *   --preclear              clear everything before, then the new layout after
 *   --preclear --no-clear   clear everything before, nothing after
 *   --no-clear              do not clear at all; preserve existing content
 *
 * --preclear --no-clear is the preferred pairing when the screen should end up
 * blank. The preclear covers the whole of smem_len, which is by definition a
 * superset of the post-switch page layout, and zeroed bytes read as zero at any
 * pitch. The post-switch clear would therefore only rewrite zeroes over zeroes,
 * so skipping it removes a full-region memset from the transition without
 * weakening the guarantee. Plain --no-clear is for transitions that must keep
 * whatever is already on screen, such as a quick switch between games.
 *
 * Page count is explicit when the caller supplies --pages. This is useful on
 * Onion because MainUI normally uses three 640x480 pages while RetroArch/game
 * paths are expected to use two pages, matching stock yres_virtual = yres*2.
 * If --pages is omitted, the current driver's page count is preserved, clamped
 * to the known safe range of 2..3 pages. An explicit --pages outside that range
 * is rejected rather than clamped, so a caller typo fails loudly instead of
 * silently running a layout it did not ask for.
 *
 * Usage:
 *   fbmode WxH [--pages N] [--preclear] [--no-clear] [--linger MS]
 *              [--timeout MS]
 *   fbmode --probe        report current geometry to stdout, change nothing
 *
 * Diagnostics are written to stderr. --probe writes machine-readable state to
 * stdout and is the only normal stdout output, so shell callers can parse it
 * without diagnostic text corrupting the result.
 *
 * Exit codes:
 *   0  requested visible/virtual geometry latched and verified
 *   2  invalid arguments, mmap failure, ioctl failure, or invalid layout
 *   3  timed out waiting for the driver to report the requested layout
 */

#include <errno.h>
#include <fcntl.h>
#include <linux/fb.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <time.h>
#include <unistd.h>

#ifndef FBIO_WAITFORVSYNC
#define FBIO_WAITFORVSYNC _IOW('F', 0x20, uint32_t)
#endif

#define FB_DEV "/dev/fb0"

/*
 * Diagnostics intentionally go to stderr.
 *
 * runtime.sh uses `fbmode --probe` as a machine-readable query, so stdout must
 * remain clean. Keeping diagnostics on stderr also lets Onion decide whether
 * to discard them or route them into its existing runtime logging without this
 * helper opening/writing its own SD-card log on every framebuffer transition.
 *
 * In other words:
 *
 *     stdout  = probe data only
 *     stderr  = human-readable diagnostics
 *
 * That separation is part of the shell API, not just a logging preference.
 */

static long now_ms(void)
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec * 1000L + ts.tv_nsec / 1000000L;
}

static void lg(const char *fmt, ...)
{
    va_list ap;

    fputs("fbmode: ", stderr);
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    va_end(ap);
    fputc('\n', stderr);
}

/*
 * Best-effort vertical-sync wait.
 *
 * SigmaStar builds differ in how completely FBIO_WAITFORVSYNC is implemented,
 * so failure is deliberately not fatal. The correctness of the transition does
 * not depend on this ioctl: correctness comes from verifying the post-PUT
 * FBIOGET_VSCREENINFO state. VSYNC is only used to reduce the chance of asking
 * the display controller to pan/commit halfway through a scanout.
 */
static void vsync(int fd)
{
    uint32_t crtc = 0;
    ioctl(fd, FBIO_WAITFORVSYNC, &crtc);
}

/*
 * Clear exactly `len` bytes of the framebuffer mapping.
 *
 * This helper is used for two different clears:
 *
 *   1. PRE-CLEAR: clear the complete old framebuffer memory region before a
 *      geometry change. This removes pixels laid out at the old stride before
 *      the driver starts interpreting the same memory with a new stride.
 *
 *   2. POST-CLEAR: after the new mode has latched, clear exactly the pages that
 *      make up the new virtual framebuffer, using the driver's newly reported
 *      line_length.
 *
 * mmap failure is treated as a transition failure. Silently continuing would
 * defeat the purpose of fbmode: the mode could be accepted while stale pixels
 * remain in memory at page offsets/strides belonging to the previous geometry.
 *
 * msync is retained even though framebuffer mappings are device memory rather
 * than ordinary files. On this target it is cheap compared with the transition
 * itself, and makes the intent explicit that all zeroing should be visible to
 * the framebuffer owner before the mapping is released.
 */
static int clear_bytes(int fd, size_t len)
{
    uint8_t *fb;

    if (len == 0)
        return 0;

    fb = mmap(NULL, len, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (fb == MAP_FAILED) {
        lg("mmap failed: %s", strerror(errno));
        return -1;
    }

    memset(fb, 0, len);
    msync(fb, len, MS_SYNC);
    munmap(fb, len);
    return 0;
}

int main(int argc, char *argv[])
{
    int fd, want_x = 0, want_y = 0, pages = 0, pages_specified = 0;
    int do_clear = 1, preclear = 0, linger_ms = 0, timeout_ms = 400, probe = 0;
    struct fb_var_screeninfo v, chk;
    struct fb_fix_screeninfo f;
    uint32_t want_y_virtual;
    long t0;

    /*
     * Argument parsing intentionally stays small: fbmode is an internal Onion
     * helper, not a general fbset replacement. Unknown arguments are ignored so
     * old callers that still pass `-v` remain compatible.
     */
    for (int i = 1; i < argc; i++) {
        if (!strcmp(argv[i], "--probe"))
            probe = 1;
        else if (!strcmp(argv[i], "--preclear"))
            preclear = 1;
        else if (!strcmp(argv[i], "--no-clear"))
            do_clear = 0;
        else if (!strcmp(argv[i], "-v"))
            ; /* backwards-compatible no-op: diagnostics are always stderr */
        else if (!strcmp(argv[i], "--pages") && i + 1 < argc) {
            /* Track presence separately from value: atoi() maps both "0" and
             * non-numeric text to 0, and neither should be mistaken for the
             * caller having omitted --pages entirely. */
            pages = atoi(argv[++i]);
            pages_specified = 1;
        }
        else if (!strcmp(argv[i], "--linger") && i + 1 < argc)
            linger_ms = atoi(argv[++i]);
        else if (!strcmp(argv[i], "--timeout") && i + 1 < argc)
            timeout_ms = atoi(argv[++i]);
        else if (strchr(argv[i], 'x'))
            sscanf(argv[i], "%dx%d", &want_x, &want_y);
    }

    if ((fd = open(FB_DEV, O_RDWR)) < 0) {
        lg("open %s failed: %s", FB_DEV, strerror(errno));
        return 2;
    }

    if (ioctl(fd, FBIOGET_VSCREENINFO, &v) < 0 ||
        ioctl(fd, FBIOGET_FSCREENINFO, &f) < 0) {
        lg("initial GET failed: %s", strerror(errno));
        close(fd);
        return 2;
    }

    /* Probe mode deliberately does not change any state. Keep its stdout
     * machine-readable because runtime.sh uses it to avoid redundant commits
     * and to verify page count as well as visible resolution. */
    if (probe) {
        printf("%ux%u virtual %ux%u bpp %u line_length %u pages %u\n",
               v.xres, v.yres, v.xres_virtual, v.yres_virtual,
               v.bits_per_pixel, f.line_length,
               v.yres ? v.yres_virtual / v.yres : 0);
        close(fd);
        return 0;
    }

    /* A non-probe invocation must name a real visible geometry. */
    if (want_x <= 0 || want_y <= 0) {
        lg("missing or invalid WxH mode");
        close(fd);
        return 2;
    }

    /* Preserve the current page count only when the caller did not specify
     * one. Onion's known useful states are two pages (RetroArch/game paths)
     * and three pages (MainUI), so clamp inherited values to that range. */
    if (!pages_specified) {
        pages = v.yres ? (int)(v.yres_virtual / v.yres) : 2;
        if (pages < 2)
            pages = 2;
        if (pages > 3)
            pages = 3;
    } else if (pages < 2 || pages > 3) {
        lg("invalid page count: %d", pages);
        close(fd);
        return 2;
    }

    /*
     * Virtual height is the page count expressed in scanlines. Keep this value
     * separate because we verify it after the PUT. Merely seeing the requested
     * visible WxH is not enough: the driver may still be using the wrong number
     * of backing pages.
     */
    want_y_virtual = (uint32_t)want_y * (uint32_t)pages;

    /* Preclear the complete currently mapped memory region before changing
     * geometry. Page boundaries move when pitch/height change, so clearing only
     * the future pages after the PUT leaves a short interval where old-stride
     * pixels can be interpreted with the new stride. */
    if (preclear && f.smem_len && clear_bytes(fd, f.smem_len) < 0) {
        close(fd);
        return 2;
    }

    /* Park scanout on page 0 before changing pitch and page boundaries. This
     * avoids asking the driver to re-base an already-offset scanout while also
     * changing the geometry that defines where each page begins. */
    if (v.yoffset != 0 || v.xoffset != 0) {
        v.xoffset = 0;
        v.yoffset = 0;
        v.activate = FB_ACTIVATE_VBL;
        if (ioctl(fd, FBIOPAN_DISPLAY, &v) < 0) {
            lg("initial PAN failed: %s", strerror(errno));
            close(fd);
            return 2;
        }
        vsync(fd);
    }

    /*
     * Build the exact layout we want. xres_virtual intentionally matches xres:
     * Onion flips vertically between full-screen pages; there is no horizontal
     * panning requirement here. Both offsets start at zero so the commit itself
     * does not inherit a page offset from the previous owner.
     */
    v.xres = v.xres_virtual = (uint32_t)want_x;
    v.yres = (uint32_t)want_y;
    v.yres_virtual = want_y_virtual;
    v.xoffset = v.yoffset = 0;
    v.bits_per_pixel = 32;
    /* FORCE defeats the driver's "same mode, skip it" shortcut. That matters
     * when visible WxH is unchanged but virtual geometry/page count needs to be
     * repaired, e.g. 752x560 with three pages -> 752x560 with two pages. */
    v.activate = FB_ACTIVATE_NOW | FB_ACTIVATE_FORCE;

    /*
     * A best-effort VSYNC before the PUT avoids committing in the middle of a
     * scan where supported. FB_ACTIVATE_FORCE is important because a same-WxH
     * request may still be changing virtual height/page count.
     */
    vsync(fd);
    if (ioctl(fd, FBIOPUT_VSCREENINFO, &v) < 0) {
        lg("PUT %dx%d failed: %s", want_x, want_y, strerror(errno));
        close(fd);
        return 2;
    }

    /* ioctl success only means the request was accepted. Do not return until
     * the driver itself reports both the requested visible geometry and the
     * requested virtual layout. Checking yres_virtual is important because a
     * same-WxH transition can still be changing two pages to three or vice
     * versa. Initialize chk first so timeout diagnostics are always defined,
     * even if every GET during polling fails. */
    chk = v;
    t0 = now_ms();
    for (;;) {
        if (ioctl(fd, FBIOGET_VSCREENINFO, &chk) == 0 &&
            chk.xres == (uint32_t)want_x &&
            chk.yres == (uint32_t)want_y &&
            chk.xres_virtual == (uint32_t)want_x &&
            chk.yres_virtual == want_y_virtual)
            break;

        if (now_ms() - t0 > timeout_ms) {
            lg("TIMEOUT: driver reports %ux%u virtual %ux%u",
               chk.xres, chk.yres, chk.xres_virtual, chk.yres_virtual);
            close(fd);
            return 3;
        }
        /* 2 ms is short enough to observe the latch closely without spinning. */
        usleep(2000);
    }

    /*
     * Fixed screen info must be re-read after the latch. In particular,
     * line_length belongs to the active mode and is the only safe authority for
     * row pitch. Using `xres * 4` would assume the driver never pads rows.
     */
    if (ioctl(fd, FBIOGET_FSCREENINFO, &f) < 0) {
        lg("final GET fix failed: %s", strerror(errno));
        close(fd);
        return 2;
    }

    /* Re-read fixed info after the mode latches because line_length is the
     * authority on framebuffer row pitch. Never derive the clear stride from
     * xres*4: the driver is free to pad scanlines. Clear only the requested new
     * page layout and verify that it fits inside the advertised framebuffer
     * memory before touching it. */
    /*
     * Clear the new virtual layout page-by-page as one contiguous range. The
     * page size comes from real line_length * visible height, not from WxH*4.
     *
     * Checking the requested length against smem_len is defensive but useful:
     * it prevents a bad page-count/driver response from turning a display fix
     * into an out-of-range framebuffer write.
     */
    if (f.smem_len) {
        size_t page_bytes = (size_t)f.line_length * chk.yres;
        size_t clear_len = page_bytes * (size_t)pages;

        /*
         * Validate the latched layout even when clearing is skipped. A page
         * layout that does not fit in framebuffer memory is an invalid result
         * regardless of whether this invocation intends to write to it, and
         * reporting it lets the caller fall back rather than hand a bad
         * layout to the incoming owner.
         */
        if (clear_len > f.smem_len) {
            lg("framebuffer too small: need %zu bytes, have %u",
               clear_len, f.smem_len);
            close(fd);
            return 2;
        }

        if (do_clear && clear_bytes(fd, clear_len) < 0) {
            close(fd);
            return 2;
        }
    }

    /* Finish on page 0. The incoming owner can then allocate/fill its own
     * buffers from a known scanout base instead of inheriting an arbitrary page
     * offset from the previous owner. */
    chk.xoffset = chk.yoffset = 0;
    chk.activate = FB_ACTIVATE_VBL;
    if (ioctl(fd, FBIOPAN_DISPLAY, &chk) < 0) {
        lg("final PAN failed: %s", strerror(errno));
        close(fd);
        return 2;
    }
    vsync(fd);

    /*
     * At this point all conditions fbmode promises are true:
     *   - the driver reports the requested visible geometry;
     *   - the requested virtual page layout is active;
     *   - requested pages have been cleared at the active pitch (unless
     *     --no-clear was explicitly requested);
     *   - scanout is parked on page 0.
     */
    lg("%dx%d latched in %ldms, %d pages, pitch %u",
       want_x, want_y, now_ms() - t0, pages, f.line_length);

    /* Keep fb0 open briefly when requested. On this driver that gives the
     * incoming process time to open the framebuffer before this transition
     * descriptor disappears, which proved useful around ownership handoffs. */
    if (linger_ms > 0)
        usleep((useconds_t)linger_ms * 1000);

    close(fd);
    return 0;
}
