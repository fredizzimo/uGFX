/*
 * This file is subject to the terms of the GFX License. If a copy of
 * the license was not distributed with this file, you can obtain one at:
 *
 *              http://ugfx.org/license.html
 */

/**
 * @file    boards/base/Mikromedia-STM32-M4-ILI9341/ginput_lld_mouse_board.h
 * @brief   GINPUT Touch low level driver source for the MCU.
 */

#ifndef _GINPUT_LLD_MOUSE_BOARD_H
#define _GINPUT_LLD_MOUSE_BOARD_H

/* read ADC if more than this many ticks since last read */
#define ADC_UPDATE_INTERVAL 3

#define ADC_NUM_CHANNELS   2
#define ADC_BUF_DEPTH      1

static const ADCConversionGroup adcgrpcfg = {
  FALSE,
  ADC_NUM_CHANNELS,
  NULL,
  NULL,
  /* HW dependent part.*/
  0,
  ADC_CR2_SWSTART,
  0,
  0,
  ADC_SQR1_NUM_CH(ADC_NUM_CHANNELS),
  0,
  ADC_SQR3_SQ2_N(ADC_CHANNEL_IN8) | ADC_SQR3_SQ1_N(ADC_CHANNEL_IN9)
};

static systime_t last_update;
static volatile uint16_t tpx, tpy, detect;

static inline void delay(uint16_t dly) {
  static uint16_t i;
  for(i = 0; i < dly; i++)
    asm("nop");
}


void read_mikro_tp(void) {
	systime_t now = chTimeNow();

	adcsample_t samples[ADC_NUM_CHANNELS * ADC_BUF_DEPTH];
	uint16_t _detect, _tpx, _tpy;

	if(now < last_update || ((now - last_update) > ADC_UPDATE_INTERVAL)) {
		// detect button press
		// sample[0] will go from ~200 to ~4000 when pressed
	    adcConvert(&ADCD1, &adcgrpcfg, samples, ADC_BUF_DEPTH);
	    _detect = samples[0];

	    // read x channel
	   	palSetPad(GPIOB, GPIOB_DRIVEA);
	    palClearPad(GPIOB, GPIOB_DRIVEB);
	    chThdSleepMilliseconds(1);
	    adcConvert(&ADCD1, &adcgrpcfg, samples, ADC_BUF_DEPTH);
	    _tpx = samples[1];

	    // read y channel (invert)
	    palClearPad(GPIOB, GPIOB_DRIVEA);
	    palSetPad(GPIOB, GPIOB_DRIVEB);
	    chThdSleepMilliseconds(1);
	    adcConvert(&ADCD1, &adcgrpcfg, samples, ADC_BUF_DEPTH);
	    _tpy = samples[0];

	    // ready for next read
	    palClearPad(GPIOB, GPIOB_DRIVEA);
	    palClearPad(GPIOB, GPIOB_DRIVEB);

	    chSysLock();
	   	tpx = _tpx;
	   	tpy = _tpy;
	   	detect = _detect;
	    last_update = now;
	    chSysUnlock();
	}
}

/**
 * @brief   Initialise the board for the touch.
 *
 * @notapi
 */
static inline void init_board(void) {
	adcStart(&ADCD1, NULL);
	last_update = chTimeNow();

	// leave DRIVEA & DRIVEB ready for next read
	palClearPad(GPIOB, GPIOB_DRIVEA);
	palClearPad(GPIOB, GPIOB_DRIVEB);
	chThdSleepMilliseconds(1);
}

/**
 * @brief   Check whether the surface is currently touched
 * @return	TRUE if the surface is currently touched
 *
 * @notapi
 */
static inline bool_t getpin_pressed(void) {
	read_mikro_tp();
	return (detect > 2000) ? true : false;
}

/**
 * @brief   Aquire the bus ready for readings
 *
 * @notapi
 */
static inline void aquire_bus(void) {

}

/**
 * @brief   Release the bus after readings
 *
 * @notapi
 */
static inline void release_bus(void) {

}

/**
 * @brief   Read an x value from touch controller
 * @return	The value read from the controller
 *
 * @notapi
 */
static inline uint16_t read_x_value(void) {
	read_mikro_tp();
	return tpx;
}

/**
 * @brief   Read an y value from touch controller
 * @return	The value read from the controller
 *
 * @notapi
 */
static inline uint16_t read_y_value(void) {
	read_mikro_tp();
	return tpy;
}

#endif /* _GINPUT_LLD_MOUSE_BOARD_H */
