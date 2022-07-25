#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "scaler_neon.h"

//
//	arm NEON / C integer scalers for miyoomini
//	args/	src :	src offset		address of top left corner
//		dst :	dst offset		address	of top left corner
//		sw  :	src width		pixels
//		sh  :	src height		pixels
//		sp  :	src pitch (stride)	bytes	if 0, (src width * [2|4]) is used
//		dp  :	dst pitch (stride)	bytes	if 0, (src width * [2|4] * multiplier) is used
//
//	** NOTE **
//	since 32bit aligned addresses need to be processed for NEON scalers,
//	x-offset and stride pixels must be even# in the case of 16bpp,
//	if odd#, then handled by the C scaler
//

//	memcpy_neon (dst/src must be aligned 4, size must be aligned 2)
static inline void memcpy_neon(void* dst, void* src, uint32_t size) {
	asm volatile (
	"	bic r4, %[sz], #127	;"
	"	add r3, %[s], %[sz]	;"	// r3 = endofs
	"	add r4, %[s], r4	;"	// r4 = s128ofs
	"	cmp %[s], r4		;"
	"	beq 2f			;"
	"1:	vldmia %[s]!, {q8-q15}	;"	// 128 bytes
	"	vstmia %[d]!, {q8-q15}	;"
	"	cmp %[s], r4		;"
	"	bne 1b			;"
	"2:	cmp %[s], r3		;"
	"	beq 7f			;"
	"	tst %[sz], #64		;"
	"	beq 3f			;"
	"	vldmia %[s]!, {q8-q11}	;"	// 64 bytes
	"	vstmia %[d]!, {q8-q11}	;"
	"	cmp %[s], r3		;"
	"	beq 7f			;"
	"3:	tst %[sz], #32		;"
	"	beq 4f			;"
	"	vldmia %[s]!, {q12-q13}	;"	// 32 bytes
	"	vstmia %[d]!, {q12-q13}	;"
	"	cmp %[s], r3		;"
	"	beq 7f			;"
	"4:	tst %[sz], #16		;"
	"	beq 5f			;"
	"	vldmia %[s]!, {q14}	;"	// 16 bytes
	"	vstmia %[d]!, {q14}	;"
	"	cmp %[s], r3		;"
	"	beq 7f			;"
	"5:	tst %[sz], #8		;"
	"	beq 6f			;"
	"	vldmia %[s]!, {d30}	;"	// 8 bytes
	"	vstmia %[d]!, {d30}	;"
	"	cmp %[s], r3		;"
	"	beq 7f			;"
	"6:	ldrh r4, [%[s]],#2	;"	// rest
	"	strh r4, [%[d]],#2	;"
	"	cmp %[s], r3		;"
	"	bne 6b			;"
	"7:				"
	: [s]"+r"(src), [d]"+r"(dst)
	: [sz]"r"(size)
	: "r3","r4","q8","q9","q10","q11","q12","q13","q14","q15","memory","cc"
	);
}

//
//	NEON scalers
//

void scale1x_n16(void* __restrict src, void* __restrict dst, uint32_t sw, uint32_t sh, uint32_t sp, uint32_t dp) {
	if (!sw||!sh) { return; }
	uint32_t swl = sw*sizeof(uint16_t);
	if (!sp) { sp = swl; } if (!dp) { dp = swl*1; }
	if ( ((uintptr_t)src&3)||((uintptr_t)dst&3)||(sp&3)||(dp&3) ) { scale1x_c16(src,dst,sw,sh,sp,dp); return; }
	if ((swl == sp)&&(sp == dp)) memcpy_neon(dst, src, sp*sh);
	else for (; sh>0; sh--, src=(uint8_t*)src+sp, dst=(uint8_t*)dst+dp) memcpy_neon(dst, src, swl);
}

void scale1x_n32(void* __restrict src, void* __restrict dst, uint32_t sw, uint32_t sh, uint32_t sp, uint32_t dp) {
	if (!sw||!sh) { return; }
	uint32_t swl = sw*sizeof(uint32_t);
	if (!sp) { sp = swl; } if (!dp) { dp = swl*1; }
	if ( ((uintptr_t)src&3)||((uintptr_t)dst&3)||(sp&3)||(dp&3) ) { scale1x_c32(src,dst,sw,sh,sp,dp); return; }
	if ((swl == sp)&&(sp == dp)) memcpy_neon(dst, src, sp*sh);
	else for (; sh>0; sh--, src=(uint8_t*)src+sp, dst=(uint8_t*)dst+dp) memcpy_neon(dst, src, swl);
}

