/*
 * This file is subject to the terms of the GFX License. If a copy of
 * the license was not distributed with this file, you can obtain one at:
 *
 *              http://ugfx.org/license.html
 */

#include "gfx.h"

// Used by the NULL window manager
#define MIN_WIN_WIDTH	3
#define MIN_WIN_HEIGHT	3

/*-----------------------------------------------
 * The default window manager (GNullWindowManager)
 *-----------------------------------------------*/

#if GFX_USE_GWIN && GWIN_NEED_WINDOWMANAGER

#include "src/gwin/class_gwin.h"

/*-----------------------------------------------
 * Data
 *-----------------------------------------------*/

static void WM_Init(void);
static void WM_DeInit(void);
static bool_t WM_Add(GHandle gh, const GWindowInit *pInit);
static void WM_Delete(GHandle gh);
static void WM_Redraw(GHandle gh, int flags);
static void WM_Size(GHandle gh, coord_t w, coord_t h);
static void WM_Move(GHandle gh, coord_t x, coord_t y);
static void WM_Raise(GHandle gh);
static void WM_MinMax(GHandle gh, GWindowMinMax minmax);

static const gwmVMT GNullWindowManagerVMT = {
	WM_Init,
	WM_DeInit,
	WM_Add,
	WM_Delete,
	WM_Redraw,
	WM_Size,
	WM_Move,
	WM_Raise,
	WM_MinMax,
};

static const GWindowManager	GNullWindowManager = {
	&GNullWindowManagerVMT,
};

static gfxQueueASync	_GWINList;
GWindowManager *		_GWINwm;
#if GFX_USE_GTIMER
	static GTimer		RedrawTimer;
	static GDisplay *	RedrawDisplay;
	static bool_t		RedrawPreserve;
	static void			_gwinRedrawDisplay(void * param);
#endif

/*-----------------------------------------------
 * Window Routines
 *-----------------------------------------------*/

void _gwmInit(void)
{
	gfxQueueASyncInit(&_GWINList);
	_GWINwm = (GWindowManager *)&GNullWindowManager;
	_GWINwm->vmt->Init();
	#if GFX_USE_GTIMER
		gtimerInit(&RedrawTimer);
		gtimerStart(&RedrawTimer, _gwinRedrawDisplay, 0, TRUE, TIME_INFINITE);
	#endif
}

void _gwmDeinit(void)
{
	/* ToDo */
}

void gwinSetWindowManager(struct GWindowManager *gwm) {
	if (!gwm)
		gwm = (GWindowManager *)&GNullWindowManager;
	if (_GWINwm != gwm) {
		_GWINwm->vmt->DeInit();
		_GWINwm = gwm;
		_GWINwm->vmt->Init();
	}
}

void gwinSetMinMax(GHandle gh, GWindowMinMax minmax) {
	_GWINwm->vmt->MinMax(gh, minmax);
}

void gwinRaise(GHandle gh) {
	_GWINwm->vmt->Raise(gh);
}

GWindowMinMax gwinGetMinMax(GHandle gh) {
	if (gh->flags & GWIN_FLG_MINIMIZED)
		return GWIN_MINIMIZE;
	if (gh->flags & GWIN_FLG_MAXIMIZED)
		return GWIN_MAXIMIZE;
	return GWIN_NORMAL;
}

void gwinRedrawDisplay(GDisplay *g, bool_t preserve) {
	#if GFX_USE_GTIMER
			RedrawDisplay = g;
			RedrawPreserve = preserve;
			gtimerJab(&RedrawTimer);
		}
		static void	_gwinRedrawDisplay(void * param) {
			GDisplay *g			= RedrawDisplay;
			bool_t	preserve	= RedrawPreserve;
			(void) param;
	#endif

	GHandle						gh;

	for(gh = gwinGetNextWindow(0); gh; gh = gwinGetNextWindow(gh)) {
		if (!g || gh->display == g)
			_GWINwm->vmt->Redraw(gh,
					preserve ? (GWIN_WMFLG_PRESERVE|GWIN_WMFLG_NOBGCLEAR|GWIN_WMFLG_NOZORDER)
							:  (GWIN_WMFLG_NOBGCLEAR|GWIN_WMFLG_NOZORDER));
	}
}

