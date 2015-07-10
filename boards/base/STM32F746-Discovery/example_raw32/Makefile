# Possible Targets:	all clean Debug cleanDebug Release cleanRelease

##############################################################################################
# Settings
#

# General settings
	# See $(GFXLIB)/tools/gmake_scripts/readme.txt for the list of variables
	OPT_OS					= raw32
	OPT_THUMB				= yes
	OPT_LINK_OPTIMIZE		= yes
	OPT_CPU					= stm32m7

# uGFX settings
	# See $(GFXLIB)/tools/gmake_scripts/library_ugfx.mk for the list of variables
	GFXLIB					= ../uGFX
	GFXBOARD				= STM32F746-Discovery
	GFXDEMO					= modules/gwin/widgets
	#GFXDRIVERS				= 

# ChibiOS settings
ifeq ($(OPT_OS),chibios)
	# See $(GFXLIB)/tools/gmake_scripts/os_chibios.mk for the list of variables
	CHIBIOS				= ../ChibiOS
	CHIBIOS_BOARD		= ST_STM32F429I_DISCOVERY
	CHIBIOS_PLATFORM	= STM32F4xx
	CHIBIOS_PORT		= GCC/ARMCMx/STM32F4xx
	CHIBIOS_LDSCRIPT	= STM32F407xG.ld

	#CHIBIOS				= ../ChibiOS3
	#CHIBIOS_VERSION		= 3
	#CHIBIOS_BOARD		= ST_STM32F429I_DISCOVERY
	#CHIBIOS_CPUCLASS    = ARMCMx
	#CHIBIOS_PLATFORM	= STM32/STM32F4xx
	#CHIBIOS_PORT		= stm32f4xx
	#CHIBIOS_LDSCRIPT	= STM32F407xG.ld
endif

CMSIS			= CMSIS
HAL				= STM32F7xx_HAL_Driver
CONTROLLER 		= STM32F746xx

##############################################################################################
# Set these for your project
#

ARCH     = arm-none-eabi-
SRCFLAGS = -ggdb -O0
CFLAGS   =
CXXFLAGS = -fno-rtti
ASFLAGS  =
LDFLAGS  =

#SRC      = 			$(HAL)/Src/stm32f7xx_hal.c \
					$(HAL)/Src/stm32f7xx_hal_cortex.c \
					$(HAL)/Src/stm32f7xx_hal_flash.c \
					$(HAL)/Src/stm32f7xx_hal_flash_ex.c \
					$(HAL)/Src/stm32f7xx_hal_rcc.c \
					$(HAL)/Src/stm32f7xx_hal_rcc_ex.h \
					$(HAL)/Src/stm32f7xx_hal_gpio.c \
					$(HAL)/Src/stm32f7xx_hal_pwr.c \
					$(HAL)/Src/stm32f7xx_hal_pwr_ex.c \
					$(HAL)/Src/stm32f7xx_hal_ltdc.c \
					STM32746G-Discovery/stm32746g_discovery.c \
					stm32f7xx_hal_msp.c \
					stm32f7xx_it.c \
					system_stm32f7xx.c \
					ugfx_raw32_implementations.c \
					main.c \
					startup_stm32f746xx.s

OBJS     =
DEFS     =
LIBS     =
INCPATH  = 			$(CMSIS)/Device/ST/STM32F7xx/Include \
					$(CMSIS)/Include \
					$(HAL)/Inc \
					STM32746G-Discovery

LIBPATH  =
LDSCRIPT = stm32f746nghx_flash.ld

##############################################################################################
# These should be at the end
#

include $(GFXLIB)/tools/gmake_scripts/library_ugfx.mk
include $(GFXLIB)/tools/gmake_scripts/os_$(OPT_OS).mk
include $(GFXLIB)/tools/gmake_scripts/compiler_gcc.mk
# *** EOF ***

#ASFLAGS			= -ggdb -Wall
#CFLAGS     		= -ggdb -g3 -Wall -std=c99 -O0
#CPPFLAGS   		= -ggdb -Wall
#LDFLAGS    		= -fdata-sections
