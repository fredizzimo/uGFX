/*
 * This file is subject to the terms of the GFX License. If a copy of
 * the license was not distributed with this file, you can obtain one at:
 *
 *              http://ugfx.org/license.html
 */

#include "../../gfx.h"

#if GFX_USE_OS_NIOS

void _gosHeapInit(void);
void _gosThreadsInit(void);

/*********************************************************
 * Initialise
 *********************************************************/

void _gosInit(void)
{
	// Set up the heap allocator
	_gosHeapInit();

	// Start the scheduler
	_gosThreadsInit();
}

void _gosDeinit(void)
{
}

void gfxHalt(const char *msg)
{
	volatile uint32_t dummy;

	(void)msg;

	while(1) {
		dummy++;
	}
}

void gfxExit(void) {
	volatile uint32_t dummy;

	while(1) {
		dummy++;
	}
}
#include <stdio.h>
systemticks_t gfxSystemTicks(void)
{
	volatile alt_u32 ticks = alt_nticks();

	printf("Ticks: %d\r\n", ticks);
	return ticks;
}

systemticks_t gfxMillisecondsToTicks(delaytime_t ms)
{
	return ms;
}

#endif /* GFX_USE_OS_NIOS */
