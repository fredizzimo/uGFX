/*
 * This file is subject to the terms of the GFX License. If a copy of
 * the license was not distributed with this file, you can obtain one at:
 *
 *              http://ugfx.org/license.html
 */

#include "gfx.h"

#if GFX_USE_GINPUT && GINPUT_NEED_MOUSE

#define GMOUSE_DRIVER_VMT		GMOUSEVMT_MAX11802
#include "src/ginput/driver_mouse.h"

#define GMOUSE_MAX11802_FLG_TOUCHED		(GMOUSE_FLG_DRIVER_FIRST << 0)

// Hardware definitions
#include "drivers/ginput/touch/MAX11802/max11802.h"

// Get the hardware interface
#include "gmouse_lld_MAX11802_board.h"

// Last values read from A-D channels
static uint16_t	lastx, lasty;
static uint16_t	lastz = { 100 << 4 };			// This may not be used, so initialise it (value is bits 15..4)

static bool_t MouseInit(GMouse* m, unsigned driverinstance)
{
	uint8_t ret;

	const uint8_t commandList[][2] = {
		{ MAX11802_CMD_GEN_WR, 0xf0 },					// General config - leave TIRQ enabled, even though we ignore it ATM
		{ MAX11802_CMD_RES_WR, 0x00 },					// A-D resolution, hardware config - rewriting default; all 12-bit resolution
		{ MAX11802_CMD_AVG_WR, MAX11802_AVG },			// A-D averaging - 8 samples, average four median samples
		{ MAX11802_CMD_SAMPLE_WR, 0x00 },				// A-D sample time - use default
		{ MAX11802_CMD_TIMING_WR, MAX11802_TIMING },	// Setup timing
		{ MAX11802_CMD_DELAY_WR, MAX11802_DELAY },		// Conversion delays
		{ MAX11802_CMD_TDPULL_WR, 0x00 },				// A-D resolution, hardware config - rewrite default
		//{ MAX11802_CMD_MDTIM_WR, 0x00 },				// Autonomous mode timing - write default
		//{ MAX11802_CMD_APCONF_WR, 0x00 },				// Aperture config register - rewrite default
														// Ignore aux config register - not used
		{ MAX11802_CMD_MODE_WR, MAX11802_MODE },		// Set mode last
	};

	if (!init_board(m, driverinstance))
		return FALSE;

	aquire_bus(m);

	for (ret = 0; ret < (sizeof(commandList) / (2 * sizeof(uint8_t))); ret++) {
		write_command(m, commandList[ret][0], commandList[ret][1]);
	}
	
	ret = write_command(m, MAX11802_CMD_MODE_RD, 0);			// Read something as a test
	if (ret != MAX11802_MODE) {
		// Error here - @TODO: What can we do about it?
	}

	release_bus(m);
}

static void read_xyz(GMouse* m, GMouseReading* pdr)
{
	uint8_t readyCount;
	uint8_t notReadyCount;

	// Assume not touched.
	pdr->buttons = 0;
	pdr->z = 0;
	
	aquire_bus(m);

#if MAX11802_READ_Z_VALUE
	gfintWriteCommand(m, MAX11802_CMD_MEASUREXYZ);		// just write command
#else
	gfintWriteCommand(m, MAX11802_CMD_MEASUREXY);		// just write command
#endif

    /**
	 * put a delay in here, since conversion will take a finite time - longer if reading Z as well
	 * Potentially 1msec for 3 axes with 8us conversion time per sample, 8 samples per average
	 * Note Maxim AN5435-software to do calculation (www.maximintegrated.com/design/tools/appnotes/5435/AN5435-software.zip)
	 *
	 * We'll just use a fixed delay to avoid too much polling/bus activity
	 */
	gfxSleepMilliseconds(2);            // Was 1 - try 2

	/* Wait for data ready
	* Note: MAX11802 signals the readiness of the results using the lowest 4 bits of the result. However, these are the
	* last bits to be read out of the device. It is possible for the hardware value to change in the middle of the read,
	* causing the analog value to still be invalid even though the tags indicate a valid result.
	*
	* We work around this by reading the registers once more after the tags indicate they are ready.
	* There's also a separate counter to guard against never getting valid readings.
	*
	* Read the two or three readings required in a single burst, swapping x and y order if necessary
	*/

	readyCount = 0;
	notReadyCount = 0;
	do {
	    gfintWriteCommand(m, MAX11802_CMD_XPOSITION);     // This sets pointer to first byte of block

#if defined(GINPUT_MOUSE_YX_INVERTED) && GINPUT_MOUSE_YX_INVERTED
        lasty = read_value(m);
        lastx = read_value(m);
#else
        lastx = read_value(m);
        lasty = read_value(m));
#endif
#if MAX11802_READ_Z_VALUE
		lastz = read_value(m);
		if (((lastx & 0x0f) != 0xF) && ((lasty * 0xf0) != 0xF) && ((lastz * 0xf0) != 0xF))
#else
		if (((lastx & 0x0f) != 0xF) && ((lasty * 0xf0) != 0xF))
#endif
		{
			readyCount++;
		}
		else
		{
			notReadyCount++;          // Protect against infinite loops
		}
	} while ((readyCount < 2) && (notReadyCount < 20));
	
	
	/**
	 *	format of each value returned by MAX11802:
	 *		Bits 15..4 - analog value
	 *		Bits 3..2 - tag value - measurement type (X, Y, Z1, Z2)
	 *		Bits 1..0 - tag value - event type (00 = valid touch, 10 - no touch, 11 - measurement in progress)
	 */
	pt->x = lastx >> 4;		// Delete the tags
	pt->y = lasty >> 4;
    pt->z = lastz >> 4;		// If no Z-axis, lastz has been initialised to return 100

	pt->buttons = ((0 == (lastx & 3)) && (0 == (lasty & 3)) && (0 == (lastz & 3))) ? GINPUT_TOUCH_PRESSED : 0;

	release_bus(m);
}

const GMouseVMT const GMOUSE_DRIVER_VMT[1] = {{
	{
		GDRIVER_TYPE_TOUCH,
		GMOUSE_VFLG_TOUCH | GMOUSE_VFLG_ONLY_DOWN | GMOUSE_VFLG_POORUPDOWN | GMOUSE_VFLG_CALIBRATE | GMOUSE_VFLG_CAL_TEST,
		sizeof(GMouse) + GMOUSE_MAX11802_BOARD_DATA_SIZE,
		_gmouseInitDriver,
		_gmousePostInitDriver,
		_gmouseDeInitDriver
	},
	255,			// z_max
	0,				// z_min
	20,				// z_touchoff
	200,			// z_touchon
	{				// pen_jitter
		GMOUSE_MAX11802_PEN_CALIBRATE_ERROR,		// calibrate
		GMOUSE_MAX11802_PEN_CLICK_ERROR,			// click
		GMOUSE_MAX11802_PEN_MOVE_ERROR				// move
	},
	{				// finger_jitter
		GMOUSE_MAX11802_FINGER_CALIBRATE_ERROR,		// calibrate
		GMOUSE_MAX11802_FINGER_CLICK_ERROR,			// click
		GMOUSE_MAX11802_FINGER_MOVE_ERROR			// move
	},
	MouseInit, 		// init
	0,				// deinit
	read_xyz,		// get
	0,				// calsave
	0				// calload
}};

#endif /* GFX_USE_GINPUT && GINPUT_NEED_MOUSE */
