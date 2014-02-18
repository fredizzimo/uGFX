/*
 * This file is subject to the terms of the GFX License. If a copy of
 * the license was not distributed with this file, you can obtain one at:
 *
 *              http://ugfx.org/license.html
 */

/**
 * @file    drivers/ginput/touch/MCU/ginput_lld_mouse.c
 * @brief   GINPUT Touch low level driver source for the MCU.
 *
 * @defgroup Mouse Mouse
 * @ingroup GINPUT
 *
 * @{
 */

#include "gfx.h"

#if (GFX_USE_GINPUT && GINPUT_NEED_MOUSE) /*|| defined(__DOXYGEN__)*/

#include "src/ginput/driver_mouse.h"

#include "ginput_lld_mouse_board.h"

static uint16_t sampleBuf[7];

/**
 * @brief   7-point median filtering code for touchscreen samples
 *
 * @note    This is an internally used routine only.
 *
 * @notapi
 */
static void filter(void) {
	uint16_t temp;
	int i,j;

	for(i = 0; i < 4; i++) {
		for(j = i; j < 7; j++) {
			if(sampleBuf[i] > sampleBuf[j]) {
				/* Swap the values */
				temp = sampleBuf[i];
				sampleBuf[i] = sampleBuf[j];
				sampleBuf[j] = temp;
			}
		}
	}
}

/**
 * @brief   Initialise the mouse/touch.
 *
 * @notapi
 */
void ginput_lld_mouse_init(void) {
	init_board();
}

/**
 * @brief   Read the mouse/touch position.
 *
 * @param[in] pt	A pointer to the structure to fill
 *
 * @note			We use a 7 sample medium filter on each coordinate to remove analogue noise.
 * @note			During touch transition the ADC can return some very strange
 * 					results. To fix this behaviour we don't return until
 * 					we have tested the touch is in the same state at both the beginning
 * 					and the end of the reading.
 * @note			Whilst x and y can return readings in any range so long as it fits in 16 bits,
 * 					the z value must be ranged by the board file to be a rough percentage. Anything
 * 					greater than 80% pressure is a touch.
 *
 * @notapi
 */
void ginput_lld_mouse_get_reading(MouseReading *pt) {
	uint16_t i;

	// Obtain access to the bus
	aquire_bus();

	// Read the ADC for z, x, y and then z again
	while(1) {
		setup_z();
		for(i = 0; i < 7; i++)
			sampleBuf[i] = read_z();
		filter();
		pt->z = (coord_t)sampleBuf[3];

		setup_x();
		for(i = 0; i < 7; i++)
			sampleBuf[i] = read_x();
		filter();
		pt->x = (coord_t)sampleBuf[3];

		setup_y();
		for(i = 0; i < 7; i++)
			sampleBuf[i] = read_y();
		filter();
		pt->y = (coord_t)sampleBuf[3];

		pt->buttons = pt->z >= 80 ? GINPUT_TOUCH_PRESSED : 0;

		setup_z();
		for(i = 0; i < 7; i++)
			sampleBuf[i] = read_z();
		filter();
		i = (coord_t)sampleBuf[3] >= 80 ? GINPUT_TOUCH_PRESSED : 0;

		if (pt->buttons == i)
			break;
	}

	// Release the bus
	release_bus();
}

#endif /* GFX_USE_GINPUT && GINPUT_NEED_MOUSE */
/** @} */