void scale2x_n16(void* __restrict src, void* __restrict dst, uint32_t sw, uint32_t sh, uint32_t sp, uint32_t dp) {
	if (!sw||!sh) { return; }
	uint32_t swl = sw * sizeof(uint16_t);
	if (!sp) { sp = swl; } if (!dp) { dp = swl*2; }
	if ( ((uintptr_t)src&3)||((uintptr_t)dst&3)||(sp&3)||(dp&3) ) { scale2x_c16(src,dst,sw,sh,sp,dp); return; }
	uint32_t swl64 = swl & ~63;
	uint32_t swrest = swl & 63;
	uint32_t sadd = sp - swl;
	uint32_t dadd = dp*2 - swl*2;
	uint8_t* finofs = (uint8_t*)src + (sp*sh);
	asm volatile (
	"1:	add lr, %0, %2		;"	// lr  = x64bytes offset
	"	add r9, %0, %3		;"	// r9  = lineend offset
	"	add r10, %1, %7		;"	// r10 = 2x line offset
	"	cmp %0, lr		;"
	"	beq 3f			;"
	"2:	vldmia %0!, {q8-q11}	;"	// 32 pixels 64 bytes
	"	vdup.16 d0, d23[3]	;"
	"	vdup.16 d1, d23[2]	;"
	"	vext.16 d31, d1,d0,#2	;"
	"	vdup.16 d0, d23[1]	;"
	"	vdup.16 d1, d23[0]	;"
	"	vext.16 d30, d1,d0,#2	;"
	"	vdup.16 d0, d22[3]	;"
	"	vdup.16 d1, d22[2]	;"
	"	vext.16 d29, d1,d0,#2	;"
	"	vdup.16 d0, d22[1]	;"
	"	vdup.16 d1, d22[0]	;"
	"	vext.16 d28, d1,d0,#2	;"
	"	vdup.16 d0, d21[3]	;"
	"	vdup.16 d1, d21[2]	;"
	"	vext.16 d27, d1,d0,#2	;"
	"	vdup.16 d0, d21[1]	;"
	"	vdup.16 d1, d21[0]	;"
	"	vext.16 d26, d1,d0,#2	;"
	"	vdup.16 d0, d20[3]	;"
	"	vdup.16 d1, d20[2]	;"
	"	vext.16 d25, d1,d0,#2	;"
	"	vdup.16 d0, d20[1]	;"
	"	vdup.16 d1, d20[0]	;"
	"	vext.16 d24, d1,d0,#2	;"
	"	vdup.16 d0, d19[3]	;"
	"	vdup.16 d1, d19[2]	;"
	"	vext.16 d23, d1,d0,#2	;"
	"	vdup.16 d0, d19[1]	;"
	"	vdup.16 d1, d19[0]	;"
	"	vext.16 d22, d1,d0,#2	;"
	"	vdup.16 d0, d18[3]	;"
	"	vdup.16 d1, d18[2]	;"
	"	vext.16 d21, d1,d0,#2	;"
	"	vdup.16 d0, d18[1]	;"
	"	vdup.16 d1, d18[0]	;"
	"	vext.16 d20, d1,d0,#2	;"
	"	vdup.16 d0, d17[3]	;"
	"	vdup.16 d1, d17[2]	;"
	"	vext.16 d19, d1,d0,#2	;"
	"	vdup.16 d0, d17[1]	;"
	"	vdup.16 d1, d17[0]	;"
	"	vext.16 d18, d1,d0,#2	;"
	"	vdup.16 d0, d16[3]	;"
	"	vdup.16 d1, d16[2]	;"
	"	vext.16 d17, d1,d0,#2	;"
	"	vdup.16 d0, d16[1]	;"
	"	vdup.16 d1, d16[0]	;"
	"	vext.16 d16, d1,d0,#2	;"
	"	cmp %0, lr		;"
	"	vstmia %1!, {q8-q15}	;"
	"	vstmia r10!, {q8-q15}	;"
	"	bne 2b			;"
	"3:	cmp %0, r9		;"
	"	beq 5f			;"
	"	tst %8, #32		;"
	"	beq 4f			;"
	"	vldmia %0!,{q8-q9}	;"	// 16 pixels
	"	vdup.16 d0, d19[3]	;"
	"	vdup.16 d1, d19[2]	;"
	"	vext.16 d23, d1,d0,#2	;"
	"	vdup.16 d0, d19[1]	;"
	"	vdup.16 d1, d19[0]	;"
	"	vext.16 d22, d1,d0,#2	;"
	"	vdup.16 d0, d18[3]	;"
	"	vdup.16 d1, d18[2]	;"
	"	vext.16 d21, d1,d0,#2	;"
	"	vdup.16 d0, d18[1]	;"
	"	vdup.16 d1, d18[0]	;"
	"	vext.16 d20, d1,d0,#2	;"
	"	vdup.16 d0, d17[3]	;"
	"	vdup.16 d1, d17[2]	;"
	"	vext.16 d19, d1,d0,#2	;"
	"	vdup.16 d0, d17[1]	;"
	"	vdup.16 d1, d17[0]	;"
	"	vext.16 d18, d1,d0,#2	;"
	"	vdup.16 d0, d16[3]	;"
	"	vdup.16 d1, d16[2]	;"
	"	vext.16 d17, d1,d0,#2	;"
	"	vdup.16 d0, d16[1]	;"
	"	vdup.16 d1, d16[0]	;"
	"	vext.16 d16, d1,d0,#2	;"
	"	cmp %0, r9		;"
	"	vstmia %1!, {q8-q11}	;"
	"	vstmia r10!, {q8-q11}	;"
	"	beq 5f			;"
	"4:	ldrh lr, [%0],#2	;"	// rest
	"	orr lr, lr, lsl #16	;"
	"	cmp %0, r9		;"
	"	str lr, [%1],#4		;"
	"	str lr, [r10],#4	;"
	"	bne 4b			;"
	"5:	add %0, %0, %4		;"
	"	add %1, %1, %5		;"
	"	cmp %0, %6		;"
	"	bne 1b			"
	: "+r"(src), "+r"(dst)
	: "r"(swl64), "r"(swl), "r"(sadd), "r"(dadd), "r"(finofs), "r"(dp), "r"(swrest)
	: "r9","r10","lr","q0","q8","q9","q10","q11","q12","q13","q14","q15","memory","cc"
	);
}

void scale2x_n32(void* __restrict src, void* __restrict dst, uint32_t sw, uint32_t sh, uint32_t sp, uint32_t dp) {
	if (!sw||!sh) { return; }
	uint32_t swl = sw * sizeof(uint32_t);
	if (!sp) { sp = swl; } if (!dp) { dp = swl*2; }
	if ( ((uintptr_t)src&3)||((uintptr_t)dst&3)||(sp&3)||(dp&3) ) { scale2x_c32(src,dst,sw,sh,sp,dp); return; }
	uint32_t swl64 = swl & ~63;
	uint32_t sadd = sp - swl;
	uint32_t dadd = dp*2 - swl*2;
	uint8_t* finofs = (uint8_t*)src + (sp*sh);
	asm volatile (
	"1:	add lr, %0, %2		;"	// lr = x64bytes offset
	"	add r8, %0, %3		;"	// r8 = lineend offset
	"	add r9, %1, %7		;"	// r9 = 2x line offset
	"	cmp %0, lr		;"
	"	beq 3f			;"
	"2:	vldmia %0!, {q8-q11}	;"	// 16 pixels 64 bytes
	"	vdup.32 d31, d23[1]	;"
	"	vdup.32 d30, d23[0]	;"
	"	vdup.32 d29, d22[1]	;"
	"	vdup.32 d28, d22[0]	;"
	"	vdup.32 d27, d21[1]	;"
	"	vdup.32 d26, d21[0]	;"
	"	vdup.32 d25, d20[1]	;"
	"	vdup.32 d24, d20[0]	;"
	"	vdup.32 d23, d19[1]	;"
	"	vdup.32 d22, d19[0]	;"
	"	vdup.32 d21, d18[1]	;"
	"	vdup.32 d20, d18[0]	;"
	"	vdup.32 d19, d17[1]	;"
	"	vdup.32 d18, d17[0]	;"
	"	vdup.32 d17, d16[1]	;"
	"	vdup.32 d16, d16[0]	;"
	"	cmp %0, lr		;"
	"	vstmia %1!, {q8-q15}	;"
	"	vstmia r9!, {q8-q15}	;"
	"	bne 2b			;"
	"3:	cmp %0, r8		;"
	"	beq 5f			;"
	"4:	ldr lr, [%0],#4		;"	// rest
	"	vdup.32 d16, lr		;"
	"	cmp %0, r8		;"
	"	vstmia %1!, {d16}	;"
	"	vstmia r9!, {d16}	;"
	"	bne 4b			;"
	"5:	add %0, %0, %4		;"
	"	add %1, %1, %5		;"
	"	cmp %0, %6		;"
	"	bne 1b			"
	: "+r"(src), "+r"(dst)
	: "r"(swl64), "r"(swl), "r"(sadd), "r"(dadd), "r"(finofs), "r"(dp)
	: "r8","r9","lr","q8","q9","q10","q11","q12","q13","q14","q15","memory","cc"
	);
}

