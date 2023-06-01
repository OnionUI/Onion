#include <SDL/SDL.h>
#include <fcntl.h>
#include <linux/fb.h>
#include <mi_gfx.h>
#include <mi_sys.h>
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <unistd.h>

#define pixelsPa unused1
#define ALIGN4K(val) ((val + 4095) & (~4095))
//	FREEMMA		: force free all allocated MMAs when init & quit
#define FREEMMA
//	GFX_BLOCKING	: limit to 60fps but never skips frames
//			:  in case of clearing all buffers by GFX_Flip()x3,
// needs to use BLOCKING (or GFX_FlipForce())
//	GFX_FLIPWAIT	: wait until Blit is done when flip
//			:  when NOWAIT, do not clear/write source surface
// immediately after Flip 			:  if absolutely necessary, use
// GFX_WaitAllDone() before write (or GFX_FlipWait())
enum { GFX_BLOCKING = 1,
       GFX_FLIPWAIT = 2 };
//#define	DEFAULTFLIPFLAGS	(GFX_BLOCKING | GFX_FLIPWAIT)
//// low performance with blocking #define	DEFAULTFLIPFLAGS (GFX_FLIPWAIT)
//// middle performance nonblock, recommended for most cases
#define DEFAULTFLIPFLAGS 0 // high performance but with the above precautions

int fd_fb = 0;
void *fb_addr;
struct fb_fix_screeninfo finfo;
struct fb_var_screeninfo vinfo;
MI_GFX_Surface_t stSrc;
MI_GFX_Rect_t stSrcRect;
MI_GFX_Surface_t stDst;
MI_GFX_Rect_t stDstRect;
MI_GFX_Opt_t stOpt;
volatile uint32_t now_flipping;
MI_PHY shadowPa;
uint32_t shadowsize;
pthread_t flip_pt;
pthread_mutex_t flip_mx;
pthread_cond_t flip_req;
pthread_cond_t flip_start;
MI_U16 flipFence;
uint32_t flipFlags;
SDL_Surface *sHWsurface;
SDL_Surface *videosurface;
MI_GFX_Surface_t sHW;
MI_GFX_Rect_t sHWRect;
MI_GFX_Opt_t sHWOpt;
void (*flip_callback)(void *) = NULL;
void *userdata_callback = NULL;
#ifndef FREEMMA
#define MMADBMAX 100
uint32_t mma_db[MMADBMAX];
#endif

//
//	Actual Flip thread
//
static void *GFX_FlipThread(void *param)
{
    uint32_t target_offset;
    MI_U16 Fence;
    pthread_mutex_lock(&flip_mx);
    while (1) {
        while (!now_flipping)
            pthread_cond_wait(&flip_req, &flip_mx);
        Fence = flipFence;
        do {
            target_offset = vinfo.yoffset + 480;
            if (target_offset == 1440)
                target_offset = 0;
            vinfo.yoffset = target_offset;
            pthread_cond_signal(&flip_start);
            pthread_mutex_unlock(&flip_mx);
            if (flip_callback) {
                // Wait done always when callback is active
                MI_GFX_WaitAllDone(FALSE, flipFence);
                Fence = 0;
                flip_callback(userdata_callback);
            }
            else if (Fence) {
                MI_GFX_WaitAllDone(FALSE, Fence);
                Fence = 0;
            }
            ioctl(fd_fb, FBIOPAN_DISPLAY, &vinfo);
            pthread_mutex_lock(&flip_mx);
        } while (--now_flipping);
    }
    return 0;
}

//
//	Actual Flip thread ( for single HW surface )
//		blit from sHWsurface to FB every frame
//
static void *GFX_FlipThreadSingleHW(void *param)
{
    MI_GFX_Surface_t Src = sHW;
    MI_GFX_Rect_t SrcRect = sHWRect;
    MI_GFX_Surface_t Dst = stDst;
    MI_GFX_Rect_t DstRect = stDstRect;
    MI_U16 Fence;
    uint32_t sHWsize4K = ALIGN4K(sHWsurface->pitch * sHWsurface->h);
    uint32_t target_offset;

    while (1) {
        MI_SYS_FlushInvCache(sHWsurface->pixels, sHWsize4K);
        target_offset = vinfo.yoffset ^ 480;
        Dst.phyAddr = finfo.smem_start + (640 * target_offset * 4);
        MI_GFX_BitBlit(&Src, &SrcRect, &Dst, &DstRect, &stOpt, &Fence);
        usleep(0x2000); // wait about 10ms
        vinfo.yoffset = target_offset;
        if (flip_callback)
            flip_callback(userdata_callback);
        ioctl(fd_fb, FBIOPAN_DISPLAY, &vinfo);
    }
    return 0;
}

//
//	Get GFX_ColorFmt from SDL_Surface
//
static inline MI_GFX_ColorFmt_e GFX_ColorFmt(SDL_Surface *surface)
{
    if (surface) {
        if (surface->format->BytesPerPixel == 2) {
            if (surface->format->Amask == 0x0000)
                return E_MI_GFX_FMT_RGB565;
            if (surface->format->Amask == 0x8000)
                return E_MI_GFX_FMT_ARGB1555;
            if (surface->format->Amask == 0xF000)
                return E_MI_GFX_FMT_ARGB4444;
            if (surface->format->Amask == 0x0001)
                return E_MI_GFX_FMT_RGBA5551;
            if (surface->format->Amask == 0x000F)
                return E_MI_GFX_FMT_RGBA4444;
            return E_MI_GFX_FMT_RGB565;
        }
        if (surface->format->Bmask == 0x000000FF)
            return E_MI_GFX_FMT_ARGB8888;
        if (surface->format->Rmask == 0x000000FF)
            return E_MI_GFX_FMT_ABGR8888;
    }
    return E_MI_GFX_FMT_ARGB8888;
}

