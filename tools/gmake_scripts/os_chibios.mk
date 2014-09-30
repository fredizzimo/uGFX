# See readme.txt for the make API

# Requirements:
#
# CHIBIOS:			The location of the ChibiOS code	eg CHIBIOS=../chibios
# CHIBIOS_BOARD		The name of the ChibiOS board		eg CHIBIOS_BOARD=OLIMEX_SAM7_EX256
# CHIBIOS_PLATFORM	The name of the ChibiOS platform	eg CHIBIOS_PLATFORM=AT91SAM7
# CHIBIOS_PORT		The name of the ChibiOS port		eg CHIBIOS_PORT=GCC/ARM/AT91SAM7
#

# Optional:
#
# CHIBIOS_LDSCRIPT	The name of the loader script		eg CHIBIOS_LDSCRIPT=$(PORTLD)/AT91SAM7X256.ld
#

include $(CHIBIOS)/boards/$(CHIBIOS_BOARD)/board.mk
include $(CHIBIOS)/os/hal/platforms/$(CHIBIOS_PLATFORM)/platform.mk
include $(CHIBIOS)/os/hal/hal.mk
include $(CHIBIOS)/os/ports/$(CHIBIOS_PORT)/port.mk
include $(CHIBIOS)/os/kernel/kernel.mk
LDSCRIPT= $(CHIBIOS_LDSCRIPT)
INCPATH += $(PORTINC) $(KERNINC) $(TESTINC) \
          $(HALINC) $(PLATFORMINC) $(BOARDINC)
SRC  += $(PORTSRC) \
        $(KERNSRC) \
        $(TESTSRC) \
        $(HALSRC) \
        $(PLATFORMSRC) \
        $(BOARDSRC) \
        $(PORTASM)
