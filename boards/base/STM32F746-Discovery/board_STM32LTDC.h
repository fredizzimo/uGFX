/*
 * This file is subject to the terms of the GFX License. If a copy of
 * the license was not distributed with this file, you can obtain one at:
 *
 *              http://ugfx.org/license.html
 */

#ifndef _GDISP_LLD_BOARD_H
#define _GDISP_LLD_BOARD_H

#include "stm32f7xx_ll_fmc.h"
#include "stm32f746g_discovery_sdram.h"
#include <string.h>

static const ltdcConfig driverCfg = {
	480, 272,								// Width, Height (pixels)
	41, 10,									// Horizontal, Vertical sync (pixels)
	13, 2,									// Horizontal, Vertical back porch (pixels)
	32, 2,									// Horizontal, Vertical front porch (pixels)
	0,										// Sync flags
	0x000000,								// Clear color (RGB888)

	{										// Background layer config
		(LLDCOLOR_TYPE *)SDRAM_DEVICE_ADDR, // Frame buffer address
		480, 272,							// Width, Height (pixels)
		480 * LTDC_PIXELBYTES,				// Line pitch (bytes)
		LTDC_PIXELFORMAT,					// Pixel format
		0, 0,								// Start pixel position (x, y)
		480, 272,							// Size of virtual layer (cx, cy)
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

/**
  * @brief LCD special pins
  */
/* Display enable pin */
#define LCD_DISP_PIN                    GPIO_PIN_12
#define LCD_DISP_GPIO_PORT              GPIOI

/* Backlight control pin */
#define LCD_BL_CTRL_PIN                  GPIO_PIN_3
#define LCD_BL_CTRL_GPIO_PORT            GPIOK

/* Display timing */
#define RK043FN48H_FREQUENCY_DIVIDER	5

static void SetBoardPinDirectionsForLCD(void)
{
  GPIO_InitTypeDef gpio_init_structure;

  // Enable GPIOs clock */
  __HAL_RCC_GPIOE_CLK_ENABLE();
  __HAL_RCC_GPIOG_CLK_ENABLE();
  __HAL_RCC_GPIOI_CLK_ENABLE();		// Display pin
  __HAL_RCC_GPIOJ_CLK_ENABLE();
  __HAL_RCC_GPIOK_CLK_ENABLE();		// BL pin

  //*** LTDC Pins configuration
  // GPIOE configuration
  gpio_init_structure.Pin       = GPIO_PIN_4;
  gpio_init_structure.Mode      = GPIO_MODE_AF_PP;
  gpio_init_structure.Pull      = GPIO_NOPULL;
  gpio_init_structure.Speed     = GPIO_SPEED_FAST;
  gpio_init_structure.Alternate = GPIO_AF14_LTDC;
  HAL_GPIO_Init(GPIOE, &gpio_init_structure);

  // GPIOG configuration
  gpio_init_structure.Pin       = GPIO_PIN_12;
  gpio_init_structure.Mode      = GPIO_MODE_AF_PP;
  gpio_init_structure.Alternate = GPIO_AF9_LTDC;
  HAL_GPIO_Init(GPIOG, &gpio_init_structure);

  // GPIOI LTDC alternate configuration
  gpio_init_structure.Pin       = GPIO_PIN_8 | GPIO_PIN_9 | GPIO_PIN_10 | \
                                  GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_15;
  gpio_init_structure.Mode      = GPIO_MODE_AF_PP;
  gpio_init_structure.Alternate = GPIO_AF14_LTDC;
  HAL_GPIO_Init(GPIOI, &gpio_init_structure);

  // GPIOJ configuration
  gpio_init_structure.Pin       = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3 | \
                                  GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7 | \
                                  GPIO_PIN_8 | GPIO_PIN_9 | GPIO_PIN_10 | GPIO_PIN_11 | \
                                  GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_15;
  gpio_init_structure.Mode      = GPIO_MODE_AF_PP;
  gpio_init_structure.Alternate = GPIO_AF14_LTDC;
  HAL_GPIO_Init(GPIOJ, &gpio_init_structure);

  // GPIOK configuration
  gpio_init_structure.Pin       = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_4 | \
                                  GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7;
  gpio_init_structure.Mode      = GPIO_MODE_AF_PP;
  gpio_init_structure.Alternate = GPIO_AF14_LTDC;
  HAL_GPIO_Init(GPIOK, &gpio_init_structure);

  // LCD_DISP GPIO configuration
  gpio_init_structure.Pin       = LCD_DISP_PIN;     /* LCD_DISP pin has to be manually controlled */
  gpio_init_structure.Mode      = GPIO_MODE_OUTPUT_PP;
  HAL_GPIO_Init(LCD_DISP_GPIO_PORT, &gpio_init_structure);

  // LCD_BL_CTRL GPIO configuration
  gpio_init_structure.Pin       = LCD_BL_CTRL_PIN;  /* LCD_BL_CTRL pin has to be manually controlled */
  gpio_init_structure.Mode      = GPIO_MODE_OUTPUT_PP;
  HAL_GPIO_Init(LCD_BL_CTRL_GPIO_PORT, &gpio_init_structure);
}

static inline void init_board(GDisplay *g) {

	// As we are not using multiple displays we set g->board to NULL as we don't use it.
	g->board = 0;

	switch(g->controllerdisplay) {
	case 0:

		// Set pin directions
		SetBoardPinDirectionsForLCD();

		// Enable the display and turn on the backlight
		HAL_GPIO_WritePin(LCD_DISP_GPIO_PORT, LCD_DISP_PIN, GPIO_PIN_SET);
		HAL_GPIO_WritePin(LCD_BL_CTRL_GPIO_PORT, LCD_BL_CTRL_PIN, GPIO_PIN_SET);

		#define STM32_SAISRC_NOCLOCK    (0 << 23)   /**< No clock.                  */
		#define STM32_SAISRC_PLL        (1 << 23)   /**< SAI_CKIN is PLL.           */
		#define STM32_SAIR_DIV2         (0 << 16)   /**< R divided by 2.            */
		#define STM32_SAIR_DIV4         (1 << 16)   /**< R divided by 4.            */
		#define STM32_SAIR_DIV8         (2 << 16)   /**< R divided by 8.            */
		#define STM32_SAIR_DIV16        (3 << 16)   /**< R divided by 16.           */

		// RK043FN48H LCD clock configuration
		// PLLSAI_VCO Input = HSE_VALUE/PLL_M = 1 Mhz
		// PLLSAI_VCO Output = PLLSAI_VCO Input * PLLSAIN = 192 Mhz
		// PLLLCDCLK = PLLSAI_VCO Output/PLLSAIR = 192/5 = 38.4 Mhz
		// LTDC clock frequency = PLLLCDCLK / LTDC_PLLSAI_DIVR_4 = 38.4/4 = 9.6Mhz
		#define STM32_PLLSAIN_VALUE                 192
		#define STM32_PLLSAIQ_VALUE                 7
		#define STM32_PLLSAIR_VALUE                 RK043FN48H_FREQUENCY_DIVIDER
		#define STM32_PLLSAIR_POST                  STM32_SAIR_DIV4
		RCC->PLLSAICFGR = (STM32_PLLSAIN_VALUE << 6) | (STM32_PLLSAIR_VALUE << 28) | (STM32_PLLSAIQ_VALUE << 24);
		RCC->DCKCFGR1 = (RCC->DCKCFGR1 & ~RCC_DCKCFGR1_PLLSAIDIVR) | STM32_PLLSAIR_POST;
		RCC->CR |= RCC_CR_PLLSAION;

		// Initialise the SDRAM
		BSP_SDRAM_Init();

		// Clear the SDRAM
		//memset((void *)SDRAM_BANK_ADDR, 0, 0x400000);

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
