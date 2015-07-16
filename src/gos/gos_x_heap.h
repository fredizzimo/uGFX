/*
 * This file is subject to the terms of the GFX License. If a copy of
 * the license was not distributed with this file, you can obtain one at:
 *
 *              http://ugfx.org/license.html
 */

/**
 * The raw32 GOS implementation supports any 32 bit processor with or without an
 * 	underlying operating system. It uses cooperative multi-tasking. Be careful
 * 	when writing device drivers not to disturb the assumptions this creates by performing
 * 	call-backs to uGFX code unless you define the INTERRUPTS_OFF() and INTERRUPTS_ON() macros.
 * 	It still requires some C runtime library support...
 * 		enough startup to initialise the stack, interrupts, static data etc and call main().
 * 		setjmp() and longjmp()			- for threading
 * 		memcpy()						- for heap and threading
 * 		malloc(), realloc and free()	- if GFX_OS_HEAP_SIZE == 0
 *
 * 	You must also define the following routines in your own code so that timing functions will work...
 * 		systemticks_t gfxSystemTicks(void);
 *		systemticks_t gfxMillisecondsToTicks(delaytime_t ms);
 */
#ifndef _GOS_X_HEAP_H
#define _GOS_X_HEAP_H

#if GOS_NEED_X_HEAP


/*===========================================================================*/
/* Special Macros								                             */
/*===========================================================================*/

/**
 * @brief	Set the maximum size of the heap.
 * @note	If set to 0 then the C runtime library malloc() and free() are used.
 */
#ifndef GFX_OS_HEAP_SIZE
	#define GFX_OS_HEAP_SIZE	0
#endif

/*===========================================================================*/
/* Type definitions                                                          */
/*===========================================================================*/

#ifdef __cplusplus
extern "C" {
#endif

	#if GFX_OS_HEAP_SIZE != 0
		void gfxAddHeapBlock(void *ptr, size_t sz);
	#endif

	void *gfxAlloc(size_t sz);
	void *gfxRealloc(void *ptr, size_t oldsz, size_t newsz);
	void gfxFree(void *ptr);

#ifdef __cplusplus
}
#endif

#endif /* GOS_NEED_X_HEAP */
#endif /* _GOS_X_HEAP_H */
