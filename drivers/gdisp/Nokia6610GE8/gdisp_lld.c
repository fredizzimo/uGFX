/*
 * This file is subject to the terms of the GFX License. If a copy of
 * the license was not distributed with this file, you can obtain one at:
 *
 *              http://ugfx.org/license.html
 */

/**
 * @file    drivers/gdisp/Nokia6610GE8/gdisp_lld.c
 * @brief   GDISP Graphics Driver subsystem low level driver source for the Nokia6610 GE8 display.
 *
 * @addtogroup GDISP
 * @{
 */

#include "gfx.h"

#if GFX_USE_GDISP

#define GDISP_LLD_DECLARATIONS
#include "gdisp/lld/gdisp_lld.h"

/**
 * This is for the EPSON (GE8) controller driving a Nokia6610 color LCD display.
 * Note that there is also a PHILIPS (GE12) controller for the same display that this code
 * does not support.
 *
 * The controller drives a 132x132 display but a 1 pixel surround is not visible
 * which gives a visible area of 130x130.
 *
 * This controller does not support reading back over the SPI interface.
 * Additionally, the Olimex board doesn't even connect the pin.
 *
 * The hardware is capable of doing full width vertical scrolls aligned
 * on a 4 line boundary however that is not sufficient to support general vertical scrolling.
 * We also can't manually do read/modify scrolling because we can't read in SPI mode.
 *
 * The controller has some quirkyness when operating in other than rotation 0 mode.
 * When any direction is decremented it starts at location 0 rather than the end of
 * the area. Whilst this can be handled when we know the specific operation (pixel, fill, blit)
 * it cannot be handled in a generic stream operation. So, when orientation support is turned
 * on (and needed) we use complex operation specific routines instead of simple streaming
 * routines. This has a (small) performance penalty and a significant code size penalty so
 * don't turn on orientation support unless you really need it.
 *
 * Some of the more modern controllers have a broken command set. If you have one of these
 * you will recognise it by the colors being off on anything drawn after an odd (as opposed to
 * even) pixel count area being drawn. If so then set GDISP_GE8_BROKEN_CONTROLLER to TRUE
 * on your gdisp_lld_board.h file. The price is that streaming calls that are completed
 * without exactly the window size write operations and where the number of write operations
 * is odd (rather than even), it will draw an extra pixel. If this is important to you, turn on
 * orientation support and the streaming operations will be emulated (as described above).
 */

/*===========================================================================*/
/* Driver local definitions.                                                 */
/*===========================================================================*/

#if defined(GDISP_SCREEN_HEIGHT)
	#warning "GDISP: This low level driver does not support setting a screen size. It is being ignored."
	#undef GDISP_SCREEN_HEIGHT
#endif
#if defined(GDISP_SCREEN_WIDTH)
	#warning "GDISP: This low level driver does not support setting a screen size. It is being ignored."
	#undef GDISP_SCREEN_WIDTH
#endif

#include "gdisp_lld_board.h"
#include "GE8.h"

#define GDISP_SCAN_LINES			132

// Set parameters if they are not already set
#ifndef GDISP_GE8_BROKEN_CONTROLLER
	#define GDISP_GE8_BROKEN_CONTROLLER		TRUE
#endif
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
#ifndef GDISP_SLEEP_SIZE
	#define GDISP_SLEEP_SIZE		32		/* Sleep mode window lines */
#endif
#ifndef GDISP_SLEEP_POS
	#define GDISP_SLEEP_POS			((GDISP_SCAN_LINES-GDISP_SLEEP_SIZE)/2)
#endif
#ifndef GDISP_INITIAL_CONTRAST
	#define GDISP_INITIAL_CONTRAST	38
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

#if GDISP_HARDWARE_STREAM_WRITE
	static color_t savecolor;
	#if GDISP_GE8_BROKEN_CONTROLLER
		static color_t firstcolor;
	#endif
#endif

#define GDISP_FLG_ODDBYTE	(GDISP_FLG_DRIVER<<0)
#define GDISP_FLG_RUNBYTE	(GDISP_FLG_DRIVER<<1)

/*===========================================================================*/
/* Driver local functions.                                                   */
/*===========================================================================*/