//
//	Get SYS_PixelFormat from SDL_Surface
//
static inline MI_SYS_PixelFormat_e SYS_PixelFormat(SDL_Surface *surface)
{
    if (surface) {
        if (surface->format->BytesPerPixel == 2) {
            if (surface->format->Amask == 0x0000)
                return E_MI_SYS_PIXEL_FRAME_RGB565;
            if (surface->format->Amask == 0x8000)
                return E_MI_SYS_PIXEL_FRAME_ARGB1555;
            if (surface->format->Amask == 0xF000)
                return E_MI_SYS_PIXEL_FRAME_ARGB4444;
            return E_MI_SYS_PIXEL_FRAME_RGB565;
        }
        if (surface->format->Bmask == 0x000000FF)
            return E_MI_SYS_PIXEL_FRAME_ARGB8888;
        if (surface->format->Rmask == 0x000000FF)
            return E_MI_SYS_PIXEL_FRAME_ABGR8888;
        if (surface->format->Amask == 0x000000FF)
            return E_MI_SYS_PIXEL_FRAME_BGRA8888;
    }
    return E_MI_SYS_PIXEL_FRAME_ARGB8888;
}

//
//	Flush write cache of needed segments
//		x and w are not considered since 4K units
//
static inline void FlushCacheNeeded(void *pixels, uint32_t pitch, uint32_t y,
                                    uint32_t h)
{
    uintptr_t pixptr = (uintptr_t)pixels;
    uintptr_t startaddress = (pixptr + pitch * y) & (~4095);
    uint32_t size = ALIGN4K(pixptr + pitch * (y + h)) - startaddress;
    if (size)
        MI_SYS_FlushInvCache((void *)startaddress, size);
}

//
//	GFX Flip / in place of SDL_Flip
//		HW Blit : surface -> FB(backbuffer) with
// Rotate180/bppConvert/Scaling 			and Request Flip
// *Note* blit from entire surface(or clip_rect if specified) to entire
// framebuffer(or sHWsurface)
//
void GFX_FlipExec(SDL_Surface *surface, uint32_t flags)
{
    uint32_t target_offset, surfacesize;

    if ((fd_fb) && (surface) && (surface->pixelsPa)) {
        surfacesize = surface->pitch * surface->h;
        stSrc.eColorFmt = GFX_ColorFmt(surface);
        stSrc.u32Width = surface->w;
        stSrc.u32Height = surface->h;
        stSrc.u32Stride = surface->pitch;
        if (surface->clip_rect.w | surface->clip_rect.h) {
            // when clip_rect is specified
            stSrcRect.s32Xpos = surface->clip_rect.x;
            stSrcRect.s32Ypos = surface->clip_rect.y;
            stSrcRect.u32Width = surface->clip_rect.w;
            stSrcRect.u32Height = surface->clip_rect.h;
        }
        else { // entire screen
            stSrcRect.s32Xpos = 0;
            stSrcRect.s32Ypos = 0;
            stSrcRect.u32Width = stSrc.u32Width;
            stSrcRect.u32Height = stSrc.u32Height;
        }

        if (sHWsurface) {
            if (surface != sHWsurface) {
                // blit to sHWsurface when direct draw mode
                MI_U16 Fence;
                stSrc.phyAddr = surface->pixelsPa;
                FlushCacheNeeded(surface->pixels, surface->pitch,
                                 stSrcRect.s32Ypos, stSrcRect.u32Height);
                MI_GFX_BitBlit(&stSrc, &stSrcRect, &sHW, &sHWRect, &sHWOpt,
                               &Fence);
            }
            return;
        }

        if (flags & GFX_FLIPWAIT) {
            // wait for recent flip is done
            if (flipFence)
                MI_GFX_WaitAllDone(FALSE, flipFence);
            // prepare intermediate buffer if needed
            if (shadowsize < surfacesize) {
                if (shadowPa)
                    MI_SYS_MMA_Free(shadowPa);
                if (MI_SYS_MMA_Alloc(NULL, ALIGN4K(surfacesize), &shadowPa)) {
                    shadowPa = shadowsize = 0;
                    goto NOWAIT;
                }
                shadowsize = surfacesize;
            }
            // copy surface to intermediate buffer
            uint32_t ofs = surface->pitch * stSrcRect.s32Ypos;
            uint32_t size = surface->pitch * stSrcRect.u32Height;
            MI_SYS_FlushInvCache((uint8_t *)surface->pixels + ofs,
                                 ALIGN4K(size));
            MI_SYS_MemcpyPa(shadowPa + ofs, surface->pixelsPa + ofs, size);
            // blit from intermediate buffer
            stSrc.phyAddr = shadowPa;
        }
        else {
        NOWAIT:
            FlushCacheNeeded(surface->pixels, surface->pitch, stSrcRect.s32Ypos,
                             stSrcRect.u32Height);
            stSrc.phyAddr = surface->pixelsPa;
        }

        pthread_mutex_lock(&flip_mx);
        if (flags & GFX_BLOCKING) {
            while (now_flipping == 2)
                pthread_cond_wait(&flip_start, &flip_mx);
        }
        target_offset = vinfo.yoffset + 480;
        if (target_offset == 1440)
            target_offset = 0;
        stDst.phyAddr = finfo.smem_start + (640 * target_offset * 4);
        MI_GFX_BitBlit(&stSrc, &stSrcRect, &stDst, &stDstRect, &stOpt,
                       &flipFence);

        // Request Flip
        if (!now_flipping) {
            now_flipping = 1;
            pthread_cond_signal(&flip_req);
            pthread_cond_wait(&flip_start, &flip_mx);
        }
        else {
            now_flipping = 2;
        }
        pthread_mutex_unlock(&flip_mx);
    }
}
void GFX_Flip(SDL_Surface *surface) { GFX_FlipExec(surface, flipFlags); }
void GFX_FlipNoWait(SDL_Surface *surface)
{
    GFX_FlipExec(surface, flipFlags & ~GFX_FLIPWAIT);
}
void GFX_FlipWait(SDL_Surface *surface)
{
    GFX_FlipExec(surface, flipFlags | GFX_FLIPWAIT);
}
void GFX_FlipForce(SDL_Surface *surface)
{
    GFX_FlipExec(surface, flipFlags | GFX_BLOCKING);
}

