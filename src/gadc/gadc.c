/*
 * This file is subject to the terms of the GFX License. If a copy of
 * the license was not distributed with this file, you can obtain one at:
 *
 *              http://ugfx.org/license.html
 */

/**
 * @file    src/gadc/gadc.c
 * @brief   GADC sub-system code.
 *
 * @addtogroup GADC
 * @{
 */
#include "gfx.h"

#if GFX_USE_GADC

/* Include the driver defines */
#include "src/gadc/driver.h"

#if GADC_MAX_HIGH_SPEED_SAMPLERATE > GADC_MAX_SAMPLE_FREQUENCY/2
	#error "GADC: GADC_MAX_HIGH_SPEED_SAMPLERATE has been set too high. It must be less than half the maximum CPU rate"
#endif

#define GADC_MAX_LOWSPEED_DEVICES	((GADC_MAX_SAMPLE_FREQUENCY/GADC_MAX_HIGH_SPEED_SAMPLERATE)-1)
#if GADC_MAX_LOWSPEED_DEVICES > 4
	#undef GADC_MAX_LOWSPEED_DEVICES
	#define GADC_MAX_LOWSPEED_DEVICES	4
#endif

volatile bool_t GADC_Timer_Missed;

static bool_t			gadcRunning;
static gfxSem			LowSpeedSlotSem;
static gfxMutex			LowSpeedMutex;
static GTimer			LowSpeedGTimer;
static gfxQueueGSync	HighSpeedBuffers;

#if GFX_USE_GEVENT
	static GTimer		HighSpeedGTimer;
#endif


#define GADC_FLG_ISACTIVE	0x0001
#define GADC_FLG_ISDONE		0x0002
#define GADC_FLG_ERROR		0x0004
#define GADC_FLG_GTIMER		0x0008
#define GADC_FLG_STALLED	0x0010

static struct hsdev {
	// Our status flags
	uint16_t				flags;

	// Other stuff we need to track progress and for signaling
	GadcLldTimerData		lld;
	uint16_t				eventflags;
	GADCISRCallbackFunction	isrfn;
	} hs;

static struct lsdev {
	// Our status flags
	uint16_t				flags;

	// What we started with
	GadcLldNonTimerData		lld;
	GADCCallbackFunction	fn;
	void					*param;
	} ls[GADC_MAX_LOWSPEED_DEVICES];

static struct lsdev *curlsdev;

/* Find the next conversion to activate */
static inline void FindNextConversionI(void) {
	if (curlsdev) {
		/**
		 * Now we have done a low speed conversion - start looking for the next conversion
		 * We only look forward to ensure we get a high speed conversion at least once
		 * every GADC_MAX_LOWSPEED_DEVICES conversions.
		 */
		curlsdev++;

	} else {

		/* Now we have done a high speed conversion - start looking for low speed conversions */
		curlsdev = ls;
	}

	/**
	 * Look for the next thing to do.
	 */
	gadcRunning = TRUE;
	for(; curlsdev < &ls[GADC_MAX_LOWSPEED_DEVICES]; curlsdev++) {
		if ((curlsdev->flags & (GADC_FLG_ISACTIVE|GADC_FLG_ISDONE)) == GADC_FLG_ISACTIVE) {
			gadc_lld_adc_nontimerI(&curlsdev->lld);
			return;
		}
	}
	curlsdev = 0;

	/* No more low speed devices - do a high speed conversion */
	if (hs.flags & GADC_FLG_ISACTIVE) {
		hs.lld.pdata = gfxBufferGetI();
		if (hs.lld.pdata) {
			hs.lld.now = GADC_Timer_Missed || (hs.flags & GADC_FLG_STALLED);
			hs.flags &= ~GADC_FLG_STALLED;
			GADC_Timer_Missed = 0;
			gadc_lld_adc_timerI(&hs.lld);
			return;
		}

		// Oops - no free buffers - mark stalled and go back to low speed devices
		hs.flags |= GADC_FLG_STALLED;
		hs.eventflags &= ~GADC_HSADC_RUNNING;
		for(curlsdev = ls; curlsdev < &ls[GADC_MAX_LOWSPEED_DEVICES]; curlsdev++) {
			if ((curlsdev->flags & (GADC_FLG_ISACTIVE|GADC_FLG_ISDONE)) == GADC_FLG_ISACTIVE) {
				gadc_lld_adc_nontimerI(&curlsdev->lld);
				return;
			}
		}
		curlsdev = 0;
	}

	/* Nothing more to do */
	gadcRunning = FALSE;
}

