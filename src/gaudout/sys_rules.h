/*
 * This file is subject to the terms of the GFX License. If a copy of
 * the license was not distributed with this file, you can obtain one at:
 *
 *              http://ugfx.org/license.html
 */

/**
 * @file    src/gaudout/sys_rules.h
 * @brief   GAUDOUT safety rules header file.
 *
 * @addtogroup GAUDOUT
 * @{
 */

#ifndef _GAUDOUT_RULES_H
#define _GAUDOUT_RULES_H

#if GFX_USE_GAUDOUT
	#if !GFX_USE_GQUEUE
		#if GFX_DISPLAY_RULE_WARNINGS
			#warning "GAUDOUT: GFX_USE_GQUEUE is required if GFX_USE_GAUDOUT is TRUE. It has been turned on for you."
		#endif
		#undef GFX_USE_GQUEUE
		#define	GFX_USE_GQUEUE		TRUE
	#endif
	#if !GQUEUE_NEED_ASYNC
		#if GFX_DISPLAY_RULE_WARNINGS
			#warning "GAUDOUT: GQUEUE_NEED_ASYNC is required if GFX_USE_GAUDOUT is TRUE. It has been turned on for you."
		#endif
		#undef GQUEUE_NEED_ASYNC
		#define	GQUEUE_NEED_ASYNC		TRUE
	#endif
	#if !GQUEUE_NEED_GSYNC
		#if GFX_DISPLAY_RULE_WARNINGS
			#warning "GAUDOUT: GQUEUE_NEED_GSYNC is required if GFX_USE_GAUDOUT is TRUE. It has been turned on for you."
		#endif
		#undef GQUEUE_NEED_GSYNC
		#define	GQUEUE_NEED_GSYNC		TRUE
	#endif
#endif

#endif /* _GAUDOUT_RULES_H */
/** @} */