void scale3x_n16(void* __restrict src, void* __restrict dst, uint32_t sw, uint32_t sh, uint32_t sp, uint32_t dp) {
	if (!sw||!sh) { return; }
	uint32_t swl = sw * sizeof(uint16_t);
	if (!sp) { sp = swl; } if (!dp) { dp = swl*3; }
	if ( ((uintptr_t)src&3)||((uintptr_t)dst&3)||(sp&3)||(dp&3) ) { scale3x_c16(src,dst,sw,sh,sp,dp); return; }
	uint32_t swl32 = swl & ~31;
	uint32_t sadd = sp - swl;
	uint32_t dadd = dp - swl*3;
	uint32_t dwl = swl*3;
	uint32_t dwl128 = dwl & ~127;
	uint32_t dwrest = dwl & 127;
	uint8_t* finofs = (uint8_t*)src + (sp*sh);
	asm volatile (
	"1:	mov r11,%1		;"	// dst push
	"	add lr, %0, %2		;"	// lr  = x32bytes offset
	"	add r10, %0, %3		;"	// r10 = lineend offset
	"	cmp %0, lr		;"
	"	beq 3f			;"
	"2:	vldmia %0!, {q8-q9}	;"	// 16 pixels 32 bytes
	"	vdup.16 d31, d19[3]	;"	//  FFFF
	"	vdup.16 d30, d19[2]	;"	//  EEEE
	"	vdup.16 d29, d19[1]	;"	//  DDDD
	"	vdup.16 d28, d19[0]	;"	//  CCCC
	"	vext.16 d27, d30,d31,#3	;"	// EFFF
	"	vext.16 d26, d29,d30,#2	;"	// DDEE
	"	vext.16 d25, d28,d29,#1	;"	// CCCD
	"	vdup.16 d31, d18[3]	;"	//  BBBB
	"	vdup.16 d30, d18[2]	;"	//  AAAA
	"	vdup.16 d29, d18[1]	;"	//  9999
	"	vdup.16 d28, d18[0]	;"	//  8888
	"	vext.16 d24, d30,d31,#3	;"	// ABBB
	"	vext.16 d23, d29,d30,#2	;"	// 99AA
	"	vext.16 d22, d28,d29,#1	;"	// 8889
	"	vdup.16 d31, d17[3]	;"	//  7777
	"	vdup.16 d30, d17[2]	;"	//  6666
	"	vdup.16 d29, d17[1]	;"	//  5555
	"	vdup.16 d28, d17[0]	;"	//  4444
	"	vext.16 d21, d30,d31,#3	;"	// 6777
	"	vext.16 d20, d29,d30,#2	;"	// 5566
	"	vext.16 d19, d28,d29,#1	;"	// 4445
	"	vdup.16 d31, d16[3]	;"	//  3333
	"	vdup.16 d30, d16[2]	;"	//  2222
	"	vdup.16 d29, d16[1]	;"	//  1111
	"	vdup.16 d28, d16[0]	;"	//  0000
	"	vext.16 d18, d30,d31,#3	;"	// 2333
	"	vext.16 d17, d29,d30,#2	;"	// 1122
	"	vext.16 d16, d28,d29,#1	;"	// 0001
	"	cmp %0, lr		;"
	"	vstmia %1!, {q8-q13}	;"
	"	bne 2b			;"
	"3:	cmp %0, r10		;"
	"	beq 5f			;"
	"4:	ldrh lr, [%0],#2	;"	// rest
	"	orr lr, lr, lsl #16	;"
	"	cmp %0, r10		;"
	"	str lr, [%1],#4		;"
	"	strh lr, [%1],#2	;"
	"	bne 4b			;"
	"5:	add %0, %4		;"
	"	add %1, %5		;"
	"	mov r12, %1		;"	// r12 = 2x line offset
	"	add %1, %8		;"	//
	"	add %1, %5		;"	// %1 = 3x line offset
	"	add lr, r11, %7		;"	// lr = x128bytes offset
	"	add r10, r11, %8	;"	// r10 = lineend offset
	"	cmp r11, lr		;"
	"	beq 7f			;"
	"6:	vldmia r11!, {q8-q15}	;"	// 64 pixels 128 bytes
	"	vstmia r12!, {q8-q15}	;"
	"	vstmia %1!, {q8-q15}	;"
	"	cmp r11, lr		;"
	"	bne 6b			;"
	"7:	cmp r11, r10		;"
	"	beq 10f			;"
	"	tst %9, #64		;"
	"	beq 8f			;"
	"	vldmia r11!, {q8-q11}	;"	// 32 pixels
	"	vstmia r12!, {q8-q11}	;"
	"	vstmia %1!, {q8-q11}	;"
	"	cmp r11, r10		;"
	"	beq 10f			;"
	"8:	tst %9, #32		;"
	"	beq 9f			;"
	"	vldmia r11!, {q8-q9}	;"	// 16 pixels
	"	vstmia r12!, {q8-q9}	;"
	"	vstmia %1!, {q8-q9}	;"
	"	cmp r11, r10		;"
	"	beq 10f			;"
	"9:	ldrh lr, [r11],#2	;"	// rest
	"	strh lr, [r12],#2	;"
	"	strh lr, [%1],#2	;"
	"	cmp r11, r10		;"
	"	bne 9b			;"
	"10:	add %1, %5		;"
	"	cmp %0, %6		;"
	"	bne 1b			"
	: "+r"(src), "+r"(dst)
	: "r"(swl32), "r"(swl), "r"(sadd), "r"(dadd), "r"(finofs), "r"(dwl128), "r"(dwl), "r"(dwrest)
	: "r10","r11","r12","lr","q8","q9","q10","q11","q12","q13","q14","q15","memory","cc"
	);
}

void scale3x_n32(void* __restrict src, void* __restrict dst, uint32_t sw, uint32_t sh, uint32_t sp, uint32_t dp) {
	if (!sw||!sh) { return; }
	uint32_t swl = sw * sizeof(uint32_t);
	if (!sp) { sp = swl; } if (!dp) { dp = swl*3; }
	if ( ((uintptr_t)src&3)||((uintptr_t)dst&3)||(sp&3)||(dp&3) ) { scale3x_c32(src,dst,sw,sh,sp,dp); return; }
	uint32_t swl32 = swl & ~31;
	uint32_t sadd = sp - swl;
	uint32_t dadd = dp - swl*3;
	uint32_t dwl = swl*3;
	uint32_t dwl128 = dwl & ~127;
	uint32_t dwrest = dwl & 127;
	uint8_t* finofs = (uint8_t*)src + (sp*sh);
	asm volatile (
	"1:	mov r11,%1		;"	// dst push
	"	add lr, %0, %2		;"	// lr = x32bytes offset
	"	add r10, %0, %3		;"	// r10 = lineend offset
	"	cmp %0, lr		;"
	"	beq 3f			;"
	"2:	vldmia %0!,{q8-q9}	;"	// 8 pixels 32 bytes
	"	vdup.32 q15, d19[1]	;"	//  7777
	"	vdup.32 q14, d19[0]	;"	//  6666
	"	vdup.32 q1, d18[1]	;"	//  5555
	"	vdup.32 q0, d18[0]	;"	//  4444
	"	vext.32 q13, q14,q15,#3	;"	// 6777
	"	vext.32 q12, q1,q14,#2	;"	// 5566
	"	vext.32 q11, q0,q1,#1	;"	// 4445
	"	vdup.32 q15, d17[1]	;"	//  3333
	"	vdup.32 q14, d17[0]	;"	//  2222
	"	vdup.32 q1, d16[1]	;"	//  1111
	"	vdup.32 q0, d16[0]	;"	//  0000
	"	vext.32 q10, q14,q15,#3	;"	// 2333
	"	vext.32 q9, q1,q14,#2	;"	// 1122
	"	vext.32 q8, q0,q1,#1	;"	// 0001
	"	cmp %0, lr		;"
	"	vstmia %1!,{q8-q13}	;"
	"	bne 2b			;"
	"3:	cmp %0, r10		;"
	"	beq 5f			;"
	"4:	ldr lr, [%0],#4		;"	// rest
	"	vdup.32 d16, lr		;"
	"	cmp %0, r10		;"
	"	vstmia %1!, {d16}	;"
	"	str lr, [%1],#4		;"
	"	bne 4b			;"
	"5:	add %0, %4		;"
	"	add %1, %5		;"
	"	mov r12, %1		;"	// r12 = 2x line offset
	"	add %1, %8		;"	//
	"	add %1, %5		;"	// %1 = 3x line offset
	"	add lr, r11, %7		;"	// lr = x128bytes offset
	"	add r10, r11, %8	;"	// r10 = lineend offset
	"	cmp r11, lr		;"
	"	beq 7f			;"
	"6:	vldmia r11!, {q8-q15}	;"	// 32 pixels 128 bytes
	"	vstmia r12!, {q8-q15}	;"
	"	vstmia %1!, {q8-q15}	;"
	"	cmp r11, lr		;"
	"	bne 6b			;"
	"7:	cmp r11, r10		;"
	"	beq 10f			;"
	"	tst %9, #64		;"
	"	beq 8f			;"
	"	vldmia r11!, {q8-q11}	;"	// 16 pixels
	"	vstmia r12!, {q8-q11}	;"
	"	vstmia %1!, {q8-q11}	;"
	"	cmp r11, r10		;"
	"	beq 10f			;"
	"8:	tst %9, #32		;"
	"	beq 9f			;"
	"	vldmia r11!, {q8-q9}	;"	// 8 pixels
	"	vstmia r12!, {q8-q9}	;"
	"	vstmia %1!, {q8-q9}	;"
	"	cmp r11, r10		;"
	"	beq 10f			;"
	"9:	ldr lr, [r11],#4	;"	// rest
	"	str lr, [r12],#4	;"
	"	str lr, [%1],#4		;"
	"	cmp r11, r10		;"
	"	bne 9b			;"
	"10:	add %1, %5		;"
	"	cmp %0, %6		;"
	"	bne 1b			"
	: "+r"(src), "+r"(dst)
	: "r"(swl32), "r"(swl), "r"(sadd), "r"(dadd), "r"(finofs), "r"(dwl128), "r"(dwl), "r"(dwrest)
	: "r10","r11","r12","lr","q0","q1","q8","q9","q10","q11","q12","q13","q14","q15","memory","cc"
	);
}

