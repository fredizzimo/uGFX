/*
 * This file is subject to the terms of the GFX License. If a copy of
 * the license was not distributed with this file, you can obtain one at:
 *
 *              http://ugfx.org/license.html
 */

/**
 * Thread Switching Functions for the Cortex M4 & M7 with hardware floating point
 *
 * Use the EABI calling standard (ARM's AAPCS) - Save r4 - r11 and floating point
 * The context is saved at the current stack location and a pointer is maintained in the thread structure.
 */

#if !CORTEX_USE_FPU
	#warning "GOS Threads: You have specified GFX_CPU=GFX_CPU_CORTX_M?_FP with hardware floating point support but CORTEX_USE_FPU is FALSE. Try using GFX_CPU_GFX_CPU_CORTEX_M? instead"
#endif

#if GFX_COMPILER == GFX_COMPILER_GCC || GFX_COMPILER == GFX_COMPILER_CYGWIN || GFX_COMPILER == GFX_COMPILER_MINGW32 || GFX_COMPILER == GFX_COMPILER_MINGW64
	#define GFX_THREADS_DONE
	#define _gfxThreadsInit()

	static __attribute__((pcs("aapcs-vfp"),naked)) void _gfxTaskSwitch(thread *oldt, thread *newt) {
		__asm__ volatile (	"push	{r4, r5, r6, r7, r8, r9, r10, r11, lr}	\n\t"
							"vpush	{s16-s31}								\n\t"
							"str	sp, %[oldtcxt]							\n\t"
							"ldr	sp, %[newtcxt]							\n\t"
							"vpop	{s16-s31}								\n\t"
							"pop	{r4, r5, r6, r7, r8, r9, r10, r11, pc}	\n\t"
							: [newtcxt] "=m" (newt->cxt)
							: [oldtcxt] "m" (oldt->cxt)
							: "memory");
	}

	static __attribute__((pcs("aapcs-vfp"),naked)) void _gfxStartThread(thread *oldt, thread *newt) {
		newt->cxt = (char *)newt + newt->size;
		__asm__ volatile (	"push	{r4, r5, r6, r7, r8, r9, r10, r11, lr}	\n\t"
							"vpush	{s16-s31}								\n\t"
							"str	sp, %[oldtcxt]							\n\t"
							"ldr	sp, %[newtcxt]							\n\t"
							: [newtcxt] "=m" (newt->cxt)
							: [oldtcxt] "m" (oldt->cxt)
							: "memory");

		// Run the users function
		gfxThreadExit(current->fn(current->param));
	}

#elif GFX_COMPILER == GFX_COMPILER_KEIL || GFX_COMPILER == GFX_COMPILER_ARMCC

	static /*__arm*/ void _gfxTaskSwitch(thread *oldt, thread *newt) {
/*		push	{r4, r5, r6, r7, r8, r9, r10, r11, lr}
		vpush	{s16-s31}
		str	sp, %[oldtcxt]
		ldr	sp, %[newtcxt]
		vpop	{s16-s31}
		pop	{r4, r5, r6, r7, r8, r9, r10, r11, pc}
		: [newtcxt] "=m" (newt->cxt)
		: [oldtcxt] "m" (oldt->cxt)
		: "memory");
*/	}

	static /* __arm */ void _gfxStartThread(thread *oldt, thread *newt) {
		newt->cxt = (char *)newt + newt->size;
/*		push	{r4, r5, r6, r7, r8, r9, r10, r11, lr}
		vpush	{s16-s31}
		str	sp, %[oldtcxt]
		ldr	sp, %[newtcxt]
		: [newtcxt] "=m" (newt->cxt)
		: [oldtcxt] "m" (oldt->cxt)
		: "memory");
*/
		// Run the users function
		gfxThreadExit(current->fn(current->param));
	}

#else
	#warning "GOS: Threads: You have specified a specific CPU but your compiler is not supported. Defaulting to CLIB switching"
#endif
