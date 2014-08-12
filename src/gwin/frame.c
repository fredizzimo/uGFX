/*
 * This file is subject to the terms of the GFX License. If a copy of
 * the license was not distributed with this file, you can obtain one at:
 *
 *              http://ugfx.org/license.html
 */

/**
 * @file    src/gwin/frame.c
 * @brief   GWIN sub-system frame code.
 */

#include "gfx.h"

#if GFX_USE_GWIN && GWIN_NEED_FRAME

#include "src/gwin/class_gwin.h"

#if GWIN_FRAME_BORDER != GWIN_FIRST_CONTROL_FLAG
	#error "GWIN Frame: - Flag definitions don't match"
#endif

/* Some values for the default render */
#define BORDER_X		5
#define BORDER_Y		30
#define BUTTON_X		20
#define BUTTON_Y		20

/* Some useful macros for data type conversions */
#define gh2obj			((GFrameObject *)gh)

/* Forware declarations */
static void gwinFrameDraw_Std(GWidgetObject *gw, void *param);
static void _callbackBtn(void *param, GEvent *pe);

static coord_t BorderSizeLRB(GHandle gh)	{ return (gh->flags & GWIN_FRAME_BORDER) ? BORDER_X : 0; }
static coord_t BorderSizeT(GHandle gh)		{ return (gh->flags & GWIN_FRAME_BORDER) ? BORDER_Y : 0; }

static void _frameDestroy(GHandle gh) {
	/* Deregister the button callback */
	if ((gh->flags & (GWIN_FRAME_CLOSE_BTN|GWIN_FRAME_MINMAX_BTN))) {
		geventRegisterCallback(&gh2obj->gl, NULL, NULL);
		geventDetachSource(&gh2obj->gl, NULL);
	}

	/* call the gcontainer standard destroy routine */
	_gcontainerDestroy(gh);
}

static void _closeBtnDraw(struct GWidgetObject *gw, void *param) {
	(void) param;

	// the background
	if (gwinButtonIsPressed( (GHandle)gw)) {
		gdispFillArea(gw->g.x, gw->g.y, gw->g.width, gw->g.height, gdispBlendColor(Black, HTML2COLOR(0xC55152), 50));
	} else {
		gdispFillArea(gw->g.x, gw->g.y, gw->g.width, gw->g.height, HTML2COLOR(0xC55152));
	}

	// the cross
	gdispDrawLine(gw->g.x+4, gw->g.y+4, gw->g.x+gw->g.width-5, gw->g.y+gw->g.height-5, White);
	gdispDrawLine(gw->g.x+gw->g.width-5, gw->g.y+4, gw->g.x+4, gw->g.y+gw->g.height-5, White);
}

static void _closeBtnMin(struct GWidgetObject *gw, void *param) {
	(void) param;

	// the background
	if (gwinButtonIsPressed( (GHandle)gw)) {
		gdispFillArea(gw->g.x, gw->g.y, gw->g.width, gw->g.height, gdispBlendColor(Black, Grey, 50));
	} else {
		gdispFillArea(gw->g.x, gw->g.y, gw->g.width, gw->g.height, gdispBlendColor(White, Grey, 50));
	}

	// the symbol
	gdispDrawLine(gw->g.x+5, gw->g.y+gw->g.height-5, gw->g.x+gw->g.width-5, gw->g.y+gw->g.height-5, White);
}

static void _closeBtnMax(struct GWidgetObject *gw, void *param) {
	(void) param;

	// the background
	if (gwinButtonIsPressed( (GHandle)gw)) {
		gdispFillArea(gw->g.x, gw->g.y, gw->g.width, gw->g.height, gdispBlendColor(Black, Grey, 50));
	} else {
		gdispFillArea(gw->g.x, gw->g.y, gw->g.width, gw->g.height, gdispBlendColor(White, Grey, 50));
	}

	// the symbol
	gdispDrawBox(gw->g.x+4, gw->g.y+4, gw->g.width-8, gw->g.height-8, White);
	gdispDrawLine(gw->g.x+4, gw->g.y+5, gw->g.x+gw->g.width-5, gw->g.y+5, White);
	gdispDrawLine(gw->g.x+4, gw->g.y+6, gw->g.x+gw->g.width-5, gw->g.y+6, White);
}

