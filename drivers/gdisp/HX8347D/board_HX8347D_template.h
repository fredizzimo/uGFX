/*
 * This file is subject to the terms of the GFX License. If a copy of
 * the license was not distributed with this file, you can obtain one at:
 *
 *              http://ugfx.org/license.html
 */

/**
 * @file    drivers/gdisp/HX8347D/board_HX8347D_template.h
 * @brief   GDISP Graphic Driver subsystem board SPI interface for the HX8347D display.
 *
 * @addtogroup GDISP
 * @{
 */

#ifndef _GDISP_LLD_BOARD_H
#define _GDISP_LLD_BOARD_H

/**
 * @brief   Initialise the board for the display.
 *
 * @param[in] g			The GDisplay structure
 *
 * @note	Set the g->board member to whatever is appropriate. For multiple
 * 			displays this might be a pointer to the appropriate register set.
 *
 * @notapi
 */
static inline void init_board(GDisplay *g) {
	(void) g;
}

/**
 * @brief   After the initialisation.
 *
 * @param[in] g			The GDisplay structure
 *
 * @notapi
 */
static inline void post_init_board(GDisplay *g) {
	(void) g;
}

/**
 * @brief   Set or clear the lcd reset pin.
 *
 * @param[in] g			The GDisplay structure
 * @param[in] state		TRUE = lcd in reset, FALSE = normal operation
 *
 * @notapi
 */
static inline void setpin_reset(GDisplay *g, bool_t state) {
	(void) g;
	(void) state;
}

/**
 * @brief   Set the lcd back-light level.
 *
 * @param[in] g				The GDisplay structure
 * @param[in] percent		0 to 100%
 *
 * @notapi
 */
static inline void set_backlight(GDisplay *g, uint8_t percent) {
	(void) g;
	(void) percent;
}

/**
 * @brief   Take exclusive control of the bus
 *
 * @param[in] g				The GDisplay structure
 *
 * @notapi
 */
static inline void acquire_bus(GDisplay *g) {
	(void) g;
}

/**
 * @brief   Release exclusive control of the bus
 *
 * @param[in] g				The GDisplay structure
 *
 * @notapi
 */
static inline void release_bus(GDisplay *g) {
	(void) g;
}

/**
 * @brief   Set the bus in 16 bit mode
 *
 * @param[in] g				The GDisplay structure
 *
 * @notapi
 */
static inline void busmode16(GDisplay *g) {
	(void) g;
}

/**
 * @brief   Set the bus in 8 bit mode (the default)
 *
 * @param[in] g				The GDisplay structure
 *
 * @notapi
 */
static inline void busmode8(GDisplay *g) {
	(void) g;
}

/**
 * @brief   Send data to the index register.
 *
 * @param[in] g				The GDisplay structure
 * @param[in] index			The index register to set
 *
 * @notapi
 */
static inline void write_index(GDisplay *g, uint8_t index) {
	(void) g;
	(void) index;
}

/**
 * @brief   Send 8 bits of data to the lcd.
 * @pre		The bus is in 8 bit mode
 *
 * @param[in] g				The GDisplay structure
 * @param[in] data			The data to send
 *
 * @notapi
 */
static inline void write_data(GDisplay *g, uint8_t data) {
	(void) g;
	(void) data;
}

/**
 * @brief   Send 16 bits of data to the lcd.
 * @pre		The bus is in 16 bit mode
 *
 * @param[in] g				The GDisplay structure
 * @param[in] data			The data to send
 *
 * @notapi
 */
static inline void write_ram16(GDisplay *g, uint16_t data) {
	(void) g;
	(void) data;
}

#endif /* _GDISP_LLD_BOARD_H */
/** @} */
