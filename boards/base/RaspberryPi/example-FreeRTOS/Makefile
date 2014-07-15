# build environment
PREFIX ?= /your compiler path/gcc-arm-none-eabi-4_8-2014q1
ARCH ?= $(PREFIX)/bin/arm-none-eabi

CC = ${ARCH}-gcc
CPP = ${ARCH}-g++
AS = ${ARCH}-as
LD = ${ARCH}-ld
AR = ${ARCH}-ar
OBJCOPY = ${ARCH}-objcopy

PLATFORM = raspi
LINKER_SCRIPT = raspberrypi.ld

CFLAGS = -march=armv6z -g -Wall -Wextra
ASFLAGS = -g 

CFLAGS_FOR_TARGET = #-mcpu=arm1176jzf-s
ASFLAGS_FOR_TARGET = #-mcpu=arm1176jzf-s
LDFLAGS = #--error-unresolved-symbols

GFXLIB := ../uGFX
include $(GFXLIB)/gfx.mk
include $(GFXLIB)/drivers/gdisp/framebuffer/driver.mk

OSLIB := ../FreeRTOS
MODULES := $(OSLIB)/Source/portable/GCC/RaspberryPi
MODULES += $(OSLIB)/Source/portable/MemMang
MODULES += $(OSLIB)/Source
MODULES += Drivers

SRC_DIR := $(MODULES)
INC_DIR := $(addsuffix /include,$(SRC_DIR))
BUILD_DIR := $(addsuffix /build,$(SRC_DIR))

INCLUDEDIRS := $(OSLIB)/Source/portable/GCC/RaspberryPi
INCLUDEDIRS += $(OSLIB)/Source/include
INCLUDEDIRS += Drivers
INCLUDEDIRS += $(GFXINC)

INCLUDES := $(addprefix -I,$(INCLUDEDIRS))

ASRC     := $(foreach sdir,$(SRC_DIR),$(wildcard $(sdir)/*.s))
AOBJ     := $(ASRC:.s=.o)
CSRC     := $(foreach sdir,$(SRC_DIR),$(wildcard $(sdir)/*.c))
CSRC += $(GFXSRC)
COBJ     := $(CSRC:.c=.o)

vpath %.c $(SRC_DIR)
vpath %.cpp $(SRC_DIR)
vpath %.s $(SRC_DIR)

%.o: %.c
	$(CC) $(CFLAGS_FOR_TARGET) $(INCLUDES) $(CFLAGS) -c -o $*.o $<

%.o: %.s
	$(AS) $(ASFLAGS_FOR_TARGET) $(INCLUDES) $(ASFLAGS) -o $*.o $<

OBJ = $(AOBJ) $(COBJ)

bin/kernel.img: bin/kernel.elf
	${OBJCOPY} -O binary $< $@

bin/kernel.elf: LDFLAGS += -L "$(PREFIX)/lib/gcc/arm-none-eabi/4.8.3" -lgcc
bin/kernel.elf: LDFLAGS += -L "$(PREFIX)/arm-none-eabi/lib" -lc
bin/kernel.elf: $(OBJ)
	${LD} $(OBJ) -Map bin/kernel.map -o $@ -T $(LINKER_SCRIPT) ${LDFLAGS}

clean:
	rm -f bin/*.elf bin/*.img bin/*.map $(OBJ)