/*-----------------------------------------------
 * "Null" Window Manager Routines
 *-----------------------------------------------*/

static void WM_Init(void) {
	// We don't need to do anything here.
	// A full window manager would move the windows around, add borders etc

	// clear the screen
	// cycle through the windows already defined displaying them
	// or cut all the window areas out of the screen and clear the remainder
}

static void WM_DeInit(void) {
	// We don't need to do anything here.
	// A full window manager would remove any borders etc
}

static bool_t WM_Add(GHandle gh, const GWindowInit *pInit) {
	// Note the window will not currently be marked as visible

	// Put it on the end of the queue
	gfxQueueASyncPut(&_GWINList, &gh->wmq);

	// Make sure the size/position is valid - prefer position over size.
	gh->width = MIN_WIN_WIDTH; gh->height = MIN_WIN_HEIGHT;
	gh->x = gh->y = 0;
	WM_Move(gh, pInit->x, pInit->y);
	WM_Size(gh, pInit->width, pInit->height);
	return TRUE;
}

static void WM_Delete(GHandle gh) {
	// Remove it from the window list
	gfxQueueASyncRemove(&_GWINList, &gh->wmq);
}

static void WM_Redraw(GHandle gh, int flags) {
	#if GWIN_NEED_CONTAINERS
		redo_redraw:
	#endif
	if ((gh->flags & GWIN_FLG_SYSVISIBLE)) {
		if (gh->vmt->Redraw) {
			#if GDISP_NEED_CLIP
				if (!(flags & GWIN_WMFLG_KEEPCLIP))
					gdispGSetClip(gh->display, gh->x, gh->y, gh->width, gh->height);
			#endif
			gh->vmt->Redraw(gh);
		} else if (!(flags & GWIN_WMFLG_PRESERVE)) {
			#if GDISP_NEED_CLIP
				if (!(flags & GWIN_WMFLG_KEEPCLIP))
					gdispGSetClip(gh->display, gh->x, gh->y, gh->width, gh->height);
			#endif
			gdispGFillArea(gh->display, gh->x, gh->y, gh->width, gh->height, gh->bgcolor);
			if (gh->vmt->AfterClear)
				gh->vmt->AfterClear(gh);
		}

		// A real window manager would also redraw the borders here

		// A real window manager would then redraw any higher z-order windows
		// if (!(flags & GWIN_WMFLG_NOZORDER))
		//		...

	} else if (!(flags & GWIN_WMFLG_NOBGCLEAR)) {
		#if GDISP_NEED_CLIP
			if (!(flags & GWIN_WMFLG_KEEPCLIP))
				gdispGSetClip(gh->display, gh->x, gh->y, gh->width, gh->height);
		#endif
		#if GWIN_NEED_CONTAINERS
			if (gh->parent) {
				// Get the parent to redraw the area
				gh = gh->parent;
				flags |= GWIN_WMFLG_KEEPCLIP;
				goto redo_redraw;
			}
		#endif
		gdispGFillArea(gh->display, gh->x, gh->y, gh->width, gh->height, gwinGetDefaultBgColor());
	}
}