//
//	Get/Set Flipflags
//		GFX_BLOCKING/GFX_FLIPWAIT flags used for GFX_Flip/UpdateRect
//
uint32_t GFX_GetFlipFlags(void) { return flipFlags; }
void GFX_SetFlipFlags(uint32_t flags) { flipFlags = flags; }

//
//	Get/Set Flip callback, for use direct draw to framebuffer
//		(Battery icon, RetroArch OSD text, etc)
//		*Note* framebuffer is rotated 180 degrees
//
void *GFX_GetFlipCallback(void) { return (void *)flip_callback; }
void GFX_SetFlipCallback(void (*callback)(void *), void *userdata)
{
    userdata_callback = userdata;
    flip_callback = callback;
}

#ifdef FREEMMA
//
//	Free all allocated MMAs (except "daemon")
//
void freemma(void)
{
    FILE *fp;
    const char *heapinfoname = "/proc/mi_modules/mi_sys_mma/mma_heap_name0";
    char str[256];
    uint32_t offset, length, usedflag;
    uint32_t baseaddr = finfo.smem_start - 0x021000; // default baseaddr (tmp)

    // open heap information file
    fp = fopen(heapinfoname, "r");
    if (fp) {
        // skip reading until chunk information
        do {
            if (fscanf(fp, "%255s", str) == EOF) {
                fclose(fp);
                return;
            }
        } while (strcmp(str, "sys-logConfig"));
        // get MMA each chunk information and release
        while (fscanf(fp, "%x %x %x %255s", &offset, &length, &usedflag, str) !=
               EOF) {
            if (!usedflag)
                continue;                    // NA
            if (!strcmp(str, "fb_device")) { // FB .. fix baseaddr
                baseaddr = finfo.smem_start - offset;
                continue;
            }
            if (!strcmp(str, "ao-Dev0-tmp"))
                continue; // ao .. Audio buffer, skip
            // For daemon program authors, MMA allocated as "daemon" will not be
            // released
            if (strncmp(str, "daemon",
                        6)) { // others except "daemon" .. release
                if (!MI_SYS_MMA_Free(baseaddr + offset)) {
                    fprintf(stderr,
                            "MMA_Released %s offset : %08X length : %08X\n",
                            str, offset, length);
                }
            }
        }
        fclose(fp);
    }
}
#endif

//
//	Create GFX Surface / in place of SDL_CreateRGBSurface
//		supports 16/32bpp only / flags has no meaning, fixed to
// SWSURFACE 		Additional return value : surface->unused1 = Physical
// address of surface
//
SDL_Surface *GFX_CreateRGBSurface(uint32_t flags, int width, int height,
                                  int depth, uint32_t Rmask, uint32_t Gmask,
                                  uint32_t Bmask, uint32_t Amask)
{
    SDL_Surface *surface;
    MI_PHY phyAddr;
    void *virAddr;
    if (!width)
        width = 640;
    if (!height)
        height = 480;
    if (depth != 16)
        depth = 32;
    int pitch = width * (uint32_t)(depth / 8);
    uint32_t size = pitch * height;

    if (MI_SYS_MMA_Alloc(NULL, ALIGN4K(size), &phyAddr)) {
        // No MMA left .. create normal SDL surface
        return SDL_CreateRGBSurface(flags, width, height, depth, Rmask, Gmask,
                                    Bmask, Amask);
    }
#ifndef FREEMMA
    uint32_t i;
    for (i = 0; i < MMADBMAX; i++) {
        if (!mma_db[i]) {
            mma_db[i] = phyAddr;
            break;
        }
    }
    if (i == MMADBMAX) {
        MI_SYS_MMA_Free(phyAddr);
        return NULL;
    }
#endif
    MI_SYS_Mmap(phyAddr, ALIGN4K(size), &virAddr,
                TRUE); // write cache ON needs Flush when r/w Pa directly

    surface = SDL_CreateRGBSurfaceFrom(virAddr, width, height, depth, pitch,
                                       Rmask, Gmask, Bmask, Amask);
    if (surface) {
        surface->pixelsPa = phyAddr;
        memset(surface->pixels, 0, size);
    }
    return surface;
}

//
//	Free GFX Surface / in place of SDL_FreeSurface
//
void GFX_FreeSurface(SDL_Surface *surface)
{
    if (surface) {
        MI_PHY phyAddr = surface->pixelsPa;
        void *virAddr = surface->pixels;
        uint32_t size = surface->pitch * surface->h;

        // stop flip thread when sHWsurface is freed
        if (surface == sHWsurface) {
            pthread_cancel(flip_pt);
            pthread_join(flip_pt, NULL);
            sHWsurface = NULL;
        }

        SDL_FreeSurface(surface);
        if (phyAddr) {
            MI_SYS_Munmap(virAddr, ALIGN4K(size));
            MI_SYS_MMA_Free(phyAddr);
#ifndef FREEMMA
            for (uint32_t i = 0; i < MMADBMAX; i++) {
                if (mma_db[i] == phyAddr) {
                    mma_db[i] = 0;
                    break;
                }
            }
#endif
        }
    }
}

//
//	Clear entire FrameBuffer
//
void GFX_ClearFrameBuffer(void) { memset(fb_addr, 0, finfo.smem_len); }

