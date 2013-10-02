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

#if GDISP_MULTIPLE_DRIVERS && defined(GDISP_LLD_DECLARATIONS)
	// include hardware definitions
	#include "gdisp_lld_config.h"
#endif

/*===========================================================================*/
/* Error checks.                                                             */
/*===========================================================================*/

#if !GDISP_MULTIPLE_DRIVERS || defined(GDISP_LLD_DECLARATIONS)
	/**
	 * @name    GDISP hardware accelerated support
	 * @{
	 */
		/**
		 * @brief   Hardware streaming writing is supported.
		 * @details If set to @p FALSE software emulation is used.
		 * @note	Either GDISP_HARDWARE_STREAM_WRITE or GDISP_HARDWARE_DRAWPIXEL must be provided by the driver
		 */
		#ifndef GDISP_HARDWARE_STREAM_WRITE
			#define GDISP_HARDWARE_STREAM_WRITE		FALSE
		#endif

		/**
		 * @brief   Hardware streaming reading of the display surface is supported.
		 * @details If set to @p FALSE this routine is not available.
		 */
		#ifndef GDISP_HARDWARE_STREAM_READ
			#define GDISP_HARDWARE_STREAM_READ		FALSE
		#endif

		/**
		 * @brief   Hardware supports setting the cursor position within the stream window.
		 * @details If set to @p FALSE this routine is not available.
		 * @note	This is used to optimise setting of individual pixels within a stream window.
		 * 			It should therefore not be implemented unless it is cheaper than just setting
		 * 			a new window.
		 */
		#ifndef GDISP_HARDWARE_STREAM_POS
			#define GDISP_HARDWARE_STREAM_POS		FALSE
		#endif

		/**
		 * @brief   Hardware accelerated draw pixel.
		 * @details If set to @p FALSE software emulation is used.
		 * @note	Either GDISP_HARDWARE_STREAM_WRITE or GDISP_HARDWARE_DRAWPIXEL must be provided by the driver
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
#endif

/*===========================================================================*/
/* External declarations.                                                    */
/*===========================================================================*/

typedef struct GDISPDriver {
	GDISPControl				g;

	#if GDISP_MULTIPLE_DRIVERS
		const struct GDISPVMT const *	vmt;
	#endif

	uint16_t					flags;
		#define GDISP_FLG_INSTREAM		0x0001		// We are in a user based stream operation
		#define GDISP_FLG_SCRSTREAM		0x0002		// The stream area currently covers the whole screen
		#define GDISP_FLG_DRIVER		0x0004		// This flags and above are for use by the driver

	// Multithread Mutex
	#if GDISP_NEED_MULTITHREAD
		gfxMutex				mutex;
	#endif

	// Software clipping
	#if (GDISP_MULTIPLE_DRIVERS || !GDISP_HARDWARE_CLIP) && (GDISP_NEED_CLIP || GDISP_NEED_VALIDATION)
		coord_t					clipx0, clipy0;
		coord_t					clipx1, clipy1;		/* not inclusive */
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

	// In call working buffers

	#if GDISP_NEED_TEXT
		// Text rendering parameters
		struct {
			font_t		font;
			color_t		color;
			color_t		bgcolor;
			coord_t		clipx0, clipy0;
			coord_t		clipx1, clipy1;
		} t;
	#endif
	#if GDISP_LINEBUF_SIZE != 0 && ((GDISP_NEED_SCROLL && !GDISP_HARDWARE_SCROLL) || (!GDISP_HARDWARE_STREAM_WRITE && GDISP_HARDWARE_BITFILLS))
		// A pixel line buffer
		color_t		linebuf[GDISP_LINEBUF_SIZE];
	#endif

} GDISPDriver;