static void WM_Size(GHandle gh, coord_t w, coord_t h) {
	coord_t		v;

	#if GWIN_NEED_CONTAINERS
		if (gh->parent) {
			// Clip to the container
			v = gh->parent->x + gh->parent->width - ((const gcontainerVMT *)gh->parent->vmt)->RightBorder(gh->parent);
			if (gh->x+w > v)	w = v - gh->x;
			v = gh->parent->y + gh->parent->height - ((const gcontainerVMT *)gh->parent->vmt)->BottomBorder(gh->parent);
			if (gh->y+h > v) 	h = v - gh->y;
		}
	#endif

	// Clip to the screen
	v = gdispGGetWidth(gh->display);
	if (gh->x+w > v) 	w = v - gh->x;
	v = gdispGGetHeight(gh->display);
	if (gh->y+h > v) 	h = v - gh->y;

	// Give it a minimum size
	if (w < MIN_WIN_WIDTH)	w = MIN_WIN_WIDTH;
	if (h < MIN_WIN_HEIGHT)	h = MIN_WIN_HEIGHT;

	// If there has been no resize just exit
	if (gh->width == w && gh->height == h)
		return;

	// Clear the old area and then redraw
	if ((gh->flags & GWIN_FLG_SYSVISIBLE)) {
		gh->flags &= ~GWIN_FLG_SYSVISIBLE;
		WM_Redraw(gh, 0);
		gh->width = w; gh->height = h;
		gh->flags |= GWIN_FLG_SYSVISIBLE;
		WM_Redraw(gh, 0);
	} else {
		gh->width = w; gh->height = h;
	}
}

static void WM_Move(GHandle gh, coord_t x, coord_t y) {
	coord_t		v;

	#if GWIN_NEED_CONTAINERS
		if (gh->parent) {
			// Clip to the parent size
			v = gh->parent->width - ((const gcontainerVMT *)gh->parent->vmt)->LeftBorder(gh->parent) - ((const gcontainerVMT *)gh->parent->vmt)->RightBorder(gh->parent);
			if (x+gh->width > v)	x = v-gh->width;
			v = gh->parent->height - ((const gcontainerVMT *)gh->parent->vmt)->TopBorder(gh->parent) - ((const gcontainerVMT *)gh->parent->vmt)->BottomBorder(gh->parent);
			if (y+gh->height > v)	y = v-gh->height;
			if (x < 0) x = 0;
			if (y < 0) y = 0;

			// Convert to absolute position
			x += gh->parent->x + ((const gcontainerVMT *)gh->parent->vmt)->LeftBorder(gh->parent);
			y += gh->parent->y + ((const gcontainerVMT *)gh->parent->vmt)->TopBorder(gh->parent);
		}
	#endif

	// Clip to the screen
	v = gdispGGetWidth(gh->display);
	if (x+gh->width > v)	x = v-gh->width;
	v = gdispGGetHeight(gh->display);
	if (y+gh->height > v)	y = v-gh->height;
	if (x < 0) x = 0;
	if (y < 0) y = 0;

	// If there has been no move just exit
	if (gh->x == x && gh->y == y)
		return;

	// Clear the old area and then redraw
	if ((gh->flags & GWIN_FLG_SYSVISIBLE)) {
		gh->flags &= ~GWIN_FLG_SYSVISIBLE;
		WM_Redraw(gh, 0);
		gh->x = x; gh->y = y;
		gh->flags |= GWIN_FLG_SYSVISIBLE;
		WM_Redraw(gh, 0);
	} else {
		gh->x = x; gh->y = y;
	}
}

static void WM_MinMax(GHandle gh, GWindowMinMax minmax) {
	(void)gh; (void) minmax;
	// We don't support minimising, maximising or restoring
}

static void WM_Raise(GHandle gh) {
	// Take it off the list and then put it back on top
	// The order of the list then reflects the z-order.

	gfxQueueASyncRemove(&_GWINList, &gh->wmq);
	gfxQueueASyncPut(&_GWINList, &gh->wmq);

	// Redraw the window
	WM_Redraw(gh, GWIN_WMFLG_PRESERVE|GWIN_WMFLG_NOBGCLEAR);
}

GHandle gwinGetNextWindow(GHandle gh) {
	return gh ? (GHandle)gfxQueueASyncNext(&gh->wmq) : (GHandle)gfxQueueASyncPeek(&_GWINList);
}

#endif /* GFX_USE_GWIN && GWIN_NEED_WINDOWMANAGER */
/** @} */
