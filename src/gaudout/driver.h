/*
 * This file is subject to the terms of the GFX License. If a copy of
 * the license was not distributed with this file, you can obtain one at:
 *
 *              http://ugfx.org/license.html
 */

/**
 * @file    src/gaudout/driver.h
 * @brief   GAUDOUT - Audio Output driver header file.
 *
 * @defgroup Driver Driver
 * @ingroup GAUDOUT
 * @{
 */

#ifndef _GAUDOUT_LLD_H
#define _GAUDOUT_LLD_H

#include "gfx.h"

#if GFX_USE_GAUDOUT || defined(__DOXYGEN__)

/*===========================================================================*/
/* Type definitions                                                          */
/*===========================================================================*/

/*===========================================================================*/
/* External declarations.                                                    */
/*===========================================================================*/

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief				Get a block of audio data to play
 * @return				A pointer to the GAaudioData structure or NULL if none is currently available
 *
 * @note				Defined in the high level GAUDOUT code for use by the GAUDOUT drivers.
 *
 * @iclass
 * @notapi
 */
GAudioData *gaudoutGetDataBlockI(void);

/**
 * @brief				Release a block of audio data after playing
 *
 * @param[in] paud		The GAudioData block to be released.
 *
 * @note				Defined in the high level GAUDOUT code for use by the GAUDOUT drivers.
 *
 * @iclass
 * @notapi
 */
void gaudoutReleaseDataBlockI(GAudioData *paud);

/**
 * @brief				Initialise the driver
 * @return				TRUE if the channel and frequency are valid.
 *
 * @param[in] channel	The channel to use (see the driver for the available channels provided)
 * @param[in] frequency	The sample frequency to use
 *
 * @note				The driver will always have been stopped and de-init before this is called.
 *
 * @api
 */
bool_t gaudout_lld_init(uint16_t channel, uint32_t frequency);

/**
 * @brief				De-Initialise the driver
 *
 * @note				The audio output will always have been stopped first by the high level layer.
 * @note				This may be called before a @p gaudout_lld_init() has occurred.
 *
 * @api
 */
void gaudout_lld_deinit(void);

/**
 * @brief				Start the audio output playing
 *
 * @note				This may be called at any stage including while the driver
 * 						is already playing. The driver should check for data blocks
 * 						to play using @p gaudoutGetDataBlockI().
 *
 * @api
 */
void gaudout_lld_start(void);

/**
 * @brief				Stop the audio output playing.
 *
 * @note				Some drivers may only stop playing at a data block boundary.
 * @note				This may be called before a @p gaudout_lld_init() has occurred.
 *
 * @api
 */
void gaudout_lld_stop(void);

/**
 * @brief				Set the output volume.
 * @return				TRUE if successful.
 *
 * @param[in]			0->255 (0 = muted)
 *
 * @note				Some drivers may not support this. They will return FALSE.
 * @note				For stereo devices, both channels are set to the same volume.
 *
 * @api
 */
bool_t gaudout_lld_set_volume(uint8_t vol);

#ifdef __cplusplus
}
#endif

#endif /* GFX_USE_GAUDOUT */

#endif /* _GAUDOUT_LLD_H */
/** @} */