//
//	GFX Init / Prepare for HW Blit to FB, call after SDL_Init
//
void GFX_Init(void)
{
    if (!fd_fb) {
        MI_SYS_Init();
        MI_GFX_Open();
        fd_fb = open("/dev/fb0", O_RDWR);

        // 640 x 480 x 32bpp x 3screen init
        SDL_SetVideoMode(640, 480, 32, SDL_SWSURFACE);
        ioctl(fd_fb, FBIOGET_VSCREENINFO, &vinfo);
        vinfo.yres_virtual = 1440;
        vinfo.yoffset = 0;
        /* vinfo.xres = vinfo.xres_virtual = 640; vinfo.yres = 480;
        vinfo.xoffset = vinfo.yoffset = vinfo.red.msb_right =
        vinfo.green.msb_right = vinfo.blue.msb_right = vinfo.transp.msb_right =
        vinfo.blue.offset = 0; vinfo.red.length = vinfo.green.length =
        vinfo.blue.length = vinfo.transp.length = vinfo.green.offset = 8;
        vinfo.red.offset = 16; vinfo.transp.offset = 24; vinfo.bits_per_pixel =
        32; */
        ioctl(fd_fb, FBIOPUT_VSCREENINFO, &vinfo);

        // get physical address of FB
        ioctl(fd_fb, FBIOGET_FSCREENINFO, &finfo);

        // map fb memory
        fb_addr = mmap(0, finfo.smem_len, PROT_READ | PROT_WRITE, MAP_SHARED,
                       fd_fb, 0);

        // clear entire FB
        GFX_ClearFrameBuffer();
#ifdef FREEMMA
        freemma();
#else
        memset(mma_db, 0, sizeof(mma_db));
#endif
        // prepare for Flip
        stDst.phyAddr = finfo.smem_start;
        stDst.eColorFmt = E_MI_GFX_FMT_ARGB8888;
        stDst.u32Width = 640;
        stDst.u32Height = 480;
        stDst.u32Stride = 640 * 4;
        stDstRect.s32Xpos = 0;
        stDstRect.s32Ypos = 0;
        stDstRect.u32Width = 640;
        stDstRect.u32Height = 480;

        memset(&stOpt, 0, sizeof(stOpt));
        stOpt.eSrcDfbBldOp = E_MI_GFX_DFB_BLD_ONE;
        stOpt.eRotate = E_MI_GFX_ROTATE_180;

        flip_mx = (pthread_mutex_t)PTHREAD_MUTEX_INITIALIZER;
        flip_req = (pthread_cond_t)PTHREAD_COND_INITIALIZER;
        flip_start = (pthread_cond_t)PTHREAD_COND_INITIALIZER;
        now_flipping = shadowPa = shadowsize = flipFence = 0;
        sHWsurface = videosurface = NULL;
        flipFlags = DEFAULTFLIPFLAGS;
        pthread_create(&flip_pt, NULL, GFX_FlipThread, NULL);
    }
}

//
//	GFX Quit / call before SDL_Quit
//
void GFX_Quit(void)
{
    if (fd_fb) {
        pthread_cancel(flip_pt);
        pthread_join(flip_pt, NULL);

        MI_GFX_WaitAllDone(TRUE, 0);
        if (sHWsurface) {
            SDL_Surface *sHWpush = sHWsurface;
            sHWsurface = NULL;
            GFX_FreeSurface(sHWpush);
        }
        if (videosurface) {
            GFX_FreeSurface(videosurface);
            videosurface = NULL;
        }
        if (shadowPa) {
            MI_SYS_MMA_Free(shadowPa);
            shadowPa = 0;
        }
#ifdef FREEMMA
        freemma();
#else
        for (uint32_t i = 0; i < MMADBMAX; i++) {
            if (mma_db[i]) {
                if (!MI_SYS_MMA_Free(mma_db[i])) {
                    fprintf(stderr, "MMA_Released offset : %08X\n", mma_db[i]);
                    mma_db[i] = 0;
                }
            }
        }
#endif
        // clear entire FB
        GFX_ClearFrameBuffer();

        // reset yoffset
        vinfo.yoffset = 0;
        ioctl(fd_fb, FBIOPUT_VSCREENINFO, &vinfo);

        // unmap fb memory
        munmap(fb_addr, finfo.smem_len);

        close(fd_fb);
        fd_fb = 0;

        MI_GFX_Close();
        MI_SYS_Exit();
    }
}

//
//	SetVideomode / in place of SDL_SetVideoMode
//		if flags == SDL_HWSURFACE & non SDL_DOUBLEBUF, change to direct
// draw mode 		 otherwise, same as GFX_CreateRGBSurface
//
SDL_Surface *GFX_SetVideoMode(int width, int height, int bpp, uint32_t flags)
{
    if (!fd_fb)
        GFX_Init();
    if (!width)
        width = 640;
    if (!height)
        height = 480;
    if (bpp != 16)
        bpp = 32;

    // reinit Flip thread
    pthread_cancel(flip_pt);
    pthread_join(flip_pt, NULL);
    MI_GFX_WaitAllDone(TRUE, 0);
    if (sHWsurface) {
        SDL_Surface *sHWpush = sHWsurface;
        sHWsurface = NULL;
        GFX_FreeSurface(sHWpush);
    }
    if (videosurface) {
        GFX_FreeSurface(videosurface);
        videosurface = NULL;
    }
    if (shadowPa) {
        MI_SYS_MMA_Free(shadowPa);
        shadowPa = shadowsize = 0;
    }
    flip_mx = (pthread_mutex_t)PTHREAD_MUTEX_INITIALIZER;
    flip_req = (pthread_cond_t)PTHREAD_COND_INITIALIZER;
    flip_start = (pthread_cond_t)PTHREAD_COND_INITIALIZER;
    now_flipping = flipFence = vinfo.yoffset = 0;
    GFX_ClearFrameBuffer();
    ioctl(fd_fb, FBIOPAN_DISPLAY, &vinfo);

    if ((flags & SDL_HWSURFACE) && (!(flags & SDL_DOUBLEBUF))) {
        // single HW surface, direct draw mode
        sHWsurface =
            GFX_CreateRGBSurface(flags, width, height, bpp, 0, 0, 0, 0);
        if (sHWsurface) {
            sHW.phyAddr = sHWsurface->pixelsPa;
            sHW.u32Width = sHWsurface->w;
            sHW.u32Height = sHWsurface->h;
            sHW.u32Stride = sHWsurface->pitch;
            sHW.eColorFmt = GFX_ColorFmt(sHWsurface);
            sHWRect.s32Xpos = 0;
            sHWRect.s32Ypos = 0;
            sHWRect.u32Width = sHW.u32Width;
            sHWRect.u32Height = sHW.u32Height;
            memset(&sHWOpt, 0, sizeof(sHWOpt));
            sHWOpt.eSrcDfbBldOp = E_MI_GFX_DFB_BLD_ONE;
            pthread_create(&flip_pt, NULL, GFX_FlipThreadSingleHW, NULL);
        }
        else
            pthread_create(&flip_pt, NULL, GFX_FlipThread, NULL);
        return sHWsurface;
    }
    else {
        // others
        pthread_create(&flip_pt, NULL, GFX_FlipThread, NULL);
        videosurface =
            GFX_CreateRGBSurface(flags, width, height, bpp, 0, 0, 0, 0);
        return videosurface;
    }
}

