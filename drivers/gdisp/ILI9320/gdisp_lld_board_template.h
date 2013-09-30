/*
 * This file is subject to the terms of the GFX License. If a copy of
 * the license was not distributed with this file, you can obtain one at:
 *
 *              http://ugfx.org/license.html
 */

/**
 * @file    drivers/gdisp/ILI9320/gdisp_lld_board_template.h
 * @brief   GDISP Graphic Driver subsystem board interface for the ILI9320 display.
 *
 * @addtogroup GDISP
 * @{
 */

#ifndef GDISP_LLD_BOARD_H
#define GDISP_LLD_BOARD_H

static inline void init_board(void) {

}

static inline void setpin_reset(bool_t state) {

}

static inline void acquire_bus(void) {

}

static inline void release_bus(void) {

}

static inline void write_index(uint16_t data) {

}

static inline void write_data(uint16_t data) {

}

static inline void setreadmode(void) {

}

static inline void setwritemode(void) {

}

static inline uint16_t read_data(void) {

}

static inline uint16_t set_backlight(uint8_t percentage) {

}

#endif /* GDISP_LLD_BOARD_H */
/** @} */

