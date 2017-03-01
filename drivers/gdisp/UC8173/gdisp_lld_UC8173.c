/*
 * This file is subject to the terms of the GFX License. If a copy of
 * the license was not distributed with this file, you can obtain one at:
 *
 *              http://ugfx.org/license.html
 */

#include "gfx.h"

#if GFX_USE_GDISP

#define GDISP_DRIVER_VMT			GDISPVMT_UC8173
#include "gdisp_lld_config.h"
#include "../../../src/gdisp/gdisp_driver.h"
#include "UC8173.h"
#include "board_UC8173.h"

#if defined(GDISP_SCREEN_WIDTH)
	#warning "GDISP: This low level driver does not support setting a screen size. It is being ignored."
	#define GDISP_SCREEN_WIDTH		240
#endif
#if defined(GDISP_SCREEN_HEIGHT)
	#warning "GDISP: This low level driver does not support setting a screen size. It is being ignored."
	#define GDISP_SCREEN_HEIGHT		240
#endif

#ifndef GDISP_SCREEN_HEIGHT
	#define GDISP_SCREEN_HEIGHT		240
#endif
#ifndef GDISP_SCREEN_WIDTH
	#define GDISP_SCREEN_WIDTH		240
#endif

#define PRIV(g)						((UC8173_Private*)((g)->priv))
#define FRAMEBUFFER(g)				((uint8_t *)(PRIV(g)+1))
#define GDISP_FLG_NEEDFLUSH			(GDISP_FLG_DRIVER << 0)

#if GDISP_LLD_PIXELFORMAT == GDISP_PIXELFORMAT_MONO
	#define LINE_BYTES					(GDISP_SCREEN_WIDTH/8)
	#define WRITEBUFCMD					DTM4
	#define xyaddr(x, y)				(((x)>>3) + ((y) * LINE_BYTES))
	//#define xybit(x, c)				((c) << ((x) & 7))			// This one has the wrong order of the pixels inside the byte
	#define xybit(x, c)					((c) << (7-((x) & 7)))
#elif  GDISP_LLD_PIXELFORMAT == GDISP_PIXELFORMAT_GRAY4
	#define LINE_BYTES					(GDISP_SCREEN_WIDTH/4)
	#define WRITEBUFCMD					DTM2						// NOT SURE THIS IS RIGHT - MAY NEED TO USE DTM0 and then send a refresh???
	#define xyaddr(x, y)				(((x)>>2) + ((y) * LINE_BYTES))
	//#define xybit(x, c)				((c) << (((x) & 3)<<1))		// This one has the wrong order of the pixels inside the byte
	#define xybit(x, c)					((c) << (6-((x) & 3)<<1))
#else
	#error "UC8173: Unsupported driver color format"
#endif

typedef struct UC8173_Private {
	coord_t flushWindowX;
	coord_t flushWindowY;
	coord_t flushWindowWidth;
	coord_t flushWindowHeight;
} UC8173_Private;

// This function rounds a given integer up to a specified multiple. Note, multiple must be a power of 2!
static GFXINLINE void _roundUp(coord_t* numToRound, uint8_t multiple)
{
	*numToRound = (*numToRound + multiple - 1) & ~(multiple - 1);
}

static GFXINLINE void _wait_for_busy_high(GDisplay* g)
{
	while (!getpin_busy(g));
}

static GFXINLINE void _wait_for_busy_low(GDisplay* g)
{
	while (getpin_busy(g));
}

static void _load_lut(GDisplay* g, uint32_t LUT, uint8_t* LUT_Value)
{
	int i,counter;
	int MODE = 2;

	if(MODE == 0)
		counter = 512;  //512
	else
		counter = 42; 
	if(LUT == 0x26)
		counter = 128;

	write_cmd(g, LUT);
	for(i = 0; i < counter; i++) {
		write_data(g, *LUT_Value);
		LUT_Value++;
	}
}

void _load_lut2(GDisplay* g, uint32_t LUT, uint8_t* LUT_Value, uint32_t LUT_Counter)
{
	uint32_t i;

	write_cmd(g, LUT);
	for (i = 0; i < LUT_Counter; i++) {
		write_data(g, *LUT_Value);
		LUT_Value++;
	}
}

static void _upload_Temperature_LUT(GDisplay* g)
{  
	_load_lut2(g, LUT_KWVCOM, &_lut_temperature[0], 32);
	_load_lut2(g, LUT_KW, &_lut_temperature[32], 512);
	_load_lut2(g, LUT_FT, &_lut_temperature[544], 128);
}

static void _clear_lut(GDisplay* g)
{
	write_cmd(g, PON);
	_wait_for_busy_high(g);

	_load_lut(g, LUT_KW, _lut_none);
	_load_lut(g, LUT_KWVCOM, _lut_none);

    write_cmd(g, POF); 
    _wait_for_busy_low(g);
}

