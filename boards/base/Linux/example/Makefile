#
#       !!!! Do NOT edit this makefile with an editor which replace tabs by spaces !!!!
#
##############################################################################################
#
# On command line:
#
# make all = Create project
#
# make clean = Clean project files.
#
# To rebuild project do "make clean" and "make all".
#

##############################################################################################
# Start of make control
#

# Verbose compiling?
USE_VERBOSE_COMPILE=no

# Generate listing files?
USE_LISTING=no

# Your project name and executable file name - Optional, defaults to the project directory name
#PROJECT=uGFX

##############################################################################################
# Start of default section
#

TRGT =
CC   = $(TRGT)gcc
AS   = $(TRGT)gcc -x assembler-with-cpp

# List all default C defines here, like -D_DEBUG=1
DDEFS =

# List all default ASM defines here, like -D_DEBUG=1
DADEFS =

# List all default directories to look for include files here
DINCDIR =

# List the default directory to look for the libraries here
DLIBDIR =

# List all default libraries here
DLIBS = -lX11 -pthread -lrt

#
# End of default section
##############################################################################################

##############################################################################################
# Start of user section
#

# Default project name is the project directory name
ifeq ($(PROJECT),)
	PROJECT := $(notdir $(patsubst %/,%,$(dir $(abspath $(firstword $(MAKEFILE_LIST))))))
endif

# Imported source files and paths for uGFX
GFXLIB = ../ugfx
include ${GFXLIB}/gfx.mk
include ${GFXLIB}/boards/base/Linux/board.mk

# Where is our source code - alter these for your project.
# Either just include the demo makefile or add your own definitions
include $(GFXLIB)/demos/modules/gdisp/basics/demo.mk
#MYFILES = my-project-directory
#MYCSRC  = $(MYFILES)/main.c
#MYDEFS  =

# List all user C define here, like -D_DEBUG=1
UDEFS = $(MYDEFS) $(GFXDEFS)

# Define ASM defines here
UADEFS =

# List C source files here
SRC  = $(GFXSRC) \
       $(MYCSRC)

# List ASM source files here
ASRC =

# List all user directories here
UINCDIR = $(MYFILES) $(GFXINC)

# List the user directory to look for the libraries here
ULIBDIR =

# List all user libraries here
ULIBS =

# Define optimisation level here
OPT = -ggdb -O0 -fomit-frame-pointer

#
# End of user defines
##############################################################################################

# Output directory and files
ifeq ($(MAKECMDGOALS),Debug)
  BUILDDIR = bin/Debug
else ifeq ($(MAKECMDGOALS),Release)
  BUILDDIR = bin/Release
else ifeq ($(BUILDDIR),)
  BUILDDIR = .build
else ifeq ($(BUILDDIR),.)
  BUILDDIR = .build
endif

OBJDIR    = $(BUILDDIR)/obj
LSTDIR    = $(BUILDDIR)/lst
MAPDIR    = $(BUILDDIR)/map
DEPDIR    = .dep

INCDIR  = $(patsubst %,-I%,$(DINCDIR) $(UINCDIR))
LIBDIR  = $(patsubst %,-L%,$(DLIBDIR) $(ULIBDIR))
DEFS    = $(DDEFS) $(UDEFS)
ADEFS   = $(DADEFS) $(UADEFS)
COBJ	= $(addprefix $(OBJDIR)/, $(subst ../,_dot_dot/,$(SRC:.c=.o)))
AOBJ	= $(addprefix $(OBJDIR)/, $(subst ../,_dot_dot/,$(ASRC:.s=.o)))
OBJS    = $(AOBJ) $(COBJ)
LIBS    = $(DLIBS) $(ULIBS)

ASFLAGS = $(ADEFS)
CPFLAGS = $(OPT) -Wall -Wextra -Wstrict-prototypes -fverbose-asm $(DEFS)
LDFLAGS = -Wl,-Map=$(MAPDIR)/$(PROJECT).map,--cref,--no-warn-mismatch $(LIBDIR)
ifeq ($(USE_LISTING),yes)
	ASFLAGS += -Wa,-amhls=$(LSTDIR)/$(subst ../,_dot_dot/,$(<:.s=.lst))
	CPFLAGS += -Wa,-alms=$(LSTDIR)/$(subst ../,_dot_dot/,$(<:.c=.lst))
endif

ifeq ($(HOST_OSX),yes)
  ifeq ($(OSX_SDK),)
    OSX_SDK = /Developer/SDKs/MacOSX10.7.sdk
  endif
  ifeq ($(OSX_ARCH),)
    OSX_ARCH = -mmacosx-version-min=10.3 -arch i386
  endif

  LIBS += $(OSX_ARCH)
  CPFLAGS += -isysroot $(OSX_SDK) $(OSX_ARCH)
  LDFLAGS = -Wl -Map=$(MAPDIR)/$(PROJECT).map,-syslibroot,$(OSX_SDK),$(LIBDIR)
else
  # Linux, or other
  CPFLAGS += -m32
endif

# Generate dependency information
CPFLAGS += -MD -MP -MF $(DEPDIR)/$(@F).d

#
# makefile rules
#

all: $(BUILDDIR) $(OBJS) $(PROJECT)

$(BUILDDIR) $(OBJDIR) $(LSTDIR) $(MAPDIR):
	mkdir -p $(OBJDIR)
	mkdir -p $(MAPDIR)
ifeq ($(USE_LISTING),yes)
	mkdir -p $(LSTDIR)
endif
ifneq ($(USE_VERBOSE_COMPILE),yes)
	@echo Compiler Options - $(CC) -c $(CPFLAGS) -I. $(INCDIR) main.c -o $(OBJDIR)/main.o
	@echo
endif

.SECONDEXPANSION:
$(OBJDIR)/%.o : $$(subst _dot_dot/,../,%.c)
	@mkdir -p $(dir $@)
ifeq ($(USE_VERBOSE_COMPILE),yes)
	@echo
	$(CC) -c $(CPFLAGS) -I. $(INCDIR) $< -o $@
else
	@echo Compiling $<
	@$(CC) -c $(CPFLAGS) -I. $(INCDIR) $< -o $@
endif

$(OBJDIR)/%.o : $$(subst _dot_dot/,../,%.s)
	@mkdir -p $(dir $@)
ifeq ($(USE_VERBOSE_COMPILE),yes)
	@echo
	$(AS) -c $(ASFLAGS) -I. $(INCDIR) $< -o $@
else
	@echo Compiling $<
	@$(AS) -c $(ASFLAGS) -I. $(INCDIR) $< -o $@
endif

$(PROJECT): $(OBJS)
	$(CC) $(OBJS) $(LDFLAGS) $(LIBS) -o $@

gcov:
	-mkdir gcov
	$(COV) -u $(subst /,\,$(SRC))
	-mv *.gcov ./gcov

clean:
	-rm -f $(BUILDDIR)
	-rm -fR $(DEPDIR)

#
# Include the dependency files, should be the last of the makefile
#
-include $(shell mkdir $(DEPDIR) 2>/dev/null) $(wildcard $(DEPDIR)/*)

# *** EOF ***