#if !GDISP_MULTIPLE_DRIVERS || defined(GDISP_LLD_DECLARATIONS) || defined(__DOXYGEN__)
	#if GDISP_MULTIPLE_DRIVERS
		#define LLDSPEC			static
	#else
		#define LLDSPEC
	#endif

	#ifdef __cplusplus
	extern "C" {
	#endif

	/**
	 * @brief   Initialize the driver.
	 * @return	TRUE if successful.
	 * @param[in]	g		The driver structure
	 * @param[out]	g->g	The driver must fill in the GDISPControl structure
	 */
	LLDSPEC	bool_t gdisp_lld_init(GDISPDriver *g);

	#if GDISP_HARDWARE_STREAM_WRITE || defined(__DOXYGEN__)
		/**
		 * @brief   Start a streamed write operation
		 * @pre		GDISP_HARDWARE_STREAM_WRITE is TRUE
		 *
		 * @param[in]	g				The driver structure
		 * @param[in]	g->p.x,g->p.y	The window position
		 * @param[in]	g->p.cx,g->p.cy	The window size
		 *
		 * @note		The parameter variables must not be altered by the driver.
		 * @note		Streaming operations that wrap the defined window have
		 * 				undefined results.
		 * @note		This must be followed by a call to @p gdisp_lld_write_pos() if GDISP_HARDWARE_STREAM_POS is TRUE.
		 */
		LLDSPEC	void gdisp_lld_write_start(GDISPDriver *g);

		/**
		 * @brief   Send a pixel to the current streaming position and then increment that position
		 * @pre		GDISP_HARDWARE_STREAM_WRITE is TRUE
		 *
		 * @param[in]	g				The driver structure
		 * @param[in]	g->p.color		The color to display at the curent position
		 *
		 * @note		The parameter variables must not be altered by the driver.
		 */
		LLDSPEC	void gdisp_lld_write_color(GDISPDriver *g);

		/**
		 * @brief   End the current streaming write operation
		 * @pre		GDISP_HARDWARE_STREAM_WRITE is TRUE
		 *
		 * @param[in]	g				The driver structure
		 *
		 * @note		The parameter variables must not be altered by the driver.
		 */
		LLDSPEC	void gdisp_lld_write_stop(GDISPDriver *g);

		#if GDISP_HARDWARE_STREAM_POS || defined(__DOXYGEN__)
			/**
			 * @brief   Change the current position within the current streaming window
			 * @pre		GDISP_HARDWARE_STREAM_POS is TRUE and GDISP_HARDWARE_STREAM_WRITE is TRUE
			 *
			 * @param[in]	g				The driver structure
			 * @param[in]	g->p.x,g->p.y	The new position (which will always be within the existing stream window)
			 *
			 * @note		The parameter variables must not be altered by the driver.
			 */
			LLDSPEC	void gdisp_lld_write_pos(GDISPDriver *g);
		#endif
	#endif

	#if GDISP_HARDWARE_STREAM_READ || defined(__DOXYGEN__)
		/**
		 * @brief   Start a streamed read operation
		 * @pre		GDISP_HARDWARE_STREAM_READ is TRUE
		 *
		 * @param[in]	g				The driver structure
		 * @param[in]	g->p.x,g->p.y	The window position
		 * @param[in]	g->p.cx,g->p.cy	The window size
		 *
		 * @note		The parameter variables must not be altered by the driver.
		 * @note		Streaming operations that wrap the defined window have
		 * 				undefined results.
		 */
		LLDSPEC	void gdisp_lld_read_start(GDISPDriver *g);

		/**
		 * @brief   Read a pixel from the current streaming position and then increment that position
		 * @return	The color at the current position
		 * @pre		GDISP_HARDWARE_STREAM_READ is TRUE
		 *
		 * @param[in]	g				The driver structure
		 *
		 * @note		The parameter variables must not be altered by the driver.
		 */
		LLDSPEC	color_t gdisp_lld_read_color(GDISPDriver *g);

		/**
		 * @brief   End the current streaming operation
		 * @pre		GDISP_HARDWARE_STREAM_READ is TRUE
		 *
		 * @param[in]	g				The driver structure
		 *
		 * @note		The parameter variables must not be altered by the driver.
		 */
		LLDSPEC	void gdisp_lld_read_stop(GDISPDriver *g);
	#endif

	#if GDISP_HARDWARE_DRAWPIXEL || defined(__DOXYGEN__)
		/**
		 * @brief   Draw a pixel
		 * @pre		GDISP_HARDWARE_DRAWPIXEL is TRUE
		 *
		 * @param[in]	g				The driver structure
		 * @param[in]	g->p.x,g->p.y	The pixel position
		 * @param[in]	g->p.color		The color to set
		 *
		 * @note		The parameter variables must not be altered by the driver.
		 */
		LLDSPEC	void gdisp_lld_draw_pixel(GDISPDriver *g);
	#endif

	#if GDISP_HARDWARE_CLEARS || defined(__DOXYGEN__)
		/**
		 * @brief   Clear the screen using the defined color
		 * @pre		GDISP_HARDWARE_CLEARS is TRUE
		 *
		 * @param[in]	g				The driver structure
		 * @param[in]	g->p.color		The color to set
		 *
		 * @note		The parameter variables must not be altered by the driver.
		 */
		LLDSPEC	void gdisp_lld_clear(GDISPDriver *g);
	#endif

	#if GDISP_HARDWARE_FILLS || defined(__DOXYGEN__)
		/**
		 * @brief   Fill an area with a single color
		 * @pre		GDISP_HARDWARE_FILLS is TRUE
		 *
		 * @param[in]	g				The driver structure
		 * @param[in]	g->p.x,g->p.y	The area position
		 * @param[in]	g->p.cx,g->p.cy	The area size
		 * @param[in]	g->p.color		The color to set
		 *
		 * @note		The parameter variables must not be altered by the driver.
		 */
		LLDSPEC	void gdisp_lld_fill_area(GDISPDriver *g);
	#endif

	#if GDISP_HARDWARE_BITFILLS || defined(__DOXYGEN__)
		/**
		 * @brief   Fill an area using a bitmap
		 * @pre		GDISP_HARDWARE_BITFILLS is TRUE
		 *
		 * @param[in]	g				The driver structure
		 * @param[in]	g->p.x,g->p.y	The area position
		 * @param[in]	g->p.cx,g->p.cy	The area size
		 * @param[in]	g->p.x1,g->p.y1	The starting position in the bitmap
		 * @param[in]	g->p.x2			The width of a bitmap line
		 * @param[in]	g->p.ptr		The pointer to the bitmap
		 *
		 * @note		The parameter variables must not be altered by the driver.
		 */
		LLDSPEC	void gdisp_lld_blit_area(GDISPDriver *g);
	#endif

	#if GDISP_HARDWARE_PIXELREAD || defined(__DOXYGEN__)
		/**
		 * @brief   Read a pixel from the display
		 * @return	The color at the defined position
		 * @pre		GDISP_HARDWARE_PIXELREAD is TRUE (and the application needs it)
		 *
		 * @param[in]	g				The driver structure
		 * @param[in]	g->p.x,g->p.y	The pixel position
		 *
		 * @note		The parameter variables must not be altered by the driver.
		 */
		LLDSPEC	color_t gdisp_lld_get_pixel_color(GDISPDriver *g);
	#endif

	#if (GDISP_HARDWARE_SCROLL && GDISP_NEED_SCROLL) || defined(__DOXYGEN__)
		/**
		 * @brief   Scroll an area of the screen
		 * @pre		GDISP_HARDWARE_SCROLL is TRUE (and the application needs it)
		 *
		 * @param[in]	g				The driver structure
		 * @param[in]	g->p.x,g->p.y	The area position
		 * @param[in]	g->p.cx,g->p.cy	The area size
		 * @param[in]	g->p.y1			The number of lines to scroll (positive or negative)
		 *
		 * @note		The parameter variables must not be altered by the driver.
		 * @note		This can be easily implemented if the hardware supports
		 * 				display area to display area copying.
		 * @note		Clearing the exposed area on the scroll operation is not
		 * 				needed as the high level code handles this.
		 */
		LLDSPEC	void gdisp_lld_vertical_scroll(GDISPDriver *g);
	#endif

	#if (GDISP_HARDWARE_CONTROL && GDISP_NEED_CONTROL) || defined(__DOXYGEN__)
		/**
		 * @brief   Control some feature of the hardware
		 * @pre		GDISP_HARDWARE_CONTROL is TRUE (and the application needs it)
		 *
		 * @param[in]	g				The driver structure
		 * @param[in]	g->p.x			The operation to perform
		 * @param[in]	g->p.ptr		The operation parameter
		 *
		 * @note		The parameter variables must not be altered by the driver.
		 */
		LLDSPEC	void gdisp_lld_control(GDISPDriver *g);
	#endif

	#if (GDISP_HARDWARE_QUERY && GDISP_NEED_QUERY) || defined(__DOXYGEN__)
		/**
		 * @brief   Query some feature of the hardware
		 * @return	The information requested (typecast as void *)
		 * @pre		GDISP_HARDWARE_QUERY is TRUE (and the application needs it)
		 *
		 * @param[in]	g				The driver structure
		 * @param[in]	g->p.x			What to query
		 *
		 * @note		The parameter variables must not be altered by the driver.
		 */
		LLDSPEC	void *gdisp_lld_query(GDISPDriver *g);				// Uses p.x (=what);
	#endif

	#if (GDISP_HARDWARE_CLIP && (GDISP_NEED_CLIP || GDISP_NEED_VALIDATION)) || defined(__DOXYGEN__)
		/**
		 * @brief   Set the hardware clipping area
		 * @pre		GDISP_HARDWARE_CLIP is TRUE (and the application needs it)
		 *
		 * @param[in]	g				The driver structure
		 * @param[in]	g->p.x,g->p.y	The area position
		 * @param[in]	g->p.cx,g->p.cy	The area size
		 *
		 * @note		The parameter variables must not be altered by the driver.
		 */
		LLDSPEC	void gdisp_lld_set_clip(GDISPDriver *g);
	#endif

	#ifdef __cplusplus
	}
	#endif
