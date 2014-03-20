/*
 * This file is subject to the terms of the GFX License. If a copy of
 * the license was not distributed with this file, you can obtain one at:
 *
 *              http://ugfx.org/license.html
 */

/**
 * @file    drivers/gaudio/gadc/gaudio_record_config.h
 * @brief   GAUDIN Record Driver config file.
 *
 * @addtogroup GAUDIO
 * @{
 */

#ifndef GAUDIO_RECORD_CONFIG_H
#define GAUDIO_RECORD_CONFIG_H

#if GFX_USE_GAUDIO && GAUDIO_NEED_RECORD

/*===========================================================================*/
/* Driver hardware support.                                                  */
/*===========================================================================*/

/**
 * @brief	The maximum sample frequency supported by this audio device
 * @details	For this driver it matches the GADC maximum high speed sample rate
 */
#define GAUDIO_RECORD_MAX_SAMPLE_FREQUENCY			GADC_MAX_HIGH_SPEED_SAMPLERATE

/**
 * @brief	The number of audio formats supported by this driver
 */
#define GAUDIO_RECORD_NUM_FORMATS					1

/**
 * @brief	The available audio sample formats in order of preference
 */
#define GAUDIO_RECORD_FORMAT1						GADC_SAMPLE_FORMAT

/**
 * For the GAUDIO driver that uses GADC - all the remaining config definitions are specific
 * to the board.
 */
/* Include the user supplied board definitions */
#include "gaudio_record_board.h"

#endif	/* GFX_USE_GAUDIO && GAUDIO_NEED_RECORD */

#endif	/* GAUDIO_RECORD_CONFIG_H */
/** @} */
