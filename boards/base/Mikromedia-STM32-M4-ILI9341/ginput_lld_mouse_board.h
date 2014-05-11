/*
 * This file is subject to the terms of the GFX License. If a copy of
 * the license was not distributed with this file, you can obtain one at:
 *
 *              http://ugfx.org/license.html
 */

#ifndef _GINPUT_LLD_MOUSE_BOARD_H
#define _GINPUT_LLD_MOUSE_BOARD_H

#define ADC_NUM_CHANNELS   2
#define ADC_BUF_DEPTH      1

static const ADCConversionGroup adcgrpcfg = {
  FALSE,
  ADC_NUM_CHANNELS,
  0,
  0,
  /* HW dependent part.*/
  0,
  ADC_CR2_SWSTART,
  0,
  0,
  ADC_SQR1_NUM_CH(ADC_NUM_CHANNELS),
  0,
  ADC_SQR3_SQ2_N(ADC_CHANNEL_IN8) | ADC_SQR3_SQ1_N(ADC_CHANNEL_IN9)
};

static inline void init_board(void) {
	adcStart(&ADCD1, 0);
}

static inline void aquire_bus(void) {

}

static inline void release_bus(void) {
}

static inline void setup_x(void) {
   	palSetPad(GPIOB, GPIOB_DRIVEA);
    palClearPad(GPIOB, GPIOB_DRIVEB);
    chThdSleepMilliseconds(2);
}

static inline void setup_y(void) {
    palClearPad(GPIOB, GPIOB_DRIVEA);
    palSetPad(GPIOB, GPIOB_DRIVEB);
    chThdSleepMilliseconds(2);
}

static inline void setup_z(void) {
	palClearPad(GPIOB, GPIOB_DRIVEA);
	palClearPad(GPIOB, GPIOB_DRIVEB);
    chThdSleepMilliseconds(2);
}

static inline uint16_t read_x(void) {
	adcsample_t samples[ADC_NUM_CHANNELS * ADC_BUF_DEPTH];

    adcConvert(&ADCD1, &adcgrpcfg, samples, ADC_BUF_DEPTH);
	return samples[1];
}

static inline uint16_t read_y(void) {
	adcsample_t samples[ADC_NUM_CHANNELS * ADC_BUF_DEPTH];

    adcConvert(&ADCD1, &adcgrpcfg, samples, ADC_BUF_DEPTH);
    return samples[0];
}

static inline uint16_t read_z(void) {
	adcsample_t samples[ADC_NUM_CHANNELS * ADC_BUF_DEPTH];

    adcConvert(&ADCD1, &adcgrpcfg, samples, ADC_BUF_DEPTH);
    // z will go from ~200 to ~4000 when pressed
    // auto range this back to 0 to 100
    if (samples[0] > 4000)
    	return 100;
    if (samples[0] < 400)
    	return 0;
    return (samples[0] - 400) / ((4000-400)/100);
}

#endif /* _GINPUT_LLD_MOUSE_BOARD_H */
