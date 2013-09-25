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

#if !GDISP_HARDWARE_STREAM && !GDISP_HARDWARE_DRAWPIXEL
	#error "GDISP Driver: Either GDISP_HARDWARE_STREAM or GDISP_HARDWARE_DRAWPIXEL must be defined"
#endif

#if 1
	#undef INLINE
	#define INLINE	inline
#else
	#undef INLINE
	#define INLINE
#endif

/*===========================================================================*/
/* Driver local variables.                                                   */
/*===========================================================================*/

#define	GC				((GDISPDriver *)GDISP)
GDISPControl			*GDISP = &GDISP_DRIVER_STRUCT.g;

#if GDISP_NEED_MULTITHREAD
	#define MUTEX_INIT()		gfxMutexInit(&GC->mutex)
	#define MUTEX_ENTER()		gfxMutexEnter(&GC->mutex)
	#define MUTEX_EXIT()		gfxMutexExit(&GC->mutex)
#else
	#define MUTEX_INIT()
	#define MUTEX_ENTER()
	#define MUTEX_EXIT()
#endif

#if GDISP_HARDWARE_STREAM_STOP
	#define STREAM_CLEAR()		if ((GC->flags & GDISP_FLG_INSTREAM)) {	\
									gdisp_lld_stream_stop(GC);				\
									GC->flags &= ~GDISP_FLG_INSTREAM;	\
								}
#else
	#define STREAM_CLEAR()		GC->flags &= ~GDISP_FLG_INSTREAM
#endif

#define NEED_CLIPPING	(!GDISP_HARDWARE_CLIP && (GDISP_NEED_VALIDATION || GDISP_NEED_CLIP))

/*==========================================================================*/
/* Internal functions.														*/
/*==========================================================================*/

// drawpixel_clip()
// Parameters:	x,y
// Alters:		cx, cy (if using streaming)
#if GDISP_HARDWARE_DRAWPIXEL
	// Best is hardware accelerated pixel draw
	#if NEED_CLIPPING
		static INLINE void drawpixel_clip(void) {
			if (GC->p.x >= GC->clipx0 && GC->p.x < GC->clipx1 && GC->p.y >= GC->clipy0 && GC->p.y < GC->clipy1)
				gdisp_lld_draw_pixel(GC);
		}
	#else
		#define drawpixel_clip()	gdisp_lld_draw_pixel(GC)
	#endif
#else
	// Worst is streaming
	static INLINE void drawpixel_clip(void) {
		#if NEED_CLIPPING
			if (GC->p.x < GC->clipx0 || GC->p.x >= GC->clipx1 || GC->p.y < GC->clipy0 || GC->p.y >= GC->clipy1)
				return;
		#endif

		GC->p.cx = GC->p.cy = 1;
		gdisp_lld_stream_start(GC);
		gdisp_lld_stream_color(GC);
		#if GDISP_HARDWARE_STREAM_STOP
			gdisp_lld_stream_stop(GC);
		#endif
	}
#endif

// fillarea()
// Parameters:	x,y cx,cy and color
// Alters:		nothing
// Note:		This is not clipped
#if GDISP_HARDWARE_FILLS
	// Best is hardware accelerated area fill
	#define fillarea()	gdisp_lld_fill_area(GC)
#elif GDISP_HARDWARE_STREAM
	// Next best is hardware streaming
	static INLINE void fillarea(void) {
		uint32_t	area;

		area = (uint32_t)GC->p.cx * GC->p.cy;

		gdisp_lld_stream_start(GC);
		for(; area; area--)
			gdisp_lld_stream_color(GC);
		#if GDISP_HARDWARE_STREAM_STOP
			gdisp_lld_stream_stop(GC);
		#endif
	}
#else
	// Worst is drawing pixels
	static INLINE void fillarea(void) {
		coord_t x0, y0, x1, y1;

		x0 = GC->p.x;
		y0 = GC->p.y;
		x1 = GC->p.x + GC->p.cx;
		y1 = GC->p.y + GC->p.cy;
		for(; GC->p.y < y1; GC->p.y++, GC->p.x = x0)
			for(; GC->p.x < x1; GC->p.x++)
				gdisp_lld_draw_pixel(GC);
		GC->p.y = y0;
	}
#endif

#if NEED_CLIPPING
	#define TEST_CLIP_AREA(x,y,cx,cy)													\
				if ((x) < GC->clipx0) { (cx) -= GC->clipx0 - (x); (x) = GC->clipx0; }	\
				if ((y) < GC->clipy0) { (cy) -= GC->clipy0 - (y); (y) = GC->clipy0; }	\
				if ((x) + (cx) > GC->clipx1)	(cx) = GC->clipx1 - (x);				\
				if ((y) + (cy) > GC->clipy1)	(cy) = GC->clipy1 - (y);				\
				if ((cx) > 0 && (cy) > 0)
#else
	#define TEST_CLIP_AREA(x,y,cx,cy)
#endif

// Parameters:	x,y and x1
// Alters:		x,y x1,y1 cx,cy
static void hline_clip(void) {
	// Swap the points if necessary so it always goes from x to x1
	if (GC->p.x1 < GC->p.x) {
		GC->p.cx = GC->p.x; GC->p.x = GC->p.x1; GC->p.x1 = GC->p.cx;
	}

	// Clipping
	#if NEED_CLIPPING
		if (GC->p.y < GC->clipy0 || GC->p.y >= GC->clipy1) return;
		if (GC->p.x < GC->clipx0) GC->p.x = GC->clipx0;
		if (GC->p.x1 >= GC->clipx1) GC->p.x1 = GC->clipx1 - 1;
		if (GC->p.x1 < GC->p.x) return;
	#endif

	// This is an optimization for the point case. It is only worthwhile however if we
	// have hardware fills or if we support both hardware pixel drawing and hardware streaming
	#if GDISP_HARDWARE_FILLS || (GDISP_HARDWARE_DRAWPIXEL && GDISP_HARDWARE_STREAM)
		// Is this a point
		if (GC->p.x == GC->p.x1) {
			#if GDISP_HARDWARE_DRAWPIXEL
				// Best is hardware accelerated pixel draw
				gdisp_lld_draw_pixel(GC);
			#else
				// Worst is streaming
				GC->p.cx = GC->p.cy = 1;
				gdisp_lld_stream_start(GC);
				gdisp_lld_stream_color(GC);
				#if GDISP_HARDWARE_STREAM_STOP
					gdisp_lld_stream_stop(GC);
				#endif
			#endif
			return;
		}
	#endif

	#if GDISP_HARDWARE_FILLS
		// Best is hardware accelerated area fill
		GC->p.cx = GC->p.x1 - GC->p.x + 1;
		GC->p.cy = 1;
		gdisp_lld_fill_area(GC);
	#elif GDISP_HARDWARE_STREAM
		// Next best is streaming
		GC->p.cx = GC->p.x1 - GC->p.x;
		GC->p.cy = 1;
		gdisp_lld_stream_start(GC);
		do { gdisp_lld_stream_color(GC); } while(GC->p.cx--);
		#if GDISP_HARDWARE_STREAM_STOP
			gdisp_lld_stream_stop(GC);
		#endif
	#else
		// Worst is drawing pixels
		for(; GC->p.x <= GC->p.x1; GC->p.x++)
			gdisp_lld_draw_pixel(GC);
	#endif
}

// Parameters:	x,y and y1
// Alters:		x,y x1,y1 cx,cy
static void vline_clip(void) {
	// Swap the points if necessary so it always goes from y to y1
	if (GC->p.y1 < GC->p.y) {
		GC->p.cy = GC->p.y; GC->p.y = GC->p.y1; GC->p.y1 = GC->p.cy;
	}

	// Clipping
	#if NEED_CLIPPING
		if (GC->p.x < GC->clipx0 || GC->p.x >= GC->clipx1) return;
		if (GC->p.y < GC->clipy0) GC->p.y = GC->clipy0;
		if (GC->p.y1 >= GC->clipy1) GC->p.y1 = GC->clipy1 - 1;
		if (GC->p.y1 < GC->p.y) return;
	#endif

	// This is an optimization for the point case. It is only worthwhile however if we
	// have hardware fills or if we support both hardware pixel drawing and hardware streaming
	#if GDISP_HARDWARE_FILLS || (GDISP_HARDWARE_DRAWPIXEL && GDISP_HARDWARE_STREAM)
		// Is this a point
		if (GC->p.y == GC->p.y1) {
			#if GDISP_HARDWARE_DRAWPIXEL
				// Best is hardware accelerated pixel draw
				gdisp_lld_draw_pixel(GC);
			#else
				// Worst is streaming
				GC->p.cx = GC->p.cy = 1;
				gdisp_lld_stream_start(GC);
				gdisp_lld_stream_color(GC);
				#if GDISP_HARDWARE_STREAM_STOP
					gdisp_lld_stream_stop(GC);
				#endif
			#endif
			return;
		}
	#endif

	#if GDISP_HARDWARE_FILLS
		// Best is hardware accelerated area fill
		GC->p.cy = GC->p.y1 - GC->p.y + 1;
		GC->p.cx = 1;
		gdisp_lld_fill_area(GC);
	#elif GDISP_HARDWARE_STREAM
		// Next best is streaming
		GC->p.cy = GC->p.y1 - GC->p.y;
		GC->p.cx = 1;
		gdisp_lld_stream_start(GC);
		do { gdisp_lld_stream_color(GC); } while(GC->p.cy--);
		#if GDISP_HARDWARE_STREAM_STOP
			gdisp_lld_stream_stop(GC);
		#endif
	#else
		// Worst is drawing pixels
		for(; GC->p.y <= GC->p.y1; GC->p.y++)
			gdisp_lld_draw_pixel(GC);
	#endif
}