void scale4x_n16(void* __restrict src, void* __restrict dst, uint32_t sw, uint32_t sh, uint32_t sp, uint32_t dp) {
	if (!sw||!sh) { return; }
	uint32_t swl = sw * sizeof(uint16_t);
	if (!sp) { sp = swl; } if (!dp) { dp = swl*4; }
	if ( ((uintptr_t)src&3)||((uintptr_t)dst&3)||(sp&3)||(dp&3) ) { scale4x_c16(src,dst,sw,sh,sp,dp); return; }
	uint32_t swl32 = swl & ~31;
	uint32_t sadd = sp - swl;
	uint32_t dadd = dp*4 - swl*4;
	uint8_t* finofs = (uint8_t*)src + (sp*sh);
	asm volatile (
	"1:	add lr, %0, %2		;"	// lr  = x32bytes offset
	"	add r8, %0, %3		;"	// r8  = lineend offset
	"	add r9, %1, %7		;"	// r9  = 2x line offset
	"	add r10, r9, %7		;"	// r10 = 3x line offset
	"	add r11, r10, %7	;"	// r11 = 4x line offset
	"	cmp %0, lr		;"
	"	beq 3f			;"
	"2:	vldmia %0!,{q8-q9}	;"	// 16 pixels 32 bytes
	"	vdup.16 d31,d19[3]	;"
	"	vdup.16 d30,d19[2]	;"
	"	vdup.16 d29,d19[1]	;"
	"	vdup.16 d28,d19[0]	;"
	"	vdup.16 d27,d18[3]	;"
	"	vdup.16 d26,d18[2]	;"
	"	vdup.16 d25,d18[1]	;"
	"	vdup.16 d24,d18[0]	;"
	"	vdup.16 d23,d17[3]	;"
	"	vdup.16 d22,d17[2]	;"
	"	vdup.16 d21,d17[1]	;"
	"	vdup.16 d20,d17[0]	;"
	"	vdup.16 d19,d16[3]	;"
	"	vdup.16 d18,d16[2]	;"
	"	vdup.16 d17,d16[1]	;"
	"	vdup.16 d16,d16[0]	;"
	"	cmp %0, lr		;"
	"	vstmia %1!,{q8-q15}	;"
	"	vstmia r9!,{q8-q15}	;"
	"	vstmia r10!,{q8-q15}	;"
	"	vstmia r11!,{q8-q15}	;"
	"	bne 2b			;"
	"3:	cmp %0, r8		;"
	"	beq 5f			;"
	"4:	ldrh lr, [%0],#2	;"	// rest
	"	vdup.16 d16, lr		;"
	"	cmp %0, r8		;"
	"	vstmia %1!, {d16}	;"
	"	vstmia r9!, {d16}	;"
	"	vstmia r10!, {d16}	;"
	"	vstmia r11!, {d16}	;"
	"	bne 4b			;"
	"5:	add %0, %0, %4		;"
	"	add %1, %1, %5		;"
	"	cmp %0, %6		;"
	"	bne 1b			"
	: "+r"(src), "+r"(dst)
	: "r"(swl32), "r"(swl), "r"(sadd), "r"(dadd), "r"(finofs), "r"(dp)
	: "r8","r9","r10","r11","lr","q8","q9","q10","q11","q12","q13","q14","q15","memory","cc"
	);
}

void scale4x_n32(void* __restrict src, void* __restrict dst, uint32_t sw, uint32_t sh, uint32_t sp, uint32_t dp) {
	if (!sw||!sh) { return; }
	uint32_t swl = sw * sizeof(uint32_t);
	if (!sp) { sp = swl; } if (!dp) { dp = swl*4; }
	if ( ((uintptr_t)src&3)||((uintptr_t)dst&3)||(sp&3)||(dp&3) ) { scale4x_c32(src,dst,sw,sh,sp,dp); return; }
	uint32_t swl32 = swl & ~31;
	uint32_t sadd = sp - swl;
	uint32_t dadd = dp*4 - swl*4;
	uint8_t* finofs = (uint8_t*)src + (sp*sh);
	asm volatile (
	"1:	add lr, %0, %2		;"	// lr = x32bytes offset
	"	add r8, %0, %3		;"	// r8 = lineend offset
	"	add r9, %1, %7		;"	// r9 = 2x line offset
	"	add r10, r9, %7		;"	// r10 = 3x line offset
	"	add r11, r10, %7	;"	// r11 = 4x line offset
	"	cmp %0, lr		;"
	"	beq 3f			;"
	"2:	vldmia %0!,{q8-q9}	;"	// 8 pixels 32 bytes
	"	vdup.32 q15,d19[1]	;"
	"	vdup.32 q14,d19[0]	;"
	"	vdup.32 q13,d18[1]	;"
	"	vdup.32 q12,d18[0]	;"
	"	vdup.32 q11,d17[1]	;"
	"	vdup.32 q10,d17[0]	;"
	"	vdup.32 q9,d16[1]	;"
	"	vdup.32 q8,d16[0]	;"
	"	cmp %0, lr		;"
	"	vstmia %1!,{q8-q15}	;"
	"	vstmia r9!,{q8-q15}	;"
	"	vstmia r10!,{q8-q15}	;"
	"	vstmia r11!,{q8-q15}	;"
	"	bne 2b			;"
	"3:	cmp %0, r8		;"
	"	beq 5f			;"
	"4:	ldr lr, [%0],#4		;"	// rest
	"	vdup.32 q8, lr		;"
	"	cmp %0, r8		;"
	"	vstmia %1!, {q8}	;"
	"	vstmia r9!, {q8}	;"
	"	vstmia r10!, {q8}	;"
	"	vstmia r11!, {q8}	;"
	"	bne 4b			;"
	"5:	add %0, %0, %4		;"
	"	add %1, %1, %5		;"
	"	cmp %0, %6		;"
	"	bne 1b			"
	: "+r"(src), "+r"(dst)
	: "r"(swl32), "r"(swl), "r"(sadd), "r"(dadd), "r"(finofs), "r"(dp)
	: "r8","r9","r10","r11","lr","q8","q9","q10","q11","q12","q13","q14","q15","memory","cc"
	);
}

