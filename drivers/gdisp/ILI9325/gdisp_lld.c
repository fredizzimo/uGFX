/*
 * This file is subject to the terms of the GFX License. If a copy of
 * the license was not distributed with this file, you can obtain one at:
 *
 *              http://ugfx.org/license.html
 */

/**
 * @file    drivers/gdisp/ILI9325/gdisp_lld.c
 * @brief   GDISP Graphics Driver subsystem low level driver source for the ILI9325 display.
 *
 * @addtogroup GDISP
 * @{
 */

#include "gfx.h"

#if GFX_USE_GDISP /*|| defined(__DOXYGEN__)*/

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

// Some common routines and macros
#define dummy_read()				{ volatile uint16_t dummy; dummy = read_data(); (void) dummy; }
#define write_reg(reg, data)		{ write_index(reg); write_data(data); }

static void set_viewport(GDISPDriver* g) {
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
    write_reg(0x0022, color);
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

	// chinese code starts here
	write_reg(0x0000,0x0001);
	gfxSleepMilliseconds(10);

	write_reg(0x0015,0x0030);
	write_reg(0x0011,0x0040);
	write_reg(0x0010,0x1628);
	write_reg(0x0012,0x0000);
	write_reg(0x0013,0x104d);
	gfxSleepMilliseconds(10);
	write_reg(0x0012,0x0010);
	gfxSleepMilliseconds(10);
	write_reg(0x0010,0x2620);
	write_reg(0x0013,0x344d); //304d
	gfxSleepMilliseconds(10);

	write_reg(0x0001,0x0100);
	write_reg(0x0002,0x0300);
	write_reg(0x0003,0x1038);//0x1030
	write_reg(0x0008,0x0604);
	write_reg(0x0009,0x0000);
	write_reg(0x000A,0x0008);

	write_reg(0x0041,0x0002);
	write_reg(0x0060,0x2700);
	write_reg(0x0061,0x0001);
	write_reg(0x0090,0x0182);
	write_reg(0x0093,0x0001);
	write_reg(0x00a3,0x0010);
	gfxSleepMilliseconds(10);

	//################# void Gamma_Set(void) ####################//
	write_reg(0x30,0x0000);
	write_reg(0x31,0x0502);
	write_reg(0x32,0x0307);
	write_reg(0x33,0x0305);
	write_reg(0x34,0x0004);
	write_reg(0x35,0x0402);
	write_reg(0x36,0x0707);
	write_reg(0x37,0x0503);
	write_reg(0x38,0x1505);
	write_reg(0x39,0x1505);
	gfxSleepMilliseconds(10);

	//################## void Display_ON(void) ####################//
	write_reg(0x0007,0x0001);
	gfxSleepMilliseconds(10);
	write_reg(0x0007,0x0021);
	write_reg(0x0007,0x0023);
	gfxSleepMilliseconds(10);
	write_reg(0x0007,0x0033);
	gfxSleepMilliseconds(10);
	write_reg(0x0007,0x0133);

	// chinese code ends here

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
				gfxSleepMilliseconds(200);            /* Dis-charge capacitor power voltage */
				write_reg(0x0010, 0x17B0); /* SAP, BT[3:0], AP, DSTB, SLP, STB */
				write_reg(0x0011, 0x0147); /* DC1[2:0], DC0[2:0], VC[2:0] */
				gfxSleepMilliseconds(50);
				write_reg(0x0012, 0x013C); /* VREG1OUT voltage */
				gfxSleepMilliseconds(50);
				write_reg(0x0013, 0x0E00); /* VDV[4:0] for VCOM amplitude */
				write_reg(0x0029, 0x0009); /* VCM[4:0] for VCOMH */
				gfxSleepMilliseconds(50);
				write_reg(0x0007, 0x0173); /* 262K color and display ON */
				release_bus();
				set_backlight(g->g.Backlight);
				break;

			case powerSleep:
				acquire_bus();
				write_reg(0x0007, 0x0000); /* display OFF */
				write_reg(0x0010, 0x0000); /* SAP, BT[3:0], APE, AP, DSTB, SLP */
				write_reg(0x0011, 0x0000); /* DC1[2:0], DC0[2:0], VC[2:0] */
				write_reg(0x0012, 0x0000); /* VREG1OUT voltage */
				write_reg(0x0013, 0x0000); /* VDV[4:0] for VCOM amplitude */
				gfxSleepMilliseconds(200); /* Dis-charge capacitor power voltage */
				write_reg(0x0010, 0x0002); /* SAP, BT[3:0], APE, AP, DSTB, SLP */
				release_bus();
				gdisp_lld_backlight(0);
				break;

			case powerDeepSleep:
				acquire_bus();
				write_reg(0x0007, 0x0000); /* display OFF */
				write_reg(0x0010, 0x0000); /* SAP, BT[3:0], APE, AP, DSTB, SLP */
				write_reg(0x0011, 0x0000); /* DC1[2:0], DC0[2:0], VC[2:0] */
				write_reg(0x0012, 0x0000); /* VREG1OUT voltage */
				write_reg(0x0013, 0x0000); /* VDV[4:0] for VCOM amplitude */
				gfxSleepMilliseconds(200); /* Dis-charge capacitor power voltage */
				write_reg(0x0010, 0x0004); /* SAP, BT[3:0], APE, AP, DSTB, SLP */
				release_bus();
				gdisp_lld_backlight(0);
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
					write_reg(0x0001, 0x0000);
					write_reg(0x0003, 0x1030);
					write_reg(0x0060, 0x2700);
					release_bus();
					g->g.Height = GDISP_SCREEN_WIDTH;
					g->g.Width = GDISP_SCREEN_HEIGHT;
					break;

				case GDISP_ROTATE_180:
					acquire_bus();
					write_reg(0x0001, 0x0000);
					write_reg(0x0003, 0x1038);
					write_reg(0x0060, 0xa700);
					release_bus();
					g->g.Height = GDISP_SCREEN_HEIGHT;
					g->g.Width = GDISP_SCREEN_WIDTH;
					break;
	
				case GDISP_ROTATE_270:
					acquire_bus();
					write_reg(0x0001, 0x0100);
					write_reg(0x0003, 0x1030);
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
/** @} */

