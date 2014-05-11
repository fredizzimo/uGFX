/*
 * This file is subject to the terms of the GFX License. If a copy of
 * the license was not distributed with this file, you can obtain one at:
 *
 *              http://ugfx.org/license.html
 */

#include "gfx.h"

#if GFX_USE_GWIN

#include "src/gwin/class_gwin.h"

// Needed if there is no window manager
#define MIN_WIN_WIDTH	1
#define MIN_WIN_HEIGHT	1

/*-----------------------------------------------
 * Data
 *-----------------------------------------------*/

static const gwinVMT basegwinVMT = {
		"GWIN",					// The classname
		sizeof(GWindowObject),	// The object size
		0,						// The destroy routine
		0,						// The redraw routine
		0,						// The after-clear routine
};

static color_t	defaultFgColor = White;
static color_t	defaultBgColor = Black;
#if GDISP_NEED_TEXT
	static font_t	defaultFont;
#endif

/*-----------------------------------------------
 * Helper Routines
 *-----------------------------------------------*/

#if GWIN_NEED_WINDOWMANAGER
	#define _gwm_redraw(gh, flags)		_GWINwm->vmt->Redraw(gh, flags)
	#define _gwm_move(gh,x,y)			_GWINwm->vmt->Move(gh,x,y);
	#define _gwm_resize(gh,w,h)			_GWINwm->vmt->Size(gh,w,h);
#else
	static void _gwm_redraw(GHandle gh, int flags) {
		if ((gh->flags & GWIN_FLG_SYSVISIBLE)) {
			if (gh->vmt->Redraw) {
				#if GDISP_NEED_CLIP
					gdispGSetClip(gh->display, gh->x, gh->y, gh->width, gh->height);
				#endif
				gh->vmt->Redraw(gh);
			} else if (!(flags & GWIN_WMFLG_PRESERVE)) {
				#if GDISP_NEED_CLIP
					gdispGSetClip(gh->display, gh->x, gh->y, gh->width, gh->height);
				#endif
				gdispGFillArea(gh->display, gh->x, gh->y, gh->width, gh->height, gh->bgcolor);
				if (gh->vmt->AfterClear)
					gh->vmt->AfterClear(gh);
			}
		} else if (!(flags & GWIN_WMFLG_NOBGCLEAR)) {
			#if GDISP_NEED_CLIP
				gdispGSetClip(gh->display, gh->x, gh->y, gh->width, gh->height);
			#endif
			gdispGFillArea(gh->display, gh->x, gh->y, gh->width, gh->height, defaultBgColor);
		}
	}
	static void _gwm_resize(GHandle gh, coord_t width, coord_t height) {
		gh->width = width; gh->height = height;
		if (gh->width < MIN_WIN_WIDTH) { gh->width = MIN_WIN_WIDTH; }
		if (gh->height < MIN_WIN_HEIGHT) { gh->height = MIN_WIN_HEIGHT; }
		if (gh->x+gh->width > gdispGetWidth()) gh->width = gdispGetWidth() - gh->x;
		if (gh->y+gh->height > gdispGetHeight()) gh->height = gdispGetHeight() - gh->y;
		_gwm_redraw(gh, GWIN_WMFLG_PRESERVE|GWIN_WMFLG_NOBGCLEAR);
	}
	static void _gwm_move(GHandle gh, coord_t x, coord_t y) {
		gh->x = x; gh->y = y;
		if (gh->x < 0) gh->x = 0;
		if (gh->y < 0) gh->y = 0;
		if (gh->x > gdispGetWidth()-MIN_WIN_WIDTH)		gh->x = gdispGetWidth()-MIN_WIN_WIDTH;
		if (gh->y > gdispGetHeight()-MIN_WIN_HEIGHT)	gh->y = gdispGetHeight()-MIN_WIN_HEIGHT;
		if (gh->x+gh->width > gdispGetWidth()) gh->width = gdispGetWidth() - gh->x;
		if (gh->y+gh->height > gdispGetHeight()) gh->height = gdispGetHeight() - gh->y;
		_gwm_redraw(gh, GWIN_WMFLG_PRESERVE|GWIN_WMFLG_NOBGCLEAR);
	}
#endif

/*-----------------------------------------------
 * Class Routines
 *-----------------------------------------------*/

