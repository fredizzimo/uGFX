/*
 * This file is subject to the terms of the GFX License. If a copy of
 * the license was not distributed with this file, you can obtain one at:
 *
 *              http://ugfx.org/license.html
 */

#include "gfx.h"

#if GFX_USE_GDISP

#define GDISP_DRIVER_VMT			GDISPVMT_SSD1306
#include "drivers/gdisp/SSD1306/gdisp_lld_config.h"
#include "src/gdisp/driver.h"

#include "board_SSD1306.h"
#include <string.h>   // for memset

/*===========================================================================*/
/* Driver local definitions.                                                 */
/*===========================================================================*/

#ifndef GDISP_SCREEN_HEIGHT
	#define GDISP_SCREEN_HEIGHT		64		// This controller should support 32 (untested) or 64
#endif
#ifndef GDISP_SCREEN_WIDTH
	#define GDISP_SCREEN_WIDTH		128
#endif
#ifndef GDISP_INITIAL_CONTRAST
	#define GDISP_INITIAL_CONTRAST	100
#endif
#ifndef GDISP_INITIAL_BACKLIGHT
	#define GDISP_INITIAL_BACKLIGHT	100
#endif
#ifdef SSD1306_PAGE_PREFIX
	#define SSD1306_PAGE_WIDTH		(GDISP_SCREEN_WIDTH+1)
	#define SSD1306_PAGE_OFFSET		1
#else
	#define SSD1306_PAGE_WIDTH		GDISP_SCREEN_WIDTH
	#define SSD1306_PAGE_OFFSET		0
#endif

#define GDISP_FLG_NEEDFLUSH			(GDISP_FLG_DRIVER<<0)

#include "drivers/gdisp/SSD1306/SSD1306.h"

/*===========================================================================*/
/* Driver local functions.                                                   */
/*===========================================================================*/

// Some common routines and macros
#define RAM(g)							((uint8_t *)g->priv)
#define write_cmd2(g, cmd1, cmd2)		{ write_cmd(g, cmd1); write_cmd(g, cmd2); }
#define write_cmd3(g, cmd1, cmd2, cmd3)	{ write_cmd(g, cmd1); write_cmd(g, cmd2); write_cmd(g, cmd3); }

// Some common routines and macros
#define delay(us)			gfxSleepMicroseconds(us)
#define delayms(ms)			gfxSleepMilliseconds(ms)

#define xyaddr(x, y)		(SSD1306_PAGE_OFFSET + (x) + ((y)>>3)*SSD1306_PAGE_WIDTH)
#define xybit(y)			(1<<((y)&7))

/*===========================================================================*/
/* Driver exported functions.                                                */
/*===========================================================================*/

/**
 * As this controller can't update on a pixel boundary we need to maintain the
 * the entire display surface in memory so that we can do the necessary bit
 * operations. Fortunately it is a small display in monochrome.
 * 64 * 128 / 8 = 1024 bytes.
 */

