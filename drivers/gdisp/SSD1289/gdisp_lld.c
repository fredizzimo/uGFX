/*
 * This file is subject to the terms of the GFX License. If a copy of
 * the license was not distributed with this file, you can obtain one at:
 *
 *              http://ugfx.org/license.html
 */

/**
 * @file    drivers/gdisp/SSD1289/gdisp_lld.c
 * @brief   GDISP Graphics Driver subsystem low level driver source for the SSD1289 display.
 */

#include "gfx.h"

#if GFX_USE_GDISP

#define GDISP_LLD_DECLARATIONS
#include "gdisp/lld/gdisp_lld.h"
#include "gdisp_lld_board.h"

/*===========================================================================*/
/* Driver local definitions.                                                 */
/*===========================================================================*/

#ifndef GDISP_SCREEN_HEIGHT
	#define GDISP_SCREEN_HEIGHT		320
#endif
#ifndef GDISP_SCREEN_WIDTH
	#define GDISP_SCREEN_WIDTH		240
#endif
#ifndef GDISP_INITIAL_CONTRAST
	#define GDISP_INITIAL_CONTRAST	50
#endif
#ifndef GDISP_INITIAL_BACKLIGHT
	#define GDISP_INITIAL_BACKLIGHT	100
#endif

/*===========================================================================*/
/* Driver local functions.                                                   */
/*===========================================================================*/

// Some common routines and macros
#define dummy_read()				{ volatile uint16_t dummy; dummy = read_data(); (void) dummy; }
#define write_reg(reg, data)		{ write_index(reg); write_data(data); }

static void set_viewport(GDISPDriver* g) {
	/* Reg 0x44 - Horizontal RAM address position
	 * 		Upper Byte - HEA
	 * 		Lower Byte - HSA
	 * 		0 <= HSA <= HEA <= 0xEF
	 * Reg 0x45,0x46 - Vertical RAM address position
	 * 		Lower 9 bits gives 0-511 range in each value
	 * 		0 <= Reg(0x45) <= Reg(0x46) <= 0x13F
	 * Reg 0x004E is an 8 bit value - start x position
	 * Reg 0x004F is 9 bit - start y position
	 * Use a bit mask to make sure they are not set too high
	 */
	switch(g->g.Orientation) {
		case GDISP_ROTATE_0:
			write_reg(0x44, (((g->p.x+g->p.cx-1) << 8) & 0xFF00 ) | (g->p.x & 0x00FF));
			write_reg(0x45, g->p.y & 0x01FF);
			write_reg(0x46, (g->p.y+g->p.cy-1) & 0x01FF);
			write_reg(0x004e, g->p.x & 0x00FF);
			write_reg(0x004f, g->p.y & 0x01FF);
			break;
		case GDISP_ROTATE_90:
			write_reg(0x44, (((g->p.y+g->p.cy-1) << 8) & 0xFF00 ) | (g->p.y & 0x00FF));
			write_reg(0x45, (GDISP_SCREEN_HEIGHT-(g->p.x+g->p.cx)) & 0x01FF);
			write_reg(0x46, (GDISP_SCREEN_HEIGHT-1-g->p.x) & 0x01FF);
			write_reg(0x004e, g->p.y & 0x00FF);
			write_reg(0x004f, (GDISP_SCREEN_HEIGHT-1-g->p.x) & 0x01FF);
			break;
		case GDISP_ROTATE_180:
			write_reg(0x44, (((GDISP_SCREEN_WIDTH-1-g->p.x) & 0x00FF) << 8) | ((GDISP_SCREEN_WIDTH - (g->p.x+g->p.cx)) & 0x00FF));
			write_reg(0x45, (GDISP_SCREEN_HEIGHT-(g->p.y+g->p.cy)) & 0x01FF);
			write_reg(0x46, (GDISP_SCREEN_HEIGHT-1-g->p.y) & 0x01FF);
			write_reg(0x004e, (GDISP_SCREEN_WIDTH-1-g->p.x) & 0x00FF);
			write_reg(0x004f, (GDISP_SCREEN_HEIGHT-1-g->p.y) & 0x01FF);
			break;
		case GDISP_ROTATE_270:
			write_cmd2(PASET, GDISP_RAM_Y_OFFSET+g->p.x, GDISP_RAM_Y_OFFSET+g->p.x+g->p.cx-1);
			write_reg(0x44, (((GDISP_SCREEN_WIDTH-1-g->p.y) & 0x00FF) << 8) | ((GDISP_SCREEN_WIDTH-(g->p.y+g->p.cy)) & 0x00FF));
			write_reg(0x45, g->p.x & 0x01FF);
			write_reg(0x46, (g->p.x+g->p.cx-1) & 0x01FF);
			write_reg(0x004e, (GDISP_SCREEN_WIDTH-1-g->p.y) & 0x00FF);
			write_reg(0x004f, g->p.x & 0x01FF);
			break;
	}
	write_index(0x0022);
}

