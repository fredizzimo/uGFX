/*
 * This file is subject to the terms of the GFX License. If a copy of
 * the license was not distributed with this file, you can obtain one at:
 *
 *              http://ugfx.org/license.html
 */

/**
 * @file    include/gdisp/lld/gdisp_lld.h
 * @brief   GDISP Graphic Driver subsystem low level driver header.
 *
 * @addtogroup GDISP
 * @{
 */

#ifndef _GDISP_LLD_H
#define _GDISP_LLD_H

#if GFX_USE_GDISP || defined(__DOXYGEN__)

/*===========================================================================*/
/* Error checks.                                                             */
/*===========================================================================*/

/**
 * @name    GDISP hardware accelerated support
 * @{
 */
	/**
	 * @brief   Hardware streaming interface is supported.
	 * @details If set to @p FALSE software emulation is used.
	 * @note	Either GDISP_HARDWARE_STREAM or GDISP_HARDWARE_DRAWPIXEL must be provided by the driver
	 */
	#ifndef GDISP_HARDWARE_STREAM
		#define GDISP_HARDWARE_STREAM			FALSE
	#endif

	/**
	 * @brief   Hardware streaming requires an explicit end call.
	 * @details If set to @p FALSE if an explicit stream end call is not required.
	 */
	#ifndef GDISP_HARDWARE_STREAM_END
		#define GDISP_HARDWARE_STREAM_END		FALSE
	#endif

	/**
	 * @brief   Hardware accelerated draw pixel.
	 * @details If set to @p FALSE software emulation is used.
	 * @note	Either GDISP_HARDWARE_STREAM or GDISP_HARDWARE_DRAWPIXEL must be provided by the driver
	 */
	#ifndef GDISP_HARDWARE_DRAWPIXEL
		#define GDISP_HARDWARE_DRAWPIXEL		FALSE
	#endif

	/**
	 * @brief   Hardware accelerated screen clears.
	 * @details If set to @p FALSE software emulation is used.
	 * @note	This clears the entire display surface regardless of the clipping area currently set
	 */
	#ifndef GDISP_HARDWARE_CLEARS
		#define GDISP_HARDWARE_CLEARS			FALSE
	#endif

	/**
	 * @brief   Hardware accelerated rectangular fills.
	 * @details If set to @p FALSE software emulation is used.
	 */
	#ifndef GDISP_HARDWARE_FILLS
		#define GDISP_HARDWARE_FILLS			FALSE
	#endif

	/**
	 * @brief   Hardware accelerated fills from an image.
	 * @details If set to @p FALSE software emulation is used.
	 */
	#ifndef GDISP_HARDWARE_BITFILLS
		#define GDISP_HARDWARE_BITFILLS			FALSE
	#endif

	/**
	 * @brief   Hardware accelerated scrolling.
	 * @details If set to @p FALSE there is no support for scrolling.
	 */
	#ifndef GDISP_HARDWARE_SCROLL
		#define GDISP_HARDWARE_SCROLL			FALSE
	#endif

	/**
	 * @brief   Reading back of pixel values.
	 * @details If set to @p FALSE there is no support for pixel read-back.
	 */
	#ifndef GDISP_HARDWARE_PIXELREAD
		#define GDISP_HARDWARE_PIXELREAD		FALSE
	#endif

	/**
	 * @brief   The driver supports one or more control commands.
	 * @details If set to @p FALSE there is no support for control commands.
	 */
	#ifndef GDISP_HARDWARE_CONTROL
		#define GDISP_HARDWARE_CONTROL			FALSE
	#endif

	/**
	 * @brief   The driver supports a non-standard query.
	 * @details If set to @p FALSE there is no support for non-standard queries.
	 */
	#ifndef GDISP_HARDWARE_QUERY
		#define GDISP_HARDWARE_QUERY			FALSE
	#endif

	/**
	 * @brief   The driver supports a clipping in hardware.
	 * @details If set to @p FALSE there is no support for non-standard queries.
	 * @note	If this is defined the driver must perform its own clipping on all calls to
	 * 			the driver and respond appropriately if a parameter is outside the display area.
	 * @note	If this is not defined then the software ensures that all calls to the
	 * 			driver do not exceed the display area (provided GDISP_NEED_CLIP or GDISP_NEED_VALIDATION
	 * 			has been set).
	 */
	#ifndef GDISP_HARDWARE_CLIP
		#define GDISP_HARDWARE_CLIP				FALSE
	#endif
/** @} */

/**
 * @name    GDISP software algorithm choices
 * @{
 */
/** @} */

