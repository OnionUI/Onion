/*  RetroArch - A frontend for libretro.
 *  Copyright (C) 2010-2014 - Hans-Kristian Arntzen
 *  Copyright (C) 2011-2018 - Daniel De Matteis
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

/* Compile: gcc -o lcd3x_stripe.so -shared lcd3x_stripe.c -std=c99 -O3 -Wall -pedantic -fPIC */

#include "softfilter.h"
#include <stdlib.h>
#include <string.h>

#ifdef RARCH_INTERNAL
#define softfilter_get_implementation lcd3x_stripe_get_implementation
#define softfilter_thread_data lcd3x_stripe_softfilter_thread_data
#define filter_data lcd3x_stripe_filter_data
#endif

struct softfilter_thread_data
{
   void *out_data;
   const void *in_data;
   size_t out_pitch;
   size_t in_pitch;
   unsigned colfmt;
   unsigned width;
   unsigned height;
   int first;
   int last;
};

struct filter_data
{
   unsigned threads;
   struct softfilter_thread_data *workers;
   unsigned in_fmt;
};

static unsigned lcd3x_stripe_generic_input_fmts(void)
{
   return SOFTFILTER_FMT_XRGB8888 | SOFTFILTER_FMT_RGB565;
}

static unsigned lcd3x_stripe_generic_output_fmts(unsigned input_fmts)
{
   return input_fmts;
}

static unsigned lcd3x_stripe_generic_threads(void *data)
{
   struct filter_data *filt = (struct filter_data*)data;
   return filt->threads;
}

static void *lcd3x_stripe_generic_create(const struct softfilter_config *config,
      unsigned in_fmt, unsigned out_fmt,
      unsigned max_width, unsigned max_height,
      unsigned threads, softfilter_simd_mask_t simd, void *userdata)
{
   struct filter_data *filt = (struct filter_data*)calloc(1, sizeof(*filt));
   (void)simd;
   (void)config;
   (void)userdata;

   if (!filt) {
      return NULL;
   }
   /* Apparently the code is not thread-safe,
    * so force single threaded operation... */
   filt->workers = (struct softfilter_thread_data*)calloc(1, sizeof(struct softfilter_thread_data));
   filt->threads = 1;
   filt->in_fmt  = in_fmt;
   if (!filt->workers) {
      free(filt);
      return NULL;
   }
   return filt;
}

static void lcd3x_stripe_generic_output(void *data,
      unsigned *out_width, unsigned *out_height,
      unsigned width, unsigned height)
{
   *out_width = width * 3;
   *out_height = height * 3;
}

static void lcd3x_stripe_generic_destroy(void *data)
{
   struct filter_data *filt = (struct filter_data*)data;
   if (!filt) {
      return;
   }
   free(filt->workers);
   free(filt);
}

static void lcd3x_stripe_c16(void* __restrict src, void* __restrict dst, uint32_t sw, uint32_t sh, uint32_t sp, uint32_t dp) {
	if (!sw||!sh) { return; }
	uint32_t x, dx, pix, dpix1, dpix2, dpix3, swl = sw*sizeof(uint16_t);
	if (!sp) { sp = swl; } swl*=3; if (!dp) { dp = swl; }
	for (; sh>0; sh--, src=(uint8_t*)src+sp, dst=(uint8_t*)dst+dp*3) {
		uint32_t *s = (uint32_t* __restrict)src;
		uint32_t *d = (uint32_t* __restrict)dst;
		for (x=dx=0; x<(sw/2); x++, dx+=3) {
			pix = s[x];
			dpix1=(pix & 0xF800) | ((pix & 0x07E0)<<16);		// RG
			dpix2=(pix & 0xF800001F);				// Br
			dpix3=((pix>>16) & 0x07E0) | (pix & 0x001F0000);	// gb
			d[dx] = dpix1; d[dx+1] = dpix2; d[dx+2] = dpix3;
		}
		if (sw&1) {
			uint16_t *s16 = (uint16_t*)s;
			uint16_t *d16 = (uint16_t*)d;
			uint16_t pix16 = s16[x*2];
			dpix1=(pix16 & 0xF800) | ((pix16 & 0x07E0)<<16);	// RG
			d[dx] = dpix1; d16[(dx+1)*2] = (pix16 & 0x001F);	// B
		}
		memcpy((uint8_t*)dst+dp*1, dst, swl);
		memcpy((uint8_t*)dst+dp*2, dst, swl);
	}
}

