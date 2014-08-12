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

struct GFILE {
	const struct GFILEVMT *	vmt;
	uint16_t				flags;
		#define	GFILEFLG_OPEN			0x0001		// File is open
		#define	GFILEFLG_READ			0x0002		// Read the file
		#define	GFILEFLG_WRITE			0x0004		// Write the file
		#define	GFILEFLG_APPEND			0x0008		// Append on each write
		#define GFILEFLG_BINARY			0x0010		// Treat as a binary file
		#define	GFILEFLG_DELONCLOSE		0x0020		// Delete on close
		#define	GFILEFLG_CANSEEK		0x0040		// Seek operations are valid
		#define GFILEFLG_FAILONBLOCK	0x0080		// Fail on a blocking call
		#define GFILEFLG_MUSTEXIST		0x0100		// On open file must exist
		#define GFILEFLG_MUSTNOTEXIST	0x0200		// On open file must not exist
		#define GFILEFLG_TRUNC			0x0400		// On open truncate the file
	void *					obj;
	long int				pos;
};

struct gfileList {
	const struct GFILEVMT *	vmt;
	bool_t					dirs;
};

typedef struct GFILEVMT {
	const struct GFILEVMT *	next;
	uint8_t					flags;
		#define GFSFLG_WRITEABLE		0x0001
		#define GFSFLG_CASESENSITIVE	0x0002
		#define GFSFLG_SEEKABLE			0x0004
		#define GFSFLG_FAST				0x0010
		#define GFSFLG_SMALL			0x0020
		#define GFSFLG_TEXTMODES		0x0040
	char					prefix;
	bool_t		(*del)		(const char *fname);
	bool_t		(*exists)	(const char *fname);
	long int	(*filesize)	(const char *fname);
	bool_t		(*ren)		(const char *oldname, const char *newname);
	bool_t		(*open)		(GFILE *f, const char *fname);
	void		(*close)	(GFILE *f);
	int			(*read)		(GFILE *f, void *buf, int size);
	int			(*write)	(GFILE *f, const void *buf, int size);
	bool_t		(*setpos)	(GFILE *f, long int pos);
	long int	(*getsize)	(GFILE *f);
	bool_t		(*eof)		(GFILE *f);
	bool_t		(*mount)	(const char *drive);
	bool_t		(*unmount)	(const char *drive);
	bool_t		(*sync)		(GFILE *f);
	#if GFILE_NEED_FILELISTS
		gfileList *	(*flopen)	(const char *path, bool_t dirs);
		const char *(*flread)	(gfileList *pfl);
		void		(*flclose)	(gfileList *pfl);
	#endif
} GFILEVMT;

// The chain of FileSystems
#define GFILE_CHAINHEAD		0

// The table of GFILE's
static GFILE gfileArr[GFILE_MAX_GFILES];
GFILE *gfileStdIn;
GFILE *gfileStdOut;
GFILE *gfileStdErr;

// Forward definition used by some special open calls
static GFILE *findemptyfile(const char *mode);

/**
 * The order of the file-systems below determines the order
 * that they are searched to find a file.
 * The last defined is the first searched.
 */

/********************************************************
 * The ChibiOS BaseFileStream VMT
 ********************************************************/
#if GFILE_NEED_CHIBIOSFS && GFX_USE_OS_CHIBIOS
	#include "src/gfile/inc_chibiosfs.c"
#endif

/********************************************************
 * The Memory Pointer VMT
 ********************************************************/
#if GFILE_NEED_MEMFS
	#include "src/gfile/inc_memfs.c"
#endif

/********************************************************
 * The RAM file-system VMT
 ********************************************************/
#if GFILE_NEED_RAMFS
	#include "src/gfile/inc_ramfs.c"
#endif

/********************************************************
 * The FAT file-system VMT
 ********************************************************/
#if GFILE_NEED_FATFS
	#include "src/gfile/inc_fatfs.c"
#endif

/********************************************************
 * The native file-system
 ********************************************************/
#if GFILE_NEED_NATIVEFS
	#include "src/gfile/inc_nativefs.c"
#endif

/********************************************************
 * The ROM file-system VMT
 ********************************************************/
#if GFILE_NEED_ROMFS
	#include "src/gfile/inc_romfs.c"
#endif

/********************************************************
 * The virtual string file VMT
 ********************************************************/
#if GFILE_NEED_STRINGS
	#include "src/gfile/inc_strings.c"
