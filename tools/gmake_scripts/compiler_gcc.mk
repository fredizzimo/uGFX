# See readme.txt for the make API

# Add ARCH to each of the compiler programs
ifeq ($(XCC),)
	XCC = $(ARCH)gcc
endif
ifeq ($(XCXX),)
	XCXX = $(ARCH)g++
endif
ifeq ($(XAS),)
	XAS = $(ARCH)gcc -x assembler-with-cpp
endif
ifeq ($(XLD),)
	XLD = $(ARCH)gcc
endif
ifeq ($(XOC),)
	XOC = $(ARCH)objcopy
endif
ifeq ($(XOD),)
	XOD = $(ARCH)objdump
endif

# Default project name is the project directory name
ifeq ($(PROJECT),)
	ifneq ($(firstword $(abspath $(firstword $(MAKEFILE_LIST)))),$(lastword $(abspath $(firstword $(MAKEFILE_LIST)))))
	$(error Your directory contains spaces. Gmake barfs at that. Please define PROJECT)
	endif
	PROJECT := $(notdir $(patsubst %/,%,$(dir $(abspath $(firstword $(MAKEFILE_LIST))))))
endif

# Output directory and files
ifeq ($(BUILDDIR),)
	ifeq ($(MAKECMDGOALS),Debug)
	  BUILDDIR = bin/Debug
	endif
	ifeq ($(MAKECMDGOALS),Release)
	  BUILDDIR = bin/Release
	endif
	ifeq ($(MAKECMDGOALS),cleanDebug)
	  BUILDDIR = bin/Debug
	endif
	ifeq ($(MAKECMDGOALS),cleanRelease)
	  BUILDDIR = bin/Release
	endif
	ifeq ($(BUILDDIR),)
	  BUILDDIR = .build
	endif
endif

OBJDIR  = $(BUILDDIR)/obj
DEPDIR  = $(BUILDDIR)/dep

SRCFILE = $<
OBJFILE = $@
LSTFILE = $(@:.o=.lst)
MAPFILE = $(BUILDDIR)/$(PROJECT).map
EXEFILE =
ifeq ($(basename $(OPT_OS)),win32)
	EXEFILE = $(BUILDDIR)/$(PROJECT).exe
	TARGETS = $(EXEFILE)
endif
ifeq ($(basename $(OPT_OS)),linux)
	EXEFILE = $(BUILDDIR)/$(PROJECT)
	TARGETS = $(EXEFILE)
endif
ifeq ($(basename $(OPT_OS)),osx)
	EXEFILE = $(BUILDDIR)/$(PROJECT)
	TARGETS = $(EXEFILE)
endif
ifeq ($(EXEFILE),)
	LDFLAGS += -nostartfiles
	EXEFILE = $(BUILDDIR)/$(PROJECT).elf
	TARGETS = $(EXEFILE) $(BUILDDIR)/$(PROJECT).hex $(BUILDDIR)/$(PROJECT).bin $(BUILDDIR)/$(PROJECT).dmp
endif

SRCFLAGS += -I. $(patsubst %,-I%,$(INCPATH)) $(patsubst %,-D%,$(patsubst -D%,%,$(DEFS)))
LDFLAGS  += $(patsubst %,-L%,$(LIBPATH)) $(patsubst %,-l%,$(patsubst -l%,%,$(LIBS)))
OBJS	  = $(addprefix $(OBJDIR)/,$(subst ../,_dot_dot/,$(addsuffix .o,$(basename $(SRC)))))

ifneq ($(OPT_NONSTANDARD_FLAGS),yes)
	SRCFLAGS += -fomit-frame-pointer -Wall -Wextra -Wstrict-prototypes -fverbose-asm
endif
ifeq ($(OPT_LINK_OPTIMIZE),yes)
	SRCFLAGS += -ffunction-sections -fdata-sections
endif
ifeq ($(OPT_GENERATE_MAP),yes)
	ifeq ($(OPT_LINK_OPTIMIZE),yes)
		LDFLAGS += -Wl,-Map=$(MAPFILE),--cref,--no-warn-mismatch,--gc-sections
	else
		LDFLAGS += -Wl,-Map=$(MAPFILE),--cref,--no-warn-mismatch
	endif
endif
ifeq ($(OPT_GENERATE_LISTINGS),yes)
	CFLAGS += -Wa,-alms=$(LSTFILE)
	CXXFLAGS += -Wa,-alms=$(LSTFILE)
	ASFLAGS += -Wa,-amhls=$(LSTFILE)
endif
ifneq ($(LDSCRIPT),)
	LDFLAGS += -T$(LDSCRIPT)
endif

