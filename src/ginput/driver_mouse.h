/*
 * This file is subject to the terms of the GFX License. If a copy of
 * the license was not distributed with this file, you can obtain one at:
 *
 *              http://ugfx.org/license.html
 */

/**
 * @file    src/ginput/driver_mouse.h
 * @brief   GINPUT LLD header file for mouse/touch drivers.
 *
 * @defgroup Mouse Mouse
 * @ingroup GINPUT
 * @{
 */

#ifndef _LLD_GINPUT_MOUSE_H
#define _LLD_GINPUT_MOUSE_H

#if GINPUT_NEED_MOUSE || defined(__DOXYGEN__)

typedef struct MouseReading {
	coord_t		x, y, z;
	uint16_t	buttons;
	} MouseReading;

#if !GINPUT_TOUCH_NOCALIBRATE
	typedef struct MouseCalibration {
		float	ax;
		float	bx;
		float	cx;
		float	ay;
		float	by;
		float	cy;
	} MouseCalibration;
#endif

typedef struct MouseInstance {
	struct MouseInstance *				next;				// The next mouse instance
	const struct MOUSEVMT *				vmt;				// The mouse VMT
	MouseReading						r;					// The current position and state
	uint16_t							flags;				// Flags
			#define GMOUSE_FLG_ACTIVE			0x0001				// Mouse is currently active
			#define GMOUSE_FLG_DYNAMIC			0x0002				// Mouse is dynamically allocated
			#define GMOUSE_FLG_CLICK_TIMER		0x0004				// Currently timing a click event
			#define GMOUSE_FLG_INDELTA			0x0008				// Currently in a up/down transition test
			#define GMOUSE_FLG_CLIP				0x0010				// Clip reading to the display
			#define GMOUSE_FLG_CALIBRATE		0x0020				// Calibrate readings
			#define GMOUSE_FLG_CAL_INPROGRESS	0x0040				// Calibrate is currently in progress
			#define GMOUSE_FLG_CAL_SAVED		0x0080				// Calibration has been saved
			#define GMOUSE_FLG_FINGERMODE		0x0100				// Mouse is currently in finger mode
	point								clickpos;			// The position of the last click event
	systemticks_t						clicktime;			// The time of the last click event
	GDisplay *							display;			// The display the mouse is associated with
	void *								param;				// A variable for private driver use
	#if !GINPUT_TOUCH_NOCALIBRATE
		GMouseCalibrationSaveRoutine	fnsavecal;			// The calibration load routine
		GMouseCalibrationLoadRoutine	fnloadcal;			// The calibration save routine
		MouseCalibration				caldata;			// The calibration data
	#endif
} MouseInstance;

typedef struct MouseJitter {
	coord_t		calibrate;									// Maximum error for a calibration to succeed
	coord_t		click;										// Movement allowed without discarding the CLICK or CLICKCXT event
	coord_t		move;										// Movement allowed without discarding the MOVE event
} MouseJitter;

typedef struct MOUSEVMT {
	uint16_t	flags;										// Device flags
		#define GMOUSE_VFLG_TOUCH			0x0001			// This is a touch device (rather than a mouse). Button 1 is calculated from z value.
		#define GMOUSE_VFLG_NOPOLL			0x0002			// Do not poll this device - it is purely interrupt driven
		#define GMOUSE_VFLG_SELFROTATION	0x0004			// This device returns readings that are aligned with the display orientation
		#define GMOUSE_VFLG_DEFAULTFINGER	0x0008			// Default to finger mode
		#define GMOUSE_VFLG_CALIBRATE		0x0010			// This device requires calibration
		#define GMOUSE_VFLG_CAL_EXTREMES	0x0020			// Use edge to edge calibration
		#define GMOUSE_VFLG_CAL_TEST		0x0040			// Test the results of the calibration
		#define GMOUSE_VFLG_ONLY_DOWN		0x0100			// This device returns a valid position only when the mouse is down
		#define GMOUSE_VFLG_POORUPDOWN		0x0200			// Position readings during up/down are unreliable
	coord_t		z_max;										// TOUCH: Maximum possible z value (fully touched)
	coord_t		z_min;										// TOUCH: Minimum possible z value (touch off screen). Note may also be > z_max
	coord_t		z_touchon;									// TOUCH: z values between z_max and this are a solid touch on
	coord_t		z_touchoff;									// TOUCH: z values between z_min and this are a solid touch off

	MouseJitter	pen_jitter;									// PEN MODE: Jitter settings
	MouseJitter	finger_jitter;								// FINGER MODE: Jitter settings

	void (*init)((MouseInstance *pmouse);							// Required
	void (*get)(MouseInstance *pmouse, MouseReading *prd);			// Required
	void (*cal_save)(MouseInstance *pmouse, void *buf, size_t sz);	// Optional
	const char *(*cal_load)(MouseInstance *pmouse);					// Optional: Can return NULL if no data is saved.
																	// 			 Buffer is gfxFree()'d afterwards.
} MOUSEVMT;

#include "ginput_lld_mouse_config.h"

/*===========================================================================*/
/* External declarations.                                                    */
/*===========================================================================*/

#ifdef __cplusplus
extern "C" {
#endif
	/**
	 * @brief	Get a new empty mouse instance and assign it this VMT and display
	 * @note	This routine is provided to low level drivers by the high level code.
	 * @note	This routine is designed for displays that have their own dedicated mouse
	 * 			eg. Win32, X, uGFXnet.
	 * 			The display driver will during initialisation call this routine to associate
	 * 			itself with a mouse.
	 *
	 * @notapi
	 */
	MouseInstance *ginputMouseGetNewMouseForDisplay(const MOUSEVMT *vmt, GDisplay *g);

	/**
	 * @brief	Release a mouse
	 * @note	This routine is provided to low level drivers by the high level code.
	 * @note	This routine is designed for displays that have their own dedicated mouse
	 * 			eg. Win32, X, uGFXnet.
	 * 			When the display has finished with the mouse it can release it.
	 *
	 * @notapi
	 */
	void ginputMouseGetNewMouseForDisplay(MouseIntance *pmouse);

	/**
	 * @brief	Wakeup the high level code so that it attempts another read
	 *
	 * @note	This routine is provided to low level drivers by the high level code
	 *
	 * @notapi
	 */
	void ginputMouseWakeup(void);

	/**
	 * @brief	Wakeup the high level code so that it attempts another read
	 *
	 * @note	This routine is provided to low level drivers by the high level code
	 *
	 * @iclass
	 * @notapi
	 */
	void ginputMouseWakeupI(void);

#ifdef __cplusplus
}
#endif

#endif /* GINPUT_NEED_MOUSE */

#endif /* _LLD_GINPUT_MOUSE_H */
/** @} */
