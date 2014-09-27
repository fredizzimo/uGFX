GFXINC +=   $(GFXLIB)
GFXSRC +=	$(GFXLIB)/src/gfx.c

include $(GFXLIB)/src/gos/sys_make.mk
include $(GFXLIB)/src/gdriver/sys_make.mk
include $(GFXLIB)/src/gqueue/sys_make.mk
include $(GFXLIB)/src/gdisp/sys_make.mk
include $(GFXLIB)/src/gevent/sys_make.mk
include $(GFXLIB)/src/gtimer/sys_make.mk
include $(GFXLIB)/src/gwin/sys_make.mk
include $(GFXLIB)/src/ginput/sys_make.mk
include $(GFXLIB)/src/gadc/sys_make.mk
include $(GFXLIB)/src/gaudio/sys_make.mk
include $(GFXLIB)/src/gmisc/sys_make.mk
include $(GFXLIB)/src/gfile/sys_make.mk

# Include the boards and drivers
ifneq ($(GFXBOARD),)
	include $(GFXLIB)/boards/base/$(GFXBOARD)/board.mk
endif
ifneq ($(GFXDRIVERS),)
	include $(patsubst %,$(GFXLIB)/drivers/%/driver.mk,$(GFXDRIVERS))
endif
ifneq ($(GFXDEMO),)
	include $(GFXLIB)/demos/$(GFXDEMO)/demo.mk
endif

# Include the operating system define
ifeq ($(OPT_OS),win32)
	GFXDEFS += GFX_USE_OS_WIN32=TRUE
else ifeq ($(OPT_OS),linux)
	GFXDEFS += GFX_USE_OS_LINUX=TRUE
else ifeq ($(OPT_OS),osx)
	GFXDEFS += GFX_USE_OS_OSX=TRUE
else ifeq ($(OPT_OS),chibios)
	GFXDEFS += GFX_USE_OS_CHIBIOS=TRUE
else ifeq ($(OPT_OS),freertos)
	GFXDEFS += GFX_USE_OS_FREERTOS=TRUE
else ifeq ($(OPT_OS),ecos)
	GFXDEFS += GFX_USE_OS_ECOS=TRUE
else ifeq ($(OPT_OS),rawrtos)
	GFXDEFS += GFX_USE_OS_RAWRTOS=TRUE
else ifeq ($(OPT_OS),raw32)
	GFXDEFS += GFX_USE_OS_RAW32=TRUE
endif
