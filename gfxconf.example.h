/**
 * This file has a different license to the rest of the GFX system.
 * You can copy, modify and distribute this file as you see fit.
 * You do not need to publish your source modifications to this file.
 * The only thing you are not permitted to do is to relicense it
 * under a different license.
 */

/**
 * Copy this file into your project directory and rename it as gfxconf.h
 * Edit your copy to turn on the GFX features you want to use.
 */

#ifndef _GFXCONF_H
#define _GFXCONF_H

/* The operating system to use. One of these must be defined - preferably in your Makefile */
//#define GFX_USE_OS_CHIBIOS	FALSE
//#define GFX_USE_OS_WIN32		FALSE
//#define GFX_USE_OS_LINUX		FALSE
//#define GFX_USE_OS_OSX		FALSE

/* GFX subsystems to turn on */
#define GFX_USE_GDISP			FALSE
#define GFX_USE_TDISP			FALSE
#define GFX_USE_GWIN			FALSE
#define GFX_USE_GEVENT			FALSE
#define GFX_USE_GTIMER			FALSE
#define GFX_USE_GQUEUE			FALSE
#define GFX_USE_GINPUT			FALSE
#define GFX_USE_GADC			FALSE
#define GFX_USE_GAUDIN			FALSE
#define GFX_USE_GAUDOUT			FALSE
#define GFX_USE_GMISC			FALSE

/* Features for the GDISP subsystem */
#define GDISP_NEED_AUTOFLUSH		FALSE
#define GDISP_NEED_TIMERFLUSH		FALSE
#define GDISP_NEED_VALIDATION		TRUE
#define GDISP_NEED_CLIP				TRUE
#define GDISP_NEED_TEXT				TRUE
#define GDISP_NEED_CIRCLE			FALSE
#define GDISP_NEED_ELLIPSE			FALSE
#define GDISP_NEED_ARC				FALSE
#define GDISP_NEED_CONVEX_POLYGON	FALSE
#define GDISP_NEED_SCROLL			FALSE
#define GDISP_NEED_PIXELREAD		FALSE
#define GDISP_NEED_CONTROL			FALSE
#define GDISP_NEED_QUERY			FALSE
#define GDISP_NEED_IMAGE			FALSE
#define GDISP_NEED_MULTITHREAD		FALSE
#define GDISP_NEED_STREAMING		FALSE

/* GDISP - text features */
#define GDISP_NEED_ANTIALIAS		FALSE
#define GDISP_NEED_UTF8				FALSE
#define GDISP_NEED_TEXT_KERNING		FALSE

/* GDISP - fonts to include */
#define GDISP_INCLUDE_FONT_UI1					FALSE
#define GDISP_INCLUDE_FONT_UI2					FALSE
#define GDISP_INCLUDE_FONT_LARGENUMBERS			FALSE
#define GDISP_INCLUDE_FONT_DEJAVUSANS10			FALSE
#define GDISP_INCLUDE_FONT_DEJAVUSANS12			FALSE
#define GDISP_INCLUDE_FONT_DEJAVUSANS16			FALSE
#define GDISP_INCLUDE_FONT_DEJAVUSANS24			FALSE
#define GDISP_INCLUDE_FONT_DEJAVUSANS32			FALSE
#define GDISP_INCLUDE_FONT_DEJAVUSANSBOLD12		FALSE
#define GDISP_INCLUDE_FONT_FIXED_10x20			FALSE
#define GDISP_INCLUDE_FONT_FIXED_7x14			FALSE
#define GDISP_INCLUDE_FONT_FIXED_5x8			FALSE
#define GDISP_INCLUDE_FONT_DEJAVUSANS12_AA		FALSE
#define GDISP_INCLUDE_FONT_DEJAVUSANS16_AA		FALSE
#define GDISP_INCLUDE_FONT_DEJAVUSANS24_AA		FALSE
#define GDISP_INCLUDE_FONT_DEJAVUSANS32_AA		FALSE
#define GDISP_INCLUDE_FONT_DEJAVUSANSBOLD12_AA	FALSE
#define GDISP_INCLUDE_USER_FONTS				FALSE

/* GDISP image decoders */
#define GDISP_NEED_IMAGE_NATIVE		FALSE
#define GDISP_NEED_IMAGE_GIF		FALSE
#define GDISP_NEED_IMAGE_BMP		FALSE
#define GDISP_NEED_IMAGE_JPG		FALSE
#define GDISP_NEED_IMAGE_PNG		FALSE
#define GDISP_NEED_IMAGE_ACCOUNTING	FALSE

/* Optional image support that can be turned off */
/*
	#define GDISP_NEED_IMAGE_BMP_1		TRUE
	#define GDISP_NEED_IMAGE_BMP_4		TRUE
	#define GDISP_NEED_IMAGE_BMP_4_RLE	TRUE
	#define GDISP_NEED_IMAGE_BMP_8		TRUE
	#define GDISP_NEED_IMAGE_BMP_8_RLE	TRUE
	#define GDISP_NEED_IMAGE_BMP_16		TRUE
	#define GDISP_NEED_IMAGE_BMP_24		TRUE
	#define GDISP_NEED_IMAGE_BMP_32		TRUE
*/