void gadcDataReadyI(void) {

	if (curlsdev) {
		/* This interrupt must be in relation to the low speed device */

		if (curlsdev->flags & GADC_FLG_ISACTIVE) {
			curlsdev->flags |= GADC_FLG_ISDONE;
			gtimerJabI(&LowSpeedGTimer);
		}

		#if GFX_USE_OS_CHIBIOS && CHIBIOS_ADC_ISR_FULL_CODE_BUG
			/**
			 * Oops - We have just finished a low speed conversion but a bug prevents us
			 * restarting the ADC here. Other code will restart it in the thread based
			 * ADC handler.
			 */
			gadcRunning = FALSE;
			return;

		#endif

	} else {
		/* This interrupt must be in relation to the high speed device */

		if (hs.flags & GADC_FLG_ISACTIVE) {
			if (hs.lld.pdata->len) {
				/* Save the current buffer on the HighSpeedBuffers */
				gfxQueueGSyncPutI(&HighSpeedBuffers, (gfxQueueGSyncItem *)hs.lld.pdata);
				hs.lld.pdata = 0;

				/* Save the details */
				hs.eventflags = GADC_HSADC_RUNNING|GADC_HSADC_GOTBUFFER;
				if (GADC_Timer_Missed)
					hs.eventflags |= GADC_HSADC_LOSTEVENT;
				if (hs.flags & GADC_FLG_STALLED)
					hs.eventflags |= GADC_HSADC_STALL;

				/* Our signalling mechanisms */
				if (hs.isrfn)
					hs.isrfn();

				#if GFX_USE_GEVENT
					if (hs.flags & GADC_FLG_GTIMER)
						gtimerJabI(&HighSpeedGTimer);
				#endif
			} else {
				// Oops - no data in this buffer. Just return it to the free-list
				gfxBufferRelease(hs.lld.pdata);
				hs.lld.pdata = 0;
			}
		}
	}

	/**
	 * Look for the next thing to do.
	 */
	FindNextConversionI();
}

void gadcDataFailI(void) {
	if (curlsdev) {
		if ((curlsdev->flags & (GADC_FLG_ISACTIVE|GADC_FLG_ISDONE)) == GADC_FLG_ISACTIVE)
			/* Mark the error then try to repeat it */
			curlsdev->flags |= GADC_FLG_ERROR;

		#if GFX_USE_OS_CHIBIOS && CHIBIOS_ADC_ISR_FULL_CODE_BUG
			/**
			 * Oops - We have just finished a low speed conversion but a bug prevents us
			 * restarting the ADC here. Other code will restart it in the thread based
			 * ADC handler.
			 */
			gadcRunning = FALSE;
			gtimerJabI(&LowSpeedGTimer);
			return;

		#endif

	} else {
		if (hs.flags & GADC_FLG_ISACTIVE)
			/* Mark the error and then try to repeat it */
			hs.flags |= GADC_FLG_ERROR;
	}

	/* Start the next conversion */
	FindNextConversionI();
}

/* Our module initialiser */
void _gadcInit(void)
{
	gadc_lld_init();
	gfxQueueGSyncInit(&HighSpeedBuffers);
	gfxSemInit(&LowSpeedSlotSem, GADC_MAX_LOWSPEED_DEVICES, GADC_MAX_LOWSPEED_DEVICES);
	gfxMutexInit(&LowSpeedMutex);
	gtimerInit(&LowSpeedGTimer);
	#if GFX_USE_GEVENT
		gtimerInit(&HighSpeedGTimer);
	#endif
}

