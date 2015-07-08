GFXINC  +=	$(GFXLIB)/boards/base/STM32F746-Discovery
GFXSRC  +=	$(GFXLIB)/boards/base/STM32F746-Discovery/STM32F746_discovery_sdram.c \
			$(GFXLIB)/boards/base/STM32F746-Discovery/stm32f4xx_fmc.c

include $(GFXLIB)/drivers/gdisp/STM32F746Discovery/driver.mk