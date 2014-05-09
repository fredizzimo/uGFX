/*
 * This file is subject to the terms of the GFX License. If a copy of
 * the license was not distributed with this file, you can obtain one at:
 *
 *              http://ugfx.org/license.html
 */

/**
 * @file    src/gwin/gcontainer.h
 * @brief   GWIN Containers header file.
 */

#ifndef _GCONTAINER_H
#define _GCONTAINER_H

/* This file is included within "gwin/gwin.h" */

/**
 * @defgroup Containers Containers
 * @ingroup GWIN
 *
 * @details		A Container is a GWindow that supports child windows. It is also
 * 				a widget in its own right and therefore can accept user input directly.
 *
 * @pre		GFX_USE_GWIN and GWIN_NEED_CONTAINERS must be set to TRUE in your gfxconf.h
 * @{
 */

// Forward definition
struct GContainerObject;

/**
 * @brief	The GWIN Container structure
 * @note	A container is a GWIN widget that can have children.
 * @note	Do not access the members directly. Treat it as a black-box and use the method functions.
 *
 * @{
 */
typedef GWidgetObject GContainerObject;
/* @} */

/**
 * A comment/rant on the above structure:
 * We would really like the GWidgetObject member to be anonymous. While this is
 * allowed under the C11, C99, GNU and various other standards which have been
 * around forever - compiler support often requires special flags e.g
 * gcc requires the -fms-extensions flag (no wonder the language and compilers have
 * not really progressed in 30 years). As portability is a key requirement
 * we unfortunately won't use this useful feature in case we get a compiler that
 * won't support it even with special flags.
 */

#ifdef __cplusplus
extern "C" {
#endif

	/**
	 * @brief	Get the first child window
	 *
	 * @return	The first child or NULL if are no children windows
	 *
	 * @param[in] gh	The parent container or NULL to get the first top level window
	 *
	 * @api
	 */
	GHandle gwinGetFirstChild(GHandle gh);

	/**
	 * @brief	Get the next child window in the z-order
	 *
	 * @return	The next window or NULL if no more children
	 *
	 * @param[in] gh		The window to obtain the next sibling of.
	 *
	 * @note	This returns the next window under the current parent window.
	 * 			Unlike @p gwinGetNextWindow() it will only return windows that
	 * 			have the same parent as the supplied window.
	 *
	 * @api
	 */
	GHandle gwinGetSibling(GHandle gh);
#ifdef __cplusplus
}
#endif

/* Include extra container types */
#if GWIN_NEED_FRAME || defined(__DOXYGEN__)
	#include "src/gwin/frame.h"
#endif

#endif /* _GCONTAINER_H */
/** @} */
