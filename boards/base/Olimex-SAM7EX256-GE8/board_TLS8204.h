/*
 * This file is subject to the terms of the GFX License. If a copy of
 * the license was not distributed with this file, you can obtain one at:
 *
 *              http://ugfx.org/license.html
 */

#ifndef _GDISP_LLD_BOARD_H
#define _GDISP_LLD_BOARD_H

#include "board_uext.h"

#define PORT_RESET		UEXT_PORT_PIN5
#define PIN_RESET		UEXT_PORTPIN_PIN5
#define PORT_DC			UEXT_PORT_PIN6
#define PIN_DC			UEXT_PORTPIN_PIN6

#define BIT_BASH		TRUE				// Native SPI is not working yet
#define DIRECT_IO		TRUE				// ChibiOS for the AT91SAM7 requires patch 7669 or higher if DIRECT_IO is FALSE

#if DIRECT_IO
	#define PinIsOutput(port,pin)		((port)->PIO_OER = 1 << (pin), (port)->PIO_PER = 1 << (pin), (port)->PIO_MDDR = 1 << (pin), (port)->PIO_PPUDR = 1 << (pin))
	#define PinSet(port,pin)			(port)->PIO_SODR = 1 << (pin)
	#define PinClear(port,pin)			(port)->PIO_CODR = 1 << (pin)
#else
	#define PinIsOutput(port,pin)		palSetPadMode((port), (pin), PAL_MODE_OUTPUT_PUSHPULL)
	#define PinSet(port,pin)			palSetPad((port), (pin))
	#define PinClear(port,pin)			palClearPad((port), (pin))
#endif

#if BIT_BASH
	// simple delays
	void Delay(volatile unsigned long a) { while (a!=0) a--; }
	void Delayc(volatile unsigned char a) { while (a!=0) a--; }
#else
	static const SPIConfig spiconfig = {
		0,
		/* HW dependent part.*/
		UEXT_SPI_CS_PORT,
		UEXT_SPI_CS_PORTPIN,
		0x01010801			// For AT91SAM7:	8bit, CPOL=1, NCPHA = 0, ClockPhase=0, SCLK = 48Mhz/8 = 6MHz
	};
#endif

static inline void init_board(GDisplay *g) {
	(void) g;

	PinIsOutput	(PORT_DC, PIN_DC);
	PinIsOutput	(PORT_RESET, PIN_RESET);
	PinSet		(PORT_RESET, PIN_RESET);

	PinIsOutput	(UEXT_SPI_CS_PORT, UEXT_SPI_CS_PORTPIN);
	PinSet		(UEXT_SPI_CS_PORT, UEXT_SPI_CS_PORTPIN);

	#if BIT_BASH
		PinIsOutput	(UEXT_SPI_MOSI_PORT, UEXT_SPI_MOSI_PORTPIN);
		PinSet		(UEXT_SPI_MOSI_PORT, UEXT_SPI_MOSI_PORTPIN);
		PinIsOutput	(UEXT_SPI_SCK_PORT, UEXT_SPI_SCK_PORTPIN);
		PinSet		(UEXT_SPI_SCK_PORT, UEXT_SPI_SCK_PORTPIN);
	#endif
}

static inline void post_init_board(GDisplay *g) {
	(void) g;
}

static inline void setpin_reset(GDisplay *g, bool_t state) {
	(void) g;
	(void) state;

	if (state)
		PinClear(PORT_RESET, PIN_RESET);
	else
		PinSet(PORT_RESET, PIN_RESET);
}

static inline void set_backlight(GDisplay *g, uint8_t percent) {
	(void) g;
	(void) percent;
}

static inline void acquire_bus(GDisplay *g) {
	(void) g;

	#if BIT_BASH
		PinClear(UEXT_SPI_CS_PORT, UEXT_SPI_CS_PORTPIN);
	#else
		spiStart(UEXT_SPI, &spiconfig);
		spiSelect(UEXT_SPI);
	#endif
}

static inline void release_bus(GDisplay *g) {
	(void) g;

	#if BIT_BASH
		PinSet(UEXT_SPI_CS_PORT, UEXT_SPI_CS_PORTPIN);
	#else
		spiUnselect(UEXT_SPI);
		spiStop(UEXT_SPI);
	#endif
}

static inline void write_cmd(GDisplay *g, uint8_t cmd) {
	(void) g;

	// Command mode please
	PinClear(PORT_DC, PIN_DC);

	#if BIT_BASH
	{
		uint8_t bit;


		for(bit = 0x80; bit; bit >>= 1) {
			if(cmd & bit)
				PinSet(UEXT_SPI_MOSI_PORT, UEXT_SPI_MOSI_PORTPIN);
			else
				PinClear(UEXT_SPI_MOSI_PORT, UEXT_SPI_MOSI_PORTPIN);
			Delay(1);
			PinClear(UEXT_SPI_SCK_PORT, UEXT_SPI_SCK_PORTPIN);
			Delay(1);
			PinSet(UEXT_SPI_SCK_PORT, UEXT_SPI_SCK_PORTPIN);
		}
	}
	#else
		spiStartSend(UEXT_SPI, 1, &cmd);
	#endif
}

static inline void write_data(GDisplay *g, uint8_t* data, uint16_t length) {
	(void) g;

	// Data mode please
	PinSet(PORT_DC, PIN_DC);

	#if BIT_BASH
		while(length--) {
			uint8_t bit;

			for(bit = 0x80; bit; bit >>= 1) {
				if(*data & bit)
					PinSet(UEXT_SPI_MOSI_PORT, UEXT_SPI_MOSI_PORTPIN);
				else
					PinClear(UEXT_SPI_MOSI_PORT, UEXT_SPI_MOSI_PORTPIN);
				Delay(1);
				PinClear(UEXT_SPI_SCK_PORT, UEXT_SPI_SCK_PORTPIN);
				Delay(1);
				PinSet(UEXT_SPI_SCK_PORT, UEXT_SPI_SCK_PORTPIN);
			}
			data++;
		}
	#else
		spiStartSend(UEXT_SPI, length, data);
	#endif
}

#endif /* _GDISP_LLD_BOARD_H */