// Parameters:	x,y and x1,y1
// Alters:		x,y x1,y1 cx,cy
static void line_clip(void) {
	int16_t dy, dx;
	int16_t addx, addy;
	int16_t P, diff, i;

	// Is this a horizontal line (or a point)
	if (GC->p.y == GC->p.y1) {
		hline_clip();
		return;
	}

	// Is this a vertical line (or a point)
	if (GC->p.x == GC->p.x1) {
		vline_clip();
		return;
	}

	// Not horizontal or vertical

	// Use Bresenham's line drawing algorithm.
	//	This should be replaced with fixed point slope based line drawing
	//	which is more efficient on modern processors as it branches less.
	//	When clipping is needed, all the clipping could also be done up front
	//	instead of on each pixel.

	if (GC->p.x1 >= GC->p.x) {
		dx = GC->p.x1 - GC->p.x;
		addx = 1;
	} else {
		dx = GC->p.x - GC->p.x1;
		addx = -1;
	}
	if (GC->p.y1 >= GC->p.y) {
		dy = GC->p.y1 - GC->p.y;
		addy = 1;
	} else {
		dy = GC->p.y - GC->p.y1;
		addy = -1;
	}

	if (dx >= dy) {
		dy <<= 1;
		P = dy - dx;
		diff = P - dx;

		for(i=0; i<=dx; ++i) {
			drawpixel_clip();
			if (P < 0) {
				P  += dy;
				GC->p.x += addx;
			} else {
				P  += diff;
				GC->p.x += addx;
				GC->p.y += addy;
			}
		}
	} else {
		dx <<= 1;
		P = dx - dy;
		diff = P - dy;

		for(i=0; i<=dy; ++i) {
			drawpixel_clip();
			if (P < 0) {
				P  += dx;
				GC->p.y += addy;
			} else {
				P  += diff;
				GC->p.x += addx;
				GC->p.y += addy;
			}
		}
	}
}

/*===========================================================================*/
/* Driver exported functions.                                                */
/*===========================================================================*/

/* Our module initialiser */
void _gdispInit(void) {
	MUTEX_INIT();

	/* Initialise driver */
	MUTEX_ENTER();
	GC->flags = 0;
	gdisp_lld_init(GC);
	#if GDISP_NEED_VALIDATION || GDISP_NEED_CLIP
		#if GDISP_HARDWARE_CLIP
			GC->p.x = x;
			GC->p.y = y;
			GC->p.cx = cx;
			GC->p.cy = cy;
			gdisp_lld_set_clip(GC);
		#else
			GC->clipx0 = 0;
			GC->clipy0 = 0;
			GC->clipx1 = GC->g.Width;
			GC->clipy1 = GC->g.Height;
		#endif
	#endif
	MUTEX_EXIT();
}

#if GDISP_NEED_STREAMING
	void gdispStreamStart(coord_t x, coord_t y, coord_t cx, coord_t cy) {
		MUTEX_ENTER();

		#if NEED_CLIPPING
			// Test if the area is valid - if not then exit
			if (x < GC->clipx0 || x+cx > GC->clipx1 || y < GC->clipy0 || y+cy > GC->clipy1) {
				MUTEX_EXIT();
				return;
			}
		#endif

		GC->flags |= GDISP_FLG_INSTREAM;

		#if GDISP_HARDWARE_STREAM
			// Best is hardware streaming
			GC->p.x = x;
			GC->p.y = y;
			GC->p.cx = cx;
			GC->p.cy = cy;
			gdisp_lld_stream_start(GC);
		#else
			// Worst - save the parameters and use pixel drawing

			// Use x,y as the current position, x1,y1 as the save position and x2,y2 as the end position, cx = bufpos
			GC->p.x1 = GC->p.x = x;
			GC->p.y1 = GC->p.y = y;
			GC->p.x2 = x + cx;
			GC->p.y2 = y + cy;
			GC->p.cx = 0;
		#endif

		// Don't release the mutex as gdispStreamEnd() will do that.
	}

	void gdispStreamColor(color_t color) {
		#if !GDISP_HARDWARE_STREAM && GDISP_LINEBUF_SIZE != 0 && GDISP_HARDWARE_BITFILLS
			coord_t	 sx1, sy1;
		#endif

		// Don't touch the mutex as we should already own it

		// Ignore this call if we are not streaming
		if (!(GC->flags & GDISP_FLG_INSTREAM))
			return;

		#if GDISP_HARDWARE_STREAM
			// Best is hardware streaming
			GC->p.color = color;
			gdisp_lld_stream_color(GC);
		#elif GDISP_LINEBUF_SIZE != 0 && GDISP_HARDWARE_BITFILLS
			GC->linebuf[GC->p.cx++] = color;
			GC->p.x++;
			if (GC->p.cx >= GDISP_LINEBUF_SIZE) {
				sx1 = GC->p.x1;
				sy1 = GC->p.y1;
				GC->p.x -= GC->p.cx;
				GC->p.cy = 1;
				GC->p.x1 = 0;
				GC->p.y1 = 0;
				GC->p.ptr = (void *)GC->linebuf;
				gdisp_lld_blit_area(GC);
				GC->p.x1 = sx1;
				GC->p.y1 = sy1;
				GC->p.x += GC->p.cx;
				GC->p.cx = 0;
			}

			// Just wrap at end-of-line and end-of-buffer
			if (GC->p.x >= GC->p.x2) {
				if (GC->p.cx) {
					sx1 = GC->p.x1;
					sy1 = GC->p.y1;
					GC->p.x -= GC->p.cx;
					GC->p.cy = 1;
					GC->p.x1 = 0;
					GC->p.y1 = 0;
					GC->p.ptr = (void *)GC->linebuf;
					gdisp_lld_blit_area(GC);
					GC->p.x1 = sx1;
					GC->p.y1 = sy1;
					GC->p.cx = 0;
				}
				GC->p.x = GC->p.x1;
				if (++GC->p.y >= GC->p.y2)
					GC->p.y = GC->p.y1;
			}
		#else
			// Worst is using pixel drawing
			GC->p.color = color;
			gdisp_lld_draw_pixel(GC);

			// Just wrap at end-of-line and end-of-buffer
			if (++GC->p.x >= GC->p.x2) {
				GC->p.x = GC->p.x1;
				if (++GC->p.y >= GC->p.y2)
					GC->p.y = GC->p.y1;
			}
		#endif
	}

	void gdispStreamStop(void) {
		// Only release the mutex and end the stream if we are actually streaming.
		if (!(GC->flags & GDISP_FLG_INSTREAM))
			return;

		#if GDISP_HARDWARE_STREAM
			#if GDISP_HARDWARE_STREAM_STOP
				gdisp_lld_stream_stop(GC);
			#endif
		#elif GDISP_LINEBUF_SIZE != 0 && GDISP_HARDWARE_BITFILLS
			if (GC->p.cx) {
				GC->p.x -= GC->p.cx;
				GC->p.cy = 1;
				GC->p.x1 = 0;
				GC->p.y1 = 0;
				GC->p.ptr = (void *)GC->linebuf;
				gdisp_lld_blit_area(GC);
			}
		#endif
		GC->flags &= ~GDISP_FLG_INSTREAM;
		MUTEX_EXIT();
	}
#endif

void gdispDrawPixel(coord_t x, coord_t y, color_t color) {
	MUTEX_ENTER();
	GC->p.x		= x;
	GC->p.y		= y;
	GC->p.color	= color;
	drawpixel_clip();
	MUTEX_EXIT();
}
	
void gdispDrawLine(coord_t x0, coord_t y0, coord_t x1, coord_t y1, color_t color) {
	MUTEX_ENTER();
	GC->p.x = x0;
	GC->p.y = y0;
	GC->p.x1 = x1;
	GC->p.y1 = y1;
	GC->p.color = color;
	line_clip();
	MUTEX_EXIT();
}

void gdispClear(color_t color) {
	// Note - clear() ignores the clipping area. It clears the screen.
	MUTEX_ENTER();

	#if GDISP_HARDWARE_CLEARS
		// Best is hardware accelerated clear
		GC->p.color = color;
		gdisp_lld_clear(GC);
	#elif GDISP_HARDWARE_FILLS
		// Next best is hardware accelerated area fill
		GC->p.x = GC->p.y = 0;
		GC->p.cx = GC->g.Width;
		GC->p.cy = GC->g.Height;
		GC->p.color = color;
		gdisp_lld_fill_area(GC);
	#elif GDISP_HARDWARE_STREAM
		// Next best is streaming
		uint32_t	area;

		GC->p.x = GC->p.y = 0;
		GC->p.cx = GC->g.Width;
		GC->p.cy = GC->g.Height;
		GC->p.color = color;
		area = (uint32_t)GC->p.cx * GC->p.cy;

		gdisp_lld_stream_start(GC);
		for(; area; area--)
			gdisp_lld_stream_color(GC);
		#if GDISP_HARDWARE_STREAM_STOP
			gdisp_lld_stream_stop(GC);
		#endif
	#else
		// Worst is drawing pixels
		GC->p.color = color;
		for(GC->p.y = 0; GC->p.y < GC->g.Height; GC->p.y++)
			for(GC->p.x = 0; GC->p.x < GC->g.Width; GC->p.x++)
				gdisp_lld_draw_pixel(GC);
	#endif
	MUTEX_EXIT();
}

void gdispFillArea(coord_t x, coord_t y, coord_t cx, coord_t cy, color_t color) {
	MUTEX_ENTER();
	TEST_CLIP_AREA(x,y,cx,cy) {
		GC->p.x = x;
		GC->p.y = y;
		GC->p.cx = cx;
		GC->p.cy = cy;
		GC->p.color = color;
		fillarea();
	}
	MUTEX_EXIT();
}
	
void gdispBlitAreaEx(coord_t x, coord_t y, coord_t cx, coord_t cy, coord_t srcx, coord_t srcy, coord_t srccx, const pixel_t *buffer) {
	MUTEX_ENTER();

	#if NEED_CLIPPING
		// This is a different cliping to fillarea() as it needs to take into account srcx,srcy
		if (x < GC->clipx0) { cx -= GC->clipx0 - x; srcx += GC->clipx0 - x; x = GC->clipx0; }
		if (y < GC->clipy0) { cy -= GC->clipy0 - y; srcy += GC->clipy0 - x; y = GC->clipy0; }
		if (x+cx > GC->clipx1)	cx = GC->clipx1 - x;
		if (y+cy > GC->clipy1)	cy = GC->clipy1 - y;
		if (srcx+cx > srccx) cx = srccx - srcx;
		if (cx <= 0 || cy <= 0) { MUTEX_EXIT(); return; }
	#endif

	#if GDISP_HARDWARE_BITFILLS
		// Best is hardware bitfills
		GC->p.x = x;
		GC->p.y = y;
		GC->p.cx = cx;
		GC->p.cy = cy;
		GC->p.x1 = srcx;
		GC->p.y1 = srcy;
		GC->p.x2 = srccx;
		GC->p.ptr = (void *)buffer;
		gdisp_lld_blit_area(GC);
	#elif GDISP_HARDWARE_STREAM
		// Next best is hardware streaming

		// Translate buffer to the real image data, use srcx,srcy as the end point, srccx as the buffer line gap
		buffer += srcy*srccx+srcx;
		srcx = x + cx;
		srcy = y + cy;
		srccx -= cx;

		gdisp_lld_stream_start(GC);
		for(GC->p.y = y; GC->p.y < srcy; GC->p.y++, buffer += srccx) {
			for(GC->p.x = x; GC->p.x < srcx; GC->p.x++) {
				GC->p.color = *buffer++;
				gdisp_lld_stream_color(GC);
			}
		}
		#if GDISP_HARDWARE_STREAM_STOP
			gdisp_lld_stream_stop(GC);
		#endif
	#else
		// Worst is drawing pixels

		// Translate buffer to the real image data, use srcx,srcy as the end point, srccx as the buffer line gap
		buffer += srcy*srccx+srcx;
		srcx = x + cx;
		srcy = y + cy;
		srccx -= cx;

		for(GC->p.y = y; GC->p.y < srcy; GC->p.y++, buffer += srccx) {
			for(GC->p.x=x; GC->p.x < srcx; GC->p.x++) {
				GC->p.color = *buffer++;
				gdisp_lld_draw_pixel(GC);
			}
		}
	#endif
	MUTEX_EXIT();
}
	
