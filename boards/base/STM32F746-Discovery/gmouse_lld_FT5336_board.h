/*
 * This file is subject to the terms of the GFX License. If a copy of
 * the license was not distributed with this file, you can obtain one at:
 *
 *              http://ugfx.org/license.html
 */

#ifndef _GINPUT_LLD_MOUSE_BOARD_H
#define _GINPUT_LLD_MOUSE_BOARD_H

#include "stm32f7xx.h"

// Resolution and Accuracy Settings
#define GMOUSE_FT5336_PEN_CALIBRATE_ERROR		8
#define GMOUSE_FT5336_PEN_CLICK_ERROR			6
#define GMOUSE_FT5336_PEN_MOVE_ERROR			4
#define GMOUSE_FT5336_FINGER_CALIBRATE_ERROR	14
#define GMOUSE_FT5336_FINGER_CLICK_ERROR		18
#define GMOUSE_FT5336_FINGER_MOVE_ERROR			14

// How much extra data to allocate at the end of the GMouse structure for the board's use
#define GMOUSE_FT5336_BOARD_DATA_SIZE			0

// Set this to TRUE if you want self-calibration.
//	NOTE:	This is not as accurate as real calibration.
//			It requires the orientation of the touch panel to match the display.
//			It requires the active area of the touch panel to exactly match the display size.
#define GMOUSE_FT5336_SELF_CALIBRATE			FALSE

// The FT5336 slave address
#define FT5336_ADDR 0x70

static bool_t init_board(GMouse* m, unsigned instance)
{
	(void)m;
	(void)instance;

	// I2C3_SCL    GPIOH7, alternate, opendrain, highspeed
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOBEN;					// Enable clock for
	GPIOB->MODER |= GPIO_MODER_MODER7_1;					// Alternate function
	GPIOB->OTYPER |= GPIO_OTYPER_OT_7;						// OpenDrain
	GPIOB->PUPDR |= GPIO_PUPDR_PUPDR7_0;					// PullUp
	GPIOB->OSPEEDR |= GPIO_OSPEEDER_OSPEEDR7;				// HighSpeed
	GPIOB->AFR[0] |= (0b0100 << 4*0);						// AF4

	// I2C3_SDA    GPIOH8, alternate, opendrain, highspeed
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOBEN;					// Enable clock
	GPIOB->MODER |= GPIO_MODER_MODER8_1;					// Alternate function
	GPIOB->OTYPER |= GPIO_OTYPER_OT_8;						// OpenDrain
	GPIOB->PUPDR |= GPIO_PUPDR_PUPDR8_0;					// PullUp
	GPIOB->OSPEEDR |= GPIO_OSPEEDER_OSPEEDR8;				// HighSpeed
	GPIOB->AFR[1] |= (0b0100 << 4*0);						// AF4

	// Enable I2C3 peripheral clock
	RCC->APB1ENR |= RCC_APB1ENR_I2C3EN;

	// Reset I2C3 peripheral
	RCC->APB1RSTR |= RCC_APB1RSTR_I2C3RST;
	RCC->APB1RSTR &= ~RCC_APB1RSTR_I2C3RST;

	// Set Fm mode
	I2C3->CCR |= I2C_CCR_FS;

	// Set Duty to 50:50
	I2C3->CCR &= ~I2C_CCR_DUTY;

	// Set peripheral clock frequency (APB1 frequency)
	// APB1CLK running at 42 MHz
	I2C3->CR2 |= 42;

	// Set I2C bus clock speed to 400 kHz
	// Period of 400 kHz is 2.5 us, half of that is 1.25 us. TPCLK1 is
    // 40 ns (see below). 1.25 us / 40 ns = 32.
	I2C3->CCR |= (I2C_CCR_CCR & 32);

	// Rise time
	// Period of 42 MHz is 24 ns. Rise time is 1000 ns. 1000/24 = 42.
	I2C3->TRISE |= (I2C_TRISE_TRISE & 42);

	// Disable POS
	I2C3->CR1 &=~ I2C_CR1_POS;

	// Enable I2C3
	I2C3->CR1 |= I2C_CR1_PE;

	return TRUE;
}

static void write_reg(GMouse* m, uint8_t reg, uint8_t val)
{
	(void)m;

	// Generate start condition
	I2C3->CR1 |= I2C_CR1_START;
	while (!(I2C3->SR2 & I2C_SR2_MSL));
	while (!(I2C3->SR1 & I2C_SR1_SB));

	// Send slave address (Write = last bit is 0)
	I2C3->DR = ((FT5336_ADDR | 0) & I2C_DR_DR);
	while (!(I2C3->SR1 & I2C_SR1_ADDR));

	// Read SR1/SR2 register to clear the SB bit (see STM32F4xx RM)
	(void)I2C3->SR1;
	(void)I2C3->SR2;

	// Send register address
	while (!(I2C3->SR1 & I2C_SR1_TXE));
	I2C3->DR = (reg & I2C_DR_DR);

	// Send data
	while (!(I2C3->SR1 & I2C_SR1_TXE));
	I2C3->DR = (val & I2C_DR_DR);

	// Generate stop condition when we are done transmitting
	while (!(I2C3->SR1 & I2C_SR1_TXE));
	I2C3->CR1 |= I2C_CR1_STOP;
}

