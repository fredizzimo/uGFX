# See readme.txt for the make API

# Requirements:
#
# CHIBIOS:			The location of the ChibiOS code	eg CHIBIOS=../chibios
#

# Optional:
#

CHIBIOS_BOARD			= simulator
CHIBIOS_PLATFORM		= Win32
CHIBIOS_PORT			= GCC/SIMIA32

DEFS += SIMULATOR SHELL_USE_IPRINTF=FALSE

include $(GFXLIB)/tools/gmake_scripts/os_chibios.mk
include $(GFXLIB)/tools/gmake_scripts/os_win32.mk