static inline void scale5x_n16line(void* src, void* dst, uint32_t swl) {
	asm volatile (
	"	bic r4, %2, #15		;"	// r4 = swl16
	"	add r3, %0, %2		;"	// r3 = lineend offset
	"	add r4, %0, r4		;"	// r4 = x16bytes offset
	"	cmp %0, r4		;"
	"	beq 2f			;"
	"1:	vldmia %0!, {q8}	;"	// 8 pixels 16 bytes
	"	vdup.16 d25, d17[3]	;"	//  7777
	"	vdup.16 d27, d17[2]	;"	//  6666
	"	vdup.16 d26, d17[1]	;"	//  5555
	"	vdup.16 d21, d17[0]	;"	//  4444
	"	vext.16 d24, d27,d25,#1	;"	// 6667
	"	vext.16 d23, d26,d27,#2	;"	// 5566
	"	vext.16 d22, d21,d26,#3	;"	// 4555
	"	vdup.16 d20, d16[3]	;"	//  3333
	"	vdup.16 d27, d16[2]	;"	//  2222
	"	vdup.16 d26, d16[1]	;"	//  1111
	"	vdup.16 d16, d16[0]	;"	//  0000
	"	vext.16 d19, d27,d20,#1	;"	// 2223
	"	vext.16 d18, d26,d27,#2	;"	// 1122
	"	vext.16 d17, d16,d26,#3	;"	// 0111
	"	cmp %0, r4		;"
	"	vstmia %1!, {q8-q12}	;"
	"	bne 1b			;"
	"2:	cmp %0, r3		;"
	"	beq 4f			;"
	"3:	ldrh r4, [%0],#2	;"	// rest
	"	orr r4, r4, lsl #16	;"
	"	cmp %0, r3		;"
	"	str r4, [%1],#4		;"
	"	str r4, [%1],#4		;"
	"	strh r4, [%1],#2	;"
	"	bne 3b			;"
	"4:				"
	: "+r"(src), "+r"(dst)
	: "r"(swl)
	: "r3","r4","q8","q9","q10","q11","q12","q13","memory","cc"
	);
}

void scale5x_n16(void* __restrict src, void* __restrict dst, uint32_t sw, uint32_t sh, uint32_t sp, uint32_t dp) {
	if (!sw||!sh) { return; }
	uint32_t swl = sw * sizeof(uint16_t);
	uint32_t dwl = swl*5;
	if (!sp) { sp = swl; } if (!dp) { dp = dwl; }
	if ( ((uintptr_t)src&3)||((uintptr_t)dst&3)||(sp&3)||(dp&3) ) { scale5x_c16(src,dst,sw,sh,sp,dp); return; }
	void* __restrict dstsrc;
	for (; sh>0; sh--, src=(uint8_t*)src+sp) {
		scale5x_n16line(src, dst, swl);
		dstsrc = dst; dst = (uint8_t*)dst+dp;
		for (uint32_t i=4; i>0; i--, dst=(uint8_t*)dst+dp) memcpy_neon(dst, dstsrc, dwl);
	}
}

static inline void scale5x_n32line(void* src, void* dst, uint32_t swl) {
	asm volatile (
	"	bic r4, %2, #15		;"	// r4 = swl16
	"	add r3, %0, %2		;"	// r3 = lineend offset
	"	add r4, %0, r4		;"	// r4 = x16bytes offset
	"	cmp %0, r4		;"
	"	beq 2f			;"
	"1:	vldmia %0!,{q8}		;"	// 4 pixels 16 bytes
	"	vdup.32 q12, d17[1]	;"	// 3333
	"	vdup.32 q14, d17[0]	;"	//  2222
	"	vdup.32 q13, d16[1]	;"	//  1111
	"	vdup.32 q8, d16[0]	;"	// 0000
	"	vext.32 q11, q14,q12,#1	;"	// 2223
	"	vext.32 q10, q13,q14,#2	;"	// 1122
	"	vext.32 q9, q8,q13,#3	;"	// 0111
	"	cmp %0, r4		;"
	"	vstmia %1!,{q8-q12}	;"
	"	bne 1b			;"
	"2:	cmp %0, r3		;"
	"	beq 4f			;"
	"3:	ldr r4, [%0],#4		;"	// rest
	"	vdup.32 q8, r4		;"
	"	cmp %0, r3		;"
	"	vstmia %1!, {q8}	;"
	"	str r4, [%1],#4		;"
	"	bne 3b			;"
	"4:				"
	: "+r"(src), "+r"(dst)
	: "r"(swl)
	: "r3","r4","q8","q9","q10","q11","q12","q13","q14","memory","cc"
	);
}

void scale5x_n32(void* __restrict src, void* __restrict dst, uint32_t sw, uint32_t sh, uint32_t sp, uint32_t dp) {
	if (!sw||!sh) { return; }
	uint32_t swl = sw * sizeof(uint32_t);
	uint32_t dwl = swl*5;
	if (!sp) { sp = swl; } if (!dp) { dp = dwl; }
	if ( ((uintptr_t)src&3)||((uintptr_t)dst&3)||(sp&3)||(dp&3) ) { scale5x_c32(src,dst,sw,sh,sp,dp); return; }
	void* __restrict dstsrc;
	for (; sh>0; sh--, src=(uint8_t*)src+sp) {
		scale5x_n32line(src, dst, swl);
		dstsrc = dst; dst = (uint8_t*)dst+dp;
		for (uint32_t i=4; i>0; i--, dst=(uint8_t*)dst+dp) memcpy_neon(dst, dstsrc, dwl);
	}
}