void _gwinInit(void)
{
	#if GWIN_NEED_WINDOWMANAGER
		extern void _gwmInit(void);

		_gwmInit();
	#endif
	#if GWIN_NEED_WIDGET
		extern void _gwidgetInit(void);

		_gwidgetInit();
	#endif
	#if GWIN_NEED_CONTAINERS
		extern void _gcontainerInit(void);

		_gcontainerInit();
	#endif
}

void _gwinDeinit(void)
{
	#if GWIN_NEED_CONTAINERS
		extern void _gcontainerDeinit(void);

		_gcontainerDeinit();
	#endif
	#if GWIN_NEED_WIDGET
		extern void _gwidgetDeinit(void);

		_gwidgetDeinit();
	#endif
	#if GWIN_NEED_WINDOWMANAGER
		extern void _gwmDeinit(void);

		_gwmDeinit();
	#endif
}

// Internal routine for use by GWIN components only
// Initialise a window creating it dynamically if required.
GHandle _gwindowCreate(GDisplay *g, GWindowObject *pgw, const GWindowInit *pInit, const gwinVMT *vmt, uint32_t flags) {
	// Allocate the structure if necessary
	if (!pgw) {
		if (!(pgw = gfxAlloc(vmt->size)))
			return 0;
		pgw->flags = flags|GWIN_FLG_DYNAMIC;
	} else
		pgw->flags = flags;
	
	// Initialise all basic fields
	pgw->display = g;
	pgw->vmt = vmt;
	pgw->color = defaultFgColor;
	pgw->bgcolor = defaultBgColor;
	#if GDISP_NEED_TEXT
		pgw->font = defaultFont;
	#endif

	#if GWIN_NEED_CONTAINERS
		if (pInit->parent) {
			if (!(pInit->parent->flags & GWIN_FLG_CONTAINER) || pgw->display != pInit->parent->display) {
				if ((pgw->flags & GWIN_FLG_DYNAMIC))
					gfxFree(pgw);
				return 0;
			}
			pgw->parent = pInit->parent;
		} else
			pgw->parent = 0;
	#endif

	#if GWIN_NEED_WINDOWMANAGER
		if (!_GWINwm->vmt->Add(pgw, pInit)) {
			if ((pgw->flags & GWIN_FLG_DYNAMIC))
				gfxFree(pgw);
			return 0;
		}
	#else
		pgw->x = pgw->y = pgw->width = pgw->height = 0;
		_gwm_move(pgw, pInit->x, pInit->y);
		_gwm_resize(pgw, pInit->width, pInit->height);
	#endif

	#if GWIN_NEED_CONTAINERS
		// Notify the parent it has been added
		if (pgw->parent && ((gcontainerVMT *)pgw->parent->vmt)->NotifyAdd)
			((gcontainerVMT *)pgw->parent->vmt)->NotifyAdd(pgw->parent, pgw);
	#endif

	return (GHandle)pgw;
}

/*-----------------------------------------------
 * Routines that affect all windows
 *-----------------------------------------------*/

void gwinClearInit(GWindowInit *pwi) {
	char		*p;
	unsigned	len;

	for(p = (char *)pwi, len = sizeof(GWindowInit); len; len--)
		*p++ = 0;
}

void gwinSetDefaultColor(color_t clr) {
	defaultFgColor = clr;
}

color_t gwinGetDefaultColor(void) {
	return defaultFgColor;
}

void gwinSetDefaultBgColor(color_t bgclr) {
	defaultBgColor = bgclr;
}

color_t gwinGetDefaultBgColor(void) {
	return defaultBgColor;
}

#if GDISP_NEED_TEXT
	void gwinSetDefaultFont(font_t font) {
		defaultFont = font;
	}

	font_t gwinGetDefaultFont(void) {
		return defaultFont;
	}
#endif

/*-----------------------------------------------
 * The GWindow Routines
 *-----------------------------------------------*/

GHandle gwinGWindowCreate(GDisplay *g, GWindowObject *pgw, const GWindowInit *pInit) {
	if (!(pgw = _gwindowCreate(g, pgw, pInit, &basegwinVMT, 0)))
		return 0;

	gwinSetVisible(pgw, pInit->show);

	return pgw;
}