// Some macros just to make reading the code easier
#define delayms(ms)						gfxSleepMilliseconds(ms)
#define write_data2(d1, d2)				{ write_data(d1); write_data(d2); }
#define write_data3(d1, d2, d3)			{ write_data(d1); write_data(d2); write_data(d3); }
#define write_data4(d1, d2, d3, d4)		{ write_data(d1); write_data(d2); write_data(d3); write_data(d4); }
#define write_cmd1(cmd, d1)				{ write_cmd(cmd); write_data(d1); }
#define write_cmd2(cmd, d1, d2)			{ write_cmd(cmd); write_data2(d1, d2); }
#define write_cmd3(cmd, d1, d2, d3)		{ write_cmd(cmd); write_data3(d1, d2, d3); }
#define write_cmd4(cmd, d1, d2, d3, d4)	{ write_cmd(cmd); write_data4(d1, d2, d3, d4); }

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

	// Get the bus for the following initialisation commands
	acquire_bus();
	
	write_cmd4(DISCTL, 0x00, GDISP_SCAN_LINES/4-1, 0x0A, 0x00);	// Display control - How the controller drives the LCD
															// P1: 0x00 = 2 divisions, switching period=8 (default)
															// P2: 0x20 = nlines/4 - 1 = 132/4 - 1 = 32)
															// P3: 0x0A = standard inverse highlight, inversion every frame
															// P4: 0x00 = dispersion on
	write_cmd1(COMSCN, 0x01);							// COM scan - How the LCD is connected to the controller
															// P1: 0x01 = Scan 1->80, 160<-81
	write_cmd(OSCON);									// Internal oscillator ON
	write_cmd(SLPOUT);									// Sleep out
	write_cmd1(PWRCTR, 0x0F);							// Power control - reference voltage regulator on, circuit voltage follower on, BOOST ON
	write_cmd3(DATCTL, 0x00, 0x00, 0x02);				// Data control
															// P1: 0x00 = page address normal, column address normal, address scan in column direction
															// P2: 0x00 = RGB sequence (default value)
															// P3: 0x02 = 4 bits per colour (Type A)
	write_cmd2(VOLCTR, GDISP_INITIAL_CONTRAST, 0x03);	// Voltage control (contrast setting)
															// P1 = Contrast
															// P2 = 3 resistance ratio (only value that works)
	delayms(100);										// Allow power supply to stabilise
	write_cmd(DISON);									// Turn on the display

	// Release the bus
	release_bus();
	
	/* Turn on the back-light */
	set_backlight(GDISP_INITIAL_BACKLIGHT);

	/* Initialise the GDISP structure to match */
	g->g.Orientation = GDISP_ROTATE_0;
	g->g.Powermode = powerOn;
	g->g.Backlight = GDISP_INITIAL_BACKLIGHT;
	g->g.Contrast = GDISP_INITIAL_CONTRAST;
	g->g.Width = GDISP_SCREEN_WIDTH;
	g->g.Height = GDISP_SCREEN_HEIGHT;
	return TRUE;
}

#if GDISP_HARDWARE_STREAM_WRITE
	LLDSPEC	void gdisp_lld_write_start(GDISPDriver *g) {
		acquire_bus();
		write_cmd2(CASET, GDISP_RAM_X_OFFSET+g->p.x, GDISP_RAM_X_OFFSET+g->p.x+g->p.cx-1);			// Column address set
		write_cmd2(PASET, GDISP_RAM_Y_OFFSET+g->p.y, GDISP_RAM_Y_OFFSET+g->p.y+g->p.cy-1);			// Page address set
		write_cmd(RAMWR);
		g->flags &= ~(GDISP_FLG_ODDBYTE|GDISP_FLG_RUNBYTE);
	}
	LLDSPEC	void gdisp_lld_write_color(GDISPDriver *g) {
		#if GDISP_GE8_BROKEN_CONTROLLER
			if (!(g->flags & GDISP_FLG_RUNBYTE)) {
				firstcolor = g->p.color;
				g->flags |= GDISP_FLG_RUNBYTE;
			}
		#endif
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
			#if GDISP_GE8_BROKEN_CONTROLLER
				/**
				 * We have a real problem here - we need to write a singular pixel
				 * Methods that are supposed to work...
				 * 1/ Write the pixel (2 bytes) and then send a NOP command. This doesn't work, the pixel doesn't get written
				 * 		and it is maintained in the latch where it causes problems for the next window.
				 * 2/ Just write a dummy extra pixel as stuff beyond the window gets discarded. This doesn't work as contrary to
				 * 		the documentation the buffer wraps and the first pixel gets overwritten.
				 * 3/ Put the controller in 16 bits per pixel Type B mode where each pixel is performed by writing two bytes. This
				 * 		also doesn't work as the controller refuses to enter Type B mode (it stays in Type A mode).
				 *
				 * These methods might work on some controllers - just not on the one of the broken versions.
				 *
				 * For these broken controllers:
				 * 	We know we can wrap to the first byte (just overprint it) if we are at the end of the stream area.
				 * 	If not, we need to create a one pixel by one pixel window to fix this - Uuch. Fortunately this should only happen if the
				 * 	user application uses the streaming calls and then terminates the stream early or after buffer wrap.
				 * 	Since this is such an unlikely situation we just don't handle it.
				 */
				write_data3(((savecolor >> 4) & 0xFF), (((savecolor << 4) & 0xF0)|((firstcolor >> 8) & 0x0F)), (firstcolor & 0xFF));
			#else
				write_data2(((savecolor >> 4) & 0xFF), ((savecolor << 4) & 0xF0));
				write_cmd(NOP);
			#endif
		}

		release_bus();
	}
