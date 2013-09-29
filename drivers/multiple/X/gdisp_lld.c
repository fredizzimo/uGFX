/*
 * This file is subject to the terms of the GFX License. If a copy of
 * the license was not distributed with this file, you can obtain one at:
 *
 *              http://ugfx.org/license.html
 */

/**
 * @file    drivers/multiple/X/gdisp_lld.c
 * @brief   GDISP Graphics Driver subsystem low level driver source for X.
 */

#include "gfx.h"

#if GFX_USE_GDISP

#define GDISP_LLD_DECLARATIONS
#include "gdisp/lld/gdisp_lld.h"

/**
 * Our color model - Default or 24 bit only.
 *
 * At present we don't define this as we don't need to.
 * It may however be useful later if we implement bitblits.
 * As this may be dead code we don't include it in gdisp/options.h
 */
#ifndef GDISP_FORCE_24BIT	
	#define GDISP_FORCE_24BIT	FALSE
#endif

#ifndef GDISP_SCREEN_HEIGHT
	#define GDISP_SCREEN_HEIGHT		480
#endif
#ifndef GDISP_SCREEN_WIDTH
	#define GDISP_SCREEN_WIDTH		640
#endif

#if GINPUT_NEED_MOUSE
	/* Include mouse support code */
	#include "ginput/lld/mouse.h"
#endif

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <stdio.h>
#include <stdlib.h>

Display			*dis;
int				scr;
Window			win;
Pixmap			pix;
XEvent			evt;
GC 				gc;
Colormap		cmap;
XVisualInfo		vis;
int				depth;
#if GINPUT_NEED_MOUSE
	coord_t			mousex, mousey;
	uint16_t		mousebuttons;
#endif

static void ProcessEvent(void) {
	switch(evt.type) {
	case Expose:
		XCopyArea(dis, pix, win, gc,
			evt.xexpose.x, evt.xexpose.y,
			evt.xexpose.width, evt.xexpose.height,   
			evt.xexpose.x, evt.xexpose.y);
		break;
#if GINPUT_NEED_MOUSE
	case ButtonPress:
		mousex = evt.xbutton.x;
		mousey = evt.xbutton.y;
		switch(evt.xbutton.button){
		case 1:	mousebuttons |= GINPUT_MOUSE_BTN_LEFT;		break;
		case 2:	mousebuttons |= GINPUT_MOUSE_BTN_MIDDLE;	break;
		case 3:	mousebuttons |= GINPUT_MOUSE_BTN_RIGHT;		break;
		case 4:	mousebuttons |= GINPUT_MOUSE_BTN_4;			break;
		}
		#if GINPUT_MOUSE_POLL_PERIOD == TIME_INFINITE
			ginputMouseWakeup();
		#endif
		break;
	case ButtonRelease:
		mousex = evt.xbutton.x;
		mousey = evt.xbutton.y;
		switch(evt.xbutton.button){
		case 1:	mousebuttons &= ~GINPUT_MOUSE_BTN_LEFT;		break;
		case 2:	mousebuttons &= ~GINPUT_MOUSE_BTN_MIDDLE;	break;
		case 3:	mousebuttons &= ~GINPUT_MOUSE_BTN_RIGHT;	break;
		case 4:	mousebuttons &= ~GINPUT_MOUSE_BTN_4;		break;
		}
		#if GINPUT_MOUSE_POLL_PERIOD == TIME_INFINITE
			ginputMouseWakeup();
		#endif
		break;
	case MotionNotify:
		mousex = evt.xmotion.x;
		mousey = evt.xmotion.y;
		#if GINPUT_MOUSE_POLL_PERIOD == TIME_INFINITE
			ginputMouseWakeup();
		#endif
		break;
#endif
	}
}

/* this is the X11 thread which keeps track of all events */
static DECLARE_THREAD_STACK(waXThread, 1024);
static DECLARE_THREAD_FUNCTION(ThreadX, arg) {
	(void)arg;

	while(1) {
		gfxSleepMilliseconds(100);
		while(XPending(dis)) {
			XNextEvent(dis, &evt);
			ProcessEvent();
		}
	}
	return 0;
}
 
