/*
 * This file is subject to the terms of the GFX License. If a copy of
 * the license was not distributed with this file, you can obtain one at:
 *
 *              http://ugfx.org/license.html
 */

#include "gfx.h"

#if GFX_USE_GINPUT && GINPUT_NEED_MOUSE

#define GMOUSE_DRIVER_VMT		GMOUSEVMT_FT5336
#include "src/ginput/ginput_driver_mouse.h"

// Get the hardware interface
#include "gmouse_lld_FT5336_board.h"

// Hardware definitions
#include "drivers/ginput/touch/FT5336/ft5336.h"

static bool_t ft5336Init(GMouse* m, unsigned driverinstance)
{
	if (!init_board(m, driverinstance)) {
		return FALSE;
	}

	// Init default values. (From NHD-3.5-320240MF-ATXL-CTP-1 datasheet)
	// Valid touching detect threshold
	write_reg(m, FT5336_TH_GROUP_REG, 0x16);

	// Touch difference threshold
	write_reg(m, FT5336_TH_DIFF_REG, 0xA0);

	// Delay to enter 'Monitor' status (s)
	write_reg(m, FT5336_TIMEENTERMONITOR_REG, 0x0A);

	// Period of 'Active' status (ms)
	write_reg(m, FT5336_PERIODACTIVE_REG, 0x06);

	// Timer to enter 'idle' when in 'Monitor' (ms)
	write_reg(m, FT5336_PERIODMONITOR_REG, 0x28);

	return TRUE;
}

static bool_t ft5336ReadXYZ(GMouse* m, GMouseReading* pdr)
{
	// Assume not touched.
	pdr->buttons = 0;
	pdr->z = 0; 

	// Only take a reading if we are touched.
	if ((read_byte(m, FT5336_TD_STAT_REG) & 0x07)) {

		/* Get the X, Y, Z values */
		pdr->x = (coord_t)(read_word(m, FT5336_P1_XH_REG) & 0x0FFF);
		pdr->y = (coord_t)(read_word(m, FT5336_P1_YH_REG) & 0xFFFF);
		pdr->z = 1;

		// Rescale X,Y if we are using self-calibration
		#if GMOUSE_FT5336_SELF_CALIBRATE
			#if GDISP_NEED_CONTROL
				switch(gdispGGetOrientation(m->display)) {
				default:
				case GDISP_ROTATE_0:
				case GDISP_ROTATE_180:
					pdr->x = gdispGGetWidth(m->display) - pdr->x / (4096/gdispGGetWidth(m->display));
					pdr->y = pdr->y / (4096/gdispGGetHeight(m->display));
					break;
				case GDISP_ROTATE_90:
				case GDISP_ROTATE_270:
					pdr->x = gdispGGetHeight(m->display) - pdr->x / (4096/gdispGGetHeight(m->display));
					pdr->y = pdr->y / (4096/gdispGGetWidth(m->display));
					break;
				}
			#else
				pdr->x = gdispGGetWidth(m->display) - pdr->x / (4096/gdispGGetWidth(m->display));
				pdr->y = pdr->y / (4096/gdispGGetHeight(m->display));
			#endif
		#endif
	}

	return TRUE;
}

const GMouseVMT const GMOUSE_DRIVER_VMT[1] = {{
	{
		GDRIVER_TYPE_TOUCH,
		#if GMOUSE_FT5336_SELF_CALIBRATE
			GMOUSE_VFLG_TOUCH | GMOUSE_VFLG_ONLY_DOWN | GMOUSE_VFLG_POORUPDOWN,
		#else
			GMOUSE_VFLG_TOUCH | GMOUSE_VFLG_ONLY_DOWN | GMOUSE_VFLG_POORUPDOWN | GMOUSE_VFLG_CALIBRATE | GMOUSE_VFLG_CAL_TEST,
		#endif
		sizeof(GMouse) + GMOUSE_FT5336_BOARD_DATA_SIZE,
		_gmouseInitDriver,
		_gmousePostInitDriver,
		_gmouseDeInitDriver
	},
	1,				// z_max - (currently?) not supported
	0,				// z_min - (currently?) not supported
	1,				// z_touchon
	0,				// z_touchoff
	{				// pen_jitter
		GMOUSE_FT5336_PEN_CALIBRATE_ERROR,			// calibrate
		GMOUSE_FT5336_PEN_CLICK_ERROR,				// click
		GMOUSE_FT5336_PEN_MOVE_ERROR				// move
	},
	{				// finger_jitter
		GMOUSE_FT5336_FINGER_CALIBRATE_ERROR,		// calibrate
		GMOUSE_FT5336_FINGER_CLICK_ERROR,			// click
		GMOUSE_FT5336_FINGER_MOVE_ERROR				// move
	},
	ft5336Init, 	// init
	0,				// deinit
	ft5336ReadXYZ,	// get
	0,				// calsave
	0				// calload
}};

#endif /* GFX_USE_GINPUT && GINPUT_NEED_MOUSE */
