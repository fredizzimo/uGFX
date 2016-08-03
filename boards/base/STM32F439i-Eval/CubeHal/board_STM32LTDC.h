/*
 * This file is subject to the terms of the GFX License. If a copy of
 * the license was not distributed with this file, you can obtain one at:
 *
 *              http://ugfx.org/license.html
 */

#ifndef _GDISP_LLD_BOARD_H
#define _GDISP_LLD_BOARD_H

// Avoid naming collisions with CubeHAL
#undef Red
#undef Green
#undef Blue

// Include CubeHAL
#include "stm32f4xx_hal.h"
#include "stm324x9i_eval_sdram.h"

// Panel parameters
#define  AMPIRE640480_WIDTH    ((uint16_t)640)             /* LCD PIXEL WIDTH            */
#define  AMPIRE640480_HEIGHT   ((uint16_t)480)             /* LCD PIXEL HEIGHT           */
#define  AMPIRE640480_HSYNC            ((uint16_t)30)      /* Horizontal synchronization */
#define  AMPIRE640480_HBP              ((uint16_t)114)     /* Horizontal back porch      */
#define  AMPIRE640480_HFP              ((uint16_t)16)      /* Horizontal front porch     */
#define  AMPIRE640480_VSYNC            ((uint16_t)3)       /* Vertical synchronization   */
#define  AMPIRE640480_VBP              ((uint16_t)32)      /* Vertical back porch        */
#define  AMPIRE640480_VFP              ((uint16_t)10)      /* Vertical front porch       */
#define  AMPIRE640480_FREQUENCY_DIVIDER     3              /* LCD Frequency divider      */