static int FatalXIOError(Display *d) {
	(void) d;

	/* The window has closed */
	fprintf(stderr, "GFX Window closed!\n");
	exit(0);
}

LLDSPEC bool_t gdisp_lld_init(GDISPDriver *g) {
	XSizeHints				*pSH;
	XSetWindowAttributes	xa;
	XTextProperty			WindowTitle;
	char *					WindowTitleText;
	gfxThreadHandle			hth;

	#if GFX_USE_OS_LINUX || GFX_USE_OS_OSX
		XInitThreads();
	#endif

	dis = XOpenDisplay(NULL);
	scr = DefaultScreen(dis);

	#if GDISP_FORCE_24BIT	
		if (!XMatchVisualInfo(dis, scr, 24, TrueColor, &vis)) {
			fprintf(stderr, "Your display has no TrueColor mode\n");
			XCloseDisplay(dis);
			return FALSE;
		}
		cmap = XCreateColormap(dis, RootWindow(dis, scr),
				vis.visual, AllocNone);
	#else
		vis.visual = CopyFromParent;
		vis.depth = DefaultDepth(dis, scr);
		cmap = DefaultColormap(dis, scr);
	#endif
	fprintf(stderr, "Running GFX Window in %d bit color\n", vis.depth);

	xa.colormap = cmap;
	xa.border_pixel = 0xFFFFFF;
	xa.background_pixel = 0x000000;
	
	win = XCreateWindow(dis, RootWindow(dis, scr), 16, 16,
			GDISP_SCREEN_WIDTH, GDISP_SCREEN_HEIGHT,
			0, vis.depth, InputOutput, vis.visual,
			CWBackPixel|CWColormap|CWBorderPixel, &xa);
	XSync(dis, TRUE);
	
	WindowTitleText = "GFX";
	XStringListToTextProperty(&WindowTitleText, 1, &WindowTitle);
	XSetWMName(dis, win, &WindowTitle);
	XSetWMIconName(dis, win, &WindowTitle);
	XSync(dis, TRUE);
			
	pSH = XAllocSizeHints();
	pSH->flags = PSize | PMinSize | PMaxSize;
	pSH->min_width = pSH->max_width = pSH->base_width = GDISP_SCREEN_WIDTH;
	pSH->min_height = pSH->max_height = pSH->base_height = GDISP_SCREEN_HEIGHT;
	XSetWMNormalHints(dis, win, pSH);
	XFree(pSH);
	XSync(dis, TRUE);
	
	pix = XCreatePixmap(dis, win, 
				GDISP_SCREEN_WIDTH, GDISP_SCREEN_HEIGHT, vis.depth);
	XSync(dis, TRUE);

	gc = XCreateGC(dis, win, 0, 0);
	XSetBackground(dis, gc, BlackPixel(dis, scr));
	XSync(dis, TRUE);

	XSelectInput(dis, win, StructureNotifyMask);
	XMapWindow(dis, win);
	do { XNextEvent(dis, &evt); } while (evt.type != MapNotify);

	/* start the X11 thread */
	XSetIOErrorHandler(FatalXIOError);
	XSelectInput(dis, win,
		ExposureMask | ButtonPressMask | ButtonReleaseMask | PointerMotionMask);

	if (!(hth = gfxThreadCreate(waXThread, sizeof(waXThread), HIGH_PRIORITY, ThreadX, 0))) {
		fprintf(stderr, "Cannot start X Thread\n");
		XCloseDisplay(dis);
		exit(0);
	}
	#if GFX_USE_OS_LINUX || GFX_USE_OS_OSX
		pthread_detach(hth);
	#endif
	gfxThreadClose(hth);
	
    /* Initialise the GDISP structure to match */
    g->g.Orientation = GDISP_ROTATE_0;
    g->g.Powermode = powerOn;
    g->g.Backlight = 100;
    g->g.Contrast = 50;
    g->g.Width = GDISP_SCREEN_WIDTH;
    g->g.Height = GDISP_SCREEN_HEIGHT;
    return TRUE;
}