#if 0 && GINPUT_NEED_MOUSE
	static void _mouseDown(GWidgetObject *gw, coord_t x, coord_t y) {
	
	}

	static void _mouseUp(GWidgetObject *gw, coord_t x, coord_t y) {

	}

	static void _mouseMove(GWidgetObject *gw, coord_t x, coord_t y) {
	
	}
#endif

static const gcontainerVMT frameVMT = {
	{
		{
			"Frame",					// The classname
			sizeof(GFrameObject),		// The object size
			_frameDestroy,				// The destroy routie
			_gcontainerRedraw,			// The redraw routine
			0,							// The after-clear routine
		},
		gwinFrameDraw_Std,				// The default drawing routine
		#if GINPUT_NEED_MOUSE
			{
				0,//_mouseDown,				// Process mouse down event
				0,//_mouseUp,				// Process mouse up events
				0,//_mouseMove,				// Process mouse move events
			},
		#endif
		#if GINPUT_NEED_TOGGLE
			{
				0,						// 1 toggle role
				0,						// Assign Toggles
				0,						// Get Toggles
				0,						// Process toggle off events
				0,						// Process toggle on events
			},
		#endif
		#if GINPUT_NEED_DIAL
			{
				0,						// 1 dial roles
				0,						// Assign Dials
				0,						// Get Dials
				0,						// Process dial move events
			},
		#endif
	},
	BorderSizeLRB,						// The size of the left border (mandatory)
	BorderSizeT,						// The size of the top border (mandatory)
	BorderSizeLRB,						// The size of the right border (mandatory)
	BorderSizeLRB,						// The size of the bottom border (mandatory)
	0,									// A child has been added (optional)
	0,									// A child has been deleted (optional)
};

GHandle gwinGFrameCreate(GDisplay *g, GFrameObject *fo, GWidgetInit *pInit, uint32_t flags) {
	if (!(fo = (GFrameObject *)_gcontainerCreate(g, &fo->gc, pInit, &frameVMT)))
		return 0;

	fo->btnClose = 0;
	fo->btnMin = 0;
	fo->btnMax = 0;

	/* Buttons require a border */
	if ((flags & (GWIN_FRAME_CLOSE_BTN|GWIN_FRAME_MINMAX_BTN)))
		flags |= GWIN_FRAME_BORDER;

	/* create and initialize the listener if any button is present. */
	if ((flags & (GWIN_FRAME_CLOSE_BTN|GWIN_FRAME_MINMAX_BTN))) {
		geventListenerInit(&fo->gl);
		gwinAttachListener(&fo->gl);
		geventRegisterCallback(&fo->gl, _callbackBtn, (GHandle)fo);
	}

	/* create close button if necessary */
	if ((flags & GWIN_FRAME_CLOSE_BTN)) {
		GWidgetInit wi;

		gwinWidgetClearInit(&wi);
		wi.g.show = TRUE;
		wi.g.parent = &fo->gc.g;

		wi.g.x = fo->gc.g.width - BORDER_X - BUTTON_X;
		wi.g.y = (BORDER_Y - BUTTON_Y) / 2;
		wi.g.width = BUTTON_X;
		wi.g.height = BUTTON_Y;
		wi.text = "Frame Close Button";
		wi.customDraw = _closeBtnDraw;
		fo->btnClose = gwinGButtonCreate(g, 0, &wi);
	}

	/* create minimize and maximize buttons if necessary */
	if ((flags & GWIN_FRAME_MINMAX_BTN)) {
		GWidgetInit wi;

		gwinWidgetClearInit(&wi);
		wi.g.show = TRUE;
		wi.g.parent = &fo->gc.g;

		wi.g.x = (flags & GWIN_FRAME_CLOSE_BTN) ? fo->gc.g.width - 2*BORDER_X - 2*BUTTON_X : fo->gc.g.width - BORDER_X - BUTTON_X;
		wi.g.y = (BORDER_Y - BUTTON_Y) / 2;
		wi.g.width = BUTTON_X;
		wi.g.height = BUTTON_Y;
		wi.text = "Frame Max Button";
		wi.customDraw = _closeBtnMax;
		fo->btnMax = gwinGButtonCreate(g, 0, &wi);

		wi.g.x = (flags & GWIN_FRAME_CLOSE_BTN) ? fo->gc.g.width - 3*BORDER_X - 3*BUTTON_X : fo->gc.g.width - BORDER_X - BUTTON_X;
		wi.g.y = (BORDER_Y - BUTTON_Y) / 2;
		wi.g.width = BUTTON_X;
		wi.g.height = BUTTON_Y;
		wi.text = "Frame Min Button";
		wi.customDraw = _closeBtnMin;
		fo->btnMin = gwinGButtonCreate(g, 0, &wi);
	}

	/* Apply flags. We apply these here so the controls above are outside the child area */
	fo->gc.g.flags |= flags;

	gwinSetVisible(&fo->gc.g, pInit->g.show);

	return &fo->gc.g;
}