#if GDISP_NEED_CLIP
	void gdispSetClip(coord_t x, coord_t y, coord_t cx, coord_t cy) {
		MUTEX_ENTER();
		#if GDISP_HARDWARE_CLIP
			// Best is using hardware clipping
			GC->p.x = x;
			GC->p.y = y;
			GC->p.cx = cx;
			GC->p.cy = cy;
			gdisp_lld_set_clip(GC);
		#else
			// Worst is using software clipping
			if (x < 0) { cx += x; x = 0; }
			if (y < 0) { cy += y; y = 0; }
			if (cx <= 0 || cy <= 0 || x >= GC->g.Width || y >= GC->g.Height) { MUTEX_EXIT(); return; }
			GC->clipx0 = x;
			GC->clipy0 = y;
			GC->clipx1 = x+cx;	if (GC->clipx1 > GC->g.Width) GC->clipx1 = GC->g.Width;
			GC->clipy1 = y+cy;	if (GC->clipy1 > GC->g.Height) GC->clipy1 = GC->g.Height;
		#endif
		MUTEX_EXIT();
	}
#endif

#if GDISP_NEED_CIRCLE
	void gdispDrawCircle(coord_t x, coord_t y, coord_t radius, color_t color) {
		coord_t a, b, P;

		MUTEX_ENTER();

		// Calculate intermediates
		a = 1;
		b = radius;
		P = 4 - radius;
		GC->p.color = color;

		// Away we go using Bresenham's circle algorithm
		// Optimized to prevent double drawing
		GC->p.x = x; GC->p.y = y + b; drawpixel_clip();
		GC->p.x = x; GC->p.y = y - b; drawpixel_clip();
		GC->p.x = x + b; GC->p.y = y; drawpixel_clip();
		GC->p.x = x - b; GC->p.y = y; drawpixel_clip();
		do {
			GC->p.x = x + a; GC->p.y = y + b; drawpixel_clip();
			GC->p.x = x + a; GC->p.y = y - b; drawpixel_clip();
			GC->p.x = x + b; GC->p.y = y + a; drawpixel_clip();
			GC->p.x = x - b; GC->p.y = y + a; drawpixel_clip();
			GC->p.x = x - a; GC->p.y = y + b; drawpixel_clip();
			GC->p.x = x - a; GC->p.y = y - b; drawpixel_clip();
			GC->p.x = x + b; GC->p.y = y - a; drawpixel_clip();
			GC->p.x = x - b; GC->p.y = y - a; drawpixel_clip();
			if (P < 0)
				P += 3 + 2*a++;
			else
				P += 5 + 2*(a++ - b--);
		} while(a < b);
		GC->p.x = x + a; GC->p.y = y + b; drawpixel_clip();
		GC->p.x = x + a; GC->p.y = y - b; drawpixel_clip();
		GC->p.x = x - a; GC->p.y = y + b; drawpixel_clip();
		GC->p.x = x - a; GC->p.y = y - b; drawpixel_clip();
		MUTEX_EXIT();
	}
#endif

#if GDISP_NEED_CIRCLE
	void gdispFillCircle(coord_t x, coord_t y, coord_t radius, color_t color) {
		coord_t a, b, P;

		MUTEX_ENTER();

		// Calculate intermediates
		a = 1;
		b = radius;
		P = 4 - radius;
		GC->p.color = color;

		// Away we go using Bresenham's circle algorithm
		// This is optimized to prevent overdrawing by drawing a line only when a variable is about to change value
		GC->p.y = y; GC->p.x = x-b; GC->p.x1 = x+b; hline_clip();
		GC->p.y = y+b; GC->p.x = x; drawpixel_clip();
		GC->p.y = y-b; GC->p.x = x; drawpixel_clip();
		do {
			GC->p.y = y+a; GC->p.x = x-b; GC->p.x1 = x+b; hline_clip();
			GC->p.y = y-a; GC->p.x = x-b; GC->p.x1 = x+b; hline_clip();
			if (P < 0) {
				P += 3 + 2*a++;
			} else {
				GC->p.y = y+b; GC->p.x = x-a; GC->p.x1 = x+a; hline_clip();
				GC->p.y = y-b; GC->p.x = x-a; GC->p.x1 = x+a; hline_clip();
				P += 5 + 2*(a++ - b--);
			}
		} while(a < b);
		GC->p.y = y+a; GC->p.x = x-b; GC->p.x1 = x+b; hline_clip();
		GC->p.y = y-a; GC->p.x = x-b; GC->p.x1 = x+b; hline_clip();
		MUTEX_EXIT();
	}
#endif

