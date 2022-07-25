/*  RetroArch - A frontend for libretro.
 *  Copyright (C) 2010-2014 - Hans-Kristian Arntzen
 *  Copyright (C) 2011-2017 - Daniel De Matteis
 *  Copyright (C) 2011-2017 - Higor Euripedes
 *  Copyright (C) 2019-2021 - James Leaver
 *  Copyright (C)      2021 - John Parton
 *
 *  RetroArch is free software: you can redistribute it and/or modify it under the terms
 *  of the GNU General Public License as published by the Free Software Found-
 *  ation, either version 3 of the License, or (at your option) any later version.
 *
 *  RetroArch is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
 *  without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 *  PURPOSE.  See the GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along with RetroArch.
 *  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdlib.h>
#include <string.h>

#include <SDL/SDL.h>
#include <SDL/SDL_video.h>

#include <retro_assert.h>
#include <gfx/video_frame.h>
#include <retro_assert.h>
#include <string/stdstring.h>
#include <encodings/utf.h>
#include <features/features_cpu.h>

#include "gfx.c"
#include "scaler_neon.c"
#include <sys/mman.h>
#include <sys/time.h>

#ifdef HAVE_CONFIG_H
#include "../../config.h"
#endif

#ifdef HAVE_MENU
#include "../../menu/menu_driver.h"
#endif

#include "../../dingux/dingux_utils.h"

#include "../../verbosity.h"
#include "../../gfx/drivers_font_renderer/bitmap.h"
#include "../../configuration.h"
#include "../../retroarch.h"

#define likely(x)   __builtin_expect(!!(x), 1)
#define unlikely(x) __builtin_expect(!!(x), 0)

#define SDL_MIYOOMINI_WIDTH  640
#define SDL_MIYOOMINI_HEIGHT 480
#define RGUI_MENU_WIDTH  320
#define RGUI_MENU_HEIGHT 240
#define SDL_NUM_FONT_GLYPHS 256
#define OSD_TEXT_Y_MARGIN 4
#define OSD_TEXT_LINES_MAX 3	/* 1 .. 7 */
#define OSD_TEXT_LINE_LEN ((uint32_t)(RGUI_MENU_WIDTH / FONT_WIDTH_STRIDE)-1)
#define OSD_TEXT_LEN_MAX (OSD_TEXT_LINE_LEN * OSD_TEXT_LINES_MAX)

typedef struct sdl_miyoomini_video sdl_miyoomini_video_t;
struct sdl_miyoomini_video
{
   SDL_Surface *screen;
   void (*scale_func)(void* data, void* __restrict src, void* __restrict dst, uint32_t sw, uint32_t sh, uint32_t sp, uint32_t dp);
   /* Scaling/padding/cropping parameters */
   unsigned content_width;
   unsigned content_height;
   unsigned frame_width;
   unsigned frame_height;
   unsigned video_x;
   unsigned video_y;
   unsigned video_w;
   unsigned video_h;
   unsigned rotate;
   bool rgb32;
   bool menu_active;
   bool was_in_menu;
   retro_time_t last_frame_time;
   retro_time_t ff_frame_time_min;
   enum dingux_ipu_filter_type filter_type;
   bool vsync;
   bool keep_aspect;
   bool scale_integer;
   bool quitting;
   bitmapfont_lut_t *osd_font;
   uint32_t font_colour32;
   SDL_Surface *menuscreen;
   SDL_Surface *menuscreen_rgui;
   unsigned msg_count;
   char msg_tmp[OSD_TEXT_LEN_MAX];
};

/* Clear OSD text area, without video_rect, rotate180 */
static void sdl_miyoomini_clear_msgarea(void* buf, unsigned x, unsigned y, unsigned w, unsigned h, unsigned lines) {
   if ( ( x == 0 ) && ( w == SDL_MIYOOMINI_WIDTH  ) && ( y == 0 ) && ( h == SDL_MIYOOMINI_HEIGHT ) ) return;

   uint32_t x0 = SDL_MIYOOMINI_WIDTH - (x + w); /* left margin , right margin = x */
   uint32_t y0 = SDL_MIYOOMINI_HEIGHT - (y + h); /* top margin , bottom margin = y */
   uint32_t sl = x0 * sizeof(uint32_t); /* left buffer size */
   uint32_t sr = x * sizeof(uint32_t); /* right buffer size */
   uint32_t sw = w * sizeof(uint32_t); /* pitch */
   uint32_t ss = SDL_MIYOOMINI_WIDTH * sizeof(uint32_t); /* stride */
   uint32_t vy = OSD_TEXT_Y_MARGIN + 2; /* clear start y offset */
   uint32_t vh = FONT_HEIGHT_STRIDE * 2 * lines - 2; /* clear height */
   uint32_t vh1 = (y0 < vy) ? 0 : (y0 - vy); if (vh1 > vh) vh1 = vh;
   uint32_t vh2 = vh - vh1;
   uint32_t ssl = ss * vh1 + sl;
   uint32_t srl = sr + sl;
   void* ofs = buf + vy * ss;

   if (ssl) memset(ofs, 0, ssl);
   ofs += ssl + sw;
   for (; vh2>1; vh2--, ofs += ss) { if (srl) memset(ofs, 0, srl); }
   if ((vh2) && (sr)) memset(ofs, 0, sr);
}