LLDSPEC bool_t gdisp_lld_init(GDisplay *g) {
	// The private area is the display surface.
	g->priv = gfxAlloc(GDISP_SCREEN_HEIGHT/8 * SSD1306_PAGE_WIDTH);

	// Fill in the prefix command byte on each page line of the display buffer
	// We can do it during initialisation as this byte is never overwritten.
	#ifdef SSD1306_PAGE_PREFIX
		{
			unsigned	i;

			for(i=0; i < GDISP_SCREEN_HEIGHT/8 * SSD1306_PAGE_WIDTH; i+=SSD1306_PAGE_WIDTH)
				RAM(g)[i] = SSD1306_PAGE_PREFIX;
		}
	#endif

	// Initialise the board interface
	init_board(g);

	// Hardware reset
	setpin_reset(g, TRUE);
	gfxSleepMilliseconds(20);
	setpin_reset(g, FALSE);
	gfxSleepMilliseconds(20);

	acquire_bus(g);

	write_cmd(g, SSD1306_DISPLAYOFF);
	write_cmd2(g, SSD1306_SETDISPLAYCLOCKDIV, 0x80);
	write_cmd2(g, SSD1306_SETMULTIPLEX, GDISP_SCREEN_HEIGHT-1);
	write_cmd2(g, SSD1306_SETPRECHARGE, 0x1F);
	write_cmd2(g, SSD1306_SETDISPLAYOFFSET, 0);
	write_cmd(g, SSD1306_SETSTARTLINE | 0);
	write_cmd2(g, SSD1306_ENABLE_CHARGE_PUMP, 0x14);
	write_cmd2(g, SSD1306_MEMORYMODE, 0);
	write_cmd(g, SSD1306_COLSCANDEC);
	write_cmd(g, SSD1306_ROWSCANDEC);
	#if GDISP_SCREEN_HEIGHT == 64
		write_cmd2(g, SSD1306_SETCOMPINS, 0x12);
	#else
		write_cmd2(g, SSD1306_SETCOMPINS, 0x22);
	#endif
	write_cmd2(g, SSD1306_SETCONTRAST, (uint8_t)(GDISP_INITIAL_CONTRAST*256/101));	// Set initial contrast.
	write_cmd2(g, SSD1306_SETVCOMDETECT, 0x10);
	write_cmd(g, SSD1306_DISPLAYON);
	write_cmd(g, SSD1306_NORMALDISPLAY);
	write_cmd3(g, SSD1306_HV_COLUMN_ADDRESS, 0, GDISP_SCREEN_WIDTH-1);
	write_cmd3(g, SSD1306_HV_PAGE_ADDRESS, 0, GDISP_SCREEN_HEIGHT/8-1);

    // Finish Init
    post_init_board(g);

 	// Release the bus
	release_bus(g);

	/* Initialise the GDISP structure */
	g->g.Width = GDISP_SCREEN_WIDTH;
	g->g.Height = GDISP_SCREEN_HEIGHT;
	g->g.Orientation = GDISP_ROTATE_0;
	g->g.Powermode = powerOn;
	g->g.Backlight = GDISP_INITIAL_BACKLIGHT;
	g->g.Contrast = GDISP_INITIAL_CONTRAST;
	return TRUE;
}

#if GDISP_HARDWARE_FLUSH
	LLDSPEC void gdisp_lld_flush(GDisplay *g) {
		uint8_t * ram;
		unsigned pages;

		// Don't flush if we don't need it.
		if (!(g->flags & GDISP_FLG_NEEDFLUSH))
			return;
		ram = RAM(g);
		pages = GDISP_SCREEN_HEIGHT/8;

		acquire_bus(g);
		write_cmd(g, SSD1306_SETSTARTLINE | 0);

		while (pages--) {
			write_data(g, ram, SSD1306_PAGE_WIDTH);
			ram += SSD1306_PAGE_WIDTH;
		}
		release_bus(g);
	}
#endif

#if GDISP_HARDWARE_CLEARS
	LLDSPEC void gdisp_lld_clear(GDisplay *g) {
		uint8_t fill = (gdispColor2Native(g->p.color) == Black) ? 0 : 0xff;
		int bytes = GDISP_SCREEN_WIDTH * GDISP_SCREEN_HEIGHT/8;
		memset(RAM(g), fill, bytes);
		g->flags |= GDISP_FLG_NEEDFLUSH;
	}
#endif

#if GDISP_HARDWARE_FILLS
	LLDSPEC void gdisp_lld_fill_area(GDisplay *g) {
		coord_t sy = g->p.y;
		coord_t ey = sy + g->p.cy - 1;
		coord_t sx = g->p.x;
		coord_t ex = g->p.x + g->p.cx - 1;
		if (g->g.Orientation == GDISP_ROTATE_90 || g->g.Orientation == GDISP_ROTATE_270) {
			coord_t tmp; 
			tmp = sx; sx = sy; sy = tmp;
			tmp = ex; ex = ey; ey = tmp;
		}
		unsigned spage = sy / 8; 
		uint8_t * base = RAM(g) + GDISP_SCREEN_WIDTH * spage;
		uint8_t mask = 0xff << (sy&7);
		unsigned zpages = (ey / 8) - spage;
		coord_t col;
		if (gdispColor2Native(g->p.color)==Black) {
			while (zpages--) {
				for (col = sx; col <= ex; col++)
					base[col] &= ~mask;
				mask = 0xff;
				base += GDISP_SCREEN_WIDTH;
			}
			mask &= (0xff >> (7 - (ey&7)));
			for (col = sx; col <= ex; col++)
				base[col] &= ~mask;
		} else {
			while (zpages--) {
				for (col = sx; col <= ex; col++)
					base[col] |= mask;
				mask = 0xff;
				base += GDISP_SCREEN_WIDTH;
			}
			mask &= (0xff >> (7 - (ey&7)));
			for (col = sx; col <= ex; col++)
				base[col] |= mask;
		}
	}
#endif

