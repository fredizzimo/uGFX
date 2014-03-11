/*
 * This file is subject to the terms of the GFX License. If a copy of
 * the license was not distributed with this file, you can obtain one at:
 *
 *              http://ugfx.org/license.html
 */

/**
 * @file    src/gaudio/gaudio.c
 * @brief   GAUDIO sub-system code.
 *
 * @addtogroup GAUDIO
 * @{
 */
#include "gfx.h"

#if GFX_USE_GAUDIO

static gfxQueueGSync	freeList;

#if GAUDIO_NEED_PLAY
	#include "src/gaudio/driver_play.h"

	static gfxQueueASync	playList;
	static gfxSem			playComplete;
	static uint16_t			playFlags;
		#define PLAYFLG_USEEVENTS	0x0001
		#define PLAYFLG_PLAYING		0x0002
		#define PLAYFLG_ISINIT		0x0004
	#if GFX_USE_GEVENT
		static GTimer playTimer;
		static void PlayTimerCallback(void *param);
	#endif
#endif

#if GAUDIO_NEED_RECORD
	#include "src/gaudio/driver_record.h"

	static gfxQueueGSync	recordList;
	static uint16_t			recordFlags;
		#define RECORDFLG_USEEVENTS		0x0001
		#define RECORDFLG_RECORDING		0x0002
		#define RECORDFLG_STALLED		0x0004
		#define RECORDFLG_ISINIT		0x0008
	#if GFX_USE_GEVENT
		static GTimer recordTimer;
		static void RecordTimerCallback(void *param);
	#endif
#endif


void _gaudioInit(void)
{
	gfxQueueGSyncInit(&freeList);
	#if GAUDIO_NEED_PLAY
		gfxQueueASyncInit(&playList);
		#if GFX_USE_GEVENT
			gtimerInit(&playTimer);
		#endif
		gfxSemInit(&playComplete, 0, 0);
	#endif
	#if GAUDIO_NEED_RECORD
		gfxQueueGSyncInit(&recordList);
		#if GFX_USE_GEVENT
			gtimerInit(&recordTimer);
		#endif
	#endif
}

void _gaudioDeinit(void)
{
	#if GAUDIO_NEED_PLAY
		#if GFX_USE_GEVENT
			gtimerDeinit(&playTimer);
		#endif
		gfxSemDestroy(&playComplete);
	#endif
	#if GAUDIO_NEED_RECORD
		#if GFX_USE_GEVENT
			gtimerDeinit(&recordTimer);
		#endif
	#endif
}

bool_t gaudioAllocBuffers(unsigned num, size_t size) {
	GAudioData *paud;

	if (num < 1)
		return FALSE;

	// Round up to a multiple of 4 to prevent problems with structure alignment
	size = (size + 3) & ~0x03;

	// Allocate the memory
	if (!(paud = gfxAlloc((size+sizeof(GAudioData)) * num)))
		return FALSE;

	// Add each of them to our free list
	for(;num--; paud = (GAudioData *)((char *)(paud+1)+size)) {
		paud->size = size;
		gfxQueueGSyncPut(&freeList, (gfxQueueGSyncItem *)paud);
	}

	return TRUE;
}

void gaudioReleaseBuffer(GAudioData *paud) {
	gfxQueueGSyncPut(&freeList, (gfxQueueGSyncItem *)paud);
}

GAudioData *gaudioGetBuffer(delaytime_t ms) {
	return (GAudioData *)gfxQueueGSyncGet(&freeList, ms);
}

#if GAUDIO_NEED_PLAY

	bool_t gaudioPlayInit(uint16_t channel, uint32_t frequency, ArrayDataFormat format) {
		gaudioPlayStop();
		playFlags &= ~PLAYFLG_ISINIT;
		if (!gaudio_play_lld_init(channel, frequency, format))
			return FALSE;
		playFlags |= PLAYFLG_ISINIT;
		return TRUE;
	}

	void gaudioPlay(GAudioData *paud) {
		if (!(playFlags & PLAYFLG_ISINIT)) {
			// Oops - init failed - return it directly to the free-list
			if (paud) {
				gfxQueueGSyncPut(&freeList, (gfxQueueGSyncItem *)paud);
				gfxYield();				// Make sure we get no endless cpu hogging loops
			}
			return;
		}

		if (paud)
			gfxQueueASyncPut(&playList, (gfxQueueASyncItem *)paud);
		playFlags |= PLAYFLG_PLAYING;
		gaudio_play_lld_start();
	}

	void gaudioPlayPause(void) {
		if ((playFlags & (PLAYFLG_ISINIT|PLAYFLG_PLAYING)) == (PLAYFLG_ISINIT|PLAYFLG_PLAYING))
			gaudio_play_lld_stop();
	}

	void gaudioPlayStop(void) {
		GAudioData	*paud;

		if (playFlags & PLAYFLG_PLAYING)
			gaudio_play_lld_stop();
		while((paud = (GAudioData *)gfxQueueASyncGet(&playList)))
			gfxQueueGSyncPut(&freeList, (gfxQueueGSyncItem *)paud);
	}

	bool_t gaudioPlaySetVolume(uint8_t vol) {
		return gaudio_play_lld_set_volume(vol);
	}

	bool_t gaudioPlayWait(delaytime_t ms) {
		if (!(playFlags & PLAYFLG_PLAYING))
			return TRUE;
		return gfxSemWait(&playComplete, ms);
	}

	#if GFX_USE_GEVENT
		static void PlayTimerCallback(void *param) {
			(void) param;
			GSourceListener	*psl;
			GEventAudioPlay	*pe;

			psl = 0;
			while ((psl = geventGetSourceListener((GSourceHandle)&playTimer, psl))) {
				if (!(pe = (GEventAudioPlay *)geventGetEventBuffer(psl))) {
					// This listener is missing - save this.
					psl->srcflags |= GAUDIO_PLAY_LOSTEVENT;
					continue;
				}

				pe->type = GEVENT_AUDIO_PLAY;
				pe->flags = psl->srcflags;
				psl->srcflags = 0;
				if ((playFlags & PLAYFLG_PLAYING))
					pe->flags |= GAUDIO_PLAY_PLAYING;
				if (!gfxQueueGSyncIsEmpty(&freeList))
					pe->flags |= GAUDIO_PLAY_FREEBLOCK;
				geventSendEvent(psl);
			}
		}

		GSourceHandle gaudioPlayGetSource(void) {
			if (!gtimerIsActive(&playTimer))
				gtimerStart(&playTimer, PlayTimerCallback, 0, TRUE, TIME_INFINITE);
			playFlags |= PLAYFLG_USEEVENTS;
			return (GSourceHandle)&playTimer;
		}
	#endif

	/**
	 * Routines provided for use by drivers.
	 */

	GAudioData *gaudioPlayGetDataBlockI(void) {
		return (GAudioData *)gfxQueueASyncGetI(&playList);
	}

	void gaudioPlayReleaseDataBlockI(GAudioData *paud) {
		gfxQueueGSyncPutI(&freeList, (gfxQueueGSyncItem *)paud);
		#if GFX_USE_GEVENT
			if (playFlags & PLAYFLG_USEEVENTS)
				gtimerJabI(&playTimer);
		#endif
	}

	void gaudioPlayDoneI(void) {
		playFlags &= ~PLAYFLG_PLAYING;
		#if GFX_USE_GEVENT
			if (playFlags & PLAYFLG_USEEVENTS)
				gtimerJabI(&playTimer);
		#endif
		gfxSemSignalI(&playComplete);			// This should really be gfxSemSignalAllI(&playComplete);
	}
