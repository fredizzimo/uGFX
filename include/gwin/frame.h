/*
 * This file is subject to the terms of the GFX License. If a copy of
 * the license was not distributed with this file, you can obtain one at:
 *
 *              http://ugfx.org/license.html
 */

/**
 * @file    include/gwin/frame.h
 * @brief   GWIN Graphic window subsystem header file.
 *
 * @defgroup Frame Frame
 * @ingroup GWIN
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

#include "gwin/class_gwin.h"

// Flags for gwinFrameCreate()
#define GWIN_FRAME_BORDER			(GWIN_FIRST_CONTROL_FLAG << 0)
#define GWIN_FRAME_CLOSE_BTN		(GWIN_FIRST_CONTROL_FLAG << 1)
#define GWIN_FRAME_MINMAX_BTN		(GWIN_FIRST_CONTROL_FLAG << 2)

typedef struct GFrameObject {
	GWidgetObject		w;

	GHandle				btnClose;
	GHandle				btnMin;
	GHandle				btnMax;
} GFrameObject;

GHandle gwinGFrameCreate(GDisplay *g, GFrameObject *fo, GWidgetInit *pInit, uint16_t flags);
#define gwinFrameCreate(fo, pInit, flags)	gwinGFrameCreate(GDISP, fo, pInit, flags);

GHandle gwinFrameGetClose(GHandle gh);

GHandle gwinFrameGetMin(GHandle gh);

GHandle gwinFrameGetMax(GHandle gh);

#endif /* _GWIN_FRAME_H */
/** @} */

