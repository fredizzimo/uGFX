# Possible Targets:	all clean Debug cleanDebug Release cleanRelease

##############################################################################################
# Settings
#

# General settings
	# See $(GFXLIB)/tools/gmake_scripts/readme.txt for the list of variables
	OPT_VERBOSE_COMPILE		= no
	OPT_GENERATE_LISTINGS	= yes
	OPT_GENERATE_MAP		= yes
	OPT_COPY_EXE			= no
	OPT_LINK_OPTIMIZE		= yes
	OPT_NONSTANDARD_FLAGS	= no
	OPT_OS					= osx

# uGFX settings
	# See $(GFXLIB)/tools/gmake_scripts/library_ugfx.mk for the list of variables
	GFXLIB					= ../uGFX
	GFXBOARD				= OSX
	#GFXDRIVERS				= multiple/uGFXnet
	GFXDEMO					= modules/gdisp/basics

# OSX settings
	# See $(GFXLIB)/tools/gmake_scripts/os_osx.mk for the list of variables
	OSX_SDK = /Developer/SDKs/MacOSX10.7.sdk
	OSX_ARCH = -mmacosx-version-min=10.3 -arch i386

##############################################################################################
# Set these for your project
#

ARCH     =
SRCFLAGS = -ggdb -O0
SRCFLAGS+= -m32
CFLAGS   =
CXXFLAGS =
ASFLAGS  =
LDFLAGS  =

SRC      =
DEFS     =
LIBS     =
INCPATH  =
LIBPATH  =

##############################################################################################
# These should be at the end
#

include $(GFXLIB)/tools/gmake_scripts/library_ugfx.mk
include $(GFXLIB)/tools/gmake_scripts/os_$(OPT_OS).mk
include $(GFXLIB)/tools/gmake_scripts/compiler_gcc.mk
# *** EOF ***
