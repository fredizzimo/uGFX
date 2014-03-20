/*
 * This file is subject to the terms of the GFX License. If a copy of
 * the license was not distributed with this file, you can obtain one at:
 *
 *              http://ugfx.org/license.html
 */

/**
 * @file    drivers/gadc/AT91SAM7/gadc_lld.c
 * @brief   GADC - Periodic ADC driver source file for the AT91SAM7 cpu.
 */

#include "gfx.h"

#if GFX_USE_GADC

#include "src/gadc/driver.h"

static GDataBuffer	*pData;
static size_t		bytesperconversion;

static void ISR_CompleteI(ADCDriver *adcp, adcsample_t *buffer, size_t n);
static void ISR_ErrorI(ADCDriver *adcp, adcerror_t err);


static ADCConversionGroup acg = {
		FALSE,					// circular
		1,						// num_channels
		ISR_CompleteI,			// end_cb
		ISR_ErrorI,				// error_cb
		0,						// channelselects
		0,						// trigger
		0,						// frequency
		};

static void ISR_CompleteI(ADCDriver *adcp, adcsample_t *buffer, size_t n) {
	(void)	adcp;
	(void)	buffer;

	if (pData) {
		// A set of timer base conversions is complete
		pData->len += n * bytesperconversion;

		// Are we finished yet?
		// In ChibiOS we (may) get a half-buffer complete. In this situation the conversions
		//	are really not complete and so we just wait for the next lot of data.
		if (pData->len + bytesperconversion > pData->size)
			gadcDataReadyI();

	} else {
		// A single non-timer conversion is complete
		gadcDataReadyI();
	}
}

static void ISR_ErrorI(ADCDriver *adcp, adcerror_t err) {
	(void)	adcp;
	(void)	err;

	gadcDataFailI();
}

void gadc_lld_init(void) {
	adcStart(&ADCD1, 0);
}

void gadc_lld_start_timer(GadcLldTimerData *pgtd) {
	int			phys;

	/* Calculate the bytes per conversion from physdev */
	/* The AT91SAM7 has AD0..7 - physdev is a bitmap of those channels */
	phys = pgtd->physdev;
	for(bytesperconversion = 0; phys; phys >>= 1)
		if (phys & 0x01)
			bytesperconversion++;
	bytesperconversion *= (gfxSampleFormatBits(GADC_SAMPLE_FORMAT)+7)/8;

	/**
	 * The AT91SAM7 ADC driver supports triggering the ADC using a timer without having to implement
	 * an interrupt handler for the timer. The driver also initialises the timer correctly for us.
	 * Because we aren't trapping the interrupt ourselves we can't increment GADC_Timer_Missed if an
	 * interrupt is missed.
	 */
	acg.frequency = pgtd->frequency;
}

void gadc_lld_stop_timer(GadcLldTimerData *pgtd) {
	(void) pgtd;
	if ((acg.trigger & ~ADC_TRIGGER_SOFTWARE) == ADC_TRIGGER_TIMER)
		adcStop(&ADCD1);
}

void gadc_lld_adc_timerI(GadcLldTimerData *pgtd) {
	/**
	 *  We don't need to calculate num_channels because the AT91SAM7 ADC does this for us.
	 */
	acg.channelselects = pgtd->physdev;
	acg.trigger = pgtd->now ? (ADC_TRIGGER_TIMER|ADC_TRIGGER_SOFTWARE) : ADC_TRIGGER_TIMER;

	pData = pgtd->pdata;
	adcStartConversionI(&ADCD1, &acg, (adcsample_t *)(pgtd->pdata+1), pData->size/bytesperconversion);

	/* Next time assume the same (still running) timer */
	acg.frequency = 0;
}

void gadc_lld_adc_nontimerI(GadcLldNonTimerData *pgntd) {
	/**
	 *  We don't need to calculate num_channels because the AT91SAM7 ADC does this for us.
	 */
	acg.channelselects = pgntd->physdev;
	acg.trigger = ADC_TRIGGER_SOFTWARE;

	pData = 0;
	adcStartConversionI(&ADCD1, &acg, pgntd->buffer, 1);
}

#endif /* GFX_USE_GADC */
