/*
 * This file is subject to the terms of the GFX License. If a copy of
 * the license was not distributed with this file, you can obtain one at:
 *
 *              http://ugfx.org/license.html
 */

/**
 * @file    drivers/gdisp/Nokia6610GE12/gdisp_lld.c
 * @brief   GDISP Graphics Driver subsystem low level driver source for the Nokia6610 GE12 display.
 *
 * @addtogroup GDISP
 * @{
 */

#include "gfx.h"

#if GFX_USE_GDISP /*|| defined(__DOXYGEN__)*/

#if defined(GDISP_SCREEN_HEIGHT)
	#warning "GDISP: This low level driver does not support setting a screen size. It is being ignored."
	#undef GDISP_SCREEN_HEIGHT
#endif
#if defined(GDISP_SCREEN_WIDTH)
	#warning "GDISP: This low level driver does not support setting a screen size. It is being ignored."
	#undef GDISP_SCREEN_WIDTH
#endif

#define GDISP_LLD_DECLARATIONS
#include "gdisp/lld/gdisp_lld.h"
#include "gdisp_lld_board.h"

/*===========================================================================*/
/* Driver local definitions.                                                 */
/*===========================================================================*/

#include "GE12.h"

#define GDISP_SCAN_LINES			132
#define GDISP_SLEEP_SIZE			32		/* Sleep mode window lines - this must be 32 on this controller */

// Set parameters if they are not already set
#ifndef GDISP_SCREEN_HEIGHT
	#define GDISP_SCREEN_HEIGHT		130
#endif
#ifndef GDISP_SCREEN_WIDTH
	#define GDISP_SCREEN_WIDTH		130
#endif
#ifndef GDISP_RAM_X_OFFSET
	#define GDISP_RAM_X_OFFSET		0		/* Offset in RAM of visible area */
#endif
#ifndef GDISP_RAM_Y_OFFSET
	#define GDISP_RAM_Y_OFFSET		2		/* Offset in RAM of visible area */
#endif
#endif
#ifndef GDISP_SLEEP_POS
	#define GDISP_SLEEP_POS			((GDISP_SCAN_LINES-GDISP_SLEEP_SIZE)/2 & ~3)
#endif
#ifndef GDISP_INITIAL_CONTRAST
	#define GDISP_INITIAL_CONTRAST	50
#endif
#ifndef GDISP_INITIAL_BACKLIGHT
	#define GDISP_INITIAL_BACKLIGHT	100
#endif

/*===========================================================================*/
/* Driver exported variables.                                                */
/*===========================================================================*/

/*===========================================================================*/
/* Driver local variables.                                                   */
/*===========================================================================*/

static color_t savecolor;

#define GDISP_FLG_ODDBYTE	(GDISP_FLG_DRIVER<<0)

/*===========================================================================*/
/* Driver local functions.                                                   */
/*===========================================================================*/

// Some macros just to make reading the code easier
#define delayms(ms)					gfxSleepMilliseconds(ms)
#define write_data2(d1, d2)			{ write_data(d1); write_data(d2); }
#define write_data3(d1, d2, d3)		{ write_data(d1); write_data(d2); write_data(d3); }
#define write_cmd1(cmd, d1)			{ write_cmd(cmd); write_data(d1); }
#define write_cmd2(cmd, d1, d2)		{ write_cmd(cmd); write_data2(d1, d2); }
#define write_cmd3(cmd, d1, d2, d3)	{ write_cmd(cmd); write_data3(d1, d2, d3); }

/*===========================================================================*/
/* Driver exported functions.                                                */
/*===========================================================================*/

LLDSPEC bool_t gdisp_lld_init(GDISPDriver *g) {
	/* Initialise your display */
	init_board();

	// Hardware reset
	setpin_reset(TRUE);
	delayms(20);
	setpin_reset(FALSE);
	delayms(20);

	acquire_bus();
	write_cmd(SLEEPOUT);								// Sleep out
	write_cmd1(COLMOD, 0x03);							// Color Interface Pixel Format - 0x03 = 12 bits-per-pixel
	write_cmd1(MADCTL, 0x00);							// Memory access controller
	write_cmd1(SETCON, 127*GDISP_INITIAL_CONTRAST/100-64);			// Write contrast
	delayms(20);
	release_bus();
	
	/* Turn on the back-light */
	set_backlight(GDISP_INITIAL_BACKLIGHT);

	/* Initialise the GDISP structure to match */
	g->g.Width = GDISP_SCREEN_WIDTH;
	g->g.Height = GDISP_SCREEN_HEIGHT;
	g->g.Orientation = GDISP_ROTATE_0;
	g->g.Powermode = powerOn;
	g->g.Backlight = GDISP_INITIAL_BACKLIGHT;
	g->g.Contrast = GDISP_INITIAL_CONTRAST;
	return TRUE;
}

