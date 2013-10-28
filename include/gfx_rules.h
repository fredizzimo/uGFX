/*
 * This file is subject to the terms of the GFX License. If a copy of
 * the license was not distributed with this file, you can obtain one at:
 *
 *              http://ugfx.org/license.html
 */

/**
 * @file    include/gfx_rules.h
 * @brief   GFX system safety rules header file.
 *
 * @addtogroup GFX
 * @{
 */

#ifndef _GFX_RULES_H
#define _GFX_RULES_H

/**
 * Safety checks on all the defines.
 *
 * These are defined in the order of their inter-dependancies.
 */

#ifndef GFX_DISPLAY_RULE_WARNINGS
	#define GFX_DISPLAY_RULE_WARNINGS	FALSE
#endif

#if !GFX_USE_OS_CHIBIOS && !GFX_USE_OS_WIN32 && !GFX_USE_OS_LINUX && !GFX_USE_OS_OSX
	#if GFX_DISPLAY_RULE_WARNINGS
		#warning "GOS: No Operating System has been defined. ChibiOS (GFX_USE_OS_CHIBIOS) has been turned on for you."
	#endif
	#undef GFX_USE_OS_CHIBIOS
	#define GFX_USE_OS_CHIBIOS	TRUE
#endif
#if GFX_USE_OS_CHIBIOS + GFX_USE_OS_WIN32 + GFX_USE_OS_LINUX + GFX_USE_OS_OSX != 1 * TRUE
	#error "GOS: More than one operation system has been defined as TRUE."
#endif

#if GFX_USE_GWIN
	#if !GFX_USE_GDISP
		#error "GWIN: GFX_USE_GDISP must be TRUE when using GWIN"
	#endif
	#if !GDISP_NEED_CLIP
		#if GFX_DISPLAY_RULE_WARNINGS
			#warning "GWIN: Drawing can occur outside the defined windows as GDISP_NEED_CLIP is FALSE"
		#endif
	#endif
	#if GWIN_NEED_IMAGE
		#if !GDISP_NEED_IMAGE
			#error "GWIN: GDISP_NEED_IMAGE is required when GWIN_NEED_IMAGE is TRUE."
		#endif
	#endif
	#if GWIN_NEED_RADIO
		#if !GDISP_NEED_CIRCLE
			#if GFX_DISPLAY_RULE_WARNINGS
				#warning "GWIN: GDISP_NEED_CIRCLE should be set to TRUE for much nicer radio button widgets."
			#endif
		#endif
	#endif
	#if GWIN_NEED_BUTTON || GWIN_NEED_SLIDER || GWIN_NEED_CHECKBOX || GWIN_NEED_LABEL || GWIN_NEED_RADIO
		#if !GWIN_NEED_WIDGET
			#if GFX_DISPLAY_RULE_WARNINGS
				#warning "GWIN: GWIN_NEED_WIDGET is required when a Widget is used. It has been turned on for you."
			#endif
			#undef GWIN_NEED_WIDGET
			#define GWIN_NEED_WIDGET	TRUE
		#endif
	#endif
	#if GWIN_NEED_LIST
		#if !GDISP_NEED_TEXT
			#error "GWIN: GDISP_NEED_TEXT is required when GWIN_NEED_LIST is TRUE."
		#endif
	#endif
	#if GWIN_NEED_WIDGET
		#if !GDISP_NEED_TEXT
			#error "GWIN: GDISP_NEED_TEXT is required if GWIN_NEED_WIDGET is TRUE."
		#endif
		#if !GFX_USE_GINPUT
			// This test also ensures that GFX_USE_GEVENT is set
			#error "GWIN: GFX_USE_GINPUT (and one or more input sources) is required if GWIN_NEED_WIDGET is TRUE"
		#endif
		#if !GWIN_NEED_WINDOWMANAGER
			#if GFX_DISPLAY_RULE_WARNINGS
				#warning "GWIN: GWIN_NEED_WINDOWMANAGER is required if GWIN_NEED_WIDGET is TRUE. It has been turned on for you."
			#endif
			#undef GWIN_NEED_WINDOWMANAGER
			#define GWIN_NEED_WINDOWMANAGER	TRUE
		#endif
		#if !GDISP_NEED_MULTITHREAD
			#if GFX_DISPLAY_RULE_WARNINGS
				#warning "GWIN: GDISP_NEED_MULTITHREAD is required if GWIN_NEED_WIDGET is TRUE. It has been turned on for you"
			#endif
			#undef GDISP_NEED_MULTITHREAD
			#define GDISP_NEED_MULTITHREAD	TRUE
		#endif
	#endif
	#if GWIN_NEED_WINDOWMANAGER
		#if !GFX_USE_GQUEUE || !GQUEUE_NEED_ASYNC
			#if GFX_DISPLAY_RULE_WARNINGS
				#warning "GWIN: GFX_USE_GQUEUE and GQUEUE_NEED_ASYNC is required if GWIN_NEED_WINDOWMANAGER is TRUE. It has been turned on for you."
			#endif
			#undef GFX_USE_GQUEUE
			#undef GQUEUE_NEED_ASYNC
			#define GFX_USE_GQUEUE		TRUE
			#define GQUEUE_NEED_ASYNC	TRUE
		#endif
	#endif
	#if GWIN_NEED_CONSOLE
		#if !GDISP_NEED_TEXT
			#error "GWIN: GDISP_NEED_TEXT is required if GWIN_NEED_CONSOLE is TRUE."
		#endif
	#endif
	#if GWIN_NEED_GRAPH
	#endif
#endif

