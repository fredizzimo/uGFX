/*
 * This file is subject to the terms of the GFX License. If a copy of
 * the license was not distributed with this file, you can obtain one at:
 *
 *              http://ugfx.org/license.html
 */

/**
 * @file    drivers/gdisp/SSD1963/board_SSD1963_template.h
 * @brief   GDISP Graphic Driver subsystem board interface for the SSD1963 display.
 *
 * @addtogroup GDISP
 * @{
 */

#ifndef _GDISP_LLD_BOARD_H
#define _GDISP_LLD_BOARD_H

/**
 * @brief	LCD panel specs
 *
 * @note	The timings need to follow the datasheet for your particular TFT/LCD screen
 * 			(the actual screen, not the controller).
 * @note	Datasheets normally use a specific set of timings and acronyms, their value refers
 * 			to the number of pixel clocks. Non-display periods refer to pulses/timings that occur
 * 			before or after the timings that actually put pixels on the screen. Display periods
 * 			refer to pulses/timings that directly put pixels on the screen.
 * @note	HDP: Horizontal Display Period, normally the width - 1<br>
 * 			HT: Horizontal Total period (display + non-display)<br>
 * 			HPS: non-display period between the start of the horizontal sync (LLINE) signal
 * 				and the first display data<br>
 * 			LPS: horizontal sync pulse (LLINE) start location in pixel clocks<br>
 * 			HPW: Horizontal sync Pulse Width<br>
 * 			VDP: Vertical Display period, normally height - 1<br>
 * 			VT: Vertical Total period (display + non-display)<br>
 * 			VPS: non-display period in lines between the start of the frame and the first display
 * 				data in number of lines<br>
 * 			FPS: vertical sync pulse (LFRAME) start location in lines.<br>
 * 			VPW: Vertical sync Pulse Width
 * @note	Here's how to convert them:<br>
 * 				SCREEN_HSYNC_FRONT_PORCH = ( HT - HPS ) - GDISP_SCREEN_WIDTH<br>
 * 				SCREEN_HSYNC_PULSE = HPW<br>
 * 				SCREEN_HSYNC_BACK_PORCH = HPS - HPW<br>
 * 				SCREEN_VSYNC_FRONT_PORCH = ( VT - VPS ) - GDISP_SCREEN_HEIGHT<br>
 * 				SCREEN_VSYNC_PULSE = VPW<br>
 * 				SCREEN_VSYNC_BACK_PORCH = VPS - LPS<br>
 */

static const LCD_Parameters	DisplayTimings[] = {
	// You need one of these array elements per display
	{
		480, 272,								// Panel width and height
		2, 2, 41,								// Horizontal Timings (back porch, front porch, pulse)
		CALC_PERIOD(480,2,2,41),				// Total Horizontal Period (calculated from above line)
		2, 2, 10,								// Vertical Timings (back porch, front porch, pulse)
		CALC_PERIOD(272,2,2,10),				// Total Vertical Period (calculated from above line)
		CALC_FPR(480,272,2,2,41,2,2,10,60ULL)	// FPR - the 60ULL is the frames per second. Note the ULL!
	},
};

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

#endif /* _GDISP_LLD_BOARD_H */
/** @} */
