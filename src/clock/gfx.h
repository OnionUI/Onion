#ifndef __GFX_H__
#define __GFX_H__

#include <stdint.h>
#include <linux/fb.h>
#include <SDL/SDL.h>

#define	pixelsPa	unused1

//	framebuffer file descriptor
extern	int	fd_fb;

//	framebuffer mmapped address
extern	void*	fb_addr;

//	framebuffer information structure
extern	struct	fb_fix_screeninfo	finfo;
extern	struct	fb_var_screeninfo	vinfo;

//	enum for Flipflags, default value is defined in gfx.c
//	GFX_BLOCKING	: limit to 60fps but never skips frames
//			:  in case of clearing all buffers by GFX_Flip()x3, needs to use BLOCKING (or GFX_FlipForce())
//	GFX_FLIPWAIT	: wait until Blit is done when flip
//			:  when NOWAIT, do not clear/write source surface immediately after Flip
//			:  if absolutely necessary, use GFX_WaitAllDone() before write (or GFX_FlipWait())
enum { GFX_BLOCKING = 1, GFX_FLIPWAIT = 2 };

//	GFX Init	Init, call after SDL_Init()
void		GFX_Init(void);

//	GFX Quit	Deinit, call before SDL_Quit()
void		GFX_Quit(void);

//	SetVideomode / in place of SDL_SetVideoMode
//		if flags == SDL_HWSURFACE & non SDL_DOUBLEBUF, change to direct draw mode (no Flip needed)
//		 otherwise, same as GFX_CreateRGBSurface
SDL_Surface*	GFX_SetVideoMode(int width, int height, int bpp, uint32_t flags);

//	Create GFX Surface / in place of SDL_CreateRGBSurface
//		supports 16/32bpp only / flags has no meaning, fixed to SWSURFACE
SDL_Surface*	GFX_CreateRGBSurface(uint32_t flags, int width, int height, int depth, uint32_t Rmask, uint32_t Gmask, uint32_t Bmask, uint32_t Amask);

//	Free GFX Surface / in place of SDL_FreeSurface
void		GFX_FreeSurface(SDL_Surface *surface);

//	Clear GFX/SDL Surface (entire)
void		GFX_ClearSurface(SDL_Surface *surface);

//	Copy GFX/SDL Surface (entire)
//		src/dst surfaces must be the same size
void		GFX_CopySurface(SDL_Surface *src, SDL_Surface *dst);

//	Duplicate GFX Surface from SDL_Surface or FB
//		if src is NULL, duplicate from FB currently displayed
SDL_Surface*	GFX_DuplicateSurface(SDL_Surface *surface);

//	GFX Flip / in place of SDL_Flip
//		with Rotate180/bppConvert/Scaling
//		*Note* blit from entire surface(or clip_rect if specified) to entire framebuffer
void		GFX_Flip(SDL_Surface *surface);
//		with GFX_FLIPWAIT
void		GFX_FlipWait(SDL_Surface *surface);
//		without GFX_FLIPWAIT
void		GFX_FlipNoWait(SDL_Surface *surface);
//		with GFX_BLOCKING
void		GFX_FlipForce(SDL_Surface *surface);

//	GFX UpdateRect / in place of SDL_UpdateRect
//		Flip after setting the update area
//		*Note* blit from entire screen(or clip_rect if specified) to framebuffer rect
void		GFX_UpdateRect(SDL_Surface *screen, int x, int y, int w, int h);
//		followings are same as GFX_Flip
void		GFX_UpdateRectWait(SDL_Surface *screen, int x, int y, int w, int h);
void		GFX_UpdateRectNoWait(SDL_Surface *screen, int x, int y, int w, int h);
void		GFX_UpdateRectForce(SDL_Surface *screen, int x, int y, int w, int h);

//	Get/Set Flipflags
//		GFX_BLOCKING/GFX_FLIPWAIT flags used for GFX_Flip/UpdateRect
uint32_t	GFX_GetFlipFlags(void);
void		GFX_SetFlipFlags(uint32_t flags);

//	Get/Set Flip callback, for use directly drawing to framebuffer
//		(Battery icon, RetroArch OSD text, etc)
//		*Note* framebuffer is rotated 180 degrees
void*		GFX_GetFlipCallback(void);
void		GFX_SetFlipCallback(void (*callback)(void*), void *userdata);

//	Clear entire FrameBuffer
void		GFX_ClearFrameBuffer(void);

//	GFX_WaitAllDone / wait all done for following NoWait functions
void		GFX_WaitAllDone(void);

//	GFX FillRect (MI_SYS ver) / in place of SDL_FillRect
//		*Note* color : in case of RGB565 : 2 pixel color values used alternately
void		GFX_FillRectSYS(SDL_Surface* dst, SDL_Rect* dstrect, uint32_t color);

//	GFX FillRect (MI_GFX ver) / in place of SDL_FillRect
//		*Note* color : in case of RGB565 : ARGB8888 color value
void		GFX_FillRect(SDL_Surface* dst, SDL_Rect* dstrect, uint32_t color);
//		NoWait: does not wait for completion
void		GFX_FillRectNoWait(SDL_Surface* dst, SDL_Rect* dstrect, uint32_t color);

//	GFX BlitSurface (MI_SYS ver) / in place of SDL_BlitSurface
//		*Note* Just a copy (no convert scale/bpp)
void		GFX_BlitSurfaceSYS(SDL_Surface *src, SDL_Rect *srcrect, SDL_Surface *dst, SDL_Rect *dstrect);

//	GFX BlitSurface (MI_GFX ver) / in place of SDL_BlitSurface
//		with scale/bpp convert
void		GFX_BlitSurface(SDL_Surface *src, SDL_Rect *srcrect, SDL_Surface *dst, SDL_Rect *dstrect);
//		with rotate : 1 = 90 / 2 = 180 / 3 = 270
void		GFX_BlitSurfaceRotate(SDL_Surface *src, SDL_Rect *srcrect, SDL_Surface *dst, SDL_Rect *dstrect, uint32_t rotate);
//		with mirror : 1 = Horizontal / 2 = Vertical / 3 = Both
void		GFX_BlitSurfaceMirror(SDL_Surface *src, SDL_Rect *srcrect, SDL_Surface *dst, SDL_Rect *dstrect, uint32_t mirror);
//		NoWait: does not wait for completion
void		GFX_BlitSurfaceNoWait(SDL_Surface *src, SDL_Rect *srcrect, SDL_Surface *dst, SDL_Rect *dstrect);
void		GFX_BlitSurfaceRotateNoWait(SDL_Surface *src, SDL_Rect *srcrect, SDL_Surface *dst, SDL_Rect *dstrect, uint32_t rotate);
void		GFX_BlitSurfaceMirrorNoWait(SDL_Surface *src, SDL_Rect *srcrect, SDL_Surface *dst, SDL_Rect *dstrect, uint32_t mirror);

#endif