LLDSPEC void gdisp_lld_draw_pixel(GDISPDriver *g)
{
	XColor	col;

	col.red = RED_OF(g->p.color) << 8;
	col.green = GREEN_OF(g->p.color) << 8;
	col.blue = BLUE_OF(g->p.color) << 8;
	XAllocColor(dis, cmap, &col);
	XSetForeground(dis, gc, col.pixel);
	XDrawPoint(dis, pix, gc, (int)g->p.x, (int)g->p.y );
	XDrawPoint(dis, win, gc, (int)g->p.x, (int)g->p.y );
	XFlush(dis);
}

#if GDISP_HARDWARE_FILLS
	LLDSPEC void gdisp_lld_fill_area(GDISPDriver *g) {
		XColor	col;

		col.red = RED_OF(g->p.color) << 8;
		col.green = GREEN_OF(g->p.color) << 8;
		col.blue = BLUE_OF(g->p.color) << 8;
		XAllocColor(dis, cmap, &col);
		XSetForeground(dis, gc, col.pixel);
		XFillRectangle(dis, pix, gc, g->p.x, g->p.y, g->p.cx, g->p.cy);
		XFillRectangle(dis, win, gc, g->p.x, g->p.y, g->p.cx, g->p.cy);
		XFlush(dis);
	}
#endif

#if 0 && GDISP_HARDWARE_BITFILLS
	LLDSPEC void gdisp_lld_blit_area(GDISPDriver *g) {
		// Start of Bitblit code

		//XImage			bitmap;
		//pixel_t			*bits;
		//	bits = malloc(vis.depth * GDISP_SCREEN_WIDTH * GDISP_SCREEN_HEIGHT);
		//	bitmap = XCreateImage(dis, vis, vis.depth, ZPixmap,
		//				0, bits, GDISP_SCREEN_WIDTH, GDISP_SCREEN_HEIGHT,
		//				0, 0);
	}
#endif

#if GDISP_HARDWARE_PIXELREAD
	LLDSPEC	color_t gdisp_lld_get_pixel_color(GDISPDriver *g) {
		XColor	color;
		XImage *img;

		img = XGetImage (dis, pix, g->p.x, g->p.y, 1, 1, AllPlanes, XYPixmap);
		color.pixel = XGetPixel (img, 0, 0);
		XFree(img);
		XQueryColor(dis, cmap, &color);
		return RGB2COLOR(color.red>>8, color.green>>8, color.blue>>8);
	}
#endif

#if GDISP_NEED_SCROLL && GDISP_HARDWARE_SCROLL
	LLDSPEC void gdisp_lld_vertical_scroll(GDISPDriver *g) {
		if (g->p.y1 > 0) {
			XCopyArea(dis, pix, pix, gc, g->p.x, g->p.y+g->p.y1, g->p.cx, g->p.cy-g->p.y1, g->p.x, g->p.y);
			XCopyArea(dis, pix, win, gc, g->p.x, g->p.y, g->p.cx, g->p.cy-g->p.y1, g->p.x, g->p.y);
		} else {
			XCopyArea(dis, pix, pix, gc, g->p.x, g->p.y, g->p.cx, g->p.cy+g->p.y1, g->p.x, g->p.y-g->p.y1);
			XCopyArea(dis, pix, win, gc, g->p.x, g->p.y-g->p.y1, g->p.cx, g->p.cy+g->p.y1, g->p.x, g->p.y-g->p.y1);
		}
	}
#endif

#if GINPUT_NEED_MOUSE

	void ginput_lld_mouse_init(void) {}

	void ginput_lld_mouse_get_reading(MouseReading *pt) {
		pt->x = mousex;
		pt->y = mousey;
		pt->z = (mousebuttons & GINPUT_MOUSE_BTN_LEFT) ? 100 : 0;
		pt->buttons = mousebuttons;
	}

#endif /* GINPUT_NEED_MOUSE */

#endif /* GFX_USE_GDISP */
/** @} */