#endif

#if GAUDIO_NEED_RECORD
	bool_t gaudioRecordInit(uint16_t channel, uint32_t frequency, ArrayDataFormat format) {
		gaudioRecordStop();
		recordFlags &= ~RECORDFLG_ISINIT;
		if (!gaudio_record_lld_init(channel, frequency, format))
			return FALSE;
		recordFlags |= RECORDFLG_ISINIT;
		return TRUE;
	}

	void gaudioRecordStart(void) {
		if (!(recordFlags & RECORDFLG_ISINIT))
			return;							// Oops - init failed

		recordFlags |= RECORDFLG_RECORDING;
		recordFlags &= ~RECORDFLG_STALLED;
		gaudio_record_lld_start();
	}

	void gaudioRecordStop(void) {
		GAudioData	*paud;

		if ((recordFlags & (RECORDFLG_RECORDING|RECORDFLG_STALLED)) == RECORDFLG_RECORDING)
			gaudio_record_lld_stop();
		recordFlags &= ~(RECORDFLG_RECORDING|RECORDFLG_STALLED);
		while((paud = (GAudioData *)gfxQueueGSyncGet(&recordList, TIME_IMMEDIATE)))
			gfxQueueGSyncPut(&freeList, (gfxQueueGSyncItem *)paud);
	}

	GAudioData *gaudioRecordGetData(delaytime_t ms) {
		return (GAudioData *)gfxQueueGSyncGet(&recordList, ms);
	}

	#if GFX_USE_GEVENT
		static void RecordTimerCallback(void *param) {
			(void) param;
			GSourceListener		*psl;
			GEventAudioRecord	*pe;

			psl = 0;
			while ((psl = geventGetSourceListener((GSourceHandle)&recordTimer, psl))) {
				if (!(pe = (GEventAudioRecord *)geventGetEventBuffer(psl))) {
					// This listener is missing - save this.
					psl->srcflags |= GAUDIO_RECORD_LOSTEVENT;
					continue;
				}
				pe->type = GEVENT_AUDIO_RECORD;
				pe->flags = psl->srcflags;
				psl->srcflags = 0;
				if ((recordFlags & RECORDFLG_RECORDING))
					pe->flags |= GAUDIO_RECORD_RECORDING;
				if ((recordFlags & RECORDFLG_STALLED))
					pe->flags |= GAUDIO_RECORD_STALL;
				if (!gfxQueueGSyncIsEmpty(&recordList))
					pe->flags |= GAUDIO_RECORD_GOTBLOCK;
				geventSendEvent(psl);
			}
		}

		GSourceHandle gaudioRecordGetSource(void) {
			if (!gtimerIsActive(&recordTimer))
				gtimerStart(&recordTimer, RecordTimerCallback, 0, TRUE, TIME_INFINITE);
			recordFlags |= RECORDFLG_USEEVENTS;
			return (GSourceHandle)&recordTimer;
		}
	#endif

	/**
	 * Routines provided for use by drivers.
	 */

	GAudioData *gaudioRecordGetFreeBlockI(void) {
		return (GAudioData *)gfxQueueGSyncGetI(&freeList);
	}

	void gaudioRecordSaveDataBlockI(GAudioData *paud) {
		gfxQueueGSyncPutI(&recordList, (gfxQueueGSyncItem *)paud);
		#if GFX_USE_GEVENT
			if (recordFlags & RECORDFLG_USEEVENTS)
				gtimerJabI(&recordTimer);
		#endif
	}

	void gaudioRecordDoneI(void) {
		recordFlags |= RECORDFLG_STALLED;
		#if GFX_USE_GEVENT
			if (recordFlags & RECORDFLG_USEEVENTS)
				gtimerJabI(&recordTimer);
		#endif
	}
#endif

#endif /* GFX_USE_GAUDIO */
/** @} */
