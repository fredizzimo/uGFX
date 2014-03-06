/*
 * This file is subject to the terms of the GFX License. If a copy of
 * the license was not distributed with this file, you can obtain one at:
 *
 *              http://ugfx.org/license.html
 */

/**
 * @file    drivers/audio/Win32/gaudout_lld_config.h
 * @brief   GAUDOUT Driver config file.
 *
 * @addtogroup GAUDOUT
 * @{
 */

#ifndef GAUDOUT_LLD_CONFIG_H
#define GAUDIN_LLD_CONFIG_H

#if GFX_USE_GAUDOUT

/*===========================================================================*/
/* Driver hardware support.                                                  */
/*===========================================================================*/

/**
 * @brief	The maximum sample frequency supported by this audio device
 */
#define GAUDOUT_MAX_SAMPLE_FREQUENCY		44100

/**
 * @brief	The number of audio formats supported by this driver
 */
#define GAUDOUT_NUM_FORMATS					2

/**
 * @brief	The available audio sample formats in order of preference
 */
#define GAUDOUT_FORMAT1						ARRAY_DATA_16BITSIGNED
#define GAUDOUT_FORMAT2						ARRAY_DATA_8BITUNSIGNED

/**
 * @brief	The number of audio channels supported by this driver
 */
#define GAUDOUT_NUM_CHANNELS				2

/**
 * @brief	Whether each channel is mono or stereo
 */
#define GAUDOUT_CHANNEL0_STEREO				FALSE
#define GAUDOUT_CHANNEL1_STEREO				TRUE

/**
 * @brief	The list of audio channel names and their uses
 * @{
 */
#define	GAUDOUT_MONO						0
#define	GAUDOUT_STEREO						1
/** @} */

#endif	/* GFX_USE_GAUDOUT */

#endif	/* GAUDOUT_LLD_CONFIG_H */
/** @} */
