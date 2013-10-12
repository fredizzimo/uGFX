/*
 * This file is subject to the terms of the GFX License. If a copy of
 * the license was not distributed with this file, you can obtain one at:
 *
 *              http://ugfx.org/license.html
 */

/**
 * @file    src/gdisp/gdisp.c
 * @brief   GDISP Driver code.
 *
 * @addtogroup GDISP
 * @{
 */
#include "gfx.h"

#if GFX_USE_GDISP

/* Include the low level driver information */
#include "gdisp/lld/gdisp_lld.h"

#if !GDISP_HARDWARE_STREAM_WRITE && !GDISP_HARDWARE_DRAWPIXEL
	#error "GDISP Driver: Either GDISP_HARDWARE_STREAM_WRITE or GDISP_HARDWARE_DRAWPIXEL must be defined"
#endif

#if 1
	#undef INLINE
	#define INLINE	inline
#else
	#undef INLINE
	#define INLINE
#endif

// Number of milliseconds for the startup logo - 0 means disabled.
#define GDISP_STARTUP_LOGO_TIMEOUT		1000

/*===========================================================================*/
/* Driver local variables.                                                   */
/*===========================================================================*/

// The controller array, the display array and the default display
#if GDISP_TOTAL_CONTROLLERS > 1
	typedef const struct GDISPVMT const	VMTEL[1];
	extern VMTEL			GDISP_CONTROLLER_LIST;
	static const struct GDISPVMT const *	ControllerList[GDISP_TOTAL_CONTROLLERS] = {GDISP_CONTROLLER_LIST};
	static const unsigned					DisplayCountList[GDISP_TOTAL_CONTROLLERS] = {GDISP_CONTROLLER_DISPLAYS};
#endif
static GDisplay GDisplayArray[GDISP_TOTAL_DISPLAYS];
GDisplay	*GDISP = GDisplayArray;

#if GDISP_NEED_MULTITHREAD
	#define MUTEX_INIT(g)		gfxMutexInit(&(g)->mutex)
	#define MUTEX_ENTER(g)		gfxMutexEnter(&(g)->mutex)
	#define MUTEX_EXIT(g)		gfxMutexExit(&(g)->mutex)
#else
	#define MUTEX_INIT(g)
	#define MUTEX_ENTER(g)
	#define MUTEX_EXIT(g)
#endif

#define NEED_CLIPPING	(!GDISP_HARDWARE_CLIP && (GDISP_NEED_VALIDATION || GDISP_NEED_CLIP))


/*==========================================================================*/
/* Internal functions.														*/
/*==========================================================================*/

#if GDISP_HARDWARE_STREAM_POS && GDISP_HARDWARE_STREAM_WRITE
	static INLINE void setglobalwindow(GDisplay *g) {
		coord_t	x, y;
		x = g->p.x; y = g->p.y;
		g->p.cx = g->g.Width; g->p.cy = g->g.Height;
		gdisp_lld_write_start(g);
		g->p.x = x; g->p.y = y;
		g->flags |= GDISP_FLG_SCRSTREAM;
	}
#endif

// drawpixel_clip(g)
// Parameters:	x,y
// Alters:		cx, cy (if using streaming)
#if GDISP_HARDWARE_DRAWPIXEL
	// Best is hardware accelerated pixel draw
	#if NEED_CLIPPING
		static INLINE void drawpixel_clip(GDisplay *g) {
			if (g->p.x >= g->clipx0 && g->p.x < g->clipx1 && g->p.y >= g->clipy0 && g->p.y < g->clipy1)
				gdisp_lld_draw_pixel(g);
		}
	#else
		#define drawpixel_clip(g)	gdisp_lld_draw_pixel(g)
	#endif
#elif GDISP_HARDWARE_STREAM_POS
	// Next best is cursor based streaming
	static INLINE void drawpixel_clip(GDisplay *g) {
		#if NEED_CLIPPING
			if (g->p.x < g->clipx0 || g->p.x >= g->clipx1 || g->p.y < g->clipy0 || g->p.y >= g->clipy1)
				return;
		#endif
		if (!(g->flags & GDISP_FLG_SCRSTREAM))
			setglobalwindow(g);
		gdisp_lld_write_pos(g);
		gdisp_lld_write_color(g);
	}
#else
	// Worst is streaming
	static INLINE void drawpixel_clip(GDisplay *g) {
		#if NEED_CLIPPING
			if (g->p.x < g->clipx0 || g->p.x >= g->clipx1 || g->p.y < g->clipy0 || g->p.y >= g->clipy1)
				return;
		#endif

		g->p.cx = g->p.cy = 1;
		gdisp_lld_write_start(g);
		gdisp_lld_write_color(g);
		gdisp_lld_write_stop(g);
	}
#endif

// fillarea(g)
// Parameters:	x,y cx,cy and color
// Alters:		nothing
// Note:		This is not clipped
// Resets the streaming area
//	if GDISP_HARDWARE_STREAM_WRITE and GDISP_HARDWARE_STREAM_POS is set.
#if GDISP_HARDWARE_FILLS
	// Best is hardware accelerated area fill
	#define fillarea(g)	gdisp_lld_fill_area(g)
#elif GDISP_HARDWARE_STREAM_WRITE
	// Next best is hardware streaming
	static INLINE void fillarea(GDisplay *g) {
		uint32_t	area;

		area = (uint32_t)g->p.cx * g->p.cy;

		#if GDISP_HARDWARE_STREAM_POS
			if ((g->flags & GDISP_FLG_SCRSTREAM)) {
				gdisp_lld_write_stop(g);
				g->flags &= ~GDISP_FLG_SCRSTREAM;
			}
		#endif

		gdisp_lld_write_start(g);
		#if GDISP_HARDWARE_STREAM_POS
			gdisp_lld_write_pos(g);
		#endif
		for(; area; area--)
			gdisp_lld_write_color(g);
		gdisp_lld_write_stop(g);
	}
#else
	// Worst is drawing pixels
	static INLINE void fillarea(GDisplay *g) {
		coord_t x0, y0, x1, y1;

		x0 = g->p.x;
		y0 = g->p.y;
		x1 = g->p.x + g->p.cx;
		y1 = g->p.y + g->p.cy;
		for(; g->p.y < y1; g->p.y++, g->p.x = x0)
			for(; g->p.x < x1; g->p.x++)
				gdisp_lld_draw_pixel(g);
		g->p.y = y0;
	}
#endif

#if NEED_CLIPPING
	#define TEST_CLIP_AREA(g)																					\
				if ((g)->p.x < (g)->clipx0) { (g)->p.cx -= (g)->clipx0 - (g)->p.x; (g)->p.x = (g)->clipx0; }	\
				if ((g)->p.y < (g)->clipy0) { (g)->p.cy -= (g)->clipy0 - (g)->p.y; (g)->p.y = (g)->clipy0; }	\
				if ((g)->p.x + (g)->p.cx > (g)->clipx1)	(g)->p.cx = (g)->clipx1 - (g)->p.x;						\
				if ((g)->p.y + (g)->p.cy > (g)->clipy1)	(g)->p.cy = (g)->clipy1 - (g)->p.y;						\
				if ((g)->p.cx > 0 && (g)->p.cy > 0)
#else
	#define TEST_CLIP_AREA(g)
#endif

// Parameters:	x,y and x1
// Alters:		x,y x1,y1 cx,cy
// Assumes the window covers the screen and a write_stop() will occur later
//	if GDISP_HARDWARE_STREAM_WRITE and GDISP_HARDWARE_STREAM_POS is set.
static void hline_clip(GDisplay *g) {
	// Swap the points if necessary so it always goes from x to x1
	if (g->p.x1 < g->p.x) {
		g->p.cx = g->p.x; g->p.x = g->p.x1; g->p.x1 = g->p.cx;
	}

	// Clipping
	#if NEED_CLIPPING
		if (g->p.y < g->clipy0 || g->p.y >= g->clipy1) return;
		if (g->p.x < g->clipx0) g->p.x = g->clipx0;
		if (g->p.x1 >= g->clipx1) g->p.x1 = g->clipx1 - 1;
		if (g->p.x1 < g->p.x) return;
	#endif

	// This is an optimization for the point case. It is only worthwhile however if we
	// have hardware fills or if we support both hardware pixel drawing and hardware streaming
	#if GDISP_HARDWARE_FILLS || (GDISP_HARDWARE_DRAWPIXEL && GDISP_HARDWARE_STREAM_WRITE)
		// Is this a point
		if (g->p.x == g->p.x1) {
			#if GDISP_HARDWARE_DRAWPIXEL
				// Best is hardware accelerated pixel draw
				gdisp_lld_draw_pixel(g);
			#elif GDISP_HARDWARE_STREAM_POS
				// Next best is cursor based streaming
				if (!(g->flags & GDISP_FLG_SCRSTREAM))
					setglobalwindow(g);
				gdisp_lld_write_pos(g);
				gdisp_lld_write_color(g);
			#else
				// Worst is streaming
				g->p.cx = g->p.cy = 1;
				gdisp_lld_write_start(g);
				gdisp_lld_write_color(g);
				gdisp_lld_write_stop(g);
			#endif
			return;
		}
	#endif

	#if GDISP_HARDWARE_FILLS
		// Best is hardware accelerated area fill
		g->p.cx = g->p.x1 - g->p.x + 1;
		g->p.cy = 1;
		gdisp_lld_fill_area(g);
	#elif GDISP_HARDWARE_STREAM_POS && GDISP_HARDWARE_STREAM_WRITE
		// Next best is cursor based streaming
		if (!(g->flags & GDISP_FLG_SCRSTREAM))
			setglobalwindow(g);
		gdisp_lld_write_pos(g);
		do { gdisp_lld_write_color(g); } while(g->p.cx--);
	#elif GDISP_HARDWARE_STREAM_WRITE
		// Next best is streaming
		g->p.cx = g->p.x1 - g->p.x + 1;
		g->p.cy = 1;
		gdisp_lld_write_start(g);
		do { gdisp_lld_write_color(g); } while(g->p.cx--);
		gdisp_lld_write_stop(g);
	#else
		// Worst is drawing pixels
		for(; g->p.x <= g->p.x1; g->p.x++)
			gdisp_lld_draw_pixel(g);
	#endif
}

// Parameters:	x,y and y1
// Alters:		x,y x1,y1 cx,cy
static void vline_clip(GDisplay *g) {
	// Swap the points if necessary so it always goes from y to y1
	if (g->p.y1 < g->p.y) {
		g->p.cy = g->p.y; g->p.y = g->p.y1; g->p.y1 = g->p.cy;
	}

	// Clipping
	#if NEED_CLIPPING
		if (g->p.x < g->clipx0 || g->p.x >= g->clipx1) return;
		if (g->p.y < g->clipy0) g->p.y = g->clipy0;
		if (g->p.y1 >= g->clipy1) g->p.y1 = g->clipy1 - 1;
		if (g->p.y1 < g->p.y) return;
	#endif

	// This is an optimization for the point case. It is only worthwhile however if we
	// have hardware fills or if we support both hardware pixel drawing and hardware streaming
	#if GDISP_HARDWARE_FILLS || (GDISP_HARDWARE_DRAWPIXEL && GDISP_HARDWARE_STREAM_WRITE) || (GDISP_HARDWARE_STREAM_POS && GDISP_HARDWARE_STREAM_WRITE)
		// Is this a point
		if (g->p.y == g->p.y1) {
			#if GDISP_HARDWARE_DRAWPIXEL
				// Best is hardware accelerated pixel draw
				gdisp_lld_draw_pixel(g);
			#elif GDISP_HARDWARE_STREAM_POS
				// Next best is cursor based streaming
				if (!(g->flags & GDISP_FLG_SCRSTREAM))
					setglobalwindow(g);
				gdisp_lld_write_pos(g);
				gdisp_lld_write_color(g);
			#else
				// Worst is streaming
				g->p.cx = g->p.cy = 1;
				gdisp_lld_write_start(g);
				gdisp_lld_write_color(g);
				gdisp_lld_write_stop(g);
			#endif
			return;
		}
	#endif

	#if GDISP_HARDWARE_FILLS
		// Best is hardware accelerated area fill
		g->p.cy = g->p.y1 - g->p.y + 1;
		g->p.cx = 1;
		gdisp_lld_fill_area(g);
	#elif GDISP_HARDWARE_STREAM_WRITE
		// Next best is streaming
		#if GDISP_HARDWARE_STREAM_POS
			if ((g->flags & GDISP_FLG_SCRSTREAM)) {
				gdisp_lld_write_stop(g);
				g->flags &= ~GDISP_FLG_SCRSTREAM;
			}
		#endif
		g->p.cy = g->p.y1 - g->p.y + 1;
		g->p.cx = 1;
		gdisp_lld_write_start(g);
		#if GDISP_HARDWARE_STREAM_POS
			gdisp_lld_write_pos(g);
		#endif
		do { gdisp_lld_write_color(g); } while(g->p.cy--);
		gdisp_lld_write_stop(g);
	#else
		// Worst is drawing pixels
		for(; g->p.y <= g->p.y1; g->p.y++)
			gdisp_lld_draw_pixel(g);
	#endif
}

