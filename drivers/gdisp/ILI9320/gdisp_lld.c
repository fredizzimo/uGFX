/*
 * This file is subject to the terms of the GFX License. If a copy of
 * the license was not distributed with this file, you can obtain one at:
 *
 *              http://ugfx.org/license.html
 */

/**
 * @file    drivers/gdisp/ILI9320/gdisp_lld.c
 * @brief   GDISP Graphics Driver subsystem low level driver source for the ILI9320 display.
 */

#include "gfx.h"

#if GFX_USE_GDISP

/* This controller is only ever used with a 240 x 320 display */
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
/* Driver local variables.                                                   */
/*===========================================================================*/
uint32_t DISPLAY_CODE;

/*===========================================================================*/
/* Driver local functions.                                                   */
/*===========================================================================*/

#define dummy_read()				{ volatile uint16_t dummy; dummy = read_data(); (void) dummy; }
#define write_reg(reg, data)		{ write_index(reg); write_data(data); }

static void set_viewport(uint16_t x, uint16_t y, uint16_t cx, uint16_t cy) {
	switch(g->g.Orientation) {
		case GDISP_ROTATE_0:
		case GDISP_ROTATE_180:
			write_reg(0x0050, x);
			write_reg(0x0051, x + cx - 1);
			write_reg(0x0052, y);
			write_reg(0x0053, y + cy - 1);
			write_reg(0x0020, x);
			write_reg(0x0021, y);
			break;

		case GDISP_ROTATE_90:
		case GDISP_ROTATE_270:
			write_reg(0x0050, y);
			write_reg(0x0051, y + cy - 1);
			write_reg(0x0052, x);
			write_reg(0x0053, x + cx - 1);
			write_reg(0x0020, y);
			write_reg(0x0021, x);
			break;
	}
	write_index(0x0022);
}

