#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdint.h>
#include <linux/fb.h>
#include <sys/ioctl.h>
#include <SDL/SDL.h>
#include <mi_sys.h>
#include <mi_gfx.h>

//#define	DOUBLEBUF
 
int			fd_fb = 0;
struct			fb_fix_screeninfo finfo;
struct			fb_var_screeninfo vinfo;
MI_PHY			fb_phyAddr;
MI_GFX_Surface_t	stSrc;
MI_GFX_Rect_t		stSrcRect;
MI_GFX_Surface_t	stDst;
MI_GFX_Rect_t		stDstRect;
MI_GFX_Opt_t		stOpt;

// 
//	GFX Init
//		Prepare for HW Blit to FB
//
void	GFX_Init(void) {
	if (fd_fb == 0) {
		SDL_SetVideoMode(640, 480, 32, SDL_SWSURFACE);
		MI_SYS_Init();
		MI_GFX_Open();

		fd_fb = open("/dev/fb0", O_RDWR);
		ioctl(fd_fb, FBIOGET_FSCREENINFO, &finfo);
		fb_phyAddr = finfo.smem_start;
		ioctl(fd_fb, FBIOGET_VSCREENINFO, &vinfo);
		vinfo.yoffset = 0;
		ioctl(fd_fb, FBIOPUT_VSCREENINFO, &vinfo);
		MI_SYS_MemsetPa(fb_phyAddr, 0, 640*480*4*2);

		stDst.phyAddr = fb_phyAddr;
		stDst.eColorFmt = E_MI_GFX_FMT_ARGB8888;
		stDst.u32Width = 640;
		stDst.u32Height = 480;
		stDst.u32Stride = 640*4;
		stDstRect.s32Xpos = 0;
		stDstRect.s32Ypos = 0;
		stDstRect.u32Width = 640;
		stDstRect.u32Height = 480;

		memset(&stOpt, 0, sizeof(stOpt));
		stOpt.eSrcDfbBldOp = E_MI_GFX_DFB_BLD_ONE;
		stOpt.eRotate = E_MI_GFX_ROTATE_180;
	}
}

//
//	GFX Quit
//
void	GFX_Quit(void) {
	if (fd_fb) {
//		MI_SYS_MemsetPa(fb_phyAddr, 0, 640*480*4*2);
		vinfo.yoffset = 0;
		ioctl(fd_fb, FBIOPUT_VSCREENINFO, &vinfo);
		close(fd_fb);
		fd_fb = 0;
		MI_GFX_Close();
		MI_SYS_Exit();
	}
}

//
//	Create GFX Surface / almost same as SDL_CreateRGBSurface
//		flags has no meaning, fixed to SWSURFACE
//		Additional return value : surface->unused1 = Physical address of surface
//
#define	pixelsPa	unused1
SDL_Surface*	GFX_CreateRGBSurface(uint32_t flags, int width, int height, int depth, uint32_t Rmask, uint32_t Gmask, uint32_t Bmask, uint32_t Amask) {
	SDL_Surface*	surface;
	MI_PHY		phyAddr;
	void*		virAddr;
	int		pitch = width * uint32_t(depth/8);
	uint32_t	size = pitch * height;

	MI_SYS_MMA_Alloc(NULL, size, &phyAddr);
	MI_SYS_Mmap(phyAddr, size, &virAddr, TRUE);	// write cache ON needs Flush when read Pa directly

	surface = SDL_CreateRGBSurfaceFrom(virAddr,width,height,depth,pitch,Rmask,Gmask,Bmask,Amask);
	if (surface != NULL) surface->pixelsPa = phyAddr;
	return surface;
}

//
//	Free GFX Surface / almost same as SDL_FreeSurface
//
void	GFX_FreeSurface(SDL_Surface *surface) {
	MI_PHY		phyAddr = surface->pixelsPa;
	void*		virAddr = surface->pixels;
	uint32_t	size = surface->pitch * surface->h;

	SDL_FreeSurface(surface);
	if (phyAddr) {
		MI_SYS_Munmap(virAddr, size);
		MI_SYS_MMA_Free(phyAddr);
	}
}

//
//	Clear GFX Surface (entire)
//
void	GFX_ClearSurface(SDL_Surface *surface) {
	MI_SYS_MemsetPa(surface->pixelsPa,0,surface->pitch * surface->h);
}

//
//	Copy GFX Surface (entire)
//		src/dst surfaces must be the same size
//
void	GFX_CopySurface(SDL_Surface *src, SDL_Surface *dst) {
	uint32_t size = src->pitch * src->h;
	if (size == uint32_t(dst->pitch * dst->h)) {
		MI_SYS_FlushInvCache(src->pixels, size);
		MI_SYS_MemcpyPa(dst->pixelsPa,src->pixelsPa,size);
	}
}

//
//	GFX Flip
//		HW Blit : surface -> FB(backbuffer) with Rotate180/bppConvert/Scaling
//		and Flip
//
void	GFX_Flip(SDL_Surface *surface) {
	MI_U16	u16Fence;

	stSrc.phyAddr = surface->pixelsPa;
	if (surface->format->BytesPerPixel == 2) {
		stSrc.eColorFmt = E_MI_GFX_FMT_RGB565;
	} else {
		stSrc.eColorFmt = E_MI_GFX_FMT_ARGB8888;
	}
	stSrc.u32Width = surface->w;
	stSrc.u32Height = surface->h;
	stSrc.u32Stride = surface->pitch;
	stSrcRect.s32Xpos = 0;
	stSrcRect.s32Ypos = 0;
	stSrcRect.u32Width = stSrc.u32Width;
	stSrcRect.u32Height = stSrc.u32Height;
#ifdef	DOUBLEBUF
	vinfo.yoffset ^= 480;
	stDst.phyAddr = fb_phyAddr + (640*vinfo.yoffset*4);
#endif
	MI_SYS_FlushInvCache(surface->pixels, surface->pitch * surface->h);
	MI_GFX_BitBlit(&stSrc,&stSrcRect,&stDst,&stDstRect,&stOpt,&u16Fence);
	MI_GFX_WaitAllDone(FALSE, u16Fence);
#ifdef	DOUBLEBUF
	ioctl(fd_fb, FBIOPAN_DISPLAY, &vinfo);	// TODO: Find a way to make this NONBLOCK
#endif
}

//	TODO: add more GFX functions such as BlitSurface , FillRect
