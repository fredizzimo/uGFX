/*
 * This file is subject to the terms of the GFX License. If a copy of
 * the license was not distributed with this file, you can obtain one at:
 *
 *              http://ugfx.org/license.html
 */

/**
 * @file	src/gwin/gwin_textedit.c
 * @brief	GWIN TextEdit widget header file
 */

#include "gfx.h"

#if GFX_USE_GWIN && GWIN_NEED_TEXTEDIT

#include "gwin_class.h"

// Text padding (between edge and text) in pixels
const int TEXT_PADDING			= 3;

// macros to assist in data type conversions
#define gh2obj					((GTexteditObject *)gh)
#define gw2obj					((GTexteditObject *)gw)

#if GINPUT_NEED_KEYBOARD
	static void _keyboardEvent(GWidgetObject* gw, GEventKeyboard* pke)
	{
		// Create a temporary buffer containing the current size
		unsigned bufSize = strlen(gwinGetText((GHandle)gw))+1;
		char buf[bufSize];
		strncpy(buf, gwinGetText((GHandle)gw), bufSize);

		// Parse the key press
		if (pke->bytecount == 1) {
			// Check backspace
			if (pke->c[0] == GKEY_BACKSPACE) {
				buf[strlen(buf)-1] = '\0';
			}

			// Append new character
			else {
				strncat(buf, &(pke->c[0]), 1);
			}

			// Set the new text
			gwinSetText((GHandle)gw, buf, TRUE);
		}

		_gwinUpdate((GHandle)gw);
	}
#endif

static void gwinTexteditDefaultDraw(GWidgetObject* gw, void* param);

static const gwidgetVMT texteditVMT = {
	{
		"TextEdit",					// The class name
		sizeof(GTexteditObject),	// The object size
		_gwidgetDestroy,			// The destroy routine
		_gwidgetRedraw, 			// The redraw routine
		0,							// The after-clear routine
	},
	gwinTexteditDefaultDraw,		// default drawing routine
	#if GINPUT_NEED_MOUSE
		{
			0,						// Process mose down events (NOT USED)
			0,						// Process mouse up events (NOT USED)
			0,						// Process mouse move events (NOT USED)
		},
	#endif
	#if GINPUT_NEED_KEYBOARD
		{
			_keyboardEvent,			// Process keyboard key down events
		},
	#endif
	#if GINPUT_NEED_TOGGLE
		{
			0,						// No toggle role
			0,						// Assign Toggles (NOT USED)
			0,						// Get Toggles (NOT USED)
			0,						// Process toggle off event (NOT USED)
			0,						// Process toggle on event (NOT USED)
		},
	#endif
	#if GINPUT_NEED_DIAL
		{
			0,						// No dial roles
			0,						// Assign Dials (NOT USED)
			0, 						// Get Dials (NOT USED)
			0,						// Procees dial move events (NOT USED)
		},
	#endif
};

GHandle gwinGTexteditCreate(GDisplay* g, GTexteditObject* widget, GWidgetInit* pInit)
{
	uint16_t flags = 0;

	if (!(widget = (GTexteditObject*)_gwidgetCreate(g, &widget->w, pInit, &texteditVMT))) {
		return 0;
	}

	widget->w.g.flags |= flags;	
	gwinSetVisible(&widget->w.g, pInit->g.show);

	return (GHandle)widget;
}

static void gwinTexteditDefaultDraw(GWidgetObject* gw, void* param)
{
	color_t				textColor;
	(void)				param;

	// Is it a valid handle?
	if (gw->g.vmt != (gwinVMT*)&texteditVMT) {
		return;
	}

	textColor = (gw->g.flags & GWIN_FLG_SYSENABLED) ? gw->pstyle->enabled.text : gw->pstyle->disabled.text;

	gdispGFillStringBox(gw->g.display, gw->g.x, gw->g.y, gw->g.width, gw->g.height, gw->text, gw->g.font, textColor, gw->pstyle->background, justifyLeft);
}

#undef gh2obj
#undef gw2obj

#endif // GFX_USE_GWIN && GWIN_NEED_TEXTEDIT
