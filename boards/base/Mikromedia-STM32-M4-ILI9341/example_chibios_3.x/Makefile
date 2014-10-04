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
	OPT_THUMB				= yes
	OPT_CPU					= stm32m4

# uGFX settings
	# See $(GFXLIB)/tools/gmake_scripts/library_ugfx.mk for the list of variables
	GFXLIB					= ../uGFX
	GFXBOARD				= Mikromedia-STM32-M4-ILI9341
	#GFXDRIVERS				= multiple/uGFXnet
	GFXDEMO					= modules/gdisp/basics

# ChibiOS settings
ifeq ($(OPT_OS),chibios)
	# See $(GFXLIB)/tools/gmake_scripts/os_chibios.mk for the list of variables
	CHIBIOS				= ../ChibiOS
	CHBIOS_VERSION		= 3
	CHIBIOS_BOARD		= 
	CHIBIOS_PLATFORM	= STM32/STM32F4xx
	CHIBIOS_PORT		= ARMCMx/compilers/GCC/mk/port_stm32f4xx
	CHIBIOS_DEFS		=
	CHIBIOS_LDSCRIPT	= STM32F407xG.ld
	# We define a non standard board script as this is not a standard ChibiOS supported board
	include $(GFXLIB)/boards/base/Mikromedia-STM32-M4-ILI9341/ChibiOS_Board/board.mk
endif

##############################################################################################
# Set these for your project
#

ARCH     = arm-none-eabi-
SRCFLAGS = -ggdb -O0
CFLAGS   =
CXXFLAGS = -fno-rtti
ASFLAGS  =
LDFLAGS  =

SRC      =
OBJS     =
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
##############################################################################
###############################################################################
# Architecture or project specific options
#

# Stack size to be allocated to the Cortex-M process stack. This stack is
# the stack used by the main() thread.
ifeq ($(USE_PROCESS_STACKSIZE),)
  USE_PROCESS_STACKSIZE = 0x400
endif

# Stack size to the allocated to the Cortex-M main/exceptions stack. This
# stack is used for processing interrupts and exceptions.
ifeq ($(USE_EXCEPTIONS_STACKSIZE),)
  USE_EXCEPTIONS_STACKSIZE = 0x400
endif

RULESPATH = $(CHIBIOS)/os/common/ports/ARMCMx/compilers/GCC
include $(RULESPATH)/rules.mk