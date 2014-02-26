/*
 * This file is subject to the terms of the GFX License. If a copy of
 * the license was not distributed with this file, you can obtain one at:
 *
 *              http://ugfx.org/license.html
 */

/**
 * @file    src/gaudout/gaudout.c
 * @brief   GAUDOUT sub-system code.
 *
 * @addtogroup GAUDOUT
 * @{
 */
#include "gfx.h"

#if GFX_USE_GAUDOUT || defined(__DOXYGEN__)

#include "src/gaudout/driver.h"

static gfxQueueASync	playlist;
static gfxQueueGSync	freelist;

static uint16_t			audFlags;
	#define AUDOUTFLG_RUNNING		0x0001
	#define AUDOUTFLG_USE_EVENTS	0x0002

#if GFX_USE_GEVENT
	static GTimer AudGTimer;

	static void AudGTimerCallback(void *param) {
		(void) param;
		GSourceListener	*psl;
		GEventADC		*pe;

		psl = 0;
		while ((psl = geventGetSourceListener((GSourceHandle)(&aud), psl))) {
			if (!(pe = (GEventAudioIn *)geventGetEventBuffer(psl))) {
				// This listener is missing - save this.
				psl->srcflags |= GAUDIN_LOSTEVENT;
				continue;
			}

			pe->type = GEVENT_AUDIO_IN;
			pe->channel = aud.channel;
			pe->count = lastcount;
			pe->buffer = lastbuffer;
			pe->flags = psl->srcflags;
			psl->srcflags = 0;
			geventSendEvent(psl);
		}
	}
#endif


void _gaudoutInit(void)
{
	gfxQueueASyncInit(&playlist);
	gfxQueueGSyncInit(&freelist);
}

void _gaudoutDeinit(void)
{
	/* ToDo */
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
		gfxQueueGSyncPut(&freelist, (gfxQueueGSyncItem *)paud);
	}

	return TRUE;
}

void gaudioReleaseBuffer(GAudioData *paud) {
	gfxQueueGSyncPut(&freelist, (gfxQueueGSyncItem *)paud);
}

GAudioData *gaudioGetBuffer(delaytime_t ms) {
	return (GAudioData *)gfxQueueGSyncGet(&freelist, ms);
}

bool_t gaudioPlayInit(uint16_t channel, uint32_t frequency) {
	gaudioPlayStop();
	gaudout_lld_deinit();
	return gaudout_lld_init(channel, frequency);
}

void gaudioPlay(GAudioData *paud) {
	if (paud)
		gfxQueueASyncPut(&playlist, (gfxQueueASyncItem *)paud);
	gaudout_lld_start();
}

void gaudioPlayPause(void) {
	gaudout_lld_stop();
}

void gaudioPlayStop(void) {
	GAudioData	*paud;

	gaudout_lld_stop();
	while((paud = (GAudioData *)gfxQueueASyncGet(&playlist)))
		gfxQueueGSyncPut(&freelist, (gfxQueueGSyncItem *)paud);
}

bool_t gaudioPlaySetVolume(uint8_t vol) {
	return gaudout_lld_set_volume(vol);
}

/**
 * Routines provided for use by drivers.
 */

GAudioData *gaudoutGetDataBlockI(void) {
	return (GAudioData *)gfxQueueASyncGet(&playlist);
}

void gaudoutReleaseDataBlockI(GAudioData *paud) {
	gfxQueueGSyncPut(&freelist, (gfxQueueGSyncItem *)paud);
}


#endif /* GFX_USE_GAUDOUT */
/** @} */