/* Print OSD text, flip callback, direct draw to framebuffer, 32bpp, 2x, rotate180 */
static void sdl_miyoomini_print_msg(void* data) {
   if (unlikely(!data)) return;
   sdl_miyoomini_video_t *vid = (sdl_miyoomini_video_t*)data;

   void *screen_buf;
   const char *str  = vid->msg_tmp;
   uint32_t str_len = strlen_size(str, OSD_TEXT_LEN_MAX);
   if (str_len) {
      screen_buf              = fb_addr + (vinfo.yoffset * SDL_MIYOOMINI_WIDTH * sizeof(uint32_t));
      bool **font_lut         = vid->osd_font->lut;
      uint32_t str_lines      = (uint32_t)((str_len - 1) / OSD_TEXT_LINE_LEN) + 1;
      uint32_t str_counter    = OSD_TEXT_LINE_LEN;
      const int x_pos_def     = SDL_MIYOOMINI_WIDTH - (FONT_WIDTH_STRIDE * 2);
      int x_pos               = x_pos_def;
      int y_pos               = OSD_TEXT_Y_MARGIN - 4 + (FONT_HEIGHT_STRIDE * 2 * str_lines);

      for (; str_len > 0; str_len--) {
         /* Check for out of bounds x coordinates */
         if (!str_counter--) {
            x_pos = x_pos_def; y_pos -= (FONT_HEIGHT_STRIDE * 2); str_counter = OSD_TEXT_LINE_LEN;
         }
         /* Deal with spaces first, for efficiency */
         if (*str == ' ') str++;
         else {
            uint32_t i, j;
            bool *symbol_lut;
            uint32_t symbol = utf8_walk(&str);

            /* Stupid hack: 'oe' ligatures are not really
             * standard extended ASCII, so we have to waste
             * CPU cycles performing a conversion from the
             * unicode values... */
            if (symbol == 339) /* Latin small ligature oe */
               symbol = 156;
            if (symbol == 338) /* Latin capital ligature oe */
               symbol = 140;

            if (symbol >= SDL_NUM_FONT_GLYPHS) continue;

            symbol_lut = font_lut[symbol];

            for (j = 0; j < FONT_HEIGHT; j++) {
               uint32_t buff_offset = ((y_pos - (j * 2) ) * SDL_MIYOOMINI_WIDTH) + x_pos;

               for (i = 0; i < FONT_WIDTH; i++) {
                  if (*(symbol_lut + i + (j * FONT_WIDTH))) {
                     uint32_t *screen_buf_ptr = (uint32_t*)screen_buf + buff_offset - (i * 2);

                     /* Bottom shadow (1) */
                     screen_buf_ptr[+0] = 0;
                     screen_buf_ptr[+1] = 0;
                     screen_buf_ptr[+2] = 0;
                     screen_buf_ptr[+3] = 0;

                     /* Bottom shadow (2) */
                     screen_buf_ptr[SDL_MIYOOMINI_WIDTH+0] = 0;
                     screen_buf_ptr[SDL_MIYOOMINI_WIDTH+1] = 0;
                     screen_buf_ptr[SDL_MIYOOMINI_WIDTH+2] = 0;
                     screen_buf_ptr[SDL_MIYOOMINI_WIDTH+3] = 0;

                     /* Text pixel + right shadow (1) */
                     screen_buf_ptr[(SDL_MIYOOMINI_WIDTH*2)+0] = 0;
                     screen_buf_ptr[(SDL_MIYOOMINI_WIDTH*2)+1] = 0;
                     screen_buf_ptr[(SDL_MIYOOMINI_WIDTH*2)+2] = vid->font_colour32;
                     screen_buf_ptr[(SDL_MIYOOMINI_WIDTH*2)+3] = vid->font_colour32;

                     /* Text pixel + right shadow (2) */
                     screen_buf_ptr[(SDL_MIYOOMINI_WIDTH*3)+0] = 0;
                     screen_buf_ptr[(SDL_MIYOOMINI_WIDTH*3)+1] = 0;
                     screen_buf_ptr[(SDL_MIYOOMINI_WIDTH*3)+2] = vid->font_colour32;
                     screen_buf_ptr[(SDL_MIYOOMINI_WIDTH*3)+3] = vid->font_colour32;
                  }
               }
            }
         }
         x_pos -= FONT_WIDTH_STRIDE * 2;
      }
      vid->msg_count |= (str_lines << 6);
   }
   if (vid->msg_count & 7) {
      /* clear recent OSD text */
      screen_buf = fb_addr;
      uint32_t target_offset = vinfo.yoffset + SDL_MIYOOMINI_HEIGHT;
      if (target_offset != SDL_MIYOOMINI_HEIGHT * 3) screen_buf += target_offset * SDL_MIYOOMINI_WIDTH * sizeof(uint32_t);
      sdl_miyoomini_clear_msgarea(screen_buf, vid->video_x, vid->video_y, vid->video_w, vid->video_h, vid->msg_count & 7);
   }
   vid->msg_count >>= 3;
}