static void lcd3x_stripe_c32(void* __restrict src, void* __restrict dst, uint32_t sw, uint32_t sh, uint32_t sp, uint32_t dp) {
	if (!sw||!sh) { return; }
	uint32_t x, dx, pix, swl = sw*sizeof(uint32_t);
	if (!sp) { sp = swl; } swl*=3; if (!dp) { dp = swl; }
	for (; sh>0; sh--, src=(uint8_t*)src+sp, dst=(uint8_t*)dst+dp*3) {
		uint32_t *s = (uint32_t* __restrict)src;
		uint32_t *d = (uint32_t* __restrict)dst;
		for (x=dx=0; x<sw; x++, dx+=3) {
			pix = s[x];
			d[dx] = pix & 0x00FF0000; d[dx+1] = pix & 0x0000FF00; d[dx+2] = pix & 0x000000FF;
		}
		memcpy((uint8_t*)dst+dp*1, dst, swl);
		memcpy((uint8_t*)dst+dp*2, dst, swl);
	}
}

static void lcd3x_stripe_work_cb_xrgb8888(void *data, void *thread_data)
{
   struct softfilter_thread_data *thr = (struct softfilter_thread_data*)thread_data;
   lcd3x_stripe_c32((void*)thr->in_data, thr->out_data, thr->width, thr->height, thr->in_pitch, thr->out_pitch);
}

static void lcd3x_stripe_work_cb_rgb565(void *data, void *thread_data)
{
   struct softfilter_thread_data *thr = (struct softfilter_thread_data*)thread_data;
   lcd3x_stripe_c16((void*)thr->in_data, thr->out_data, thr->width, thr->height, thr->in_pitch, thr->out_pitch);
}

static void lcd3x_stripe_generic_packets(void *data,
      struct softfilter_work_packet *packets,
      void *output, size_t output_stride,
      const void *input, unsigned width, unsigned height, size_t input_stride)
{
   /* We are guaranteed single threaded operation
    * (filt->threads = 1) so we don't need to loop
    * over threads and can cull some code. This only
    * makes the tiniest performance difference, but
    * every little helps when running on an o3DS... */
   struct filter_data *filt = (struct filter_data*)data;
   struct softfilter_thread_data *thr = (struct softfilter_thread_data*)&filt->workers[0];

   thr->out_data = (uint8_t*)output;
   thr->in_data = (const uint8_t*)input;
   thr->out_pitch = output_stride;
   thr->in_pitch = input_stride;
   thr->width = width;
   thr->height = height;

   if (filt->in_fmt == SOFTFILTER_FMT_XRGB8888) {
      packets[0].work = lcd3x_stripe_work_cb_xrgb8888;
   } else if (filt->in_fmt == SOFTFILTER_FMT_RGB565) {
      packets[0].work = lcd3x_stripe_work_cb_rgb565;
   }
   packets[0].thread_data = thr;
}

static const struct softfilter_implementation lcd3x_stripe_generic = {
   lcd3x_stripe_generic_input_fmts,
   lcd3x_stripe_generic_output_fmts,

   lcd3x_stripe_generic_create,
   lcd3x_stripe_generic_destroy,

   lcd3x_stripe_generic_threads,
   lcd3x_stripe_generic_output,
   lcd3x_stripe_generic_packets,

   SOFTFILTER_API_VERSION,
   "LCD3x_stripe",
   "lcd3x_stripe",
};

const struct softfilter_implementation *softfilter_get_implementation(
      softfilter_simd_mask_t simd)
{
   (void)simd;
   return &lcd3x_stripe_generic;
}

#ifdef RARCH_INTERNAL
#undef softfilter_get_implementation
#undef softfilter_thread_data
#undef filter_data
#endif