#endif

#if GDISP_HARDWARE_DRAWPIXEL
	LLDSPEC void gdisp_lld_draw_pixel(GDISPDriver *g) {
		acquire_bus();
		switch(g->g.Orientation) {
			case GDISP_ROTATE_0:
				write_cmd2(CASET, GDISP_RAM_X_OFFSET+g->p.x, GDISP_RAM_X_OFFSET+g->p.x);			// Column address set
				write_cmd2(PASET, GDISP_RAM_Y_OFFSET+g->p.y, GDISP_RAM_Y_OFFSET+g->p.y);			// Page address set
				break;
			case GDISP_ROTATE_90:
				write_cmd2(CASET, GDISP_RAM_X_OFFSET+g->p.y, GDISP_RAM_X_OFFSET+g->p.y);
				write_cmd2(PASET, GDISP_RAM_Y_OFFSET-1+g->g.Width-g->p.x, GDISP_RAM_Y_OFFSET-1+g->g.Width-g->p.x);
				break;
			case GDISP_ROTATE_180:
				write_cmd2(CASET, GDISP_RAM_X_OFFSET-1+g->g.Width-g->p.x, GDISP_RAM_X_OFFSET-1+g->g.Width-g->p.x);
				write_cmd2(PASET, GDISP_RAM_Y_OFFSET-1+g->g.Height-g->p.y, GDISP_RAM_Y_OFFSET-1+g->g.Height-g->p.y);
				break;
			case GDISP_ROTATE_270:
				write_cmd2(CASET, GDISP_RAM_X_OFFSET-1+g->g.Height-g->p.y, GDISP_RAM_X_OFFSET-1+g->g.Height-g->p.y);
				write_cmd2(PASET, GDISP_RAM_Y_OFFSET+g->p.x, GDISP_RAM_Y_OFFSET+g->p.x);
				break;
		}
		write_cmd3(RAMWR, 0, (g->p.color>>8) & 0x0F, g->p.color & 0xFF);
		release_bus();
	}
#endif

/* ---- Optional Routines ---- */

