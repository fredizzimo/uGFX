#
# This file is subject to the terms of the GFX License. If a copy of
# the license was not distributed with this file, you can obtain one at:
#
#             http://ugfx.org/license.html
#

# See readme.txt for the make API

# Requirements:
#
# CHIBIOS:			The location of the ChibiOS code	eg CHIBIOS=../chibios
# CHIBIOS_PLATFORM	The name of the ChibiOS platform	eg CHIBIOS_PLATFORM=AT91SAM7
# CHIBIOS_PORT		The name of the ChibiOS port		eg CHIBIOS_PORT=GCC/ARM/AT91SAM7
#

# Optional:
#
# CHIBIOS_LDSCRIPT	The name of the loader script		eg CHIBIOS_LDSCRIPT=AT91SAM7X256.ld
# CHIBIOS_BOARD		The name of the ChibiOS board		eg CHIBIOS_BOARD=OLIMEX_SAM7_EX256 - if not specified you must include equivalent code yourself
# CHIBIOS_STM32LIB	Use the STM32 library source for drivers instead of native drivers (yes or no) - default no
# CHIBIOS_VERSION	Which version of ChibiOS is this (2 or 3) - default is 2
#

PATHLIST += CHIBIOS

ifeq ($(CHIBIOS_VERSION),3)
  include $(CHIBIOS)/os/hal/hal.mk
  include $(CHIBIOS)/os/hal/osal/rt/osal.mk
  include $(CHIBIOS)/os/hal/ports/$(CHIBIOS_PLATFORM)/platform.mk
  include $(CHIBIOS)/os/rt/rt.mk
  include $(CHIBIOS)/os/rt/ports/$(CHIBIOS_PORT).mk
else
  include $(CHIBIOS)/os/hal/hal.mk
  include $(CHIBIOS)/os/hal/platforms/$(CHIBIOS_PLATFORM)/platform.mk
  include $(CHIBIOS)/os/kernel/kernel.mk
  include $(CHIBIOS)/os/ports/$(CHIBIOS_PORT)/port.mk
endif

ifneq ($(CHIBIOS_BOARD),)
  include $(CHIBIOS)/boards/$(CHIBIOS_BOARD)/board.mk
endif
ifeq ($(LDSCRIPT),)
  ifneq ($(CHIBIOS_LDSCRIPT),)
    LDSCRIPT= $(PORTLD)/$(CHIBIOS_LDSCRIPT)
  endif
endif

ifeq ($(CHIBIOS_STM32LIB),yes)
  include $(CHIBIOS)/ext/stm32lib/stm32lib.mk
  SRC     += $(STM32SRC)
  DEFS    += USE_STDPERIPH_DRIVER
  INCPATH += $(STM32INC)
endif

INCPATH += $(PORTINC) $(KERNINC) $(TESTINC) \
           $(HALINC) $(PLATFORMINC) $(BOARDINC)
SRC  += $(PORTSRC) \
        $(KERNSRC) \
        $(TESTSRC) \
        $(HALSRC) \
        $(PLATFORMSRC) \
        $(BOARDSRC) \
        $(PORTASM)
