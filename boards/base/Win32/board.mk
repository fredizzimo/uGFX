GFXINC  += $(GFXLIB)/boards/base/Win32
GFXSRC  +=

include $(GFXLIB)/drivers/multiple/Win32/gdisp_lld.mk
include $(GFXLIB)/drivers/gaudin/Win32/gaudin_lld.mk
