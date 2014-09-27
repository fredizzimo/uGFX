All make script files in this directory apply the following rules and assumptions:

- The scripts are written using gmake syntax
- They assume access to the following unix utilities
	rm, cp, mv, mkdir, sh
- They use and implement the following make variables

Input Variables (all optional unless otherwise specified)
----------------------------

OPT_VERBOSE_COMPILE=no|yes		- Turn on full compile messages - default no
OPT_GENERATE_LISTINGS=no|yes	- Generate listing files - default no
OPT_GENERATE_MAP=no|yes			- Generate a map file - default no
OPT_COPY_EXE=no|yes				- Copy the final program to the local project directory - default no
OPT_NATIVEOS=win32|linux|osx|chibios|freertos|ecos|raw32|rawrtos	- Mandatory: The real operating system of the machine
OPT_OS=win32|linux|osx|chibios|freertos|ecos|raw32|rawrtos		- Mandatory: Should be the same as OPT_NATIVEOS except when running an OS simulator

BUILDDIR						- Build Directory - default is ".build" or "bin/Debug" or "bin/Release" depending on the target
PROJECT							- Project Name - default is the name of the project directory

ARCH							- Architecture - default is ""
XCC								- C compiler - default is "$(ARCH)gcc"
XCXX							- C++ compiler - default is "$(ARCH)g++"
XAS								- Assembler - default is "$(ARCH)gcc -x assembler-with-cpp"
XLD								- Linker - default is "$(ARCH)gcc"

SRCFLAGS						- Compiler defines for c, c++ and assembler files - default is ""
CFLAGS							- C specific compiler defines - default is ""
CXXFLAGS						- C++ specific compiler flags - default is ""
CPPFLAGS						- C Preprocessor flags for c, c++ and assembler files - default is ""
ASFLAGS							- Assembler specific compiler flags - default is ""
LDFLAGS							- Linker flags - default is ""

The following variables are a list of space separated values. In some cases an optional prefix (if specified) will be stripped off
the variables for compatibility with old definitions.

INCPATH							- List of header include directories - default is ""
LIBPATH							- List of library include directories - default is ""
DEFS							- List of preprocessor defines (any -D prefix is ignored) - default is ""
LIBS							- List of libraries (any -l prefix is ignored) - default is ""
SRC								- List of c, c++ and assembler source files - default is ""

Variables for use in variable defintions
----------------------------------------

SRCFILE							- The original source file
OBJFILE							- The output object file
LSTFILE							- The listing file
MAPFILE							- The map file
EXEFILE							- The final project output file

Targets
----------------------------

all
clean
Debug
cleanDebug
Release
cleanRelease