static inline void scale6x_n16line(void* src, void* dst, uint32_t swl) {
	asm volatile (
	"	bic r4, %2, #15		;"	// r4 = swl16
	"	add r3, %0, %2		;"	// r3 = lineend offset
	"	add r4, %0, r4		;"	// r4 = x16bytes offset
	"	cmp %0, r4		;"
	"	beq 2f			;"
	"1:	vldmia %0!, {q8}	;"	// 8 pixels 16 bytes
	"	vdup.16 d27, d17[3]	;"	//  7777
	"	vdup.16 d25, d17[2]	;"	//  6666
	"	vdup.16 d24, d17[1]	;"	//  5555
	"	vdup.16 d22, d17[0]	;"	//  4444
	"	vext.16 d26, d25,d27,#2	;"	// 6677
	"	vext.16 d23, d22,d24,#2	;"	// 4455
	"	vdup.16 d21, d16[3]	;"	//  3333
	"	vdup.16 d19, d16[2]	;"	//  2222
	"	vdup.16 d18, d16[1]	;"	//  1111
	"	vdup.16 d16, d16[0]	;"	//  0000
	"	vext.16 d20, d19,d21,#2	;"	// 2233
	"	vext.16 d17, d16,d18,#2	;"	// 0011
	"	cmp %0, r4		;"
	"	vstmia %1!, {q8-q13}	;"
	"	bne 1b			;"
	"2:	cmp %0, r3		;"
	"	beq 4f			;"
	"3:	ldrh r4, [%0],#2	;"	// rest
	"	orr r4, r4, lsl #16	;"
	"	vdup.32 d16, r4		;"
	"	cmp %0, r3		;"
	"	vstmia %1!, {d16}	;"
	"	str r4, [%1],#4		;"
	"	bne 3b			;"
	"4:				"
	: "+r"(src), "+r"(dst)
	: "r"(swl)
	: "r3","r4","q8","q9","q10","q11","q12","q13","memory","cc"
	);
}

void scale6x_n16(void* __restrict src, void* __restrict dst, uint32_t sw, uint32_t sh, uint32_t sp, uint32_t dp) {
	if (!sw||!sh) { return; }
	uint32_t swl = sw * sizeof(uint16_t);
	uint32_t dwl = swl*6;
	if (!sp) { sp = swl; } if (!dp) { dp = dwl; }
	if ( ((uintptr_t)src&3)||((uintptr_t)dst&3)||(sp&3)||(dp&3) ) { scale6x_c16(src,dst,sw,sh,sp,dp); return; }
	void* __restrict dstsrc;
	for (; sh>0; sh--, src=(uint8_t*)src+sp) {
		scale6x_n16line(src, dst, swl);
		dstsrc = dst; dst = (uint8_t*)dst+dp;
		for (uint32_t i=5; i>0; i--, dst=(uint8_t*)dst+dp) memcpy_neon(dst, dstsrc, dwl);
	}
}

static inline void scale6x_n32line(void* src, void* dst, uint32_t swl) {
	asm volatile (
	"	bic r4, %2, #15		;"	// r4 = swl16
	"	add r3, %0, %2		;"	// r3 = lineend offset
	"	add r4, %0, r4		;"	// r4 = x16bytes offset
	"	cmp %0, r4		;"
	"	beq 2f			;"
	"1:	vldmia %0!,{q8}		;"	// 4 pixels 16 bytes
	"	vdup.32 q13, d17[1]	;"	// 3333
	"	vdup.32 q11, d17[0]	;"	// 2222
	"	vdup.32 q10, d16[1]	;"	// 1111
	"	vdup.32 q8, d16[0]	;"	// 0000
	"	vext.32 q12, q11,q13,#2	;"	// 2233
	"	vext.32 q9, q8,q10,#2	;"	// 0011
	"	cmp %0, r4		;"
	"	vstmia %1!,{q8-q13}	;"
	"	bne 1b			;"
	"2:	cmp %0, r3		;"
	"	beq 4f			;"
	"3:	ldr r4, [%0],#4		;"	// rest
	"	vdup.32 q8, r4		;"
	"	vmov d18, d16		;"
	"	cmp %0, r3		;"
	"	vstmia %1!, {d16-d18}	;"
	"	bne 3b			;"
	"4:				"
	: "+r"(src), "+r"(dst)
	: "r"(swl)
	: "r3","r4","q8","q9","q10","q11","q12","q13","memory","cc"
	);
}

void scale6x_n32(void* __restrict src, void* __restrict dst, uint32_t sw, uint32_t sh, uint32_t sp, uint32_t dp) {
	if (!sw||!sh) { return; }
	uint32_t swl = sw * sizeof(uint32_t);
	uint32_t dwl = swl*6;
	if (!sp) { sp = swl; } if (!dp) { dp = dwl; }
	if ( ((uintptr_t)src&3)||((uintptr_t)dst&3)||(sp&3)||(dp&3) ) { scale6x_c32(src,dst,sw,sh,sp,dp); return; }
	void* __restrict dstsrc;
	for (; sh>0; sh--, src=(uint8_t*)src+sp) {
		scale6x_n32line(src, dst, swl);
		dstsrc = dst; dst = (uint8_t*)dst+dp;
		for (uint32_t i=5; i>0; i--, dst=(uint8_t*)dst+dp) memcpy_neon(dst, dstsrc, dwl);
	}
}

//
//	C scalers
//

void scale1x_c16(void* __restrict src, void* __restrict dst, uint32_t sw, uint32_t sh, uint32_t sp, uint32_t dp) {
	if (!sw||!sh) { return; }
	uint32_t swl = sw*sizeof(uint16_t);
	if (!sp) { sp = swl; } if (!dp) { dp = swl*1; }
	if ((swl == sp)&&(sp == dp)) memcpy(dst, src, sp*sh);
	else for (; sh>0; sh--, src=(uint8_t*)src+sp, dst=(uint8_t*)dst+dp) memcpy(dst, src, swl);
}

void scale1x_c32(void* __restrict src, void* __restrict dst, uint32_t sw, uint32_t sh, uint32_t sp, uint32_t dp) {
	if (!sw||!sh) { return; }
	uint32_t swl = sw*sizeof(uint32_t);
	if (!sp) { sp = swl; } if (!dp) { dp = swl*1; }
	if ((swl == sp)&&(sp == dp)) memcpy(dst, src, sp*sh);
	else for (; sh>0; sh--, src=(uint8_t*)src+sp, dst=(uint8_t*)dst+dp) memcpy(dst, src, swl);
}

void scale2x_c16(void* __restrict src, void* __restrict dst, uint32_t sw, uint32_t sh, uint32_t sp, uint32_t dp) {
	if (!sw||!sh) { return; }
	uint32_t x, dx, pix, dpix1, dpix2, swl = sw*sizeof(uint16_t);
	if (!sp) { sp = swl; } swl*=2; if (!dp) { dp = swl; }
	for (; sh>0; sh--, src=(uint8_t*)src+sp, dst=(uint8_t*)dst+dp*2) {
		uint32_t *s = (uint32_t* __restrict)src;
		uint32_t *d = (uint32_t* __restrict)dst;
		for (x=dx=0; x<(sw/2); x++, dx+=2) {
			pix = s[x];
			dpix1=(pix & 0x0000FFFF)|(pix<<16);
			dpix2=(pix & 0xFFFF0000)|(pix>>16);
			d[dx] = dpix1; d[dx+1] = dpix2;
		}
		if (sw&1) {
			uint16_t *s16 = (uint16_t*)s;
			uint16_t pix16 = s16[x*2];
			d[dx] = pix16|(pix16<<16);
		}
		memcpy((uint8_t*)dst+dp*1, dst, swl);
	}
}