#endif

/********************************************************
 * Printg Routines
 ********************************************************/
#if GFILE_NEED_PRINTG
	#include "src/gfile/inc_printg.c"
#endif

/********************************************************
 * Scang Routines
 ********************************************************/
#if GFILE_NEED_SCANG
	#include "src/gfile/inc_scang.c"
#endif

/********************************************************
 * Stdio Emulation Routines
 ********************************************************/
#if GFILE_NEED_STDIO
	#include "src/gfile/inc_stdio.c"
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

void _gfileDeinit(void)
{
	/* ToDo */
}

bool_t gfileExists(const char *fname) {
	const GFILEVMT *p;

	#if GFILE_ALLOW_DEVICESPECIFIC
		if (fname[0] && fname[1] == '|') {
			for(p = FsChain; p; p = p->next) {
				if (p->prefix == fname[0])
					return p->exists && p->exists(fname+2);
			}
			return FALSE;
		}
	#endif

	for(p = FsChain; p; p = p->next) {
		if (p->exists && p->exists(fname))
			return TRUE;
	}
	return FALSE;
}

bool_t gfileDelete(const char *fname) {
	const GFILEVMT *p;

	#if GFILE_ALLOW_DEVICESPECIFIC
		if (fname[0] && fname[1] == '|') {
			for(p = FsChain; p; p = p->next) {
				if (p->prefix == fname[0])
					return p->del && p->del(fname+2);
			}
			return FALSE;
		}
	#endif

	for(p = FsChain; p; p = p->next) {
		if (p->del && p->del(fname))
			return TRUE;
	}
	return FALSE;
}

long int gfileGetFilesize(const char *fname) {
	const GFILEVMT *p;
	long int res;

	#if GFILE_ALLOW_DEVICESPECIFIC
		if (fname[0] && fname[1] == '|') {
			for(p = FsChain; p; p = p->next) {
				if (p->prefix == fname[0])
					return p->filesize ? p->filesize(fname+2) : -1;
			}
			return -1;
		}
	#endif

	for(p = FsChain; p; p = p->next) {
		if (p->filesize && (res = p->filesize(fname)) != -1)
			return res;
	}
	return -1;
}

bool_t gfileRename(const char *oldname, const char *newname) {
	const GFILEVMT *p;

	#if GFILE_ALLOW_DEVICESPECIFIC
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
			return FALSE;
		}
	#endif

	for(p = FsChain; p; p = p->next) {
		if (p->ren && p->ren(oldname,newname))
			return TRUE;
	}
	return FALSE;
}

