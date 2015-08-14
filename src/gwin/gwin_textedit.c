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
#include <string.h>

// Some settings
const int TEXT_PADDING_LEFT			= 4;
const int FOCUS_BORDER_THICKNESS	= 3;
const int CURSOR_PADDING_LEFT		= 0;
const int CURSOR_EXTRA_HEIGHT		= 1;

// Some flags
#define GTEXTEDIT_FLG_BORDER	(GWIN_FIRST_CONTROL_FLAG << 0)

// Macros to assist in data type conversions
#define gh2obj ((GTexteditObject *)gh)
#define gw2obj ((GTexteditObject *)gw)

// cursorPos is the position of the next character
// textBuffer[cursorPos++] = readKey();

static void _shiftTextLeft(char* buffer, size_t bufferSize, size_t index)
{
	// Find the end of the string
	size_t indexTerminator = index;
	while (buffer[indexTerminator] != '\0' && indexTerminator < bufferSize-1) {
		indexTerminator++;
	}

	// Shift
	memcpy(&buffer[index-1], &buffer[index], indexTerminator-index);

	// Terminate the string
	buffer[indexTerminator-1] = '\0';
}

static void _shiftTextRight(char* buffer, size_t bufferSize, size_t index, char fillChar)
{
	// Find the end of the string
	size_t indexTerminator = index;
	while (buffer[indexTerminator] != '\0' && indexTerminator < bufferSize-1) {
		indexTerminator++;
	}

	// Shift
	memcpy(&buffer[index+1], &buffer[index], indexTerminator-index);

	// Fill the gap
	buffer[index] = fillChar;
}


