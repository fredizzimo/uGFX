/*
 * This file is subject to the terms of the GFX License. If a copy of
 * the license was not distributed with this file, you can obtain one at:
 *
 *              http://ugfx.org/license.html
 */

/**
 * @file    boards/base/Olimex-SAM7EX256-GE8/gaudin_lld_board.h
 * @brief   GAUDIN Driver board config file for the Olimex SAM7EX256 board
 */

#ifndef _GAUDIN_LLD_BOARD_H
#define _GAUDIN_LLD_BOARD_H

/*===========================================================================*/
/* Audio inputs on this board                                                */
/*===========================================================================*/

#define GAUDIN_NUM_CHANNELS					1

/**
 * The list of audio channels and their uses
 */
#define	GAUDIN_MICROPHONE					0

#ifdef GAUDIN_LLD_IMPLEMENTATION
	static uint32_t gaudin_lld_physdevs[GAUDIN_NUM_CHANNELS] = {
			GADC_PHYSDEV_MICROPHONE,
			};
#endif

#endif	/* _GAUDIN_LLD_BOARD_H */
