/*
 * This file is subject to the terms of the GFX License. If a copy of
 * the license was not distributed with this file, you can obtain one at:
 *
 *              http://ugfx.org/license.html
 */

/**
 * @file    drivers/ginput/touch/MCU/ginput_lld_mouse_board_template.h
 * @brief   GINPUT Touch low level driver source for the MCU on the example board.
 *
 * @defgroup Mouse Mouse
 * @ingroup GINPUT
 *
 * @{
 */

#ifndef _GINPUT_LLD_MOUSE_BOARD_H
#define _GINPUT_LLD_MOUSE_BOARD_H

/**
 * @brief   Initialise the board for the touch.
 *
 * @notapi
 */
static inline void init_board(void) {
}

/**
 * @brief   Acquire the bus ready for readings
 *
 * @notapi
 */
static inline void aquire_bus(void) {
}

/**
 * @brief   Release the bus after readings
 *
 * @notapi
 */
static inline void release_bus(void) {
}

/**
 * @brief   Set up the device for a x coordinate read
 * @note	This is performed once followed by multiple
 * 			x coordinate read's (which are then median filtered)
 *
 * @notapi
 */
static inline void setup_x(void) {
}

/**
 * @brief   Set up the device for a y coordinate read
 * @note	This is performed once followed by multiple
 * 			y coordinate read's (which are then median filtered)
 *
 * @notapi
 */
static inline void setup_y(void) {
}

/**
 * @brief   Set up the device for a z coordinate (pressure) read
 * @note	This is performed once followed by multiple
 * 			z coordinate read's (which are then median filtered)
 *
 * @notapi
 */
static inline void setup_z(void) {
	palClearPad(GPIOB, GPIOB_DRIVEA);
	palClearPad(GPIOB, GPIOB_DRIVEB);
    chThdSleepMilliseconds(2);
}

/**
 * @brief   Read an x value from touch controller
 * @return	The value read from the controller
 *
 * @notapi
 */
static inline uint16_t read_x(void) {
}

/**
 * @brief   Read a y value from touch controller
 * @return	The value read from the controller
 *
 * @notapi
 */
static inline uint16_t read_y(void) {
}

/**
 * @brief   Read a z value from touch controller
 * @return	The value read from the controller.
 * @note	The return value must be scaled between 0 and 100.
 * 			Values over 80 are considered as "touch" down.
 *
 * @notapi
 */
static inline uint16_t read_z(void) {
}

#endif /* _GINPUT_LLD_MOUSE_BOARD_H */
/** @} */