void gwinDestroy(GHandle gh) {
	if (!gh)
		return;

	// Make the window invisible
	gwinSetVisible(gh, FALSE);

	#if GWIN_NEED_CONTAINERS
		// Notify the parent it is about to be deleted
		if (gh->parent && ((gcontainerVMT *)gh->parent->vmt)->NotifyDelete)
			((gcontainerVMT *)gh->parent->vmt)->NotifyDelete(gh->parent, gh);
	#endif

	// Remove from the window manager
	#if GWIN_NEED_WINDOWMANAGER
		_GWINwm->vmt->Delete(gh);
	#endif

	// Class destroy routine
	if (gh->vmt->Destroy)
		gh->vmt->Destroy(gh);

	// Clean up the structure
	if (gh->flags & GWIN_FLG_DYNAMIC) {
		gh->flags = 0;							// To be sure, to be sure
		gfxFree((void *)gh);
	} else
		gh->flags = 0;							// To be sure, to be sure
}

const char *gwinGetClassName(GHandle gh) {
	return gh->vmt->classname;
}

#if GWIN_NEED_CONTAINERS
	// These two sub-functions set/clear system visibility recursively.
	static bool_t setSysVisFlag(GHandle gh) {
		// If we are now visible and our parent is visible
		if ((gh->flags & GWIN_FLG_VISIBLE) && (!gh->parent || (gh->parent->flags & GWIN_FLG_SYSVISIBLE))) {
			gh->flags |= GWIN_FLG_SYSVISIBLE;
			return TRUE;
		}
		return FALSE;
	}
	static bool_t clrSysVisFlag(GHandle gh) {
		// If we are now not visible but our parent is visible
		if (!(gh->flags & GWIN_FLG_VISIBLE) || (gh->parent && !(gh->parent->flags & GWIN_FLG_SYSVISIBLE))) {
			gh->flags &= ~GWIN_FLG_SYSVISIBLE;
			return TRUE;
		}
		return FALSE;
	}
	void gwinSetVisible(GHandle gh, bool_t visible) {
		if (visible) {
			if (!(gh->flags & GWIN_FLG_VISIBLE)) {
				gh->flags |= GWIN_FLG_VISIBLE;
				_gwinRecurse(gh, setSysVisFlag);
				_gwm_redraw(gh, 0);
			}
		} else {
			if ((gh->flags & GWIN_FLG_VISIBLE)) {
				gh->flags &= ~GWIN_FLG_VISIBLE;
				_gwinRecurse(gh, clrSysVisFlag);
				_gwm_redraw(gh, 0);
			}
		}
	}
#else
	void gwinSetVisible(GHandle gh, bool_t visible) {
		if (visible) {
			if (!(gh->flags & GWIN_FLG_VISIBLE)) {
				gh->flags |= (GWIN_FLG_VISIBLE|GWIN_FLG_SYSVISIBLE);
				_gwm_redraw(gh, 0);
			}
		} else {
			if ((gh->flags & GWIN_FLG_VISIBLE)) {
				gh->flags &= ~(GWIN_FLG_VISIBLE|GWIN_FLG_SYSVISIBLE);
				_gwm_redraw(gh, 0);
			}
		}
	}
#endif

bool_t gwinGetVisible(GHandle gh) {
	return (gh->flags & GWIN_FLG_SYSVISIBLE) ? TRUE : FALSE;
}

#if GWIN_NEED_CONTAINERS
	// These two sub-functions set/clear system enable recursively.
	static bool_t setSysEnaFlag(GHandle gh) {
		// If we are now enabled and our parent is enabled
		if ((gh->flags & GWIN_FLG_ENABLED) && (!gh->parent || (gh->parent->flags & GWIN_FLG_SYSENABLED))) {
			gh->flags |= GWIN_FLG_SYSENABLED;
			return TRUE;
		}
		return FALSE;
	}
	static bool_t clrSysEnaFlag(GHandle gh) {
		// If we are now not enabled but our parent is enabled
		if (!(gh->flags & GWIN_FLG_ENABLED) || (gh->parent && !(gh->parent->flags & GWIN_FLG_SYSENABLED))) {
			gh->flags &= ~GWIN_FLG_SYSENABLED;
			return TRUE;
		}
		return FALSE;
	}
	void gwinSetEnabled(GHandle gh, bool_t enabled) {
		if (enabled) {
			if (!(gh->flags & GWIN_FLG_ENABLED)) {
				gh->flags |= GWIN_FLG_ENABLED;
				_gwinRecurse(gh, setSysEnaFlag);
				if ((gh->flags & GWIN_FLG_SYSVISIBLE))
					_gwm_redraw(gh, GWIN_WMFLG_PRESERVE);
			}
		} else {
			if ((gh->flags & GWIN_FLG_ENABLED)) {
				gh->flags &= ~GWIN_FLG_ENABLED;
				_gwinRecurse(gh, clrSysEnaFlag);
				if ((gh->flags & GWIN_FLG_SYSVISIBLE))
					_gwm_redraw(gh, GWIN_WMFLG_PRESERVE);
			}
		}
	}
