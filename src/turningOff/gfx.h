#ifndef __GFX_H__
#define __GFX_H__

#include <stdint.h>
#include <linux/fb.h>
#include <SDL/SDL.h>

#define	pixelsPa	unused1

extern	int				fd_fb;
extern	struct	fb_fix_screeninfo	finfo;
extern	struct	fb_var_screeninfo	vinfo;

void		GFX_Init(void);
void		GFX_Quit(void);
SDL_Surface*	GFX_CreateRGBSurface(uint32_t flags, int width, int height, int depth, uint32_t Rmask, uint32_t Gmask, uint32_t Bmask, uint32_t Amask);
void		GFX_FreeSurface(SDL_Surface *surface);
void		GFX_ClearSurface(SDL_Surface *surface);
void		GFX_CopySurface(SDL_Surface *src, SDL_Surface *dst);
void		GFX_Flip(SDL_Surface *surface);

#endif
