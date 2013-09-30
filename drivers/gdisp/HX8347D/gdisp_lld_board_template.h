/*
 * This file is subject to the terms of the GFX License. If a copy of
 * the license was not distributed with this file, you can obtain one at:
 *
 *              http://ugfx.org/license.html
 */

/**
 * @file    drivers/gdisp/HX8347D/gdisp_lld_board_template.h
 * @brief   GDISP Graphic Driver subsystem board SPI interface for the HX8347D display.
 *
 * @addtogroup GDISP
 * @{
 */

#ifndef _GDISP_LLD_BOARD_H
#define _GDISP_LLD_BOARD_H

/**
 * @brief   Initialise the board for the display.
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
 *
 * @param[in] percent		0 to 100%
 *
 * @notapi
 */
static inline void set_backlight(uint8_t percent) {

}

/**
 * @brief   Take exclusive control of the bus
 * @notapi
 */
static inline void acquire_bus(void) {

}

/**
 * @brief   Release exclusive control of the bus
 * @notapi
 */
static inline void release_bus(void) {

}

/**
 * @brief   Set the bus in 16 bit mode
 * @notapi
 */
static inline void busmode16(void) {

}

/**
 * @brief   Set the bus in 8 bit mode (the default)
 * @notapi
 */
static inline void busmode8(void) {

}

/**
 * @brief   Set which index register to use.
 *
 * @param[in] index		The index register to set
 *
 * @notapi
 */
static inline void write_index(uint8_t cmd) {

}

/**
 * @brief   Send a command to the lcd.
 *
 * @param[in] data		The data to send
 *
 * @notapi
 */
static inline void write_reg(uint8_t cmd, uint8_t data) {

}

static inline void write_ram16(uint16_t data) {

}

#endif /* _GDISP_LLD_BOARD_H */
/** @} */