/* Nearest neighbor scalers */
#define NN_SHIFT 16
void scalenn_16(void* data, void* __restrict src, void* __restrict dst, uint32_t sw, uint32_t sh, uint32_t sp, uint32_t dp) {
   if (unlikely(!data||!sw||!sh)) return;
   sdl_miyoomini_video_t *vid = (sdl_miyoomini_video_t*)data;
   uint32_t dw = vid->video_w;
   uint32_t dh = vid->video_h;

   uint32_t x_step = (sw << NN_SHIFT) / dw + 1;
   uint32_t y_step = (sh << NN_SHIFT) / dh + 1;

   uint32_t in_stride  = sp >> 1;
   uint32_t out_stride = dp >> 1;

   uint16_t* in_ptr  = (uint16_t*)src;
   uint16_t* out_ptr = (uint16_t*)dst;

   uint32_t oy = 0;
   uint32_t y  = 0;

   /* Reading 16bits takes a little time,
      so try not to read as much as possible in the case of 16bpp */
   if (dh > sh) {
      if (dw > sw) {
         do {
            uint32_t col = dw;
            uint32_t ox  = 0;
            uint32_t x   = 0;

            uint16_t* optrtmp1 = out_ptr;

            uint16_t pix = in_ptr[0];
            do {
               uint32_t tx = x >> NN_SHIFT;
               if (tx != ox) {
                  pix = in_ptr[tx];
                  ox  = tx;
               }
               *(out_ptr++) = pix;
               x           += x_step;
            } while (--col);

            y += y_step;
            uint32_t ty = y >> NN_SHIFT;
            uint16_t* optrtmp2 = optrtmp1;
            for(; ty == oy; y += y_step, ty = y >> NN_SHIFT) {
               if (!--dh) return;
               optrtmp2 += out_stride;
               memcpy(optrtmp2, optrtmp1, dw << 1);
            }
            in_ptr += (ty - oy) * in_stride;
            out_ptr = optrtmp2 + out_stride;
            oy      = ty;
         } while (--dh);
      } else {
         do {
            uint32_t col = dw;
            uint32_t x   = 0;

            uint16_t* optrtmp1 = out_ptr;

            do {
               *(out_ptr++) = in_ptr[x >> NN_SHIFT];
               x           += x_step;
            } while (--col);

            y += y_step;
            uint32_t ty = y >> NN_SHIFT;
            uint16_t* optrtmp2 = optrtmp1;
            for(; ty == oy; y += y_step, ty = y >> NN_SHIFT) {
               if (!--dh) return;
               optrtmp2 += out_stride;
               memcpy(optrtmp2, optrtmp1, dw << 1);
            }
            in_ptr += (ty - oy) * in_stride;
            out_ptr = optrtmp2 + out_stride;
            oy      = ty;
         } while (--dh);
      }
   } else {
      if (dw > sw) {
         do {
            uint32_t col = dw;
            uint32_t ox  = 0;
            uint32_t x   = 0;

            uint16_t* optrtmp1 = out_ptr;

            uint16_t pix = in_ptr[0];
            do {
               uint32_t tx = x >> NN_SHIFT;
               if (tx != ox) {
                  pix = in_ptr[tx];
                  ox  = tx;
               }
               *(out_ptr++) = pix;
               x           += x_step;
            } while (--col);

            y += y_step;
            uint32_t ty = y >> NN_SHIFT;
            in_ptr += (ty - oy) * in_stride;
            out_ptr = optrtmp1 + out_stride;
            oy      = ty;
         } while (--dh);
      } else {
         do {
            uint32_t col = dw;
            uint32_t x   = 0;

            uint16_t* optrtmp1 = out_ptr;

            do {
               *(out_ptr++) = in_ptr[x >> NN_SHIFT];
               x           += x_step;
            } while (--col);

            y += y_step;
            uint32_t ty = y >> NN_SHIFT;
            in_ptr += (ty - oy) * in_stride;
            out_ptr = optrtmp1 + out_stride;
            oy      = ty;
         } while (--dh);
      }
   }
}

void scalenn_32(void* data, void* __restrict src, void* __restrict dst, uint32_t sw, uint32_t sh, uint32_t sp, uint32_t dp) {
   if (unlikely(!data||!sw||!sh)) return;
   sdl_miyoomini_video_t *vid = (sdl_miyoomini_video_t*)data;
   uint32_t dw = vid->video_w;
   uint32_t dh = vid->video_h;

   uint32_t x_step = (sw << NN_SHIFT) / dw + 1;
   uint32_t y_step = (sh << NN_SHIFT) / dh + 1;

   uint32_t in_stride  = sp >> 2;
   uint32_t out_stride = dp >> 2;

   uint32_t* in_ptr  = (uint32_t*)src;
   uint32_t* out_ptr = (uint32_t*)dst;

   uint32_t oy = 0;
   uint32_t y  = 0;

   /* Reading 32bit is fast when cached,
      so the x-axis is not considered in the case of 32bpp */
   if (dh > sh) {
      do {
         uint32_t col = dw;
         uint32_t x   = 0;

         uint32_t* optrtmp1 = out_ptr;

         do {
            *(out_ptr++) = in_ptr[x >> NN_SHIFT];
            x           += x_step;
         } while (--col);

         y += y_step;
         uint32_t ty = y >> NN_SHIFT;
         uint32_t* optrtmp2 = optrtmp1;
         for(; ty == oy; y += y_step, ty = y >> NN_SHIFT) {
            if (!--dh) return;
            optrtmp2 += out_stride;
            memcpy(optrtmp2, optrtmp1, dw << 2);
         }
         in_ptr += (ty - oy) * in_stride;
         out_ptr = optrtmp2 + out_stride;
         oy      = ty;
      } while (--dh);
   } else {
      do {
         uint32_t col = dw;
         uint32_t x   = 0;

         uint32_t* optrtmp1 = out_ptr;

         do {
            *(out_ptr++) = in_ptr[x >> NN_SHIFT];
            x           += x_step;
         } while (--col);

         y += y_step;
         uint32_t ty = y >> NN_SHIFT;
         in_ptr += (ty - oy) * in_stride;
         out_ptr = optrtmp1 + out_stride;
         oy      = ty;
      } while (--dh);
   }
}