#endif  // !GDISP_MULTIPLE_DRIVERS || defined(GDISP_LLD_DECLARATIONS)


#if GDISP_MULTIPLE_DRIVERS

	typedef struct GDISPVMT {
		bool_t (*init)(GDISPDriver *g);
		void (*writestart)(GDISPDriver *g);				// Uses p.x,p.y  p.cx,p.cy
		void (*writepos)(GDISPDriver *g);				// Uses p.x,p.y
		void (*writecolor)(GDISPDriver *g);				// Uses p.color
		void (*writestop)(GDISPDriver *g);				// Uses no parameters
		void (*readstart)(GDISPDriver *g);				// Uses p.x,p.y  p.cx,p.cy
		color_t (*readcolor)(GDISPDriver *g);			// Uses no parameters
		void (*readstop)(GDISPDriver *g);				// Uses no parameters
		void (*pixel)(GDISPDriver *g);					// Uses p.x,p.y  p.color
		void (*clear)(GDISPDriver *g);					// Uses p.color
		void (*fill)(GDISPDriver *g);					// Uses p.x,p.y  p.cx,p.cy  p.color
		void (*blit)(GDISPDriver *g);					// Uses p.x,p.y  p.cx,p.cy  p.x1,p.y1 (=srcx,srcy)  p.x2 (=srccx), p.ptr (=buffer)
		color_t (*get)(GDISPDriver *g);					// Uses p.x,p.y
		void (*vscroll)(GDISPDriver *g);				// Uses p.x,p.y  p.cx,p.cy, p.y1 (=lines) p.color
		void (*control)(GDISPDriver *g);				// Uses p.x (=what)  p.ptr (=value)
		void *(*query)(GDISPDriver *g);					// Uses p.x (=what);
		void (*setclip)(GDISPDriver *g);				// Uses p.x,p.y  p.cx,p.cy
	} GDISPVMT;

	#ifdef GDISP_LLD_DECLARATIONS
		#define GDISP_DRIVER_STRUCT_INIT	{{0}, &VMT}
		static const GDISPVMT VMT = {
			gdisp_lld_init,
			#if GDISP_HARDWARE_STREAM_WRITE
				gdisp_lld_write_start,
				#if GDISP_HARDWARE_STREAM_POS
					gdisp_lld_write_pos,
				#else
					0,
				#endif
				gdisp_lld_write_color,
				gdisp_lld_write_stop,
			#else
				0, 0, 0,
			#endif
			#if GDISP_HARDWARE_STREAM_READ
				gdisp_lld_read_start,
				gdisp_lld_read_color,
				gdisp_lld_read_stop,
			#else
				0, 0, 0,
			#endif
			#if GDISP_HARDWARE_DRAWPIXEL
				gdisp_lld_draw_pixel,
			#else
				0,
			#endif
			#if GDISP_HARDWARE_CLEARS
				gdisp_lld_clear,
			#else
				0,
			#endif
			#if GDISP_HARDWARE_FILLS
				gdisp_lld_fill_area,
			#else
				0,
			#endif
			#if GDISP_HARDWARE_BITFILLS
				gdisp_lld_blit_area,
			#else
				0,
			#endif
			#if GDISP_HARDWARE_PIXELREAD
				gdisp_lld_get_pixel_color,
			#else
				0,
			#endif
			#if GDISP_HARDWARE_SCROLL && GDISP_NEED_SCROLL
				gdisp_lld_vertical_scroll,
			#else
				0,
			#endif
			#if GDISP_HARDWARE_CONTROL && GDISP_NEED_CONTROL
				gdisp_lld_control,
			#else
				0,
			#endif
			#if GDISP_HARDWARE_QUERY && GDISP_NEED_QUERY
				gdisp_lld_query,
			#else
				0,
			#endif
			#if GDISP_HARDWARE_CLIP && (GDISP_NEED_CLIP || GDISP_NEED_VALIDATION)
				gdisp_lld_set_clip,
			#else
				0,
			#endif
		};
		GDISPDriver	GDISP_DRIVER_STRUCT = {{0}, &VMT};

	#else
		#define gdisp_lld_init(g)				g->vmt->init(g)
		#define gdisp_lld_write_start(g)		g->vmt->writestart(g)
		#define gdisp_lld_write_pos(g)			g->vmt->writepos(g)
		#define gdisp_lld_write_color(g)		g->vmt->writecolor(g)
		#define gdisp_lld_write_stop(g)			g->vmt->writestop(g)
		#define gdisp_lld_read_start(g)			g->vmt->readstart(g)
		#define gdisp_lld_read_color(g)			g->vmt->readcolor(g)
		#define gdisp_lld_read_stop(g)			g->vmt->readstop(g)
		#define gdisp_lld_draw_pixel(g)			g->vmt->pixel(g)
		#define gdisp_lld_clear(g)				g->vmt->clear(g)
		#define gdisp_lld_fill_area(g)			g->vmt->fill(g)
		#define gdisp_lld_blit_area(g)			g->vmt->blit(g)
		#define gdisp_lld_get_pixel_color(g)	g->vmt->get(g)
		#define gdisp_lld_vertical_scroll(g)	g->vmt->vscroll(g)
		#define gdisp_lld_control(g)			g->vmt->control(g)
		#define gdisp_lld_query(g)				g->vmt->query(g)
		#define gdisp_lld_set_clip(g)			g->vmt->setclip(g)

		extern	GDISPDriver	GDISP_DRIVER_STRUCT;

	#endif	// GDISP_LLD_DECLARATIONS