static GFILE *findemptyfile(const char *mode) {
	GFILE *			f;

	// First find an available GFILE slot.
	for (f = gfileArr; f < &gfileArr[GFILE_MAX_GFILES]; f++) {
		if (!(f->flags & GFILEFLG_OPEN)) {
			// Get the flags
			switch(mode[0]) {
			case 'r':
				f->flags = GFILEFLG_READ|GFILEFLG_MUSTEXIST;
				while (*++mode) {
					switch(mode[0]) {
					case '+':	f->flags |= GFILEFLG_WRITE;			break;
					case 'b':	f->flags |= GFILEFLG_BINARY;		break;
					}
				}
				break;
			case 'w':
				f->flags = GFILEFLG_WRITE|GFILEFLG_TRUNC;
				while (*++mode) {
					switch(mode[0]) {
					case '+':	f->flags |= GFILEFLG_READ;			break;
					case 'b':	f->flags |= GFILEFLG_BINARY;		break;
					case 'x':	f->flags |= GFILEFLG_MUSTNOTEXIST;	break;
					}
				}
				break;
			case 'a':
				f->flags = GFILEFLG_WRITE|GFILEFLG_APPEND;
				while (*++mode) {
					switch(mode[0]) {
					case '+':	f->flags |= GFILEFLG_READ;			break;
					case 'b':	f->flags |= GFILEFLG_BINARY;		break;
					case 'x':	f->flags |= GFILEFLG_MUSTNOTEXIST;	break;
					}
				}
				break;
			default:
				return 0;
			}
			return f;
		}
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
	f->pos = 0;
	f->flags |= GFILEFLG_OPEN;
	if (p->flags & GFSFLG_SEEKABLE)
		f->flags |= GFILEFLG_CANSEEK;
	return TRUE;
}

GFILE *gfileOpen(const char *fname, const char *mode) {
	GFILE *			f;
	const GFILEVMT *p;

	// Get an empty file and set the flags
	if (!(f = findemptyfile(mode)))
		return 0;

	#if GFILE_ALLOW_DEVICESPECIFIC
		if (fname[0] && fname[1] == '|') {
			for(p = FsChain; p; p = p->next) {
				if (p->prefix == fname[0])
					return testopen(p, f, fname+2) ? f : 0;
			}

			// File not found
			return 0;
		}
	#endif

	for(p = FsChain; p; p = p->next) {
		if (testopen(p, f, fname))
			return f;
	}

	// File not found
	return 0;
}

void gfileClose(GFILE *f) {
	if (!f || !(f->flags & GFILEFLG_OPEN))
		return;
	if (f->vmt->close)
		f->vmt->close(f);
	f->flags = 0;
}

size_t gfileRead(GFILE *f, void *buf, size_t len) {
	size_t	res;

	if (!f || (f->flags & (GFILEFLG_OPEN|GFILEFLG_READ)) != (GFILEFLG_OPEN|GFILEFLG_READ))
		return 0;
	if (!f->vmt->read)
		return 0;
	if ((res = f->vmt->read(f, buf, len)) <= 0)
		return 0;
	f->pos += res;
	return res;
}

size_t gfileWrite(GFILE *f, const void *buf, size_t len) {
	size_t	res;

	if (!f || (f->flags & (GFILEFLG_OPEN|GFILEFLG_WRITE)) != (GFILEFLG_OPEN|GFILEFLG_WRITE))
		return 0;
	if (!f->vmt->write)
		return 0;
	if ((res = f->vmt->write(f, buf, len)) <= 0)
		return 0;
	f->pos += res;
	return res;
}

long int gfileGetPos(GFILE *f) {
	if (!f || !(f->flags & GFILEFLG_OPEN))
		return 0;
	return f->pos;
}

bool_t gfileSetPos(GFILE *f, long int pos) {
	if (!f || !(f->flags & GFILEFLG_OPEN))
		return FALSE;
	if (!f->vmt->setpos || !f->vmt->setpos(f, pos))
		return FALSE;
	f->pos = pos;
	return TRUE;
}

long int gfileGetSize(GFILE *f) {
	if (!f || !(f->flags & GFILEFLG_OPEN))
		return 0;
	if (!f->vmt->getsize)
		return 0;
	return f->vmt->getsize(f);
}

bool_t gfileEOF(GFILE *f) {
	if (!f || !(f->flags & GFILEFLG_OPEN))
		return TRUE;
	if (!f->vmt->eof)
		return FALSE;
	return f->vmt->eof(f);
}

bool_t gfileMount(char fs, const char* drive) {
	const GFILEVMT *p;

	// Find the correct VMT
	for(p = FsChain; p; p = p->next) {
		if (p->prefix == fs) {
			if (!p->mount)
				return FALSE;
			return p->mount(drive);
		}
	}
	return FALSE;
}

bool_t gfileUnmount(char fs, const char* drive) {
	const GFILEVMT *p;

	// Find the correct VMT
	for(p = FsChain; p; p = p->next) {
		if (p->prefix == fs) {
			if (!p->mount)
				return FALSE;
			return p->unmount(drive);
		}
	}
	return FALSE;
}

bool_t gfileSync(GFILE *f) {
	if (!f->vmt->sync)
		return FALSE;
	return f->vmt->sync(f);
}

#if GFILE_NEED_FILELISTS
	gfileList *gfileOpenFileList(char fs, const char *path, bool_t dirs) {
		const GFILEVMT *p;
		gfileList *		pfl;

		// Find the correct VMT
		for(p = FsChain; p; p = p->next) {
			if (p->prefix == fs) {
				if (!p->flopen)
					return 0;
				pfl = p->flopen(path, dirs);
				if (pfl) {
					pfl->vmt = p;
					pfl->dirs = dirs;
				}
				return pfl;
			}
		}
		return 0;
	}

	const char *gfileReadFileList(gfileList *pfl) {
		return pfl->vmt->flread ? pfl->vmt->flread(pfl) : 0;
	}

	void gfileCloseFileList(gfileList *pfl) {
		if (pfl->vmt->flclose)
			pfl->vmt->flclose(pfl);
	}
#endif

#endif /* GFX_USE_GFILE */