static const ltdcConfig driverCfg = {
	640, 480,								// Width, Height (pixels)
	30, 3,									// Horizontal, Vertical sync (pixels)
	114, 32,								// Horizontal, Vertical back porch (pixels)
	16, 10,									// Horizontal, Vertical front porch (pixels)
	0,										// Sync flags
	0x000000,								// Clear color (RGB888)

	{										// Background layer config
		(LLDCOLOR_TYPE *)SDRAM_DEVICE_ADDR, // Frame buffer address
		640, 480,							// Width, Height (pixels)
		640 * LTDC_PIXELBYTES,				// Line pitch (bytes)
		LTDC_PIXELFORMAT,					// Pixel format
		0, 0,								// Start pixel position (x, y)
		640, 480,							// Size of virtual layer (cx, cy)
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

#define LCD_MAX_PCLK ((uint8_t)0x00)
static LTDC_HandleTypeDef hltdc_eval;
static uint32_t PCLK_profile = LCD_MAX_PCLK;

static void configureLcdPins(void)
{
	GPIO_InitTypeDef GPIO_Init_Structure;

	PCLK_profile = LCD_MAX_PCLK;
  
	// LTDC configuration
	hltdc_eval.Init.HorizontalSync = (AMPIRE640480_HSYNC - 1);
	hltdc_eval.Init.VerticalSync = (AMPIRE640480_VSYNC - 1);
	hltdc_eval.Init.AccumulatedHBP = (AMPIRE640480_HSYNC + AMPIRE640480_HBP - 1);
	hltdc_eval.Init.AccumulatedVBP = (AMPIRE640480_VSYNC + AMPIRE640480_VBP - 1);  
	hltdc_eval.Init.AccumulatedActiveH = (AMPIRE640480_HEIGHT + AMPIRE640480_VSYNC + AMPIRE640480_VBP - 1);
	hltdc_eval.Init.AccumulatedActiveW = (AMPIRE640480_WIDTH + AMPIRE640480_HSYNC + AMPIRE640480_HBP - 1);
	hltdc_eval.Init.TotalHeigh = (AMPIRE640480_HEIGHT + AMPIRE640480_VSYNC + AMPIRE640480_VBP + AMPIRE640480_VFP - 1);
	hltdc_eval.Init.TotalWidth = (AMPIRE640480_WIDTH + AMPIRE640480_HSYNC + AMPIRE640480_HBP + AMPIRE640480_HFP - 1);
	hltdc_eval.LayerCfg->ImageWidth  = AMPIRE640480_WIDTH;
	hltdc_eval.LayerCfg->ImageHeight = AMPIRE640480_HEIGHT;
	hltdc_eval.Init.Backcolor.Blue = 0;
	hltdc_eval.Init.Backcolor.Green = 0;
	hltdc_eval.Init.Backcolor.Red = 0;
	hltdc_eval.Init.HSPolarity = LTDC_HSPOLARITY_AL;
	hltdc_eval.Init.VSPolarity = LTDC_VSPOLARITY_AL; 
	hltdc_eval.Init.DEPolarity = LTDC_DEPOLARITY_AL;  
	hltdc_eval.Init.PCPolarity = LTDC_PCPOLARITY_IPC;
	hltdc_eval.Instance = LTDC;
	HAL_LTDC_Init(&hltdc_eval);

	// LCD clock configuration
	static RCC_PeriphCLKInitTypeDef  periph_clk_init_struct;
	periph_clk_init_struct.PLLSAI.PLLSAIN = 151;
	periph_clk_init_struct.PeriphClockSelection = RCC_PERIPHCLK_LTDC;    
	periph_clk_init_struct.PLLSAI.PLLSAIR = 3;    
	periph_clk_init_struct.PLLSAIDivR = RCC_PLLSAIDIVR_2;
	HAL_RCCEx_PeriphCLKConfig(&periph_clk_init_struct);

	// Enable peripheral clocks
	__LTDC_CLK_ENABLE();
	__DMA2D_CLK_ENABLE(); 
	__GPIOI_CLK_ENABLE(); 
	__GPIOJ_CLK_ENABLE();
	__GPIOK_CLK_ENABLE(); 

	/*** LTDC Pins configuration ***/
	// GPIOI
	GPIO_Init_Structure.Pin       = GPIO_PIN_12 | GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_15; 
	GPIO_Init_Structure.Mode      = GPIO_MODE_AF_PP;
	GPIO_Init_Structure.Pull      = GPIO_NOPULL;
	GPIO_Init_Structure.Speed     = GPIO_SPEED_FAST;
	GPIO_Init_Structure.Alternate = GPIO_AF14_LTDC;  
	HAL_GPIO_Init(GPIOI, &GPIO_Init_Structure);

	// GPIOJ  
	GPIO_Init_Structure.Pin       = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3 | \
	                                GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7 | \
	                                GPIO_PIN_8 | GPIO_PIN_9 | GPIO_PIN_10 | GPIO_PIN_11 | \
	                                GPIO_PIN_12 | GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_15; 
	GPIO_Init_Structure.Mode      = GPIO_MODE_AF_PP;
	GPIO_Init_Structure.Pull      = GPIO_NOPULL;
	GPIO_Init_Structure.Speed     = GPIO_SPEED_FAST;
	GPIO_Init_Structure.Alternate = GPIO_AF14_LTDC;  
	HAL_GPIO_Init(GPIOJ, &GPIO_Init_Structure);  

	// GPIOK configuration 
	GPIO_Init_Structure.Pin       = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3 | \
	                                GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7; 
	GPIO_Init_Structure.Mode      = GPIO_MODE_AF_PP;
	GPIO_Init_Structure.Pull      = GPIO_NOPULL;
	GPIO_Init_Structure.Speed     = GPIO_SPEED_FAST;
	GPIO_Init_Structure.Alternate = GPIO_AF14_LTDC;  
	HAL_GPIO_Init(GPIOK, &GPIO_Init_Structure);
}

static GFXINLINE void init_board(GDisplay* g)
{
	// As we are not using multiple displays we set g->board to NULL as we don't use it
	g->board = 0;

	switch(g->controllerdisplay) {
		case 0:

		// Set pin directions
		configureLcdPins();

		// Initialise the SDRAM
		BSP_SDRAM_Init();

		break;
	}
}

static GFXINLINE void post_init_board(GDisplay* g)
{
	(void) g;
}

static GFXINLINE void set_backlight(GDisplay* g, uint8_t percent)
{
	(void) g;
}

#endif /* _GDISP_LLD_BOARD_H */
