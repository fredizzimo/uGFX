/*
 * This file is subject to the terms of the GFX License. If a copy of
 * the license was not distributed with this file, you can obtain one at:
 *
 *              http://ugfx.org/license.html
 */

#include "gfx.h"

#if GFX_USE_GDISP

#define GDISP_DRIVER_VMT			GDISPVMT_PCF8812
#include "drivers/gdisp/PCF8812/gdisp_lld_config.h"
#include "src/gdisp/driver.h"

#include "board_PCF8812.h"

/*===========================================================================*/
/* Driver local definitions.                                                 */
/*===========================================================================*/

#ifndef GDISP_SCREEN_HEIGHT
	#define GDISP_SCREEN_HEIGHT		65
#endif
#ifndef GDISP_SCREEN_WIDTH
	#define GDISP_SCREEN_WIDTH		102
#endif
#ifndef GDISP_INITIAL_CONTRAST
	#define GDISP_INITIAL_CONTRAST	51
#endif
#ifndef GDISP_INITIAL_BACKLIGHT
	#define GDISP_INITIAL_BACKLIGHT	100
#endif

#define GDISP_FLG_NEEDFLUSH			(GDISP_FLG_DRIVER << 0)

#include "drivers/gdisp/PCF8812/PCF8812.h"

/*===========================================================================*/
/* Driver local functions.                                                   */
/*===========================================================================*/

// Some common routines and macros
#define RAM(g)				((uint8_t *)g->priv)

unsigned char RAM[(GDISP_SCREEN_WIDTH * GDISP_SCREEN_HEIGHT / 8)];

#define xyaddr(x, y)   ((x) + ((y) >> 3) * GDISP_SCREEN_WIDTH)
#define xybit(y)       (1 << ((y) & 7))

/*===========================================================================*/
/* Driver exported functions.                                                */
/*===========================================================================*/

/*
 * As this controller can't update on a pixel boundary we need to maintain the
 * the entire display surface in memory so that we can do the necessary bit
 * operations. Fortunately it is a small display in monochrome.
 * 65 * 102 / 8 = 829 bytes.
 */

LLDSPEC bool_t gdisp_lld_init(GDisplay *g) {
	// The private area is the display surface.
	g->priv = gfxAlloc((GDISP_SCREEN_WIDTH * GDISP_SCREEN_HEIGHT / 8));

	// Initialise the board interface
	init_board(g);

	// Hardware reset
	setpin_reset(g, TRUE);
	gfxSleepMilliseconds(100);
	setpin_reset(g, FALSE);
	gfxSleepMilliseconds(100);

	acquire_bus(g);

	write_cmd(g, PCF8812_SET_FUNC   | PCF8812_H);
	write_cmd(g, PCF8812_SET_TEMP   | PCF8812_TEMP_MODE_1);
	write_cmd(g, PCF8812_SET_VMULT  | PCF8812_VMULT_MODE_1);
	write_cmd(g, PCF8812_SET_VOP    | 0xFF);
	write_cmd(g, PCF8812_SET_FUNC);
	write_cmd(g, PCF8812_DISPLAY    | PCF8812_DISPLAY_MODE_NORMAL);
	write_cmd(g, PCF8812_SET_X); // X = 0
	write_cmd(g, PCF8812_SET_Y); // Y = 0


	unsigned int i;
	for (i = 0; i < (GDISP_SCREEN_WIDTH * GDISP_SCREEN_HEIGHT / 8); i++)
	{
		write_data(g, (uint8_t*)0x00, 1);
	}

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

		// Don't flush if we don't need it.
		if (!(g->flags & GDISP_FLG_NEEDFLUSH))
			return;

		unsigned int i;

		acquire_bus(g);

		//write_cmd(g, PCF8812_SET_X);
		//write_cmd(g, PCF8812_SET_Y);

		for (i = 0; i < 9; i++) {
		  write_data(g, RAM(g) + (i * GDISP_SCREEN_WIDTH), GDISP_SCREEN_WIDTH);
		}
		release_bus(g);
	}
#endif

#if GDISP_HARDWARE_DRAWPIXEL
	LLDSPEC void gdisp_lld_draw_pixel(GDisplay *g) {
		coord_t		x, y;

		switch(g->g.Orientation) {
		default:
		case GDISP_ROTATE_0:
			x = g->p.x;
			y = g->p.y;
			break;
		case GDISP_ROTATE_90:
			x = g->p.y;
			y = GDISP_SCREEN_HEIGHT-1 - g->p.x;
			break;
		case GDISP_ROTATE_180:
			x = GDISP_SCREEN_WIDTH-1 - g->p.x;
			y = GDISP_SCREEN_HEIGHT-1 - g->p.y;
			break;
		case GDISP_ROTATE_270:
			x = GDISP_SCREEN_HEIGHT-1 - g->p.y;
			y = g->p.x;
			break;
		}
		if (gdispColor2Native(g->p.color) != Black)
			RAM(g)[xyaddr(x, y)] |= xybit(y);
		else
			RAM(g)[xyaddr(x, y)] &= ~xybit(y);
		g->flags |= GDISP_FLG_NEEDFLUSH;
	}
#endif

#endif // GFX_USE_GDISP