void scale2x_c32(void* __restrict src, void* __restrict dst, uint32_t sw, uint32_t sh, uint32_t sp, uint32_t dp) {
	if (!sw||!sh) { return; }
	uint32_t x, dx, pix, swl = sw*sizeof(uint32_t);
	if (!sp) { sp = swl; } swl*=2; if (!dp) { dp = swl; }
	for (; sh>0; sh--, src=(uint8_t*)src+sp, dst=(uint8_t*)dst+dp*2) {
		uint32_t *s = (uint32_t* __restrict)src;
		uint32_t *d = (uint32_t* __restrict)dst;
		for (x=dx=0; x<sw; x++, dx+=2) {
			pix = s[x];
			d[dx] = pix; d[dx+1] = pix;
		}
		memcpy((uint8_t*)dst+dp*1, dst, swl);
	}
}

void scale3x_c16(void* __restrict src, void* __restrict dst, uint32_t sw, uint32_t sh, uint32_t sp, uint32_t dp) {
	if (!sw||!sh) { return; }
	uint32_t x, dx, pix, dpix1, dpix2, swl = sw*sizeof(uint16_t);
	if (!sp) { sp = swl; } swl*=3; if (!dp) { dp = swl; }
	for (; sh>0; sh--, src=(uint8_t*)src+sp, dst=(uint8_t*)dst+dp*3) {
		uint32_t *s = (uint32_t* __restrict)src;
		uint32_t *d = (uint32_t* __restrict)dst;
		for (x=dx=0; x<(sw/2); x++, dx+=3) {
			pix = s[x];
			dpix1=(pix & 0x0000FFFF)|(pix<<16);
			dpix2=(pix & 0xFFFF0000)|(pix>>16);
			d[dx] = dpix1; d[dx+1] = pix; d[dx+2] = dpix2;
		}
		if (sw&1) {
			uint16_t *s16 = (uint16_t*)s;
			uint16_t *d16 = (uint16_t*)d;
			uint16_t pix16 = s16[x*2];
			dpix1 = pix16|(pix16<<16);
			d[dx] = dpix1; d16[(dx+1)*2] = pix16;
		}
		memcpy((uint8_t*)dst+dp*1, dst, swl);
		memcpy((uint8_t*)dst+dp*2, dst, swl);
	}
}

void scale3x_c32(void* __restrict src, void* __restrict dst, uint32_t sw, uint32_t sh, uint32_t sp, uint32_t dp) {
	if (!sw||!sh) { return; }
	uint32_t x, dx, pix, swl = sw*sizeof(uint32_t);
	if (!sp) { sp = swl; } swl*=3; if (!dp) { dp = swl; }
	for (; sh>0; sh--, src=(uint8_t*)src+sp, dst=(uint8_t*)dst+dp*3) {
		uint32_t *s = (uint32_t* __restrict)src;
		uint32_t *d = (uint32_t* __restrict)dst;
		for (x=dx=0; x<sw; x++, dx+=3) {
			pix = s[x];
			d[dx] = pix; d[dx+1] = pix; d[dx+2] = pix;
		}
		memcpy((uint8_t*)dst+dp*1, dst, swl);
		memcpy((uint8_t*)dst+dp*2, dst, swl);
	}
}

void scale4x_c16(void* __restrict src, void* __restrict dst, uint32_t sw, uint32_t sh, uint32_t sp, uint32_t dp) {
	if (!sw||!sh) { return; }
	uint32_t x, dx, pix, dpix1, dpix2, swl = sw*sizeof(uint16_t);
	if (!sp) { sp = swl; } swl*=4; if (!dp) { dp = swl; }
	for (; sh>0; sh--, src=(uint8_t*)src+sp, dst=(uint8_t*)dst+dp*4) {
		uint32_t *s = (uint32_t* __restrict)src;
		uint32_t *d = (uint32_t* __restrict)dst;
		for (x=dx=0; x<(sw/2); x++, dx+=4) {
			pix = s[x];
			dpix1=(pix & 0x0000FFFF)|(pix<<16);
			dpix2=(pix & 0xFFFF0000)|(pix>>16);
			d[dx] = dpix1; d[dx+1] = dpix1; d[dx+2] = dpix2; d[dx+3] = dpix2;
		}
		if (sw&1) {
			uint16_t *s16 = (uint16_t*)s;
			uint16_t pix16 = s16[x*2];
			dpix1 = pix16|(pix16<<16);
			d[dx] = dpix1; d[dx+1] = dpix1;
		}
		memcpy((uint8_t*)dst+dp*1, dst, swl);
		memcpy((uint8_t*)dst+dp*2, dst, swl);
		memcpy((uint8_t*)dst+dp*3, dst, swl);
	}
}

//	faster than 4x_c16 when -Ofast/-O3 and aligned width, however dp must be 4xN
void scale4x_c16b(void* __restrict src, void* __restrict dst, uint32_t sw, uint32_t sh, uint32_t sp, uint32_t dp) {
	if (!sw||!sh) { return; } if (!sp) { sp = sw*sizeof(uint16_t); } if (!dp) { dp = sw*sizeof(uint16_t)*4; }
	uint32_t x, dx, pix, dpix1, dpix2, dp32 = dp / sizeof(uint32_t);
	for (; sh>0; sh--, src=(uint8_t*)src+sp, dst=(uint8_t*)dst+dp*4) {
		uint32_t *s = (uint32_t* __restrict)src;
		uint32_t *d = (uint32_t* __restrict)dst;
		for (x=dx=0; x<(sw/2); x++, dx+=4) {
			pix = s[x];
			dpix1=(pix & 0x0000FFFF)|(pix<<16);
			dpix2=(pix & 0xFFFF0000)|(pix>>16);
			d[dx] = dpix1; d[dx+1] = dpix1; d[dx+2] = dpix2; d[dx+3] = dpix2;
			d[dp32+dx] = dpix1; d[dp32+dx+1]= dpix1; d[dp32+dx+2]= dpix2; d[dp32+dx+3]= dpix2;
			d[dp32*2+dx] = dpix1; d[dp32*2+dx+1]= dpix1; d[dp32*2+dx+2]= dpix2; d[dp32*2+dx+3]= dpix2;
			d[dp32*3+dx] = dpix1; d[dp32*3+dx+1]= dpix1; d[dp32*3+dx+2]= dpix2; d[dp32*3+dx+3]= dpix2;
		}
		if (sw&1) {
			uint16_t *s16 = (uint16_t*)s;
			uint16_t pix16 = s16[x*2];
			dpix1 = pix16|(pix16<<16);
			d[dx] = dpix1; d[dx+1] = dpix1;
			d[dp32+dx] = dpix1; d[dp32+dx+1] = dpix1;
			d[dp32*2+dx] = dpix1; d[dp32*2+dx+1] = dpix1;
			d[dp32*3+dx] = dpix1; d[dp32*3+dx+1] = dpix1;
		}
	}
}

void scale4x_c32(void* __restrict src, void* __restrict dst, uint32_t sw, uint32_t sh, uint32_t sp, uint32_t dp) {
	if (!sw||!sh) { return; }
	uint32_t x, dx, pix, swl = sw*sizeof(uint32_t);
	if (!sp) { sp = swl; } swl*=4; if (!dp) { dp = swl; }
	for (; sh>0; sh--, src=(uint8_t*)src+sp, dst=(uint8_t*)dst+dp*4) {
		uint32_t *s = (uint32_t* __restrict)src;
		uint32_t *d = (uint32_t* __restrict)dst;
		for (x=dx=0; x<sw; x++, dx+=4) {
			pix = s[x];
			d[dx] = pix; d[dx+1] = pix; d[dx+2] = pix; d[dx+3] = pix;
		}
		memcpy((uint8_t*)dst+dp*1, dst, swl);
		memcpy((uint8_t*)dst+dp*2, dst, swl);
		memcpy((uint8_t*)dst+dp*3, dst, swl);
	}
}