/**
 * @name    GDISP pixel format choices
 * @{
 */
	/**
	 * @brief   The native pixel format for this device
	 * @note	Should be set to one of the following:
	 *				GDISP_PIXELFORMAT_RGB565
	 *				GDISP_PIXELFORMAT_BGR565
	 *				GDISP_PIXELFORMAT_RGB888
	 *				GDISP_PIXELFORMAT_RGB444
	 *				GDISP_PIXELFORMAT_RGB332
	 *				GDISP_PIXELFORMAT_RGB666
	 *				GDISP_PIXELFORMAT_CUSTOM
	 * @note	If you set GDISP_PIXELFORMAT_CUSTOM you need to also define
	 *				color_t, RGB2COLOR(r,g,b), HTML2COLOR(h),
	 *              RED_OF(c), GREEN_OF(c), BLUE_OF(c),
	 *              COLOR(c) and MASKCOLOR.
	 */
	#ifndef GDISP_PIXELFORMAT
		#define GDISP_PIXELFORMAT	GDISP_PIXELFORMAT_ERROR
	#endif

	/**
	 * @brief   Do pixels require packing for a blit
	 * @note	Is only valid for a pixel format that doesn't fill it's datatype. ie formats:
	 *				GDISP_PIXELFORMAT_RGB888
	 *				GDISP_PIXELFORMAT_RGB444
	 *				GDISP_PIXELFORMAT_RGB666
	 *				GDISP_PIXELFORMAT_CUSTOM
	 * @note	If you use GDISP_PIXELFORMAT_CUSTOM and packed bit fills
	 *				you need to also define @p gdispPackPixels(buf,cx,x,y,c)
	 * @note	If you are using GDISP_HARDWARE_BITFILLS = FALSE then the pixel
	 *				format must not be a packed format as the software blit does
	 *				not support packed pixels
	 * @note	Very few cases should actually require packed pixels as the low
	 *				level driver can also pack on the fly as it is sending it
	 *				to the graphics device.
	 */
	#ifndef GDISP_PACKED_PIXELS
		#define GDISP_PACKED_PIXELS			FALSE
	#endif

	/**
	 * @brief   Do lines of pixels require packing for a blit
	 * @note	Ignored if GDISP_PACKED_PIXELS is FALSE
	 */
	#ifndef GDISP_PACKED_LINES
		#define GDISP_PACKED_LINES			FALSE
	#endif
/** @} */

/*===========================================================================*/
/* External declarations.                                                    */
/*===========================================================================*/

typedef struct GDISPDriver {
	GDISPControl			g;

	uint16_t				flags;
		#define GDISP_FLG_INSTREAM		0x0001

	// Multithread Mutex
	#if GDISP_NEED_MULTITHREAD
		gfxMutex			mutex;
	#endif

	// Software clipping
	#if !GDISP_HARDWARE_CLIP && (GDISP_NEED_CLIP || GDISP_NEED_VALIDATION)
		coord_t				clipx0, clipy0;
		coord_t				clipx1, clipy1;		/* not inclusive */
	#endif

	// Driver call parameters
	struct {
		coord_t			x, y;
		coord_t			cx, cy;
		coord_t			x1, y1;
		coord_t			x2, y2;
		color_t			color;
		void			*ptr;
	} p;

	// Text rendering parameters
	#if GDISP_NEED_TEXT
		struct {
			font_t		font;
			color_t		color;
			color_t		bgcolor;
			coord_t		clipx0, clipy0;
			coord_t		clipx1, clipy1;
		} t;
	#endif
} GDISPDriver;

extern GDISPDriver	GDISP_DRIVER_STRUCT;

#ifdef __cplusplus
extern "C" {
#endif

	bool_t gdisp_lld_init(void);
	#if GDISP_HARDWARE_STREAM
		void gdisp_lld_stream_start(void);		// Uses p.x,p.y  p.cx,p.cy
		void gdisp_lld_stream_color(void);		// Uses p.color
		#if GDISP_HARDWARE_STREAM_END
			void gdisp_lld_stream_stop(void);	// Uses no parameters
		#endif
	#endif
	#if GDISP_HARDWARE_DRAWPIXEL
		void gdisp_lld_draw_pixel(void);		// Uses p.x,p.y  p.color
	#endif
	#if GDISP_HARDWARE_CLEARS
		void gdisp_lld_clear(void);				// Uses p.color
	#endif
	#if GDISP_HARDWARE_FILLS
		void gdisp_lld_fill_area(void);			// Uses p.x,p.y  p.cx,p.cy  p.color
	#endif
	#if GDISP_HARDWARE_BITFILLS
		void gdisp_lld_blit_area_ex(void);		// Uses p.x,p.y  p.cx,p.cy  p.x1,p.y1 (=srcx,srcy)  p.x2 (=srccx), p.ptr (=buffer)
	#endif
	#if GDISP_HARDWARE_PIXELREAD && GDISP_NEED_PIXELREAD
		color_t gdisp_lld_get_pixel_color(void);	// Uses p.x,p.y
	#endif
	#if GDISP_HARDWARE_SCROLL && GDISP_NEED_SCROLL
		void gdisp_lld_vertical_scroll(void);	// Uses p.x,p.y  p.cx,p.cy, p.y1 (=lines) p.color
	#endif
	#if GDISP_HARDWARE_CONTROL && GDISP_NEED_CONTROL
		void gdisp_lld_control(void);			// Uses p.x (=what)  p.ptr (=value)
	#endif
	#if GDISP_HARDWARE_QUERY && GDISP_NEED_QUERY
		void *gdisp_lld_query(void);			// Uses p.x (=what);
	#endif
	#if GDISP_HARDWARE_CLIP && (GDISP_NEED_CLIP || GDISP_NEED_VALIDATION)
		void gdisp_lld_set_clip(void);			// Uses p.x,p.y  p.cx,p.cy
	#endif

#ifdef __cplusplus
}
#endif

#endif	/* GFX_USE_GDISP */

#endif	/* _GDISP_LLD_H */
/** @} */