#if GDISP_HARDWARE_FILLS
	LLDSPEC void gdisp_lld_fill_area(GDISPDriver *g) {
		unsigned tuples;

		tuples = (g->p.cx*g->p.cy+1)>>1;	// With an odd sized area we over-print by one pixel.
											// This extra pixel overwrites the first pixel (harmless as it is the same colour)

		acquire_bus();
		switch(g->g.Orientation) {
			case GDISP_ROTATE_0:
				write_cmd2(CASET, GDISP_RAM_X_OFFSET+g->p.x, GDISP_RAM_X_OFFSET+g->p.x+g->p.cx-1);			// Column address set
				write_cmd2(PASET, GDISP_RAM_Y_OFFSET+g->p.y, GDISP_RAM_Y_OFFSET+g->p.y+g->p.cy-1);			// Page address set
				break;
			case GDISP_ROTATE_90:
				write_cmd2(CASET, GDISP_RAM_X_OFFSET+g->p.y, GDISP_RAM_X_OFFSET+g->p.y+g->p.cy-1);
				write_cmd2(PASET, GDISP_RAM_Y_OFFSET+g->g.Width-g->p.x-g->p.cx, GDISP_RAM_Y_OFFSET+g->g.Width-g->p.x-1);
				break;
			case GDISP_ROTATE_180:
				write_cmd2(CASET, GDISP_RAM_X_OFFSET+g->g.Width-g->p.x-g->p.cx, GDISP_RAM_X_OFFSET+g->g.Width-g->p.x-1);
				write_cmd2(PASET, GDISP_RAM_Y_OFFSET+g->g.Height-g->p.y-g->p.cy, GDISP_RAM_Y_OFFSET+g->g.Height-g->p.y-1);
				break;
			case GDISP_ROTATE_270:
				write_cmd2(CASET, GDISP_RAM_X_OFFSET+g->g.Height-g->p.y-g->p.cy, GDISP_RAM_X_OFFSET+g->g.Height-g->p.y-1);
				write_cmd2(PASET, GDISP_RAM_Y_OFFSET+g->p.x, GDISP_RAM_Y_OFFSET+g->p.x+g->p.cx-1);
				break;
		}
		write_cmd(RAMWR);
		while(tuples--)
			write_data3(((g->p.color >> 4) & 0xFF), (((g->p.color << 4) & 0xF0)|((g->p.color >> 8) & 0x0F)), (g->p.color & 0xFF));
		release_bus();
	}
#endif

