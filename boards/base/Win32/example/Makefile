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

# Turn ChibiOS simimulator on or off?
USE_CHIBIOS=no

# Verbose compiling?
USE_VERBOSE_COMPILE=no

# Generate listing files?
USE_LISTING=no

# Your project name and executable file name - Optional, defaults to the project directory name
#PROJECT=uGFX

#
# End of make control
##############################################################################################

##############################################################################################
# Start of default section
#

CC   = i686-pc-mingw32-gcc -g

# List all default C defines here, like -D_DEBUG=1
DDEFS =

# List all default directories to look for include files here
DINCDIR =

# List the default directory to look for the libraries here
DLIBDIR =

# List all default libraries here
DLIBS = -lws2_32 -lgdi32 -lwinmm

# Make sure this empty for now
SRC =

#
# End of default section
##############################################################################################

##############################################################################################
# Start of user section
#

# Imported source files and paths for uGFX
GFXLIB = ../uGFX
include $(GFXLIB)/gfx.mk
include $(GFXLIB)/boards/base/Win32/board.mk

# Imported source files and paths for ChibiOS
ifeq ($(USE_CHIBIOS),yes)
	CHIBIOS = ../ChibiOS
	include $(CHIBIOS)/boards/simulator/board.mk
	include ${CHIBIOS}/os/hal/hal.mk
	include ${CHIBIOS}/os/hal/platforms/Win32/platform.mk
	include ${CHIBIOS}/os/ports/GCC/SIMIA32/port.mk
	include ${CHIBIOS}/os/kernel/kernel.mk
	DDEFS += -DSIMULATOR -DSHELL_USE_IPRINTF=FALSE
	UINCDIR += $(PORTINC) $(KERNINC) $(TESTINC) \
	          $(HALINC) $(PLATFORMINC) $(BOARDINC)
	#          ${CHIBIOS}/os/various
	SRC  += ${PORTSRC} \
	       ${KERNSRC} \
	       ${TESTSRC} \
	       ${HALSRC} \
	       ${PLATFORMSRC} \
	       $(BOARDSRC)
	GFXDEFS += -DGFX_USE_OS_CHIBIOS=TRUE
else
	GFXDEFS += -DGFX_USE_OS_WIN32=TRUE
endif

# Where is our source code - alter these for your project.
# Either just include the demo makefile or add your own definitions
include $(GFXLIB)/demos/modules/gdisp/basics/demo.mk
#MYFILES = my-project-directory
#MYCSRC  = $(MYFILES)/main.c
#MYDEFS  =

# List C source files here
SRC  += ${GFXSRC} \
	   ${MYCSRC}

# List all user C define here, like -D_DEBUG=1
UDEFS = ${GFXDEFS}

# List all user directories here
UINCDIR = ${GFXINC} \
		  ${MYFILES}

# List the user directory to look for the libraries here
ULIBDIR =

# List all user libraries here
ULIBS =

# Define optimisation level here
#OPT = -ggdb -O2 -fomit-frame-pointer
OPT = -ggdb -O0 -fomit-frame-pointer

#
# End of user defines
##############################################################################################


# Default project name is the project directory name
ifeq ($(PROJECT),)
	PROJECT := $(notdir $(patsubst %/,%,$(dir $(abspath $(firstword $(MAKEFILE_LIST))))))
endif

# Output directory and files
ifeq ($(MAKECMDGOALS),Debug)
  BUILDDIR = bin/Debug
else ifeq ($(MAKECMDGOALS),Release)
  BUILDDIR = bin/Release
else ifeq ($(MAKECMDGOALS),cleanDebug)
  BUILDDIR = bin/Debug
else ifeq ($(MAKECMDGOALS),cleanRelease)
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

LDFLAGS = -Wl,-Map=$(MAPDIR)/$(PROJECT).map,--cref,--no-warn-mismatch $(LIBDIR)
CPFLAGS = $(OPT) -Wall -Wextra -Wstrict-prototypes -fverbose-asm $(DEFS)
ifeq ($(USE_LISTING),yes)
	CPFLAGS += -Wa,-alms=$(LSTDIR)/$(<:.c=.lst)
endif

# Generate dependency information
CPFLAGS += -MD -MP -MF $(DEPDIR)/$(@F).d

#
# makefile rules
#

Debug Release: all

cleanDebug cleanRelease:	clean

all: $(BUILDDIR) $(OBJS) $(BUILDDIR)/$(PROJECT).exe MAKE_ALL_RULE_HOOK
#all: main.cp

main.cp:	$(MYCSRC)
	$(CC)  -c $(CPFLAGS) -E -P -I. $(INCDIR) $< -o $@

MAKE_ALL_RULE_HOOK:

$(BUILDDIR) $(OBJDIR) $(LSTDIR):
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

%.exe: $(OBJS)
ifeq ($(USE_VERBOSE_COMPILE),yes)
	@echo
	$(CC) $(OBJS) $(LDFLAGS) $(LIBS) -o $@
else
	@echo Linking $@
	@$(CC) $(OBJS) $(LDFLAGS) $(LIBS) -o $@
endif

gcov:
	-mkdir gcov
	$(COV) -u $(subst /,\,$(SRC))
	-mv *.gcov ./gcov

clean:
	-rm -fR $(BUILDDIR)
	-rm -fR $(DEPDIR)

#
# Include the dependency files, should be the last of the makefile
#
-include $(shell mkdir $(DEPDIR) 2>/dev/null) $(wildcard $(DEPDIR)/*)

# *** EOF ***