# Generate dependency information
SRCFLAGS += -MMD -MP -MF $(DEPDIR)/$(@F).d

#
# makefile rules
#

.PHONY: builddirs fakefile.o all clean Debug Release cleanDebug cleanRelease

Debug Release:				all
cleanDebug cleanRelease:	clean

all: builddirs fakefile.o $(TARGETS)

builddirs:
	@mkdir -p $(BUILDDIR)
	@mkdir -p $(OBJDIR)
	@mkdir -p $(DEPDIR)

fakefile.o:
ifneq ($(OPT_VERBOSE_COMPILE),yes)
	@echo Compiler Options - $(XCC) -c $(CPPFLAGS) $(CFLAGS) $(SRCFLAGS) fakefile.c -o $(OBJDIR)/$@
	@echo
endif

.SECONDEXPANSION:
$(OBJDIR)/%.o : $$(subst _dot_dot/,../,%.c)
	@mkdir -p $(dir $@)
ifeq ($(OPT_VERBOSE_COMPILE),yes)
	@echo
	$(XCC) -c $(CPPFLAGS) $(CFLAGS) $(SRCFLAGS) $< -o $@
else
	@echo Compiling $<
	@$(XCC) -c $(CPPFLAGS) $(CFLAGS) $(SRCFLAGS) $< -o $@
endif

$(OBJDIR)/%.o : $$(subst _dot_dot/,../,%.cpp)
	@mkdir -p $(dir $@)
ifeq ($(OPT_VERBOSE_COMPILE),yes)
	@echo
	$(XCXX) -c $(CPPFLAGS) $(CXXFLAGS) $(SRCFLAGS) $< -o $@
else
	@echo Compiling $<
	@$(XCXX) -c $(CPPFLAGS) $(CXXFLAGS) $(SRCFLAGS) $< -o $@
endif

$(OBJDIR)/%.o : $$(subst _dot_dot/,../,%.c++)
	@mkdir -p $(dir $@)
ifeq ($(OPT_VERBOSE_COMPILE),yes)
	@echo
	$(XCXX) -c $(CPPFLAGS) $(CXXFLAGS) $(SRCFLAGS) $< -o $@
else
	@echo Compiling $<
	@$(XCXX) -c $(CPPFLAGS) $(CXXFLAGS) $(SRCFLAGS) $< -o $@
endif

$(OBJDIR)/%.o : $$(subst _dot_dot/,../,%.s)
	@mkdir -p $(dir $@)
ifeq ($(OPT_VERBOSE_COMPILE),yes)
	@echo
	$(XAS) -c $(CPPFLAGS) $(ASFLAGS) $(SRCFLAGS) $< -o $@
else
	@echo Compiling $<
	@$(XAS) -c $(CPPFLAGS) $(ASFLAGS) $(SRCFLAGS) $< -o $@
endif

$(EXEFILE): $(OBJS)
	@mkdir -p $(dir $@)
ifeq ($(OPT_VERBOSE_COMPILE),yes)
	@echo
	$(XLD) $(OBJS) $(LDFLAGS) -o $@
else
	@echo Linking $@
	@$(XLD) $(OBJS) $(LDFLAGS) -o $@
endif
ifeq ($(OPT_COPY_EXE),yes)
	@cp $@ .
endif

%.hex: %.elf $(LDSCRIPT)
ifeq ($(OPT_VERBOSE_COMPILE),yes)
	$(XOC) -O ihex $< $@
else
	@echo Creating $@
	@$(XOC) -O ihex $< $@
endif
ifeq ($(OPT_COPY_EXE),yes)
	@cp $@ .
endif

%.bin: %.elf $(LDSCRIPT)
ifeq ($(USE_VERBOSE_COMPILE),yes)
	$(XOC) -O binary $< $@
else
	@echo Creating $@
	@$(XOC) -O binary $< $@
endif
ifeq ($(OPT_COPY_EXE),yes)
	@cp $@ .
endif

%.dmp: %.elf $(LDSCRIPT)
ifeq ($(USE_VERBOSE_COMPILE),yes)
	$(XOD) -x --syms $< > $@
else
	@echo Creating $@
	@$(XOD) -x --syms $< > $@
	@echo Done
endif
ifeq ($(OPT_COPY_EXE),yes)
	@cp $@ .
endif

gcov:
	-mkdir gcov
	$(COV) -u $(subst /,\,$(SRC))
	-mv *.gcov ./gcov

#
# Include the dependency files, should be the last of the makefile except for clean
#
-include $(shell mkdir -p $(DEPDIR) 2>/dev/null) $(wildcard $(DEPDIR)/*)

clean:
	-rm -fR $(BUILDDIR)

# *** EOF ***