/*===========================================================================*/
/* Driver interrupt handlers.                                                */
/*===========================================================================*/

/*===========================================================================*/
/* Driver exported functions.                                                */
/*===========================================================================*/

LLDSPEC bool_t gdisp_lld_init(GDISPDriver *g) {
	/* Initialise your display */
	init_board();

	// Hardware reset
	setpin_reset(TRUE);
	gfxSleepMilliseconds(20);
	setpin_reset(FALSE);
	gfxSleepMilliseconds(20);

	// Get the bus for the following initialisation commands
	acquire_bus();
	
	write_reg(0x0000,0x0001);		gfxSleepMicroseconds(5);
	write_reg(0x0003,0xA8A4);    	gfxSleepMicroseconds(5);
	write_reg(0x000C,0x0000);    	gfxSleepMicroseconds(5);
	write_reg(0x000D,0x080C);    	gfxSleepMicroseconds(5);
    write_reg(0x000E,0x2B00);    	gfxSleepMicroseconds(5);
    write_reg(0x001E,0x00B0);    	gfxSleepMicroseconds(5);
	write_reg(0x0001,0x2B3F);		gfxSleepMicroseconds(5);
    write_reg(0x0002,0x0600);    	gfxSleepMicroseconds(5);
    write_reg(0x0010,0x0000);    	gfxSleepMicroseconds(5);
    write_reg(0x0011,0x6070);    	gfxSleepMicroseconds(5);
    write_reg(0x0005,0x0000);    	gfxSleepMicroseconds(5);
    write_reg(0x0006,0x0000);    	gfxSleepMicroseconds(5);
    write_reg(0x0016,0xEF1C);    	gfxSleepMicroseconds(5);
    write_reg(0x0017,0x0003);    	gfxSleepMicroseconds(5);
    write_reg(0x0007,0x0133);    	gfxSleepMicroseconds(5);
    write_reg(0x000B,0x0000);    	gfxSleepMicroseconds(5);
    write_reg(0x000F,0x0000);    	gfxSleepMicroseconds(5);
    write_reg(0x0041,0x0000);    	gfxSleepMicroseconds(5);
    write_reg(0x0042,0x0000);    	gfxSleepMicroseconds(5);
    write_reg(0x0048,0x0000);    	gfxSleepMicroseconds(5);
    write_reg(0x0049,0x013F);    	gfxSleepMicroseconds(5);
    write_reg(0x004A,0x0000);    	gfxSleepMicroseconds(5);
    write_reg(0x004B,0x0000);    	gfxSleepMicroseconds(5);
    write_reg(0x0044,0xEF00);    	gfxSleepMicroseconds(5);
    write_reg(0x0045,0x0000);    	gfxSleepMicroseconds(5);
    write_reg(0x0046,0x013F);    	gfxSleepMicroseconds(5);
    write_reg(0x0030,0x0707);    	gfxSleepMicroseconds(5);
    write_reg(0x0031,0x0204);    	gfxSleepMicroseconds(5);
    write_reg(0x0032,0x0204);    	gfxSleepMicroseconds(5);
    write_reg(0x0033,0x0502);    	gfxSleepMicroseconds(5);
    write_reg(0x0034,0x0507);    	gfxSleepMicroseconds(5);
    write_reg(0x0035,0x0204);    	gfxSleepMicroseconds(5);
    write_reg(0x0036,0x0204);    	gfxSleepMicroseconds(5);
    write_reg(0x0037,0x0502);    	gfxSleepMicroseconds(5);
    write_reg(0x003A,0x0302);    	gfxSleepMicroseconds(5);
    write_reg(0x003B,0x0302);    	gfxSleepMicroseconds(5);
    write_reg(0x0023,0x0000);    	gfxSleepMicroseconds(5);
    write_reg(0x0024,0x0000);    	gfxSleepMicroseconds(5);
    write_reg(0x0025,0x8000);    	gfxSleepMicroseconds(5);
    write_reg(0x004f,0x0000);		gfxSleepMicroseconds(5);
    write_reg(0x004e,0x0000);		gfxSleepMicroseconds(5);

 	// Release the bus
	release_bus();
	
	/* Turn on the back-light */
	set_backlight(GDISP_INITIAL_BACKLIGHT);

   /* Initialise the GDISP structure */
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
		set_viewport(g);
	}
	LLDSPEC	void gdisp_lld_write_color(GDISPDriver *g) {
		write_data(g->p.color);
	}
	LLDSPEC	void gdisp_lld_write_stop(GDISPDriver *g) {
		release_bus();
	}
