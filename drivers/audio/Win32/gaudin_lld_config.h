/*
 * This file is subject to the terms of the GFX License. If a copy of
 * the license was not distributed with this file, you can obtain one at:
 *
 *              http://ugfx.org/license.html
 */

/**
 * @file    drivers/audio/Win32/gaudin_lld_config.h
 * @brief   GAUDIN Driver config file.
 *
 * @addtogroup GAUDIN
 * @{
 */

#ifndef GAUDIN_LLD_CONFIG_H
#define GAUDIN_LLD_CONFIG_H

#if GFX_USE_GAUDIN

/*===========================================================================*/
/* Driver hardware support.                                                  */
/*===========================================================================*/

/**
 * @brief	The audio input sample type
 */
//typedef uint8_t		audin_sample_t;
typedef int16_t		audin_sample_t;

/**
 * @brief	The maximum sample frequency supported by this audio device
 */
#define GAUDIN_MAX_SAMPLE_FREQUENCY			44100

/**
 * @brief	The number of bits in a sample
 */
//#define GAUDIN_BITS_PER_SAMPLE				8
#define GAUDIN_BITS_PER_SAMPLE				16

/**
 * @brief	The format of an audio sample
 */
//#define GAUDIN_SAMPLE_FORMAT				ARRAY_DATA_8BITUNSIGNED
#define GAUDIN_SAMPLE_FORMAT				ARRAY_DATA_16BITSIGNED

/**
 * @brief	The number of audio channels supported by this driver
 */
#define GAUDIN_NUM_CHANNELS					2

/**
 * @brief	The list of audio channels and their uses
 * @{
 */
#define	GAUDIN_MONO							0
#define	GAUDIN_STEREO						1
/** @} */

#endif	/* GFX_USE_GAUDIN */

#endif	/* GAUDIN_LLD_CONFIG_H */
/** @} */
