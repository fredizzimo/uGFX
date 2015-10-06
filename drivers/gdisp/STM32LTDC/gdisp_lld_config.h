/*
 * This file is subject to the terms of the GFX License. If a copy of
 * the license was not distributed with this file, you can obtain one at:
 *
 *              http://ugfx.org/license.html
 */

#ifndef _GDISP_LLD_CONFIG_H
#define _GDISP_LLD_CONFIG_H

#if GFX_USE_GDISP

/*===========================================================================*/
/* Driver hardware support.                                                  */
/*===========================================================================*/

#define	LTDC_USE_DMA2D					FALSE	// Currently has display artifacts
#define GDISP_HARDWARE_DRAWPIXEL		TRUE
#define GDISP_HARDWARE_PIXELREAD		TRUE
#define GDISP_HARDWARE_CONTROL			TRUE
#define GDISP_LLD_PIXELFORMAT			GDISP_PIXELFORMAT_RGB565


/*===========================================================================*/
/* Don't change stuff below this line. Please.                               */
/*===========================================================================*/

#if LTDC_USE_DMA2D
 	#define GDISP_HARDWARE_FILLS		TRUE
 	#define GDISP_HARDWARE_BITFILLS		TRUE
#else
 	#define GDISP_HARDWARE_FILLS		FALSE
 	#define GDISP_HARDWARE_BITFILLS		FALSE
#endif /* GDISP_USE_DMA2D */

#endif	/* GFX_USE_GDISP */

#endif	/* _GDISP_LLD_CONFIG_H */
