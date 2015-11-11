/*
 * This file is subject to the terms of the GFX License. If a copy of
 * the license was not distributed with this file, you can obtain one at:
 *
 *              http://ugfx.org/license.html
 */

/**
 * @file    gfx.h
 * @brief   GFX system header file.
 *
 * @addtogroup GFX
 *
 * @brief	Main module to glue all the others together
 *
 * @{
 */

#ifndef _GFX_H
#define _GFX_H

/**
 * These two definitions below are required before anything else so that we can
 * turn module definitions off and on.
 */

/**
 * @brief   Generic 'false' boolean constant.
 */
#if !defined(FALSE) || defined(__DOXYGEN__)
	#define FALSE       0
#endif

/**
 * @brief   Generic 'true' boolean constant.
 */
#if !defined(TRUE) || defined(__DOXYGEN__)
	#define TRUE        -1
#endif

/**
 * @brief   Mark a function as deprecated.
 */
#ifndef DEPRECATED
	#if defined(__GNUC__) || defined(__MINGW32_) || defined(__CYGWIN__)
		#define DEPRECATED(msg)		__attribute__((deprecated(msg)))
	#elif defined(_MSC_VER)
		#define DEPRECATED(msg)		__declspec(deprecated(msg))
	#else
		#define DEPRECATED(msg)
	#endif
#endif

/* gfxconf.h is the user's project configuration for the GFX system. */
#include "gfxconf.h"

/**
 * @name    GFX sub-systems that can be turned on
 * @{
 */
	/**
	 * @brief   GFX Driver API
	 * @details	Defaults to TRUE
	 * @note	Not much useful can be done without a driver
	 */
	#ifndef GFX_USE_GDRIVER
		#define GFX_USE_GDRIVER	TRUE
	#endif
	/**
	 * @brief   GFX Graphics Display Basic API
	 * @details	Defaults to FALSE
	 * @note	Also add the specific hardware driver to your makefile.
	 * 			Eg.  include $(GFXLIB)/drivers/gdisp/Nokia6610/driver.mk
	 */
	#ifndef GFX_USE_GDISP
		#define GFX_USE_GDISP	FALSE
	#endif
	/**
	 * @brief   GFX Graphics Windowing API
	 * @details	Defaults to FALSE
	 * @details	Extends the GDISP API to add the concept of graphic windows.
	 * @note	Also supports high-level "window" objects such as console windows,
	 * 			buttons, graphing etc
	 */
	#ifndef GFX_USE_GWIN
		#define GFX_USE_GWIN	FALSE
	#endif
	/**
	 * @brief   GFX Event API
	 * @details	Defaults to FALSE
	 * @details	Defines the concept of a "Source" that can send "Events" to "Listeners".
	 */
	#ifndef GFX_USE_GEVENT
		#define GFX_USE_GEVENT	FALSE
	#endif
	/**
	 * @brief   GFX Timer API
	 * @details	Defaults to FALSE
	 * @details	Provides thread context timers - both one-shot and periodic.
	 */
	#ifndef GFX_USE_GTIMER
		#define GFX_USE_GTIMER	FALSE
	#endif
	/**
	 * @brief   GFX Queue API
	 * @details	Defaults to FALSE
	 * @details	Provides queue management.
	 */
	#ifndef GFX_USE_GQUEUE
		#define GFX_USE_GQUEUE	FALSE
	#endif
	/**
	 * @brief   GFX Input Device API
	 * @details	Defaults to FALSE
	 * @note	Also add the specific hardware drivers to your makefile.
	 * 			Eg.
	 * 				include $(GFXLIB)/drivers/ginput/toggle/Pal/driver.mk
	 * 			and...
	 * 				include $(GFXLIB)/drivers/ginput/touch/MCU/driver.mk
	 */
	#ifndef GFX_USE_GINPUT
		#define GFX_USE_GINPUT	FALSE
	#endif
	/**
	 * @brief   GFX Generic Periodic ADC API
	 * @details	Defaults to FALSE
	 */
	#ifndef GFX_USE_GADC
		#define GFX_USE_GADC	FALSE
	#endif
	/**
	 * @brief   GFX Audio API
	 * @details	Defaults to FALSE
	 * @note	Also add the specific hardware drivers to your makefile.
	 * 			Eg.
	 * 				include $(GFXLIB)/drivers/gaudio/GADC/driver.mk
	 */
	#ifndef GFX_USE_GAUDIO
		#define GFX_USE_GAUDIO	FALSE
	#endif
	/**
	 * @brief   GFX Miscellaneous Routines API
	 * @details	Defaults to FALSE
	 * @note	Turning this on without turning on any GMISC_NEED_xxx macros will result
	 * 			in no extra code being compiled in. GMISC is made up from the sum of its
	 * 			parts.
	 */
	#ifndef GFX_USE_GMISC
		#define GFX_USE_GMISC	FALSE
	#endif
	/**
	 * @brief   GFX File API
	 * @details	Defaults to FALSE
	 */
	#ifndef GFX_USE_GFILE
		#define GFX_USE_GFILE	FALSE
	#endif
