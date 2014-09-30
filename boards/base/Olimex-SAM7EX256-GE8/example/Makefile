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
	OPT_NATIVEOS			= chibios
	OPT_OS					= chibios

# uGFX settings
	# See $(GFXLIB)/tools/gmake_scripts/library_ugfx.mk for the list of variables
	GFXLIB					= ../uGFX
	GFXBOARD				= Olimex-SAM7EX256-GE8
	#GFXDRIVERS				= multiple/uGFXnet
	GFXDEMO					= modules/gdisp/basics

# ChibiOS settings
ifeq ($(OPT_OS),chibios)
	# See $(GFXLIB)/tools/gmake_scripts/os_chibios.mk for the list of variables
	CHIBIOS				= ../ChibiOS
	CHIBIOS_BOARD		= OLIMEX_SAM7_EX256
	CHIBIOS_PLATFORM	= AT91SAM7
	CHIBIOS_PORT		= GCC/ARM/AT91SAM7
	CHIBIOS_DEFS		=
	#CHIBIOS_LDSCRIPT	= $(PORTLD)/AT91SAM7X256.ld
	# We define a non standard linker script here just to give us some more stack space
	CHIBIOS_LDSCRIPT	= linker.ld
endif

##############################################################################################
# Set these for your project
#

ARCH     = arm-none-eabi-
SRCFLAGS = -ggdb -O0
SRCFLAGS+= -mcpu=arm7tdmi -mabi=apcs-gnu -mno-thumb-interwork
CFLAGS   =
CXXFLAGS = -fno-rtti
ASFLAGS  =
LDFLAGS  = -mcpu=arm7tdmi

SRC      =
DEFS     =
LIBS     =
INCPATH  =
LIBPATH  =
LDSCRIPT =

##############################################################################################
# These should be at the end
#

include $(GFXLIB)/tools/gmake_scripts/library_ugfx.mk
include $(GFXLIB)/tools/gmake_scripts/os_$(OPT_OS).mk
include $(GFXLIB)/tools/gmake_scripts/compiler_gcc.mk
# *** EOF ***
