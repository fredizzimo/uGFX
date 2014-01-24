/*
 * This file is subject to the terms of the GFX License. If a copy of
 * the license was not distributed with this file, you can obtain one at:
 *
 *              http://ugfx.org/license.html
 */

/**
 * @file    src/gfile/gfile.c
 * @brief   GFILE code.
 *
 */

#define GFILE_IMPLEMENTATION
#include "gfx.h"

#if GFX_USE_GFILE

// The chain of FileSystems
#define GFILE_CHAINHEAD		0

// The table of GFILE's
static GFILE gfileArr[GFILE_MAX_GFILES];
GFILE *gfileStdIn;
GFILE *gfileStdOut;
GFILE *gfileStdErr;

/**
 * The order of the file-systems below determines the order
 * that they are searched to find a file.
 * The last defined is the first searched.
 */

/********************************************************
 * The RAM file-system VMT
 ********************************************************/
#if GFILE_NEED_RAMFS
	#include "../src/gfile/inc_ramfs.c"
#endif

/********************************************************
 * The FAT file-system VMT
 ********************************************************/
#ifndef GFILE_NEED_FATFS
	#include "../src/gfile/inc_fatfs.c"
#endif

/********************************************************
 * The native file-system
 ********************************************************/
#if GFILE_NEED_NATIVEFS
	#include "../src/gfile/inc_nativefs.c"
#endif

/********************************************************
 * The ROM file-system VMT
 ********************************************************/
#if GFILE_NEED_ROMFS
	#include "../src/gfile/inc_romfs.c"
#endif

/********************************************************
 * IO routines
 ********************************************************/

/**
 * The chain of file systems.
 */
static const GFILEVMT const * FsChain = GFILE_CHAINHEAD;

/**
 * The init routine
 */
void _gfileInit(void) {
	#if GFILE_NEED_NATIVEFS
		NativeStdIn.flags = GFILEFLG_OPEN|GFILEFLG_READ;
		NativeStdIn.vmt = &FsNativeVMT;
		NativeStdIn.obj = (void *)stdin;
		NativeStdIn.pos = 0;
		gfileStdIn = &NativeStdIn;
		NativeStdOut.flags = GFILEFLG_OPEN|GFILEFLG_WRITE|GFILEFLG_APPEND;
		NativeStdOut.vmt = &FsNativeVMT;
		NativeStdOut.obj = (void *)stdout;
		NativeStdOut.pos = 0;
		gfileStdOut = &NativeStdOut;
		NativeStdErr.flags = GFILEFLG_OPEN|GFILEFLG_WRITE|GFILEFLG_APPEND;
		NativeStdErr.vmt = &FsNativeVMT;
		NativeStdErr.obj = (void *)stderr;
		NativeStdErr.pos = 0;
		gfileStdErr = &NativeStdErr;
	#endif
}

bool_t	gfileExists(const char *fname) {
	const GFILEVMT *p;

	if (fname[0] && fname[1] == '|') {
		for(p = FsChain; p; p = p->next) {
			if (p->prefix == fname[0])
				return p->exists && p->exists(fname+2);
		}
	} else {
		for(p = FsChain; p; p = p->next) {
			if (p->exists && p->exists(fname))
				return TRUE;
		}
	}
	return FALSE;
}

bool_t	gfileDelete(const char *fname) {
	const GFILEVMT *p;

	if (fname[0] && fname[1] == '|') {
		for(p = FsChain; p; p = p->next) {
			if (p->prefix == fname[0])
				return p->del && p->del(fname+2);
		}
	} else {
		for(p = FsChain; p; p = p->next) {
			if (p->del && p->del(fname))
				return TRUE;
		}
	}
	return FALSE;
}

long int gfileGetFilesize(const char *fname) {
	const GFILEVMT *p;

	if (fname[0] && fname[1] == '|') {
		for(p = FsChain; p; p = p->next) {
			if (p->prefix == fname[0])
				return p->filesize ? p->filesize(fname+2) : -1;
		}
	} else {
		long int res;

		for(p = FsChain; p; p = p->next) {
			if (p->filesize && (res = p->filesize(fname)) != -1)
				return res;
		}
	}
	return -1;
}

