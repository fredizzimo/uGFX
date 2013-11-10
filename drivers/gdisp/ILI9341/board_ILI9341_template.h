/*
 * This file is subject to the terms of the GFX License. If a copy of
 * the license was not distributed with this file, you can obtain one at:
 *
 *              http://ugfx.org/license.html
 */

/**
 * @file    drivers/gdisp/ILI9341/board_ILI9341_template.h
 * @brief   GDISP Graphic Driver subsystem board interface for the ILI9341 display.
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
 * @brief   Send data to the index register.
 *
 * @param[in] g				The GDisplay structure
 * @param[in] index			The index register to set
 *
 * @notapi
 */
static inline void write_index(GDisplay *g, uint16_t index) {
	(void) g;
	(void) index;
}

/**
 * @brief   Send data to the lcd.
 *
 * @param[in] g				The GDisplay structure
 * @param[in] data			The data to send
 * 
 * @notapi
 */
static inline void write_data(GDisplay *g, uint16_t data) {
	(void) g;
	(void) data;
}

/**
 * @brief   Set the bus in read mode
 *
 * @param[in] g				The GDisplay structure
 *
 * @notapi
 */
static inline void setreadmode(GDisplay *g) {
	(void) g;
}

/**
 * @brief   Set the bus back into write mode
 *
 * @param[in] g				The GDisplay structure
 *
 * @notapi
 */
static inline void setwritemode(GDisplay *g) {
	(void) g;
}

/**
 * @brief   Read data from the lcd.
 * @return	The data from the lcd
 *
 * @param[in] g				The GDisplay structure
 *
 * @notapi
 */
static inline uint16_t read_data(GDisplay *g) {
	(void) g;
	return 0;
}

#endif /* _GDISP_LLD_BOARD_H */
/** @} */
