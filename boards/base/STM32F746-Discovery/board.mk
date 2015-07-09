GFXINC  +=	$(GFXLIB)/boards/base/STM32F746-Discovery
GFXSRC  +=	$(GFXLIB)/boards/base/STM32F746-Discovery/stm32f746g_discovery_sdram.c \
			$(GFXLIB)/boards/base/STM32F746-Discovery/stm32f7xx_ll_fmc.c

include $(GFXLIB)/drivers/gdisp/STM32LTDC/driver.mk