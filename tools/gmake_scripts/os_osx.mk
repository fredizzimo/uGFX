# See readme.txt for the make API

# Requirements:
#
# OSX_SDK	The location of the SDK eg. OSX_SDK  = /Developer/SDKs/MacOSX10.7.sdk
# OSX_ARCH	The architecture flags eg.  OSX_ARCH = -mmacosx-version-min=10.3 -arch i386
#

SRCFLAGS	+= -isysroot $(OSX_SDK) $(OSX_ARCH)
LDFLAGS		+= -pthread -Wl,-syslibroot,$(OSX_SDK) $(OSX_ARCH)