//
//	Clear GFX/SDL Surface (entire)
//
void GFX_ClearSurface(SDL_Surface *surface)
{
    if (surface) {
        uint32_t size = surface->pitch * surface->h;
        memset(surface->pixels, 0, size);
    }
}

//
//	Copy GFX/SDL Surface (entire)
//		src/dst surfaces must be the same size
//
void GFX_CopySurface(SDL_Surface *src, SDL_Surface *dst)
{
    if ((src) && (dst)) {
        uint32_t size = src->pitch * src->h;
        if (size == (uint32_t)(dst->pitch * dst->h)) {
            if ((src->pixelsPa) && (dst->pixelsPa)) {
                MI_SYS_FlushInvCache(src->pixels, ALIGN4K(size));
                MI_SYS_FlushInvCache(dst->pixels, ALIGN4K(size));
                MI_SYS_MemcpyPa(dst->pixelsPa, src->pixelsPa, size);
            }
            else {
                memcpy(dst->pixels, src->pixels, size);
            }
        }
    }
}

//
//	Rotate 640x480x32bpp surface NEON / for duplicate FB
//
void RotateSurfaceNEON(void *src)
{
    asm volatile("	add r1,%0,#(640*240*4)	;"
                 "	mov r2,r1		;"
                 "	mov r3,r1		;"
                 "1:	vldmia r1,{q0-q7}	;"
                 "	vrev64.32 d31,d0	;"
                 "	vrev64.32 d30,d1	;"
                 "	vrev64.32 d29,d2	;"
                 "	vrev64.32 d28,d3	;"
                 "	vrev64.32 d27,d4	;"
                 "	vrev64.32 d26,d5	;"
                 "	vrev64.32 d25,d6	;"
                 "	vrev64.32 d24,d7	;"
                 "	vrev64.32 d23,d8	;"
                 "	vrev64.32 d22,d9	;"
                 "	vrev64.32 d21,d10	;"
                 "	vrev64.32 d20,d11	;"
                 "	vrev64.32 d19,d12	;"
                 "	vrev64.32 d18,d13	;"
                 "	vrev64.32 d17,d14	;"
                 "	vrev64.32 d16,d15	;"
                 "	vldmdb r3!,{q0-q7}	;"
                 "	vstmdb r2!,{q8-q15}	;"
                 "	vrev64.32 d31,d0	;"
                 "	vrev64.32 d30,d1	;"
                 "	vrev64.32 d29,d2	;"
                 "	vrev64.32 d28,d3	;"
                 "	vrev64.32 d27,d4	;"
                 "	vrev64.32 d26,d5	;"
                 "	vrev64.32 d25,d6	;"
                 "	vrev64.32 d24,d7	;"
                 "	vrev64.32 d23,d8	;"
                 "	vrev64.32 d22,d9	;"
                 "	vrev64.32 d21,d10	;"
                 "	vrev64.32 d20,d11	;"
                 "	vrev64.32 d19,d12	;"
                 "	vrev64.32 d18,d13	;"
                 "	vrev64.32 d17,d14	;"
                 "	vrev64.32 d16,d15	;"
                 "	vstmia r1!,{q8-q15}	;"
                 "	cmp %0,r2		;"
                 "	bne 1b			" ::"r"(src)
                 : "r1", "r2", "r3", "q0", "q1", "q2", "q3", "q4", "q5", "q6",
                   "q7", "q8", "q9", "q10", "q11", "q12", "q13", "q14", "q15",
                   "memory", "cc");
}

//
//	Duplicate GFX Surface from SDL_Surface or FB
//		if src is NULL, duplicate from FB currently displayed
//
SDL_Surface *GFX_DuplicateSurface(SDL_Surface *src)
{
    SDL_Surface *dst;
    if (src) {
        dst = GFX_CreateRGBSurface(0, src->w, src->h, src->format->BitsPerPixel,
                                   src->format->Rmask, src->format->Gmask,
                                   src->format->Bmask, src->format->Amask);
        if (dst)
            GFX_CopySurface(src, dst);
    }
    else {
        dst = GFX_CreateRGBSurface(0, 640, 480, 32, 0, 0, 0, 0);
        if (dst) {
            MI_GFX_WaitAllDone(TRUE, 0);
            MI_SYS_MemcpyPa(dst->pixelsPa,
                            finfo.smem_start + 640 * vinfo.yoffset * 4,
                            640 * 480 * 4);
            RotateSurfaceNEON(dst->pixels);
        }
    }
    return dst;
}