/* Bridge to NEON scalers in scaler_neon.c */
void scale1x_16(void* data, void* __restrict src, void* __restrict dst, uint32_t sw, uint32_t sh, uint32_t sp, uint32_t dp) {
     scale1x_n16(src, dst, sw, sh, sp, dp); }
void scale1x_32(void* data, void* __restrict src, void* __restrict dst, uint32_t sw, uint32_t sh, uint32_t sp, uint32_t dp) {
     scale1x_n32(src, dst, sw, sh, sp, dp); }
void scale2x_16(void* data, void* __restrict src, void* __restrict dst, uint32_t sw, uint32_t sh, uint32_t sp, uint32_t dp) {
     scale2x_n16(src, dst, sw, sh, sp, dp); }
void scale2x_32(void* data, void* __restrict src, void* __restrict dst, uint32_t sw, uint32_t sh, uint32_t sp, uint32_t dp) {
     scale2x_n32(src, dst, sw, sh, sp, dp); }
void scale4x_16(void* data, void* __restrict src, void* __restrict dst, uint32_t sw, uint32_t sh, uint32_t sp, uint32_t dp) {
     scale4x_n16(src, dst, sw, sh, sp, dp); }
void scale4x_32(void* data, void* __restrict src, void* __restrict dst, uint32_t sw, uint32_t sh, uint32_t sp, uint32_t dp) {
     scale4x_n32(src, dst, sw, sh, sp, dp); }

/* Clear border x3 screens for framebuffer (rotate180) */
static void sdl_miyoomini_clear_border(void* buf, unsigned x, unsigned y, unsigned w, unsigned h) {
   if ( (x == 0) && (y == 0) && (w == SDL_MIYOOMINI_WIDTH) && (h == SDL_MIYOOMINI_HEIGHT) ) return;
   if ( (w == 0) || (h == 0) ) { memset(buf, 0, SDL_MIYOOMINI_WIDTH * SDL_MIYOOMINI_HEIGHT * sizeof(uint32_t) * 3); return; }

   uint32_t x0 = SDL_MIYOOMINI_WIDTH - (x + w); /* left margin , right margin = x */
   uint32_t y0 = SDL_MIYOOMINI_HEIGHT - (y + h); /* top margin , bottom margin = y */
   uint32_t sl = x0 * sizeof(uint32_t); /* left buffer size */
   uint32_t sr = x * sizeof(uint32_t); /* right buffer size */
   uint32_t st = y0 * SDL_MIYOOMINI_WIDTH * sizeof(uint32_t); /* top buffer size */
   uint32_t sb = y * SDL_MIYOOMINI_WIDTH * sizeof(uint32_t); /* bottom buffer size */
   uint32_t srl = sr + sl;
   uint32_t stl = st + sl;
   uint32_t srb = sr + sb;
   uint32_t srbtl = srl + sb + st;
   uint32_t sw = w * sizeof(uint32_t); /* pitch */
   uint32_t ss = SDL_MIYOOMINI_WIDTH * sizeof(uint32_t); /* stride */
   uint32_t i;

   if (stl) memset(buf, 0, stl); /* 1st top + 1st left */
   buf += stl + sw;
   for (i=h-1; i>0; i--, buf += ss) {
      if (srl) memset(buf, 0, srl); /* right + left */
   }
   if (srbtl) memset(buf, 0, srbtl); /* last right + bottom + top + 1st left */
   buf += srbtl + sw;
   for (i=h-1; i>0; i--, buf += ss) {
      if (srl) memset(buf, 0, srl); /* right + left */
   }
   if (srbtl) memset(buf, 0, srbtl); /* last right + bottom + top + 1st left */
   buf += srbtl + sw;
   for (i=h-1; i>0; i--, buf += ss) {
      if (srl) memset(buf, 0, srl); /* right + left */
   }
   if (srb) memset(buf, 0, srb); /* last right + last bottom */
}

/* Set CPU governor */
enum cpugov { PERFORMANCE = 0, POWERSAVE = 1, ONDEMAND = 2 };
static void sdl_miyoomini_set_cpugovernor(enum cpugov gov) {
   const char govstr[3][12] = { "performance", "powersave", "ondemand" };
   int fd = open("/sys/devices/system/cpu/cpufreq/policy0/scaling_governor", O_WRONLY);
   if (fd > 0) {
      write(fd, govstr[gov], strlen(govstr[gov])); close(fd);
   }
}

static void sdl_miyoomini_init_font_color(sdl_miyoomini_video_t *vid) {
   settings_t *settings = config_get_ptr();
   uint32_t red         = 0xFF;
   uint32_t green       = 0xFF;
   uint32_t blue        = 0xFF;

   if (settings) {
      red   = (uint32_t)((settings->floats.video_msg_color_r * 255.0f) + 0.5f) & 0xFF;
      green = (uint32_t)((settings->floats.video_msg_color_g * 255.0f) + 0.5f) & 0xFF;
      blue  = (uint32_t)((settings->floats.video_msg_color_b * 255.0f) + 0.5f) & 0xFF;
   }

   /* Convert to XRGB8888 */
   vid->font_colour32 = (red << 16) | (green << 8) | blue;
}

