/*
 * This file is subject to the terms of the GFX License. If a copy of
 * the license was not distributed with this file, you can obtain one at:
 *
 *              http://ugfx.org/license.html
 */

/**
 * @file    drivers/gdisp/ST7565/board_ST7565_template.h
 * @brief   GDISP Graphic Driver subsystem board interface for the ST7565 display.
 *
 * @addtogroup GDISP
 * @{
 */

#ifndef _GDISP_LLD_BOARD_H
#define _GDISP_LLD_BOARD_H

/**
 * @brief	Optional parameters that can be put in this file.
 * @note	The values listed below are the defaults.
 *
 * @note	#define GDISP_SCREEN_HEIGHT		600
 * @note	#define GDISP_SCREEN_WIDTH		800
 *
 * @note	Number of pixels per byte<br>
 * 			#define EINK_PPB				4
 *
 * @note	Delay for generating clock pulses.
 *			Unit is approximate clock cycles of the CPU (0 to 15).
 *			This should be atleast 50 ns.<br>
 *			#define EINK_CLOCKDELAY			0
 *
 * @note	Width of one framebuffer block.
 *			Must be divisible by EINK_PPB and evenly divide GDISP_SCREEN_WIDTH.<br>
 *			#define EINK_BLOCKWIDTH			20
 *
 * @note
 * @note	Height of one framebuffer block.
 *			Must evenly divide GDISP_SCREEN_WIDTH.<br>
 *			#define EINK_BLOCKHEIGHT		20
 *
 * @note	Number of block buffers to use for framebuffer emulation.<br>
 * 			#define EINK_NUMBUFFERS			40
 *
 * @note	Do a "blinking" clear, i.e. clear to opposite polarity first.
 *			This reduces the image persistence.<br>
 *			#define EINK_BLINKCLEAR			TRUE
 *
 * @note	Number of passes to use when clearing the display<br>
 *			#define EINK_CLEARCOUNT			10
 *
 * @note	Number of passes to use when writing to the display<br>
 *			#define EINK_WRITECOUNT			4
 */

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
 * @brief	Delay for display waveforms. Should be an accurate microsecond delay.
 *
 * @param[in] us		The number of microseconds
 */
static void eink_delay(int us) {
	(void) us;
}

/**
 * @brief	Turn the E-ink panel Vdd supply (+3.3V) on or off.
 *
 * @param[in] g			The GDisplay structure
 * @param[in] on		On or off
 */
static inline void setpower_vdd(GDisplay *g, bool_t on) {
	(void) g;
	(void) on;
}

/**
 * @brief	Turn the E-ink panel negative supplies (-15V, -20V) on or off.
 *
 * @param[in] g			The GDisplay structure
 * @param[in] on		On or off
 */
static inline void setpower_vneg(GDisplay *g, bool_t on) {
	(void) g;
	(void) on;
}

/**
 * @brief	Turn the E-ink panel positive supplies (-15V, -20V) on or off.
 *
 * @param[in] g			The GDisplay structure
 * @param[in] on		On or off
 */
static inline void setpower_vpos(GDisplay *g, bool_t on) {
	(void) g;
	(void) on;
}

/**
 * @brief	Set the state of the LE (source driver Latch Enable) pin.
 *
 * @param[in] g			The GDisplay structure
 * @param[in] on		On or off
 */
static inline void setpin_le(GDisplay *g, bool_t on) {
	(void) g;
	(void) on;
}

/**
 * @brief	Set the state of the OE (source driver Output Enable) pin.
 *
 * @param[in] g			The GDisplay structure
 * @param[in] on		On or off
 */
static inline void setpin_oe(GDisplay *g, bool_t on) {
	(void) g;
	(void) on;
}

/**
 * @brief	Set the state of the CL (source driver Clock) pin.
 *
 * @param[in] g			The GDisplay structure
 * @param[in] on		On or off
 */
static inline void setpin_cl(GDisplay *g, bool_t on) {
	(void) g;
	(void) on;
}

/**
 * @brief	Set the state of the SPH (source driver Start Pulse Horizontal) pin.
 *
 * @param[in] g			The GDisplay structure
 * @param[in] on		On or off
 */
static inline void setpin_sph(GDisplay *g, bool_t on) {
	(void) g;
	(void) on;
}

/**
 * @brief	Set the state of the D0-D7 (source driver Data) pins.
 *
 * @param[in] g			The GDisplay structure
 * @param[in] value		The byte to write
 */
static inline void setpins_data(GDisplay *g, uint8_t value) {
	(void) g;
	(void) value;
}

/**
 * @brief	Set the state of the CKV (gate driver Clock Vertical) pin.
 *
 * @param[in] g			The GDisplay structure
 * @param[in] on		On or off
 */
static inline void setpin_ckv(GDisplay *g, bool_t on) {
	(void) g;
	(void) on;
}

/**
 * @brief	Set the state of the GMODE (gate driver Gate Mode) pin.
 *
 * @param[in] g			The GDisplay structure
 * @param[in] on		On or off
 */
static inline void setpin_gmode(GDisplay *g, bool_t on) {
	(void) g;
	(void) on;
}

/**
 * @brief	Set the state of the SPV (gate driver Start Pulse Vertical) pin.
 *
 * @param[in] g			The GDisplay structure
 * @param[in] on		On or off
 */
static inline void setpin_spv(GDisplay *g, bool_t on) {
	(void) g;
	(void) on;
}

#endif /* _GDISP_LLD_BOARD_H */
/** @} */
