/*
 * This file is subject to the terms of the GFX License. If a copy of
 * the license was not distributed with this file, you can obtain one at:
 *
 *              http://ugfx.org/license.html
 */

/**
 * @file    src/gos/gos_options.h
 * @brief   GOS - Operating System options header file.
 *
 * @addtogroup GOS
 * @{
 */

#ifndef _GOS_OPTIONS_H
#define _GOS_OPTIONS_H

/**
 * @name    The operating system to use. One (and only one) of these must be defined.
 * @{
 */
	/**
	 * @brief   Use ChibiOS
	 * @details	Defaults to FALSE
	 */
	#ifndef GFX_USE_OS_CHIBIOS
		#define GFX_USE_OS_CHIBIOS		FALSE
	#endif
	/**
	 * @brief   Use FreeRTOS
	 * @details	Defaults to FALSE
	 */
	#ifndef GFX_USE_OS_FREERTOS
		#define GFX_USE_OS_FREERTOS		FALSE
	#endif
	/**
	 * @brief   Use Win32
	 * @details	Defaults to FALSE
	 */
	#ifndef GFX_USE_OS_WIN32
		#define GFX_USE_OS_WIN32		FALSE
	#endif
	/**
	 * @brief   Use a linux based system running X11
	 * @details	Defaults to FALSE
	 */
	#ifndef GFX_USE_OS_LINUX
		#define GFX_USE_OS_LINUX		FALSE
	#endif
	/**
	 * @brief   Use a Mac OS-X based system
	 * @details	Defaults to FALSE
	 */
	#ifndef GFX_USE_OS_OSX
		#define GFX_USE_OS_OSX			FALSE
	#endif
	/**
	 * @brief   Use a Raw 32-bit CPU based system (Bare Metal)
	 * @details	Defaults to FALSE
	 */
	#ifndef GFX_USE_OS_RAW32
		#define GFX_USE_OS_RAW32		FALSE
	#endif
	/**
	 * @brief   Use a eCos
	 * @details	Defaults to FALSE
	 */
	#ifndef GFX_USE_OS_ECOS
		#define GFX_USE_OS_ECOS			FALSE
	#endif
	/**
	 * @brief   Use RAWRTOS
	 * @details Defaults to FALSE
	 */
	#ifndef GFX_USE_OS_RAWRTOS
		#define GFX_USE_OS_RAWRTOS		FALSE
	#endif
	/**
	 * @brief   Use Arduino
	 * @details	Defaults to FALSE
	 */
	#ifndef GFX_USE_OS_ARDUINO
		#define GFX_USE_OS_ARDUINO		FALSE
	#endif
	/**
	 * @brief	Use CMSIS RTOS compatible OS
	 * @details	Defaults to FALSE
	 */
	#ifndef GFX_USE_OS_CMSIS
		#define GFX_USE_OS_CMSIS		FALSE
	#endif
	/**
	 * @brief   Use Keil CMSIS
	 * @details	Defaults to FALSE
	 */
	#ifndef GFX_USE_OS_KEIL
		#define GFX_USE_OS_KEIL			FALSE
	#endif
/**
 * @}
 *
 * @name    GOS Optional Parameters
 * @{
 */
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
 	 * @brief	Should uGFX avoid initializing the operating system
 	 * @details	Defaults to FALSE
 	 * @note	This is not relevant to all operating systems eg Win32 never initializes the
 	 * 			operating system as uGFX runs as an application outside the boot process.
 	 * @note	Operating system initialization is not necessarily implemented for all
 	 * 			operating systems yet even when it is relevant. These operating systems
 	 * 			will display a compile warning reminding you to initialize the operating
 	 * 			system in your application code. Note that on these operating systems the
 	 * 			demo applications will not work without modification.
 	 */
 	#ifndef GFX_OS_NO_INIT
 		#define GFX_OS_NO_INIT			FALSE
 	#endif
 	/**
 	 * @brief	Turn off warnings about initializing the operating system
 	 * @details	Defaults to FALSE
 	 * @note	This is only relevant where GOS cannot initialise the operating
 	 * 			system automatically or the operating system initialisation has been
 	 * 			explicitly turned off.
 	 */
	#ifndef GFX_OS_INIT_NO_WARNING
		#define GFX_OS_INIT_NO_WARNING	FALSE
	#endif
 	/**
 	 * @brief	Should uGFX stuff be added to the FreeRTOS+Tracer
 	 * @details	Defaults to FALSE
 	 */
 	#ifndef GFX_FREERTOS_USE_TRACE
 		#define GFX_FREERTOS_USE_TRACE	FALSE
 	#endif
 	/**
 	 * @brief	How much RAM should uGFX use for the heap
 	 * @details	Defaults to 0.
 	 * @note	Only used when the generic ugfx heap code is used (GFX_USE_OS_RAW32 and GFX_USE_OS_ARDUINO)
 	 * @note	If 0 then the standard C runtime malloc(), free() and realloc()
 	 * 			are used.
 	 * @note	If it is non-zero then this is the number of bytes of RAM
 	 * 			to use for the heap (gfxAlloc() and gfxFree()). No C
 	 * 			runtime routines will be used and a new routine @p gfxAddHeapBlock()
 	 * 			is added allowing the user to add extra memory blocks to the heap.
 	 */
	#ifndef GFX_OS_HEAP_SIZE
		#define GFX_OS_HEAP_SIZE		0
	#endif
/** @} */

#endif /* _GOS_OPTIONS_H */
/** @} */