static void sdl_miyoomini_gfx_free(void *data) {
   sdl_miyoomini_video_t *vid = (sdl_miyoomini_video_t*)data;
   if (unlikely(!vid)) return;

   if (GFX_GetFlipCallback()) {
      GFX_SetFlipCallback(NULL, NULL); usleep(0x2000); /* wait for finish callback */
   }
   GFX_WaitAllDone();
   if (vid->screen) GFX_FreeSurface(vid->screen);
   if (vid->menuscreen) GFX_FreeSurface(vid->menuscreen);
   if (vid->menuscreen_rgui) GFX_FreeSurface(vid->menuscreen_rgui);
   GFX_Quit();

   if (vid->osd_font) bitmapfont_free_lut(vid->osd_font);

   free(vid);

   sdl_miyoomini_set_cpugovernor(ONDEMAND);
}

static void sdl_miyoomini_input_driver_init(
      const char *input_driver_name, const char *joypad_driver_name,
      input_driver_t **input, void **input_data) {
   /* Sanity check */
   if (!input || !input_data) return;

   *input      = NULL;
   *input_data = NULL;

   /* If input driver name is empty, cannot
    * initialise anything... */
   if (string_is_empty(input_driver_name)) return;

   if (string_is_equal(input_driver_name, "sdl_dingux")) {
      *input_data = input_driver_init_wrap(&input_sdl_dingux,
            joypad_driver_name);
      if (*input_data) *input = &input_sdl_dingux;
      return;
   }

#if defined(HAVE_SDL) || defined(HAVE_SDL2)
   if (string_is_equal(input_driver_name, "sdl")) {
      *input_data = input_driver_init_wrap(&input_sdl,
            joypad_driver_name);
      if (*input_data) *input = &input_sdl;
      return;
   }
#endif

#if defined(HAVE_UDEV)
   if (string_is_equal(input_driver_name, "udev")) {
      *input_data = input_driver_init_wrap(&input_udev,
            joypad_driver_name);
      if (*input_data) *input = &input_udev;
      return;
   }
#endif

#if defined(__linux__)
   if (string_is_equal(input_driver_name, "linuxraw")) {
      *input_data = input_driver_init_wrap(&input_linuxraw,
            joypad_driver_name);
      if (*input_data) *input = &input_linuxraw;
      return;
   }
#endif
}

static void sdl_miyoomini_set_output(
      sdl_miyoomini_video_t* vid,
      unsigned width, unsigned height, bool rgb32) {
   vid->content_width  = width;
   vid->content_height = height;
   if (vid->rotate & 1) { width = vid->content_height; height = vid->content_width; }

   /* Calculate scaling factor */
   uint32_t xmul = (SDL_MIYOOMINI_WIDTH<<16) / width;
   uint32_t ymul = (SDL_MIYOOMINI_HEIGHT<<16) / height;
   uint32_t mul = xmul < ymul ? xmul : ymul;
   uint32_t mul_int = (mul>>16);

   /* Change to aspect/fullscreen scaler when integer & screen size is over (no crop) */
   if (vid->scale_integer && mul_int) {
      /* Integer Scaling */
      vid->video_w = width * mul_int;
      vid->video_h = height * mul_int;
      if (!vid->keep_aspect) {
         /* Integer + Fullscreen , keep 4:3 for CRT console emulators */
         uint32_t Wx3 = vid->video_w * 3;
         uint32_t Hx4 = vid->video_h * 4;
         if (Wx3 != Hx4) {
            if (Wx3 > Hx4) vid->video_h = Wx3 / 4;
            else           vid->video_w = Hx4 / 3;
         }
      }
      vid->video_x = (SDL_MIYOOMINI_WIDTH - vid->video_w) >> 1;
      vid->video_y = (SDL_MIYOOMINI_HEIGHT - vid->video_h) >> 1;
   } else if (vid->keep_aspect) {
      /* Aspect Scaling */
      if (xmul > ymul) {
         vid->video_w  = (width * SDL_MIYOOMINI_HEIGHT) / height;
         vid->video_h = SDL_MIYOOMINI_HEIGHT;
         vid->video_x = (SDL_MIYOOMINI_WIDTH - vid->video_w) >> 1;
         vid->video_y = 0;
      } else {
         vid->video_w  = SDL_MIYOOMINI_WIDTH;
         vid->video_h = (height * SDL_MIYOOMINI_WIDTH) / width;
         vid->video_x = 0;
         vid->video_y = (SDL_MIYOOMINI_HEIGHT - vid->video_h) >> 1;
      }
   } else {
      /* Fullscreen */
      vid->video_w = SDL_MIYOOMINI_WIDTH;
      vid->video_h = SDL_MIYOOMINI_HEIGHT;
      vid->video_x = 0;
      vid->video_y = 0;
   }
   /* align to x4 bytes */
   if (!rgb32) { vid->video_x &= ~1; vid->video_w &= ~1; }

   /* Select scaler to use */
   uint32_t scale_mul = 0;
   if ( (vid->filter_type != DINGUX_IPU_FILTER_NEAREST) || (vid->scale_integer && mul_int && vid->keep_aspect) ) {
      scale_mul = 1;
      if ( (vid->scale_integer) || (vid->filter_type == DINGUX_IPU_FILTER_BICUBIC) ) {
         if      ((xmul > ymul ? xmul : ymul) >= (640<<16)/256) scale_mul = 4; /* w <= 256 or h <= 192 */
         else if ((xmul > ymul ? xmul : ymul) >= (640<<16)/512) scale_mul = 2; /* w <= 512 or h <= 384 */
      }
   }
   vid->frame_width  = scale_mul ? vid->content_width  * scale_mul : vid->video_w;
   vid->frame_height = scale_mul ? vid->content_height * scale_mul : vid->video_h;

   switch (scale_mul) {
      case 0:
         vid->scale_func = rgb32 ? scalenn_32 : scalenn_16;
         break;
      case 2:
         vid->scale_func = rgb32 ? scale2x_32 : scale2x_16;
         break;
      case 4:
         vid->scale_func = rgb32 ? scale4x_32 : scale4x_16;
         break;
      default:
         vid->scale_func = rgb32 ? scale1x_32 : scale1x_16;
         break;
   }

/* for DEBUG
   fprintf(stderr,"cw:%d ch:%d fw:%d fh:%d x:%d y:%d w:%d h:%d mul:%f scale_mul:%d\n",vid->content_width,vid->content_height,
         vid->frame_width,vid->frame_height,vid->video_x,vid->video_y,vid->video_w,vid->video_h,(float)mul/(1<<16),scale_mul);
*/

   /* Attempt to change video mode */
   GFX_WaitAllDone();
   if (vid->screen) GFX_FreeSurface(vid->screen);
   vid->screen = GFX_CreateRGBSurface(
         0, vid->frame_width, vid->frame_height, rgb32 ? 32 : 16, 0, 0, 0, 0);

   /* Check whether selected display mode is valid */
   if (unlikely(!vid->screen)) RARCH_ERR("[MI_GFX]: Failed to init GFX surface\n");
   /* Clear border */
   else if (!vid->menu_active) sdl_miyoomini_clear_border(fb_addr, vid->video_x, vid->video_y, vid->video_w, vid->video_h);
}

