/*
 * This file is subject to the terms of the GFX License. If a copy of
 * the license was not distributed with this file, you can obtain one at:
 *
 *              http://ugfx.org/license.html
 */

/**
 * @file    src/gwin/frame.h
 * @brief   GWIN Graphic window subsystem header file.
 *
 * @defgroup Frame Frame
 * @ingroup Containers
 *
 * @details		A frame is a rectangular window that can have optional border as well as buttons to
 *				close, maximize and minimize it. The main purpose of this widget is to contain children. 	
 *
 * @pre			GFX_USE_GWIN must be set to TRUE in your gfxconf.h
 * @pre			GWIN_NEED_FRAME must be set to TRUE in your gfxconf.h
 * @{
 */

#ifndef _GWIN_FRAME_H
#define _GWIN_FRAME_H

/**
 * @brief	 Flags for gwinFrameCreate()
 * @{
 */
#define GWIN_FRAME_BORDER			0x00000001
#define GWIN_FRAME_CLOSE_BTN		0x00000002
#define GWIN_FRAME_MINMAX_BTN		0x00000004
/** @} */

typedef struct GFrameObject {
	GContainerObject	gc;

	GListener			gl;			// internal listener for the buttons
	// These could probably be removed... I have to think harder later
	GHandle				btnClose;
	GHandle				btnMin;
	GHandle				btnMax;
} GFrameObject;

/**
 * @brief				Create a frame widget
 *
 * @details				This widget provides a window like we know it from desktop systems.
 *
 * @param[in] g			The GDisplay to display this window on
 * @param[in] fo		The GFrameObject structure to initialize. If this is NULL the structure is dynamically allocated.
 * @param[in] pInit		The initialization parameters
 * @param[in] flags		Some flags, see notes.
 *
 * @note				Possible flags are: GWIN_FRAME_BORDER, GWIN_FRAME_CLOSE_BTN, GWIN_FRAME_MINMAX_BTN.
 *						When the close or the minimize/maximize buttons are used, the boarder is automatically invoked.
 * @note				These frame buttons are processed internally. The close button will invoke a gwinDestroy() which will
 *						destroy the window itself and EVERY child it contains (also children of children).
 *
 * @return				NULL if there is no resulting widget. A valid GHandle otherwise.
 *
 * @api
 */
GHandle gwinGFrameCreate(GDisplay *g, GFrameObject *fo, GWidgetInit *pInit, uint32_t flags);
#define gwinFrameCreate(fo, pInit, flags)	gwinGFrameCreate(GDISP, fo, pInit, flags);

#endif /* _GWIN_FRAME_H */
/** @} */

