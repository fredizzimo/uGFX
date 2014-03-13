/*
 * This file is subject to the terms of the GFX License. If a copy of
 * the license was not distributed with this file, you can obtain one at:
 *
 *              http://ugfx.org/license.html
 */

/**
 * @file    drivers/gaudio/Win32/gaudio_record_config.h
 * @brief   GAUDIO Record Driver config file.
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
 */
#define GAUDIO_RECORD_MAX_SAMPLE_FREQUENCY			44100

/**
 * @brief	The number of audio formats supported by this driver
 */
#define GAUDIO_RECORD_NUM_FORMATS					2

/**
 * @brief	The available audio sample formats in order of preference
 */
#define GAUDIO_RECORD_FORMAT1						ARRAY_DATA_16BITSIGNED
#define GAUDIO_RECORD_FORMAT2						ARRAY_DATA_8BITUNSIGNED

/**
 * @brief	The number of audio channels supported by this driver
 */
#define GAUDIO_RECORD_NUM_CHANNELS					2

/**
 * @brief	Whether each channel is mono or stereo
 */
#define GAUDIO_RECORD_CHANNEL0_IS_STEREO			FALSE
#define GAUDIO_RECORD_CHANNEL1_IS_STEREO			TRUE

/**
 * @brief	The list of audio channels and their uses
 * @{
 */
#define	GAUDIO_RECORD_MONO							0
#define	GAUDIO_RECORD_STEREO						1
/** @} */

#endif	/* GFX_USE_GAUDIO && GAUDIO_NEED_RECORD */

#endif	/* GAUDIO_RECORD_CONFIG_H */
/** @} */
