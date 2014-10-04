# Possible Targets:	all clean Debug cleanDebug Release cleanRelease

##############################################################################################
# Settings
#

# General settings
	# See $(GFXLIB)/tools/gmake_scripts/readme.txt for the list of variables
	OPT_VERBOSE_COMPILE		= no
	OPT_GENERATE_LISTINGS	= yes
	OPT_GENERATE_MAP		= yes
	OPT_COPY_EXE			= no
	OPT_LINK_OPTIMIZE		= yes
	OPT_NONSTANDARD_FLAGS	= no
	# For Win32 this variable can be set to "win32" (native win32 api) or "win32.chibios" (ChibiOS simulator). 
	OPT_OS					= win32

# uGFX settings
	# See $(GFXLIB)/tools/gmake_scripts/library_ugfx.mk for the list of variables
	GFXLIB					= ../uGFX
	GFXBOARD				= Win32
	#GFXDRIVERS				= multiple/uGFXnet
	GFXDEMO					= modules/gdisp/basics

# ChibiOS settings
ifeq ($(OPT_OS),win32.chibios)
	# See $(GFXLIB)/tools/gmake_scripts/os_win32.chibios.mk for the list of variables
	CHIBIOS					= ../ChibiOS
endif

# Win32 settings
	# See $(GFXLIB)/tools/gmake_scripts/os_win32.mk for the list of variables

##############################################################################################
# Set these for your project
#

ARCH     = i686-pc-mingw32-
SRCFLAGS = -ggdb -O0
CFLAGS   =
CXXFLAGS =
ASFLAGS  =
LDFLAGS  =

SRC      =
OBJS     =
DEFS     =
LIBS     =
INCPATH  =
LIBPATH  =

##############################################################################################
# These should be at the end
#

include $(GFXLIB)/tools/gmake_scripts/library_ugfx.mk
include $(GFXLIB)/tools/gmake_scripts/os_$(OPT_OS).mk
include $(GFXLIB)/tools/gmake_scripts/compiler_gcc.mk
# *** EOF ***
