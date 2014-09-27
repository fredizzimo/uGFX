# Possible Targets:	all clean Debug cleanDebug Release cleanRelease

##############################################################################################
# Settings
#

# See $(GFXLIB)/tools/gmake_scripts/readme.txt for the meaning of these variables
OPT_VERBOSE_COMPILE		= no
OPT_GENERATE_LISTINGS	= yes
OPT_GENERATE_MAP		= yes
OPT_COPY_EXE			= no
OPT_NATIVEOS			= linux
OPT_OS					= linux

# See $(GFXLIB)/tools/gmake_scripts/library_ugfx.mk for the meaning of these variables
GFXLIB = ../uGFX
GFXBOARD = Linux
#GFXDRIVERS = multiple/uGFXnet
GFXDEMO = modules/gdisp/basics

##############################################################################################
# Set these for your project
#

ARCH     =
SRCFLAGS = -m32 -ggdb -O0 -fomit-frame-pointer -Wall -Wextra -Wstrict-prototypes -fverbose-asm
CFLAGS   =
CXXFLAGS =
ASFLAGS  =
LDFLAGS  = -pthread

SRC      =
DEFS     =
LIBS     =
INCPATH  =
LIBPATH  =

##############################################################################################
# These should be at the end
#

include $(GFXLIB)/tools/gmake_scripts/library_ugfx.mk
include $(GFXLIB)/tools/gmake_scripts/compiler_gcc.mk
# *** EOF ***