#else
	void gwinSetEnabled(GHandle gh, bool_t enabled) {
		if (enabled) {
			if (!(gh->flags & GWIN_FLG_ENABLED)) {
				gh->flags |= (GWIN_FLG_ENABLED|GWIN_FLG_SYSENABLED);
				_gwm_redraw(gh, GWIN_WMFLG_PRESERVE|GWIN_WMFLG_NOBGCLEAR);
			}
		} else {
			if ((gh->flags & GWIN_FLG_ENABLED)) {
				gh->flags &= ~(GWIN_FLG_ENABLED|GWIN_FLG_SYSENABLED);
				_gwm_redraw(gh, GWIN_WMFLG_PRESERVE|GWIN_WMFLG_NOBGCLEAR);
			}
		}
	}
#endif

bool_t gwinGetEnabled(GHandle gh) {
	return (gh->flags & GWIN_FLG_SYSENABLED) ? TRUE : FALSE;
}

void gwinMove(GHandle gh, coord_t x, coord_t y) {
	_gwm_move(gh, x, y);
}

void gwinResize(GHandle gh, coord_t width, coord_t height) {
	_gwm_resize(gh, width, height);
}

void gwinRedraw(GHandle gh) {
	_gwm_redraw(gh, GWIN_WMFLG_PRESERVE|GWIN_WMFLG_NOBGCLEAR);
}

#if GDISP_NEED_TEXT
	void gwinSetFont(GHandle gh, font_t font) {
		gh->font = font;
	}
#endif

void gwinClear(GHandle gh) {
	/*
	 * Don't render anything when the window is not visible but 
	 * still call the AfterClear() routine as some widgets will
	 * need this to clear internal buffers or similar
	 */
	if (!(gh->flags & GWIN_FLG_SYSVISIBLE)) {
		if (gh->vmt->AfterClear)
			gh->vmt->AfterClear(gh);
	} else {

		#if GDISP_NEED_CLIP
			gdispGSetClip(gh->display, gh->x, gh->y, gh->width, gh->height);
		#endif

		gdispGFillArea(gh->display, gh->x, gh->y, gh->width, gh->height, gh->bgcolor);
		if (gh->vmt->AfterClear)
			gh->vmt->AfterClear(gh);
	}

	#if GWIN_NEED_CONTAINERS
		for (gh = gwinGetFirstChild(gh); gh; gh = gwinGetSibling(gh))
			gwinRedraw(gh);
	#endif
}

void gwinDrawPixel(GHandle gh, coord_t x, coord_t y) {
	if (!(gh->flags & GWIN_FLG_SYSVISIBLE))
		return;

	#if GDISP_NEED_CLIP
		gdispGSetClip(gh->display, gh->x, gh->y, gh->width, gh->height);
	#endif
	gdispGDrawPixel(gh->display, gh->x+x, gh->y+y, gh->color);
}

void gwinDrawLine(GHandle gh, coord_t x0, coord_t y0, coord_t x1, coord_t y1) {
	if (!(gh->flags & GWIN_FLG_SYSVISIBLE))
		return;

	#if GDISP_NEED_CLIP
		gdispGSetClip(gh->display, gh->x, gh->y, gh->width, gh->height);
	#endif
	gdispGDrawLine(gh->display, gh->x+x0, gh->y+y0, gh->x+x1, gh->y+y1, gh->color);
}

void gwinDrawBox(GHandle gh, coord_t x, coord_t y, coord_t cx, coord_t cy) {
	if (!(gh->flags & GWIN_FLG_SYSVISIBLE))
		return;

	#if GDISP_NEED_CLIP
		gdispGSetClip(gh->display, gh->x, gh->y, gh->width, gh->height);
	#endif
	gdispGDrawBox(gh->display, gh->x+x, gh->y+y, cx, cy, gh->color);
}

void gwinFillArea(GHandle gh, coord_t x, coord_t y, coord_t cx, coord_t cy) {
	if (!(gh->flags & GWIN_FLG_SYSVISIBLE))
		return;

	#if GDISP_NEED_CLIP
		gdispGSetClip(gh->display, gh->x, gh->y, gh->width, gh->height);
	#endif
	gdispGFillArea(gh->display, gh->x+x, gh->y+y, cx, cy, gh->color);
}