//
//	GFX UpdateRect / in place of SDL_UpdateRect
//		Flip after setting the update area
//		*Note* blit from entire screen(or clip_rect if specified) to
// framebuffer(or sHWsurface) rect
//
void GFX_UpdateRectExec(SDL_Surface *screen, int x, int y, int w, int h,
                        uint32_t flags)
{
    if ((fd_fb) && (screen) && (screen->pixelsPa)) {
        if (x | y | w | h) {
            if (!sHWsurface) {
                MI_GFX_Rect_t DstRectPush = stDstRect;
                // for rotate180
                stDstRect.s32Xpos = 640 - (x + w);
                stDstRect.s32Ypos = 480 - (y + h);
                // for RetroArch rotate function
                stDstRect.u32Width = (stOpt.eRotate & 1) ? h : w;
                stDstRect.u32Height = (stOpt.eRotate & 1) ? w : h;
                GFX_FlipExec(screen, flags);
                stDstRect = DstRectPush;
            }
            else if (screen != sHWsurface) {
                // direct draw mode, dest = sHWsurface
                MI_GFX_Rect_t DstRectPush = sHWRect;
                sHWRect.s32Xpos = x;
                sHWRect.s32Ypos = y;
                sHWRect.u32Width = w;
                sHWRect.u32Height = h;
                GFX_FlipExec(screen, flags);
                sHWRect = DstRectPush;
            }
        }
        else {
            GFX_FlipExec(screen, flags);
        }
    }
}
void GFX_UpdateRect(SDL_Surface *screen, int x, int y, int w, int h)
{
    GFX_UpdateRectExec(screen, x, y, w, h, flipFlags);
}
void GFX_UpdateRectNoWait(SDL_Surface *screen, int x, int y, int w, int h)
{
    GFX_UpdateRectExec(screen, x, y, w, h, flipFlags & ~GFX_FLIPWAIT);
}
void GFX_UpdateRectWait(SDL_Surface *screen, int x, int y, int w, int h)
{
    GFX_UpdateRectExec(screen, x, y, w, h, flipFlags | GFX_FLIPWAIT);
}
void GFX_UpdateRectForce(SDL_Surface *screen, int x, int y, int w, int h)
{
    GFX_UpdateRectExec(screen, x, y, w, h, flipFlags | GFX_BLOCKING);
}

//
//	Check Rect Overflow for FillRect/BlitSurfaceSYS
//
SDL_Rect *CheckRect(SDL_Surface *dst, SDL_Rect *dstrect)
{
    if ((!dst) || (!dstrect))
        return NULL;

    int w = dstrect->w;
    int h = dstrect->h;
    if (dst->clip_rect.w | dst->clip_rect.h) {
        if (dst->clip_rect.x > dstrect->x) {
            w -= (dst->clip_rect.x - dstrect->x);
            dstrect->x = dst->clip_rect.x;
        }
        if ((dst->clip_rect.x + dst->clip_rect.w) < (dstrect->x + w)) {
            w = dst->clip_rect.w - (dstrect->x - dst->clip_rect.x);
        }
        if (dst->clip_rect.y > dstrect->y) {
            h -= (dst->clip_rect.y - dstrect->y);
            dstrect->y = dst->clip_rect.y;
        }
        if ((dst->clip_rect.y + dst->clip_rect.h) < (dstrect->y + h)) {
            h = dst->clip_rect.h - (dstrect->y - dst->clip_rect.y);
        }
    }
    if (dstrect->x < 0) {
        w += dstrect->x;
        dstrect->x = 0;
    }
    if (dstrect->y < 0) {
        h += dstrect->y;
        dstrect->y = 0;
    }
    if ((dstrect->x + w) > dst->w) {
        w = dst->w - dstrect->x;
    }
    if ((dstrect->y + h) > dst->h) {
        h = dst->h - dstrect->y;
    }
    if ((w <= 0) || (h <= 0) || (dstrect->x >= dst->w) ||
        (dstrect->y >= dst->h))
        return NULL;
    dstrect->w = w;
    dstrect->h = h;
    return dstrect;
}

//
//	GFX FillRect (MI_SYS ver) / in place of SDL_FillRect
//		*Note* color : in case of RGB565 : 2 pixel color values used
// alternately
//
void GFX_FillRectSYS(SDL_Surface *dst, SDL_Rect *dstrect, uint32_t color)
{
    if ((dst) && (dst->pixelsPa)) {
        SDL_Rect dstrect_tmp;
        if (!dstrect) {
            dstrect_tmp.x = 0;
            dstrect_tmp.y = 0;
            dstrect_tmp.w = dst->w;
            dstrect_tmp.h = dst->h;
        }
        else
            memcpy(&dstrect_tmp, dstrect, sizeof(dstrect_tmp));
        if (!(CheckRect(dst, &dstrect_tmp)))
            return;

        MI_SYS_FrameData_t Buf;
        MI_SYS_WindowRect_t Rect;

        Buf.phyAddr[0] = dst->pixelsPa;
        Buf.u16Width = dst->w;
        Buf.u16Height = dst->h;
        Buf.u32Stride[0] = dst->pitch;
        Buf.ePixelFormat = SYS_PixelFormat(dst);
        Rect.u16X = dstrect_tmp.x;
        Rect.u16Y = dstrect_tmp.y;
        Rect.u16Width = dstrect_tmp.w;
        Rect.u16Height = dstrect_tmp.h;

        FlushCacheNeeded(dst->pixels, dst->pitch, Rect.u16Y, Rect.u16Height);
        MI_SYS_BufFillPa(&Buf, color, &Rect);
    }
    else
        SDL_FillRect(dst, dstrect, color);
}

