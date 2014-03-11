/*
 * This file is subject to the terms of the GFX License. If a copy of
 * the license was not distributed with this file, you can obtain one at:
 *
 *              http://ugfx.org/license.html
 */

/**
 * @file    drivers/gaudio/Win32/gaudio_play_config.h
 * @brief   GAUDIO Play Driver config file.
 *
 * @addtogroup GAUDIO
 * @{
 */

#ifndef GAUDIO_PLAY_CONFIG_H
#define GAUDIO_PLAY_CONFIG_H

#if GFX_USE_GAUDIO && GAUDIO_NEED_PLAY

/*===========================================================================*/
/* Driver hardware support.                                                  */
/*===========================================================================*/

/**
 * @brief	The maximum sample frequency supported by this audio device
 */
#define GAUDIO_PLAY_MAX_SAMPLE_FREQUENCY		44100

/**
 * @brief	The number of audio formats supported by this driver
 */
#define GAUDIO_PLAY_NUM_FORMATS					2

/**
 * @brief	The available audio sample formats in order of preference
 */
#define GAUDIO_PLAY_FORMAT1						ARRAY_DATA_16BITSIGNED
#define GAUDIO_PLAY_FORMAT2						ARRAY_DATA_8BITUNSIGNED

/**
 * @brief	The number of audio channels supported by this driver
 */
#define GAUDIO_PLAY_NUM_CHANNELS				2

/**
 * @brief	Whether each channel is mono or stereo
 */
#define GAUDIO_PLAY_CHANNEL0_IS_STEREO			FALSE
#define GAUDIO_PLAY_CHANNEL1_IS_STEREO			TRUE

/**
 * @brief	The list of audio channel names and their uses
 * @{
 */
#define	GAUDIO_PLAY_MONO						0
#define	GAUDIO_PLAY_STEREO						1
/** @} */

#endif	/* GFX_USE_GAUDIO && GAUDIO_NEED_PLAY */

#endif	/* GAUDIO_PLAY_CONFIG_H */
/** @} */
