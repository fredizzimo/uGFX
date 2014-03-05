/*
 * This file is subject to the terms of the GFX License. If a copy of
 * the license was not distributed with this file, you can obtain one at:
 *
 *              http://ugfx.org/license.html
 */

/**
 * @file    src/gaudout/sys_defs.h
 *
 * @addtogroup GAUDOUT
 *
 * @brief	Module to output audio data (under development)
 *
 * @{
 */

#ifndef _GAUDOUT_H
#define _GAUDOUT_H

#include "gfx.h"

#if GFX_USE_GAUDOUT || defined(__DOXYGEN__)

/* Include the driver defines */
#include "gaudout_lld_config.h"


/*===========================================================================*/
/* Type definitions                                                          */
/*===========================================================================*/

/**
 * @brief	Contains Audio Data Samples
 * @note	This structure is followed immediately by the sample data itself.
 * 			When allocating the buffers for the sample data put this structure
 * 			at the beginning of the buffer.
 */
typedef struct GAudioData {
	gfxQueueASyncItem	next;		// @< Used for queuing the buffers
	size_t				size;		// @< The size of the buffer area following this structure (in bytes)
	size_t				len;		// @< The length of the data in the buffer area (in bytes)
} GAudioData;


// Event types for GAUDOUT
#define GEVENT_AUDIO_OUT			(GEVENT_GAUDOUT_FIRST+0)

/**
 * @brief   The Audio output event structure.
 * @{
 */
typedef struct GEventAudioOut_t {
	#if GFX_USE_GEVENT || defined(__DOXYGEN__)
	/**
	 * @brief The type of this event (GEVENT_AUDIO_OUT)
	 */
		GEventType				type;
	#endif
	/**
	 * @brief The event flags
	 */
	uint16_t				flags;
		/**
		 * @brief   The event flag values.
		 * @{
		 */
		#define	GAUDOUT_LOSTEVENT		0x0001		/**< @brief The last GEVENT_AUDIO_OUT event was lost */
		#define	GAUDOUT_PLAYING			0x0002		/**< @brief The audio out system is currently playing */
		#define	GAUDOUT_FREEBLOCK		0x0004		/**< @brief An audio buffer has been freed */
		/** @} */
} GEventAudioOut;
/** @} */

/*===========================================================================*/
/* External declarations.                                                    */
/*===========================================================================*/

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief		Allocate some audio buffers and put them on the free list
 * @return		TRUE is it succeeded. FALSE on allocation failure.
 *
 * @param[in] num	The number of buffers to allocate
 * @param[in] size	The size (in bytes) of each buffer
 *
 * @api
 */
bool_t gaudioAllocBuffers(unsigned num, size_t size);

/**
 * @brief		Get an audio buffer from the free list
 * @return		A GAudioData pointer or NULL if the timeout is exceeded
 *
 * @params[in] ms	The maximum amount of time in milliseconds to wait for a buffer if one is not available.
 *
 * @api
 */
GAudioData *gaudioGetBuffer(delaytime_t ms);

/**
 * @brief		Release a buffer back to the free list
 *
 * @param[in] paud		The buffer to put (back) on the free-list.
 *
 * @note		This call should be used to return any buffers that were taken from
 * 				the free-list once they have been finished with. It can also be used
 * 				to put new buffers onto the free-list. Just make sure the "size" field
 * 				of the GAudioData structure has been filled in first.
 *
 * @api
 */
void gaudioReleaseBuffer(GAudioData *paud);

/**
 * @brief		Set the audio device to play on the specified channel and with the specified
 * 				sample frequency.
 * @return		TRUE is successful, FALSE if the driver doesn't accept those parameters.
 *
 * @param[in] channel	The audio output channel to use.
 * @param[in] frequency	The audio sample rate in samples per second
 * @param[in] format	The audio sample format
 *
 * @note		Some channels are mono, and some are stereo. See your driver config file
 * 				to determine which channels to use and whether they are stereo or not.
 * @note		Only one channel can be playing at a time. Calling this will stop any
 * 				currently playing channel.
 *
 * @api
 */
bool_t gaudioPlayInit(uint16_t channel, uint32_t frequency, ArrayDataFormat format);

/**
 * @brief		Play the specified sample data.
 * @details		The sample data is output to the audio channel. On completion the buffer is returned to the free-list.
 * @pre			@p gaudioPlayInit must have been called first to set the channel and sample frequency.
 *
 * @param[in] paud	The audio sample buffer to play. It can be NULL (used to restart paused audio)
 *
 * @note		Calling this will cancel any pause.
 * @note		Before calling this function the len field of the GAudioData structure must be
 * 				specified (in bytes).
 * @note		For stereo channels the sample data is interleaved in the buffer.
 * @note		This call returns before the data has completed playing. Subject to available buffers (which
 * 				can be obtained from the free-list), any number of buffers may be played. They will be queued
 * 				for playing in the order they are supplied to this routine and played when previous buffers are
 * 				complete. In this way continuous playing can be obtained without audio gaps.
 *
 * @api
 */
void gaudioPlay(GAudioData *paud);

/**
 * @brief		Pause any currently playing sounds.
 *
 * @note		If nothing is currently playing this routine does nothing. To restart playing call @p gaudioPlay()
 * 				with or without a new sample buffer.
 * @note		Some drivers will not respond until a buffer boundary.
 *
 * @api
 */
void gaudioPlayPause(void);

/**
 * @brief		Stop any currently playing sounds.
 *
 * @note		This stops any playing sounds and returns any currently queued buffers back to the free-list.
 * @note		Some drivers will not respond until a buffer boundary.
 *
 * @api
 */
void gaudioPlayStop(void);

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
bool_t gaudioPlaySetVolume(uint8_t vol);

#if GFX_USE_GEVENT || defined(__DOXYGEN__)
	/**
	 * @brief   			Turn on sending results to the GEVENT sub-system.
	 * @details				Returns a GSourceHandle to listen for GEVENT_AUDIO_OUT events.
	 *
	 * @note				The audio output will not use the GEVENT system unless this is
	 * 						called first. This saves processing time if the application does
	 * 						not want to use the GEVENT sub-system for audio output.
	 * 						Once turned on it can only be turned off by calling @p gaudioPlayInit() again.
	 * @note				The audio output is capable of signalling via this method and other methods
	 * 						at the same time.
	 *
	 * @return				The GSourceHandle
	 *
	 * @api
	 */
	GSourceHandle gaudioPlayGetSource(void);
#endif

#ifdef __cplusplus
}
#endif

#endif /* GFX_USE_GAUDOUT */

#endif /* _GAUDOUT_H */
/** @} */

