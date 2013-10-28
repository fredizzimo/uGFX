GFXINC  += $(GFXLIB)/boards/base/Olimex-SAM7EX256-GE8
GFXSRC  +=
GFXDEFS += -DGFX_USE_OS_CHIBIOS=TRUE

#This board has a Nokia6610GE8 display 
include $(GFXLIB)/drivers/gdisp/Nokia6610GE8/gdisp_lld.mk
#This board supports GADC via the AT91SAM7 driver
include $(GFXLIB)/drivers/gadc/AT91SAM7/gadc_lld.mk
#This board supports GINPUT dials via the GADC driver
include $(GFXLIB)/drivers/ginput/dial/GADC/ginput_lld.mk
#This board supports GINPUT toggles via the Pal driver
include $(GFXLIB)/drivers/ginput/toggle/Pal/ginput_lld.mk
#This board support GAUDIN via the GADC driver
include $(GFXLIB)/drivers/gaudin/gadc/gaudin_lld.mk
