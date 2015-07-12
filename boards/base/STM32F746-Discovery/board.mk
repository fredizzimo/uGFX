GFXINC  +=	$(GFXLIB)/boards/base/STM32F746-Discovery
GFXSRC  +=	$(GFXLIB)/boards/base/STM32F746-Discovery/stm32f746g_discovery_sdram.c \
			$(GFXLIB)/boards/base/STM32F746-Discovery/stm32f7xx_ll_fmc.c
GFXDEFS +=	STM32F746xx

ifeq ($(OPT_OS),raw32)
	GFXSRC	+=	$(HAL)/Src/stm32f7xx_hal.c \
				$(HAL)/Src/stm32f7xx_hal_cortex.c \
				$(HAL)/Src/stm32f7xx_hal_flash.c \
				$(HAL)/Src/stm32f7xx_hal_flash_ex.c \
				$(HAL)/Src/stm32f7xx_hal_rcc.c \
				$(HAL)/Src/stm32f7xx_hal_rcc_ex.h \
				$(HAL)/Src/stm32f7xx_hal_gpio.c \
				$(HAL)/Src/stm32f7xx_hal_pwr.c \
				$(HAL)/Src/stm32f7xx_hal_pwr_ex.c \
				$(HAL)/Src/stm32f7xx_hal_sdram.c \
				$(HAL)/Src/stm32f7xx_hal_dma.c
	GFXSRC	+=	$(GFXLIB)/boards/base/STM32F746-Discovery/stm32f746g_raw32_startup.s \
				$(GFXLIB)/boards/base/STM32F746-Discovery/stm32f746g_raw32_ugfx.c \
				$(GFXLIB)/boards/base/STM32F746-Discovery/stm32f746g_raw32_system.c \
				$(GFXLIB)/boards/base/STM32F746-Discovery/stm32f746g_raw32_interrupts.c
	GFXDEFS	+=	GFX_OS_EXTRA_INIT_FUNCTION=Raw32OSInit
	SRCFLAGS+=	-std=c99
	GFXINC	+=	$(CMSIS)/Device/ST/STM32F7xx/Include \
				$(CMSIS)/Include \
				$(HAL)/Inc
	LDSCRIPT = $(GFXLIB)/boards/base/STM32F746-Discovery/stm32f746nghx_flash.ld
endif

include $(GFXLIB)/drivers/gdisp/STM32LTDC/driver.mk