#if GDISP_HARDWARE_STREAM_WRITE
	LLDSPEC	void gdisp_lld_write_start(GDISPDriver *g) {
		acquire_bus();
		write_cmd2(CASET, GDISP_RAM_X_OFFSET+g->p.x, GDISP_RAM_X_OFFSET+g->p.x+g->p.cx-1);			// Column address set
		write_cmd2(PASET, GDISP_RAM_Y_OFFSET+g->p.y, GDISP_RAM_Y_OFFSET+g->p.y+g->p.cy-1);			// Page address set
		write_cmd(RAMWR);
		g->flags &= ~GDISP_FLG_ODDBYTE;
	}
	LLDSPEC	void gdisp_lld_write_color(GDISPDriver *g) {
		if ((g->flags & GDISP_FLG_ODDBYTE)) {
			// Write the pair of pixels to the display
			write_data3(((savecolor >> 4) & 0xFF), (((savecolor << 4) & 0xF0)|((g->p.color >> 8) & 0x0F)), (g->p.color & 0xFF));
			g->flags &= ~GDISP_FLG_ODDBYTE;
		} else {
			savecolor = g->p.color;
			g->flags |= GDISP_FLG_ODDBYTE;
		}
	}
	LLDSPEC	void gdisp_lld_write_stop(GDISPDriver *g) {
		if ((g->flags & GDISP_FLG_ODDBYTE)) {
			write_data2(((savecolor >> 4) & 0xFF), ((savecolor << 4) & 0xF0));
			write_cmd(NOP);
		}
		release_bus();
	}
#endif

#if GDISP_NEED_CONTROL && GDISP_HARDWARE_CONTROL
	LLDSPEC void gdisp_lld_control(GDISPDriver *g) {
		/* The hardware is capable of supporting...
		 * 	GDISP_CONTROL_POWER				- supported
		 * 	GDISP_CONTROL_ORIENTATION		- supported
		 * 	GDISP_CONTROL_BACKLIGHT			- supported
		 * 	GDISP_CONTROL_CONTRAST			- supported
		 */
		switch(g->p.x) {
		case GDISP_CONTROL_POWER:
			if (g->g.Powermode == (powermode_t)g->p.ptr)
				return;
			switch((powermode_t)g->p.ptr) {
			case powerOff:
				acquire_bus();
				write_cmd(SLEEPIN);
				release_bus();
				break;
			case powerOn:
				acquire_bus();
				write_cmd(SLEEPOUT);
				delayms(20);
				write_cmd(NORON);							// Set Normal mode (my)
				release_bus();
				break;
			case powerSleep:
				acquire_bus();
				write_cmd(SLEEPOUT);
				delayms(20);
				write_cmd2(PTLAR, GDISP_SLEEP_POS, GDISP_SLEEP_POS+GDISP_SLEEP_SIZE)
				write_cmd(PTLON);
				release_bus();
				break;
			default:
				return;
			}
			g->g.Powermode = (powermode_t)g->p.ptr;
			return;

		case GDISP_CONTROL_ORIENTATION:
			if (g->g.Orientation == (orientation_t)g->p.ptr)
				return;
			switch((orientation_t)g->p.ptr) {
			case GDISP_ROTATE_0:
				acquire_bus();
				write_cmd1(MADCTL, 0x00);
				release_bus();
				g->g.Height = GDISP_SCREEN_HEIGHT;
				g->g.Width = GDISP_SCREEN_WIDTH;
				break;
			case GDISP_ROTATE_90:
				acquire_bus();
				write_cmd1(MADCTL, 0xA0);					// MY, MX, V, LAO, RGB, X, X, X
				release_bus();
				g->g.Height = GDISP_SCREEN_WIDTH;
				g->g.Width = GDISP_SCREEN_HEIGHT;
				break;
			case GDISP_ROTATE_180:
				acquire_bus();
				write_cmd1(MADCTL, 0xC0);
				release_bus();
				g->g.Height = GDISP_SCREEN_HEIGHT;
				g->g.Width = GDISP_SCREEN_WIDTH;
				break;
			case GDISP_ROTATE_270:
				acquire_bus();
				write_cmd1(MADCTL, 0x60);
				release_bus();
				g->g.Height = GDISP_SCREEN_WIDTH;
				g->g.Width = GDISP_SCREEN_HEIGHT;
				break;
			default:
				return;
			}
			g->g.Orientation = (orientation_t)g->p.ptr;
			return;

		case GDISP_CONTROL_BACKLIGHT:
			if ((unsigned)g->p.ptr > 100) g->p.ptr = (void *)100;
			set_backlight((unsigned)g->p.ptr);
			g->g.Backlight = (unsigned)g->p.ptr;
			return;
		case GDISP_CONTROL_CONTRAST:
			if ((unsigned)g->p.ptr > 100) g->p.ptr = (void *)100;
			acquire_bus();
			write_cmd1(CONTRAST,(unsigned)127*g->p.ptr/100-64);
			release_bus();
			g->g.Contrast = (unsigned)g->p.ptr;
			return;
		}
	}
#endif

#endif /* GFX_USE_GDISP */
/** @} */
