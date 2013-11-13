To use this driver:

This driver is special in that it implements both the gdisp low level driver
and a touchscreen driver.

1. Add in your gfxconf.h:
	a) #define GFX_USE_GDISP			TRUE
	b) Optionally #define GFX_USE_GINPUT			TRUE
					#define GINPUT_USE_MOUSE		TRUE
	c) Any optional high level driver defines (see gdisp.h) eg: GDISP_NEED_MULTITHREAD
	d) Optionally the following (with appropriate values):
		#define GDISP_SCREEN_WIDTH	640
		#define GDISP_SCREEN_HEIGHT	480

2. To your makefile add the following lines:
	include $(GFXLIB)/gfx.mk
	include $(GFXLIB)/drivers/multiple/uGFXnet/gdisp_lld.mk

3. Make sure you have networking libraries included.