void gwinBlitArea(GHandle gh, coord_t x, coord_t y, coord_t cx, coord_t cy, coord_t srcx, coord_t srcy, coord_t srccx, const pixel_t *buffer) {
	if (!(gh->flags & GWIN_FLG_SYSVISIBLE))
		return;

	#if GDISP_NEED_CLIP
		gdispGSetClip(gh->display, gh->x, gh->y, gh->width, gh->height);
	#endif
	gdispGBlitArea(gh->display, gh->x+x, gh->y+y, cx, cy, srcx, srcy, srccx, buffer);
}

#if GDISP_NEED_CIRCLE
	void gwinDrawCircle(GHandle gh, coord_t x, coord_t y, coord_t radius) {
		if (!(gh->flags & GWIN_FLG_SYSVISIBLE))
			return;

		#if GDISP_NEED_CLIP
			gdispGSetClip(gh->display, gh->x, gh->y, gh->width, gh->height);
		#endif
		gdispGDrawCircle(gh->display, gh->x+x, gh->y+y, radius, gh->color);
	}

	void gwinFillCircle(GHandle gh, coord_t x, coord_t y, coord_t radius) {
		if (!(gh->flags & GWIN_FLG_SYSVISIBLE))
			return;

		#if GDISP_NEED_CLIP
			gdispGSetClip(gh->display, gh->x, gh->y, gh->width, gh->height);
		#endif
		gdispGFillCircle(gh->display, gh->x+x, gh->y+y, radius, gh->color);
	}
#endif

#if GDISP_NEED_ELLIPSE
	void gwinDrawEllipse(GHandle gh, coord_t x, coord_t y, coord_t a, coord_t b) {
		if (!(gh->flags & GWIN_FLG_SYSVISIBLE))
			return;

		#if GDISP_NEED_CLIP
			gdispGSetClip(gh->display, gh->x, gh->y, gh->width, gh->height);
		#endif
		gdispGDrawEllipse(gh->display, gh->x+x, gh->y+y, a, b, gh->color);
	}

	void gwinFillEllipse(GHandle gh, coord_t x, coord_t y, coord_t a, coord_t b) {
		if (!(gh->flags & GWIN_FLG_SYSVISIBLE))
			return;

		#if GDISP_NEED_CLIP
			gdispGSetClip(gh->display, gh->x, gh->y, gh->width, gh->height);
		#endif
		gdispGFillEllipse(gh->display, gh->x+x, gh->y+y, a, b, gh->color);
	}
#endif

#if GDISP_NEED_ARC
	void gwinDrawArc(GHandle gh, coord_t x, coord_t y, coord_t radius, coord_t startangle, coord_t endangle) {
		if (!(gh->flags & GWIN_FLG_SYSVISIBLE))
			return;

		#if GDISP_NEED_CLIP
			gdispGSetClip(gh->display, gh->x, gh->y, gh->width, gh->height);
		#endif
		gdispGDrawArc(gh->display, gh->x+x, gh->y+y, radius, startangle, endangle, gh->color);
	}

	void gwinFillArc(GHandle gh, coord_t x, coord_t y, coord_t radius, coord_t startangle, coord_t endangle) {
		if (!(gh->flags & GWIN_FLG_SYSVISIBLE))
			return;

		#if GDISP_NEED_CLIP
			gdispGSetClip(gh->display, gh->x, gh->y, gh->width, gh->height);
		#endif
		gdispGFillArc(gh->display, gh->x+x, gh->y+y, radius, startangle, endangle, gh->color);
	}
#endif

#if GDISP_NEED_PIXELREAD
	color_t gwinGetPixelColor(GHandle gh, coord_t x, coord_t y) {
		if (!(gh->flags & GWIN_FLG_SYSVISIBLE))
			return defaultBgColor;

		#if GDISP_NEED_CLIP
			gdispGSetClip(gh->display, gh->x, gh->y, gh->width, gh->height);
		#endif
		return gdispGGetPixelColor(gh->display, gh->x+x, gh->y+y);
	}
#endif

