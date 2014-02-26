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
 * @brief	The audio input sample type
 */
//typedef uint8_t		audout_sample_t;
typedef int16_t		audout_sample_t;

/**
 * @brief	The maximum sample frequency supported by this audio device
 */
#define GAUDOUT_MAX_SAMPLE_FREQUENCY			44100

/**
 * @brief	The number of bits in a sample
 */
//#define GAUDOUT_BITS_PER_SAMPLE				8
#define GAUDOUT_BITS_PER_SAMPLE				16

/**
 * @brief	The format of an audio sample
 */
//#define GAUDOUT_SAMPLE_FORMAT				ARRAY_DATA_8BITUNSIGNED
#define GAUDOUT_SAMPLE_FORMAT				ARRAY_DATA_16BITSIGNED

/**
 * @brief	The number of audio channels supported by this driver
 */
#define GAUDOUT_NUM_CHANNELS				2

/**
 * @brief	The list of audio channels and their uses
 * @{
 */
#define	GAUDOUT_MONO						0
#define	GAUDOUT_STEREO						1
/** @} */

#endif	/* GFX_USE_GAUDOUT */

#endif	/* GAUDOUT_LLD_CONFIG_H */
/** @} */