#if GDISP_NEED_ELLIPSE
	void gdispDrawEllipse(coord_t x, coord_t y, coord_t a, coord_t b, color_t color) {
		coord_t	dx, dy;
		int32_t	a2, b2;
		int32_t	err, e2;

		MUTEX_ENTER();

		// Calculate intermediates
		dx = 0;
		dy = b;
		a2 = a*a;
		b2 = b*b;
		err = b2-(2*b-1)*a2;
		GC->p.color = color;

		// Away we go using Bresenham's ellipse algorithm
		do {
			GC->p.x = x + dx; GC->p.y = y + dy; drawpixel_clip();
			GC->p.x = x - dx; GC->p.y = y + dy; drawpixel_clip();
			GC->p.x = x - dx; GC->p.y = y - dy; drawpixel_clip();
			GC->p.x = x + dx; GC->p.y = y - dy; drawpixel_clip();

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
		MUTEX_EXIT();
	}
#endif
	
#if GDISP_NEED_ELLIPSE
	void gdispFillEllipse(coord_t x, coord_t y, coord_t a, coord_t b, color_t color) {
		coord_t	dx, dy;
		int32_t	a2, b2;
		int32_t	err, e2;

		MUTEX_ENTER();

		// Calculate intermediates
		dx = 0;
		dy = b;
		a2 = a*a;
		b2 = b*b;
		err = b2-(2*b-1)*a2;
		GC->p.color = color;

		// Away we go using Bresenham's ellipse algorithm
		// This is optimized to prevent overdrawing by drawing a line only when a y is about to change value
		do {
			e2 = 2*err;
			if(e2 <  (2*dx+1)*b2) {
				dx++;
				err += (2*dx+1)*b2;
			}
			if(e2 > -(2*dy-1)*a2) {
				GC->p.y = y + dy; GC->p.x = x - dx; GC->p.x1 = x + dx; hline_clip();
				if (y) { GC->p.y = y - dy; GC->p.x = x - dx; GC->p.x1 = x + dx; hline_clip(); }
				dy--;
				err -= (2*dy-1)*a2;
			}
		} while(dy >= 0);
		MUTEX_EXIT();
	}
#endif

#if GDISP_NEED_ARC
	#if !GMISC_NEED_FIXEDTRIG && !GMISC_NEED_FASTTRIG
		#include <math.h>
	#endif

	void gdispDrawArc(coord_t x, coord_t y, coord_t radius, coord_t start, coord_t end, color_t color) {
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

		MUTEX_ENTER();
		GC->p.color = color;

		if (full) {
			// Draw full sectors
			// Optimized to prevent double drawing
			a = 1;
			b = radius;
			P = 4 - radius;
			if (full & 0x60) { GC->p.y = y+b; GC->p.x = x; drawpixel_clip(); }
			if (full & 0x06) { GC->p.y = y-b; GC->p.x = x; drawpixel_clip(); }
			if (full & 0x81) { GC->p.y = y; GC->p.x = x+b; drawpixel_clip(); }
			if (full & 0x18) { GC->p.y = y; GC->p.x = x-b; drawpixel_clip(); }
			do {
				if (full & 0x01) { GC->p.x = x+b; GC->p.y = y-a; drawpixel_clip(); }
				if (full & 0x02) { GC->p.x = x+a; GC->p.y = y-b; drawpixel_clip(); }
				if (full & 0x04) { GC->p.x = x-a; GC->p.y = y-b; drawpixel_clip(); }
				if (full & 0x08) { GC->p.x = x-b; GC->p.y = y-a; drawpixel_clip(); }
				if (full & 0x10) { GC->p.x = x-b; GC->p.y = y+a; drawpixel_clip(); }
				if (full & 0x20) { GC->p.x = x-a; GC->p.y = y+b; drawpixel_clip(); }
				if (full & 0x40) { GC->p.x = x+a; GC->p.y = y+b; drawpixel_clip(); }
				if (full & 0x80) { GC->p.x = x+b; GC->p.y = y+a; drawpixel_clip(); }
				if (P < 0)
					P += 3 + 2*a++;
				else
					P += 5 + 2*(a++ - b--);
			} while(a < b);
			if (full & 0xC0) { GC->p.x = x+a; GC->p.y = y+b; drawpixel_clip(); }
			if (full & 0x0C) { GC->p.x = x-a; GC->p.y = y-b; drawpixel_clip(); }
			if (full & 0x03) { GC->p.x = x+a; GC->p.y = y-b; drawpixel_clip(); }
			if (full & 0x30) { GC->p.x = x-a; GC->p.y = y+b; drawpixel_clip(); }
			if (full == 0xFF)
				return;
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
			if ((sbit & 0x20) || (ebit & 0x40)) { GC->p.x = x; GC->p.y = y+b; drawpixel_clip(); }
			if ((sbit & 0x02) || (ebit & 0x04)) { GC->p.x = x; GC->p.y = y-b; drawpixel_clip(); }
			if ((sbit & 0x80) || (ebit & 0x01)) { GC->p.x = x+b; GC->p.y = y; drawpixel_clip(); }
			if ((sbit & 0x08) || (ebit & 0x10)) { GC->p.x = x-b; GC->p.y = y; drawpixel_clip(); }
			do {
				if (((sbit & 0x01) && a >= sedge) || ((ebit & 0x01) && a <= eedge)) { GC->p.x = x+b; GC->p.y = y-a; drawpixel_clip(); }
				if (((sbit & 0x02) && a <= sedge) || ((ebit & 0x02) && a >= eedge)) { GC->p.x = x+a; GC->p.y = y-b; drawpixel_clip(); }
				if (((sbit & 0x04) && a >= sedge) || ((ebit & 0x04) && a <= eedge)) { GC->p.x = x-a; GC->p.y = y-b; drawpixel_clip(); }
				if (((sbit & 0x08) && a <= sedge) || ((ebit & 0x08) && a >= eedge)) { GC->p.x = x-b; GC->p.y = y-a; drawpixel_clip(); }
				if (((sbit & 0x10) && a >= sedge) || ((ebit & 0x10) && a <= eedge)) { GC->p.x = x-b; GC->p.y = y+a; drawpixel_clip(); }
				if (((sbit & 0x20) && a <= sedge) || ((ebit & 0x20) && a >= eedge)) { GC->p.x = x-a; GC->p.y = y+b; drawpixel_clip(); }
				if (((sbit & 0x40) && a >= sedge) || ((ebit & 0x40) && a <= eedge)) { GC->p.x = x+a; GC->p.y = y+b; drawpixel_clip(); }
				if (((sbit & 0x80) && a <= sedge) || ((ebit & 0x80) && a >= eedge)) { GC->p.x = x+b; GC->p.y = y+a; drawpixel_clip(); }
				if (P < 0)
					P += 3 + 2*a++;
				else
					P += 5 + 2*(a++ - b--);
			} while(a < b);
			if (((sbit & 0x40) && a >= sedge) || ((ebit & 0x40) && a <= eedge) || ((sbit & 0x80) && a <= sedge) || ((ebit & 0x80) && a >= eedge))
				{ GC->p.x = x+a; GC->p.y = y+b; drawpixel_clip(); }
			if (((sbit & 0x04) && a >= sedge) || ((ebit & 0x04) && a <= eedge) || ((sbit & 0x08) && a <= sedge) || ((ebit & 0x08) && a >= eedge))
				{ GC->p.x = x-a; GC->p.y = y-b; drawpixel_clip(); }
			if (((sbit & 0x01) && a >= sedge) || ((ebit & 0x01) && a <= eedge) || ((sbit & 0x02) && a <= sedge) || ((ebit & 0x02) && a >= eedge))
				{ GC->p.x = x+a; GC->p.y = y-b; drawpixel_clip(); }
			if (((sbit & 0x10) && a >= sedge) || ((ebit & 0x10) && a <= eedge) || ((sbit & 0x20) && a <= sedge) || ((ebit & 0x20) && a >= eedge))
				{ GC->p.x = x-a; GC->p.y = y+b; drawpixel_clip(); }
		} else if (end < start) {
			// Draw start/end sector where it is a non-internal angle
			// Optimized to prevent double drawing
			a = 1;
			b = radius;
			P = 4 - radius;
			if (sbit & 0x60) { GC->p.x = x; GC->p.y = y+b; drawpixel_clip(); }
			if (sbit & 0x06) { GC->p.x = x; GC->p.y = y-b; drawpixel_clip(); }
			if (sbit & 0x81) { GC->p.x = x+b; GC->p.y = y; drawpixel_clip(); }
			if (sbit & 0x18) { GC->p.x = x-b; GC->p.y = y; drawpixel_clip(); }
			do {
				if ((sbit & 0x01) && (a >= sedge || a <= eedge)) { GC->p.x = x+b; GC->p.y = y-a; drawpixel_clip(); }
				if ((sbit & 0x02) && (a <= sedge || a >= eedge)) { GC->p.x = x+a; GC->p.y = y-b; drawpixel_clip(); }
				if ((sbit & 0x04) && (a >= sedge || a <= eedge)) { GC->p.x = x-a; GC->p.y = y-b; drawpixel_clip(); }
				if ((sbit & 0x08) && (a <= sedge || a >= eedge)) { GC->p.x = x-b; GC->p.y = y-a; drawpixel_clip(); }
				if ((sbit & 0x10) && (a >= sedge || a <= eedge)) { GC->p.x = x-b; GC->p.y = y+a; drawpixel_clip(); }
				if ((sbit & 0x20) && (a <= sedge || a >= eedge)) { GC->p.x = x-a; GC->p.y = y+b; drawpixel_clip(); }
				if ((sbit & 0x40) && (a >= sedge || a <= eedge)) { GC->p.x = x+a; GC->p.y = y+b; drawpixel_clip(); }
				if ((sbit & 0x80) && (a <= sedge || a >= eedge)) { GC->p.x = x+b; GC->p.y = y+a; drawpixel_clip(); }
				if (P < 0)
					P += 3 + 2*a++;
				else
					P += 5 + 2*(a++ - b--);
			} while(a < b);
			if (((sbit & 0x04) && (a >= sedge || a <= eedge)) || ((sbit & 0x08) && (a <= sedge || a >= eedge)))
				{ GC->p.x = x-a; GC->p.y = y-b; drawpixel_clip(); }
			if (((sbit & 0x40) && (a >= sedge || a <= eedge)) || ((sbit & 0x80) && (a <= sedge || a >= eedge)))
				{ GC->p.x = x+a; GC->p.y = y+b; drawpixel_clip(); }
			if (((sbit & 0x01) && (a >= sedge || a <= eedge)) || ((sbit & 0x02) && (a <= sedge || a >= eedge)))
				{ GC->p.x = x+a; GC->p.y = y-b; drawpixel_clip(); }
			if (((sbit & 0x10) && (a >= sedge || a <= eedge)) || ((sbit & 0x20) && (a <= sedge || a >= eedge)))
				{ GC->p.x = x-a; GC->p.y = y+b; drawpixel_clip(); }
		} else {
			// Draw start/end sector where it is a internal angle
			// Optimized to prevent double drawing
			a = 1;
			b = radius;
			P = 4 - radius;
			if (((sbit & 0x20) && !eedge) || ((sbit & 0x40) && !sedge)) { GC->p.x = x; GC->p.y = y+b; drawpixel_clip(); }
			if (((sbit & 0x02) && !eedge) || ((sbit & 0x04) && !sedge)) { GC->p.x = x; GC->p.y = y-b; drawpixel_clip(); }
			if (((sbit & 0x80) && !eedge) || ((sbit & 0x01) && !sedge)) { GC->p.x = x+b; GC->p.y = y; drawpixel_clip(); }
			if (((sbit & 0x08) && !eedge) || ((sbit & 0x10) && !sedge)) { GC->p.x = x-b; GC->p.y = y; drawpixel_clip(); }
			do {
				if (((sbit & 0x01) && a >= sedge && a <= eedge)) { GC->p.x = x+b; GC->p.y = y-a; drawpixel_clip(); }
				if (((sbit & 0x02) && a <= sedge && a >= eedge)) { GC->p.x = x+a; GC->p.y = y-b; drawpixel_clip(); }
				if (((sbit & 0x04) && a >= sedge && a <= eedge)) { GC->p.x = x-a; GC->p.y = y-b; drawpixel_clip(); }
				if (((sbit & 0x08) && a <= sedge && a >= eedge)) { GC->p.x = x-b; GC->p.y = y-a; drawpixel_clip(); }
				if (((sbit & 0x10) && a >= sedge && a <= eedge)) { GC->p.x = x-b; GC->p.y = y+a; drawpixel_clip(); }
				if (((sbit & 0x20) && a <= sedge && a >= eedge)) { GC->p.x = x-a; GC->p.y = y+b; drawpixel_clip(); }
				if (((sbit & 0x40) && a >= sedge && a <= eedge)) { GC->p.x = x+a; GC->p.y = y+b; drawpixel_clip(); }
				if (((sbit & 0x80) && a <= sedge && a >= eedge)) { GC->p.x = x+b; GC->p.y = y+a; drawpixel_clip(); }
				if (P < 0)
					P += 3 + 2*a++;
				else
					P += 5 + 2*(a++ - b--);
			} while(a < b);
			if (((sbit & 0x04) && a >= sedge && a <= eedge) || ((sbit & 0x08) && a <= sedge && a >= eedge))
				{ GC->p.x = x-a; GC->p.y = y-b; drawpixel_clip(); }
			if (((sbit & 0x40) && a >= sedge && a <= eedge) || ((sbit & 0x80) && a <= sedge && a >= eedge))
				{ GC->p.x = x+a; GC->p.y = y+b; drawpixel_clip(); }
			if (((sbit & 0x01) && a >= sedge && a <= eedge) || ((sbit & 0x02) && a <= sedge && a >= eedge))
				{ GC->p.x = x+a; GC->p.y = y-b; drawpixel_clip(); }
			if (((sbit & 0x10) && a >= sedge && a <= eedge) || ((sbit & 0x20) && a <= sedge && a >= eedge))
				{ GC->p.x = x-a; GC->p.y = y+b; drawpixel_clip(); }
		}
		MUTEX_EXIT();
	}
#endif

#if GDISP_NEED_ARC
	void gdispFillArc(coord_t x, coord_t y, coord_t radius, coord_t start, coord_t end, color_t color) {
		coord_t a, b, P;
		coord_t	sy, ey;
		fixed	sxa, sxb, sxd, exa, exb, exd;
		uint8_t	qtr;

		MUTEX_ENTER();

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
		GC->p.color = color;
		sxb += sxa;
		exb += exa;

		// Away we go using Bresenham's circle algorithm
		// This is optimized to prevent overdrawing by drawing a line only when a variable is about to change value

		switch(qtr) {
		case 0:		// S2E2 sy <= ey
		case 1:		// S1E2 sy <= ey
			if (ey && sy) {
				GC->p.x = x; GC->p.x1 = x;									// E2S
				sxa -= sxd; exa -= exd;
			} else if (sy) {
				GC->p.x = x-b; GC->p.x1 = x;								// C2S
				sxa -= sxd;
			} else if (ey) {
				GC->p.x = x; GC->p.x1 = x+b;								// E2C
				exa -= exd;
			} else {
				GC->p.x = x-b; GC->p.x1 = x+b;								// C2C
			}
			GC->p.y = y;
			hline_clip();
			do {
				if (-a >= ey) {
					GC->p.y = y-a; GC->p.x = NONFIXED(exa); GC->p.x1 = NONFIXED(sxa); hline_clip();		// E2S
					sxa -= sxd; exa -= exd;
				} else if (-a >= sy) {
					GC->p.y = y-a; GC->p.x = x-b; GC->p.x1 = NONFIXED(sxa); hline_clip();				// C2S
					sxa -= sxd;
				} else if (qtr & 1) {
					GC->p.y = y-a; GC->p.x = x-b; GC->p.x1 = x+b; hline_clip();							// C2C
				}
				if (P < 0) {
					P += 3 + 2*a++;
				} else {
					if (-b >= ey) {
						GC->p.y = y-b; GC->p.x = NONFIXED(exb); GC->p.x1 = NONFIXED(sxb); hline_clip();	// E2S
						sxb += sxd; exb += exd;
					} else if (-b >= sy) {
						GC->p.y = y-b; GC->p.x = x-a; GC->p.x1 = NONFIXED(sxb); hline_clip();			// C2S
						sxb += sxd;
					} else if (qtr & 1) {
						GC->p.y = y-b; GC->p.x = x-a; GC->p.x1 = x+a; hline_clip();						// C2C
					}
					P += 5 + 2*(a++ - b--);
				}
			} while(a < b);
			if (-a >= ey) {
				GC->p.y = y-a; GC->p.x = NONFIXED(exa); GC->p.x1 = NONFIXED(sxa); hline_clip();			// E2S
			} else if (-a >= sy) {
				GC->p.y = y-a; GC->p.x = x-b; GC->p.x1 = NONFIXED(sxa); hline_clip();					// C2S
			} else if (qtr & 1) {
				GC->p.y = y-a; GC->p.x = x-b; GC->p.x1 = x+b; hline_clip();								// C2C
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
			GC->p.y = y; GC->p.x = x; GC->p.x1 = x+b; hline_clip();								// SE2C
			sxa += sxd; exa -= exd;
			do {
				if (-a >= ey) {
					GC->p.y = y-a; GC->p.x = NONFIXED(exa); GC->p.x1 = x+b; hline_clip();		// E2C
					exa -= exd;
				} else if (!(qtr & 4)) {
					GC->p.y = y-a; GC->p.x = x-b; GC->p.x1 = x+b; hline_clip();					// C2C
				}
				if (a <= sy) {
					GC->p.y = y+a; GC->p.x = NONFIXED(sxa); GC->p.x1 = x+b; hline_clip();		// S2C
					sxa += sxd;
				} else if (!(qtr & 1)) {
					GC->p.y = y+a; GC->p.x = x-b; GC->p.x1 = x+b; hline_clip();					// C2C
				}
				if (P < 0) {
					P += 3 + 2*a++;
				} else {
					if (-b >= ey) {
						GC->p.y = y-b; GC->p.x = NONFIXED(exb); GC->p.x1 = x+a; hline_clip();		// E2C
						exb += exd;
					} else if (!(qtr & 4)) {
						GC->p.y = y-b; GC->p.x = x-a; GC->p.x1 = x+a; hline_clip();					// C2C
					}
					if (b <= sy) {
						GC->p.y = y+b; GC->p.x = NONFIXED(sxb); GC->p.x1 = x+a; hline_clip();		// S2C
						sxb -= sxd;
					} else if (!(qtr & 1)) {
						GC->p.y = y+b; GC->p.x = x-a; GC->p.x1 = x+a; hline_clip(); 				// C2C
					}
					P += 5 + 2*(a++ - b--);
				}
			} while(a < b);
			if (-a >= ey) {
				GC->p.y = y-a; GC->p.x = NONFIXED(exa); GC->p.x1 = x+b; hline_clip();				// E2C
			} else if (!(qtr & 4)) {
				GC->p.y = y-a; GC->p.x = x-b; GC->p.x1 = x+b; hline_clip();							// C2C
			}
			if (a <= sy) {
				GC->p.y = y+a; GC->p.x = NONFIXED(sxa); GC->p.x1 = x+a; hline_clip();				// S2C
			} else if (!(qtr & 1)) {
				GC->p.y = y+a; GC->p.x = x-b; GC->p.x1 = x+a; hline_clip();							// C2C
			}
			break;

		case 4:		// S2E1 sy <= ey
		case 5:		// S1E1 sy <= ey
			GC->p.y = y; GC->p.x = x-b; GC->p.x1 = x+b; hline_clip();									// C2C
			do {
				if (-a >= ey) {
					GC->p.y = y-a; GC->p.x = x-b; GC->p.x1 = NONFIXED(sxa); hline_clip();				// C2S
					GC->p.y = y-a; GC->p.x = NONFIXED(exa); GC->p.x1 = x+b; hline_clip();				// E2C
					sxa -= sxd; exa -= exd;
				} else if (-a >= sy) {
					GC->p.y = y-a; GC->p.x = x-b; GC->p.x1 = NONFIXED(sxa); hline_clip();				// C2S
					sxa -= sxd;
				} else if (qtr & 1) {
					GC->p.y = y-a; GC->p.x = x-b; GC->p.x1 = x+b; hline_clip();							// C2C
				}
				GC->p.y = y+a; GC->p.x = x-b; GC->p.x1 = x+b; hline_clip();								// C2C
				if (P < 0) {
					P += 3 + 2*a++;
				} else {
					if (-b >= ey) {
						GC->p.y = y-b; GC->p.x = x-a; GC->p.x1 = NONFIXED(sxb); hline_clip();			// C2S
						GC->p.y = y-b; GC->p.x = NONFIXED(exb); GC->p.x1 = x+a; hline_clip();			// E2C
						sxb += sxd; exb += exd;
					} else if (-b >= sy) {
						GC->p.y = y-b; GC->p.x = x-a; GC->p.x1 = NONFIXED(sxb); hline_clip();			// C2S
						sxb += sxd;
					} else if (qtr & 1) {
						GC->p.y = y-b; GC->p.x = x-a; GC->p.x1 = x+a; hline_clip();						// C2C
					}
					GC->p.y = y+b; GC->p.x = x-a; GC->p.x1 = x+a; hline_clip();							// C2C
					P += 5 + 2*(a++ - b--);
				}
			} while(a < b);
			if (-a >= ey) {
				GC->p.y = y-a; GC->p.x = x-b; GC->p.x1 = NONFIXED(sxa); hline_clip();					// C2S
				GC->p.y = y-a; GC->p.x = NONFIXED(exa); GC->p.x1 = x+b; hline_clip();					// E2C
			} else if (-a >= sy) {
				GC->p.y = y-a; GC->p.x = x-b; GC->p.x1 = NONFIXED(sxa); hline_clip();					// C2S
			} else if (qtr & 1) {
				GC->p.y = y-a; GC->p.x = x-b; GC->p.x1 = x+b; hline_clip();								// C2C
			}
			GC->p.y = y+b; GC->p.x = x-a; GC->p.x1 = x+a; hline_clip();									// C2C
			break;

		case 8:		// S2E3 sy <= ey
		case 9:		// S1E3 sy <= ey
		case 12:	// S2E4 sy <= ey
		case 13:	// S1E4 sy <= ey
		case 24:	// S2E3 sy > ey
		case 25:	// S1E3 sy > ey
		case 28:	// S2E3 sy > ey
		case 29:	// S1E3 sy > ey
			GC->p.y = y; GC->p.x = x-b; GC->p.x1 = x; hline_clip();								// C2SE
			sxa -= sxd; exa += exd;
			do {
				if (-a >= sy) {
					GC->p.y = y-a; GC->p.x = x-b; GC->p.x1 = NONFIXED(sxa); hline_clip();		// C2S
					sxa -= sxd;
				} else if (qtr & 1) {
					GC->p.y = y-a; GC->p.x = x-b; GC->p.x1 = x+b; hline_clip();					// C2C
				}
				if (a <= ey) {
					GC->p.y = y+a; GC->p.x = x-b; GC->p.x1 = NONFIXED(exa); hline_clip();		// C2E
					exa += exd;
				} else if (qtr & 4) {
					GC->p.y = y+a; GC->p.x = x-b; GC->p.x1 = x+b; hline_clip();					// C2C
				}
				if (P < 0) {
					P += 3 + 2*a++;
				} else {
					if (-b >= sy) {
						GC->p.y = y-b; GC->p.x = x-a; GC->p.x1 = NONFIXED(sxb); hline_clip();		// C2S
						sxb += sxd;
					} else if (qtr & 1) {
						GC->p.y = y-b; GC->p.x = x-a; GC->p.x1 = x+a; hline_clip();					// C2C
					}
					if (b <= ey) {
						GC->p.y = y+b; GC->p.x = x-a; GC->p.x1 = NONFIXED(exb); hline_clip();		// C2E
						exb -= exd;
					} else if (qtr & 4) {
						GC->p.y = y+b; GC->p.x = x-a; GC->p.x1 = x+a; hline_clip(); 				// C2C
					}
					P += 5 + 2*(a++ - b--);
				}
			} while(a < b);
			if (-a >= sy) {
				GC->p.y = y-a; GC->p.x = x-b; GC->p.x1 = NONFIXED(sxa); hline_clip();				// C2S
			} else if (qtr & 1) {
				GC->p.y = y-a; GC->p.x = x-b; GC->p.x1 = x+b; hline_clip();							// C2C
			}
			if (a <= ey) {
				GC->p.y = y+a; GC->p.x = x-b; GC->p.x1 = NONFIXED(exa); hline_clip();				// C2E
			} else if (qtr & 4) {
				GC->p.y = y+a; GC->p.x = x-b; GC->p.x1 = x+a; hline_clip();							// C2C
			}
			break;

		case 10:	// S3E3 sy <= ey
		case 14:	// S3E4 sy <= ey
			GC->p.y = y; GC->p.x = x; drawpixel_clip();													// S2E
			sxa += sxd; exa += exd;
			do {
				if (a <= sy) {
					GC->p.y = y+a; GC->p.x = NONFIXED(sxa); GC->p.x1 = NONFIXED(exa); hline_clip();		// S2E
					sxa += sxd; exa += exd;
				} else if (a <= ey) {
					GC->p.y = y+a; GC->p.x = x-b; GC->p.x1 = NONFIXED(exa); hline_clip();				// C2E
					exa += exd;
				} else if (qtr & 4) {
					GC->p.y = y+a; GC->p.x = x-b; GC->p.x1 = x+b; hline_clip();							// C2C
				}
				if (P < 0) {
					P += 3 + 2*a++;
				} else {
					if (b <= sy) {
						GC->p.y = y+b; GC->p.x = NONFIXED(sxb); GC->p.x1 = NONFIXED(exb); hline_clip();		// S2E
						sxb -= sxd; exb -= exd;
					} else if (b <= ey) {
						GC->p.y = y+b; GC->p.x = x-a; GC->p.x1 = NONFIXED(exb); hline_clip();				// C2E
						exb -= exd;
					} else if (qtr & 4) {
						GC->p.y = y+b; GC->p.x = x-a; GC->p.x1 = x+a; hline_clip();							// C2C
					}
					P += 5 + 2*(a++ - b--);
				}
			} while(a < b);
			if (a <= sy) {
				GC->p.y = y+a; GC->p.x = NONFIXED(sxa); GC->p.x1 = NONFIXED(exa); hline_clip();		// S2E
			} else if (a <= ey) {
				GC->p.y = y+a; GC->p.x = x-b; GC->p.x1 = NONFIXED(exa); hline_clip();				// C2E
			} else if (qtr & 4) {
				GC->p.y = y+a; GC->p.x = x-b; GC->p.x1 = x+b; hline_clip();							// C2C
			}
			break;

		case 11:	// S4E3 sy <= ey
		case 15:	// S4E4 sy <= ey
			GC->p.y = y; GC->p.x = x-b; GC->p.x1 = x+b; hline_clip();									// C2C
			do {
				GC->p.y = y-a; GC->p.x = x-b; GC->p.x1 = x+b; hline_clip();								// C2C
				if (a <= sy) {
					GC->p.y = y+a; GC->p.x = x-b; GC->p.x1 = NONFIXED(exa); hline_clip();				// C2E
					GC->p.y = y+a; GC->p.x = NONFIXED(sxa); GC->p.x1 = x+b; hline_clip();				// S2C
					sxa += sxd; exa += exd;
				} else if (a <= ey) {
					GC->p.y = y+a; GC->p.x = x-b; GC->p.x1 = NONFIXED(exa); hline_clip();				// C2E
					exa += exd;
				} else if (qtr & 4) {
					GC->p.y = y+a; GC->p.x = x-b; GC->p.x1 = x+b; hline_clip();							// C2C
				}
				if (P < 0) {
					P += 3 + 2*a++;
				} else {
					GC->p.y = y-b; GC->p.x = x-a; GC->p.x1 = x+a; hline_clip();							// C2C
					if (b <= sy) {
						GC->p.y = y+b; GC->p.x = x-a; GC->p.x1 = NONFIXED(exb); hline_clip();			// C2E
						GC->p.y = y+b; GC->p.x = NONFIXED(sxb); GC->p.x1 = x+a; hline_clip();			// S2C
						sxb -= sxd; exb -= exd;
					} else if (b <= ey) {
						GC->p.y = y+b; GC->p.x = x-a; GC->p.x1 = NONFIXED(exb); hline_clip();			// C2E
						exb -= exd;
					} else if (qtr & 4) {
						GC->p.y = y+b; GC->p.x = x-a; GC->p.x1 = x+a; hline_clip();						// C2C
					}
					P += 5 + 2*(a++ - b--);
				}
			} while(a < b);
			GC->p.y = y-a; GC->p.x = x-b; GC->p.x1 = x+b; hline_clip();									// C2C
			if (a <= sy) {
				GC->p.y = y+a; GC->p.x = x-b; GC->p.x1 = NONFIXED(exa); hline_clip();					// C2E
				GC->p.y = y+a; GC->p.x = NONFIXED(sxa); GC->p.x1 = x+b; hline_clip();					// S2C
			} else if (a <= ey) {
				GC->p.y = y+a; GC->p.x = x-b; GC->p.x1 = NONFIXED(exa); hline_clip();					// C2E
			} else if (qtr & 4) {
				GC->p.y = y+a; GC->p.x = x-b; GC->p.x1 = x+b; hline_clip();								// C2C
			}
			break;

		case 16:	// S2E2	sy > ey
		case 20:	// S2E1 sy > ey
			GC->p.y = y; GC->p.x = x-b; GC->p.x1 = x+b; hline_clip();									// C2C
			sxa -= sxd; exa -= exd;
			do {
				if (-a >= sy) {
					GC->p.y = y-a; GC->p.x = x-b; GC->p.x1 = NONFIXED(sxa); hline_clip();				// C2S
					GC->p.y = y-a; GC->p.x = NONFIXED(exa); GC->p.x1 = x+b; hline_clip();				// E2C
					sxa -= sxd; exa -= exd;
				} else if (-a >= ey) {
					GC->p.y = y-a; GC->p.x = NONFIXED(exa); GC->p.x1 = x+b; hline_clip();				// E2C
					exa -= exd;
				} else if (!(qtr & 4)){
					GC->p.y = y-a; GC->p.x = x-b; GC->p.x1 = x+b; hline_clip(); 						// C2C
				}
				GC->p.y = y+a; GC->p.x = x-b; GC->p.x1 = x+b; hline_clip(); 							// C2C
				if (P < 0) {
					P += 3 + 2*a++;
				} else {
					if (-b >= sy) {
						GC->p.y = y-b; GC->p.x = x-a; GC->p.x1 = NONFIXED(sxb); hline_clip();			// C2S
						GC->p.y = y-b; GC->p.x = NONFIXED(exb); GC->p.x1 = x+a; hline_clip();			// E2C
						sxb += sxd; exb += exd;
					} else if (-b >= ey) {
						GC->p.y = y-b; GC->p.x = NONFIXED(exb); GC->p.x1 = x+a; hline_clip();			// E2C
						exb += exd;
					} else if (!(qtr & 4)){
						GC->p.y = y-b; GC->p.x = x-a; GC->p.x1 = x+a; hline_clip(); 					// C2C
					}
					GC->p.y = y+b; GC->p.x = x-a; GC->p.x1 = x+a; hline_clip(); 						// C2C
					P += 5 + 2*(a++ - b--);
				}
			} while(a < b);
			if (-a >= sy) {
				GC->p.y = y-a; GC->p.x = x-b; GC->p.x1 = NONFIXED(sxa); hline_clip();					// C2S
				GC->p.y = y-a; GC->p.x = NONFIXED(exa); GC->p.x1 = x+b; hline_clip();					// E2C
			} else if (-a >= ey) {
				GC->p.y = y-a; GC->p.x = NONFIXED(exa); GC->p.x1 = x+b; hline_clip();					// E2C
			} else if (!(qtr & 4)){
				GC->p.y = y-a; GC->p.x = x-b; GC->p.x1 = x+b; hline_clip(); 							// C2C
			}
			GC->p.y = y+a; GC->p.x = x-b; GC->p.x1 = x+b; hline_clip(); 								// C2C
			break;

		case 17:	// S1E2 sy > ey
		case 21:	// S1E1 sy > ey
			if (sy) {
				GC->p.x = x; GC->p.x1 = x;																// E2S
				sxa -= sxd; exa -= exd;
			} else {
				GC->p.x = x; GC->p.x1 = x+b;															// E2C
				exa -= exd;
			}
			GC->p.y = y;
			hline_clip();
			do {
				if (-a >= sy) {
					GC->p.y = y-a; GC->p.x = NONFIXED(exa); GC->p.x1 = NONFIXED(sxa); hline_clip();		// E2S
					sxa -= sxd; exa -= exd;
				} else if (-a >= ey) {
					GC->p.y = y-a; GC->p.x = NONFIXED(exa); GC->p.x1 = x+b; hline_clip();				// E2C
					exa -= exd;
				} else if (!(qtr & 4)) {
					GC->p.y = y-a; GC->p.x = x-b; GC->p.x1 = x+b; hline_clip();							// C2C
				}
				if (P < 0) {
					P += 3 + 2*a++;
				} else {
					if (-b >= sy) {
						GC->p.y = y-b; GC->p.x = NONFIXED(exb); GC->p.x1 = NONFIXED(sxb); hline_clip();	// E2S
						sxb += sxd; exb += exd;
					} else if (-b >= ey) {
						GC->p.y = y-b; GC->p.x = NONFIXED(exb); GC->p.x1 = x+a; hline_clip();			// E2C
						exb += exd;
					} else if (!(qtr & 4)) {
						GC->p.y = y-b; GC->p.x = x-a; GC->p.x1 = x+a; hline_clip();						// C2C
					}
					P += 5 + 2*(a++ - b--);
				}
			} while(a < b);
			if (-a >= sy) {
				GC->p.y = y-a; GC->p.x = NONFIXED(exa); GC->p.x1 = NONFIXED(sxa); hline_clip();			// E2S
			} else if (-a >= ey) {
				GC->p.y = y-a; GC->p.x = NONFIXED(exa); GC->p.x1 = x+b; hline_clip();					// E2C
			} else if (!(qtr & 4)) {
				GC->p.y = y-a; GC->p.x = x-b; GC->p.x1 = x+b; hline_clip();								// C2C
			}
			break;

		case 26:	// S3E3 sy > ey
		case 27:	// S4E3 sy > ey
			GC->p.y = y; GC->p.x = x-b; GC->p.x1 = x+b; hline_clip();									// C2C
			do {
				GC->p.y = y-a; GC->p.x = x-b; GC->p.x1 = x+b; hline_clip();								// C2C
				if (a <= ey) {
					GC->p.y = y+a; GC->p.x = x-b; GC->p.x1 = NONFIXED(exa); hline_clip();				// C2E
					GC->p.y = y+a; GC->p.x = NONFIXED(sxa); GC->p.x1 = x+b; hline_clip();				// S2C
					sxa += sxd; exa += exd;
				} else if (a <= sy) {
					GC->p.y = y+a; GC->p.x = NONFIXED(sxa); GC->p.x1 = x+b; hline_clip();				// S2C
					sxa += sxd;
				} else if (!(qtr & 1)) {
					GC->p.y = y+a; GC->p.x = x-b; GC->p.x1 = x+b; hline_clip();							// C2C
				}
				if (P < 0) {
					P += 3 + 2*a++;
				} else {
					GC->p.y = y-b; GC->p.x = x-a; GC->p.x1 = x+a; hline_clip();							// C2C
					if (b <= ey) {
						GC->p.y = y+b; GC->p.x = x-a; GC->p.x1 = NONFIXED(exb); hline_clip();			// C2E
						GC->p.y = y+b; GC->p.x = NONFIXED(sxb); GC->p.x1 = x+a; hline_clip();			// S2C
						sxb -= sxd; exb -= exd;
					} else if (b <= sy) {
						GC->p.y = y+b; GC->p.x = NONFIXED(sxb); GC->p.x1 = x+a; hline_clip();			// S2C
						sxb -= sxd;
					} else if (!(qtr & 1)) {
						GC->p.y = y+b; GC->p.x = x-a; GC->p.x1 = x+a; hline_clip();						// C2C
					}
					P += 5 + 2*(a++ - b--);
				}
			} while(a < b);
			GC->p.y = y-a; GC->p.x = x-b; GC->p.x1 = x+b; hline_clip();									// C2C
			if (a <= ey) {
				GC->p.y = y+a; GC->p.x = x-b; GC->p.x1 = NONFIXED(exa); hline_clip();					// C2E
				GC->p.y = y+a; GC->p.x = NONFIXED(sxa); GC->p.x1 = x+b; hline_clip();					// S2C
			} else if (a <= sy) {
				GC->p.y = y+a; GC->p.x = NONFIXED(sxa); GC->p.x1 = x+b; hline_clip();					// S2C
			} else if (!(qtr & 4)) {
				GC->p.y = y+a; GC->p.x = x-b; GC->p.x1 = x+b; hline_clip();								// C2C
			}
			break;

		case 30:	// S3E4 sy > ey
		case 31:	// S4E4 sy > ey
			do {
				if (a <= ey) {
					GC->p.y = y+a; GC->p.x = NONFIXED(sxa); GC->p.x1 = NONFIXED(exa); hline_clip();		// S2E
					sxa += sxd; exa += exd;
				} else if (a <= sy) {
					GC->p.y = y+a; GC->p.x = NONFIXED(sxa); GC->p.x1 = x+b; hline_clip();				// S2C
					sxa += sxd;
				} else if (!(qtr & 1)) {
					GC->p.y = y+a; GC->p.x = x-b; GC->p.x1 = x+b; hline_clip();							// C2C
				}
				if (P < 0) {
					P += 3 + 2*a++;
				} else {
					if (b <= ey) {
						GC->p.y = y+b; GC->p.x = NONFIXED(sxb); GC->p.x1 = NONFIXED(exb); hline_clip();	// S2E
						sxb -= sxd; exb -= exd;
					} else if (b <= sy) {
						GC->p.y = y+b; GC->p.x = NONFIXED(sxb); GC->p.x1 = x+a; hline_clip();			// S2C
						sxb -= sxd;
					} else if (!(qtr & 1)) {
						GC->p.y = y+b; GC->p.x = x-a; GC->p.x1 = x+a; hline_clip();						// C2C
					}
					P += 5 + 2*(a++ - b--);
				}
			} while(a < b);
			if (a <= ey) {
				GC->p.y = y+a; GC->p.x = NONFIXED(sxa); GC->p.x1 = x+b; hline_clip();				// S2C
			} else if (a <= sy) {
				GC->p.y = y+a; GC->p.x = NONFIXED(sxa); GC->p.x1 = x+b; hline_clip();					// S2C
			} else if (!(qtr & 4)) {
				GC->p.y = y+a; GC->p.x = x-b; GC->p.x1 = x+b; hline_clip();								// C2C
			}
			break;
		}

		MUTEX_EXIT();
	}

#endif

#if GDISP_NEED_ARC
	void gdispDrawRoundedBox(coord_t x, coord_t y, coord_t cx, coord_t cy, coord_t radius, color_t color) {
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
	void gdispFillRoundedBox(coord_t x, coord_t y, coord_t cx, coord_t cy, coord_t radius, color_t color) {
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
	color_t gdispGetPixelColor(coord_t x, coord_t y) {
		color_t		c;

		/* Always synchronous as it must return a value */
		MUTEX_ENTER();
		#if GDISP_HARDWARE_PIXELREAD
			// Best is direct pixel read
			GC->p.x = x;
			GC->p.y = y;
			c = gdisp_lld_get_pixel_color(GC);
		#elif GDISP_HARDWARE_STREAM && GDISP_HARDWARE_STREAM_READ
			// Next best is hardware streaming
			GC->p.x = x;
			GC->p.y = y;
			GC->p.cx = 1;
			GC->p.cy = 1;
			gdisp_lld_stream_start(GC);
			c = gdisp_lld_stream_read(GC);
			#if GDISP_HARDWARE_STREAM_STOP
				gdisp_lld_stream_stop(GC);
			#endif
		#else
			// Worst is "not possible"
			#error "GDISP: GDISP_NEED_PIXELREAD has been set but there is no hardware support for reading the display"
		#endif
		MUTEX_EXIT();

		return c;
	}
#endif

#if GDISP_NEED_SCROLL
	void gdispVerticalScroll(coord_t x, coord_t y, coord_t cx, coord_t cy, int lines, color_t bgcolor) {
		coord_t		abslines;
		#if !GDISP_HARDWARE_SCROLL
			coord_t 	fy, dy, ix, fx, i, j;
		#endif

		MUTEX_ENTER();
		#if NEED_CLIPPING
			if (x < GC->clipx0) { cx -= GC->clipx0 - x; x = GC->clipx0; }
			if (y < GC->clipy0) { cy -= GC->clipy0 - y; y = GC->clipy0; }
			if (!lines || cx <= 0 || cy <= 0 || x >= GC->clipx1 || y >= GC->clipy1) { MUTEX_EXIT(); return; }
			if (x+cx > GC->clipx1)	cx = GC->clipx1 - x;
			if (y+cy > GC->clipy1)	cy = GC->clipy1 - y;
		#endif

		abslines = lines < 0 ? -lines : lines;
		if (abslines >= cy) {
			abslines = cy;
			cy = 0;
		} else {
			#if GDISP_HARDWARE_SCROLL
				GC->p.x = x;
				GC->p.y = y;
				GC->p.cx = cx;
				GC->p.cy = cy;
				GC->p.y1 = lines;
				GC->p.color = bgcolor;
				gdisp_lld_vertical_scroll(GC);
				cy -= abslines;
			#elif GDISP_LINEBUF_SIZE == 0
				#error "GDISP: GDISP_NEED_SCROLL is set but there is no hardware support and GDISP_LINEBUF_SIZE is zero."
			#else
				cy -= abslines;
				if (lines < 0) {
					fy = y+cx-1;
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
						#if GDISP_HARDWARE_STREAM && GDISP_HARDWARE_STREAM_READ
							// Best is hardware streaming
							GC->p.x = x+ix;
							GC->p.y = fy+lines;
							GC->p.cx = fx;
							GC->p.cy = 1;
							gdisp_lld_stream_start(GC);
							for(j=0; j < fx; j++)
								GC->linebuf[j] = gdisp_lld_stream_read(GC);
							#if GDISP_HARDWARE_STREAM_STOP
								gdisp_lld_stream_stop(GC);
							#endif
						#elif GDISP_HARDWARE_PIXELREAD
							// Next best is single pixel reads
							for(j=0; j < fx; j++) {
								GC->p.x = x+ix+j;
								GC->p.y = fy+lines;
								GC->linebuf[j] = gdisp_lld_get_pixel_color(GC);
							}
						#else
							// Worst is "not possible"
							#error "GDISP: GDISP_NEED_SCROLL is set but there is no hardware support for scrolling or reading pixels."
						#endif

						// Write that line to the new location
						#if GDISP_HARDWARE_BITFILLS
							// Best is hardware bitfills
							GC->p.x = x+ix;
							GC->p.y = fy;
							GC->p.cx = fx;
							GC->p.cy = 1;
							GC->p.x1 = 0;
							GC->p.y1 = 0;
							GC->p.x2 = fx;
							GC->p.ptr = (void *)GC->linebuf;
							gdisp_lld_blit_area(GC);
						#elif GDISP_HARDWARE_STREAM
							// Next best is hardware streaming
							GC->p.x = x+ix;
							GC->p.y = fy;
							GC->p.cx = fx;
							GC->p.cy = 1;
							gdisp_lld_stream_start(GC);
							for(j = 0; j < fx; j++) {
								GC->p.color = GC->linebuf[j];
								gdisp_lld_stream_color(GC);
							}
							#if GDISP_HARDWARE_STREAM_STOP
								gdisp_lld_stream_stop(GC);
							#endif
						#else
							// Worst is drawing pixels
							GC->p.y = fy;
							for(GC->p.x = x+ix, j = 0; j < fx; GC->p.x++, j++) {
								GC->p.color = GC->linebuf[j];
								gdisp_lld_draw_pixel(GC);
							}
						#endif
					}
				}
			#endif
		}

		/* fill the remaining gap */
		GC->p.x = x;
		GC->p.y = lines > 0 ? (y+cy) : y;
		GC->p.cx = cx;
		GC->p.cy = abslines;
		GC->p.color = bgcolor;
		fillarea();
		MUTEX_EXIT();
	}
#endif

#if GDISP_NEED_CONTROL
	#if GDISP_HARDWARE_CONTROL
		void gdispControl(unsigned what, void *value) {
			MUTEX_ENTER();
			GC->p.x = what;
			GC->p.ptr = value;
			gdisp_lld_control(GC);
			#if GDISP_NEED_CLIP || GDISP_NEED_VALIDATION
				if (what == GDISP_CONTROL_ORIENTATION) {
					#if GDISP_HARDWARE_CLIP
						// Best is hardware clipping
						GC->p.x = 0;
						GC->p.y = 0;
						GC->p.cx = GC->g.Width;
						GC->p.cy = GC->g.Height;
						gdisp_lld_set_clip(GC);
					#else
						// Worst is software clipping
						GC->clipx0 = 0;
						GC->clipy0 = 0;
						GC->clipx1 = GC->g.Width;
						GC->clipy1 = GC->g.Height;
					#endif
				}
			#endif
			MUTEX_EXIT();
		}
	#else
		void gdispControl(unsigned what, void *value) {
			(void)what;
			(void)value;
			/* Ignore everything */
		}
	#endif
#endif

#if GDISP_NEED_QUERY
	#if GDISP_HARDWARE_QUERY
		void *gdispQuery(unsigned what) {
			void *res;

			MUTEX_ENTER();
			GC->p.x = (coord_t)what;
			res = gdisp_lld_query(GC);
			MUTEX_EXIT();
			return res;
		}
	#else
		void *gdispQuery(unsigned what) {
			(void) what;
			return (void *)-1;
		}
	#endif
#endif

/*===========================================================================*/
/* High Level Driver Routines.                                               */
/*===========================================================================*/

void gdispDrawBox(coord_t x, coord_t y, coord_t cx, coord_t cy, color_t color) {
	if (cx <= 0 || cy <= 0) return;
	cx = x+cx-1; cy = y+cy-1;			// cx, cy are now the end point.

	MUTEX_ENTER();

	GC->p.color = color;

	if (cx - x > 2) {
		GC->p.x = x; GC->p.y = y; GC->p.x1 = cx; hline_clip();
		if (y != cy) {
			GC->p.x = x; GC->p.y = cy; GC->p.x1 = cx; hline_clip();
			if (cy - y > 2) {
				y++; cy--;
				GC->p.x = x; GC->p.y = y; GC->p.y1 = cy; vline_clip();
				GC->p.x = cx; GC->p.y = y; GC->p.y1 = cy; vline_clip();
			}
		}
	} else {
		GC->p.x = x; GC->p.y = y; GC->p.y1 = cy; vline_clip();
		if (x != cx) {
			GC->p.x = cx; GC->p.y = y; GC->p.y1 = cy; vline_clip();
		}
	}
	MUTEX_EXIT();
}

#if GDISP_NEED_CONVEX_POLYGON
	void gdispDrawPoly(coord_t tx, coord_t ty, const point *pntarray, unsigned cnt, color_t color) {
		const point	*epnt, *p;

		epnt = &pntarray[cnt-1];

		MUTEX_ENTER();
		GC->p.color = color;
		for(p = pntarray; p < epnt; p++) {
			GC->p.x=tx+p->x; GC->p.y=ty+p->y; GC->p.x1=tx+p[1].x; GC->p.y1=ty+p[1].y; line_clip();
		}
		GC->p.x=tx+p->x; GC->p.y=ty+p->y; GC->p.x1=tx+pntarray->x; GC->p.y1=ty+pntarray->y; line_clip();
		MUTEX_EXIT();
	}

	void gdispFillConvexPoly(coord_t tx, coord_t ty, const point *pntarray, unsigned cnt, color_t color) {
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

		MUTEX_ENTER();
		GC->p.color = color;
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
					GC->p.x=tx+lxc; GC->p.y=ty+y; GC->p.x1=tx+rxc-1; hline_clip();
				} else if (lxc > rxc) {
					GC->p.x=tx+rxc; GC->p.y=ty+y; GC->p.x1=tx+lxc-1; hline_clip();
				}

				lx += lk;
				rx += rk;
			}

			if (!cnt) {
				MUTEX_EXIT();
				return;
			}
			cnt--;

			/* Replace the appropriate point */
			if (ymax == lpnt->y) {
				for (lpnt = lpnt <= pntarray ? epnts : lpnt-1; lpnt->y == y; cnt--) {
					if (!cnt) {
						MUTEX_EXIT();
						return;
					}
					lx = FIXED(lpnt->x);
					lpnt = lpnt <= pntarray ? epnts : lpnt-1;
				}
				lk = (FIXED(lpnt->x) - lx) / (lpnt->y - y);
			} else {
				for (rpnt = rpnt >= epnts ? pntarray : rpnt+1; rpnt->y == y; cnt--) {
					if (!cnt) {
						MUTEX_EXIT();
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
			#define GD	((GDISPDriver *)state)
			if (y < GD->t.clipy0 || y >= GD->t.clipy1 || x < GD->t.clipx0 || x+count > GD->t.clipx1) return;
			if (alpha == 255) {
				GD->p.x = x; GD->p.y = y; GD->p.x1 = x+count-1; GD->p.color = GD->t.color;
				hline_clip();
			} else {
				for (; count; count--, x++) {
					GD->p.x = x; GD->p.y = y;
					GD->p.color = gdispBlendColor(GD->t.color, gdisp_lld_get_pixel_color(GD), alpha);
					drawpixel_clip();
				}
			}
			#undef GD
		}
	#else
		static void drawcharline(int16_t x, int16_t y, uint8_t count, uint8_t alpha, void *state) {
			#define GD	((GDISPDriver *)state)
			if (y < GD->t.clipy0 || y >= GD->t.clipy1 || x < GD->t.clipx0 || x+count > GD->t.clipx1) return;
			if (alpha > 0x80) {			// A best approximation when using anti-aliased fonts but we can't actually draw them anti-aliased
				GD->p.x = x; GD->p.y = y; GD->p.x1 = x+count-1; GD->p.color = GD->t.color;
				hline_clip();
			}
			#undef GD
		}
	#endif

	#if GDISP_NEED_ANTIALIAS
		static void fillcharline(int16_t x, int16_t y, uint8_t count, uint8_t alpha, void *state) {
			#define GD	((GDISPDriver *)state)
			if (y < GD->t.clipy0 || y >= GD->t.clipy1 || x < GD->t.clipx0 || x+count > GD->t.clipx1) return;
			if (alpha == 255) {
				GD->p.color = GD->t.color;
			} else {
				GD->p.color = gdispBlendColor(GD->t.color, GD->t.bgcolor, alpha);
			}
			GD->p.x = x; GD->p.y = y; GD->p.x1 = x+count-1;
			hline_clip();
			#undef GD
		}
	#else
		#define fillcharline	drawcharline
	#endif

	/* Callback to render characters. */
	static uint8_t drawcharglyph(int16_t x, int16_t y, mf_char ch, void *state) {
		return mf_render_character(GC->t.font, x, y, ch, drawcharline, state);
	}

	/* Callback to render characters. */
	static uint8_t fillcharglyph(int16_t x, int16_t y, mf_char ch, void *state) {
		return mf_render_character(GC->t.font, x, y, ch, fillcharline, state);
	}

	void gdispDrawChar(coord_t x, coord_t y, uint16_t c, font_t font, color_t color) {
		MUTEX_ENTER();
		GC->t.font = font;
		GC->t.clipx0 = x;
		GC->t.clipy0 = y;
		GC->t.clipx1 = x + mf_character_width(font, c) + font->baseline_x;
		GC->t.clipy1 = y + font->height;
		GC->t.color = color;
		mf_render_character(font, x, y, c, drawcharline, GC);
		MUTEX_EXIT();
	}

	void gdispFillChar(coord_t x, coord_t y, uint16_t c, font_t font, color_t color, color_t bgcolor) {
		MUTEX_ENTER();
		GC->p.cx = mf_character_width(font, c) + font->baseline_x;
		GC->p.cy = font->height;
		GC->t.font = font;
		GC->t.clipx0 = GC->p.x = x;
		GC->t.clipy0 = GC->p.y = y;
		GC->t.clipx1 = GC->p.x+GC->p.cx;
		GC->t.clipy1 = GC->p.y+GC->p.cy;
		GC->t.color = color;
		GC->t.bgcolor = GC->p.color = bgcolor;

		TEST_CLIP_AREA(GC->p.x, GC->p.y, GC->p.cx, GC->p.cy) {
			fillarea();
			mf_render_character(font, x, y, c, fillcharline, GC);
		}
		MUTEX_EXIT();
	}

	void gdispDrawString(coord_t x, coord_t y, const char *str, font_t font, color_t color) {
		MUTEX_ENTER();
		GC->t.font = font;
		GC->t.clipx0 = x;
		GC->t.clipy0 = y;
		GC->t.clipx1 = x + mf_get_string_width(font, str, 0, 0);
		GC->t.clipy1 = y + font->height;
		GC->t.color = color;

		mf_render_aligned(font, x+font->baseline_x, y, MF_ALIGN_LEFT, str, 0, drawcharglyph, GC);
		MUTEX_EXIT();
	}

	void gdispFillString(coord_t x, coord_t y, const char *str, font_t font, color_t color, color_t bgcolor) {
		MUTEX_ENTER();
		GC->p.cx = mf_get_string_width(font, str, 0, 0);
		GC->p.cy = font->height;
		GC->t.font = font;
		GC->t.clipx0 = GC->p.x = x;
		GC->t.clipy0 = GC->p.y = y;
		GC->t.clipx1 = GC->p.x+GC->p.cx;
		GC->t.clipy1 = GC->p.y+GC->p.cy;
		GC->t.color = color;
		GC->t.bgcolor = GC->p.color = bgcolor;

		TEST_CLIP_AREA(GC->p.x, GC->p.y, GC->p.cx, GC->p.cy) {
			fillarea();
			mf_render_aligned(font, x+font->baseline_x, y, MF_ALIGN_LEFT, str, 0, fillcharglyph, GC);
		}
		MUTEX_EXIT();
	}

	void gdispDrawStringBox(coord_t x, coord_t y, coord_t cx, coord_t cy, const char* str, font_t font, color_t color, justify_t justify) {
		MUTEX_ENTER();
		GC->t.font = font;
		GC->t.clipx0 = x;
		GC->t.clipy0 = y;
		GC->t.clipx1 = x+cx;
		GC->t.clipy1 = y+cy;
		GC->t.color = color;

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

		mf_render_aligned(font, x, y, justify, str, 0, drawcharglyph, GC);
		MUTEX_EXIT();
	}

	void gdispFillStringBox(coord_t x, coord_t y, coord_t cx, coord_t cy, const char* str, font_t font, color_t color, color_t bgcolor, justify_t justify) {
		MUTEX_ENTER();
		GC->p.cx = cx;
		GC->p.cy = cy;
		GC->t.font = font;
		GC->t.clipx0 = GC->p.x = x;
		GC->t.clipy0 = GC->p.y = y;
		GC->t.clipx1 = x+cx;
		GC->t.clipy1 = y+cy;
		GC->t.color = color;
		GC->t.bgcolor = GC->p.color = bgcolor;

		TEST_CLIP_AREA(GC->p.x, GC->p.y, GC->p.cx, GC->p.cy) {

			// background fill
			fillarea();

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
			mf_render_aligned(font, x, y, justify, str, 0, fillcharglyph, GC);
		}
		MUTEX_EXIT();
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
