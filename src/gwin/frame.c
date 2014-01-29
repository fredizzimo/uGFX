/*
 * This file is subject to the terms of the GFX License. If a copy of
 * the license was not distributed with this file, you can obtain one at:
 *
 *              http://ugfx.org/license.html
 */

/**
 * @file    src/gwin/frame.c
 * @brief   GWIN sub-system frame code.
 *
 * @defgroup Frame Frame
 * @ingroup GWIN
 *
 * @{
 */

#include "gfx.h"

#if GFX_USE_GWIN && GWIN_NEED_FRAME

/* Some values for the default render */
#define BORDER_X		5
#define BORDER_Y		30
#define BUTTON_X		20
#define BUTTON_Y		20

/* Some useful macros for data type conversions */
#define gh2obj			((GFrameObject *)gh)

/* Forware declarations */
void gwinFrameDraw_Std(GWindowObject *gw);
static void _callbackBtn(void *param, GEvent *pe);

static void _frameDestroy(GHandle gh) {
	/* Deregister the button callback */
	geventRegisterCallback(&gh2obj->gl, NULL, NULL);
	geventDetachSource(&gh2obj->gl, NULL);

	/* call the gwidget standard destroy routine */
	_gwidgetDestroy(gh);
}

static const ggroupVMT frameVMT = {
	{
		"Frame",					// The classname
		sizeof(GFrameObject),		// The object size
		_frameDestroy,				// The destroy routie
		gwinFrameDraw_Std,			// The redraw routine
		0,							// The after-clear routine
	},
};

GHandle gwinGFrameCreate(GDisplay *g, GFrameObject *fo, GWindowInit *pInit, uint32_t flags) {
	if (!(fo = (GFrameObject *)_ggroupCreate(g, &fo->w, pInit, &frameVMT)))
		return 0;

	fo->btnClose = NULL;
	fo->btnMin = NULL;
	fo->btnMax = NULL;

	/* create and initialize the listener if any button is present. */
	if ((flags & GWIN_FRAME_CLOSE_BTN) || (flags & GWIN_FRAME_MINMAX_BTN)) {
		flags |= GWIN_FRAME_BORDER;				// Buttons require a border
		geventListenerInit(&fo->gl);
		gwinAttachListener(&fo->gl);
		geventRegisterCallback(&fo->gl, _callbackBtn, (GHandle)fo);
	}

	/* create close button if necessary */
	if ((flags & GWIN_FRAME_CLOSE_BTN)) {
		GWidgetInit wi;

		wi.customDraw = 0;
		wi.customParam = 0;
		wi.customStyle = 0;
		wi.g.show = TRUE;

		wi.g.x = fo->w.g.width - BORDER_X - BUTTON_X;
		wi.g.y = (BORDER_Y - BUTTON_Y) / 2;
		wi.g.width = BUTTON_X;
		wi.g.height = BUTTON_Y;
		wi.text = "X";
		fo->btnClose = gwinButtonCreate(NULL, &wi);		
		gwinAddChild((GHandle)fo, fo->btnClose, FALSE);
	}

	/* create minimize and maximize buttons if necessary */
	if ((flags & GWIN_FRAME_MINMAX_BTN)) {
		GWidgetInit wi;

		wi.customDraw = 0;
		wi.customParam = 0;
		wi.customStyle = 0;
		wi.g.show = TRUE;

		wi.g.x = (fo->w.g.flags & GWIN_FRAME_CLOSE_BTN) ? fo->w.g.width - 2*BORDER_X - 2*BUTTON_X : fo->w.g.width - BORDER_X - BUTTON_X;
		wi.g.y = (BORDER_Y - BUTTON_Y) / 2;
		wi.g.width = BUTTON_X;
		wi.g.height = BUTTON_Y;
		wi.text = "O";
		fo->btnMin = gwinButtonCreate(NULL, &wi);		
		gwinAddChild((GHandle)fo, fo->btnMin, FALSE);

		wi.g.x = (fo->w.g.flags & GWIN_FRAME_CLOSE_BTN) ? fo->w.g.width - 3*BORDER_X - 3*BUTTON_X : fo->w.g.width - BORDER_X - BUTTON_X;
		wi.g.y = (BORDER_Y - BUTTON_Y) / 2;
		wi.g.width = BUTTON_X;
		wi.g.height = BUTTON_Y;
		wi.text = "_";
		fo->btnMax = gwinButtonCreate(NULL, &wi);
		gwinAddChild((GHandle)fo, fo->btnMax, FALSE);
	}

	fo->w.g.flags |= flags & (GWIN_FRAME_BORDER|GWIN_FRAME_CLOSE_BTN|GWIN_FRAME_MINMAX_BTN);
	gwinSetVisible(&fo->w.g, pInit->show);

	return (GHandle)fo;
}

/* Process a button event */
static void _callbackBtn(void *param, GEvent *pe) {
	switch (pe->type) {
		case GEVENT_GWIN_BUTTON:
			if (((GEventGWinButton *)pe)->button == ((GFrameObject*)(GHandle)param)->btnClose)
				gwinDestroy((GHandle)param);

			else if (((GEventGWinButton *)pe)->button == ((GFrameObject*)(GHandle)param)->btnMin)
				;/* ToDo */

			else if (((GEventGWinButton *)pe)->button == ((GFrameObject*)(GHandle)param)->btnMax)
				;/* ToDo */

			break;

		default:
			break;
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Render routine                                                                                //
///////////////////////////////////////////////////////////////////////////////////////////////////

void gwinFrameDraw_Std(GHandle gh) {
	color_t			border;
	color_t			background;
	color_t			text;

	if (gh->vmt != (gwinVMT *)&frameVMT)
		return;

	// do some magic to make the background lighter than the widgets. Fix this somewhen.
	border = HTML2COLOR(0x2698DE);
	background = HTML2COLOR(0xEEEEEE);
	text = White;

	#if GDISP_NEED_CLIP
		gdispGSetClip(gh->display, gh->x, gh->y, gh->width, gh->height);
	#endif 

	// Render the actual frame (with border, if any)
	if (gh->flags & GWIN_FRAME_BORDER) {
		gdispGFillArea(gh->display, gh->x, gh->y, gh->width, gh->height, border);
		gdispGFillArea(gh->display, gh->x + BORDER_X, gh->y + BORDER_Y, gh->width - 2*BORDER_X, gh->width - BORDER_Y - BORDER_X, background);
	} else {
		// This ensure that the actual frame content (it's children) render at the same spot, no mather whether the frame has a border or not
		gdispGFillArea(gh->display, gh->x + BORDER_X, gh->y + BORDER_Y, gh->width, gh->height, background);
	}

/*	// Render frame title - if any
	if (gw->text != NULL) {
		coord_t text_y;

		text_y = ((BORDER_Y - gdispGetFontMetric(gw->g.font, fontHeight))/2);

		gdispGDrawString(gw->g.display, gw->g.x + BORDER_X, gw->g.y + text_y, gw->text, gw->g.font, pcol->text);
	}
*/

	gwinRedrawChildren(gh);
}

#endif  /* (GFX_USE_GWIN && GWIN_NEED_FRAME) || defined(__DOXYGEN__) */
/** @} */