#if GINPUT_NEED_KEYBOARD
	static void _keyboardEvent(GWidgetObject* gw, GEventKeyboard* pke)
	{
		// Is it a special key?
		if (pke->keystate & GKEYSTATE_SPECIAL) {
			// Arrow keys to move the cursor
			switch ((uint8_t)pke->c[0]) {
			case GKEY_LEFT:
				if (gw2obj->cursorPos > 0) {
					gw2obj->cursorPos--;
				}
				break;

			case GKEY_RIGHT:
				if (gw2obj->cursorPos < strlen(gw2obj->textBuffer)) {
					gw2obj->cursorPos++;
				}
				break;


			default:
				break;
			}
		}

		// Parse the key press
		else if (pke->bytecount >= 1) {
			// Is it backspace?
			if (pke->c[0] == GKEY_BACKSPACE) {
				if (gw2obj->cursorPos == 0) {
					return;
				}
				_shiftTextLeft(gw2obj->textBuffer, gw2obj->bufferSize, gw2obj->cursorPos--);
			}

			// Is it delete?
			else if (pke->c[0] == GKEY_DEL) {
				// Check whether there is a character on the right side of the cursor
				if (gw2obj->textBuffer[gw2obj->cursorPos] == '\0') {
					return;
				}
				_shiftTextLeft(gw2obj->textBuffer, gw2obj->bufferSize, gw2obj->cursorPos+1);
			}

			// Add a new character
			else {
				// Prevent buffer overflow
				if (gw2obj->cursorPos >= gw2obj->bufferSize) {
					return;
				}

				// Shift everything right from the cursor by one character. This includes the '\0'. Then inser the new character.
				_shiftTextRight(gw2obj->textBuffer, gw2obj->bufferSize, gw2obj->cursorPos++, pke->c[0]);
			}

			// Set the new text
			gwinSetText((GHandle)gw, gw2obj->textBuffer, FALSE);
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

GHandle gwinGTexteditCreate(GDisplay* g, GTexteditObject* widget, GWidgetInit* pInit, size_t bufSize)
{
	uint16_t flags = 0;

	// Create the underlying widget
	if (!(widget = (GTexteditObject*)_gwidgetCreate(g, &widget->w, pInit, &texteditVMT))) {
		return 0;
	}

	// Allocate the text buffer
	widget->bufferSize = bufSize;
	widget->textBuffer = gfxAlloc(widget->bufferSize);
	if (widget->textBuffer == 0) {
		return 0;
	}

	// Initialize the text buffer
	size_t i = 0;
	for (i = 0; i < bufSize; i++) {
		widget->textBuffer[i] = '\0';
	}

	// Set text and cursor position
	strncpy(widget->textBuffer, gwinGetText((GHandle)widget), widget->bufferSize);	// FixMe: pInit->text leads to a segfault
	widget->cursorPos = strlen(widget->textBuffer);

	widget->w.g.flags |= flags  | GTEXTEDIT_FLG_BORDER;;
	gwinSetVisible(&widget->w.g, pInit->g.show);

	return (GHandle)widget;
}

void gwinTexteditSetBorder(GHandle gh, bool_t border)
{
	// Is it a valid handle?
	if (gh->vmt != (gwinVMT*)&texteditVMT) {
		return;
	}

	if (border) {
		gh2obj->w.g.flags |= GTEXTEDIT_FLG_BORDER;
	} else {
		gh2obj->w.g.flags &=~ GTEXTEDIT_FLG_BORDER;
	}
}

static void gwinTexteditDefaultDraw(GWidgetObject* gw, void* param)
{

	(void)param;

	// Is it a valid handle?
	if (gw->g.vmt != (gwinVMT*)&texteditVMT) {
		return;
	}

	// Retrieve colors
	color_t textColor = (gw->g.flags & GWIN_FLG_SYSENABLED) ? gw->pstyle->enabled.text : gw->pstyle->disabled.text;
	color_t cursorColor = (gw->g.flags & GWIN_FLG_SYSENABLED) ? gw->pstyle->enabled.edge : gw->pstyle->disabled.edge;

	// Render background and string
	gdispGFillArea(gw->g.display, gw->g.x, gw->g.y, gw->g.width, gw->g.height, gw->pstyle->background);
	gdispGFillStringBox(gw->g.display, gw->g.x + TEXT_PADDING_LEFT, gw->g.y, gw->g.width, gw->g.height, gw->text, gw->g.font, textColor, gw->pstyle->background, justifyLeft);

	// Render border (if supposed to)
	if (gw2obj->w.g.flags & GTEXTEDIT_FLG_BORDER) {
		gdispGDrawBox(gw->g.display, gw->g.x, gw->g.y, gw->g.width, gw->g.height, (gw->g.flags & GWIN_FLG_SYSENABLED) ? gw->pstyle->enabled.edge : gw->pstyle->disabled.edge);
	}

	// Render highlighted border of focused
	if (gwinGetFocus() == (GHandle)gw) {
		int i = 0;
		for (i = 0; i < FOCUS_BORDER_THICKNESS; i++) {
			gdispGDrawBox(gw->g.display, gw->g.x+i, gw->g.y+i, gw->g.width-2*i, gw->g.height-2*i, gw->pstyle->focus);
		}
	}

	// Render cursor (if focused)
	if (gwinGetFocus() == (GHandle)gw) {
		// Calculate cursor stuff
		coord_t textWidth = gdispGetStringWidthCount(gw2obj->textBuffer, gw->g.font, gw2obj->cursorPos);
		coord_t cursorHeight = gdispGetFontMetric(gw->g.font, fontHeight);
		coord_t cursorSpacingTop = (gw->g.height - cursorHeight)/2 - CURSOR_EXTRA_HEIGHT;
		coord_t cursorSpacingBottom = (gw->g.height - cursorHeight)/2 - CURSOR_EXTRA_HEIGHT;

		// Draw cursor
		coord_t lineX0 = gw->g.x + textWidth + CURSOR_PADDING_LEFT + TEXT_PADDING_LEFT + gdispGetFontMetric(gw->g.font, fontBaselineX)/2;
		coord_t lineX1 = lineX0;
		coord_t lineY0 = gw->g.y + cursorSpacingTop;
		coord_t lineY1 = gw->g.y + gw->g.height - cursorSpacingBottom;
		gdispGDrawLine(gw->g.display, lineX0, lineY0, lineX1, lineY1, cursorColor);
	}
}

#undef gh2obj
#undef gw2obj

#endif // GFX_USE_GWIN && GWIN_NEED_TEXTEDIT