#endif

#if GDISP_HARDWARE_STREAM_READ
	LLDSPEC	void gdisp_lld_read_start(GDISPDriver *g) {
		acquire_bus();
		set_viewport(g);
		setreadmode();
		dummy_read();
	}
	LLDSPEC	color_t gdisp_lld_read_color(GDISPDriver *g) {
		return read_data();
	}
	LLDSPEC	void gdisp_lld_read_stop(GDISPDriver *g) {
		setwritemode();
		release_bus();
	}
#endif

#if GDISP_HARDWARE_FILLS && defined(GDISP_USE_DMA)
	LLDSPEC void gdisp_lld_fill_area(GDISPDriver *g) {
		acquire_bus();
		set_viewport(g);
		dma_with_noinc(&color, g->p.cx*g->p.cy)
		release_bus();
	}
#endif

#if GDISP_HARDWARE_BITFILLS && defined(GDISP_USE_DMA)
	LLDSPEC void gdisp_lld_blit_area(GDISPDriver *g) {
		pixel_t		*buffer;
		coord_t		ycnt;

		buffer = (pixel_t *)g->p.ptr + g->p.x1 + g->p.y1 * g->p.x2;

		acquire_bus();
		set_viewport(g);
		if (g->p.x2 == g->p.cx) {
			dma_with_inc(buffer, g->p.cx*g->p.cy);
		} else {
			for (ycnt = g->p.cy; ycnt; ycnt--, buffer += g->p.x2)
				dma_with_inc(buffer, g->p.cy);
		}
		release_bus();
	}
#endif

#if GDISP_NEED_CONTROL && GDISP_HARDWARE_CONTROL
	LLDSPEC void gdisp_lld_control(GDISPDriver *g) {
		switch(g->p.x) {
		case GDISP_CONTROL_POWER:
			if (g->g.Powermode == (powermode_t)g->p.ptr)
				return;
			switch((powermode_t)g->p.ptr) {
			case powerOff:
				acquire_bus();
				write_reg(0x0010, 0x0000);	// leave sleep mode
				write_reg(0x0007, 0x0000);	// halt operation
				write_reg(0x0000, 0x0000);	// turn off oscillator
				write_reg(0x0010, 0x0001);	// enter sleep mode
				release_bus();
				break;
			case powerOn:
				acquire_bus();
				write_reg(0x0010, 0x0000);	// leave sleep mode
				release_bus();
				if (g->g.Powermode != powerSleep)
					gdisp_lld_init();
				break;
			case powerSleep:
				acquire_bus();
				write_reg(0x0010, 0x0001);	// enter sleep mode
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
				/* ID = 11 AM = 0 */
				write_reg(0x0011, 0x6070);
				release_bus();
				g->g.Height = GDISP_SCREEN_HEIGHT;
				g->g.Width = GDISP_SCREEN_WIDTH;
				break;
			case GDISP_ROTATE_90:
				acquire_bus();
				/* ID = 01 AM = 1 */
				write_reg(0x0011, 0x6058);
				release_bus();
				g->g.Height = GDISP_SCREEN_WIDTH;
				g->g.Width = GDISP_SCREEN_HEIGHT;
				break;
			case GDISP_ROTATE_180:
				acquire_bus();
				/* ID = 00 AM = 0 */
				write_reg(0x0011, 0x6040);
				release_bus();
				g->g.Height = GDISP_SCREEN_HEIGHT;
				g->g.Width = GDISP_SCREEN_WIDTH;
				break;
			case GDISP_ROTATE_270:
				acquire_bus();
				/* ID = 10 AM = 1 */
				write_reg(0x0011, 0x6068);
				release_bus();
				g->g.Height = GDISP_SCREEN_WIDTH;
				g->g.Width = GDISP_SCREEN_HEIGHT;
				break;
			default:
				return;
			}
			g->g.Orientation = (orientation_t)value;
			return;

        case GDISP_CONTROL_BACKLIGHT:
            if ((unsigned)g->p.ptr > 100)
            	g->p.ptr = (void *)100;
            set_backlight((unsigned)g->p.ptr);
            g->g.Backlight = (unsigned)g->p.ptr;
            return;

		//case GDISP_CONTROL_CONTRAST:
        default:
            return;
		}
	}
#endif

#endif /* GFX_USE_GDISP */