static void *sdl_miyoomini_gfx_init(const video_info_t *video,
      input_driver_t **input, void **input_data) {
   sdl_miyoomini_video_t *vid                    = NULL;
   uint32_t sdl_subsystem_flags                  = SDL_WasInit(0);
   settings_t *settings                          = config_get_ptr();
   const char *input_driver_name                 = settings->arrays.input_driver;
   const char *joypad_driver_name                = settings->arrays.input_joypad_driver;

   sdl_miyoomini_set_cpugovernor(PERFORMANCE);

   /* Initialise graphics subsystem, if required */
   if (sdl_subsystem_flags == 0) {
      if (SDL_Init(SDL_INIT_VIDEO) < 0) return NULL;
   } else if ((sdl_subsystem_flags & SDL_INIT_VIDEO) == 0) {
      if (SDL_InitSubSystem(SDL_INIT_VIDEO) < 0) return NULL;
   }

   vid = (sdl_miyoomini_video_t*)calloc(1, sizeof(*vid));
   if (!vid) return NULL;

   GFX_Init();

   vid->menuscreen = GFX_CreateRGBSurface(
         0, SDL_MIYOOMINI_WIDTH, SDL_MIYOOMINI_HEIGHT, 16, 0, 0, 0, 0);
   vid->menuscreen_rgui = GFX_CreateRGBSurface(
         0, RGUI_MENU_WIDTH, RGUI_MENU_HEIGHT, 16, 0, 0, 0, 0);

   if (!vid->menuscreen||!vid->menuscreen_rgui) {
      RARCH_ERR("[MI_GFX]: Failed to init GFX surface\n");
      goto error;
   }

   vid->content_width     = SDL_MIYOOMINI_WIDTH;
   vid->content_height    = SDL_MIYOOMINI_HEIGHT;
   vid->rgb32             = video->rgb32;
   vid->vsync             = video->vsync;
   vid->keep_aspect       = settings->bools.video_dingux_ipu_keep_aspect;
   vid->scale_integer     = settings->bools.video_scale_integer;
   vid->filter_type       = (enum dingux_ipu_filter_type)settings->uints.video_dingux_ipu_filter_type;
   vid->menu_active       = false;
   vid->was_in_menu       = false;
   vid->quitting          = false;
   vid->ff_frame_time_min = 16667;

   sdl_miyoomini_set_output(vid, vid->content_width, vid->content_height, vid->rgb32);

   GFX_SetFlipFlags(vid->vsync ? GFX_BLOCKING|GFX_FLIPWAIT : 0);

   sdl_miyoomini_input_driver_init(input_driver_name,
         joypad_driver_name, input, input_data);

   /* Initialise OSD font */
   sdl_miyoomini_init_font_color(vid);

   vid->osd_font = bitmapfont_get_lut();

   if (!vid->osd_font ||
       vid->osd_font->glyph_max <
            (SDL_NUM_FONT_GLYPHS - 1)) {
      RARCH_ERR("[SDL1]: Failed to init OSD font\n");
      goto error;
   }

   return vid;

error:
   sdl_miyoomini_gfx_free(vid);
   return NULL;
}