/* Process a button event */
static void _callbackBtn(void *param, GEvent *pe) {
	switch (pe->type) {
		case GEVENT_GWIN_BUTTON:
			if (((GEventGWinButton *)pe)->button == ((GFrameObject*)(GHandle)param)->btnClose)
				gwinDestroy((GHandle)param);

			else if (((GEventGWinButton *)pe)->button == ((GFrameObject*)(GHandle)param)->btnMin) {
				;/* ToDo */

			} else if (((GEventGWinButton *)pe)->button == ((GFrameObject*)(GHandle)param)->btnMax) {
				;/* ToDo */
			}

			break;

		default:
			break;
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Default render routines                                                                       //
///////////////////////////////////////////////////////////////////////////////////////////////////

static const GColorSet* _getDrawColors(GWidgetObject *gw) {
	if (!(gw->g.flags & GWIN_FLG_SYSENABLED))
		return &gw->pstyle->disabled;
	//if ((gw->g.flags & GBUTTON_FLG_PRESSED))
	//	return &gw->pstyle->pressed;

	return &gw->pstyle->enabled;
}

static void gwinFrameDraw_Std(GWidgetObject *gw, void *param) {
	const GColorSet		*pcol;
	(void)param;

	if (gw->g.vmt != (gwinVMT *)&frameVMT)
		return;

	pcol = _getDrawColors(gw);

	// Render the actual frame (with border, if any)
	if (gw->g.flags & GWIN_FRAME_BORDER) {
		gdispGFillArea(gw->g.display, gw->g.x + BORDER_X, gw->g.y + BORDER_Y, gw->g.width - 2*BORDER_X, gw->g.height - BORDER_Y - BORDER_X, gw->pstyle->background);
		gdispGFillStringBox(gw->g.display, gw->g.x, gw->g.y, gw->g.width, BORDER_Y, gw->text, gw->g.font, gdispContrastColor(pcol->edge), pcol->edge, justifyCenter);
		gdispGFillArea(gw->g.display, gw->g.x, gw->g.y+BORDER_Y, BORDER_X, gw->g.height-(BORDER_Y+BORDER_X), pcol->edge);
		gdispGFillArea(gw->g.display, gw->g.x+gw->g.width-BORDER_X, gw->g.y+BORDER_Y, BORDER_X, gw->g.height-(BORDER_Y+BORDER_X), pcol->edge);
		gdispGFillArea(gw->g.display, gw->g.x, gw->g.y+gw->g.height-BORDER_X, gw->g.width, BORDER_X, pcol->edge);
	} else {
		gdispGFillArea(gw->g.display, gw->g.x, gw->g.y, gw->g.width, gw->g.height, gw->pstyle->background);
	}
}

#endif  /* (GFX_USE_GWIN && GWIN_NEED_FRAME) || defined(__DOXYGEN__) */