static void _invertFramebuffer(GDisplay* g)
{
	uint32_t i;

	for (i = 0; i < LINE_BYTES*GDISP_SCREEN_HEIGHT; i++) {
		FRAMEBUFFER(g)[i] = ~(FRAMEBUFFER(g)[i]);
	}
	
	// We should flush these changes to the display controller framebuffer at some point
	g->flags |= GDISP_FLG_NEEDFLUSH;
}

LLDSPEC bool_t gdisp_lld_init(GDisplay* g)
{
	// Allocate the private area plus the framebuffer
	g->priv = gfxAlloc(sizeof(UC8173_Private) + LINE_BYTES*GDISP_SCREEN_HEIGHT);
	if (!g->priv) {
		return FALSE;
	}

	// Initialize the private area
	PRIV(g)->flushWindowX = 0;
	PRIV(g)->flushWindowY = 0;
	PRIV(g)->flushWindowWidth = GDISP_SCREEN_WIDTH;
	PRIV(g)->flushWindowHeight = GDISP_SCREEN_HEIGHT;

	// Initialise the board interface
	if (!init_board(g)) {
		return FALSE;
	}

	// Hardware reset
	setpin_reset(g, FALSE);
	gfxSleepMilliseconds(100);
	setpin_reset(g, TRUE);
	gfxSleepMilliseconds(1000);

	// Acquire the bus
	acquire_bus(g);

	// Booster soft-start
	write_cmd(g, BTST);
	write_data(g, 0x17);   //0x17
	write_data(g, 0x97);   //0x97
	write_data(g, 0x20);   //0x20
  
	// Power settings
	write_cmd(g, PWR);
	write_data(g, 0x03);
	write_data(g, 0x00);
	write_data(g, 0x2B);  //1C 2B
	write_data(g, 0x2B);  //1C 2B
	write_data(g, 0x00);

	// Power-on
	write_cmd(g, PON);
	_wait_for_busy_high(g);

	// Panel setting register
	write_cmd(g, PSR);
	write_data(g, 0x0F);  //0x0B
	write_data(g, 0x86);  //0x86

	// Power-off sequence
	write_cmd(g, PFS);
	write_data(g, 0x00);

	// PLL control
	write_cmd(g, LPRD);
	write_data(g, 0x25);

	// Internal temperature sensor enable
	write_cmd(g, TSE);
	write_data(g, 0x00);		// Use internal temperature sensor

	// VCOM and data interval settings
	write_cmd(g, CDI);
	write_data(g, 0xE1);
	write_data(g, 0x20);
	write_data(g, 0x10);
  
	// Set display panel resolution
	write_cmd(g, TRES);
	write_data(g, 0xEF);
	write_data(g, 0x00);
	write_data(g, 0xEF);

	// Undocumented register, taken from sample code
	write_cmd(g, GDS);
	write_data(g, 0xA9);
	write_data(g, 0xA9);
	write_data(g, 0xEB);
	write_data(g, 0xEB);
	write_data(g, 0x02);

	// Auto measure VCOM
	write_cmd(g, AMV);
	write_data(g, 0x11);		// 5 seconds, enabled

	_wait_for_busy_high(g);

	// Get current VCOM value
	// write_cmd(g, VV);
	// unsigned char vcom_temp = spi_9b_get();
	// vcom_temp = vcom_temp + 4;
	// Auto_VCOM = vcom_temp;

	// VCM_DC setting
	write_cmd(g, VDCS);
	write_data(g, 0x12);		// Write vcom_temp here
  
	// Undocumented register, taken from sample code
	write_cmd(g, VBDS);
	write_data(g, 0x22);
  
	// Undocumented register, taken from sample code
	write_cmd(g, LVSEL);
	write_data(g, 0x02);
  
	// Undocumented register, taken from sample code
	write_cmd(g, GBS);
	write_data(g, 0x02);
	write_data(g, 0x02);
  
	// Undocumented register, taken from sample code
	write_cmd(g, GSS);
	write_data(g, 0x02);
	write_data(g, 0x02);
  
	// Undocumented register, taken from sample code
	write_cmd(g, DF);			// For REGAL (???)
	write_data(g, 0x1F);
	
	// Clear the look-up table
	_clear_lut(g);	
	
	// Finish Init
	post_init_board(g);

 	// Release the bus
	release_bus(g);

	// Initialise the GDISP structure
	g->g.Width = GDISP_SCREEN_WIDTH;
	g->g.Height = GDISP_SCREEN_HEIGHT;
	g->g.Orientation = GDISP_ROTATE_0;
	g->g.Powermode = powerOn;
	g->g.Backlight = 0;
	g->g.Contrast = 0;

	return TRUE;
}