#if GDISP_NEED_TEXT
	void gwinDrawChar(GHandle gh, coord_t x, coord_t y, char c) {
		if (!(gh->flags & GWIN_FLG_SYSVISIBLE) || !gh->font)
			return;

		#if GDISP_NEED_CLIP
			gdispGSetClip(gh->display, gh->x, gh->y, gh->width, gh->height);
		#endif
		gdispGDrawChar(gh->display, gh->x+x, gh->y+y, c, gh->font, gh->color);
	}

	void gwinFillChar(GHandle gh, coord_t x, coord_t y, char c) {
		if (!(gh->flags & GWIN_FLG_SYSVISIBLE) || !gh->font)
			return;

		#if GDISP_NEED_CLIP
			gdispGSetClip(gh->display, gh->x, gh->y, gh->width, gh->height);
		#endif
		gdispGFillChar(gh->display, gh->x+x, gh->y+y, c, gh->font, gh->color, gh->bgcolor);
	}

	void gwinDrawString(GHandle gh, coord_t x, coord_t y, const char *str) {
		if (!(gh->flags & GWIN_FLG_SYSVISIBLE) || !gh->font)
			return;

		#if GDISP_NEED_CLIP
			gdispGSetClip(gh->display, gh->x, gh->y, gh->width, gh->height);
		#endif
		gdispGDrawString(gh->display, gh->x+x, gh->y+y, str, gh->font, gh->color);
	}

	void gwinFillString(GHandle gh, coord_t x, coord_t y, const char *str) {
		if (!(gh->flags & GWIN_FLG_SYSVISIBLE) || !gh->font)
			return;

		#if GDISP_NEED_CLIP
			gdispGSetClip(gh->display, gh->x, gh->y, gh->width, gh->height);
		#endif
		gdispGFillString(gh->display, gh->x+x, gh->y+y, str, gh->font, gh->color, gh->bgcolor);
	}

	void gwinDrawStringBox(GHandle gh, coord_t x, coord_t y, coord_t cx, coord_t cy, const char* str, justify_t justify) {
		if (!(gh->flags & GWIN_FLG_SYSVISIBLE) || !gh->font)
			return;

		#if GDISP_NEED_CLIP
			gdispGSetClip(gh->display, gh->x, gh->y, gh->width, gh->height);
		#endif
		gdispGDrawStringBox(gh->display, gh->x+x, gh->y+y, cx, cy, str, gh->font, gh->color, justify);
	}

	void gwinFillStringBox(GHandle gh, coord_t x, coord_t y, coord_t cx, coord_t cy, const char* str, justify_t justify) {
		if (!(gh->flags & GWIN_FLG_SYSVISIBLE) || !gh->font)
			return;

		#if GDISP_NEED_CLIP
			gdispGSetClip(gh->display, gh->x, gh->y, gh->width, gh->height);
		#endif
		gdispGFillStringBox(gh->display, gh->x+x, gh->y+y, cx, cy, str, gh->font, gh->color, gh->bgcolor, justify);
	}
#endif

#if GDISP_NEED_CONVEX_POLYGON
	void gwinDrawPoly(GHandle gh, coord_t tx, coord_t ty, const point *pntarray, unsigned cnt) {
		if (!(gh->flags & GWIN_FLG_SYSVISIBLE))
			return;

		#if GDISP_NEED_CLIP
			gdispGSetClip(gh->display, gh->x, gh->y, gh->width, gh->height);
		#endif
		gdispGDrawPoly(gh->display, tx+gh->x, ty+gh->y, pntarray, cnt, gh->color);
	}

	void gwinFillConvexPoly(GHandle gh, coord_t tx, coord_t ty, const point *pntarray, unsigned cnt) {
		if (!(gh->flags & GWIN_FLG_SYSVISIBLE))
			return;

		#if GDISP_NEED_CLIP
			gdispGSetClip(gh->display, gh->x, gh->y, gh->width, gh->height);
		#endif
		gdispGFillConvexPoly(gh->display, tx+gh->x, ty+gh->y, pntarray, cnt, gh->color);
	}
#endif

#if GDISP_NEED_IMAGE
	gdispImageError gwinDrawImage(GHandle gh, gdispImage *img, coord_t x, coord_t y, coord_t cx, coord_t cy, coord_t sx, coord_t sy) {
		if (!(gh->flags & GWIN_FLG_SYSVISIBLE))
			return GDISP_IMAGE_ERR_OK;

		#if GDISP_NEED_CLIP
			gdispGSetClip(gh->display, gh->x, gh->y, gh->width, gh->height);
		#endif
		return gdispGImageDraw(gh->display, img, gh->x+x, gh->y+y, cx, cy, sx, sy);
	}
#endif

#endif /* GFX_USE_GWIN */
/** @} */