LLDSPEC bool_t gdisp_lld_init(GDISPDriver *g) {
	/* Initialise your display */
	init_board();

	/* Hardware reset */
	setpin_reset(TRUE);
	gfxSleepMicroseconds(1000);
	setpin_reset(FALSE);
	gfxSleepMicroseconds(1000);

	acquire_bus();
	write_index(0);				// Get controller version
	dummy_read();
    DISPLAY_CODE = read_data();
    write_reg(0x0000, 0x0001); //start Int. osc
    gfxSleepMicroseconds(500);
    write_reg(0x0001, 0x0100); //Set SS bit (shift direction of outputs is from S720 to S1)
    write_reg(0x0002, 0x0700); //select  the line inversion
    write_reg(0x0003, 0x1038); //Entry mode(Horizontal : increment,Vertical : increment, AM=1)
    write_reg(0x0004, 0x0000); //Resize control(No resizing)
    write_reg(0x0008, 0x0202); //front and back porch 2 lines
    write_reg(0x0009, 0x0000); //select normal scan
    write_reg(0x000A, 0x0000); //display control 4
    write_reg(0x000C, 0x0000); //system interface(2 transfer /pixel), internal sys clock,
    write_reg(0x000D, 0x0000); //Frame marker position
    write_reg(0x000F, 0x0000); //selects clk, enable and sync signal polarity,
    write_reg(0x0010, 0x0000); //
    write_reg(0x0011, 0x0000); //power control 2 reference voltages = 1:1,
    write_reg(0x0012, 0x0000); //power control 3 VRH
    write_reg(0x0013, 0x0000); //power control 4 VCOM amplitude
    gfxSleepMicroseconds(500);
    write_reg(0x0010, 0x17B0); //power control 1 BT,AP
    write_reg(0x0011, 0x0137); //power control 2 DC,VC
    gfxSleepMicroseconds(500);
    write_reg(0x0012, 0x0139); //power control 3 VRH
    gfxSleepMicroseconds(500);
    write_reg(0x0013, 0x1d00); //power control 4 vcom amplitude
    write_reg(0x0029, 0x0011); //power control 7 VCOMH
    gfxSleepMicroseconds(500);
    write_reg(0x0030, 0x0007);
    write_reg(0x0031, 0x0403);
    write_reg(0x0032, 0x0404);
    write_reg(0x0035, 0x0002);
    write_reg(0x0036, 0x0707);
    write_reg(0x0037, 0x0606);
    write_reg(0x0038, 0x0106);
    write_reg(0x0039, 0x0007);
    write_reg(0x003c, 0x0700);
    write_reg(0x003d, 0x0707);
    write_reg(0x0020, 0x0000); //starting Horizontal GRAM Address
    write_reg(0x0021, 0x0000); //starting Vertical GRAM Address
    write_reg(0x0050, 0x0000); //Horizontal GRAM Start Position
    write_reg(0x0051, 0x00EF); //Horizontal GRAM end Position
    write_reg(0x0052, 0x0000); //Vertical GRAM Start Position
    write_reg(0x0053, 0x013F); //Vertical GRAM end Position
	switch (DISPLAY_CODE) {   
		case 0x9320:
        	write_reg(0x0060, 0x2700); //starts scanning from G1, and 320 drive lines
        	break;
      	case 0x9325:
     		write_reg(0x0060, 0xA700); //starts scanning from G1, and 320 drive lines
			break;
	}

    write_reg(0x0061, 0x0001); //fixed base display
    write_reg(0x006a, 0x0000); //no scroll
    write_reg(0x0090, 0x0010); //set Clocks/Line =16, Internal Operation Clock Frequency=fosc/1,
    write_reg(0x0092, 0x0000); //set gate output non-overlap period=0
    write_reg(0x0093, 0x0003); //set Source Output Position=3
    write_reg(0x0095, 0x0110); //RGB interface(Clocks per line period=16 clocks)
    write_reg(0x0097, 0x0110); //set Gate Non-overlap Period 0 locksc
    write_reg(0x0098, 0x0110); //
    write_reg(0x0007, 0x0173); //display On
    release_bus();

	// Turn on the backlight
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

#if GDISP_NEED_CONTROL && GDISP_HARDWARE_CONTROL
	LLDSPEC void gdisp_lld_control(GDISPDriver *g) {
		switch(g->p.x) {
		case GDISP_CONTROL_POWER:
			if (g->g.Powermode == (powermode_t)g->p.ptr)
				return;
			switch((powermode_t)g->p.ptr) {
				case powerOff:
					acquire_bus();
					write_reg(0x0007, 0x0000);
					write_reg(0x0010, 0x0000);
					write_reg(0x0011, 0x0000);
					write_reg(0x0012, 0x0000);
					write_reg(0x0013, 0x0000);
					release_bus();

					set_backlight(0);
					break;

				case powerOn:
					//*************Power On sequence ******************//
					acquire_bus();
					write_reg(0x0010, 0x0000); /* SAP, BT[3:0], AP, DSTB, SLP, STB */
					write_reg(0x0011, 0x0000); /* DC1[2:0], DC0[2:0], VC[2:0] */
					write_reg(0x0012, 0x0000); /* VREG1OUT voltage */
					write_reg(0x0013, 0x0000); /* VDV[4:0] for VCOM amplitude */
					gfxSleepMicroseconds(2000);            /* Dis-charge capacitor power voltage */
					write_reg(0x0010, 0x17B0); /* SAP, BT[3:0], AP, DSTB, SLP, STB */
					write_reg(0x0011, 0x0147); /* DC1[2:0], DC0[2:0], VC[2:0] */
					gfxSleepMicroseconds(500);
					write_reg(0x0012, 0x013C); /* VREG1OUT voltage */
					gfxSleepMicroseconds(500);
					write_reg(0x0013, 0x0E00); /* VDV[4:0] for VCOM amplitude */
					write_reg(0x0029, 0x0009); /* VCM[4:0] for VCOMH */
					gfxSleepMicroseconds(500);
					write_reg(0x0007, 0x0173); /* 262K color and display ON */
					release_bus();

					set_backlight(g->g.Backlight);
					if(g->g.Powermode != powerSleep || g->g.Powermode != powerDeepSleep)
						gdisp_lld_init();
					break;

				case powerSleep:
					acquire_bus();
					write_reg(0x0007, 0x0000); /* display OFF */
					write_reg(0x0010, 0x0000); /* SAP, BT[3:0], APE, AP, DSTB, SLP */
					write_reg(0x0011, 0x0000); /* DC1[2:0], DC0[2:0], VC[2:0] */
					write_reg(0x0012, 0x0000); /* VREG1OUT voltage */
					write_reg(0x0013, 0x0000); /* VDV[4:0] for VCOM amplitude */
					gfxSleepMicroseconds(2000); /* Dis-charge capacitor power voltage */
					write_reg(0x0010, 0x0002); /* SAP, BT[3:0], APE, AP, DSTB, SLP */
					release_bus();

					set_backlight(0);
					break;

				case powerDeepSleep:
					acquire_bus();
					write_reg(0x0007, 0x0000); /* display OFF */
					write_reg(0x0010, 0x0000); /* SAP, BT[3:0], APE, AP, DSTB, SLP */
					write_reg(0x0011, 0x0000); /* DC1[2:0], DC0[2:0], VC[2:0] */
					write_reg(0x0012, 0x0000); /* VREG1OUT voltage */
					write_reg(0x0013, 0x0000); /* VDV[4:0] for VCOM amplitude */
					gfxSleepMicroseconds(2000); /* Dis-charge capacitor power voltage */
					write_reg(0x0010, 0x0004); /* SAP, BT[3:0], APE, AP, DSTB, SLP */
					release_bus();

					set_backlight(0);
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
					write_reg(0x0001, 0x0100);
					write_reg(0x0003, 0x1038);
					write_reg(0x0060, 0x2700);
					release_bus();

					g->g.Height = GDISP_SCREEN_HEIGHT;
					g->g.Width = GDISP_SCREEN_WIDTH;
					break;

				case GDISP_ROTATE_90:
					acquire_bus();
					write_reg(0x0001, 0x0100);
					write_reg(0x0003, 0x1030);
					write_reg(0x0060, 0x2700);
					release_bus();

					g->g.Height = GDISP_SCREEN_WIDTH;
					g->g.Width = GDISP_SCREEN_HEIGHT;
					break;

				case GDISP_ROTATE_180:
					acquire_bus();
					write_reg(0x0001, 0x0000);
					write_reg(0x0003, 0x1030);
					write_reg(0x0060, 0x2700);
					release_bus();

					g->g.Height = GDISP_SCREEN_HEIGHT;
					g->g.Width = GDISP_SCREEN_WIDTH;
					break;

				case GDISP_ROTATE_270:
					acquire_bus();
					write_reg(0x0001, 0x0000);
					write_reg(0x0003, 0x1038);
					write_reg(0x0060, 0xA700);
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
			default:
				return;
		}
	}
#endif

#endif /* GFX_USE_GDISP */
