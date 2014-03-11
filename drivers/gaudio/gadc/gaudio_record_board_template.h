/*
 * This file is subject to the terms of the GFX License. If a copy of
 * the license was not distributed with this file, you can obtain one at:
 *
 *              http://ugfx.org/license.html
 */

/**
 * @file    drivers/gaudio/gadc/gaudio_record_board_template.h
 * @brief   GAUDIO Record Driver board config board file
 *
 * @addtogroup GAUDIO
 * @{
 */

#ifndef _GAUDIO_RECORD_BOARD_H
#define _GAUDIO_RECORD_BOARD_H

/*===========================================================================*/
/* Audio inputs on this board                                                */
/*===========================================================================*/

/**
 * @brief	The number of audio channels supported by this driver
 * @note	This is an example
 */
#define GAUDIO_RECORD_NUM_CHANNELS					1

/**
 * @brief	The list of audio channels and their uses
 * @note	This is an example
 * @{
 */
#define	GAUDIO_RECORD_MICROPHONE					0
/** @} */

/**
 * @brief	The audio channel to GADC physical device assignment
 * @note	This is an example
 * @{
 */
#ifdef GAUDIO_RECORD_LLD_IMPLEMENTATION
	static uint32_t gaudin_lld_physdevs[GAUDIO_RECORD_NUM_CHANNELS] = {
			GADC_PHYSDEV_MICROPHONE,
			};
#endif
/** @} */

#endif	/* _GAUDIO_RECORD_BOARD_H */
/** @} */