void _gadcDeinit(void)
{
	/* commented stuff is ToDo */

	// gadc_lld_deinit();
	gfxQueueGSyncDeinit(&HighSpeedBuffers);
	gfxSemDestroy(&LowSpeedSlotSem);
	gfxMutexDestroy(&LowSpeedMutex);
	gtimerDeinit(&LowSpeedGTimer);
	#if GFX_USE_GEVENT
		gtimerDeinit(&HighSpeedGTimer);
	#endif	
}

static inline void StartADC(bool_t onNoHS) {
	gfxSystemLock();
	if (!gadcRunning || (onNoHS && !curlsdev))
		FindNextConversionI();
	gfxSystemUnlock();
}

static void BSemSignalCallback(adcsample_t *buffer, void *param) {
	(void) buffer;

	/* Signal the BinarySemaphore parameter */
	gfxSemSignal((gfxSem *)param);
}

#if GFX_USE_GEVENT
	static void HighSpeedGTimerCallback(void *param) {
		(void) param;
		GSourceListener	*psl;
		GEventADC		*pe;

		psl = 0;
		while ((psl = geventGetSourceListener((GSourceHandle)(&HighSpeedGTimer), psl))) {
			if (!(pe = (GEventADC *)geventGetEventBuffer(psl))) {
				// This listener is missing - save this.
				psl->srcflags |= GADC_HSADC_LOSTEVENT;
				continue;
			}

			pe->type = GEVENT_ADC;
			pe->flags = hs.eventflags | psl->srcflags;
			psl->srcflags = 0;
			geventSendEvent(psl);
		}
	}
#endif

static void LowSpeedGTimerCallback(void *param) {
	(void) param;
	GADCCallbackFunction	fn;
	void					*prm;
	adcsample_t				*buffer;
	struct lsdev			*p;

	#if GFX_USE_OS_CHIBIOS && CHIBIOS_ADC_ISR_FULL_CODE_BUG
		/* Ensure the ADC is running if it needs to be - Bugfix HACK */
		StartADC(FALSE);
	#endif

	/**
	 * Look for completed low speed timers.
	 * We don't need to take the mutex as we are the only place that things are freed and we
	 * do that atomically.
	 */
	for(p=ls; p < &ls[GADC_MAX_LOWSPEED_DEVICES]; p++) {
		if ((p->flags & (GADC_FLG_ISACTIVE|GADC_FLG_ISDONE)) == (GADC_FLG_ISACTIVE|GADC_FLG_ISDONE)) {
			/* This item is done - perform its callback */
			fn = p->fn;				// Save the callback details
			prm = p->param;
			buffer = p->lld.buffer;
			p->fn = 0;				// Needed to prevent the compiler removing the local variables
			p->param = 0;			// Needed to prevent the compiler removing the local variables
			p->lld.buffer = 0;		// Needed to prevent the compiler removing the local variables
			p->flags = 0;			// The slot is available (indivisible operation)
			gfxSemSignal(&LowSpeedSlotSem);	// Tell everyone
			fn(buffer, prm);		// Perform the callback
		}
	}

}

void gadcHighSpeedInit(uint32_t physdev, uint32_t frequency)
{
	gadcHighSpeedStop();		/* This does the init for us */

	/* Just save the details and reset everything for now */
	hs.lld.physdev = physdev;
	hs.lld.frequency = frequency;
	hs.lld.pdata = 0;
	hs.lld.now = FALSE;
	hs.isrfn = 0;
}

#if GFX_USE_GEVENT
	GSourceHandle gadcHighSpeedGetSource(void) {
		if (!gtimerIsActive(&HighSpeedGTimer))
			gtimerStart(&HighSpeedGTimer, HighSpeedGTimerCallback, 0, TRUE, TIME_INFINITE);
		hs.flags |= GADC_FLG_GTIMER;
		return (GSourceHandle)&HighSpeedGTimer;
	}
#endif

void gadcHighSpeedSetISRCallback(GADCISRCallbackFunction isrfn) {
	hs.isrfn = isrfn;
}

