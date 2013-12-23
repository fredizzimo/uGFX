#include "gfx.h"

#include "doomdef.h"

#include "m_argv.h"
#include "d_main.h"

// Emulate a command line
static int argc = 1;
static const char * const *argv = { "doom", };

int main(void) {
	gfxInit();

    myargc = argc; 
    myargv = argv; 
    D_DoomMain();
} 
