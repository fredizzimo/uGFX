/*
 * This file is subject to the terms of the GFX License. If a copy of
 * the license was not distributed with this file, you can obtain one at:
 *
 *              http://ugfx.org/license.html
 */

#include "gfx.h"

#if GFX_USE_GDISP

#define GDISP_DRIVER_VMT			GDISPVMT_SSD1963
#include "drivers/gdisp/SSD1963/gdisp_lld_config.h"
#include "src/gdisp/driver.h"

#define CALC_PERIOD(w,b,f,p)				(p+b+w+f)
#define CALC_FPR(w,h,hb,hf,hp,vb,vf,vp,fps)	((fps * CALC_PERIOD(w,hb,hf,hp) * CALC_PERIOD(h,vb,vf,vp) * 1048576)/100000000)


typedef struct LCD_Parameters {
	coord_t		width, height;					// Panel width and height
	uint16_t	hbporch;						// Horizontal Back Porch
	uint16_t	hfporch;						// Horizontal Front Porch
	uint16_t	hpulse;							// Horizontal Pulse
	uint16_t	hperiod;						// Horizontal Period (Total)
	uint16_t	vbporch;						// Vertical Back Porch
	uint16_t	vfporch;						// Vertical Front Porch
	uint16_t	vpulse;							// Vertical Pulse
	uint16_t	vperiod;						// Vertical Period (Total)
	uint32_t	fpr;							// Calculated FPR
	uint16_t	flags;							// For command "SSD1963_SET_GDISP_MODE"
		/* Set the pannel data width */
		#define LCD_PANEL_DATA_WIDTH_24BIT 				(1<<5)						// 18bit default
		/* Set the color deeph enhancement */
		#define LCD_PANEL_ENABLE_FRC					((1<<3) | (1<<4))
		#define LCD_PANEL_ENABLE_DITHERING				(1<<4)						// no enhancement default
		/* Set the dot clock pulse polarity */
		#define LCD_PANEL_LSHIFT_FALLING_EDGE			(1<<2)						// default rising edge
		/* Set the horizontal sync pulse polarity */
		#define LCD_PANEL_LLINE_ACTIVE_HIGH				(1<<1)						// default active low
		/* Set the vertical sync pulse polarity */
		#define LCD_PANEL_LFRAME_ACTIVE_HIGH			(1<0)						// default active low
		/* Set the lcd panel mode */
		#define LCD_PANEL_MODE_TTL						((1<<7) << 8)				// default mode is Hsync+Vsync +DE
		/* Set the lcd panel interface type */										// default TFT mode
		#define LCD_PANEL_TYPE_SERIAL_RGB_MODE			((1<<6) << 8)				// Serial RGB mode
		#define LCD_PANEL_TYPE_SERIAL_RGB_DUMMY_MODE	(((1<<5) | (1<<6)) << 8)	// Serial RGB+dummy mode



} LCD_Parameters;

#include "board_SSD1963.h"

/*===========================================================================*/
/* Driver local definitions.                                                 */
/*===========================================================================*/

#ifndef GDISP_INITIAL_CONTRAST
	#define GDISP_INITIAL_CONTRAST	50
#endif
#ifndef GDISP_INITIAL_BACKLIGHT
	#define GDISP_INITIAL_BACKLIGHT	90
#endif

/*===========================================================================*/
/* Driver local functions.                                                   */
/*===========================================================================*/

#include "drivers/gdisp/SSD1963/ssd1963.h"

#define dummy_read(g)               { volatile uint16_t dummy; dummy = read_data(g); (void) dummy; }
#define write_reg(g, reg, data)		{ write_index(g, reg); write_data(g, data); }
#define write_data16(g, data)		{ write_data(g, (data)>>8); write_data(g, (data) & 0xFF); }

