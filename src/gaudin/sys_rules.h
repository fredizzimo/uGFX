/*
 * This file is subject to the terms of the GFX License. If a copy of
 * the license was not distributed with this file, you can obtain one at:
 *
 *              http://ugfx.org/license.html
 */

/**
 * @file    src/gaudin/sys_rules.h
 * @brief   GAUDIN safety rules header file.
 *
 * @addtogroup GAUDIN
 * @{
 */

#ifndef _GAUDIN_RULES_H
#define _GAUDIN_RULES_H

#if GFX_USE_GAUDIN
	#if GFX_USE_GEVENT && !GFX_USE_GTIMER
		#if GFX_DISPLAY_RULE_WARNINGS
			#warning "GAUDIN: GFX_USE_GTIMER is required if GFX_USE_GAUDIN and GFX_USE_GEVENT are TRUE. It has been turned on for you."
		#endif
		#undef GFX_USE_GTIMER
		#define	GFX_USE_GTIMER		TRUE
	#endif
#endif

#endif /* _GAUDIN_RULES_H */
/** @} */