//
//	GFX FillRect (MI_GFX ver) / in place of SDL_FillRect
//		*Note* color : in case of RGB565 : ARGB8888 color value
//		nowait : 0 = wait until done / 1 = no wait
//
void GFX_FillRectExec(SDL_Surface *dst, SDL_Rect *dstrect, uint32_t color,
                      uint32_t nowait)
{
    if ((dst) && (dst->pixelsPa)) {
        SDL_Rect dstrect_tmp;
        if (!dstrect) {
            dstrect_tmp.x = 0;
            dstrect_tmp.y = 0;
            dstrect_tmp.w = dst->w;
            dstrect_tmp.h = dst->h;
        }
        else
            memcpy(&dstrect_tmp, dstrect, sizeof(dstrect_tmp));
        if (!(CheckRect(dst, &dstrect_tmp)))
            return;

        MI_GFX_Surface_t Dst;
        MI_GFX_Rect_t DstRect;
        MI_U16 Fence;

        Dst.phyAddr = dst->pixelsPa;
        Dst.eColorFmt = GFX_ColorFmt(dst);
        Dst.u32Width = dst->w;
        Dst.u32Height = dst->h;
        Dst.u32Stride = dst->pitch;
        DstRect.s32Xpos = dstrect_tmp.x;
        DstRect.s32Ypos = dstrect_tmp.y;
        DstRect.u32Width = dstrect_tmp.w;
        DstRect.u32Height = dstrect_tmp.h;

        FlushCacheNeeded(dst->pixels, dst->pitch, DstRect.s32Ypos,
                         DstRect.u32Height);
        MI_GFX_QuickFill(&Dst, &DstRect, color, &Fence);
        if (!nowait)
            MI_GFX_WaitAllDone(FALSE, Fence);
    }
    else
        SDL_FillRect(dst, dstrect, color);
}
void GFX_FillRect(SDL_Surface *dst, SDL_Rect *dstrect, uint32_t color)
{
    GFX_FillRectExec(dst, dstrect, color, 0);
}
void GFX_FillRectNoWait(SDL_Surface *dst, SDL_Rect *dstrect, uint32_t color)
{
    GFX_FillRectExec(dst, dstrect, color, 1);
}

//
//	GFX_WaitAllDone / wait all done for No Wait functions
//
void GFX_WaitAllDone(void) { MI_GFX_WaitAllDone(TRUE, 0); }

//
//	GFX BlitSurface (MI_SYS ver) / in place of SDL_BlitSurface
//		*Note* Just a copy (no convert scale/bpp)
//
void GFX_BlitSurfaceSYS(SDL_Surface *src, SDL_Rect *srcrect, SDL_Surface *dst,
                        SDL_Rect *dstrect)
{
    if ((src) && (dst) && (src->pixelsPa) && (dst->pixelsPa)) {
        MI_SYS_FrameData_t SrcBuf;
        MI_SYS_FrameData_t DstBuf;
        MI_SYS_WindowRect_t SrcRect;
        MI_SYS_WindowRect_t DstRect;

        SDL_Rect srcrect_tmp, dstrect_tmp;
        if (!srcrect) {
            srcrect_tmp.x = 0;
            srcrect_tmp.y = 0;
            srcrect_tmp.w = src->w;
            srcrect_tmp.h = src->h;
        }
        else
            memcpy(&srcrect_tmp, srcrect, sizeof(srcrect_tmp));
        if (!dstrect) {
            dstrect_tmp.x = 0;
            dstrect_tmp.y = 0;
            dstrect_tmp.w = dst->w;
            dstrect_tmp.h = dst->h;
        }
        else {
            if (dstrect->w | dstrect->h)
                memcpy(&dstrect_tmp, dstrect, sizeof(dstrect_tmp));
            else {
                dstrect_tmp.x = dstrect->x;
                dstrect_tmp.y = dstrect->y;
                dstrect_tmp.w = src->w - dstrect->x;
                dstrect_tmp.h = src->h - dstrect->y;
            }
        }
        // adjust rects from dst surface size and clip_rect
        if (!(CheckRect(dst, &srcrect_tmp)))
            return;
        if (!(CheckRect(dst, &dstrect_tmp)))
            return;

        memset(&SrcBuf, 0, sizeof(SrcBuf));
        SrcBuf.phyAddr[0] = src->pixelsPa;
        SrcBuf.u16Width = src->w;
        SrcBuf.u16Height = src->h;
        SrcBuf.u32Stride[0] = src->pitch;
        SrcBuf.ePixelFormat = SYS_PixelFormat(src);
        SrcRect.u16X = srcrect_tmp.x;
        SrcRect.u16Y = srcrect_tmp.y;
        SrcRect.u16Width = srcrect_tmp.w;
        SrcRect.u16Height = srcrect_tmp.h;

        memset(&DstBuf, 0, sizeof(DstBuf));
        DstBuf.phyAddr[0] = dst->pixelsPa;
        DstBuf.u16Width = dst->w;
        // **HACK** rect.h is not working properly for some reason, so adjust
        // dst height
        DstBuf.u16Height = dstrect_tmp.y + dstrect_tmp.h; // dst->h;
        DstBuf.u32Stride[0] = dst->pitch;
        DstBuf.ePixelFormat = SYS_PixelFormat(dst);
        DstRect.u16X = dstrect_tmp.x;
        DstRect.u16Y = dstrect_tmp.y;
        DstRect.u16Width = dstrect_tmp.w;
        DstRect.u16Height = dstrect_tmp.h;

        FlushCacheNeeded(src->pixels, src->pitch, SrcRect.u16Y,
                         SrcRect.u16Height);
        FlushCacheNeeded(dst->pixels, dst->pitch, DstRect.u16Y,
                         DstRect.u16Height);
        MI_SYS_BufBlitPa(&DstBuf, &DstRect, &SrcBuf, &SrcRect);
    }
    else
        SDL_BlitSurface(src, srcrect, dst, dstrect);
}