/** @} */

/**
 * @name    GFX system-wide options
 * @{
 */
 	/**
	 * @brief	Should various inline ugfx functions be non-inline.
	 * @details	Defaults to FALSE
	 * @note	Generally there is no need to set this to TRUE as it will have huge performance impacts
	 *			in the driver level.
	 */
	#ifndef GFX_NO_INLINE
		#define GFX_NO_INLINE			FALSE
	#endif
	/**
	 * @brief	Enable compiler specific code
	 * @details	Defaults to GFX_COMPILER_UNKNOWN
	 * @note	This is setting enables optimisations that are compiler specific. It does
	 * 			not need to be specified as reasonable defaults and various auto-detection
	 * 			will happen as required.
	 * @note	Currently only used by ugfx generic thread handling (GOS_USE_OS_RAW32 and GOS_USE_OS_ARDUINO)
	 */
	#ifndef GFX_COMPILER
		#define GFX_COMPILER			GFX_COMPILER_UNKNOWN
	#endif
	#define GFX_COMPILER_UNKNOWN		0		// Unknown compiler
	#define GFX_COMPILER_MINGW32		1		// MingW32 (x86) compiler for windows
	/**
	 * @brief	Enable cpu specific code
	 * @details	Defaults to GFX_CPU_UNKNOWN
	 * @note	This is setting enables optimisations that are cpu specific. It does
	 * 			not need to be specified as reasonable defaults and various auto-detection
	 * 			will happen as required.
	 * @note	Currently only used by ugfx generic thread handling (GOS_USE_OS_RAW32 and GOS_USE_OS_ARDUINO)
	 * @{
	 */
	#ifndef GFX_CPU
		#define GFX_CPU					GFX_CPU_UNKNOWN
	#endif
	#define GFX_CPU_UNKNOWN				0		//**< Unknown cpu
	#define GFX_CPU_CORTEX_M0			1		//**< Cortex M0
	#define GFX_CPU_CORTEX_M1			2		//**< Cortex M1
	#define GFX_CPU_CORTEX_M2			3		//**< Cortex M2
	#define GFX_CPU_CORTEX_M3			4		//**< Cortex M3
	#define GFX_CPU_CORTEX_M4			5		//**< Cortex M4
	#define GFX_CPU_CORTEX_M4_FP		6		//**< Cortex M4 with hardware floating point
	#define GFX_CPU_CORTEX_M7			7		//**< Cortex M7
	#define GFX_CPU_CORTEX_M7_FP		8		//**< Cortex M7 with hardware floating point
	/** @} */
	/**
	 * @brief   Does this CPU generate no alignment faults
	 * @details	Defaults to FALSE
	 * @note	Turning this on can increase code size and speed but
	 * 			should not be turned on with a CPU that can generate
	 * 			alignment segfaults.
	 * @note	If you are unsure leave this as FALSE as that generates
	 * 			the more conservative code.
	 */
	#ifndef GFX_CPU_NO_ALIGNMENT_FAULTS
		#define GFX_CPU_NO_ALIGNMENT_FAULTS		FALSE
	#endif
	/**
	 * @brief	What is the CPU endianness
	 * @details	Defaults to GFX_CPU_ENDIAN_UNKNOWN
	 * @note	This is setting enables optimisations that are cpu endian specific. It does
	 * 			not need to be specified as reasonable defaults and various auto-detection
	 * 			will happen as required.
	 * @{
	 */
	#ifndef GFX_CPU_ENDIAN
		#define GFX_CPU_ENDIAN			GFX_CPU_ENDIAN_UNKNOWN
	#endif
	#define GFX_CPU_ENDIAN_UNKNOWN		0				//**< Unknown endianness
	#define GFX_CPU_ENDIAN_LITTLE		0x04030201		//**< Little Endian
	#define GFX_CPU_ENDIAN_BIG			0x01020304		//**< Big Endian
	/** @} */

/** @} */


#if GFX_NO_INLINE
	#define GFXINLINE