// Parameters:	x,y and x1,y1
// Alters:		x,y x1,y1 cx,cy
static void line_clip(GDisplay *g) {
	int16_t dy, dx;
	int16_t addx, addy;
	int16_t P, diff, i;

	// Is this a horizontal line (or a point)
	if (g->p.y == g->p.y1) {
		hline_clip(g);
		return;
	}

	// Is this a vertical line (or a point)
	if (g->p.x == g->p.x1) {
		vline_clip(g);
		return;
	}

	// Not horizontal or vertical

	// Use Bresenham's line drawing algorithm.
	//	This should be replaced with fixed point slope based line drawing
	//	which is more efficient on modern processors as it branches less.
	//	When clipping is needed, all the clipping could also be done up front
	//	instead of on each pixel.

	if (g->p.x1 >= g->p.x) {
		dx = g->p.x1 - g->p.x;
		addx = 1;
	} else {
		dx = g->p.x - g->p.x1;
		addx = -1;
	}
	if (g->p.y1 >= g->p.y) {
		dy = g->p.y1 - g->p.y;
		addy = 1;
	} else {
		dy = g->p.y - g->p.y1;
		addy = -1;
	}

	if (dx >= dy) {
		dy <<= 1;
		P = dy - dx;
		diff = P - dx;

		for(i=0; i<=dx; ++i) {
			drawpixel_clip(g);
			if (P < 0) {
				P  += dy;
				g->p.x += addx;
			} else {
				P  += diff;
				g->p.x += addx;
				g->p.y += addy;
			}
		}
	} else {
		dx <<= 1;
		P = dx - dy;
		diff = P - dy;

		for(i=0; i<=dy; ++i) {
			drawpixel_clip(g);
			if (P < 0) {
				P  += dx;
				g->p.y += addy;
			} else {
				P  += diff;
				g->p.x += addx;
				g->p.y += addy;
			}
		}
	}
}

#if GDISP_STARTUP_LOGO_TIMEOUT > 0
	static void StatupLogoDisplay(GDisplay *g) {
		gdispGClear(g, Black);
		gdispGFillArea(g, g->g.Width/4, g->g.Height/4, g->g.Width/2, g->g.Height/2, Blue);
	}
#endif

/*===========================================================================*/
/* Driver exported functions.                                                */
/*===========================================================================*/

