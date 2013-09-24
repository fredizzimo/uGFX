/*
 * This file is subject to the terms of the GFX License. If a copy of
 * the license was not distributed with this file, you can obtain one at:
 *
 *              http://ugfx.org/license.html
 */

/**
 * @file    include/gmisc/options.h
 * @brief   GMISC - Miscellaneous Routines options header file.
 *
 * @addtogroup GMISC
 * @{
 */

#ifndef _GMISC_OPTIONS_H
#define _GMISC_OPTIONS_H

/**
 * @name    GMISC Functionality to be included
 * @{
 */
	/**
	 * @brief   Include array operation functions
	 * @details	Defaults to FALSE
	 */
	#ifndef GMISC_NEED_ARRAYOPS
		#define GMISC_NEED_ARRAYOPS		FALSE
	#endif
	/**
	 * @brief   Include fast fixed point trig functions (sin, cos)
	 * @details	Defaults to FALSE
	 */
	#ifndef GMISC_NEED_FIXEDTRIG
		#define GMISC_NEED_FIXEDTRIG		FALSE
	#endif
	/**
	 * @brief   Include fast inverse square root (x^-1/2)
	 * @details	Defaults to FALSE
	 */
	#ifndef GMISC_NEED_INVSQRT
		#define GMISC_NEED_INVSQRT		FALSE
	#endif
/**
 * @}
 *
 * @name    GMISC Optional Sizing Parameters
 * @{
 */
/** @} */

#endif /* _GMISC_OPTIONS_H */
/** @} */