#else
	#if defined(__KEIL__) || defined(__C51__)
		#define GFXINLINE	__inline
	#else
		#define GFXINLINE	inline
	#endif
#endif

#if GFX_CPU_ENDIAN == GFX_CPU_ENDIAN_UNKNOWN
	#if (defined(__BYTE_ORDER__)&&(__BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__)) \
			|| defined(__LITTLE_ENDIAN__) \
			|| defined(__LITTLE_ENDIAN) \
			|| defined(_LITTLE_ENDIAN)
		#undef GFX_CPU_ENDIAN
		#define GFX_CPU_ENDIAN			GFX_CPU_ENDIAN_LITTLE
	#elif (defined(__BYTE_ORDER__)&&(__BYTE_ORDER__ == __ORDER_BIG_ENDIAN__)) \
			|| defined(__BIG_ENDIAN__) \
			|| defined(__BIG_ENDIAN) \
			|| defined(_BIG_ENDIAN)
		#undef GFX_CPU_ENDIAN
		#define GFX_CPU_ENDIAN			GFX_CPU_ENDIAN_BIG
	#endif
#endif

/**
 * Get all the options for each sub-system.
 *
 */
#include "src/gos/gos_options.h"
#include "src/gdriver/gdriver_options.h"
#include "src/gfile/gfile_options.h"
#include "src/gmisc/gmisc_options.h"
#include "src/gqueue/gqueue_options.h"
#include "src/gevent/gevent_options.h"
#include "src/gtimer/gtimer_options.h"
#include "src/gdisp/gdisp_options.h"
#include "src/gwin/gwin_options.h"
#include "src/ginput/ginput_options.h"
#include "src/gadc/gadc_options.h"
#include "src/gaudio/gaudio_options.h"

/**
 * Interdependency safety checks on the sub-systems.
 * These must be in dependency order.
 *
 */
#ifndef GFX_DISPLAY_RULE_WARNINGS
	#define GFX_DISPLAY_RULE_WARNINGS	FALSE
#endif
#include "src/gwin/gwin_rules.h"
#include "src/ginput/ginput_rules.h"
#include "src/gdisp/gdisp_rules.h"
#include "src/gaudio/gaudio_rules.h"
#include "src/gadc/gadc_rules.h"
#include "src/gevent/gevent_rules.h"
#include "src/gtimer/gtimer_rules.h"
#include "src/gqueue/gqueue_rules.h"
#include "src/gmisc/gmisc_rules.h"
#include "src/gfile/gfile_rules.h"
#include "src/gdriver/gdriver_rules.h"
#include "src/gos/gos_rules.h"

/**
 *  Include the sub-system header files
 */
#include "src/gos/gos.h"
//#include "src/gdriver/gdriver.h"			// This module is only included by source that needs it.
#include "src/gfile/gfile.h"
#include "src/gmisc/gmisc.h"
#include "src/gqueue/gqueue.h"
#include "src/gevent/gevent.h"
#include "src/gtimer/gtimer.h"
#include "src/gdisp/gdisp.h"
#include "src/gwin/gwin.h"
#include "src/ginput/ginput.h"
#include "src/gadc/gadc.h"
#include "src/gaudio/gaudio.h"

#ifdef __cplusplus
extern "C" {
#endif

	/**
	 * @brief	The one call to start it all
	 *
	 * @note	This will initialise each sub-system that has been turned on.
	 * 			For example, if GFX_USE_GDISP is defined then display will be initialised
	 * 			and cleared to black.
	 * @note	If you define GFX_OS_NO_INIT as TRUE in your gfxconf.h file then ugfx doesn't try to
	 * 			initialise the operating system for you when you call @p gfxInit().
	 * @note	If you define GFX_OS_EXTRA_INIT_FUNCTION in your gfxconf.h file the macro is the
	 * 			name of a void function with no parameters that is called immediately after
	 * 			operating system initialisation (whether or not GFX_OS_NO_INIT is set).
	 * @note	If you define GFX_OS_EXTRA_DEINIT_FUNCTION in your gfxconf.h file the macro is the
	 * 			name of a void function with no parameters that is called immediately before
	 * 			operating system de-initialisation (as ugfx is exiting).
	 *
	 * @api
	 */
	void gfxInit(void);

	/**
	 * @brief	The one call to end it all
	 *
	 * @note	This will de-initialise each sub-system that has been turned on.
	 *
	 * @api
	 */
	void gfxDeinit(void);

#ifdef __cplusplus
}
#endif

#endif /* _GFX_H */
/** @} */

