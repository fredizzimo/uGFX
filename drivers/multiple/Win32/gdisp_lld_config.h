/*
 * This file is subject to the terms of the GFX License. If a copy of
 * the license was not distributed with this file, you can obtain one at:
 *
 *              http://ugfx.org/license.html
 */

/**
 * @file    drivers/multiple/Win32/gdisp_lld_config.h
 * @brief   GDISP Graphic Driver subsystem low level driver header for Win32.
 *
 * @addtogroup GDISP
 * @{
 */

#ifndef _GDISP_LLD_CONFIG_H
#define _GDISP_LLD_CONFIG_H

#if GFX_USE_GDISP

/*===========================================================================*/
/* Driver hardware support.                                                  */
/*===========================================================================*/

// Calling gdispGFlush() is optional for this driver but can be used by the
//	application to force a display update. eg after streaming.

#define GDISP_HARDWARE_FLUSH			TRUE
#define GDISP_HARDWARE_CONTROL			TRUE

//#define GDISP_WIN32_STREAMING_TEST
#ifdef GDISP_WIN32_STREAMING_TEST
	// These streaming routines are here only to debug the high level gdisp
	//	code for streaming controllers. They are slow, inefficient and have
	//	lots of debugging turned on.
	#define GDISP_HARDWARE_STREAM_WRITE		TRUE
	#define GDISP_HARDWARE_STREAM_READ		TRUE
	#define GDISP_HARDWARE_STREAM_POS		TRUE
#else
	// The proper way on the Win32. These routines are nice and fast.
	#define GDISP_HARDWARE_DRAWPIXEL		TRUE
	#define GDISP_HARDWARE_FILLS			TRUE
	#define GDISP_HARDWARE_PIXELREAD		TRUE
	#define GDISP_HARDWARE_BITFILLS			TRUE
	#define GDISP_HARDWARE_SCROLL			TRUE
#endif

#define GDISP_PIXELFORMAT				GDISP_PIXELFORMAT_RGB888

#endif	/* GFX_USE_GDISP */

#endif	/* _GDISP_LLD_CONFIG_H */
/** @} */

