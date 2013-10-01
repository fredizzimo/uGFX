/*
 * This file is subject to the terms of the GFX License. If a copy of
 * the license was not distributed with this file, you can obtain one at:
 *
 *              http://ugfx.org/license.html
 */

/**
 * @file    drivers/gdisp/Nokia6610GE12/gdisp_lld_board_template.h
 * @brief   GDISP Graphic Driver subsystem board interface for the Nokia6610 GE12 display.
 *
 * @addtogroup GDISP
 * @{
 */

#ifndef _GDISP_LLD_BOARD_H
#define _GDISP_LLD_BOARD_H

/*
 * Set various display properties. These properties mostly depend on the exact controller chip you get.
 * The defaults should work for most controllers.
 */
//#define GDISP_SCREEN_HEIGHT				130		// The visible display height
//#define GDISP_SCREEN_WIDTH				130		// The visible display width
//#define GDISP_RAM_X_OFFSET				0		// The x offset of the visible area
//#define GDISP_RAM_Y_OFFSET				2		// The y offset of the visible area
//#define GDISP_SLEEP_POS					50		// The position of the sleep mode partial display
//#define GDISP_INITIAL_CONTRAST			50		// The initial contrast percentage
//#define GDISP_INITIAL_BACKLIGHT			100		// The initial backlight percentage

/**
 * @brief   Initialise the board for the display.
 * @notes	Performs the following functions:
 *			1. initialise the spi port used by your display
 *			2. initialise the reset pin (initial state not-in-reset)
 *			3. initialise the chip select pin (initial state not-active)
 *			4. initialise the backlight pin (initial state back-light off)
 *
 * @notapi
 */
static inline void init_board(void) {

}

/**
 * @brief   Set or clear the lcd reset pin.
 *
 * @param[in] state		TRUE = lcd in reset, FALSE = normal operation
 * 
 * @notapi
 */
static inline void setpin_reset(bool_t state) {

}

/**
 * @brief   Set the lcd back-light level.
 * @note	For now 0% turns the backlight off, anything else the backlight is on.
 *			While the hardware supports PWM backlight control, we are not using it
 *			yet.
 *
 * @param[in] percent		0 to 100%
 * 
 * @notapi
 */
static inline void set_backlight(uint8_t percent) {

}

/**
 * @brief   Take exclusive control of the bus
 *
 * @notapi
 */
static inline void acquire_bus(void) {

}

/**
 * @brief   Release exclusive control of the bus
 *
 * @notapi
 */
static inline void release_bus(void) {

}

/**
 * @brief   Send an 8 bit command to the lcd.
 *
 * @param[in] cmd		The command to send
 *
 * @notapi
 */
static inline void write_cmd(uint16_t cmd) {

}

/**
 * @brief   Send an 8 bit data to the lcd.
 *
 * @param[in] data		The data to send
 * 
 * @notapi
 */
static inline void write_data(uint16_t data) {

}

#if GDISP_HARDWARE_READPIXEL || GDISP_HARDWARE_SCROLL
/**
 * @brief   Read data from the lcd.
 *
 * @return	The data from the lcd
 * @note	The chip select may need to be asserted/de-asserted
 * 			around the actual spi read
 * 
 * @notapi
 */
static inline uint16_t read_data(void) {

}
#endif

#endif /* _GDISP_LLD_BOARD_H */
/** @} */