//
//	GFX BlitSurface (MI_GFX ver) / in place of SDL_BlitSurface
//		with scale/bpp convert and rotate/mirror
//		rotate : 1 = 90 / 2 = 180 / 3 = 270
//		mirror : 1 = Horizontal / 2 = Vertical / 3 = Both
//		nowait : 0 = wait until done / 1 = no wait
//
void GFX_BlitSurfaceExec(SDL_Surface *src, SDL_Rect *srcrect, SDL_Surface *dst,
                         SDL_Rect *dstrect, uint32_t rotate, uint32_t mirror,
                         uint32_t nowait)
{
    if ((src) && (dst) && (src->pixelsPa) && (dst->pixelsPa)) {
        MI_GFX_Surface_t Src;
        MI_GFX_Surface_t Dst;
        MI_GFX_Rect_t SrcRect;
        MI_GFX_Rect_t DstRect;
        MI_GFX_Opt_t Opt;
        MI_U16 Fence;

        memset(&Opt, 0, sizeof(Opt));
        Opt.eSrcDfbBldOp = E_MI_GFX_DFB_BLD_ONE;
        Opt.eRotate = (MI_GFX_Rotate_e)rotate;
        Opt.eMirror = (MI_GFX_Mirror_e)mirror;
        Opt.stClipRect.s32Xpos = dst->clip_rect.x;
        Opt.stClipRect.s32Ypos = dst->clip_rect.y;
        Opt.stClipRect.u32Width = dst->clip_rect.w;
        Opt.stClipRect.u32Height = dst->clip_rect.h;

        Src.phyAddr = src->pixelsPa;
        Src.u32Width = src->w;
        Src.u32Height = src->h;
        Src.u32Stride = src->pitch;
        Src.eColorFmt = GFX_ColorFmt(src);
        if (srcrect) {
            SrcRect.s32Xpos = srcrect->x;
            SrcRect.s32Ypos = srcrect->y;
            SrcRect.u32Width = srcrect->w;
            SrcRect.u32Height = srcrect->h;
        }
        else {
            SrcRect.s32Xpos = 0;
            SrcRect.s32Ypos = 0;
            SrcRect.u32Width = Src.u32Width;
            SrcRect.u32Height = Src.u32Height;
        }

        Dst.phyAddr = dst->pixelsPa;
        Dst.u32Width = dst->w;
        Dst.u32Height = dst->h;
        Dst.u32Stride = dst->pitch;
        Dst.eColorFmt = GFX_ColorFmt(dst);
        if (dstrect) {
            DstRect.s32Xpos = dstrect->x;
            DstRect.s32Ypos = dstrect->y;
            if (dstrect->w | dstrect->h) {
                DstRect.u32Width = dstrect->w;
                DstRect.u32Height = dstrect->h;
            }
            else {
                DstRect.u32Width = SrcRect.u32Width;
                DstRect.u32Height = SrcRect.u32Height;
            }
        }
        else {
            DstRect.s32Xpos = 0;
            DstRect.s32Ypos = 0;
            DstRect.u32Width = Dst.u32Width;
            DstRect.u32Height = Dst.u32Height;
        }

        FlushCacheNeeded(src->pixels, src->pitch, SrcRect.s32Ypos,
                         SrcRect.u32Height);
        if (rotate & 1)
            FlushCacheNeeded(dst->pixels, dst->pitch, DstRect.s32Ypos,
                             DstRect.u32Width);
        else
            FlushCacheNeeded(dst->pixels, dst->pitch, DstRect.s32Ypos,
                             DstRect.u32Height);
        MI_GFX_BitBlit(&Src, &SrcRect, &Dst, &DstRect, &Opt, &Fence);
        if (!nowait)
            MI_GFX_WaitAllDone(FALSE, Fence);
    }
    else
        SDL_BlitSurface(src, srcrect, dst, dstrect);
}
void GFX_BlitSurface(SDL_Surface *src, SDL_Rect *srcrect, SDL_Surface *dst,
                     SDL_Rect *dstrect)
{
    GFX_BlitSurfaceExec(src, srcrect, dst, dstrect, 0, 0, 0);
}
void GFX_BlitSurfaceNoWait(SDL_Surface *src, SDL_Rect *srcrect,
                           SDL_Surface *dst, SDL_Rect *dstrect)
{
    GFX_BlitSurfaceExec(src, srcrect, dst, dstrect, 0, 0, 1);
}
void GFX_BlitSurfaceRotate(SDL_Surface *src, SDL_Rect *srcrect,
                           SDL_Surface *dst, SDL_Rect *dstrect, uint32_t rotate)
{
    GFX_BlitSurfaceExec(src, srcrect, dst, dstrect, rotate, 0, 0);
}
void GFX_BlitSurfaceRotateNoWait(SDL_Surface *src, SDL_Rect *srcrect,
                                 SDL_Surface *dst, SDL_Rect *dstrect,
                                 uint32_t rotate)
{
    GFX_BlitSurfaceExec(src, srcrect, dst, dstrect, rotate, 0, 1);
}
void GFX_BlitSurfaceMirror(SDL_Surface *src, SDL_Rect *srcrect,
                           SDL_Surface *dst, SDL_Rect *dstrect, uint32_t mirror)
{
    GFX_BlitSurfaceExec(src, srcrect, dst, dstrect, 0, mirror, 0);
}
void GFX_BlitSurfaceMirrorNoWait(SDL_Surface *src, SDL_Rect *srcrect,
                                 SDL_Surface *dst, SDL_Rect *dstrect,
                                 uint32_t mirror)
{
    GFX_BlitSurfaceExec(src, srcrect, dst, dstrect, 0, mirror, 1);
}
// TODO: add Alpha blend / Colorkey blit