#if GDISP_HARDWARE_BITFILLS
	LLDSPEC void gdisp_lld_blit_area(GDISPDriver *g) {
		coord_t			lg, x, y;
		color_t			c1, c2;
		unsigned		tuples;
		const pixel_t	*buffer;
		#if GDISP_PACKED_PIXELS
			unsigned		pnum, pstart;
			const uint8_t	*p;
		#else
			const pixel_t	*p;
		#endif

		tuples = (g->p.cx * g->p.cy + 1)>>1;
		buffer = (const pixel_t *)g->p.ptr;

		/* Set up the data window to transfer */
		acquire_bus();
		switch(g->g.Orientation) {
			case GDISP_ROTATE_0:
				write_cmd2(CASET, GDISP_RAM_X_OFFSET+g->p.x, GDISP_RAM_X_OFFSET+g->p.x+g->p.cx-1);			// Column address set
				write_cmd2(PASET, GDISP_RAM_Y_OFFSET+g->p.y, GDISP_RAM_Y_OFFSET+g->p.y+g->p.cy-1);			// Page address set
				break;
			case GDISP_ROTATE_90:
				write_cmd2(CASET, GDISP_RAM_X_OFFSET+g->p.y, GDISP_RAM_X_OFFSET+g->p.y+g->p.cy-1);
				write_cmd2(PASET, GDISP_RAM_Y_OFFSET+g->g.Width-g->p.x-g->p.cx, GDISP_RAM_Y_OFFSET+g->g.Width-g->p.x-1);
				break;
			case GDISP_ROTATE_180:
				write_cmd2(CASET, GDISP_RAM_X_OFFSET+g->g.Width-g->p.x-g->p.cx, GDISP_RAM_X_OFFSET+g->g.Width-g->p.x-1);
				write_cmd2(PASET, GDISP_RAM_Y_OFFSET+g->g.Height-g->p.y-g->p.cy, GDISP_RAM_Y_OFFSET+g->g.Height-g->p.y-1);
				break;
			case GDISP_ROTATE_270:
				write_cmd2(CASET, GDISP_RAM_X_OFFSET+g->g.Height-g->p.y-g->p.cy, GDISP_RAM_X_OFFSET+g->g.Height-g->p.y-1);
				write_cmd2(PASET, GDISP_RAM_Y_OFFSET+g->p.x, GDISP_RAM_Y_OFFSET+g->p.x+g->p.cx-1);
				break;
		}
		write_cmd(RAMWR);

		/*
		 * Due to the way the Nokia6610 handles a decrementing column or page,
		 * we have to make adjustments as to where it is actually drawing from in the bitmap.
		 * For example, for 90 degree rotation the column is decremented on each
		 * memory write. The controller always starts with column 0 and then decrements
		 * to column cx-1, cx-2 etc. We therefore have to write-out the last bitmap line first.
		 */
		switch(g->g.Orientation) {
		case GDISP_ROTATE_0:		x = 0;			y = 0;			break;
		case GDISP_ROTATE_90:		x = g->p.cx-1;	y = 0;			break;
		case GDISP_ROTATE_180:		x = g->p.cx-1;	y = g->p.cy-1;	break;
		case GDISP_ROTATE_270:		x = 0;			y = g->p.cy-1;	break;
		}

		#if !GDISP_PACKED_PIXELS
			// Although this controller uses packed pixels we support unpacked pixel
			//  formats in this blit by packing the data as we feed it to the controller.

			lg = g->p.x2 - g->p.cx;					// The buffer gap between lines
			buffer += g->p.y1 * g->p.x2 + g->p.x1;	// The buffer start position
			p = buffer + g->p.x2*y + x;				// Adjustment for controller craziness

			while(tuples--) {
				/* Get a pixel */
				c1 = *p++;

				/* Check for line or buffer wrapping */
				if (++x >= g->p.cx) {
					x = 0;
					p += lg;
					if (++y >= g->p.cy) {
						y = 0;
						p = buffer;
					}
				}

				/* Get the next pixel */
				c2 = *p++;

				/* Check for line or buffer wrapping */
				if (++x >= g->p.cx) {
					x = 0;
					p += lg;
					if (++y >= g->p.cy) {
						y = 0;
						p = buffer;
					}
				}

				/* Write the pair of pixels to the display */
				write_data3(((c1 >> 4) & 0xFF), (((c1 << 4) & 0xF0)|((c2 >> 8) & 0x0F)), (c2 & 0xFF));
			}

		#else

			// Although this controller uses packed pixels, we may have to feed it into
			//  the controller with different packing to the source bitmap
			// There are 2 pixels per 3 bytes

			#if !GDISP_PACKED_LINES
				srccx = (g->p.x2 + 1) & ~1;
			#endif
			pstart = g->p.y1 * g->p.x2 + g->p.x1;										// The starting pixel number
			buffer = (const pixel_t)(((const uint8_t *)buffer) + ((pstart>>1) * 3));	// The buffer start position
			lg = ((g->p.x2-g->p.cx)>>1)*3;												// The buffer gap between lines
			pnum = pstart + g->p.x2*y + x;												// Adjustment for controller craziness
			p = ((const uint8_t *)buffer) + (((g->p.x2*y + x)>>1)*3);					// Adjustment for controller craziness

			while (tuples--) {
				/* Get a pixel */
				switch(pnum++ & 1) {
				case 0:		c1 = (((color_t)p[0]) << 4)|(((color_t)p[1])>>4);				break;
				case 1:		c1 = (((color_t)p[1]&0x0F) << 8)|((color_t)p[1]);	p += 3;		break;
				}

				/* Check for line or buffer wrapping */
				if (++x >= g->p.cx) {
					x = 0;
					p += lg;
					pnum += g->p.x2 - g->p.cx;
					if (++y >= g->p.cy) {
						y = 0;
						p = (const uint8_t *)buffer;
						pnum = pstart;
					}
				}

				/* Get the next pixel */
				switch(pnum++ & 1) {
				case 0:		c2 = (((color_t)p[0]) << 4)|(((color_t)p[1])>>4);				break;
				case 1:		c2 = (((color_t)p[1]&0x0F) << 8)|((color_t)p[1]);	p += 3;		break;
				}

				/* Check for line or buffer wrapping */
				if (++x >= g->p.cx) {
					x = 0;
					p += lg;
					pnum += g->p.x2 - g->p.cx;
					if (++y >= g->p.cy) {
						y = 0;
						p = (const uint8_t *)buffer;
						pnum = pstart;
					}
				}

				/* Write the pair of pixels to the display */
				write_data3(((c1 >> 4) & 0xFF), (((c1 << 4) & 0xF0)|((c2 >> 8) & 0x0F)), (c2 & 0xFF));
			}
		#endif

		/* All done */
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
			acquire_bus();
			switch((powermode_t)g->p.ptr) {
				case powerOff:
					set_backlight(0);									// Turn off the backlight
					write_cmd(DISOFF);									// Turn off the display
					write_cmd1(PWRCTR, 0x00);							// Power control - all off
					write_cmd(SLPIN);									// Sleep in
					write_cmd(OSCOFF);									// Internal oscillator off
					break;
				case powerOn:
					write_cmd(OSCON);									// Internal oscillator on
					write_cmd(SLPOUT);									// Sleep out
					write_cmd1(PWRCTR, 0x0F);							// Power control - reference voltage regulator on, circuit voltage follower on, BOOST ON
					write_cmd2(VOLCTR, g->g.Contrast, 0x03);			// Voltage control (contrast setting)
					delayms(100);										// Allow power supply to stabilise
					write_cmd(DISON);									// Turn on the display
					write_cmd(PTLOUT);									// Remove sleep window
					set_backlight(g->g.Backlight);						// Turn on the backlight
					break;
				case powerSleep:
					write_cmd(OSCON);									// Internal oscillator on
					write_cmd(SLPOUT);									// Sleep out
					write_cmd1(PWRCTR, 0x0F);							// Power control - reference voltage regulator on, circuit voltage follower on, BOOST ON
					write_cmd2(VOLCTR, g->g.Contrast, 0x03);			// Voltage control (contrast setting)
					delayms(100);										// Allow power supply to stabilise
					write_cmd(DISON);									// Turn on the display
					write_cmd2(PTLIN, GDISP_SLEEP_POS/4, (GDISP_SLEEP_POS+GDISP_SLEEP_SIZE)/4);	// Sleep Window
					set_backlight(g->g.Backlight);						// Turn on the backlight
					break;
				case powerDeepSleep:
					write_cmd(OSCON);									// Internal oscillator on
					write_cmd(SLPOUT);									// Sleep out
					write_cmd1(PWRCTR, 0x0F);							// Power control - reference voltage regulator on, circuit voltage follower on, BOOST ON
					write_cmd2(VOLCTR, g->g.Contrast, 0x03);			// Voltage control (contrast setting)
					delayms(100);										// Allow power supply to stabilise
					write_cmd(DISON);									// Turn on the display
					write_cmd2(PTLIN, GDISP_SLEEP_POS/4, (GDISP_SLEEP_POS+GDISP_SLEEP_SIZE)/4);	// Sleep Window
					set_backlight(0);									// Turn off the backlight
					break;
				default:
					release_bus();
					return;
			}
			release_bus();
			g->g.Powermode = (powermode_t)g->p.ptr;
			return;
		#if GDISP_NOKIA_ORIENTATION
			case GDISP_CONTROL_ORIENTATION:
				if (g->g.Orientation == (orientation_t)g->p.ptr)
					return;
				acquire_bus();
				switch((orientation_t)g->p.ptr) {
					case GDISP_ROTATE_0:
						write_cmd3(DATCTL, 0x00, 0x00, 0x02);	// P1: page normal, column normal, scan in column direction
						g->g.Height = GDISP_SCREEN_HEIGHT;
						g->g.Width = GDISP_SCREEN_WIDTH;
						break;
					case GDISP_ROTATE_90:
						write_cmd3(DATCTL, 0x05, 0x00, 0x02);	// P1: page reverse, column normal, scan in page direction
						g->g.Height = GDISP_SCREEN_WIDTH;
						g->g.Width = GDISP_SCREEN_HEIGHT;
						break;
					case GDISP_ROTATE_180:
						write_cmd3(DATCTL, 0x03, 0x00, 0x02);	// P1: page reverse, column reverse, scan in column direction
						g->g.Height = GDISP_SCREEN_HEIGHT;
						g->g.Width = GDISP_SCREEN_WIDTH;
						break;
					case GDISP_ROTATE_270:
						write_cmd3(DATCTL, 0x06, 0x00, 0x02);	// P1: page normal, column reverse, scan in page direction
						g->g.Height = GDISP_SCREEN_WIDTH;
						g->g.Width = GDISP_SCREEN_HEIGHT;
						break;
					default:
						release_bus();
						return;
				}
				release_bus();
				g->g.Orientation = (orientation_t)g->p.ptr;
				return;
		#endif
		case GDISP_CONTROL_BACKLIGHT:
			if ((unsigned)g->p.ptr > 100) g->p.ptr = (void *)100;
			set_backlight((unsigned)g->p.ptr);
			g->g.Backlight = (unsigned)g->p.ptr;
			return;
		case GDISP_CONTROL_CONTRAST:
			if ((unsigned)g->p.ptr > 100) g->p.ptr = (void *)100;
			acquire_bus();
			write_cmd2(VOLCTR, (unsigned)g->p.ptr, 0x03);
			release_bus();
			g->g.Contrast = (unsigned)g->p.ptr;
			return;
		}
	}
#endif

#endif /* GFX_USE_GDISP */
/** @} */
