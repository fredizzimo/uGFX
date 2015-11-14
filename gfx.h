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
	 * @{
	 */
	#ifndef GFX_COMPILER
		#define GFX_COMPILER			GFX_COMPILER_UNKNOWN
	#endif
	#define GFX_COMPILER_UNKNOWN		0		//**< Unknown compiler
	#define GFX_COMPILER_GCC			1		//**< Standard GCC/G++
	#define GFX_COMPILER_MINGW32		2		//**< MingW32 (x86) compiler for windows
	#define GFX_COMPILER_MINGW64		3		//**< MingW64 (x64) compiler for windows
	#define GFX_COMPILER_CYGWIN			4		//**< Cygwin (x86) unix emulator compiler for windows
	#define GFX_COMPILER_ARMCC			5		//**< ARMCC compiler
	#define GFX_COMPILER_KEIL			6		//**< Keil (use this when working with uVision IDE)
	#define GFX_COMPILER_CLANG			7		//**< CLang (LLVM) compiler
	#define GFX_COMPILER_HP				8		//**< HP C/aC++
	#define GFX_COMPILER_IBMXL			9		//**< IBM XL C/C++ Compiler
	#define GFX_COMPILER_ICC			10		//**< Intel ICC/ICPC Compiler
	#define GFX_COMPILER_VS				11		//**< Microsoft Visual Studio
	#define GFX_COMPILER_OSS			12		//**< Oracle Solaris Studio
	#define GFX_COMPILER_PGCC			13		//**< Portland PGCC/PGCPP
	#define GFX_COMPILER_TURBOC			14		//**< Borland Turbo C
	#define GFX_COMPILER_BORLAND		15		//**< Borland C++
	#define GFX_COMPILER_COMEAU			16		//**< Comeau C++
	#define GFX_COMPILER_COMPAQ			17		//**< Compaq C
	#define GFX_COMPILER_DEC			18		//**< The older DEC C Compiler
	#define GFX_COMPILER_CRAY			19		//**< Cray C/C++
	#define GFX_COMPILER_DAIB			20		//**< Diab C/C++
	#define GFX_COMPILER_DMARS			21		//**< Digital Mars/Symantic C++/Zortech C++
	#define GFX_COMPILER_KAI			22		//**< Kai C++
	#define GFX_COMPILER_LCC			23		//**< LCC
	#define GFX_COMPILER_HIGHC			24		//**< Metaware High C/C++
	#define GFX_COMPILER_METROWORKS		25		//**< Metroworks
	#define GFX_COMPILER_MIPSPRO		26		//**< MIPS Pro
	#define GFX_COMPILER_MPW			27		//**< MPW C++
	#define GFX_COMPILER_NORCROFT		28		//**< Norcroft ARM
	#define GFX_COMPILER_SASC			29		//**< SAS/C
	#define GFX_COMPILER_SCO			30		//**< SCO OpenServer
	#define GFX_COMPILER_TINYC			31		//**< Tiny C
	#define GFX_COMPILER_USL			32		//**< USL C
	#define GFX_COMPILER_WATCOM			33		//**< Watcom
	/** @} */
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
	#define GFX_CPU_CORTEX_M0			0x01	//**< Cortex M0
	#define GFX_CPU_CORTEX_M1			0x02	//**< Cortex M1
	#define GFX_CPU_CORTEX_M2			0x03	//**< Cortex M2
	#define GFX_CPU_CORTEX_M3			0x04	//**< Cortex M3
	#define GFX_CPU_CORTEX_M4			0x05	//**< Cortex M4
	#define GFX_CPU_CORTEX_M4_FP		0x06	//**< Cortex M4 with hardware floating point
	#define GFX_CPU_CORTEX_M7			0x07	//**< Cortex M7
	#define GFX_CPU_CORTEX_M7_FP		0x08	//**< Cortex M7 with hardware floating point
	#define GFX_CPU_X86					0x10	//**< Intel x86
	#define GFX_CPU_X64					0x11	//**< Intel x64
	#define GFX_CPU_IA64				0x12	//**< Intel Itanium
	#define GFX_CPU_POWERPC32			0x20	//**< PowerPC
	#define GFX_CPU_POWERPC64			0x21	//**< PowerPC
	#define GFX_CPU_SPARC				0x22	//**< Sparc
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
	#define GFX_CPU_ENDIAN_LITTLE		0x03020100		//**< Little endian
	#define GFX_CPU_ENDIAN_BIG			0x00010203		//**< Big endian
	#define GFX_CPU_ENDIAN_WBDWL		0x02030001		//**< Words are big endian, DWords are little endian eg. Honeywell 316
	#define GFX_CPU_ENDIAN_WLDWB		0x01000302		//**< Words are little endian, DWords are big endian eg PDP-11
	/** @} */

/** @} */

/* Try to auto-detect some stuff from the compiler itself */
#if GFX_COMPILER == GFX_COMPILER_UNKNOWN
	#undef GFX_COMPILER
	#if defined(__MINGW32__)
		#define GFX_COMPILER	GFX_COMPILER_MINGW32
	#elif defined(__MINGW64__)
		#define GFX_COMPILER	GFX_COMPILER_MINGW64
	#elif defined(__CYGWIN__)
		#define GFX_COMPILER	GFX_COMPILER_CYGWIN
	#elif defined(__KEIL__) || defined(__C51__) || (defined(__CC_ARM) && defined(__EDG__))
		#define GFX_COMPILER	GFX_COMPILER_KEIL
	#elif defined(__clang__)
		#define GFX_COMPILER	GFX_COMPILER_CLANG
	#elif defined(__ICC) || defined(__INTEL_COMPILER)
		#define GFX_COMPILER	GFX_COMPILER_ICC
	#elif defined(__GNUC__) || defined(__GNUG__)
		#define GFX_COMPILER	GFX_COMPILER_GCC
	#elif defined(__HP_cc) || defined(__HP_aCC)
		#define GFX_COMPILER	GFX_COMPILER_HP
	#elif defined(__IBMC__) || defined(__IBMCPP__)
		#define GFX_COMPILER	GFX_COMPILER_IBMXL
	#elif defined(_MSC_VER)
		#define GFX_COMPILER	GFX_COMPILER_VS
	#elif defined(__PGI)
		#define GFX_COMPILER	GFX_COMPILER_PGCC
	#elif defined(__SUNPRO_C) || defined(__SUNPRO_CC)
		#define GFX_COMPILER	GFX_COMPILER_OSS
	#elif defined(__TURBOC__)
		#define GFX_COMPILER	GFX_COMPILER_TURBOC
	#elif defined(__BORLANDC__)
		#define GFX_COMPILER	GFX_COMPILER_BORLAND
	#elif defined(__COMO__)
		#define GFX_COMPILER	GFX_COMPILER_COMEAU
	#elif defined(__DECC) || defined(__VAXC) || defined(VAXC) || defined(__DECCXX)
		#define GFX_COMPILER	GFX_COMPILER_COMPAQ
	#elif defined(__osf__) && defined(__LANGUAGE_C__)
		#define GFX_COMPILER	GFX_COMPILER_DEC
	#elif defined(_CRAYC)
		#define GFX_COMPILER	GFX_COMPILER_CRAY
	#elif defined(__DCC__)
		#define GFX_COMPILER	GFX_COMPILER_DAIB
	#elif defined(__DMC__) || defined(__SC__) || defined(__ZTC__)
		#define GFX_COMPILER	GFX_COMPILER_DMARS
	#elif defined(__KCC)
		#define GFX_COMPILER	GFX_COMPILER_KAI
	#elif defined(__LCC__)
		#define GFX_COMPILER	GFX_COMPILER_LCC
	#elif defined(__HIGHC__)
		#define GFX_COMPILER	GFX_COMPILER_HIGHC
	#elif defined(__MWERKS__)
		#define GFX_COMPILER	GFX_COMPILER_METROWORKS
	#elif defined(__sgi)
		#define GFX_COMPILER	GFX_COMPILER_MIPSPRO
	#elif defined(__MRC__)
		#define GFX_COMPILER	GFX_COMPILER_MPW
	#elif defined(__CC_NORCROFT)
		#define GFX_COMPILER	GFX_COMPILER_NORCROFT
	#elif defined(__SASC__)
		#define GFX_COMPILER	GFX_COMPILER_SASC
	#elif defined( _SCO_DS )
		#define GFX_COMPILER	GFX_COMPILER_SCO
	#elif defined(__TINYC__)
		#define GFX_COMPILER	GFX_COMPILER_TINYC
	#elif defined( __USLC__ )
		#define GFX_COMPILER	GFX_COMPILER_USL
	#elif defined(__WATCOMC__)
		#define GFX_COMPILER	GFX_COMPILER_WATCOM
	#else
		#define GFX_COMPILER	GFX_COMPILER_UNKNOWN
	#endif
#endif
#if GFX_CPU == GFX_CPU_UNKNOWN
	#undef GFX_CPU
	#if defined(__ia64) || defined(__itanium__) || defined(_M_IA64)
		#define GFX_CPU		GFX_CPU_IA64
	#elif defined(__powerpc__) || defined(__ppc__) || defined(__PPC__)
		#if defined(__powerpc64__) || defined(__ppc64__) || defined(__PPC64__) || defined(__64BIT__) || defined(_LP64) || defined(__LP64__)
			#define GFX_CPU		GFX_CPU_POWERPC64
		#else
			#define GFX_CPU		GFX_CPU_POWERPC32
		#endif
	#elif defined(__sparc)
		#define GFX_CPU		GFX_CPU_SPARC
	#elif defined(__x86_64__) || defined(_M_X64)
		#define GFX_CPU		GFX_CPU_X86
	#elif defined(__i386) || defined(_M_IX86)
		#define GFX_CPU		GFX_CPU_X64
	#else
		#define GFX_CPU		GFX_CPU_UNKNOWN
	#endif
#endif
#if GFX_CPU_ENDIAN == GFX_CPU_ENDIAN_UNKNOWN
	#undef GFX_CPU_ENDIAN
	#if (defined(__BYTE_ORDER__)&&(__BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__)) \
			|| defined(__LITTLE_ENDIAN__) || defined(__LITTLE_ENDIAN) || defined(_LITTLE_ENDIAN) \
			|| defined(__MIPSEL__) || defined(__MIPSEL) || defined(_MIPSEL) \
			|| defined(__THUMBEL__) \
			|| defined(__AARCH64EL__) \
			|| defined(__ARMEL__)
		#define GFX_CPU_ENDIAN			GFX_CPU_ENDIAN_LITTLE
	#elif (defined(__BYTE_ORDER__)&&(__BYTE_ORDER__ == __ORDER_BIG_ENDIAN__)) \
			|| defined(__BIG_ENDIAN__) || defined(__BIG_ENDIAN) || defined(_BIG_ENDIAN) \
			|| defined(__MIPSEB__) || defined(__MIPSEB) || defined(_MIPSEB) \
			|| defined(__THUMBEB__) \
			|| defined(__AARCH64EB__) \
			|| defined(__ARMEB__)
		#define GFX_CPU_ENDIAN			GFX_CPU_ENDIAN_BIG
	#else
		#define GFX_CPU_ENDIAN			GFX_CPU_ENDIAN_UNKNOWN
	#endif
#endif

#if GFX_NO_INLINE
	#define GFXINLINE
#else
	#if GFX_COMPILER == GFX_COMPILER_KEIL
		#define GFXINLINE	__inline
	#else
		#define GFXINLINE	inline
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

