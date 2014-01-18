/*
 * This file is subject to the terms of the GFX License. If a copy of
 * the license was not distributed with this file, you can obtain one at:
 *
 *              http://ugfx.org/license.html
 */

/**
 * @file    include/gwin/ggroup.h
 * @brief   GWIN Widgets header file.
 */

#ifndef _GGROUP_H
#define _GGROUP_H

/* This file is included within "gwin/gwin.h" */

/**
 * @defgroup Widget Widget
 * @ingroup GWIN
 *
 * @details		Groups provide parent/child relation features and dynamic layouts.
 *
 * @pre			GFX_NEED_GWIN needs to be set to TRUE in your gfxconf.h.
 * @pre			GWIN_NEED_GROUPS needs to be set to TRUE in your gfxconf.h.
 *
 * @{
 */

/**
 * @brief	A group object structure
 * @note	Do not access the members directly. Treat it as a black-box and use the method functions.
 * @{
 */
typedef struct GGroupObject {
	GWindowObject		g;

	GHandle				parent;				// @< The parent widget
	GHandle				sibling;			// @< The widget to its left (add right later as well)
	GHandle				child;				// @< The child widget
} GGroupObject;
/** @} */


#endif /* _GGROUP_H */