static uint8_t read_byte(GMouse* m, uint8_t reg)
{
	(void)m;

	uint8_t ret = 0x00;

	// Generate start condition
	I2C3->CR1 |= I2C_CR1_START;
	while (!(I2C3->SR2 & I2C_SR2_MSL));
	while (!(I2C3->SR1 & I2C_SR1_SB));

	// Send slave address (Write = last bit is 0)
	I2C3->DR = ((FT5336_ADDR | 0) & I2C_DR_DR);
	while (!(I2C3->SR1 & I2C_SR1_ADDR));

	// Read SR1/SR2 register to clear the SB bit (see STM32F4xx RM)
	(void)I2C3->SR1;
	(void)I2C3->SR2;

	// Send register address that we want to read
	I2C3->DR = (reg & I2C_DR_DR);
	while (!(I2C3->SR1 & I2C_SR1_BTF));

	// Generate start condition (repeated start)
	I2C3->CR1 |= I2C_CR1_START;
	while (!(I2C3->SR2 & I2C_SR2_MSL));
	while (!(I2C3->SR1 & I2C_SR1_SB));

	// Send slave address (Read = last bit is 1)
	I2C3->DR = ((FT5336_ADDR | 1) & I2C_DR_DR);
	while (!(I2C3->SR1 & I2C_SR1_ADDR));

	// Set up for one byte receival
	I2C3->CR1 &= ~I2C_CR1_POS;
	//I2C3->CR1 |= I2C_CR1_ACK;

	// Read SR1/SR2 register to clear the SB bit (see STM32F4xx RM)
	(void)I2C3->SR1;
	(void)I2C3->SR2;

	// Clean SR1_ACK. This needs to be done on the last byte received from slave
	I2C3->CR1 &= ~I2C_CR1_ACK;

	// Generate stop condition
	I2C3->CR1 |= I2C_CR1_STOP;

	return ret;
}

static uint16_t read_word(GMouse* m, uint8_t reg)
{
	(void)m;

	uint16_t ret = 0x00;

	// Generate start condition
	I2C3->CR1 |= I2C_CR1_START;
	while (!(I2C3->SR2 & I2C_SR2_MSL));
	while (!(I2C3->SR1 & I2C_SR1_SB));

	// Send slave address (Write = last bit is 0)
	I2C3->DR = ((FT5336_ADDR | 0) & I2C_DR_DR);
	while (!(I2C3->SR1 & I2C_SR1_ADDR));

	// Read SR1/SR2 register to clear the SB bit (see STM32F4xx RM)
	(void)I2C3->SR1;
	(void)I2C3->SR2;

	// Send register address that we want to read
	I2C3->DR = (reg & I2C_DR_DR);
	while (!(I2C3->SR1 & I2C_SR1_BTF));

	// Generate start condition (repeated start)
	I2C3->CR1 |= I2C_CR1_START;
	while (!(I2C3->SR2 & I2C_SR2_MSL));
	while (!(I2C3->SR1 & I2C_SR1_SB));

	// Send slave address (Read = last bit is 1)
	I2C3->DR = ((FT5336_ADDR | 1) & I2C_DR_DR);
	while (!(I2C3->SR1 & I2C_SR1_ADDR));

	// Set up for two byte receival
	I2C3->CR1 |= I2C_CR1_POS;
	//I2C3->CR1 &= ~I2C_CR1_ACK;
	I2C3->SR1 &= ~I2C_SR1_ADDR;

	// Read SR1/SR2 register to clear the SB bit (see STM32F4xx RM)
	(void)I2C3->SR1;
	(void)I2C3->SR2;

	// The slave should now send a byte to us.
	while (!(I2C3->SR1 & I2C_SR1_RXNE));
	ret = (uint16_t)((I2C3->DR & 0x00FF) << 8);

	// Set STOP and clear ACK right after reading the second last byte
	I2C3->CR1 |= I2C_CR1_STOP;
	I2C3->CR1 &= ~I2C_CR1_ACK;

	// The second byte becomes available after sending the stop condition
	ret |= (I2C3->DR & 0x00FF);

	// Get back to original state
	I2C3->CR1 &= ~I2C_CR1_POS;

	return ret;
}

#endif /* _GINPUT_LLD_MOUSE_BOARD_H */