bool_t	gfileRename(const char *oldname, const char *newname) {
	const GFILEVMT *p;

	if ((oldname[0] && oldname[1] == '|') || (newname[0] && newname[1] == '|')) {
		char ch;

		if (oldname[0] && oldname[1] == '|') {
			ch = oldname[0];
			oldname += 2;
			if (newname[0] && newname[1] == '|') {
				if (newname[0] != ch)
					// Both oldname and newname are fs specific but different ones.
					return FALSE;
				newname += 2;
			}
		} else {
			ch = newname[0];
			newname += 2;
		}
		for(p = FsChain; p; p = p->next) {
			if (p->prefix == ch)
				return p->ren && p->ren(oldname, newname);
		}
	} else {
		for(p = FsChain; p; p = p->next) {
			if (p->ren && p->ren(oldname,newname))
				return TRUE;
		}
	}
	return FALSE;
}

static uint16_t mode2flags(const char *mode) {
	uint16_t	flags;

	switch(mode[0]) {
	case 'r':
		flags = GFILEFLG_READ|GFILEFLG_MUSTEXIST;
		while (*++mode) {
			switch(mode[0]) {
			case '+':	flags |= GFILEFLG_WRITE;		break;
			case 'b':	flags |= GFILEFLG_BINARY;		break;
			}
		}
		return flags;
	case 'w':
		flags = GFILEFLG_WRITE|GFILEFLG_TRUNC;
		while (*++mode) {
			switch(mode[0]) {
			case '+':	flags |= GFILEFLG_READ;			break;
			case 'b':	flags |= GFILEFLG_BINARY;		break;
			case 'x':	flags |= GFILEFLG_MUSTNOTEXIST;	break;
			}
		}
		return flags;
	case 'a':
		flags = GFILEFLG_WRITE|GFILEFLG_APPEND;
		while (*++mode) {
			switch(mode[0]) {
			case '+':	flags |= GFILEFLG_READ;			break;
			case 'b':	flags |= GFILEFLG_BINARY;		break;
			case 'x':	flags |= GFILEFLG_MUSTNOTEXIST;	break;
			}
		}
		return flags;
	}
	return 0;
}

static bool_t testopen(const GFILEVMT *p, GFILE *f, const char *fname) {
	// If we want write but the fs doesn't allow it then return
	if ((f->flags & GFILEFLG_WRITE) && !(p->flags & GFSFLG_WRITEABLE))
		return FALSE;

	// Try to open
	if (!p->open || !p->open(f, fname))
		return FALSE;

	// File is open - fill in all the details
	f->vmt = p;
	f->err = 0;
	f->pos = 0;
	f->flags |= GFILEFLG_OPEN;
	if (p->flags & GFSFLG_SEEKABLE)
		f->flags |= GFILEFLG_CANSEEK;
	return TRUE;
}

GFILE *gfileOpen(const char *fname, const char *mode) {
	GFILE *f;
	const GFILEVMT *p;

	// First find an available GFILE slot.
	for (f = gfileArr; f < &gfileArr[GFILE_MAX_GFILES]; f++) {
		if (!(f->flags & GFILEFLG_OPEN)) {

			// Get the requested mode
			if (!(f->flags = mode2flags(mode)))
				return FALSE;

			// Try to open the file
			if (fname[0] && fname[1] == '|') {
				for(p = FsChain; p; p = p->next) {
					if (p->prefix == fname[0])
						return testopen(p, f, fname+2);
				}
			} else {
				for(p = FsChain; p; p = p->next) {
					if (testopen(p, f, fname))
						return TRUE;
				}
			}

			// File not found
			return FALSE;
		}
	}

	// No available slot
	return FALSE;
}

void gfileClose(GFILE *f) {
	// Make sure it is one of the system GFILE's
	if (f < gfileArr || f >= &gfileArr[GFILE_MAX_GFILES])
		return;
}

size_t	gfileRead(GFILE *f, char *buf, size_t len) {

}

size_t	gfileWrite(GFILE *f, const char *buf, size_t len) {

}

long int	gfileGetPos(GFILE *f) {

}

bool_t gfileSetPos(GFILE *f, long int pos) {

}

long int	gfileGetSize(GFILE *f) {

}

/********************************************************
 * printg routines
 ********************************************************/
#if GFILE_NEED_PRINTG
#endif

/********************************************************
 * scang routines
 ********************************************************/
#if GFILE_NEED_SCANG
#endif

/********************************************************
 * stdio emulation routines
 ********************************************************/
#ifndef GFILE_NEED_STDIO
	#define GFILE_NEED_STDIO		FALSE
#endif

#endif /* GFX_USE_GFILE */