static bool sdl_miyoomini_gfx_frame(void *data, const void *frame,
      unsigned width, unsigned height, uint64_t frame_count,
      unsigned pitch, const char *msg, video_frame_info_t *video_info) {
   sdl_miyoomini_video_t* vid = (sdl_miyoomini_video_t*)data;

   /* Return early if:
    * - Input sdl_miyoomini_video_t struct is NULL
    *   (cannot realistically happen)
    * - Menu is inactive and input 'content' frame
    *   data is NULL (may happen when e.g. a running
    *   core skips a frame) */
   if (unlikely(!vid || (!frame && !vid->menu_active))) return true;

   /* If fast forward is currently active, we may
    * push frames at an 'unlimited' rate. Since the
    * display has a fixed refresh rate of 60 Hz, this
    * represents wasted effort. We therefore drop any
    * 'excess' frames in this case.
    * (Note that we *only* do this when fast forwarding.
    * Attempting this trick while running content normally
    * will cause bad frame pacing) */
   if (unlikely(video_info->input_driver_nonblock_state)) {
      retro_time_t current_time = cpu_features_get_time_usec();

      if ((current_time - vid->last_frame_time) < vid->ff_frame_time_min)
         return true;

      vid->last_frame_time = current_time;
   }

#ifdef HAVE_MENU
   menu_driver_frame(video_info->menu_is_alive, video_info);
#endif

   /* Render OSD text at flip */
   if (msg) {
      memcpy(vid->msg_tmp, msg, sizeof(vid->msg_tmp));
      GFX_SetFlipCallback(sdl_miyoomini_print_msg, vid);
   } else if (vid->msg_count) {
      vid->msg_tmp[0] = 0;
      GFX_SetFlipCallback(sdl_miyoomini_print_msg, vid);
   } else {
      GFX_SetFlipCallback(NULL, NULL);
   }

   if (likely(!vid->menu_active)) {
      /* Clear border if we were in the menu on the previous frame */
      if (unlikely(vid->was_in_menu)) {
         sdl_miyoomini_clear_border(fb_addr, vid->video_x, vid->video_y, vid->video_w, vid->video_h);
         vid->was_in_menu = false;
      }
      /* Update video mode if width/height have changed */
      if (unlikely( (vid->content_width  != width ) ||
                    (vid->content_height != height) )) {
         sdl_miyoomini_set_output(vid, width, height, vid->rgb32);
      }
      /* WaitAllDone when frametime is too fast ( < 8.192ms ) */
      static long recent_usec;
      struct timeval tod;
      gettimeofday(&tod, NULL);
      if (tod.tv_usec < recent_usec) recent_usec -= 1000000;
      if (tod.tv_usec - recent_usec < 8192) MI_GFX_WaitAllDone(FALSE, flipFence);
      recent_usec = tod.tv_usec;
      /* Blit frame to GFX surface */
      vid->scale_func(vid, (void*)frame, vid->screen->pixels, width, height, pitch, vid->screen->pitch);
      GFX_UpdateRect(vid->screen, vid->video_x, vid->video_y, vid->video_w, vid->video_h);
   } else {
      scale2x_n16(vid->menuscreen_rgui->pixels, vid->menuscreen->pixels, RGUI_MENU_WIDTH, RGUI_MENU_HEIGHT, 0,0);
      stOpt.eRotate = E_MI_GFX_ROTATE_180;
      GFX_Flip(vid->menuscreen);
      stOpt.eRotate = vid->rotate;
   }
   return true;
}

static void sdl_miyoomini_set_texture_enable(void *data, bool state, bool full_screen) {
   sdl_miyoomini_video_t *vid = (sdl_miyoomini_video_t*)data;
   if (unlikely(!vid)) return;

   if (state == vid->menu_active) return;
   vid->menu_active = state;

   if (state) {
      sdl_miyoomini_set_cpugovernor(POWERSAVE);
      vid->was_in_menu = true;
   } else sdl_miyoomini_set_cpugovernor(PERFORMANCE);
}

static void sdl_miyoomini_set_texture_frame(void *data, const void *frame, bool rgb32,
      unsigned width, unsigned height, float alpha) {
   sdl_miyoomini_video_t *vid = (sdl_miyoomini_video_t*)data;

   if (unlikely( !vid || rgb32 || (width != RGUI_MENU_WIDTH) || (height != RGUI_MENU_HEIGHT))) return;

   memcpy_neon(vid->menuscreen_rgui->pixels, (void*)frame,
      RGUI_MENU_WIDTH * RGUI_MENU_HEIGHT * sizeof(uint16_t));
}

static void sdl_miyoomini_gfx_set_nonblock_state(void *data, bool toggle,
      bool adaptive_vsync_enabled, unsigned swap_interval) {
   sdl_miyoomini_video_t *vid = (sdl_miyoomini_video_t*)data;
   if (unlikely(!vid)) return;

   bool vsync            = !toggle;

   /* Check whether vsync status has changed */
   if (vid->vsync != vsync)
   {
      vid->vsync              = vsync;
      GFX_SetFlipFlags(vsync ? GFX_BLOCKING|GFX_FLIPWAIT : 0);
   }
}

static void sdl_miyoomini_gfx_check_window(sdl_miyoomini_video_t *vid) {
   SDL_Event event;

   SDL_PumpEvents();
   while (SDL_PeepEvents(&event, 1, SDL_GETEVENT, SDL_QUITMASK))
   {
      if (event.type != SDL_QUIT)
         continue;

      vid->quitting = true;
      break;
   }
}

static bool sdl_miyoomini_gfx_alive(void *data) {
   sdl_miyoomini_video_t *vid = (sdl_miyoomini_video_t*)data;
   if (unlikely(!vid)) return false;

   sdl_miyoomini_gfx_check_window(vid);
   return !vid->quitting;
}

static bool sdl_miyoomini_gfx_focus(void *data) { return true; }
static bool sdl_miyoomini_gfx_suppress_screensaver(void *data, bool enable) { return false; }
static bool sdl_miyoomini_gfx_has_windowed(void *data) { return false; }