#if GDISP_HARDWARE_FLUSH
	LLDSPEC void gdisp_lld_flush(GDisplay* g)
	{
		coord_t y;

		// Don't flush unless we really need to
		if (!(g->flags & GDISP_FLG_NEEDFLUSH)) {
			return;
		}
		
		// Round the flushing window width and height up to the next multiple of four
		_roundUp(&(PRIV(g)->flushWindowWidth), 4);
		_roundUp(&(PRIV(g)->flushWindowWidth), 4);

		// Acquire the bus to communicate with the display controller
		acquire_bus(g);
		
		// Upload the new temperature LUT
		_upload_Temperature_LUT(g);

		// Setup the window
		write_cmd(g, DTMW);
		write_data(g, (uint8_t)((PRIV(g)->flushWindowX >> 0) & 0xFF));
		write_data(g, (uint8_t)((PRIV(g)->flushWindowY >> 8) & 0x03));
		write_data(g, (uint8_t)((PRIV(g)->flushWindowY >> 0) & 0xFF));
		write_data(g, (uint8_t)((((PRIV(g)->flushWindowWidth)-1) >> 0) & 0xFF));
		write_data(g, (uint8_t)((((PRIV(g)->flushWindowHeight)-1) >> 8) & 0x03));
		write_data(g, (uint8_t)((((PRIV(g)->flushWindowHeight)-1) >> 0) & 0xFF));

		// Dump our framebuffer
		// Note: The display controller doesn't allow changing the vertical scanning direction
		// so we have to manually send the lines "the other way around" here.
		write_cmd(g, WRITEBUFCMD);
		for (y = GDISP_SCREEN_HEIGHT-1; y >= 0; y--) {
			write_data_burst(g, FRAMEBUFFER(g)+y*LINE_BYTES, LINE_BYTES);
		}

		// Power-up the DC/DC converter to update the display panel
		write_cmd(g, PON);
		_wait_for_busy_high(g);

		// Refresh the panel contents
		write_cmd(g, DRF);
		write_data(g, 0x08);	// Enable REGAL function
		write_data(g, 0x00);
		write_data(g, 0x00);
		write_data(g, 0x00);
		write_data(g, 0xEF);
		write_data(g, 0x00);
		write_data(g, 0xEF);
		_wait_for_busy_high(g);

		// Power-down the DC/DC converter to make all the low-power pussys happy
		write_cmd(g, POF);
		_wait_for_busy_low(g);

		// Release the bus again
		release_bus(g);

		// Clear the 'need-flushing' flag
		g->flags &=~ GDISP_FLG_NEEDFLUSH;
	}
#endif

#if GDISP_HARDWARE_DRAWPIXEL
	LLDSPEC void gdisp_lld_draw_pixel(GDisplay* g)
	{
		coord_t			x, y;
		LLDCOLOR_TYPE	*p;

		// Handle the different possible orientations
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
			x = GDISP_SCREEN_WIDTH-1 - g->p.y;
			y = g->p.x;
			break;
		}

		// Modify the framebuffer content
		p = &FRAMEBUFFER(g)[xyaddr(x,y)];
		*p &=~ xybit(x, LLDCOLOR_MASK());
		*p |= xybit(x, gdispColor2Native(g->p.color));

//#warning ToDo
// There appears to be an issue in the silicone, still talking to the manufacturer about this one. Update will follow!
#if 0
		// Update the flush window region
		if (g->flags & GDISP_FLG_NEEDFLUSH) {
			if (x < PRIV(g)->flushWindowX)
				PRIV(g)->flushWindowX = x;
			if (y < PRIV(g)->flushWindowY)
				PRIV(g)->flushWindowY = y;
			if (x > PRIV(g)->flushWindowX + PRIV(g)->flushWindowWidth)
				PRIV(g)->flushWindowWidth = 
		} else {
			PRIV(g)->flushWindowX = x;
			PRIV(g)->flushWindowY = y;
			PRIV(g)->flushWindowWidth = 1;
			PRIV(g)->flushWindowHeight = 1;
		}
#else
		PRIV(g)->flushWindowX = 0;
		PRIV(g)->flushWindowY = 0;
		PRIV(g)->flushWindowWidth = GDISP_SCREEN_WIDTH;
		PRIV(g)->flushWindowHeight = GDISP_SCREEN_HEIGHT;
#endif

		// We should flush these changes to the display controller framebuffer at some point
		g->flags |= GDISP_FLG_NEEDFLUSH;
	}
#endif
	
#if GDISP_NEED_CONTROL && GDISP_HARDWARE_CONTROL
	LLDSPEC void gdisp_lld_control(GDisplay* g) {
		switch(g->p.x) {
		case GDISP_CONTROL_INVERT:
			_invertFramebuffer(g);
			break;
		
		default:
			break;
		}
	}
#endif

#endif // GFX_USE_GDISP
