/*
 * This file is subject to the terms of the GFX License. If a copy of
 * the license was not distributed with this file, you can obtain one at:
 *
 *              http://ugfx.org/license.html
 */

#ifndef _GINPUT_LLD_MOUSE_BOARD_H
#define _GINPUT_LLD_MOUSE_BOARD_H

#define GMOUSE_ADS7843_PEN_CALIBRATE_ERROR		2
#define GMOUSE_ADS7843_PEN_CLICK_ERROR			2
#define GMOUSE_ADS7843_PEN_MOVE_ERROR			2
#define GMOUSE_ADS7843_FINGER_CALIBRATE_ERROR	4
#define GMOUSE_ADS7843_FINGER_CLICK_ERROR		4
#define GMOUSE_ADS7843_FINGER_MOVE_ERROR		4

static bool_t init_board(GMouse* m, unsigned driverinstance) {

}

static inline bool_t getpin_pressed(void) {

}

static inline void aquire_bus(void) {

}

static inline void release_bus(void) {

}

static inline uint16_t read_value(uint16_t port) {

}

#endif /* _GINPUT_LLD_MOUSE_BOARD_H */
