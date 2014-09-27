# Possible Targets:	all clean Debug cleanDebug Release cleanRelease

##############################################################################################
# Settings
#

# See $(GFXLIB)/tools/gmake_scripts/readme.txt for the meaning of these variables
OPT_VERBOSE_COMPILE		= no
OPT_GENERATE_LISTINGS	= yes
OPT_GENERATE_MAP		= yes
OPT_COPY_EXE			= no
OPT_NATIVEOS			= win32
OPT_OS					= win32

# See $(GFXLIB)/tools/gmake_scripts/library_ugfx.mk for the meaning of these variables
GFXLIB = ../uGFX
GFXBOARD = Win32
#GFXDRIVERS = multiple/uGFXnet
GFXDEMO = modules/gdisp/basics

# Win32 - ChibiOS simulator
ifeq ($(OPT_OS),chibios)
	# Required: Location of the ChibiOS code
	CHIBIOS = ../ChibiOS
endif


##############################################################################################
# Set these for your project
#

ARCH     = i686-pc-mingw32-
SRCFLAGS = -ggdb -O0 -fomit-frame-pointer -Wall -Wextra -Wstrict-prototypes -fverbose-asm
CFLAGS   =
CXXFLAGS =
ASFLAGS  =
LDFLAGS  =

SRC      =
DEFS     =
LIBS     =
INCPATH  =
LIBPATH  =

##############################################################################################
# Optional: Win32 - ChibiOS Simulator
#

ifeq ($(OPT_OS),chibios)
	include $(CHIBIOS)/boards/simulator/board.mk
	include $(CHIBIOS)/os/hal/hal.mk
	include $(CHIBIOS)/os/hal/platforms/Win32/platform.mk
	include $(CHIBIOS)/os/ports/GCC/SIMIA32/port.mk
	include $(CHIBIOS)/os/kernel/kernel.mk
	DEFS += SIMULATOR SHELL_USE_IPRINTF=FALSE
	INCPATH += $(PORTINC) $(KERNINC) $(TESTINC) \
	          $(HALINC) $(PLATFORMINC) $(BOARDINC)
	#          $(CHIBIOS)/os/various
	SRC  += $(PORTSRC) \
	        $(KERNSRC) \
	        $(TESTSRC) \
	        $(HALSRC) \
	        $(PLATFORMSRC) \
	        $(BOARDSRC)
endif

##############################################################################################
# These should be at the end
#

include $(GFXLIB)/tools/gmake_scripts/library_ugfx.mk
include $(GFXLIB)/tools/gmake_scripts/compiler_gcc.mk
# *** EOF ***