/* Features for the TDISP subsystem. */
#define TDISP_NEED_MULTITHREAD	FALSE

/* Features for the GWIN subsystem. */
#define GWIN_NEED_WINDOWMANAGER	FALSE
#define GWIN_NEED_CONSOLE		FALSE
#define GWIN_NEED_GRAPH			FALSE
#define GWIN_NEED_WIDGET		FALSE
#define GWIN_NEED_LABEL			FALSE
#define GWIN_NEED_BUTTON		FALSE
#define GWIN_NEED_SLIDER		FALSE
#define GWIN_NEED_CHECKBOX		FALSE
#define GWIN_NEED_IMAGE			FALSE
#define GWIN_NEED_RADIO			FALSE
#define GWIN_NEED_LIST			FALSE

/* Features for the GEVENT subsystem. */
#define GEVENT_ASSERT_NO_RESOURCE	FALSE

/* Features for the GTIMER subsystem. */
/* NONE */

/* Features for the GQUEUE subsystem. */
#define GQUEUE_NEED_ASYNC		FALSE
#define GQUEUE_NEED_GSYNC		FALSE
#define GQUEUE_NEED_FSYNC		FALSE

/* Features for the GINPUT subsystem. */
#define GINPUT_NEED_MOUSE		FALSE
#define GINPUT_NEED_KEYBOARD	FALSE
#define GINPUT_NEED_TOGGLE		FALSE
#define GINPUT_NEED_DIAL		FALSE

/* Features for the GADC subsystem. */
/* NONE */

/* Features for the GAUDIN subsystem. */
/* NONE */

/* Features for the GAUDOUT subsystem. */
/* NONE */

/* Features for the GMISC subsystem. */
#define GMISC_NEED_ARRAYOPS		FALSE
#define GMISC_NEED_FASTTRIG		FALSE
#define GMISC_NEED_FIXEDTRIG	FALSE
#define GMISC_NEED_INVSQRT		FALSE

/* Optional Multiple Display support */
/*
	#define GDISP_TOTAL_DISPLAYS			1
	#define GDISP_TOTAL_CONTROLLERS			1

	// Extra stuff required when GDISP_TOTAL_CONTROLLERS > 1
	#define GDISP_CONTROLLER_LIST			GDISPVMT_Win32, GDISPVMT_Win32
	#define GDISP_CONTROLLER_DISPLAYS		1, 1
	#define GDISP_PIXELFORMAT				GDISP_PIXELFORMAT_RGB888

	// Optional extra stuff when GDISP_TOTAL_CONTROLLERS > 1
	#define GDISP_HARDWARE_STREAM_WRITE		FALSE
	#define GDISP_HARDWARE_STREAM_READ		FALSE
	#define GDISP_HARDWARE_STREAM_POS		FALSE
	#define GDISP_HARDWARE_DRAWPIXEL		FALSE
	#define GDISP_HARDWARE_CLEARS			FALSE
	#define GDISP_HARDWARE_FILLS			FALSE
	#define GDISP_HARDWARE_BITFILLS			FALSE
	#define GDISP_HARDWARE_SCROLL			FALSE
	#define GDISP_HARDWARE_PIXELREAD		FALSE
	#define GDISP_HARDWARE_CONTROL			FALSE
	#define GDISP_HARDWARE_QUERY			FALSE
	#define GDISP_HARDWARE_CLIP				FALSE
*/

/* Optional Parameters for various subsystems */
/*
	#define GDISP_DEFAULT_ORIENTATION		GDISP_ROTATE_LANDSCAPE
	#define GDISP_LINEBUF_SIZE				128
	#define GEVENT_MAXIMUM_SIZE				32
	#define GEVENT_MAX_SOURCE_LISTENERS		32
	#define GTIMER_THREAD_PRIORITY			HIGH_PRIORITY
	#define GTIMER_THREAD_WORKAREA_SIZE		1024
	#define GADC_MAX_LOWSPEED_DEVICES		4
	#define GWIN_BUTTON_LAZY_RELEASE		FALSE
	#define GWIN_CONSOLE_USE_BASESTREAM		FALSE
	#define GWIN_CONSOLE_USE_FLOAT			FALSE
	#define GWIN_NEED_IMAGE_ANIMATION		FALSE
	#define GMISC_INVSQRT_MIXED_ENDIAN		FALSE
	#define GMISC_INVSQRT_REAL_SLOW			FALSE
*/

/* Optional Low Level Driver Definitions */
/*
	#define GDISP_SCREEN_WIDTH			320
	#define GDISP_SCREEN_HEIGHT			240
	#define TDISP_COLUMNS				16
	#define TDISP_ROWS					2
*/

#endif /* _GFXCONF_H */