GDataBuffer *gadcHighSpeedGetData(delaytime_t ms) {
	return (GDataBuffer *)gfxQueueGSyncGet(&HighSpeedBuffers, ms);
}

GDataBuffer *gadcHighSpeedGetDataI(void) {
	return (GDataBuffer *)gfxQueueGSyncGetI(&HighSpeedBuffers);
}

void gadcHighSpeedStart(void) {
	/* If its already going we don't need to do anything */
	if (hs.flags & GADC_FLG_ISACTIVE)
		return;

	hs.flags = GADC_FLG_ISACTIVE;
	gadc_lld_start_timer(&hs.lld);
	StartADC(FALSE);
}

void gadcHighSpeedStop(void) {
	if (hs.flags & GADC_FLG_ISACTIVE) {
		/* No more from us */
		hs.flags = 0;
		gadc_lld_stop_timer(&hs.lld);
		/*
		 * There might be a buffer still locked up by the driver - if so release it.
		 */
		if (hs.lld.pdata) {
			gfxBufferRelease(hs.lld.pdata);
			hs.lld.pdata = 0;
		}

		/*
		 * We have to pass TRUE to StartADC() as we might have the ADC marked as active when it isn't
		 * due to stopping the timer while it was converting.
		 */
		StartADC(TRUE);
	}
}

void gadcLowSpeedGet(uint32_t physdev, adcsample_t *buffer) {
	struct lsdev	*p;
	gfxSem			mysem;

	/* Start the Low Speed Timer */
	gfxSemInit(&mysem, 1, 1);
	gfxMutexEnter(&LowSpeedMutex);
	if (!gtimerIsActive(&LowSpeedGTimer))
		gtimerStart(&LowSpeedGTimer, LowSpeedGTimerCallback, 0, TRUE, TIME_INFINITE);
	gfxMutexExit(&LowSpeedMutex);

	while(1) {
		/* Wait for an available slot */
		gfxSemWait(&LowSpeedSlotSem, TIME_INFINITE);

		/* Find a slot */
		gfxMutexEnter(&LowSpeedMutex);
		for(p = ls; p < &ls[GADC_MAX_LOWSPEED_DEVICES]; p++) {
			if (!(p->flags & GADC_FLG_ISACTIVE)) {
				p->lld.physdev = physdev;
				p->lld.buffer = buffer;
				p->fn = BSemSignalCallback;
				p->param = &mysem;
				p->flags = GADC_FLG_ISACTIVE;
				gfxMutexExit(&LowSpeedMutex);
				StartADC(FALSE);
				gfxSemWait(&mysem, TIME_INFINITE);
				return;
			}
		}
		gfxMutexExit(&LowSpeedMutex);

		/**
		 *  We should never get here - the count semaphore must be wrong.
		 *  Decrement it and try again.
		 */
	}
}

bool_t gadcLowSpeedStart(uint32_t physdev, adcsample_t *buffer, GADCCallbackFunction fn, void *param) {
	struct lsdev *p;

	/* Start the Low Speed Timer */
	gfxMutexEnter(&LowSpeedMutex);
	if (!gtimerIsActive(&LowSpeedGTimer))
		gtimerStart(&LowSpeedGTimer, LowSpeedGTimerCallback, 0, TRUE, TIME_INFINITE);

	/* Find a slot */
	for(p = ls; p < &ls[GADC_MAX_LOWSPEED_DEVICES]; p++) {
		if (!(p->flags & GADC_FLG_ISACTIVE)) {
			/* We know we have a slot - this should never wait anyway */
			gfxSemWait(&LowSpeedSlotSem, TIME_IMMEDIATE);
			p->lld.physdev = physdev;
			p->lld.buffer = buffer;
			p->fn = fn;
			p->param = param;
			p->flags = GADC_FLG_ISACTIVE;
			gfxMutexExit(&LowSpeedMutex);
			StartADC(FALSE);
			return TRUE;
		}
	}
	gfxMutexExit(&LowSpeedMutex);
	return FALSE;
}

#endif /* GFX_USE_GADC */
/** @} */