#else		// GDISP_MULTIPLE_DRIVERS
	#ifdef GDISP_LLD_DECLARATIONS
		GDISPDriver	GDISP_DRIVER_STRUCT;
	#else
		extern	GDISPDriver	GDISP_DRIVER_STRUCT;
	#endif

#endif		// GDISP_MULTIPLE_DRIVERS

		/* Verify information for packed pixels and define a non-packed pixel macro */
		#if !GDISP_PACKED_PIXELS
			#define gdispPackPixels(buf,cx,x,y,c)	{ ((color_t *)(buf))[(y)*(cx)+(x)] = (c); }
		#elif !GDISP_HARDWARE_BITFILLS
			#error "GDISP: packed pixel formats are only supported for hardware accelerated drivers."
		#elif GDISP_PIXELFORMAT != GDISP_PIXELFORMAT_RGB888 \
				&& GDISP_PIXELFORMAT != GDISP_PIXELFORMAT_RGB444 \
				&& GDISP_PIXELFORMAT != GDISP_PIXELFORMAT_RGB666 \
				&& GDISP_PIXELFORMAT != GDISP_PIXELFORMAT_CUSTOM
			#error "GDISP: A packed pixel format has been specified for an unsupported pixel format."
		#endif

		/* Support routine for packed pixel formats */
		#if !defined(gdispPackPixels) || defined(__DOXYGEN__)
			/**
			 * @brief   Pack a pixel into a pixel buffer.
			 * @note    This function performs no buffer boundary checking
			 *			regardless of whether GDISP_NEED_CLIP has been specified.
			 *
			 * @param[in] buf		The buffer to put the pixel in
			 * @param[in] cx		The width of a pixel line
			 * @param[in] x, y		The location of the pixel to place
			 * @param[in] color		The color to put into the buffer
			 *
			 * @api
			 */
			void gdispPackPixels(const pixel_t *buf, coord_t cx, coord_t x, coord_t y, color_t color);
		#endif

#endif	/* GFX_USE_GDISP */

#endif	/* _GDISP_LLD_H */
/** @} */