static inline void set_viewport(GDisplay* g) {

	switch(g->g.Orientation) {
		default:
		case GDISP_ROTATE_0:
			write_index(g, SSD1963_SET_COLUMN_ADDRESS);
			write_data16(g, g->p.x);
			write_data16(g, g->p.x+g->p.cx-1);
			write_index(g, SSD1963_SET_PAGE_ADDRESS);
			write_data16(g, g->p.y);
			write_data16(g, g->p.y+g->p.cy-1);
			write_index(g, SSD1963_WRITE_MEMORY_START);
			break;
		case GDISP_ROTATE_90:
			write_index(g, SSD1963_SET_COLUMN_ADDRESS);
			write_data16(g, g->p.y);
			write_data16(g, g->p.y+g->p.cy-1);
			write_index(g, SSD1963_SET_PAGE_ADDRESS);
			write_data16(g, GDISP_SCREEN_HEIGHT-1 - (g->p.x+g->p.cx-1) );
			write_data16(g, GDISP_SCREEN_HEIGHT-1 - (g->p.x));
			write_index(g, SSD1963_WRITE_MEMORY_START);
			break;
		case GDISP_ROTATE_180:
			write_index(g, SSD1963_SET_COLUMN_ADDRESS);
			write_data16(g, GDISP_SCREEN_WIDTH-1 - (g->p.x+g->p.cx-1));
			write_data16(g, GDISP_SCREEN_WIDTH-1 - (g->p.x));
			write_index(g, SSD1963_SET_PAGE_ADDRESS);
			write_data16(g, GDISP_SCREEN_HEIGHT-1 - (g->p.y+g->p.cy-1));
			write_data16(g, GDISP_SCREEN_HEIGHT-1 - (g->p.y));
			write_index(g, SSD1963_WRITE_MEMORY_START);
			break;
		case GDISP_ROTATE_270:
			write_index(g, SSD1963_SET_COLUMN_ADDRESS);
			write_data16(g, GDISP_SCREEN_WIDTH-1 - (g->p.y+g->p.cy-1));
			write_data16(g, GDISP_SCREEN_WIDTH-1 - (g->p.y));
			write_index(g, SSD1963_SET_PAGE_ADDRESS);
			write_data16(g, g->p.x);
			write_data16(g, g->p.x+g->p.cx-1);
			write_index(g, SSD1963_WRITE_MEMORY_START);
			break;
	}




}

/**
 * The backlight is controlled by the controller.
 */
static inline void set_backlight(GDisplay *g, uint8_t percent) {
	//duty_cycle is 00..FF
	//Work in progress: the SSD1963 has a built-in PWM, its output can
	//be used by a Dynamic Background Control or by a host (user)
	//Check your LCD's hardware, the PWM connection is default left open and instead
	//connected to a LED connection on the breakout board
	write_index(g, SSD1963_SET_PWM_CONF);		//set PWM for BackLight
	write_data(g, 4);							// PWMF[7:0] = 4,
												// PWM signal frequency = PLL clock / (256 * (PWMF[7:0] + 1)) / 256
												//                      = aprox 366Hz for a PLL freq = 120MHz
	if ( percent==0xFF )						// use percent==0xFF to turn off SSD1963 pwm in power SLEEP or DEEP SLEEP mode
		write_data(g, 0x00);
	else
		write_data(g, (55+percent*2) & 0x00FF);
	write_data(g, 0x01);						// controlled by host (not DBC), enabled
	write_data(g, 0x00);						// DBC manual brightness; Set the brightness level: 00 Dimmest, FF Brightest
	write_data(g, 0x00);						// DBC minimum brightness; Set the minimum brightness level: 00 Dimmest, FF Brightest
	write_data(g, 0x00);						// Brightness prescaler of Transition Effect; Set the brightness prescaler: 0 Dimmest, F Brightest

}

/*===========================================================================*/
/* Driver interrupt handlers.                                                */
/*===========================================================================*/

/*===========================================================================*/
/* Driver exported functions.                                                */
/*===========================================================================*/

