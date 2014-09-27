# See readme.txt for the make API

# Requirements:
#
# GFXLIB:		The location of the uGFX code eg GFXLIB=../ugfx
#

# Optional:
#
# GFXBOARD:		The uGFX Board to include eg GFXBOARD=Win32
# GFXDRIVERS:	The uGFX Drivers to include - separate multiple drivers with spaces eg GFXDRIVERS=multiple/uGFXnet
# GFXDEMO:		Compile a uGFX standard demo? If blank you need to include your own project files. eg GFXDEMO=modules/gwin/widgets
#


include $(GFXLIB)/gfx.mk
SRC     += $(GFXSRC)
DEFS    += $(GFXDEFS)
INCPATH += $(GFXINC)
LIBS	+= $(GFXLIBS)
