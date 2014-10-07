/*
 * Copyright (c) 2012, 2013, Joel Bodenmann aka Tectu <joel@unormal.org>
 * Copyright (c) 2012, 2013, Andrew Hannam aka inmarket
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *    * Redistributions of source code must retain the above copyright
 *      notice, this list of conditions and the following disclaimer.
 *    * Redistributions in binary form must reproduce the above copyright
 *      notice, this list of conditions and the following disclaimer in the
 *      documentation and/or other materials provided with the distribution.
 *    * Neither the name of the <organization> nor the
 *      names of its contributors may be used to endorse or promote products
 *      derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL <COPYRIGHT HOLDER> BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "gfx.h"

// We get nasty and look at some internal structures - get the relevant information
#include "src/gdriver/sys_defs.h"
#include "src/ginput/driver_mouse.h"

#include <string.h>

static GConsoleObject			gc;
static GListener				gl;
static font_t					font;

/*------------------------------------------------------------------------*
 * GINPUT Touch Driver Calibrator.                                        *
 *------------------------------------------------------------------------*/
int main(void) {
	GSourceHandle			gs;
	GEventMouse				*pem;
	coord_t					swidth, sheight;
	GHandle					ghc;
	bool_t					isFirstTime;
	bool_t					isCalibrated;
	bool_t					isTouch;
	bool_t					isFinger;
	const char *			isFingerText;
	const char *			deviceText;
	coord_t					bWidth, bHeight;
	GMouse *				m;
	GMouseVMT *				vmt;

	gfxInit();		// Initialize the display

	// Get the display dimensions
	swidth = gdispGetWidth();
	sheight = gdispGetHeight();

	// Create our title
	font = gdispOpenFont("UI2");
	gwinSetDefaultFont(font);
	bWidth = gdispGetStringWidth("Next", font);
	bHeight = gdispGetStringWidth("Prev", font);
	if (bHeight > bWidth) bWidth = bHeight;
	bWidth += 4;
	bHeight = gdispGetFontMetric(font, fontHeight)+2;
	gdispFillStringBox(0, 0, swidth, bHeight, "Touch Calibration", font, Red, White, justifyLeft);

	// Create our main display window
	{
		GWindowInit				wi;

		gwinClearInit(&wi);
		wi.show = TRUE; wi.x = 0; wi.y = bHeight; wi.width = swidth; wi.height = sheight-bHeight;
		ghc = gwinConsoleCreate(&gc, &wi);
	}
	gwinClear(ghc);

	// Initialize the listener
	geventListenerInit(&gl);

	// Copy the current mouse's VMT so we can play with it.
	m = (GMouse *)gdriverGetInstance(GDRIVER_TYPE_MOUSE, 0);
	if (!m) gfxHalt("No mouse instance 0");
	vmt = gfxAlloc(sizeof(GMouseVMT));
	if (!vmt) gfxHalt("Could not allocate memory for mouse VMT");
	memcpy(vmt, m->d.vmt, sizeof(GMouseVMT));

	// Swap VMT's on the current mouse to our RAM copy
	m->d.vmt = (const GDriverVMT *)vmt;

	// Listen for events
	gs = ginputGetMouse(0);
	geventAttachSource(&gl, gs, GLISTEN_MOUSEDOWNMOVES|GLISTEN_MOUSEMETA);

	// Is the mouse good enough initially for buttons?
	isFirstTime = TRUE;
	isCalibrated = (vmt->d.flags & GMOUSE_VFLG_CALIBRATE) ? FALSE : TRUE;
	if (isCalibrated)
		gdispFillStringBox(swidth-1*bWidth, 0, bWidth  , bHeight, "Next", font, Black, Gray, justifyCenter);

	/*
	 * Test: Device Type
	 */

StepDeviceType:
	gwinClear(ghc);
	gwinSetColor(ghc, Yellow);
	gwinPrintf(ghc, "\n1. Device Type\n\n");

	// Get the type of device and the current mode
	isTouch = (vmt->d.flags & GMOUSE_VFLG_TOUCH) ? TRUE : FALSE;
	isFinger = (m->flags & GMOUSE_FLG_FINGERMODE) ? TRUE : FALSE;
	isFingerText = isFinger ? "finger" : "pen";
	deviceText = isTouch ? isFingerText : "mouse";

	gwinSetColor(ghc, White);
	gwinPrintf(ghc, "This is detected as a %s device\n\n", isTouch ? "TOUCH" : "MOUSE");
	gwinPrintf(ghc, "It is currently in %s mode\n\n", isFinger ? "FINGER" : "PEN");

	if (!isCalibrated)
		gwinPrintf(ghc, "Press and release your %s to move on to the next test.\n", deviceText);
	else if (isFirstTime)
		gwinPrintf(ghc, "Press Next to continue.\n");
	else
		gwinPrintf(ghc, "Press Next or Back to continue.\n");

	while(1) {
		pem = (GEventMouse *)geventEventWait(&gl, TIME_INFINITE);
		if (isCalibrated) {
			if (pem->y < bHeight && pem->x >= swidth-2*bWidth) {
				if ((pem->buttons & GMETA_MOUSE_UP)) {
					if (pem->x >= swidth-bWidth)
						break;
					if (!isFirstTime)
						goto StepDrawing;
				}
			}
		} else if ((pem->buttons & GMETA_MOUSE_UP))
			break;
	}

	/*
	 * Test: Mouse raw reading
	 */

StepRawReading:
	gwinClear(ghc);
	gwinSetColor(ghc, Yellow);
	gwinPrintf(ghc, "\n2. Raw Mouse Output\n\n");

	// Make sure we are in uncalibrated mode
	m->flags &= ~(GMOUSE_FLG_CALIBRATE|GMOUSE_FLG_CLIP);

	gwinSetColor(ghc, White);
	if (isTouch)
		gwinPrintf(ghc, "Press and hold on the surface.\n\n");
	else
		gwinPrintf(ghc, "Press and hold the mouse button.\n\n");
	gwinPrintf(ghc, "The raw values coming from your mouse driver will display.\n\n");

	gwinPrintf(ghc, "Release your %s to move on to the next test.\n", deviceText);

	// For this test turn on ALL mouse movement events
	geventAttachSource(&gl, gs, GLISTEN_MOUSEDOWNMOVES|GLISTEN_MOUSEUPMOVES|GLISTEN_MOUSEMETA|GLISTEN_MOUSENOFILTER);

	while(1) {
		// Always sleep a bit first to enable other events. We actually don't
		// mind missing events for this test.
		gfxSleepMilliseconds(100);
		pem = (GEventMouse *)geventEventWait(&gl, TIME_INFINITE);
		if ((pem->buttons & GMETA_MOUSE_UP))
			break;
		gwinPrintf(ghc, "%u, %u z=%u b=0x%04x\n", pem->x, pem->y, pem->z, pem->buttons & ~GINPUT_MISSED_MOUSE_EVENT);
	}

	// Reset to calibrated
	if (isCalibrated) {
		m->flags |= GMOUSE_FLG_CLIP;
		if ((vmt->d.flags & GMOUSE_VFLG_CALIBRATE))
			m->flags |= GMOUSE_FLG_CALIBRATE;
	}

	// Reset to just changed movements.
	geventAttachSource(&gl, gs, GLISTEN_MOUSEDOWNMOVES|GLISTEN_MOUSEMETA);

	/*
	 * Test: Calibration
	 */

StepCalibrate:
	gwinClear(ghc);
	gwinSetColor(ghc, Yellow);
	gwinPrintf(ghc, "\n3. Calibration Jitter\n\n");
	gwinSetColor(ghc, White);
	if ((vmt->d.flags & GMOUSE_VFLG_CALIBRATE)) {
		gwinPrintf(ghc, "You will be presented with a number of points to touch.\nPress them in turn.\n\n"
				"If the calibration repeatedly fails, increase the jitter for %s calibration and try again.\n\n", isFingerText);
		gwinPrintf(ghc, "Press and release your %s to start the calibration.\n", deviceText);
	} else {
		gwinPrintf(ghc, "This device does not need calibration.\n\n");
	}
	if (isCalibrated)
		gwinPrintf(ghc, "Press Next or Back to continue.\n");
	else
		gwinPrintf(ghc, "Press and release your %s to move on to the next test.\n", deviceText);

	while(1) {
		pem = (GEventMouse *)geventEventWait(&gl, TIME_INFINITE);
		if (isCalibrated) {
			if (pem->y < bHeight && pem->x >= swidth-2*bWidth) {
				if ((pem->buttons & GMETA_MOUSE_UP)) {
					if (pem->x >= swidth-bWidth)
						break;
					goto StepRawReading;
				}
			}
		} else if ((pem->buttons & GMETA_MOUSE_UP))
			break;
	}

	// Calibrate
	if ((vmt->d.flags & GMOUSE_VFLG_CALIBRATE)) {
		ginputCalibrateMouse(0);
		isCalibrated = TRUE;

		// Calibration used the whole screen - re-establish our title and Next and Previous Buttons
		gdispFillStringBox(0, 0, swidth, bHeight, "Touch Calibration", font, Green, White, justifyLeft);
		gdispFillStringBox(swidth-2*bWidth, 0, bWidth-1, bHeight, "Prev", font, Black, Gray, justifyCenter);
		gdispFillStringBox(swidth-1*bWidth, 0, bWidth  , bHeight, "Next", font, Black, Gray, justifyCenter);
	}

	/*
	 * Test: Mouse coords
	 */

StepMouseCoords:
	gwinClear(ghc);
	gwinSetColor(ghc, Yellow);
	gwinPrintf(ghc, "\n4. Show Mouse Coordinates\n\n");

	gwinSetColor(ghc, White);
	if (isTouch)
		gwinPrintf(ghc, "Press and hold on the surface.\n\n");
	else
		gwinPrintf(ghc, "Press and hold the mouse button.\n\n");
	gwinPrintf(ghc, "Numbers will display in this window.\n"
			"Check the coordinates against where it should be on the screen.\n\n");

	gwinPrintf(ghc, "Press Next or Back to continue.\n");

	// For this test normal mouse movement events
	geventAttachSource(&gl, gs, GLISTEN_MOUSEDOWNMOVES|GLISTEN_MOUSEMETA);

	while(1) {
		// Always sleep a bit first to enable other events. We actually don't
		// mind missing events for this test.
		gfxSleepMilliseconds(100);
		pem = (GEventMouse *)geventEventWait(&gl, TIME_INFINITE);
		if (pem->y < bHeight && pem->x >= swidth-2*bWidth) {
			if ((pem->buttons & GMETA_MOUSE_UP)) {
				if (pem->x >= swidth-bWidth)
					break;
				goto StepCalibrate;
			}
		}
		gwinPrintf(ghc, "%u, %u\n", pem->x, pem->y);
	}

	// Reset to just changed movements.
	geventAttachSource(&gl, gs, GLISTEN_MOUSEDOWNMOVES|GLISTEN_MOUSEMETA);

	/*
	 * Test: Mouse movement jitter
	 */

StepMovementJitter:
	gwinClear(ghc);
	gwinSetColor(ghc, Yellow);
	gwinPrintf(ghc, "\n5. Movement Jitter\n\n");

	gwinSetColor(ghc, White);
	if (isTouch)
		gwinPrintf(ghc, "Press firmly on the surface and move around as if to draw.\n\n");
	else
		gwinPrintf(ghc, "Press and hold the mouse button and move around as if to draw.\n\n");

	gwinPrintf(ghc, "Dots will display in this window. Ensure that when you stop moving your %s that "
			"new dots stop displaying.\nNew dots should only display when your %s is moving.\n\n"
			"Adjust %s movement jitter to the smallest value that this reliably works for.\n\n", deviceText, deviceText, isFingerText);
	gwinPrintf(ghc, "Press Next or Back to continue.\n\n");

	while(1) {
		pem = (GEventMouse *)geventEventWait(&gl, TIME_INFINITE);
		if (pem->y < bHeight && pem->x >= swidth-2*bWidth) {
			if ((pem->buttons & GMETA_MOUSE_UP)) {
				if (pem->x >= swidth-bWidth)
					break;
				goto StepMouseCoords;
			}
		}
		if ((pem->buttons & GINPUT_MOUSE_BTN_LEFT))
			gwinPrintf(ghc, ".");
	}

	/*
	 * Test: Click Jitter
	 */

StepClickJitter:
	gwinClear(ghc);
	gwinSetColor(ghc, Yellow);
	gwinPrintf(ghc, "\n6. Click Jitter\n\n");

	gwinSetColor(ghc, White);
	if (isTouch)
		gwinPrintf(ghc, "Press and release the touch surface to \"click\".\nTry both short and long presses.\n");
	else
		gwinPrintf(ghc, "Click the mouse with the left and right buttons.\n\n");
	gwinPrintf(ghc, "Dots will display in this window. A yellow dash is a left (or short) click. "
			"A red x is a right (or long) click.\n\n"
			"Adjust %s click jitter to the smallest value that this reliably works for.\n"
			"Note: moving your %s during a click cancels it.\n\n", isFingerText, deviceText);
	gwinPrintf(ghc, "Press Next or Back to continue.\n\n");

	while(1) {
		pem = (GEventMouse *)geventEventWait(&gl, TIME_INFINITE);
		if (pem->y < bHeight && pem->x >= swidth-2*bWidth) {
			if ((pem->buttons & GMETA_MOUSE_UP)) {
				if (pem->x >= swidth-bWidth)
					break;
				goto StepMovementJitter;
			}
		}
		if ((pem->buttons & GMETA_MOUSE_CLICK)) {
			gwinSetColor(ghc, Yellow);
			gwinPrintf(ghc, "-");
		}
		if ((pem->buttons & GMETA_MOUSE_CXTCLICK)) {
			gwinSetColor(ghc, Red);
			gwinPrintf(ghc, "x");
		}
	}

	/*
	 * Test: Polling frequency
	 */

StepDrawing:
	gwinClear(ghc);
	gwinSetColor(ghc, Yellow);
	gwinPrintf(ghc, "\n7. Drawing\n\n");

	gwinSetColor(ghc, White);
	gwinPrintf(ghc, "Press firmly on the surface (or press and hold the mouse button) and move around as if to draw.\n\n");
	gwinPrintf(ghc, "A green line will follow your %s.\n\n", deviceText);
	gwinPrintf(ghc, "This is the last test but you can press Next or Back to continue.\n\n");

	while(1) {
		pem = (GEventMouse *)geventEventWait(&gl, TIME_INFINITE);
		if (pem->y < bHeight && pem->x >= swidth-2*bWidth) {
			if ((pem->buttons & GMETA_MOUSE_UP)) {
				if (pem->x >= swidth-bWidth)
					break;
				goto StepClickJitter;
			}
		}
		gdispDrawPixel(pem->x, pem->y, Green);
	}

	// Can't let this really exit
	isFirstTime = FALSE;
	goto StepDeviceType;
}