LLDSPEC bool_t gdisp_lld_init(GDisplay *g) {
	LCD_Parameters *	lcdp;

	// The private area for this controller is the LCD timings
	lcdp = (void *)&DisplayTimings[g->controllerdisplay];
	g->priv = lcdp;

	// Initialise the board interface
	init_board(g);

	// Hardware reset
	setpin_reset(g, TRUE);
	gfxSleepMilliseconds(20);
	setpin_reset(g, FALSE);
	gfxSleepMilliseconds(20);

	// Get the bus for the following initialisation commands
	acquire_bus(g);
	
	write_index(g, SSD1963_SOFT_RESET);
	gfxSleepMilliseconds(5);

	/* Driver PLL config */
	write_index(g, SSD1963_SET_PLL_MN);
	write_data(g, 35);								// PLLclk = REFclk (10Mhz) * 36 (360Mhz)
	write_data(g, 2);								// SYSclk = PLLclk / 3  (120MHz)
	write_data(g, 4);								// Apply calculation bit, else it is ignored
	write_reg(g, SSD1963_SET_PLL, 0x01);			// Enable PLL
	gfxSleepMilliseconds(5);
	write_reg(g, SSD1963_SET_PLL, 0x03);			// Use PLL
	gfxSleepMilliseconds(5);
	write_index(g, SSD1963_SOFT_RESET);
	gfxSleepMilliseconds(5);

	/* LCD panel parameters */
	write_index(g, SSD1963_SET_GDISP_MODE);
	write_data(g, lcdp->flags & 0xFF);
	write_data(g, (lcdp->flags >> 8) & 0xFF);
	write_data16(g, lcdp->width-1);
	write_data16(g, lcdp->height-1);
	write_data(g, 0x00);							// RGB

	write_reg(g, SSD1963_SET_PIXEL_DATA_INTERFACE, SSD1963_PDI_16BIT565);

	/* LCD Clock specs */
	write_index(g, SSD1963_SET_LSHIFT_FREQ);
	write_data(g, (lcdp->fpr >> 16) & 0xFF);
	write_data(g, (lcdp->fpr >> 8) & 0xFF);
	write_data(g, lcdp->fpr & 0xFF);

	write_index(g, SSD1963_SET_HORI_PERIOD);
	write_data16(g, lcdp->hperiod);
	write_data16(g, lcdp->hpulse + lcdp->hbporch);
	write_data(g, lcdp->hpulse);
	write_data(g, 0x00);
	write_data(g, 0x00);
	write_data(g, 0x00);

	write_index(g, SSD1963_SET_VERT_PERIOD);
	write_data16(g, lcdp->vperiod);
	write_data16(g, lcdp->vpulse + lcdp->vbporch);
	write_data(g, lcdp->vpulse);
	write_data(g, 0x00);
	write_data(g, 0x00);

	/* Tear effect indicator ON. This is used to tell the host MCU when the driver is not refreshing the panel (during front/back porch) */
	write_reg(g, SSD1963_SET_TEAR_ON, 0x00);


	/* Turn on */
	write_index(g, SSD1963_SET_DISPLAY_ON);

	/* Turn on the back-light */
	set_backlight(g, GDISP_INITIAL_BACKLIGHT);

	// Finish Init
	post_init_board(g);

	// Release the bus
	release_bus(g);

	/* Initialise the GDISP structure */
	g->g.Width = lcdp->width;
	g->g.Height = lcdp->height;
	g->g.Orientation = GDISP_ROTATE_0;
	g->g.Powermode = powerOn;
	g->g.Backlight = GDISP_INITIAL_BACKLIGHT;
	g->g.Contrast = GDISP_INITIAL_CONTRAST;
	return TRUE;
}