#if GDISP_HARDWARE_DRAWPIXEL
	LLDSPEC void gdisp_lld_draw_pixel(GDisplay *g) {
		coord_t x = g->p.x;
		coord_t y = g->p.y;
		if (g->g.Orientation == GDISP_ROTATE_90 || g->g.Orientation == GDISP_ROTATE_270) {
			coord_t tmp; 
			tmp = x; x = y; y = tmp;
		}
		if (gdispColor2Native(g->p.color) != Black)
			RAM(g)[xyaddr(x, y)] |= xybit(y);
		else
			RAM(g)[xyaddr(x, y)] &= ~xybit(y);
		g->flags |= GDISP_FLG_NEEDFLUSH;
	}
#endif

#if GDISP_HARDWARE_PIXELREAD
	LLDSPEC color_t gdisp_lld_get_pixel_color(GDisplay *g) {
		coord_t x = g->p.x;
		coord_t y = g->p.y;
		if (g->g.Orientation == GDISP_ROTATE_90 || g->g.Orientation == GDISP_ROTATE_270) {
			coord_t tmp; 
			tmp = x; x = y; y = tmp;
		}
		return (RAM(g)[xyaddr(x, y)] & xybit(y)) ? White : Black;
	}
#endif

#if GDISP_NEED_CONTROL && GDISP_HARDWARE_CONTROL
	LLDSPEC void gdisp_lld_control(GDisplay *g) {
		switch(g->p.x) {
		case GDISP_CONTROL_POWER:
			if (g->g.Powermode == (powermode_t)g->p.ptr)
				return;
			switch((powermode_t)g->p.ptr) {
			case powerOff:
			case powerSleep:
			case powerDeepSleep:
				acquire_bus(g);
				write_cmd(g, SSD1306_DISPLAYOFF);
				release_bus(g);
				break;
			case powerOn:
				acquire_bus(g);
				write_cmd(g, SSD1306_DISPLAYON);
				release_bus(g);
				break;
			default:
				return;
			}
			g->g.Powermode = (powermode_t)g->p.ptr;
			return;

		case GDISP_CONTROL_ORIENTATION:
			if (g->g.Orientation == (orientation_t)g->p.ptr)
				return;
			orientation_t orient = (orientation_t)g->p.ptr;
			switch(orient) {
			case GDISP_ROTATE_0:
			case GDISP_ROTATE_180:
				g->g.Height = GDISP_SCREEN_HEIGHT;
				g->g.Width = GDISP_SCREEN_WIDTH;
				break;
			case GDISP_ROTATE_90:
			case GDISP_ROTATE_270:
				g->g.Height = GDISP_SCREEN_WIDTH;
				g->g.Width = GDISP_SCREEN_HEIGHT;
				break;
			default:
				return;
			}
			// Remap the rows and columns according to orientation.  This just
			// eliminates the need to reverse x or y directions in the drawing
			// routines.  There is still the need to swap x and y for 90 and 270.
			// However, without these, the hardware fill routine would be much
			// more complicated. 
			acquire_bus(g);
			switch(orient) {
			default:
			case GDISP_ROTATE_0:
				write_cmd(g, SSD1306_COLSCANDEC);
				write_cmd(g, SSD1306_ROWSCANDEC);
				break;
			case GDISP_ROTATE_180:
				write_cmd(g, SSD1306_COLSCANINC);
				write_cmd(g, SSD1306_ROWSCANINC);
				break;
			case GDISP_ROTATE_90:
				write_cmd(g, SSD1306_COLSCANDEC);
				write_cmd(g, SSD1306_ROWSCANINC);
				break;
			case GDISP_ROTATE_270:
				write_cmd(g, SSD1306_COLSCANINC);
				write_cmd(g, SSD1306_ROWSCANDEC);
				break;
			}
			release_bus(g);
			g->g.Orientation = orient;
			return;

		case GDISP_CONTROL_CONTRAST:
            if ((unsigned)g->p.ptr > 100)
            	g->p.ptr = (void *)100;
			acquire_bus(g);
			write_cmd2(g, SSD1306_SETCONTRAST, (((unsigned)g->p.ptr)<<8)/101);
			release_bus(g);
            g->g.Contrast = (unsigned)g->p.ptr;
			return;

		// Our own special controller code to inverse the display
		// 0 = normal, 1 = inverse
		case GDISP_CONTROL_INVERSE:
			acquire_bus(g);
			write_cmd(g, g->p.ptr ? SSD1306_INVERTDISPLAY : SSD1306_NORMALDISPLAY);
			release_bus(g);
			return;
		}
	}
#endif // GDISP_NEED_CONTROL

#endif // GFX_USE_GDISP
