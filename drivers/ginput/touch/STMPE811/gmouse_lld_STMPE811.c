/*
 * This file is subject to the terms of the GFX License. If a copy of
 * the license was not distributed with this file, you can obtain one at:
 *
 *              http://ugfx.org/license.html
 */

#include "gfx.h"

#if GFX_USE_GINPUT && GINPUT_NEED_MOUSE

#define GMOUSE_DRIVER_VMT		GMOUSEVMT_STMPE811
#include "src/ginput/driver_mouse.h"

#define GMOUSE_STMPE811_FLG_TOUCHED		(GMOUSE_FLG_DRIVER_FIRST<<0)

// Hardware definitions
#include "drivers/ginput/touch/STMPE811/stmpe811.h"

// Get the hardware interface
#include "gmouse_lld_STMPE811_board.h"

static bool_t MouseInit(GMouse* m, unsigned driverinstance) {
	if (!init_board(m, driverinstance))
		return FALSE;

	aquire_bus(m);

	write_reg(m, STMPE811_REG_SYS_CTRL1, 0x02);		// Software chip reset
	gfxSleepMilliseconds(10);

	write_reg(m, STMPE811_REG_SYS_CTRL2, 0x0C);		// Temperature sensor clock off, GPIO clock off, touch clock on, ADC clock on

	#if GMOUSE_STMPE811_GPIO_IRQPIN
		write_reg(m, STMPE811_REG_INT_EN, 0x01);	// Interrupt on INT pin when touch is detected
	#else
		write_reg(m, STMPE811_REG_INT_EN, 0x00);	// Don't Interrupt on INT pin when touch is detected
	#endif

	write_reg(m, STMPE811_REG_ADC_CTRL1, 0x48);		// ADC conversion time = 80 clock ticks, 12-bit ADC, internal voltage refernce
	gfxSleepMilliseconds(2);

	write_reg(m, STMPE811_REG_ADC_CTRL2, 0x01);		// ADC speed 3.25MHz
	write_reg(m, STMPE811_REG_GPIO_AF, 0x00);		// GPIO alternate function - OFF
	write_reg(m, STMPE811_REG_TSC_CFG, 0x9A);		// Averaging 4, touch detect delay 500 us, panel driver settling time 500 us
	write_reg(m, STMPE811_REG_FIFO_TH, 0x40);		// FIFO threshold = 64
	write_reg(m, STMPE811_REG_FIFO_STA, 0x01);		// FIFO reset enable
	write_reg(m, STMPE811_REG_FIFO_STA, 0x00);		// FIFO reset disable
	write_reg(m, STMPE811_REG_TSC_FRACT_XYZ, 0x07);	// Z axis data format
	write_reg(m, STMPE811_REG_TSC_I_DRIVE, 0x01);	// 50mA touchscreen line current
	write_reg(m, STMPE811_REG_TSC_CTRL, 0x00);		// X&Y&Z
	write_reg(m, STMPE811_REG_TSC_CTRL, 0x01);		// X&Y&Z, TSC enable
	write_reg(m, STMPE811_REG_INT_STA, 0xFF);		// Clear all interrupts
	#if GMOUSE_STMPE811_GPIO_IRQPIN
		if (read_byte(m, STMPE811_REG_TSC_CTRL) & 0x80)
			m->flags |= GMOUSE_STMPE811_FLG_TOUCHED;
	#endif
	write_reg(m, STMPE811_REG_INT_CTRL, 0x01);	// Level interrupt, enable interrupts

	release_bus(m);
	return TRUE;
}

