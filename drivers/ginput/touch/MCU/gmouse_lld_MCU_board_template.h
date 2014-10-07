/*
 * This file is subject to the terms of the GFX License. If a copy of
 * the license was not distributed with this file, you can obtain one at:
 *
 *              http://ugfx.org/license.html
 */

#ifndef _LLD_GMOUSE_MCU_BOARD_H
#define _LLD_GMOUSE_MCU_BOARD_H

// Either define your jitter settings here or define them in the config include file
#if 0
	#include "gmouse_lld_MCU_config.h"
#else
	#define GMOUSE_MCU_PEN_CALIBRATE_ERROR		2
	#define GMOUSE_MCU_PEN_CLICK_ERROR			2
	#define GMOUSE_MCU_PEN_MOVE_ERROR			2
	#define GMOUSE_MCU_FINGER_CALIBRATE_ERROR	4
	#define GMOUSE_MCU_FINGER_CLICK_ERROR		4
	#define GMOUSE_MCU_FINGER_MOVE_ERROR		4
#endif

// Now board specific settings...

#define BOARD_DATA_SIZE		0			// How many extra bytes to add on the end of the mouse structure for the board's use

#define Z_MIN				0			// The minimum Z reading
#define Z_MAX				100			// The maximum Z reading
#define Z_TOUCHON			80			// Values between this and Z_MAX are definitely pressed
#define Z_TOUCHOFF			70			// Values between this and Z_MIN are definitely not pressed

static bool_t init_board(GMouse *m, unsigned driverinstance) {
}

static void read_xyz(GMouse *m, GMouseReading *prd) {
}

#endif /* _LLD_GMOUSE_MCU_BOARD_H */
