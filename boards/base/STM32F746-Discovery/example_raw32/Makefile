# Possible Targets:	all clean Debug cleanDebug Release cleanRelease

##############################################################################################
# Settings
#

# General settings
	# See $(GFXLIB)/tools/gmake_scripts/readme.txt for the list of variables
	OPT_OS					= raw32
	OPT_THUMB				= yes
	OPT_LINK_OPTIMIZE		= no
	OPT_CPU					= stm32m7

# uGFX settings
	# See $(GFXLIB)/tools/gmake_scripts/library_ugfx.mk for the list of variables
	GFXLIB					= ../ugfx
	GFXBOARD				= STM32F746-Discovery
	GFXDEMO					= modules/gdisp/basics
	#GFXDRIVERS				=

# ChibiOS settings
# Note: not supported by ChibiOS yet!
ifeq ($(OPT_OS),chibios)
	# See $(GFXLIB)/tools/gmake_scripts/os_chibios.mk for the list of variables
	CHIBIOS				= ../ChibiOS
	CHIBIOS_BOARD		= ST_STM32F746_DISCOVERY
	CHIBIOS_PLATFORM	= STM32F7xx
	CHIBIOS_PORT		= GCC/ARMCMx/STM32F7xx
	CHIBIOS_LDSCRIPT	= STM32F746.ld

	#CHIBIOS			= ../ChibiOS3
	#CHIBIOS_VERSION	= 3
	#CHIBIOS_BOARD		= ST_STM32F746_DISCOVERY
	#CHIBIOS_CPUCLASS   = ARMCMx
	#CHIBIOS_PLATFORM	= STM32/STM32F7xx
	#CHIBIOS_PORT		= stm32f7xx
	#CHIBIOS_LDSCRIPT	= STM32F746.ld
endif

# Raw32 settings
CMSIS			= ../STM32/CMSIS
HAL				= ../STM32/STM32F7xx_HAL_Driver

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
DEFS     =	GOS_RAW_HEAP_SIZE=40960
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