//	faster than 4x_c32 when -Ofast/-O3 and aligned width
void scale4x_c32b(void* __restrict src, void* __restrict dst, uint32_t sw, uint32_t sh, uint32_t sp, uint32_t dp) {
	if (!sw||!sh) { return; } if (!sp) { sp = sw*sizeof(uint32_t); } if (!dp) { dp = sw*sizeof(uint32_t)*4; }
	uint32_t x, dx, pix, dp32 = dp / sizeof(uint32_t);
	for (; sh>0; sh--, src=(uint8_t*)src+sp, dst=(uint8_t*)dst+dp*4) {
		uint32_t *s = (uint32_t* __restrict)src;
		uint32_t *d = (uint32_t* __restrict)dst;
		for (x=dx=0; x<sw; x++, dx+=4) {
			pix = s[x];
			d[dx] = pix; d[dx+1] = pix; d[dx+2] = pix; d[dx+3] = pix;
			d[dp32+dx] = pix; d[dp32+dx+1]= pix; d[dp32+dx+2]= pix; d[dp32+dx+3]= pix;
			d[dp32*2+dx] = pix; d[dp32*2+dx+1]= pix; d[dp32*2+dx+2]= pix; d[dp32*2+dx+3]= pix;
			d[dp32*3+dx] = pix; d[dp32*3+dx+1]= pix; d[dp32*3+dx+2]= pix; d[dp32*3+dx+3]= pix;
		}
	}
}

void scale5x_c16(void* __restrict src, void* __restrict dst, uint32_t sw, uint32_t sh, uint32_t sp, uint32_t dp) {
	if (!sw||!sh) { return; }
	uint32_t x, dx, pix, dpix1, dpix2, swl = sw*sizeof(uint16_t);
	if (!sp) { sp = swl; } swl*=5; if (!dp) { dp = swl; }
	for (; sh>0; sh--, src=(uint8_t*)src+sp, dst=(uint8_t*)dst+dp*5) {
		uint32_t *s = (uint32_t* __restrict)src;
		uint32_t *d = (uint32_t* __restrict)dst;
		for (x=dx=0; x<(sw/2); x++, dx+=5) {
			pix = s[x];
			dpix1=(pix & 0x0000FFFF)|(pix<<16);
			dpix2=(pix & 0xFFFF0000)|(pix>>16);
			d[dx] = dpix1; d[dx+1] = dpix1; d[dx+2] = pix; d[dx+3] = dpix2; d[dx+4] = dpix2;
		}
		if (sw&1) {
			uint16_t *s16 = (uint16_t*)s;
			uint16_t *d16 = (uint16_t*)d;
			uint16_t pix16 = s16[x*2];
			dpix1 = pix16|(pix16<<16);
			d[dx] = dpix1; d[dx+1] = dpix1; d16[(dx+2)*2] = pix16;
		}
		memcpy((uint8_t*)dst+dp*1, dst, swl);
		memcpy((uint8_t*)dst+dp*2, dst, swl);
		memcpy((uint8_t*)dst+dp*3, dst, swl);
		memcpy((uint8_t*)dst+dp*4, dst, swl);
	}
}

void scale5x_c32(void* __restrict src, void* __restrict dst, uint32_t sw, uint32_t sh, uint32_t sp, uint32_t dp) {
	if (!sw||!sh) { return; }
	uint32_t x, dx, pix, swl = sw*sizeof(uint32_t);
	if (!sp) { sp = swl; } swl*=5; if (!dp) { dp = swl; }
	for (; sh>0; sh--, src=(uint8_t*)src+sp, dst=(uint8_t*)dst+dp*5) {
		uint32_t *s = (uint32_t* __restrict)src;
		uint32_t *d = (uint32_t* __restrict)dst;
		for (x=dx=0; x<sw; x++, dx+=5) {
			pix = s[x];
			d[dx] = pix; d[dx+1] = pix; d[dx+2] = pix; d[dx+3] = pix; d[dx+4] = pix;
		}
		memcpy((uint8_t*)dst+dp*1, dst, swl);
		memcpy((uint8_t*)dst+dp*2, dst, swl);
		memcpy((uint8_t*)dst+dp*3, dst, swl);
		memcpy((uint8_t*)dst+dp*4, dst, swl);
	}
}

void scale6x_c16(void* __restrict src, void* __restrict dst, uint32_t sw, uint32_t sh, uint32_t sp, uint32_t dp) {
	if (!sw||!sh) { return; }
	uint32_t x, dx, pix, dpix1, dpix2, swl = sw*sizeof(uint16_t);
	if (!sp) { sp = swl; } swl*=6; if (!dp) { dp = swl; }
	for (; sh>0; sh--, src=(uint8_t*)src+sp, dst=(uint8_t*)dst+dp*6) {
		uint32_t *s = (uint32_t* __restrict)src;
		uint32_t *d = (uint32_t* __restrict)dst;
		for (x=dx=0; x<(sw/2); x++, dx+=6) {
			pix = s[x];
			dpix1=(pix & 0x0000FFFF)|(pix<<16);
			dpix2=(pix & 0xFFFF0000)|(pix>>16);
			d[dx] = dpix1; d[dx+1] = dpix1; d[dx+2] = dpix1; d[dx+3] = dpix2; d[dx+4] = dpix2; d[dx+5] = dpix2;
		}
		if (sw&1) {
			uint16_t *s16 = (uint16_t*)s;
			uint16_t pix16 = s16[x*2];
			dpix1 = pix16|(pix16<<16);
			d[dx] = dpix1; d[dx+1] = dpix1; d[dx+2] = dpix1;
		}
		memcpy((uint8_t*)dst+dp*1, dst, swl);
		memcpy((uint8_t*)dst+dp*2, dst, swl);
		memcpy((uint8_t*)dst+dp*3, dst, swl);
		memcpy((uint8_t*)dst+dp*4, dst, swl);
		memcpy((uint8_t*)dst+dp*5, dst, swl);
	}
}

void scale6x_c32(void* __restrict src, void* __restrict dst, uint32_t sw, uint32_t sh, uint32_t sp, uint32_t dp) {
	if (!sw||!sh) { return; }
	uint32_t x, dx, pix, swl = sw*sizeof(uint32_t);
	if (!sp) { sp = swl; } swl*=6; if (!dp) { dp = swl; }
	for (; sh>0; sh--, src=(uint8_t*)src+sp, dst=(uint8_t*)dst+dp*6) {
		uint32_t *s = (uint32_t* __restrict)src;
		uint32_t *d = (uint32_t* __restrict)dst;
		for (x=dx=0; x<sw; x++, dx+=6) {
			pix = s[x];
			d[dx] = pix; d[dx+1] = pix; d[dx+2] = pix; d[dx+3] = pix; d[dx+4] = pix; d[dx+5] = pix;
		}
		memcpy((uint8_t*)dst+dp*1, dst, swl);
		memcpy((uint8_t*)dst+dp*2, dst, swl);
		memcpy((uint8_t*)dst+dp*3, dst, swl);
		memcpy((uint8_t*)dst+dp*4, dst, swl);
		memcpy((uint8_t*)dst+dp*5, dst, swl);
	}
}