static void read_xyz(GMouse* m, GMouseReading* pdr)
{
	bool_t	clearfifo;		// Do we need to clear the FIFO

	// Assume not touched.
	pdr->buttons = 0;
	pdr->z = 0;
	
	aquire_bus(m);

	#if GMOUSE_STMPE811_GPIO_IRQPIN
		// Check if the touch controller IRQ pin has gone off
		clearfifo = FALSE;
		if(getpin_irq(m)) {
			write_reg(m, STMPE811_REG_INT_STA, 0xFF);						// clear all interrupts
			if (read_byte(m, STMPE811_REG_TSC_CTRL) & 0x80)					// set the new touched status
				m->flags |= GMOUSE_STMPE811_FLG_TOUCHED;
			else
				m->flags &= ~GMOUSE_STMPE811_FLG_TOUCHED;
			clearfifo = TRUE;												// only take the last FIFO reading
		}

	#else
		// Poll to get the touched status
		uint16_t	last_touched;

		last_touched = m->flags;
		if (read_byte(m, STMPE811_REG_TSC_CTRL) & 0x80)						// set the new touched status
			m->flags |= GMOUSE_STMPE811_FLG_TOUCHED;
		else
			m->flags &= ~GMOUSE_STMPE811_FLG_TOUCHED;
		clearfifo = ((m->flags ^ last_touched) & GMOUSE_STMPE811_FLG_TOUCHED) ? TRUE : FALSE;
	#endif

	// If not touched don't do any more
	if ((m->flags & GMOUSE_STMPE811_FLG_TOUCHED)) {

		// Clear the fifo if it is too full
		#if !GMOUSE_STMPE811_SLOW_CPU
			if (!clearfifo && (read_byte(m, STMPE811_REG_FIFO_STA) & 0xD0))
		#endif
				clearfifo = TRUE;

		do {
			/* Get the X, Y, Z values */
			/* This could be done in a single 4 byte read to STMPE811_REG_TSC_DATA_XYZ (incr or non-incr) */
			pdr->x = (coord_t)read_word(m, STMPE811_REG_TSC_DATA_X);
			pdr->y = (coord_t)read_word(m, STMPE811_REG_TSC_DATA_Y);
			pdr->z = (coord_t)read_byte(m, STMPE811_REG_TSC_DATA_Z);
		} while(clearfifo && !(read_byte(m, STMPE811_REG_FIFO_STA) & 0x20));

		#if GMOUSE_STMPE811_SELF_CALIBRATE
			// Rescale X,Y,Z - If we are using self-calibration
			pdr->x = gdispGGetWidth(m->display) - pdr->x / (4096/gdispGGetWidth(m->display));
			pdr->y = pdr->y / (4096/gdispGGetHeight(m->display));
		#endif

		/* Force another read if we have more results */
		if (!clearfifo && !(read_byte(m, STMPE811_REG_FIFO_STA) & 0x20))
			_gmouseWakeup(m);
	}

	release_bus(m);
}

const GMouseVMT const GMOUSE_DRIVER_VMT[1] = {{
	{
		GDRIVER_TYPE_TOUCH,
		#if GMOUSE_STMPE811_SELF_CALIBRATE
			GMOUSE_VFLG_TOUCH | GMOUSE_VFLG_ONLY_DOWN | GMOUSE_VFLG_POORUPDOWN,
		#else
			GMOUSE_VFLG_TOUCH | GMOUSE_VFLG_ONLY_DOWN | GMOUSE_VFLG_POORUPDOWN | GMOUSE_VFLG_CALIBRATE | GMOUSE_VFLG_CAL_TEST,
		#endif
		sizeof(GMouse) + GMOUSE_STMPE811_BOARD_DATA_SIZE,
		_gmouseInitDriver,
		_gmousePostInitDriver,
		_gmouseDeInitDriver
	},
	255,			// z_max
	0,				// z_min
	200,			// z_touchon
	20,				// z_touchoff
	{				// pen_jitter
		GMOUSE_STMPE811_PEN_CALIBRATE_ERROR,			// calibrate
		GMOUSE_STMPE811_PEN_CLICK_ERROR,				// click
		GMOUSE_STMPE811_PEN_MOVE_ERROR				// move
	},
	{				// finger_jitter
		GMOUSE_STMPE811_FINGER_CALIBRATE_ERROR,		// calibrate
		GMOUSE_STMPE811_FINGER_CLICK_ERROR,			// click
		GMOUSE_STMPE811_FINGER_MOVE_ERROR			// move
	},
	MouseInit, 		// init
	0,				// deinit
	read_xyz,		// get
	0,				// calsave
	0				// calload
}};

#endif /* GFX_USE_GINPUT && GINPUT_NEED_MOUSE */