#if GDISP_HARDWARE_STREAM_WRITE
	LLDSPEC	void gdisp_lld_write_start(GDisplay *g) {
		acquire_bus(g);
		set_viewport(g);
	}
	LLDSPEC	void gdisp_lld_write_color(GDisplay *g) {
		write_data(g, gdispColor2Native(g->p.color));
	}
	LLDSPEC	void gdisp_lld_write_stop(GDisplay *g) {
		release_bus(g);
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
					acquire_bus(g);
					release_bus(g);
					break;
				case powerOn:
					acquire_bus(g);
					dummy_read(g);
					dummy_read(g);
					/* Wait for 1msec to let the PLL stable if was stopped by deep sleep mode  */
					gfxSleepMilliseconds(100);
					write_index(g, SSD1963_EXIT_SLEEP_MODE);
					gfxSleepMilliseconds(5);
					/* Restore the back-light */
					set_backlight(g, gdispGGetBacklight(g));
					release_bus(g);
					break;
				case powerSleep:
					acquire_bus(g);
					/* Turn off the back-light pwm from SSD1963 */
					set_backlight(g, 0xFF);
					write_index(g, SSD1963_ENTER_SLEEP_MODE);
					gfxSleepMilliseconds(5);
					release_bus(g);
					break;
				case powerDeepSleep:
					acquire_bus(g);
					/* Turn off the back-light pwm from SSD1963 */
					set_backlight(g, 0xFF);
					write_index(g, SSD1963_ENTER_SLEEP_MODE);
					write_index(g, SSD1963_SET_DEEP_SLEEP);
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
			switch((orientation_t)g->p.ptr) {
			case GDISP_ROTATE_0:
				acquire_bus(g);
				write_index(g, SSD1963_SET_ADDRESS_MODE);
				write_data(g, 0x00);
				release_bus(g);
				g->g.Height = ((LCD_Parameters *)g->priv)->height;
				g->g.Width = ((LCD_Parameters *)g->priv)->width;
				break;
			case GDISP_ROTATE_90:
				acquire_bus(g);
				write_index(g, SSD1963_SET_ADDRESS_MODE);
				write_data(g, SSD1963_ADDR_MODE_PAGE_ADDR_ORDER | SSD1963_ADDR_MODE_PAG_COL_ADDR_ORDER );
				release_bus(g);
				g->g.Height = ((LCD_Parameters *)g->priv)->width;
				g->g.Width = ((LCD_Parameters *)g->priv)->height;
				break;
			case GDISP_ROTATE_180:
				acquire_bus(g);
				write_index(g, SSD1963_SET_ADDRESS_MODE);
				write_data(g, SSD1963_ADDR_MODE_PAGE_ADDR_ORDER | SSD1963_ADDR_MODE_COL_ADDR_ORDER);
				release_bus(g);
				g->g.Height = ((LCD_Parameters *)g->priv)->height;
				g->g.Width = ((LCD_Parameters *)g->priv)->width;
				break;
			case GDISP_ROTATE_270:
				acquire_bus(g);
				write_index(g, SSD1963_SET_ADDRESS_MODE);
				write_data(g,  SSD1963_ADDR_MODE_COL_ADDR_ORDER | SSD1963_ADDR_MODE_PAG_COL_ADDR_ORDER );
				release_bus(g);
				g->g.Height = ((LCD_Parameters *)g->priv)->width;
				g->g.Width = ((LCD_Parameters *)g->priv)->height;
				break;
			default:
				return;
			}
			g->g.Orientation = (orientation_t)g->p.ptr;
			return;

		case GDISP_CONTROL_BACKLIGHT:
			if ((unsigned)g->p.ptr > 100)
				g->p.ptr = (void *)100;
			set_backlight(g, (unsigned)g->p.ptr);
			g->g.Backlight = (unsigned)g->p.ptr;
			return;

		//case GDISP_CONTROL_CONTRAST:
		default:
			return;
		}
	}
#endif

#endif /* GFX_USE_GDISP */
