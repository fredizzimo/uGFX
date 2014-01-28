/*
 * This file is subject to the terms of the GFX License. If a copy of
 * the license was not distributed with this file, you can obtain one at:
 *
 *              http://ugfx.org/license.html
 */

/**
 * This file is included by src/gfile/gfile.c
 */

/********************************************************
 * The ROM file-system VMT
 ********************************************************/

#include <string.h>

typedef struct ROMFS_DIRENTRY {
	const struct ROMFS_DIRENTRY *	next;
	const char *					name;
	long int						size;
	const char *					file;
} ROMFS_DIRENTRY;

#define ROMFS_DIRENTRY_HEAD		0

#include "romfs_files.h"

static const ROMFS_DIRENTRY const *FsROMHead = ROMFS_DIRENTRY_HEAD;

static bool_t ROMExists(const char *fname);
static long int	ROMFilesize(const char *fname);
static bool_t ROMOpen(GFILE *f, const char *fname);
static void ROMClose(GFILE *f);
static int ROMRead(GFILE *f, char *buf, int size);
static bool_t ROMSetpos(GFILE *f, long int pos);
static long int ROMGetsize(GFILE *f);
static bool_t ROMEof(GFILE *f);

static const GFILEVMT FsROMVMT = {
	GFILE_CHAINHEAD,									// next
	GFSFLG_CASESENSITIVE|GFSFLG_SEEKABLE|GFSFLG_FAST,	// flags
	'S',												// prefix
	0, ROMExists, ROMFilesize, 0,
	ROMOpen, ROMClose, ROMRead, 0,
	ROMSetpos, ROMGetsize, ROMEof,
};
#undef GFILE_CHAINHEAD
#define GFILE_CHAINHEAD		&FsROMVMT

static ROMFS_DIRENTRY *ROMFindFile(const char *fname) {
	const ROMFS_DIRENTRY *p;

	for(p = FsROMHead; p; p = p->next) {
		if (!strcmp(p->name, fname))
			break;
	}
	return p;
}
static bool_t ROMExists(const char *fname) { return ROMFindFile(fname) != 0; }
static long int	ROMFilesize(const char *fname) {
	const ROMFS_DIRENTRY *p;

	if (!(p = ROMFindFile(fname))) return -1;
	return p->size;
}
static bool_t ROMOpen(GFILE *f, const char *fname) {
	const ROMFS_DIRENTRY *p;

	if (!(p = ROMFindFile(fname))) return FALSE;
	f->obj = (void *)p;
	return TRUE;
}
static void ROMClose(GFILE *f) { (void)f; }
static int ROMRead(GFILE *f, char *buf, int size) {
	const ROMFS_DIRENTRY *p;

	p = (const ROMFS_DIRENTRY *)f->obj;
	if (p->size - f->pos < size)
		size = p->size - f->pos;
	if (size <= 0)	return 0;
	memcpy(buf, p->file+f->pos, size);
	return size;
}
static bool_t ROMSetpos(GFILE *f, long int pos) { return pos <= ((const ROMFS_DIRENTRY *)f->obj)->size; }
static long int ROMGetsize(GFILE *f) { return ((const ROMFS_DIRENTRY *)f->obj)->size; }
static bool_t ROMEof(GFILE *f) { return f->pos >= ((const ROMFS_DIRENTRY *)f->obj)->size; }
