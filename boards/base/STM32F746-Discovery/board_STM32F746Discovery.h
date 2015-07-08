/*
 * This file is subject to the terms of the GFX License. If a copy of
 * the license was not distributed with this file, you can obtain one at:
 *
 *              http://ugfx.org/license.html
 */

#ifndef _GDISP_LLD_BOARD_H
#define _GDISP_LLD_BOARD_H

#include "stm32f4xx_fmc.h"
#include "stm32f429i_discovery_sdram.h"
#include <string.h>

static const ltdcConfig driverCfg = {
	480, 270,								// Width, Height (pixels)
	41, 10,									// Horizontal, Vertical sync (pixels)
	13, 2,									// Horizontal, Vertical back porch (pixels)
	32, 2,									// Horizontal, Vertical front porch (pixels)
	0,										// Sync flags
	0x000000,								// Clear color (RGB888)

	{										// Background layer config
		(LLDCOLOR_TYPE *)SDRAM_BANK_ADDR,	// Frame buffer address
		480, 270,							// Width, Height (pixels)
		480 * LTDC_PIXELBYTES,				// Line pitch (bytes)
		LTDC_PIXELFORMAT,					// Pixel format
		0, 0,								// Start pixel position (x, y)
		480, 270,							// Size of virtual layer (cx, cy)
		LTDC_COLOR_FUCHSIA,					// Default color (ARGB8888)
		0x980088,							// Color key (RGB888)
		LTDC_BLEND_FIX1_FIX2,				// Blending factors
		0,									// Palette (RGB888, can be NULL)
		0,									// Palette length
		0xFF,								// Constant alpha factor
		LTDC_LEF_ENABLE						// Layer configuration flags
	},

	LTDC_UNUSED_LAYER_CONFIG				// Foreground layer config
};

static inline void init_board(GDisplay *g) {

	// As we are not using multiple displays we set g->board to NULL as we don't use it.
	g->board = 0;

	switch(g->controllerdisplay) {
	case 0:	
		#define STM32_SAISRC_NOCLOCK    (0 << 23)   /**< No clock.                  */
		#define STM32_SAISRC_PLL        (1 << 23)   /**< SAI_CKIN is PLL.           */
		#define STM32_SAIR_DIV2         (0 << 16)   /**< R divided by 2.            */
		#define STM32_SAIR_DIV4         (1 << 16)   /**< R divided by 4.            */
		#define STM32_SAIR_DIV8         (2 << 16)   /**< R divided by 8.            */
		#define STM32_SAIR_DIV16        (3 << 16)   /**< R divided by 16.           */

		#define STM32_PLLSAIN_VALUE                 192
		#define STM32_PLLSAIQ_VALUE                 7
		#define STM32_PLLSAIR_VALUE                 4
		#define STM32_PLLSAIR_POST                  STM32_SAIR_DIV4

		/* PLLSAI activation.*/
		RCC->PLLSAICFGR = (STM32_PLLSAIN_VALUE << 6) | (STM32_PLLSAIR_VALUE << 28) | (STM32_PLLSAIQ_VALUE << 24);
		RCC->DCKCFGR = (RCC->DCKCFGR & ~RCC_DCKCFGR_PLLSAIDIVR) | STM32_PLLSAIR_POST;
		RCC->CR |= RCC_CR_PLLSAION;

		// Initialise the SDRAM
		SDRAM_Init();

		// Clear the SDRAM
		memset((void *)SDRAM_BANK_ADDR, 0, 0x400000);

		break;
	}
}

static inline void post_init_board(GDisplay *g) {
	(void) g;
}

static inline void set_backlight(GDisplay *g, uint8_t percent) {
	(void) g;
	(void) percent;
}

#endif /* _GDISP_LLD_BOARD_H */
