/*
 * This file is subject to the terms of the GFX License. If a copy of
 * the license was not distributed with this file, you can obtain one at:
 *
 *              http://ugfx.org/license.html
 */

/**
 * @file    src/ginput/sys_options.h
 * @brief   GINPUT sub-system options header file.
 *
 * @addtogroup GINPUT
 * @{
 */

#ifndef _GINPUT_OPTIONS_H
#define _GINPUT_OPTIONS_H

/**
 * @name    GINPUT Functionality to be included
 * @{
 */
	/**
	 * @brief   Should mouse/touch functions be included.
	 * @details	Defaults to FALSE
	 * @note	Also add the a mouse/touch hardware driver to your makefile.
	 * 			Eg.
	 * 				include $(GFXLIB)/drivers/ginput/touch/MCU/ginput_lld.mk
	 */
	#ifndef GINPUT_NEED_MOUSE
		#define GINPUT_NEED_MOUSE		FALSE
	#endif
	/**
	 * @brief   Should keyboard functions be included.
	 * @details	Defaults to FALSE
	 * @note	Also add the a keyboard hardware driver to your makefile.
	 * 			Eg.
	 * 				include $(GFXLIB)/drivers/ginput/keyboard/XXXX/ginput_lld.mk
	 */
	#ifndef GINPUT_NEED_KEYBOARD
		#define GINPUT_NEED_KEYBOARD	FALSE
	#endif
	/**
	 * @brief   Should hardware toggle/switch/button functions be included.
	 * @details	Defaults to FALSE
	 * @note	Also add the a toggle hardware driver to your makefile.
	 * 			Eg.
	 * 				include $(GFXLIB)/drivers/ginput/toggle/Pal/ginput_lld.mk
	 */
	#ifndef GINPUT_NEED_TOGGLE
		#define GINPUT_NEED_TOGGLE		FALSE
	#endif
	/**
	 * @brief   Should analog dial functions be included.
	 * @details	Defaults to FALSE
	 * @note	Also add the a dial hardware driver to your makefile.
	 * 			Eg.
	 * 				include $(GFXLIB)/drivers/ginput/dial/analog/ginput_lld.mk
	 */
	#ifndef GINPUT_NEED_DIAL
		#define GINPUT_NEED_DIAL		FALSE
	#endif
/**
 * @}
 *
 * @name    GINPUT Optional Sizing Parameters
 * @{
 */
/**
 * @}
 *
 * @name    GINPUT Optional Low Level Driver Defines
 * @{
 */
	/**
	 * @brief   Turn off touch mouse support.
	 * @details	Defaults to FALSE
	 * @note	Touch device handling requires a lot of code. If your mouse doesn't require it
	 * 			this can be turned off to save a lot of space.
	 */
	#ifndef GINPUT_TOUCH_NOTOUCH
		#define GINPUT_TOUCH_NOTOUCH					FALSE
	#endif
	/**
	 * @brief   Turn off calibration support.
	 * @details	Defaults to FALSE
	 * @note	Calibration requires a lot of code. If your mouse doesn't require it
	 * 			this can be turned off to save a lot of space.
	 */
	#ifndef GINPUT_TOUCH_NOCALIBRATE
		#define GINPUT_TOUCH_NOCALIBRATE				FALSE
	#endif
	/**
	 * @brief   Milliseconds between mouse polls.
	 * @details	Defaults to 25 millseconds
	 * @note	How often mice should be polled. More often leads to smoother mouse movement
	 * 			but increases CPU usage. If no mouse drivers need polling the poll is not
	 * 			started.
	 */
	#ifndef GINPUT_MOUSE_POLL_PERIOD
		#define GINPUT_MOUSE_POLL_PERIOD				25
	#endif

	/**
	 * @brief   Milliseconds separating a CLICK from a CXTCLICK.
	 * @details	Defaults to 700 millseconds
	 * @note	How long it takes for a click to turn into a CXTCLICK on a touch device.
	 */
	#ifndef GINPUT_TOUCH_CLICK_TIME
		#define GINPUT_TOUCH_CLICK_TIME					700
	#endif
/** @} */

#endif /* _GINPUT_OPTIONS_H */
/** @} */