/* Our module initialiser */
void _gdispInit(void) {
	GDisplay		*g;
	unsigned		i;
	#if GDISP_TOTAL_CONTROLLERS > 1
		unsigned				j;
	#endif


	/* Initialise driver */
	#if GDISP_TOTAL_CONTROLLERS > 1
		for(g = GDisplayArray, j=0; j < GDISP_TOTAL_CONTROLLERS; j++)
			for(i = 0; i < DisplayCountList[j]; g++, i++) {
				g->vmt = ControllerList[j];
	#else
		for(g = GDisplayArray, i = 0; i < GDISP_TOTAL_DISPLAYS; g++, i++) {
	#endif
			MUTEX_INIT(g);
			MUTEX_ENTER(g);
			g->flags = 0;
			gdisp_lld_init(g, i);
			#if GDISP_NEED_VALIDATION || GDISP_NEED_CLIP
				#if GDISP_HARDWARE_CLIP
					g->p.x = x;
					g->p.y = y;
					g->p.cx = cx;
					g->p.cy = cy;
					gdisp_lld_set_clip(g);
				#else
					g->clipx0 = 0;
					g->clipy0 = 0;
					g->clipx1 = g->g.Width;
					g->clipy1 = g->g.Height;
				#endif
			#endif
			MUTEX_EXIT(g);
			#if GDISP_STARTUP_LOGO_TIMEOUT > 0
				StatupLogoDisplay(g);
			#else
				gdispGClear(g, Black);
			#endif
	}
	#if GDISP_STARTUP_LOGO_TIMEOUT > 0
		gfxSleepMilliseconds(GDISP_STARTUP_LOGO_TIMEOUT);
		for(g = GDisplayArray, i = 0; i < GDISP_TOTAL_DISPLAYS; g++, i++)
			gdispGClear(g, Black);
	#endif
}

void gdispSetDisplay(unsigned display) {
	if (display < GDISP_TOTAL_DISPLAYS)
		GDISP = &GDisplayArray[display];
}

#if GDISP_NEED_STREAMING
	void gdispGStreamStart(GDisplay *g, coord_t x, coord_t y, coord_t cx, coord_t cy) {
		MUTEX_ENTER(g);

		#if NEED_CLIPPING
			// Test if the area is valid - if not then exit
			if (x < g->clipx0 || x+cx > g->clipx1 || y < g->clipy0 || y+cy > g->clipy1) {
				MUTEX_EXIT(g);
				return;
			}
		#endif

		g->flags |= GDISP_FLG_INSTREAM;

		#if GDISP_HARDWARE_STREAM_WRITE
			// Best is hardware streaming
			g->p.x = x;
			g->p.y = y;
			g->p.cx = cx;
			g->p.cy = cy;
			gdisp_lld_write_start(g);
			#if GDISP_HARDWARE_STREAM_POS
				gdisp_lld_write_pos(g);
			#endif
		#else
			// Worst - save the parameters and use pixel drawing

			// Use x,y as the current position, x1,y1 as the save position and x2,y2 as the end position, cx = bufpos
			g->p.x1 = g->p.x = x;
			g->p.y1 = g->p.y = y;
			g->p.x2 = x + cx;
			g->p.y2 = y + cy;
			#if (GDISP_LINEBUF_SIZE != 0 && GDISP_HARDWARE_BITFILLS) || GDISP_HARDWARE_FILLS
				g->p.cx = 0;
				g->p.cy = 1;
			#endif
		#endif

		// Don't release the mutex as gdispStreamEnd() will do that.
	}

	void gdispGStreamColor(GDisplay *g, color_t color) {
		#if !GDISP_HARDWARE_STREAM_WRITE && GDISP_LINEBUF_SIZE != 0 && GDISP_HARDWARE_BITFILLS
			coord_t	 sx1, sy1;
		#endif

		// Don't touch the mutex as we should already own it

		// Ignore this call if we are not streaming
		if (!(g->flags & GDISP_FLG_INSTREAM))
			return;

		#if GDISP_HARDWARE_STREAM_WRITE
			// Best is hardware streaming
			g->p.color = color;
			gdisp_lld_write_color(g);
		#elif GDISP_LINEBUF_SIZE != 0 && GDISP_HARDWARE_BITFILLS
			g->linebuf[g->p.cx++] = color;
			if (g->p.cx >= GDISP_LINEBUF_SIZE) {
				sx1 = g->p.x1;
				sy1 = g->p.y1;
				g->p.x1 = 0;
				g->p.y1 = 0;
				g->p.ptr = (void *)g->linebuf;
				gdisp_lld_blit_area(g);
				g->p.x1 = sx1;
				g->p.y1 = sy1;
				g->p.x += g->p.cx;
				g->p.cx = 0;
			}

			// Just wrap at end-of-line and end-of-buffer
			if (g->p.x+g->p.cx >= g->p.x2) {
				if (g->p.cx) {
					sx1 = g->p.x1;
					sy1 = g->p.y1;
					g->p.x1 = 0;
					g->p.y1 = 0;
					g->p.ptr = (void *)g->linebuf;
					gdisp_lld_blit_area(g);
					g->p.x1 = sx1;
					g->p.y1 = sy1;
					g->p.cx = 0;
				}
				g->p.x = g->p.x1;
				if (++g->p.y >= g->p.y2)
					g->p.y = g->p.y1;
			}
		#elif GDISP_HARDWARE_FILLS
			// Only slightly better than drawing pixels is to look for runs and use fill area
			if (!g->p.cx || g->p.color == color) {
				g->p.cx++;
				g->p.color = color;
			} else {
				if (g->p.cx == 1)
					gdisp_lld_draw_pixel(g);
				else
					gdisp_lld_fill_area(g);
				g->p.x += g->p.cx;
				g->p.color = color;
				g->p.cx = 1;
			}
			// Just wrap at end-of-line and end-of-buffer
			if (g->p.x+g->p.cx >= g->p.x2) {
				if (g->p.cx) {
					if (g->p.cx == 1)
						gdisp_lld_draw_pixel(g);
					else
						gdisp_lld_fill_area(g);
					g->p.cx = 0;
				}
				g->p.x = g->p.x1;
				if (++g->p.y >= g->p.y2)
					g->p.y = g->p.y1;
			}
		#else
			// Worst is using pixel drawing
			g->p.color = color;
			gdisp_lld_draw_pixel(g);

			// Just wrap at end-of-line and end-of-buffer
			if (++g->p.x >= g->p.x2) {
				g->p.x = g->p.x1;
				if (++g->p.y >= g->p.y2)
					g->p.y = g->p.y1;
			}
		#endif
	}

	void gdispGStreamStop(GDisplay *g) {
		// Only release the mutex and end the stream if we are actually streaming.
		if (!(g->flags & GDISP_FLG_INSTREAM))
			return;

		#if GDISP_HARDWARE_STREAM_WRITE
			gdisp_lld_write_stop(g);
		#elif GDISP_LINEBUF_SIZE != 0 && GDISP_HARDWARE_BITFILLS
			if (g->p.cx) {
				g->p.x1 = 0;
				g->p.y1 = 0;
				g->p.ptr = (void *)g->linebuf;
				gdisp_lld_blit_area(g);
			}
		#elif GDISP_HARDWARE_FILLS
			if (g->p.cx) {
				if (g->p.cx == 1)
					gdisp_lld_draw_pixel(g);
				else
					gdisp_lld_fill_area(g);
			}
		#endif
		g->flags &= ~GDISP_FLG_INSTREAM;
		MUTEX_EXIT(g);
	}
#endif

void gdispGDrawPixel(GDisplay *g, coord_t x, coord_t y, color_t color) {
	MUTEX_ENTER(g);
	g->p.x		= x;
	g->p.y		= y;
	g->p.color	= color;
	drawpixel_clip(g);
	#if GDISP_HARDWARE_STREAM_POS && GDISP_HARDWARE_STREAM_WRITE
		if ((g->flags & GDISP_FLG_SCRSTREAM)) {
			gdisp_lld_write_stop(g);
			g->flags &= ~GDISP_FLG_SCRSTREAM;
		}
	#endif
	MUTEX_EXIT(g);
}
	
void gdispGDrawLine(GDisplay *g, coord_t x0, coord_t y0, coord_t x1, coord_t y1, color_t color) {
	MUTEX_ENTER(g);
	g->p.x = x0;
	g->p.y = y0;
	g->p.x1 = x1;
	g->p.y1 = y1;
	g->p.color = color;
	line_clip(g);
	#if GDISP_HARDWARE_STREAM_POS && GDISP_HARDWARE_STREAM_WRITE
		if ((g->flags & GDISP_FLG_SCRSTREAM)) {
			gdisp_lld_write_stop(g);
			g->flags &= ~GDISP_FLG_SCRSTREAM;
		}
	#endif
	MUTEX_EXIT(g);
}

void gdispGClear(GDisplay *g, color_t color) {
	// Note - clear() ignores the clipping area. It clears the screen.
	MUTEX_ENTER(g);

	#if GDISP_HARDWARE_CLEARS
		// Best is hardware accelerated clear
		g->p.color = color;
		gdisp_lld_clear(g);
	#elif GDISP_HARDWARE_FILLS
		// Next best is hardware accelerated area fill
		g->p.x = g->p.y = 0;
		g->p.cx = g->g.Width;
		g->p.cy = g->g.Height;
		g->p.color = color;
		gdisp_lld_fill_area(g);
	#elif GDISP_HARDWARE_STREAM_WRITE
		// Next best is streaming
		uint32_t	area;

		g->p.x = g->p.y = 0;
		g->p.cx = g->g.Width;
		g->p.cy = g->g.Height;
		g->p.color = color;
		area = (uint32_t)g->p.cx * g->p.cy;

		gdisp_lld_write_start(g);
		#if GDISP_HARDWARE_STREAM_POS
			gdisp_lld_write_pos(g);
		#endif
		for(; area; area--)
			gdisp_lld_write_color(g);
		gdisp_lld_write_stop(g);
	#else
		// Worst is drawing pixels
		g->p.color = color;
		for(g->p.y = 0; g->p.y < g->g.Height; g->p.y++)
			for(g->p.x = 0; g->p.x < g->g.Width; g->p.x++)
				gdisp_lld_draw_pixel(g);
	#endif
	MUTEX_EXIT(g);
}

void gdispGFillArea(GDisplay *g, coord_t x, coord_t y, coord_t cx, coord_t cy, color_t color) {
	MUTEX_ENTER(g);
	g->p.x = x;
	g->p.y = y;
	g->p.cx = cx;
	g->p.cy = cy;
	g->p.color = color;
	TEST_CLIP_AREA(g) {
		fillarea(g);
	}
	MUTEX_EXIT(g);
}
	
void gdispGBlitArea(GDisplay *g, coord_t x, coord_t y, coord_t cx, coord_t cy, coord_t srcx, coord_t srcy, coord_t srccx, const pixel_t *buffer) {
	MUTEX_ENTER(g);

	#if NEED_CLIPPING
		// This is a different cliping to fillarea(g) as it needs to take into account srcx,srcy
		if (x < g->clipx0) { cx -= g->clipx0 - x; srcx += g->clipx0 - x; x = g->clipx0; }
		if (y < g->clipy0) { cy -= g->clipy0 - y; srcy += g->clipy0 - x; y = g->clipy0; }
		if (x+cx > g->clipx1)	cx = g->clipx1 - x;
		if (y+cy > g->clipy1)	cy = g->clipy1 - y;
		if (srcx+cx > srccx) cx = srccx - srcx;
		if (cx <= 0 || cy <= 0) { MUTEX_EXIT(g); return; }
	#endif

	#if GDISP_HARDWARE_BITFILLS
		// Best is hardware bitfills
		g->p.x = x;
		g->p.y = y;
		g->p.cx = cx;
		g->p.cy = cy;
		g->p.x1 = srcx;
		g->p.y1 = srcy;
		g->p.x2 = srccx;
		g->p.ptr = (void *)buffer;
		gdisp_lld_blit_area(g);
	#elif GDISP_HARDWARE_STREAM_WRITE
		// Next best is hardware streaming

		// Translate buffer to the real image data, use srcx,srcy as the end point, srccx as the buffer line gap
		buffer += srcy*srccx+srcx;
		srcx = x + cx;
		srcy = y + cy;
		srccx -= cx;

		g->p.x = x;
		g->p.y = y;
		g->p.cx = cx;
		g->p.cy = cy;
		gdisp_lld_write_start(g);
		#if GDISP_HARDWARE_STREAM_POS
			gdisp_lld_write_pos(g);
		#endif
		for(g->p.y = y; g->p.y < srcy; g->p.y++, buffer += srccx) {
			for(g->p.x = x; g->p.x < srcx; g->p.x++) {
				g->p.color = *buffer++;
				gdisp_lld_write_color(g);
			}
		}
		gdisp_lld_write_stop(g);
	#elif GDISP_HARDWARE_FILLS
		// Only slightly better than drawing pixels is to look for runs and use fill area

		// Translate buffer to the real image data, use srcx,srcy as the end point, srccx as the buffer line gap
		buffer += srcy*srccx+srcx;
		srcx = x + cx;
		srcy = y + cy;
		srccx -= cx;

		g->p.cy = 1;
		for(g->p.y = y; g->p.y < srcy; g->p.y++, buffer += srccx) {
			for(g->p.x=x; g->p.x < srcx; g->p.x += g->p.cx) {
				g->p.cx=1;
				g->p.color = *buffer++;
				while(g->p.x+g->p.cx < srcx && *buffer == g->p.color) {
					g->p.cx++;
					buffer++;
				}
				if (g->p.cx == 1) {
					gdisp_lld_draw_pixel(g);
				} else {
					gdisp_lld_fill_area(g);
				}
			}
		}
	#else
		// Worst is drawing pixels

		// Translate buffer to the real image data, use srcx,srcy as the end point, srccx as the buffer line gap
		buffer += srcy*srccx+srcx;
		srcx = x + cx;
		srcy = y + cy;
		srccx -= cx;

		for(g->p.y = y; g->p.y < srcy; g->p.y++, buffer += srccx) {
			for(g->p.x=x; g->p.x < srcx; g->p.x++) {
				g->p.color = *buffer++;
				gdisp_lld_draw_pixel(g);
			}
		}
	#endif
	MUTEX_EXIT(g);
}
	
#if GDISP_NEED_CLIP
	void gdispGSetClip(GDisplay *g, coord_t x, coord_t y, coord_t cx, coord_t cy) {
		MUTEX_ENTER(g);
		#if GDISP_HARDWARE_CLIP
			// Best is using hardware clipping
			g->p.x = x;
			g->p.y = y;
			g->p.cx = cx;
			g->p.cy = cy;
			gdisp_lld_set_clip(g);
		#else
			// Worst is using software clipping
			if (x < 0) { cx += x; x = 0; }
			if (y < 0) { cy += y; y = 0; }
			if (cx <= 0 || cy <= 0 || x >= g->g.Width || y >= g->g.Height) { MUTEX_EXIT(g); return; }
			g->clipx0 = x;
			g->clipy0 = y;
			g->clipx1 = x+cx;	if (g->clipx1 > g->g.Width) g->clipx1 = g->g.Width;
			g->clipy1 = y+cy;	if (g->clipy1 > g->g.Height) g->clipy1 = g->g.Height;
		#endif
		MUTEX_EXIT(g);
	}
#endif

#if GDISP_NEED_CIRCLE
	void gdispGDrawCircle(GDisplay *g, coord_t x, coord_t y, coord_t radius, color_t color) {
		coord_t a, b, P;

		MUTEX_ENTER(g);

		// Calculate intermediates
		a = 1;
		b = radius;
		P = 4 - radius;
		g->p.color = color;

		// Away we go using Bresenham's circle algorithm
		// Optimized to prevent double drawing
		g->p.x = x; g->p.y = y + b; drawpixel_clip(g);
		g->p.x = x; g->p.y = y - b; drawpixel_clip(g);
		g->p.x = x + b; g->p.y = y; drawpixel_clip(g);
		g->p.x = x - b; g->p.y = y; drawpixel_clip(g);
		do {
			g->p.x = x + a; g->p.y = y + b; drawpixel_clip(g);
			g->p.x = x + a; g->p.y = y - b; drawpixel_clip(g);
			g->p.x = x + b; g->p.y = y + a; drawpixel_clip(g);
			g->p.x = x - b; g->p.y = y + a; drawpixel_clip(g);
			g->p.x = x - a; g->p.y = y + b; drawpixel_clip(g);
			g->p.x = x - a; g->p.y = y - b; drawpixel_clip(g);
			g->p.x = x + b; g->p.y = y - a; drawpixel_clip(g);
			g->p.x = x - b; g->p.y = y - a; drawpixel_clip(g);
			if (P < 0)
				P += 3 + 2*a++;
			else
				P += 5 + 2*(a++ - b--);
		} while(a < b);
		g->p.x = x + a; g->p.y = y + b; drawpixel_clip(g);
		g->p.x = x + a; g->p.y = y - b; drawpixel_clip(g);
		g->p.x = x - a; g->p.y = y + b; drawpixel_clip(g);
		g->p.x = x - a; g->p.y = y - b; drawpixel_clip(g);

		#if GDISP_HARDWARE_STREAM_POS && GDISP_HARDWARE_STREAM_WRITE
			if ((g->flags & GDISP_FLG_SCRSTREAM)) {
				gdisp_lld_write_stop(g);
				g->flags &= ~GDISP_FLG_SCRSTREAM;
			}
		#endif
		MUTEX_EXIT(g);
	}
#endif

#if GDISP_NEED_CIRCLE
	void gdispGFillCircle(GDisplay *g, coord_t x, coord_t y, coord_t radius, color_t color) {
		coord_t a, b, P;

		MUTEX_ENTER(g);

		// Calculate intermediates
		a = 1;
		b = radius;
		P = 4 - radius;
		g->p.color = color;

		// Away we go using Bresenham's circle algorithm
		// This is optimized to prevent overdrawing by drawing a line only when a variable is about to change value
		g->p.y = y; g->p.x = x-b; g->p.x1 = x+b; hline_clip(g);
		g->p.y = y+b; g->p.x = x; drawpixel_clip(g);
		g->p.y = y-b; g->p.x = x; drawpixel_clip(g);
		do {
			g->p.y = y+a; g->p.x = x-b; g->p.x1 = x+b; hline_clip(g);
			g->p.y = y-a; g->p.x = x-b; g->p.x1 = x+b; hline_clip(g);
			if (P < 0) {
				P += 3 + 2*a++;
			} else {
				g->p.y = y+b; g->p.x = x-a; g->p.x1 = x+a; hline_clip(g);
				g->p.y = y-b; g->p.x = x-a; g->p.x1 = x+a; hline_clip(g);
				P += 5 + 2*(a++ - b--);
			}
		} while(a < b);
		g->p.y = y+a; g->p.x = x-b; g->p.x1 = x+b; hline_clip(g);
		g->p.y = y-a; g->p.x = x-b; g->p.x1 = x+b; hline_clip(g);

		#if GDISP_HARDWARE_STREAM_POS && GDISP_HARDWARE_STREAM_WRITE
			if ((g->flags & GDISP_FLG_SCRSTREAM)) {
				gdisp_lld_write_stop(g);
				g->flags &= ~GDISP_FLG_SCRSTREAM;
			}
		#endif
		MUTEX_EXIT(g);
	}
#endif

#if GDISP_NEED_ELLIPSE
	void gdispGDrawEllipse(GDisplay *g, coord_t x, coord_t y, coord_t a, coord_t b, color_t color) {
		coord_t	dx, dy;
		int32_t	a2, b2;
		int32_t	err, e2;

		MUTEX_ENTER(g);

		// Calculate intermediates
		dx = 0;
		dy = b;
		a2 = a*a;
		b2 = b*b;
		err = b2-(2*b-1)*a2;
		g->p.color = color;

		// Away we go using Bresenham's ellipse algorithm
		do {
			g->p.x = x + dx; g->p.y = y + dy; drawpixel_clip(g);
			g->p.x = x - dx; g->p.y = y + dy; drawpixel_clip(g);
			g->p.x = x - dx; g->p.y = y - dy; drawpixel_clip(g);
			g->p.x = x + dx; g->p.y = y - dy; drawpixel_clip(g);

			e2 = 2*err;
			if(e2 <  (2*dx+1)*b2) {
				dx++;
				err += (2*dx+1)*b2;
			}
			if(e2 > -(2*dy-1)*a2) {
				dy--;
				err -= (2*dy-1)*a2;
			}
		} while(dy >= 0);

		#if GDISP_HARDWARE_STREAM_POS && GDISP_HARDWARE_STREAM_WRITE
			if ((g->flags & GDISP_FLG_SCRSTREAM)) {
				gdisp_lld_write_stop(g);
				g->flags &= ~GDISP_FLG_SCRSTREAM;
			}
		#endif
		MUTEX_EXIT(g);
	}
#endif
	
#if GDISP_NEED_ELLIPSE
	void gdispGFillEllipse(GDisplay *g, coord_t x, coord_t y, coord_t a, coord_t b, color_t color) {
		coord_t	dx, dy;
		int32_t	a2, b2;
		int32_t	err, e2;

		MUTEX_ENTER(g);

		// Calculate intermediates
		dx = 0;
		dy = b;
		a2 = a*a;
		b2 = b*b;
		err = b2-(2*b-1)*a2;
		g->p.color = color;

		// Away we go using Bresenham's ellipse algorithm
		// This is optimized to prevent overdrawing by drawing a line only when a y is about to change value
		do {
			e2 = 2*err;
			if(e2 <  (2*dx+1)*b2) {
				dx++;
				err += (2*dx+1)*b2;
			}
			if(e2 > -(2*dy-1)*a2) {
				g->p.y = y + dy; g->p.x = x - dx; g->p.x1 = x + dx; hline_clip(g);
				if (y) { g->p.y = y - dy; g->p.x = x - dx; g->p.x1 = x + dx; hline_clip(g); }
				dy--;
				err -= (2*dy-1)*a2;
			}
		} while(dy >= 0);

		#if GDISP_HARDWARE_STREAM_POS && GDISP_HARDWARE_STREAM_WRITE
			if ((g->flags & GDISP_FLG_SCRSTREAM)) {
				gdisp_lld_write_stop(g);
				g->flags &= ~GDISP_FLG_SCRSTREAM;
			}
		#endif
		MUTEX_EXIT(g);
	}
#endif

#if GDISP_NEED_ARC
	#if !GMISC_NEED_FIXEDTRIG && !GMISC_NEED_FASTTRIG
		#include <math.h>
	#endif

	void gdispGDrawArc(GDisplay *g, coord_t x, coord_t y, coord_t radius, coord_t start, coord_t end, color_t color) {
		coord_t a, b, P, sedge, eedge;
		uint8_t	full, sbit, ebit, tbit;

		// Normalize the angles
		if (start < 0)
			start -= (start/360-1)*360;
		else if (start >= 360)
			start %= 360;
		if (end < 0)
			end -= (end/360-1)*360;
		else if (end >= 360)
			end %= 360;

		sbit = 1<<(start/45);
		ebit = 1<<(end/45);
		full = 0;
		if (start == end) {
			full = 0xFF;
		} else if (end < start) {
			for(tbit=sbit<<1; tbit; tbit<<=1) full |= tbit;
			for(tbit=ebit>>1; tbit; tbit>>=1) full |= tbit;
		} else if (sbit < 0x80) {
			for(tbit=sbit<<1; tbit < ebit; tbit<<=1) full |= tbit;
		}

		MUTEX_ENTER(g);
		g->p.color = color;

		if (full) {
			// Draw full sectors
			// Optimized to prevent double drawing
			a = 1;
			b = radius;
			P = 4 - radius;
			if (full & 0x60) { g->p.y = y+b; g->p.x = x; drawpixel_clip(g); }
			if (full & 0x06) { g->p.y = y-b; g->p.x = x; drawpixel_clip(g); }
			if (full & 0x81) { g->p.y = y; g->p.x = x+b; drawpixel_clip(g); }
			if (full & 0x18) { g->p.y = y; g->p.x = x-b; drawpixel_clip(g); }
			do {
				if (full & 0x01) { g->p.x = x+b; g->p.y = y-a; drawpixel_clip(g); }
				if (full & 0x02) { g->p.x = x+a; g->p.y = y-b; drawpixel_clip(g); }
				if (full & 0x04) { g->p.x = x-a; g->p.y = y-b; drawpixel_clip(g); }
				if (full & 0x08) { g->p.x = x-b; g->p.y = y-a; drawpixel_clip(g); }
				if (full & 0x10) { g->p.x = x-b; g->p.y = y+a; drawpixel_clip(g); }
				if (full & 0x20) { g->p.x = x-a; g->p.y = y+b; drawpixel_clip(g); }
				if (full & 0x40) { g->p.x = x+a; g->p.y = y+b; drawpixel_clip(g); }
				if (full & 0x80) { g->p.x = x+b; g->p.y = y+a; drawpixel_clip(g); }
				if (P < 0)
					P += 3 + 2*a++;
				else
					P += 5 + 2*(a++ - b--);
			} while(a < b);
			if (full & 0xC0) { g->p.x = x+a; g->p.y = y+b; drawpixel_clip(g); }
			if (full & 0x0C) { g->p.x = x-a; g->p.y = y-b; drawpixel_clip(g); }
			if (full & 0x03) { g->p.x = x+a; g->p.y = y-b; drawpixel_clip(g); }
			if (full & 0x30) { g->p.x = x-a; g->p.y = y+b; drawpixel_clip(g); }
			if (full == 0xFF) {
				#if GDISP_HARDWARE_STREAM_POS && GDISP_HARDWARE_STREAM_WRITE
					if ((g->flags & GDISP_FLG_SCRSTREAM)) {
						gdisp_lld_write_stop(g);
						g->flags &= ~GDISP_FLG_SCRSTREAM;
					}
				#endif
				MUTEX_EXIT;
				return;
			}
		}

		#if GFX_USE_GMISC && GMISC_NEED_FIXEDTRIG
			sedge = NONFIXED(radius * ((sbit & 0x99) ? ffsin(start) : ffcos(start)) + FIXED0_5);
			eedge = NONFIXED(radius * ((ebit & 0x99) ? ffsin(end) : ffcos(end)) + FIXED0_5);
		#elif GFX_USE_GMISC && GMISC_NEED_FASTTRIG
			sedge = round(radius * ((sbit & 0x99) ? fsin(start) : fcos(start)));
			eedge = round(radius * ((ebit & 0x99) ? fsin(end) : fcos(end)));
		#else
			sedge = round(radius * ((sbit & 0x99) ? sin(start*M_PI/180) : cos(start*M_PI/180)));
			eedge = round(radius * ((ebit & 0x99) ? sin(end*M_PI/180) : cos(end*M_PI/180)));
		#endif
		if (sbit & 0xB4) sedge = -sedge;
		if (ebit & 0xB4) eedge = -eedge;

		if (sbit != ebit) {
			// Draw start and end sectors
			// Optimized to prevent double drawing
			a = 1;
			b = radius;
			P = 4 - radius;
			if ((sbit & 0x20) || (ebit & 0x40)) { g->p.x = x; g->p.y = y+b; drawpixel_clip(g); }
			if ((sbit & 0x02) || (ebit & 0x04)) { g->p.x = x; g->p.y = y-b; drawpixel_clip(g); }
			if ((sbit & 0x80) || (ebit & 0x01)) { g->p.x = x+b; g->p.y = y; drawpixel_clip(g); }
			if ((sbit & 0x08) || (ebit & 0x10)) { g->p.x = x-b; g->p.y = y; drawpixel_clip(g); }
			do {
				if (((sbit & 0x01) && a >= sedge) || ((ebit & 0x01) && a <= eedge)) { g->p.x = x+b; g->p.y = y-a; drawpixel_clip(g); }
				if (((sbit & 0x02) && a <= sedge) || ((ebit & 0x02) && a >= eedge)) { g->p.x = x+a; g->p.y = y-b; drawpixel_clip(g); }
				if (((sbit & 0x04) && a >= sedge) || ((ebit & 0x04) && a <= eedge)) { g->p.x = x-a; g->p.y = y-b; drawpixel_clip(g); }
				if (((sbit & 0x08) && a <= sedge) || ((ebit & 0x08) && a >= eedge)) { g->p.x = x-b; g->p.y = y-a; drawpixel_clip(g); }
				if (((sbit & 0x10) && a >= sedge) || ((ebit & 0x10) && a <= eedge)) { g->p.x = x-b; g->p.y = y+a; drawpixel_clip(g); }
				if (((sbit & 0x20) && a <= sedge) || ((ebit & 0x20) && a >= eedge)) { g->p.x = x-a; g->p.y = y+b; drawpixel_clip(g); }
				if (((sbit & 0x40) && a >= sedge) || ((ebit & 0x40) && a <= eedge)) { g->p.x = x+a; g->p.y = y+b; drawpixel_clip(g); }
				if (((sbit & 0x80) && a <= sedge) || ((ebit & 0x80) && a >= eedge)) { g->p.x = x+b; g->p.y = y+a; drawpixel_clip(g); }
				if (P < 0)
					P += 3 + 2*a++;
				else
					P += 5 + 2*(a++ - b--);
			} while(a < b);
			if (((sbit & 0x40) && a >= sedge) || ((ebit & 0x40) && a <= eedge) || ((sbit & 0x80) && a <= sedge) || ((ebit & 0x80) && a >= eedge))
				{ g->p.x = x+a; g->p.y = y+b; drawpixel_clip(g); }
			if (((sbit & 0x04) && a >= sedge) || ((ebit & 0x04) && a <= eedge) || ((sbit & 0x08) && a <= sedge) || ((ebit & 0x08) && a >= eedge))
				{ g->p.x = x-a; g->p.y = y-b; drawpixel_clip(g); }
			if (((sbit & 0x01) && a >= sedge) || ((ebit & 0x01) && a <= eedge) || ((sbit & 0x02) && a <= sedge) || ((ebit & 0x02) && a >= eedge))
				{ g->p.x = x+a; g->p.y = y-b; drawpixel_clip(g); }
			if (((sbit & 0x10) && a >= sedge) || ((ebit & 0x10) && a <= eedge) || ((sbit & 0x20) && a <= sedge) || ((ebit & 0x20) && a >= eedge))
				{ g->p.x = x-a; g->p.y = y+b; drawpixel_clip(g); }
		} else if (end < start) {
			// Draw start/end sector where it is a non-internal angle
			// Optimized to prevent double drawing
			a = 1;
			b = radius;
			P = 4 - radius;
			if (sbit & 0x60) { g->p.x = x; g->p.y = y+b; drawpixel_clip(g); }
			if (sbit & 0x06) { g->p.x = x; g->p.y = y-b; drawpixel_clip(g); }
			if (sbit & 0x81) { g->p.x = x+b; g->p.y = y; drawpixel_clip(g); }
			if (sbit & 0x18) { g->p.x = x-b; g->p.y = y; drawpixel_clip(g); }
			do {
				if ((sbit & 0x01) && (a >= sedge || a <= eedge)) { g->p.x = x+b; g->p.y = y-a; drawpixel_clip(g); }
				if ((sbit & 0x02) && (a <= sedge || a >= eedge)) { g->p.x = x+a; g->p.y = y-b; drawpixel_clip(g); }
				if ((sbit & 0x04) && (a >= sedge || a <= eedge)) { g->p.x = x-a; g->p.y = y-b; drawpixel_clip(g); }
				if ((sbit & 0x08) && (a <= sedge || a >= eedge)) { g->p.x = x-b; g->p.y = y-a; drawpixel_clip(g); }
				if ((sbit & 0x10) && (a >= sedge || a <= eedge)) { g->p.x = x-b; g->p.y = y+a; drawpixel_clip(g); }
				if ((sbit & 0x20) && (a <= sedge || a >= eedge)) { g->p.x = x-a; g->p.y = y+b; drawpixel_clip(g); }
				if ((sbit & 0x40) && (a >= sedge || a <= eedge)) { g->p.x = x+a; g->p.y = y+b; drawpixel_clip(g); }
				if ((sbit & 0x80) && (a <= sedge || a >= eedge)) { g->p.x = x+b; g->p.y = y+a; drawpixel_clip(g); }
				if (P < 0)
					P += 3 + 2*a++;
				else
					P += 5 + 2*(a++ - b--);
			} while(a < b);
			if (((sbit & 0x04) && (a >= sedge || a <= eedge)) || ((sbit & 0x08) && (a <= sedge || a >= eedge)))
				{ g->p.x = x-a; g->p.y = y-b; drawpixel_clip(g); }
			if (((sbit & 0x40) && (a >= sedge || a <= eedge)) || ((sbit & 0x80) && (a <= sedge || a >= eedge)))
				{ g->p.x = x+a; g->p.y = y+b; drawpixel_clip(g); }
			if (((sbit & 0x01) && (a >= sedge || a <= eedge)) || ((sbit & 0x02) && (a <= sedge || a >= eedge)))
				{ g->p.x = x+a; g->p.y = y-b; drawpixel_clip(g); }
			if (((sbit & 0x10) && (a >= sedge || a <= eedge)) || ((sbit & 0x20) && (a <= sedge || a >= eedge)))
				{ g->p.x = x-a; g->p.y = y+b; drawpixel_clip(g); }
		} else {
			// Draw start/end sector where it is a internal angle
			// Optimized to prevent double drawing
			a = 1;
			b = radius;
			P = 4 - radius;
			if (((sbit & 0x20) && !eedge) || ((sbit & 0x40) && !sedge)) { g->p.x = x; g->p.y = y+b; drawpixel_clip(g); }
			if (((sbit & 0x02) && !eedge) || ((sbit & 0x04) && !sedge)) { g->p.x = x; g->p.y = y-b; drawpixel_clip(g); }
			if (((sbit & 0x80) && !eedge) || ((sbit & 0x01) && !sedge)) { g->p.x = x+b; g->p.y = y; drawpixel_clip(g); }
			if (((sbit & 0x08) && !eedge) || ((sbit & 0x10) && !sedge)) { g->p.x = x-b; g->p.y = y; drawpixel_clip(g); }
			do {
				if (((sbit & 0x01) && a >= sedge && a <= eedge)) { g->p.x = x+b; g->p.y = y-a; drawpixel_clip(g); }
				if (((sbit & 0x02) && a <= sedge && a >= eedge)) { g->p.x = x+a; g->p.y = y-b; drawpixel_clip(g); }
				if (((sbit & 0x04) && a >= sedge && a <= eedge)) { g->p.x = x-a; g->p.y = y-b; drawpixel_clip(g); }
				if (((sbit & 0x08) && a <= sedge && a >= eedge)) { g->p.x = x-b; g->p.y = y-a; drawpixel_clip(g); }
				if (((sbit & 0x10) && a >= sedge && a <= eedge)) { g->p.x = x-b; g->p.y = y+a; drawpixel_clip(g); }
				if (((sbit & 0x20) && a <= sedge && a >= eedge)) { g->p.x = x-a; g->p.y = y+b; drawpixel_clip(g); }
				if (((sbit & 0x40) && a >= sedge && a <= eedge)) { g->p.x = x+a; g->p.y = y+b; drawpixel_clip(g); }
				if (((sbit & 0x80) && a <= sedge && a >= eedge)) { g->p.x = x+b; g->p.y = y+a; drawpixel_clip(g); }
				if (P < 0)
					P += 3 + 2*a++;
				else
					P += 5 + 2*(a++ - b--);
			} while(a < b);
			if (((sbit & 0x04) && a >= sedge && a <= eedge) || ((sbit & 0x08) && a <= sedge && a >= eedge))
				{ g->p.x = x-a; g->p.y = y-b; drawpixel_clip(g); }
			if (((sbit & 0x40) && a >= sedge && a <= eedge) || ((sbit & 0x80) && a <= sedge && a >= eedge))
				{ g->p.x = x+a; g->p.y = y+b; drawpixel_clip(g); }
			if (((sbit & 0x01) && a >= sedge && a <= eedge) || ((sbit & 0x02) && a <= sedge && a >= eedge))
				{ g->p.x = x+a; g->p.y = y-b; drawpixel_clip(g); }
			if (((sbit & 0x10) && a >= sedge && a <= eedge) || ((sbit & 0x20) && a <= sedge && a >= eedge))
				{ g->p.x = x-a; g->p.y = y+b; drawpixel_clip(g); }
		}

		#if GDISP_HARDWARE_STREAM_POS && GDISP_HARDWARE_STREAM_WRITE
			if ((g->flags & GDISP_FLG_SCRSTREAM)) {
				gdisp_lld_write_stop(g);
				g->flags &= ~GDISP_FLG_SCRSTREAM;
			}
		#endif
		MUTEX_EXIT(g);
	}
#endif

#if GDISP_NEED_ARC
	void gdispGFillArc(GDisplay *g, coord_t x, coord_t y, coord_t radius, coord_t start, coord_t end, color_t color) {
		coord_t a, b, P;
		coord_t	sy, ey;
		fixed	sxa, sxb, sxd, exa, exb, exd;
		uint8_t	qtr;

		MUTEX_ENTER(g);

		// Do the trig to get the formulas for the start and end lines.
		sxa = exa = FIXED(x)+FIXED0_5;
		#if GFX_USE_GMISC && GMISC_NEED_FIXEDTRIG
			sxb = radius*ffcos(start);	sy = -NONFIXED(radius*ffsin(start) + FIXED0_5);
			exb = radius*ffcos(end);	ey = -NONFIXED(radius*ffsin(end) + FIXED0_5);
		#elif GFX_USE_GMISC && GMISC_NEED_FASTTRIG
			sxb = FP2FIXED(radius*fcos(start));	sy = -round(radius*fsin(start));
			exb = FP2FIXED(radius*fcos(end));	ey = -round(radius*fsin(end));
		#else
			sxb = FP2FIXED(radius*cos(start*M_PI/180));	sy = -round(radius*sin(start*M_PI/180));
			exb = FP2FIXED(radius*cos(end*M_PI/180));	ey = -round(radius*sin(end*M_PI/180));
		#endif
		sxd = sy ? sxb/sy : sxb;
		exd = ey ? exb/ey : exb;

		// Calculate which quarters and which direction we are traveling
		qtr = 0;
		if (sxb > 0)	qtr |= 0x01;		// S1=0001(1), S2=0000(0), S3=0010(2), S4=0011(3)
		if (sy > 0) 	qtr |= 0x02;
		if (exb > 0)	qtr |= 0x04;		// E1=0100(4), E2=0000(0), E3=1000(8), E4=1100(12)
		if (ey > 0) 	qtr |= 0x08;
		if (sy > ey)	qtr |= 0x10;		// order of start and end lines

		// Calculate intermediates
		a = 1;
		b = radius;
		P = 4 - radius;
		g->p.color = color;
		sxb += sxa;
		exb += exa;

		// Away we go using Bresenham's circle algorithm
		// This is optimized to prevent overdrawing by drawing a line only when a variable is about to change value

		switch(qtr) {
		case 0:		// S2E2 sy <= ey
		case 1:		// S1E2 sy <= ey
			if (ey && sy) {
				g->p.x = x; g->p.x1 = x;									// E2S
				sxa -= sxd; exa -= exd;
			} else if (sy) {
				g->p.x = x-b; g->p.x1 = x;								// C2S
				sxa -= sxd;
			} else if (ey) {
				g->p.x = x; g->p.x1 = x+b;								// E2C
				exa -= exd;
			} else {
				g->p.x = x-b; g->p.x1 = x+b;								// C2C
			}
			g->p.y = y;
			hline_clip(g);
			do {
				if (-a >= ey) {
					g->p.y = y-a; g->p.x = NONFIXED(exa); g->p.x1 = NONFIXED(sxa); hline_clip(g);		// E2S
					sxa -= sxd; exa -= exd;
				} else if (-a >= sy) {
					g->p.y = y-a; g->p.x = x-b; g->p.x1 = NONFIXED(sxa); hline_clip(g);				// C2S
					sxa -= sxd;
				} else if (qtr & 1) {
					g->p.y = y-a; g->p.x = x-b; g->p.x1 = x+b; hline_clip(g);							// C2C
				}
				if (P < 0) {
					P += 3 + 2*a++;
				} else {
					if (-b >= ey) {
						g->p.y = y-b; g->p.x = NONFIXED(exb); g->p.x1 = NONFIXED(sxb); hline_clip(g);	// E2S
						sxb += sxd; exb += exd;
					} else if (-b >= sy) {
						g->p.y = y-b; g->p.x = x-a; g->p.x1 = NONFIXED(sxb); hline_clip(g);			// C2S
						sxb += sxd;
					} else if (qtr & 1) {
						g->p.y = y-b; g->p.x = x-a; g->p.x1 = x+a; hline_clip(g);						// C2C
					}
					P += 5 + 2*(a++ - b--);
				}
			} while(a < b);
			if (-a >= ey) {
				g->p.y = y-a; g->p.x = NONFIXED(exa); g->p.x1 = NONFIXED(sxa); hline_clip(g);			// E2S
			} else if (-a >= sy) {
				g->p.y = y-a; g->p.x = x-b; g->p.x1 = NONFIXED(sxa); hline_clip(g);					// C2S
			} else if (qtr & 1) {
				g->p.y = y-a; g->p.x = x-b; g->p.x1 = x+b; hline_clip(g);								// C2C
			}
			break;

		case 2:		// S3E2 sy <= ey
		case 3:		// S4E2 sy <= ey
		case 6:		// S3E1 sy <= ey
		case 7:		// S4E1 sy <= ey
		case 18:	// S3E2 sy > ey
		case 19:	// S4E2 sy > ey
		case 22:	// S3E1 sy > ey
		case 23:	// S4E1 sy > ey
			g->p.y = y; g->p.x = x; g->p.x1 = x+b; hline_clip(g);								// SE2C
			sxa += sxd; exa -= exd;
			do {
				if (-a >= ey) {
					g->p.y = y-a; g->p.x = NONFIXED(exa); g->p.x1 = x+b; hline_clip(g);		// E2C
					exa -= exd;
				} else if (!(qtr & 4)) {
					g->p.y = y-a; g->p.x = x-b; g->p.x1 = x+b; hline_clip(g);					// C2C
				}
				if (a <= sy) {
					g->p.y = y+a; g->p.x = NONFIXED(sxa); g->p.x1 = x+b; hline_clip(g);		// S2C
					sxa += sxd;
				} else if (!(qtr & 1)) {
					g->p.y = y+a; g->p.x = x-b; g->p.x1 = x+b; hline_clip(g);					// C2C
				}
				if (P < 0) {
					P += 3 + 2*a++;
				} else {
					if (-b >= ey) {
						g->p.y = y-b; g->p.x = NONFIXED(exb); g->p.x1 = x+a; hline_clip(g);		// E2C
						exb += exd;
					} else if (!(qtr & 4)) {
						g->p.y = y-b; g->p.x = x-a; g->p.x1 = x+a; hline_clip(g);					// C2C
					}
					if (b <= sy) {
						g->p.y = y+b; g->p.x = NONFIXED(sxb); g->p.x1 = x+a; hline_clip(g);		// S2C
						sxb -= sxd;
					} else if (!(qtr & 1)) {
						g->p.y = y+b; g->p.x = x-a; g->p.x1 = x+a; hline_clip(g); 				// C2C
					}
					P += 5 + 2*(a++ - b--);
				}
			} while(a < b);
			if (-a >= ey) {
				g->p.y = y-a; g->p.x = NONFIXED(exa); g->p.x1 = x+b; hline_clip(g);				// E2C
			} else if (!(qtr & 4)) {
				g->p.y = y-a; g->p.x = x-b; g->p.x1 = x+b; hline_clip(g);							// C2C
			}
			if (a <= sy) {
				g->p.y = y+a; g->p.x = NONFIXED(sxa); g->p.x1 = x+a; hline_clip(g);				// S2C
			} else if (!(qtr & 1)) {
				g->p.y = y+a; g->p.x = x-b; g->p.x1 = x+a; hline_clip(g);							// C2C
			}
			break;

		case 4:		// S2E1 sy <= ey
		case 5:		// S1E1 sy <= ey
			g->p.y = y; g->p.x = x-b; g->p.x1 = x+b; hline_clip(g);									// C2C
			do {
				if (-a >= ey) {
					g->p.y = y-a; g->p.x = x-b; g->p.x1 = NONFIXED(sxa); hline_clip(g);				// C2S
					g->p.y = y-a; g->p.x = NONFIXED(exa); g->p.x1 = x+b; hline_clip(g);				// E2C
					sxa -= sxd; exa -= exd;
				} else if (-a >= sy) {
					g->p.y = y-a; g->p.x = x-b; g->p.x1 = NONFIXED(sxa); hline_clip(g);				// C2S
					sxa -= sxd;
				} else if (qtr & 1) {
					g->p.y = y-a; g->p.x = x-b; g->p.x1 = x+b; hline_clip(g);							// C2C
				}
				g->p.y = y+a; g->p.x = x-b; g->p.x1 = x+b; hline_clip(g);								// C2C
				if (P < 0) {
					P += 3 + 2*a++;
				} else {
					if (-b >= ey) {
						g->p.y = y-b; g->p.x = x-a; g->p.x1 = NONFIXED(sxb); hline_clip(g);			// C2S
						g->p.y = y-b; g->p.x = NONFIXED(exb); g->p.x1 = x+a; hline_clip(g);			// E2C
						sxb += sxd; exb += exd;
					} else if (-b >= sy) {
						g->p.y = y-b; g->p.x = x-a; g->p.x1 = NONFIXED(sxb); hline_clip(g);			// C2S
						sxb += sxd;
					} else if (qtr & 1) {
						g->p.y = y-b; g->p.x = x-a; g->p.x1 = x+a; hline_clip(g);						// C2C
					}
					g->p.y = y+b; g->p.x = x-a; g->p.x1 = x+a; hline_clip(g);							// C2C
					P += 5 + 2*(a++ - b--);
				}
			} while(a < b);
			if (-a >= ey) {
				g->p.y = y-a; g->p.x = x-b; g->p.x1 = NONFIXED(sxa); hline_clip(g);					// C2S
				g->p.y = y-a; g->p.x = NONFIXED(exa); g->p.x1 = x+b; hline_clip(g);					// E2C
			} else if (-a >= sy) {
				g->p.y = y-a; g->p.x = x-b; g->p.x1 = NONFIXED(sxa); hline_clip(g);					// C2S
			} else if (qtr & 1) {
				g->p.y = y-a; g->p.x = x-b; g->p.x1 = x+b; hline_clip(g);								// C2C
			}
			g->p.y = y+b; g->p.x = x-a; g->p.x1 = x+a; hline_clip(g);									// C2C
			break;

		case 8:		// S2E3 sy <= ey
		case 9:		// S1E3 sy <= ey
		case 12:	// S2E4 sy <= ey
		case 13:	// S1E4 sy <= ey
		case 24:	// S2E3 sy > ey
		case 25:	// S1E3 sy > ey
		case 28:	// S2E3 sy > ey
		case 29:	// S1E3 sy > ey
			g->p.y = y; g->p.x = x-b; g->p.x1 = x; hline_clip(g);								// C2SE
			sxa -= sxd; exa += exd;
			do {
				if (-a >= sy) {
					g->p.y = y-a; g->p.x = x-b; g->p.x1 = NONFIXED(sxa); hline_clip(g);		// C2S
					sxa -= sxd;
				} else if (qtr & 1) {
					g->p.y = y-a; g->p.x = x-b; g->p.x1 = x+b; hline_clip(g);					// C2C
				}
				if (a <= ey) {
					g->p.y = y+a; g->p.x = x-b; g->p.x1 = NONFIXED(exa); hline_clip(g);		// C2E
					exa += exd;
				} else if (qtr & 4) {
					g->p.y = y+a; g->p.x = x-b; g->p.x1 = x+b; hline_clip(g);					// C2C
				}
				if (P < 0) {
					P += 3 + 2*a++;
				} else {
					if (-b >= sy) {
						g->p.y = y-b; g->p.x = x-a; g->p.x1 = NONFIXED(sxb); hline_clip(g);		// C2S
						sxb += sxd;
					} else if (qtr & 1) {
						g->p.y = y-b; g->p.x = x-a; g->p.x1 = x+a; hline_clip(g);					// C2C
					}
					if (b <= ey) {
						g->p.y = y+b; g->p.x = x-a; g->p.x1 = NONFIXED(exb); hline_clip(g);		// C2E
						exb -= exd;
					} else if (qtr & 4) {
						g->p.y = y+b; g->p.x = x-a; g->p.x1 = x+a; hline_clip(g); 				// C2C
					}
					P += 5 + 2*(a++ - b--);
				}
			} while(a < b);
			if (-a >= sy) {
				g->p.y = y-a; g->p.x = x-b; g->p.x1 = NONFIXED(sxa); hline_clip(g);				// C2S
			} else if (qtr & 1) {
				g->p.y = y-a; g->p.x = x-b; g->p.x1 = x+b; hline_clip(g);							// C2C
			}
			if (a <= ey) {
				g->p.y = y+a; g->p.x = x-b; g->p.x1 = NONFIXED(exa); hline_clip(g);				// C2E
			} else if (qtr & 4) {
				g->p.y = y+a; g->p.x = x-b; g->p.x1 = x+a; hline_clip(g);							// C2C
			}
			break;

		case 10:	// S3E3 sy <= ey
		case 14:	// S3E4 sy <= ey
			g->p.y = y; g->p.x = x; drawpixel_clip(g);													// S2E
			sxa += sxd; exa += exd;
			do {
				if (a <= sy) {
					g->p.y = y+a; g->p.x = NONFIXED(sxa); g->p.x1 = NONFIXED(exa); hline_clip(g);		// S2E
					sxa += sxd; exa += exd;
				} else if (a <= ey) {
					g->p.y = y+a; g->p.x = x-b; g->p.x1 = NONFIXED(exa); hline_clip(g);				// C2E
					exa += exd;
				} else if (qtr & 4) {
					g->p.y = y+a; g->p.x = x-b; g->p.x1 = x+b; hline_clip(g);							// C2C
				}
				if (P < 0) {
					P += 3 + 2*a++;
				} else {
					if (b <= sy) {
						g->p.y = y+b; g->p.x = NONFIXED(sxb); g->p.x1 = NONFIXED(exb); hline_clip(g);		// S2E
						sxb -= sxd; exb -= exd;
					} else if (b <= ey) {
						g->p.y = y+b; g->p.x = x-a; g->p.x1 = NONFIXED(exb); hline_clip(g);				// C2E
						exb -= exd;
					} else if (qtr & 4) {
						g->p.y = y+b; g->p.x = x-a; g->p.x1 = x+a; hline_clip(g);							// C2C
					}
					P += 5 + 2*(a++ - b--);
				}
			} while(a < b);
			if (a <= sy) {
				g->p.y = y+a; g->p.x = NONFIXED(sxa); g->p.x1 = NONFIXED(exa); hline_clip(g);		// S2E
			} else if (a <= ey) {
				g->p.y = y+a; g->p.x = x-b; g->p.x1 = NONFIXED(exa); hline_clip(g);				// C2E
			} else if (qtr & 4) {
				g->p.y = y+a; g->p.x = x-b; g->p.x1 = x+b; hline_clip(g);							// C2C
			}
			break;

		case 11:	// S4E3 sy <= ey
		case 15:	// S4E4 sy <= ey
			g->p.y = y; g->p.x = x-b; g->p.x1 = x+b; hline_clip(g);									// C2C
			do {
				g->p.y = y-a; g->p.x = x-b; g->p.x1 = x+b; hline_clip(g);								// C2C
				if (a <= sy) {
					g->p.y = y+a; g->p.x = x-b; g->p.x1 = NONFIXED(exa); hline_clip(g);				// C2E
					g->p.y = y+a; g->p.x = NONFIXED(sxa); g->p.x1 = x+b; hline_clip(g);				// S2C
					sxa += sxd; exa += exd;
				} else if (a <= ey) {
					g->p.y = y+a; g->p.x = x-b; g->p.x1 = NONFIXED(exa); hline_clip(g);				// C2E
					exa += exd;
				} else if (qtr & 4) {
					g->p.y = y+a; g->p.x = x-b; g->p.x1 = x+b; hline_clip(g);							// C2C
				}
				if (P < 0) {
					P += 3 + 2*a++;
				} else {
					g->p.y = y-b; g->p.x = x-a; g->p.x1 = x+a; hline_clip(g);							// C2C
					if (b <= sy) {
						g->p.y = y+b; g->p.x = x-a; g->p.x1 = NONFIXED(exb); hline_clip(g);			// C2E
						g->p.y = y+b; g->p.x = NONFIXED(sxb); g->p.x1 = x+a; hline_clip(g);			// S2C
						sxb -= sxd; exb -= exd;
					} else if (b <= ey) {
						g->p.y = y+b; g->p.x = x-a; g->p.x1 = NONFIXED(exb); hline_clip(g);			// C2E
						exb -= exd;
					} else if (qtr & 4) {
						g->p.y = y+b; g->p.x = x-a; g->p.x1 = x+a; hline_clip(g);						// C2C
					}
					P += 5 + 2*(a++ - b--);
				}
			} while(a < b);
			g->p.y = y-a; g->p.x = x-b; g->p.x1 = x+b; hline_clip(g);									// C2C
			if (a <= sy) {
				g->p.y = y+a; g->p.x = x-b; g->p.x1 = NONFIXED(exa); hline_clip(g);					// C2E
				g->p.y = y+a; g->p.x = NONFIXED(sxa); g->p.x1 = x+b; hline_clip(g);					// S2C
			} else if (a <= ey) {
				g->p.y = y+a; g->p.x = x-b; g->p.x1 = NONFIXED(exa); hline_clip(g);					// C2E
			} else if (qtr & 4) {
				g->p.y = y+a; g->p.x = x-b; g->p.x1 = x+b; hline_clip(g);								// C2C
			}
			break;

		case 16:	// S2E2	sy > ey
		case 20:	// S2E1 sy > ey
			g->p.y = y; g->p.x = x-b; g->p.x1 = x+b; hline_clip(g);									// C2C
			sxa -= sxd; exa -= exd;
			do {
				if (-a >= sy) {
					g->p.y = y-a; g->p.x = x-b; g->p.x1 = NONFIXED(sxa); hline_clip(g);				// C2S
					g->p.y = y-a; g->p.x = NONFIXED(exa); g->p.x1 = x+b; hline_clip(g);				// E2C
					sxa -= sxd; exa -= exd;
				} else if (-a >= ey) {
					g->p.y = y-a; g->p.x = NONFIXED(exa); g->p.x1 = x+b; hline_clip(g);				// E2C
					exa -= exd;
				} else if (!(qtr & 4)){
					g->p.y = y-a; g->p.x = x-b; g->p.x1 = x+b; hline_clip(g); 						// C2C
				}
				g->p.y = y+a; g->p.x = x-b; g->p.x1 = x+b; hline_clip(g); 							// C2C
				if (P < 0) {
					P += 3 + 2*a++;
				} else {
					if (-b >= sy) {
						g->p.y = y-b; g->p.x = x-a; g->p.x1 = NONFIXED(sxb); hline_clip(g);			// C2S
						g->p.y = y-b; g->p.x = NONFIXED(exb); g->p.x1 = x+a; hline_clip(g);			// E2C
						sxb += sxd; exb += exd;
					} else if (-b >= ey) {
						g->p.y = y-b; g->p.x = NONFIXED(exb); g->p.x1 = x+a; hline_clip(g);			// E2C
						exb += exd;
					} else if (!(qtr & 4)){
						g->p.y = y-b; g->p.x = x-a; g->p.x1 = x+a; hline_clip(g); 					// C2C
					}
					g->p.y = y+b; g->p.x = x-a; g->p.x1 = x+a; hline_clip(g); 						// C2C
					P += 5 + 2*(a++ - b--);
				}
			} while(a < b);
			if (-a >= sy) {
				g->p.y = y-a; g->p.x = x-b; g->p.x1 = NONFIXED(sxa); hline_clip(g);					// C2S
				g->p.y = y-a; g->p.x = NONFIXED(exa); g->p.x1 = x+b; hline_clip(g);					// E2C
			} else if (-a >= ey) {
				g->p.y = y-a; g->p.x = NONFIXED(exa); g->p.x1 = x+b; hline_clip(g);					// E2C
			} else if (!(qtr & 4)){
				g->p.y = y-a; g->p.x = x-b; g->p.x1 = x+b; hline_clip(g); 							// C2C
			}
			g->p.y = y+a; g->p.x = x-b; g->p.x1 = x+b; hline_clip(g); 								// C2C
			break;

		case 17:	// S1E2 sy > ey
		case 21:	// S1E1 sy > ey
			if (sy) {
				g->p.x = x; g->p.x1 = x;																// E2S
				sxa -= sxd; exa -= exd;
			} else {
				g->p.x = x; g->p.x1 = x+b;															// E2C
				exa -= exd;
			}
			g->p.y = y;
			hline_clip(g);
			do {
				if (-a >= sy) {
					g->p.y = y-a; g->p.x = NONFIXED(exa); g->p.x1 = NONFIXED(sxa); hline_clip(g);		// E2S
					sxa -= sxd; exa -= exd;
				} else if (-a >= ey) {
					g->p.y = y-a; g->p.x = NONFIXED(exa); g->p.x1 = x+b; hline_clip(g);				// E2C
					exa -= exd;
				} else if (!(qtr & 4)) {
					g->p.y = y-a; g->p.x = x-b; g->p.x1 = x+b; hline_clip(g);							// C2C
				}
				if (P < 0) {
					P += 3 + 2*a++;
				} else {
					if (-b >= sy) {
						g->p.y = y-b; g->p.x = NONFIXED(exb); g->p.x1 = NONFIXED(sxb); hline_clip(g);	// E2S
						sxb += sxd; exb += exd;
					} else if (-b >= ey) {
						g->p.y = y-b; g->p.x = NONFIXED(exb); g->p.x1 = x+a; hline_clip(g);			// E2C
						exb += exd;
					} else if (!(qtr & 4)) {
						g->p.y = y-b; g->p.x = x-a; g->p.x1 = x+a; hline_clip(g);						// C2C
					}
					P += 5 + 2*(a++ - b--);
				}
			} while(a < b);
			if (-a >= sy) {
				g->p.y = y-a; g->p.x = NONFIXED(exa); g->p.x1 = NONFIXED(sxa); hline_clip(g);			// E2S
			} else if (-a >= ey) {
				g->p.y = y-a; g->p.x = NONFIXED(exa); g->p.x1 = x+b; hline_clip(g);					// E2C
			} else if (!(qtr & 4)) {
				g->p.y = y-a; g->p.x = x-b; g->p.x1 = x+b; hline_clip(g);								// C2C
			}
			break;

		case 26:	// S3E3 sy > ey
		case 27:	// S4E3 sy > ey
			g->p.y = y; g->p.x = x-b; g->p.x1 = x+b; hline_clip(g);									// C2C
			do {
				g->p.y = y-a; g->p.x = x-b; g->p.x1 = x+b; hline_clip(g);								// C2C
				if (a <= ey) {
					g->p.y = y+a; g->p.x = x-b; g->p.x1 = NONFIXED(exa); hline_clip(g);				// C2E
					g->p.y = y+a; g->p.x = NONFIXED(sxa); g->p.x1 = x+b; hline_clip(g);				// S2C
					sxa += sxd; exa += exd;
				} else if (a <= sy) {
					g->p.y = y+a; g->p.x = NONFIXED(sxa); g->p.x1 = x+b; hline_clip(g);				// S2C
					sxa += sxd;
				} else if (!(qtr & 1)) {
					g->p.y = y+a; g->p.x = x-b; g->p.x1 = x+b; hline_clip(g);							// C2C
				}
				if (P < 0) {
					P += 3 + 2*a++;
				} else {
					g->p.y = y-b; g->p.x = x-a; g->p.x1 = x+a; hline_clip(g);							// C2C
					if (b <= ey) {
						g->p.y = y+b; g->p.x = x-a; g->p.x1 = NONFIXED(exb); hline_clip(g);			// C2E
						g->p.y = y+b; g->p.x = NONFIXED(sxb); g->p.x1 = x+a; hline_clip(g);			// S2C
						sxb -= sxd; exb -= exd;
					} else if (b <= sy) {
						g->p.y = y+b; g->p.x = NONFIXED(sxb); g->p.x1 = x+a; hline_clip(g);			// S2C
						sxb -= sxd;
					} else if (!(qtr & 1)) {
						g->p.y = y+b; g->p.x = x-a; g->p.x1 = x+a; hline_clip(g);						// C2C
					}
					P += 5 + 2*(a++ - b--);
				}
			} while(a < b);
			g->p.y = y-a; g->p.x = x-b; g->p.x1 = x+b; hline_clip(g);									// C2C
			if (a <= ey) {
				g->p.y = y+a; g->p.x = x-b; g->p.x1 = NONFIXED(exa); hline_clip(g);					// C2E
				g->p.y = y+a; g->p.x = NONFIXED(sxa); g->p.x1 = x+b; hline_clip(g);					// S2C
			} else if (a <= sy) {
				g->p.y = y+a; g->p.x = NONFIXED(sxa); g->p.x1 = x+b; hline_clip(g);					// S2C
			} else if (!(qtr & 4)) {
				g->p.y = y+a; g->p.x = x-b; g->p.x1 = x+b; hline_clip(g);								// C2C
			}
			break;

		case 30:	// S3E4 sy > ey
		case 31:	// S4E4 sy > ey
			do {
				if (a <= ey) {
					g->p.y = y+a; g->p.x = NONFIXED(sxa); g->p.x1 = NONFIXED(exa); hline_clip(g);		// S2E
					sxa += sxd; exa += exd;
				} else if (a <= sy) {
					g->p.y = y+a; g->p.x = NONFIXED(sxa); g->p.x1 = x+b; hline_clip(g);				// S2C
					sxa += sxd;
				} else if (!(qtr & 1)) {
					g->p.y = y+a; g->p.x = x-b; g->p.x1 = x+b; hline_clip(g);							// C2C
				}
				if (P < 0) {
					P += 3 + 2*a++;
				} else {
					if (b <= ey) {
						g->p.y = y+b; g->p.x = NONFIXED(sxb); g->p.x1 = NONFIXED(exb); hline_clip(g);	// S2E
						sxb -= sxd; exb -= exd;
					} else if (b <= sy) {
						g->p.y = y+b; g->p.x = NONFIXED(sxb); g->p.x1 = x+a; hline_clip(g);			// S2C
						sxb -= sxd;
					} else if (!(qtr & 1)) {
						g->p.y = y+b; g->p.x = x-a; g->p.x1 = x+a; hline_clip(g);						// C2C
					}
					P += 5 + 2*(a++ - b--);
				}
			} while(a < b);
			if (a <= ey) {
				g->p.y = y+a; g->p.x = NONFIXED(sxa); g->p.x1 = x+b; hline_clip(g);				// S2C
			} else if (a <= sy) {
				g->p.y = y+a; g->p.x = NONFIXED(sxa); g->p.x1 = x+b; hline_clip(g);					// S2C
			} else if (!(qtr & 4)) {
				g->p.y = y+a; g->p.x = x-b; g->p.x1 = x+b; hline_clip(g);								// C2C
			}
			break;
		}

		#if GDISP_HARDWARE_STREAM_POS && GDISP_HARDWARE_STREAM_WRITE
			if ((g->flags & GDISP_FLG_SCRSTREAM)) {
				gdisp_lld_write_stop(g);
				g->flags &= ~GDISP_FLG_SCRSTREAM;
			}
		#endif
		MUTEX_EXIT(g);
	}

#endif

#if GDISP_NEED_ARC
	void gdispGDrawRoundedBox(GDisplay *g, coord_t x, coord_t y, coord_t cx, coord_t cy, coord_t radius, color_t color) {
		if (2*radius > cx || 2*radius > cy) {
			gdispDrawBox(x, y, cx, cy, color);
			return;
		}
		gdispDrawArc(x+radius, y+radius, radius, 90, 180, color);
		gdispDrawLine(x+radius+1, y, x+cx-2-radius, y, color);
		gdispDrawArc(x+cx-1-radius, y+radius, radius, 0, 90, color);
		gdispDrawLine(x+cx-1, y+radius+1, x+cx-1, y+cy-2-radius, color);
		gdispDrawArc(x+cx-1-radius, y+cy-1-radius, radius, 270, 360, color);
		gdispDrawLine(x+radius+1, y+cy-1, x+cx-2-radius, y+cy-1, color);
		gdispDrawArc(x+radius, y+cy-1-radius, radius, 180, 270, color);
		gdispDrawLine(x, y+radius+1, x, y+cy-2-radius, color);
	}
#endif

#if GDISP_NEED_ARC
	void gdispGFillRoundedBox(GDisplay *g, coord_t x, coord_t y, coord_t cx, coord_t cy, coord_t radius, color_t color) {
		coord_t radius2;

		radius2 = radius*2;
		if (radius2 > cx || radius2 > cy) {
			gdispFillArea(x, y, cx, cy, color);
			return;
		}
		gdispFillArc(x+radius, y+radius, radius, 90, 180, color);
		gdispFillArea(x+radius+1, y, cx-radius2, radius, color);
		gdispFillArc(x+cx-1-radius, y+radius, radius, 0, 90, color);
		gdispFillArc(x+cx-1-radius, y+cy-1-radius, radius, 270, 360, color);
		gdispFillArea(x+radius+1, y+cy-radius, cx-radius2, radius, color);
		gdispFillArc(x+radius, y+cy-1-radius, radius, 180, 270, color);
		gdispFillArea(x, y+radius, cx, cy-radius2, color);
	}
#endif

#if GDISP_NEED_PIXELREAD
	color_t gdispGGetPixelColor(GDisplay *g, coord_t x, coord_t y) {
		color_t		c;

		/* Always synchronous as it must return a value */
		MUTEX_ENTER(g);
		#if GDISP_HARDWARE_PIXELREAD
			// Best is direct pixel read
			g->p.x = x;
			g->p.y = y;
			c = gdisp_lld_get_pixel_color(g);
		#elif GDISP_HARDWARE_STREAM_READ
			// Next best is hardware streaming
			g->p.x = x;
			g->p.y = y;
			g->p.cx = 1;
			g->p.cy = 1;
			gdisp_lld_read_start(g);
			c = gdisp_lld_read_color(g);
			gdisp_lld_read_stop(g);
		#else
			// Worst is "not possible"
			#error "GDISP: GDISP_NEED_PIXELREAD has been set but there is no hardware support for reading the display"
		#endif
		MUTEX_EXIT(g);

		return c;
	}
#endif

#if GDISP_NEED_SCROLL
	void gdispGVerticalScroll(GDisplay *g, coord_t x, coord_t y, coord_t cx, coord_t cy, int lines, color_t bgcolor) {
		coord_t		abslines;
		#if !GDISP_HARDWARE_SCROLL
			coord_t 	fy, dy, ix, fx, i, j;
		#endif

		MUTEX_ENTER(g);
		#if NEED_CLIPPING
			if (x < g->clipx0) { cx -= g->clipx0 - x; x = g->clipx0; }
			if (y < g->clipy0) { cy -= g->clipy0 - y; y = g->clipy0; }
			if (!lines || cx <= 0 || cy <= 0 || x >= g->clipx1 || y >= g->clipy1) { MUTEX_EXIT(g); return; }
			if (x+cx > g->clipx1)	cx = g->clipx1 - x;
			if (y+cy > g->clipy1)	cy = g->clipy1 - y;
		#endif

		abslines = lines < 0 ? -lines : lines;
		if (abslines >= cy) {
			abslines = cy;
			cy = 0;
		} else {
			#if GDISP_HARDWARE_SCROLL
				g->p.x = x;
				g->p.y = y;
				g->p.cx = cx;
				g->p.cy = cy;
				g->p.y1 = lines;
				g->p.color = bgcolor;
				gdisp_lld_vertical_scroll(g);
				cy -= abslines;
			#elif GDISP_LINEBUF_SIZE == 0
				#error "GDISP: GDISP_NEED_SCROLL is set but there is no hardware support and GDISP_LINEBUF_SIZE is zero."
			#else
				cy -= abslines;
				if (lines < 0) {
					fy = y+cy-1;
					dy = -1;
				} else {
					fy = y;
					dy = 1;
				}
				// Move the screen - one line at a time
				for(i = 0; i < cy; i++, fy += dy) {

					// Handle where the buffer is smaller than a line
					for(ix=0; ix < cx; ix += GDISP_LINEBUF_SIZE) {

						// Calculate the data we can move in one operation
						fx = cx - ix;
						if (fx > GDISP_LINEBUF_SIZE)
							fx = GDISP_LINEBUF_SIZE;

						// Read one line of data from the screen
						#if GDISP_HARDWARE_STREAM_READ
							// Best is hardware streaming
							g->p.x = x+ix;
							g->p.y = fy+lines;
							g->p.cx = fx;
							g->p.cy = 1;
							gdisp_lld_read_start(g);
							for(j=0; j < fx; j++)
								g->linebuf[j] = gdisp_lld_read_color(g);
							gdisp_lld_read_stop(g);
						#elif GDISP_HARDWARE_PIXELREAD
							// Next best is single pixel reads
							for(j=0; j < fx; j++) {
								g->p.x = x+ix+j;
								g->p.y = fy+lines;
								g->linebuf[j] = gdisp_lld_get_pixel_color(g);
							}
						#else
							// Worst is "not possible"
							#error "GDISP: GDISP_NEED_SCROLL is set but there is no hardware support for scrolling or reading pixels."
						#endif

						// Write that line to the new location
						#if GDISP_HARDWARE_BITFILLS
							// Best is hardware bitfills
							g->p.x = x+ix;
							g->p.y = fy;
							g->p.cx = fx;
							g->p.cy = 1;
							g->p.x1 = 0;
							g->p.y1 = 0;
							g->p.x2 = fx;
							g->p.ptr = (void *)g->linebuf;
							gdisp_lld_blit_area(g);
						#elif GDISP_HARDWARE_STREAM_WRITE
							// Next best is hardware streaming
							g->p.x = x+ix;
							g->p.y = fy;
							g->p.cx = fx;
							g->p.cy = 1;
							gdisp_lld_write_start(g);
							#if GDISP_HARDWARE_STREAM_POS
								gdisp_lld_write_pos(g);
							#endif
							for(j = 0; j < fx; j++) {
								g->p.color = g->linebuf[j];
								gdisp_lld_write_color(g);
							}
							gdisp_lld_write_stop(g);
						#else
							// Worst is drawing pixels
							g->p.y = fy;
							for(g->p.x = x+ix, j = 0; j < fx; g->p.x++, j++) {
								g->p.color = g->linebuf[j];
								gdisp_lld_draw_pixel(g);
							}
						#endif
					}
				}
			#endif
		}

		/* fill the remaining gap */
		g->p.x = x;
		g->p.y = lines > 0 ? (y+cy) : y;
		g->p.cx = cx;
		g->p.cy = abslines;
		g->p.color = bgcolor;
		fillarea(g);
		MUTEX_EXIT(g);
	}
#endif

#if GDISP_NEED_CONTROL
	#if GDISP_HARDWARE_CONTROL
		void gdispGControl(GDisplay *g, unsigned what, void *value) {
			MUTEX_ENTER(g);
			g->p.x = what;
			g->p.ptr = value;
			gdisp_lld_control(g);
			#if GDISP_NEED_CLIP || GDISP_NEED_VALIDATION
				if (what == GDISP_CONTROL_ORIENTATION) {
					#if GDISP_HARDWARE_CLIP
						// Best is hardware clipping
						g->p.x = 0;
						g->p.y = 0;
						g->p.cx = g->g.Width;
						g->p.cy = g->g.Height;
						gdisp_lld_set_clip(g);
					#else
						// Worst is software clipping
						g->clipx0 = 0;
						g->clipy0 = 0;
						g->clipx1 = g->g.Width;
						g->clipy1 = g->g.Height;
					#endif
				}
			#endif
			MUTEX_EXIT(g);
		}
	#else
		void gdispGControl(GDisplay *g, unsigned what, void *value) {
			(void)what;
			(void)value;
			/* Ignore everything */
		}
	#endif
#endif

#if GDISP_NEED_QUERY
	#if GDISP_HARDWARE_QUERY
		void *gdispGQuery(GDisplay *g, unsigned what) {
			void *res;

			MUTEX_ENTER(g);
			g->p.x = (coord_t)what;
			res = gdisp_lld_query(g);
			MUTEX_EXIT(g);
			return res;
		}
	#else
		void *gdispGQuery(GDisplay *g, unsigned what) {
			(void) what;
			return (void *)-1;
		}
	#endif
#endif

/*===========================================================================*/
/* High Level Driver Routines.                                               */
/*===========================================================================*/

void gdispGDrawBox(GDisplay *g, coord_t x, coord_t y, coord_t cx, coord_t cy, color_t color) {
	if (cx <= 0 || cy <= 0) return;
	cx = x+cx-1; cy = y+cy-1;			// cx, cy are now the end point.

	MUTEX_ENTER(g);

	g->p.color = color;

	if (cx - x > 2) {
		g->p.x = x; g->p.y = y; g->p.x1 = cx; hline_clip(g);
		if (y != cy) {
			g->p.x = x; g->p.y = cy; g->p.x1 = cx; hline_clip(g);
			if (cy - y > 2) {
				y++; cy--;
				g->p.x = x; g->p.y = y; g->p.y1 = cy; vline_clip(g);
				g->p.x = cx; g->p.y = y; g->p.y1 = cy; vline_clip(g);
			}
		}
	} else {
		g->p.x = x; g->p.y = y; g->p.y1 = cy; vline_clip(g);
		if (x != cx) {
			g->p.x = cx; g->p.y = y; g->p.y1 = cy; vline_clip(g);
		}
	}

	#if GDISP_HARDWARE_STREAM_POS && GDISP_HARDWARE_STREAM_WRITE
		if ((g->flags & GDISP_FLG_SCRSTREAM)) {
			gdisp_lld_write_stop(g);
			g->flags &= ~GDISP_FLG_SCRSTREAM;
		}
	#endif
	MUTEX_EXIT(g);
}

#if GDISP_NEED_CONVEX_POLYGON
	void gdispGDrawPoly(GDisplay *g, coord_t tx, coord_t ty, const point *pntarray, unsigned cnt, color_t color) {
		const point	*epnt, *p;

		epnt = &pntarray[cnt-1];

		MUTEX_ENTER(g);
		g->p.color = color;
		for(p = pntarray; p < epnt; p++) {
			g->p.x=tx+p->x; g->p.y=ty+p->y; g->p.x1=tx+p[1].x; g->p.y1=ty+p[1].y; line_clip(g);
		}
		g->p.x=tx+p->x; g->p.y=ty+p->y; g->p.x1=tx+pntarray->x; g->p.y1=ty+pntarray->y; line_clip(g);

		#if GDISP_HARDWARE_STREAM_POS && GDISP_HARDWARE_STREAM_WRITE
			if ((g->flags & GDISP_FLG_SCRSTREAM)) {
				gdisp_lld_write_stop(g);
				g->flags &= ~GDISP_FLG_SCRSTREAM;
			}
		#endif
		MUTEX_EXIT(g);
	}

	void gdispGFillConvexPoly(GDisplay *g, coord_t tx, coord_t ty, const point *pntarray, unsigned cnt, color_t color) {
		const point	*lpnt, *rpnt, *epnts;
		fixed		lx, rx, lk, rk;
		coord_t		y, ymax, lxc, rxc;

		epnts = &pntarray[cnt-1];

		/* Find a top point */
		rpnt = pntarray;
		for(lpnt=pntarray+1; lpnt <= epnts; lpnt++) {
			if (lpnt->y < rpnt->y)
				rpnt = lpnt;
		}
		lx = rx = FIXED(rpnt->x);
		y = rpnt->y;

		/* Work out the slopes of the two attached line segs */
		for (lpnt = rpnt <= pntarray ? epnts : rpnt-1; lpnt->y == y; cnt--) {
			if (!cnt) return;
			lx = FIXED(lpnt->x);
			lpnt = lpnt <= pntarray ? epnts : lpnt-1;
		}
		for (rpnt = rpnt >= epnts ? pntarray : rpnt+1; rpnt->y == y; cnt--) {
			if (!cnt) return;
			rx = FIXED(rpnt->x);
			rpnt = rpnt >= epnts ? pntarray : rpnt+1;
		}
		lk = (FIXED(lpnt->x) - lx) / (lpnt->y - y);
		rk = (FIXED(rpnt->x) - rx) / (rpnt->y - y);

		MUTEX_ENTER(g);
		g->p.color = color;
		while(1) {
			/* Determine our boundary */
			ymax = rpnt->y < lpnt->y ? rpnt->y : lpnt->y;

			/* Scan down the line segments until we hit a boundary */
			for(; y < ymax; y++) {
				lxc = NONFIXED(lx);
				rxc = NONFIXED(rx);
				/*
				 * Doesn't print the right hand point in order to allow polygon joining.
				 * Also ensures that we draw from left to right with the minimum number
				 * of pixels.
				 */
				if (lxc < rxc) {
					g->p.x=tx+lxc; g->p.y=ty+y; g->p.x1=tx+rxc-1; hline_clip(g);
				} else if (lxc > rxc) {
					g->p.x=tx+rxc; g->p.y=ty+y; g->p.x1=tx+lxc-1; hline_clip(g);
				}

				lx += lk;
				rx += rk;
			}

			if (!cnt) {
				#if GDISP_HARDWARE_STREAM_POS && GDISP_HARDWARE_STREAM_WRITE
					if ((g->flags & GDISP_FLG_SCRSTREAM)) {
						gdisp_lld_write_stop(g);
						g->flags &= ~GDISP_FLG_SCRSTREAM;
					}
				#endif
				MUTEX_EXIT(g);
				return;
			}
			cnt--;

			/* Replace the appropriate point */
			if (ymax == lpnt->y) {
				for (lpnt = lpnt <= pntarray ? epnts : lpnt-1; lpnt->y == y; cnt--) {
					if (!cnt) {
						#if GDISP_HARDWARE_STREAM_POS && GDISP_HARDWARE_STREAM_WRITE
							if ((g->flags & GDISP_FLG_SCRSTREAM)) {
								gdisp_lld_write_stop(g);
								g->flags &= ~GDISP_FLG_SCRSTREAM;
							}
						#endif
						MUTEX_EXIT(g);
						return;
					}
					lx = FIXED(lpnt->x);
					lpnt = lpnt <= pntarray ? epnts : lpnt-1;
				}
				lk = (FIXED(lpnt->x) - lx) / (lpnt->y - y);
			} else {
				for (rpnt = rpnt >= epnts ? pntarray : rpnt+1; rpnt->y == y; cnt--) {
					if (!cnt) {
						#if GDISP_HARDWARE_STREAM_POS && GDISP_HARDWARE_STREAM_WRITE
							if ((g->flags & GDISP_FLG_SCRSTREAM)) {
								gdisp_lld_write_stop(g);
								g->flags &= ~GDISP_FLG_SCRSTREAM;
							}
						#endif
						MUTEX_EXIT(g);
						return;
					}
					rx = FIXED(rpnt->x);
					rpnt = rpnt >= epnts ? pntarray : rpnt+1;
				}
				rk = (FIXED(rpnt->x) - rx) / (rpnt->y - y);
			}
		}
	}
#endif

#if GDISP_NEED_TEXT
	#include "mcufont.h"

	#if GDISP_NEED_ANTIALIAS && GDISP_HARDWARE_PIXELREAD
		static void drawcharline(int16_t x, int16_t y, uint8_t count, uint8_t alpha, void *state) {
			#define GD	((GDisplay *)state)
			if (y < GD->t.clipy0 || y >= GD->t.clipy1 || x < GD->t.clipx0 || x+count > GD->t.clipx1) return;
			if (alpha == 255) {
				GD->p.x = x; GD->p.y = y; GD->p.x1 = x+count-1; GD->p.color = GD->t.color;
				hline_clip(g);
			} else {
				for (; count; count--, x++) {
					GD->p.x = x; GD->p.y = y;
					GD->p.color = gdispBlendColor(GD->t.color, gdisp_lld_get_pixel_color(GD), alpha);
					drawpixel_clip(g);
				}
			}
			#undef GD
		}
	#else
		static void drawcharline(int16_t x, int16_t y, uint8_t count, uint8_t alpha, void *state) {
			#define GD	((GDisplay *)state)
			if (y < GD->t.clipy0 || y >= GD->t.clipy1 || x < GD->t.clipx0 || x+count > GD->t.clipx1) return;
			if (alpha > 0x80) {			// A best approximation when using anti-aliased fonts but we can't actually draw them anti-aliased
				GD->p.x = x; GD->p.y = y; GD->p.x1 = x+count-1; GD->p.color = GD->t.color;
				hline_clip(g);
			}
			#undef GD
		}
	#endif

	#if GDISP_NEED_ANTIALIAS
		static void fillcharline(int16_t x, int16_t y, uint8_t count, uint8_t alpha, void *state) {
			#define GD	((GDisplay *)state)
			if (y < GD->t.clipy0 || y >= GD->t.clipy1 || x < GD->t.clipx0 || x+count > GD->t.clipx1) return;
			if (alpha == 255) {
				GD->p.color = GD->t.color;
			} else {
				GD->p.color = gdispBlendColor(GD->t.color, GD->t.bgcolor, alpha);
			}
			GD->p.x = x; GD->p.y = y; GD->p.x1 = x+count-1;
			hline_clip(g);
			#undef GD
		}
	#else
		#define fillcharline	drawcharline
	#endif

	/* Callback to render characters. */
	static uint8_t drawcharglyph(int16_t x, int16_t y, mf_char ch, void *state) {
		return mf_render_character(g->t.font, x, y, ch, drawcharline, state);
	}

	/* Callback to render characters. */
	static uint8_t fillcharglyph(int16_t x, int16_t y, mf_char ch, void *state) {
		return mf_render_character(g->t.font, x, y, ch, fillcharline, state);
	}

	void gdispGDrawChar(GDisplay *g, coord_t x, coord_t y, uint16_t c, font_t font, color_t color) {
		MUTEX_ENTER(g);
		g->t.font = font;
		g->t.clipx0 = x;
		g->t.clipy0 = y;
		g->t.clipx1 = x + mf_character_width(font, c) + font->baseline_x;
		g->t.clipy1 = y + font->height;
		g->t.color = color;
		mf_render_character(font, x, y, c, drawcharline, g);
		#if GDISP_HARDWARE_STREAM_POS && GDISP_HARDWARE_STREAM_WRITE
			if ((g->flags & GDISP_FLG_SCRSTREAM)) {
				gdisp_lld_write_stop(g);
				g->flags &= ~GDISP_FLG_SCRSTREAM;
			}
		#endif
		MUTEX_EXIT(g);
	}

	void gdispGFillChar(GDisplay *g, coord_t x, coord_t y, uint16_t c, font_t font, color_t color, color_t bgcolor) {
		MUTEX_ENTER(g);
		g->p.cx = mf_character_width(font, c) + font->baseline_x;
		g->p.cy = font->height;
		g->t.font = font;
		g->t.clipx0 = g->p.x = x;
		g->t.clipy0 = g->p.y = y;
		g->t.clipx1 = g->p.x+g->p.cx;
		g->t.clipy1 = g->p.y+g->p.cy;
		g->t.color = color;
		g->t.bgcolor = g->p.color = bgcolor;

		TEST_CLIP_AREA(g) {
			fillarea(g);
			mf_render_character(font, x, y, c, fillcharline, g);
		}
		#if GDISP_HARDWARE_STREAM_POS && GDISP_HARDWARE_STREAM_WRITE
			if ((g->flags & GDISP_FLG_SCRSTREAM)) {
				gdisp_lld_write_stop(g);
				g->flags &= ~GDISP_FLG_SCRSTREAM;
			}
		#endif
		MUTEX_EXIT(g);
	}

	void gdispGDrawString(GDisplay *g, coord_t x, coord_t y, const char *str, font_t font, color_t color) {
		MUTEX_ENTER(g);
		g->t.font = font;
		g->t.clipx0 = x;
		g->t.clipy0 = y;
		g->t.clipx1 = x + mf_get_string_width(font, str, 0, 0);
		g->t.clipy1 = y + font->height;
		g->t.color = color;

		mf_render_aligned(font, x+font->baseline_x, y, MF_ALIGN_LEFT, str, 0, drawcharglyph, g);
		#if GDISP_HARDWARE_STREAM_POS && GDISP_HARDWARE_STREAM_WRITE
			if ((g->flags & GDISP_FLG_SCRSTREAM)) {
				gdisp_lld_write_stop(g);
				g->flags &= ~GDISP_FLG_SCRSTREAM;
			}
		#endif
		MUTEX_EXIT(g);
	}

	void gdispGFillString(GDisplay *g, coord_t x, coord_t y, const char *str, font_t font, color_t color, color_t bgcolor) {
		MUTEX_ENTER(g);
		g->p.cx = mf_get_string_width(font, str, 0, 0);
		g->p.cy = font->height;
		g->t.font = font;
		g->t.clipx0 = g->p.x = x;
		g->t.clipy0 = g->p.y = y;
		g->t.clipx1 = g->p.x+g->p.cx;
		g->t.clipy1 = g->p.y+g->p.cy;
		g->t.color = color;
		g->t.bgcolor = g->p.color = bgcolor;

		TEST_CLIP_AREA(g) {
			fillarea(g);
			mf_render_aligned(font, x+font->baseline_x, y, MF_ALIGN_LEFT, str, 0, fillcharglyph, g);
		}

		#if GDISP_HARDWARE_STREAM_POS && GDISP_HARDWARE_STREAM_WRITE
			if ((g->flags & GDISP_FLG_SCRSTREAM)) {
				gdisp_lld_write_stop(g);
				g->flags &= ~GDISP_FLG_SCRSTREAM;
			}
		#endif
		MUTEX_EXIT(g);
	}

	void gdispGDrawStringBox(GDisplay *g, coord_t x, coord_t y, coord_t cx, coord_t cy, const char* str, font_t font, color_t color, justify_t justify) {
		MUTEX_ENTER(g);
		g->t.font = font;
		g->t.clipx0 = x;
		g->t.clipy0 = y;
		g->t.clipx1 = x+cx;
		g->t.clipy1 = y+cy;
		g->t.color = color;

		/* Select the anchor position */
		switch(justify) {
		case justifyCenter:
			x += (cx + 1) / 2;
			break;
		case justifyRight:
			x += cx;
			break;
		default:	// justifyLeft
			x += font->baseline_x;
			break;
		}
		y += (cy+1 - font->height)/2;

		mf_render_aligned(font, x, y, justify, str, 0, drawcharglyph, g);

		#if GDISP_HARDWARE_STREAM_POS && GDISP_HARDWARE_STREAM_WRITE
			if ((g->flags & GDISP_FLG_SCRSTREAM)) {
				gdisp_lld_write_stop(g);
				g->flags &= ~GDISP_FLG_SCRSTREAM;
			}
		#endif
		MUTEX_EXIT(g);
	}

	void gdispGFillStringBox(GDisplay *g, coord_t x, coord_t y, coord_t cx, coord_t cy, const char* str, font_t font, color_t color, color_t bgcolor, justify_t justify) {
		MUTEX_ENTER(g);
		g->p.cx = cx;
		g->p.cy = cy;
		g->t.font = font;
		g->t.clipx0 = g->p.x = x;
		g->t.clipy0 = g->p.y = y;
		g->t.clipx1 = x+cx;
		g->t.clipy1 = y+cy;
		g->t.color = color;
		g->t.bgcolor = g->p.color = bgcolor;

		TEST_CLIP_AREA(g) {

			// background fill
			fillarea(g);

			/* Select the anchor position */
			switch(justify) {
			case justifyCenter:
				x += (cx + 1) / 2;
				break;
			case justifyRight:
				x += cx;
				break;
			default:	// justifyLeft
				x += font->baseline_x;
				break;
			}
			y += (cy+1 - font->height)/2;

			/* Render */
			mf_render_aligned(font, x, y, justify, str, 0, fillcharglyph, g);
		}

		#if GDISP_HARDWARE_STREAM_POS && GDISP_HARDWARE_STREAM_WRITE
			if ((g->flags & GDISP_FLG_SCRSTREAM)) {
				gdisp_lld_write_stop(g);
				g->flags &= ~GDISP_FLG_SCRSTREAM;
			}
		#endif
		MUTEX_EXIT(g);
	}

	coord_t gdispGetFontMetric(font_t font, fontmetric_t metric) {
		/* No mutex required as we only read static data */
		switch(metric) {
		case fontHeight:			return font->height;
		case fontDescendersHeight:	return font->height - font->baseline_y;
		case fontLineSpacing:		return font->line_height;
		case fontCharPadding:		return 0;
		case fontMinWidth:			return font->min_x_advance;
		case fontMaxWidth:			return font->max_x_advance;
		}
		return 0;
	}

	coord_t gdispGetCharWidth(char c, font_t font) {
		/* No mutex required as we only read static data */
		return mf_character_width(font, c);
	}

	coord_t gdispGetStringWidth(const char* str, font_t font) {
		/* No mutex required as we only read static data */
		return mf_get_string_width(font, str, 0, 0);
	}
#endif

color_t gdispBlendColor(color_t fg, color_t bg, uint8_t alpha)
{
	uint16_t fg_ratio = alpha + 1;
	uint16_t bg_ratio = 256 - alpha;
	uint16_t r, g, b;
	
	r = RED_OF(fg) * fg_ratio;
	g = GREEN_OF(fg) * fg_ratio;
	b = BLUE_OF(fg) * fg_ratio;
	
	r += RED_OF(bg) * bg_ratio;
	g += GREEN_OF(bg) * bg_ratio;
	b += BLUE_OF(bg) * bg_ratio;
	
	r /= 256;
	g /= 256;
	b /= 256;
	
	return RGB2COLOR(r, g, b);
}

#if (!defined(gdispPackPixels) && !defined(GDISP_PIXELFORMAT_CUSTOM))
	void gdispPackPixels(pixel_t *buf, coord_t cx, coord_t x, coord_t y, color_t color) {
		/* No mutex required as we only read static data */
		#if defined(GDISP_PIXELFORMAT_RGB888)
			#error "GDISP: Packed pixels not supported yet"
		#elif defined(GDISP_PIXELFORMAT_RGB444)
			#error "GDISP: Packed pixels not supported yet"
		#elif defined(GDISP_PIXELFORMAT_RGB666)
			#error "GDISP: Packed pixels not supported yet"
		#elif
			#error "GDISP: Unsupported packed pixel format"
		#endif
	}
#endif


#endif /* GFX_USE_GDISP */
/** @} */
