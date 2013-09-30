/*
 * This file is subject to the terms of the GFX License. If a copy of
 * the license was not distributed with this file, you can obtain one at:
 *
 *              http://ugfx.org/license.html
 */

/**
 * @file    drivers/gdisp/HX8347D/gdisp_lld_board_st_stm32f4_discovery.h
 * @brief   GDISP Graphic Driver subsystem board SPI interface for the HX8347D display.
 *
 * @addtogroup GDISP
 * @{
 */

#ifndef _GDISP_LLD_BOARD_H
#define _GDISP_LLD_BOARD_H

// Overrides
#ifndef GDISP_INITIAL_BACKLIGHT
	#define GDISP_INITIAL_BACKLIGHT	50
#endif

/* Pin assignments */
#define SET_RST		palSetPad(GPIOB, 8)
#define CLR_RST		palClearPad(GPIOB, 8)
#define SET_DATA	palSetPad(GPIOB, 9)
#define CLR_DATA	palClearPad(GPIOB, 9)
#define SET_CS		palSetPad(GPIOA, 4)
#define CLR_CS		palClearPad(GPIOA, 4)

/* PWM configuration structure. We use timer 4 channel 2 (orange LED on board). */
static const PWMConfig pwmcfg = {
	1000000,		/* 1 MHz PWM clock frequency. */
	100,			/* PWM period is 100 cycles. */
	NULL,
	{
		{PWM_OUTPUT_ACTIVE_HIGH, NULL},
		{PWM_OUTPUT_ACTIVE_HIGH, NULL},
		{PWM_OUTPUT_ACTIVE_HIGH, NULL},
		{PWM_OUTPUT_ACTIVE_HIGH, NULL}
	},
	0
};

/*
 * SPI1 configuration structure.
 * Speed 42MHz, CPHA=0, CPOL=0, 8bits frames, MSb transmitted first.
 * The slave select line is the pin 4 on the port GPIOA.
 */
static const SPIConfig spi1cfg_8bit = {
	NULL,
	/* HW dependent part.*/
	GPIOA,
	4,
	0 //SPI_CR1_BR_0
};

/*
 * SPI1 configuration structure.
 * Speed 42MHz, CPHA=0, CPOL=0, 16bits frames, MSb transmitted first.
 * The slave select line is the pin 4 on the port GPIOA.
 */
static const SPIConfig spi1cfg_16bit = {
	NULL,
	/* HW dependent part.*/
	GPIOA,
	4,
	SPI_CR1_DFF //SPI_CR1_BR_0
};

/**
 * @brief   Initialise the board for the display.
 * @notes	This board definition uses GPIO and SPI. It assumes exclusive access to these GPIO pins but not necessarily the SPI port.
 *
 * @notapi
 */
static inline void init_board(void) {
	/* Display backlight control */
	/* TIM4 is an alternate function 2 (AF2) */
	pwmStart(&PWMD4, &pwmcfg);
	palSetPadMode(GPIOD, 13, PAL_MODE_ALTERNATE(2));
	pwmEnableChannel(&PWMD4, 1, 100);

	palSetPadMode(GPIOB, 8, PAL_MODE_OUTPUT_PUSHPULL|PAL_STM32_OSPEED_HIGHEST);			/* RST    */
	palSetPadMode(GPIOB, 9, PAL_MODE_OUTPUT_PUSHPULL|PAL_STM32_OSPEED_HIGHEST);			/* RS     */
	/*
	 * Initializes the SPI driver 1. The SPI1 signals are routed as follow:
	 * PB12 - NSS.
	 * PB13 - SCK.
	 * PB14 - MISO.
	 * PB15 - MOSI.
	 */
	SET_CS; SET_DATA;
	spiStart(&SPID1, &spi1cfg_8bit);
	palSetPadMode(GPIOA, 4, PAL_MODE_OUTPUT_PUSHPULL|PAL_STM32_OSPEED_HIGHEST);			/* NSS.     */
	palSetPadMode(GPIOA, 5, PAL_MODE_ALTERNATE(5)|PAL_STM32_OSPEED_HIGHEST);			/* SCK.     */
	palSetPadMode(GPIOA, 6, PAL_MODE_ALTERNATE(5));										/* MISO.    */
	palSetPadMode(GPIOA, 7, PAL_MODE_ALTERNATE(5)|PAL_STM32_OSPEED_HIGHEST);			/* MOSI.    */

}

/**
 * @brief   Set or clear the lcd reset pin.
 *
 * @param[in] state		TRUE = lcd in reset, FALSE = normal operation
 *
 * @notapi
 */
static inline void setpin_reset(bool_t state) {
	if (state) {
		CLR_RST;
	} else {
		SET_RST;
	}
}

/**
 * @brief   Set the lcd back-light level.
 *
 * @param[in] percent		0 to 100%
 *
 * @notapi
 */
static inline void set_backlight(uint8_t percent) {
	pwmEnableChannel(&PWMD4, 1, percent);
}

/**
 * @brief   Take exclusive control of the bus
 * @notapi
 */
static inline void acquire_bus(void) {
	spiAcquireBus(&SPID1);
    while(((SPI1->SR & SPI_SR_TXE) == 0) || ((SPI1->SR & SPI_SR_BSY) != 0));		// Safety
	CLR_CS;
}

/**
 * @brief   Release exclusive control of the bus
 * @notapi
 */
static inline void release_bus(void) {
	SET_CS;
	spiReleaseBus(&SPID1);
}

/**
 * @brief   Set the bus in 16 bit mode
 * @notapi
 */
static inline void busmode16(void) {
	spiStart(&SPID1, &spi1cfg_16bit);
}

/**
 * @brief   Set the bus in 8 bit mode (the default)
 * @notapi
 */
static inline void busmode8(void) {
	spiStart(&SPID1, &spi1cfg_8bit);
}

/**
 * @brief   Set which index register to use.
 *
 * @param[in] index		The index register to set
 *
 * @notapi
 */
static inline void write_index(uint8_t cmd) {
    CLR_DATA;
    SPI1->DR = cmd;
    while(((SPI1->SR & SPI_SR_TXE) == 0) || ((SPI1->SR & SPI_SR_BSY) != 0));
    SET_DATA;
}

/**
 * @brief   Send a command to the lcd.
 *
 * @param[in] data		The data to send
 *
 * @notapi
 */
static inline void write_reg(uint8_t cmd, uint8_t data) {
	write_index(cmd);
    SPI1->DR = data;
    while(((SPI1->SR & SPI_SR_TXE) == 0) || ((SPI1->SR & SPI_SR_BSY) != 0));
}

static inline void write_ram16(uint16_t data) {
    SPI1->DR      = data;
    while((SPI1->SR & SPI_SR_TXE) == 0);
}

#endif /* _GDISP_LLD_BOARD_H */
/** @} */