static void sdl_miyoomini_gfx_set_rotation(void *data, unsigned rotation) {
   sdl_miyoomini_video_t *vid = (sdl_miyoomini_video_t*)data;
   if (unlikely(!vid)) return;
   switch (rotation) {
      case 1:
         stOpt.eRotate = E_MI_GFX_ROTATE_90; break;
      case 2:
         stOpt.eRotate = E_MI_GFX_ROTATE_0; break;
      case 3:
         stOpt.eRotate = E_MI_GFX_ROTATE_270; break;
      default:
         stOpt.eRotate = E_MI_GFX_ROTATE_180; break;
   }
   if (vid->rotate != stOpt.eRotate) {
      vid->rotate = stOpt.eRotate;
      sdl_miyoomini_set_output(vid, vid->content_width, vid->content_height, vid->rgb32);
   }
}

static void sdl_miyoomini_gfx_viewport_info(void *data, struct video_viewport *vp) {
   sdl_miyoomini_video_t *vid = (sdl_miyoomini_video_t*)data;
   if (unlikely(!vid)) return;

   vp->x           = vid->video_x;
   vp->y           = vid->video_y;
   vp->width       = vid->video_w;
   vp->height      = vid->video_h;
   vp->full_width  = SDL_MIYOOMINI_WIDTH;
   vp->full_height = SDL_MIYOOMINI_HEIGHT;
}

static float sdl_miyoomini_get_refresh_rate(void *data) { return 60.0f; }

static void sdl_miyoomini_set_filtering(void *data, unsigned index, bool smooth, bool ctx_scaling) {
   sdl_miyoomini_video_t *vid = (sdl_miyoomini_video_t*)data;
   settings_t *settings       = config_get_ptr();
   if (unlikely(!vid || !settings)) return;

   enum dingux_ipu_filter_type ipu_filter_type = (settings) ?
         (enum dingux_ipu_filter_type)settings->uints.video_dingux_ipu_filter_type :
         DINGUX_IPU_FILTER_BICUBIC;

   /* Update software filter setting, if required */
   if (vid->filter_type != ipu_filter_type) {
      vid->filter_type = ipu_filter_type;
      sdl_miyoomini_set_output(vid, vid->content_width, vid->content_height, vid->rgb32);
   }
}

static void sdl_miyoomini_apply_state_changes(void *data) {
   sdl_miyoomini_video_t *vid = (sdl_miyoomini_video_t*)data;
   settings_t *settings       = config_get_ptr();
   if (unlikely(!vid || !settings)) return;

   bool keep_aspect       = (settings) ? settings->bools.video_dingux_ipu_keep_aspect : true;
   bool integer_scaling   = (settings) ? settings->bools.video_scale_integer : false;

   if ((vid->keep_aspect != keep_aspect) ||
       (vid->scale_integer != integer_scaling)) {
      vid->keep_aspect   = keep_aspect;
      vid->scale_integer = integer_scaling;

      /* Aspect/scaling changes require all frame
       * dimension/padding/cropping parameters to
       * be recalculated. Easiest method is to just
       * (re-)set the current output video mode */
      sdl_miyoomini_set_output(vid, vid->content_width, vid->content_height, vid->rgb32);
   }
}

static uint32_t sdl_miyoomini_get_flags(void *data) { return 0; }

static const video_poke_interface_t sdl_miyoomini_poke_interface = {
   sdl_miyoomini_get_flags,
   NULL, /* load_texture */
   NULL, /* unload_texture */
   NULL, /* set_video_mode */
   sdl_miyoomini_get_refresh_rate,
   sdl_miyoomini_set_filtering,
   NULL, /* get_video_output_size */
   NULL, /* get_video_output_prev */
   NULL, /* get_video_output_next */
   NULL, /* get_current_framebuffer */
   NULL, /* get_proc_address */
   NULL, /* set_aspect_ratio */
   sdl_miyoomini_apply_state_changes,
   sdl_miyoomini_set_texture_frame,
   sdl_miyoomini_set_texture_enable,
   NULL, /* set_osd_msg */
   NULL, /* sdl_show_mouse */
   NULL, /* sdl_grab_mouse_toggle */
   NULL, /* get_current_shader */
   NULL, /* get_current_software_framebuffer */
   NULL, /* get_hw_render_interface */
   NULL, /* set_hdr_max_nits */
   NULL, /* set_hdr_paper_white_nits */
   NULL, /* set_hdr_contrast */
   NULL  /* set_hdr_expand_gamut */
};

static void sdl_miyoomini_get_poke_interface(void *data, const video_poke_interface_t **iface) {
   *iface = &sdl_miyoomini_poke_interface;
}

static bool sdl_miyoomini_gfx_set_shader(void *data,
      enum rarch_shader_type type, const char *path) { return false; }

video_driver_t video_sdl_dingux = {
   sdl_miyoomini_gfx_init,
   sdl_miyoomini_gfx_frame,
   sdl_miyoomini_gfx_set_nonblock_state,
   sdl_miyoomini_gfx_alive,
   sdl_miyoomini_gfx_focus,
   sdl_miyoomini_gfx_suppress_screensaver,
   sdl_miyoomini_gfx_has_windowed,
   sdl_miyoomini_gfx_set_shader,
   sdl_miyoomini_gfx_free,
   "sdl_dingux",
   NULL, /* set_viewport */
   sdl_miyoomini_gfx_set_rotation,
   sdl_miyoomini_gfx_viewport_info,
   NULL, /* read_viewport  */
   NULL, /* read_frame_raw */
#ifdef HAVE_OVERLAY
   NULL, /* get_overlay_interface */
#endif
#ifdef HAVE_VIDEO_LAYOUT
   NULL, /* get_video_layout_render_interface */
#endif
   sdl_miyoomini_get_poke_interface
};