#if GFX_USE_GINPUT
	#if !GFX_USE_GEVENT
		#if GFX_DISPLAY_RULE_WARNINGS
			#warning "GINPUT: GFX_USE_GEVENT is required if GFX_USE_GINPUT is TRUE. It has been turned on for you."
		#endif
		#undef GFX_USE_GEVENT
		#define	GFX_USE_GEVENT		TRUE
	#endif
	#if !GFX_USE_GTIMER
		#if GFX_DISPLAY_RULE_WARNINGS
			#warning "GINPUT: GFX_USE_GTIMER is required if GFX_USE_GINPUT is TRUE. It has been turned on for you."
		#endif
		#undef GFX_USE_GTIMER
		#define	GFX_USE_GTIMER		TRUE
	#endif
#endif

#if GFX_USE_GDISP
	#if GDISP_TOTAL_CONTROLLERS > 1
		#ifndef GDISP_CONTROLLER_LIST
			#error "GDISP Multiple Controllers: You must specify a value for GDISP_CONTROLLER_LIST"
		#endif
		#ifndef GDISP_CONTROLLER_DISPLAYS
			#error "GDISP Multiple Controllers: You must specify a value for GDISP_CONTROLLER_DISPLAYS"
		#endif
		#ifndef GDISP_PIXELFORMAT
			#error "GDISP Multiple Controllers: You must specify a value for GDISP_PIXELFORMAT"
		#endif
	#endif
	#if GDISP_NEED_AUTOFLUSH && GDISP_NEED_TIMERFLUSH
		#if GFX_DISPLAY_RULE_WARNINGS
			#warning "GDISP: Both GDISP_NEED_AUTOFLUSH and GDISP_NEED_TIMERFLUSH has been set. GDISP_NEED_TIMERFLUSH has disabled for you."
		#endif
		#undef GDISP_NEED_TIMERFLUSH
		#define GDISP_NEED_TIMERFLUSH		FALSE
	#endif
	#if GDISP_NEED_TIMERFLUSH
		#if GDISP_NEED_TIMERFLUSH < 50 || GDISP_NEED_TIMERFLUSH > 1200
			#error "GDISP: GDISP_NEED_TIMERFLUSH has been set to an invalid value (FALSE, 50-1200)."
		#endif
		#if !GFX_USE_GTIMER
			#if GFX_DISPLAY_RULE_WARNINGS
				#warning "GDISP: GDISP_NEED_TIMERFLUSH has been set but GFX_USE_GTIMER has not been set. It has been turned on for you."
			#endif
			#undef GFX_USE_GTIMER
			#define GFX_USE_GTIMER				TRUE
			#undef GDISP_NEED_MULTITHREAD
			#define GDISP_NEED_MULTITHREAD		TRUE
		#endif
	#endif
	#if GDISP_NEED_ANTIALIAS && !GDISP_NEED_PIXELREAD
		#if GDISP_HARDWARE_PIXELREAD
			#if GFX_DISPLAY_RULE_WARNINGS
				#warning "GDISP: GDISP_NEED_ANTIALIAS has been set but GDISP_NEED_PIXELREAD has not. It has been turned on for you."
			#endif
			#undef GDISP_NEED_PIXELREAD
			#define GDISP_NEED_PIXELREAD	TRUE
		#else
			#if GFX_DISPLAY_RULE_WARNINGS
				#warning "GDISP: GDISP_NEED_ANTIALIAS has been set but your hardware does not support reading back pixels. Anti-aliasing will only occur for filled characters."
			#endif
		#endif
	#endif
	#if (defined(GDISP_INCLUDE_FONT_SMALL) && GDISP_INCLUDE_FONT_SMALL) || (defined(GDISP_INCLUDE_FONT_LARGER) && GDISP_INCLUDE_FONT_LARGER)
		#if GFX_DISPLAY_RULE_WARNINGS
			#warning "GDISP: An old font (Small or Larger) has been defined. A single default font of UI2 has been added instead."
			#warning "GDISP: Please see <$(GFXLIB)/include/gdisp/fonts/fonts.h> for a list of available font names."
		#endif
		#undef GDISP_INCLUDE_FONT_UI2
		#define GDISP_INCLUDE_FONT_UI2		TRUE
	#endif
#endif

#if GFX_USE_TDISP
#endif

#if GFX_USE_GAUDIN
	#if GFX_USE_GEVENT && !GFX_USE_GTIMER
		#if GFX_DISPLAY_RULE_WARNINGS
			#warning "GAUDIN: GFX_USE_GTIMER is required if GFX_USE_GAUDIN and GFX_USE_GEVENT are TRUE. It has been turned on for you."
		#endif
		#undef GFX_USE_GTIMER
		#define	GFX_USE_GTIMER		TRUE
	#endif
#endif

#if GFX_USE_GADC
	#if !GFX_USE_GTIMER
		#if GFX_DISPLAY_RULE_WARNINGS
			#warning "GADC: GFX_USE_GTIMER is required if GFX_USE_GADC is TRUE. It has been turned on for you."
		#endif
		#undef GFX_USE_GTIMER
		#define	GFX_USE_GTIMER		TRUE
	#endif
#endif

#if GFX_USE_GEVENT
#endif

#if GFX_USE_GTIMER
	#if GFX_USE_GDISP && !GDISP_NEED_MULTITHREAD
		#if GFX_DISPLAY_RULE_WARNINGS
			#warning "GTIMER: GDISP_NEED_MULTITHREAD has not been specified."
			#warning "GTIMER: Make sure you are not performing any GDISP/GWIN drawing operations in the timer callback!"
		#endif
	#endif
#endif

#if GFX_USE_GAUDOUT
#endif

#if GFX_USE_GQUEUE
#endif

#if GFX_USE_GMISC
#endif

#endif /* _GFX_H */
/** @} */
