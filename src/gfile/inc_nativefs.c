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
 * The native file-system
 ********************************************************/

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

static GFILE NativeStdIn;
static GFILE NativeStdOut;
static GFILE NativeStdErr;

static bool_t NativeDel(const char *fname);
static bool_t NativeExists(const char *fname);
static long int	NativeFilesize(const char *fname);
static bool_t NativeRen(const char *oldname, const char *newname);
static bool_t NativeOpen(GFILE *f, const char *fname, const char *mode);
static void NativeClose(GFILE *f);
static int NativeRead(GFILE *f, char *buf, int size);
static int NativeWrite(GFILE *f, char *buf, int size);
static bool_t NativeSetpos(GFILE *f, long int pos);
static long int NativeGetsize(GFILE *f);
static bool_t NativeEof(GFILE *f);

static const GFILEVMT FsNativeVMT = {
	GFILE_CHAINHEAD,									// next
	'N',												// prefix
	#if !defined(WIN32) && !GFX_USE_OS_WIN32
		GFSFLG_CASESENSITIVE|
	#endif
	GFSFLG_WRITEABLE|GFSFLG_SEEKABLE|GFSFLG_FAST,		// flags
	NativeDel, NativeExists, NativeFilesize, NativeRen,
	NativeOpen, NativeClose, NativeRead, NativeWrite,
	NativeSetpos, NativeGetsize, NativeEof,
};
#undef GFILE_CHAINHEAD
#define GFILE_CHAINHEAD		&FsNativeVMT

static bool_t NativeDel(const char *fname) { return remove(fname) ? FALSE : TRUE; }
static bool_t NativeExists(const char *fname) { return access(fname, 0) ? FALSE : TRUE; }
static long int	NativeFilesize(const char *fname) {
	struct stat st;
	if (stat(fname, &st)) return -1;
	return st.st_size;
}
static bool_t NativeRen(const char *oldname, const char *newname) { return rename(oldname, newname) ? FALSE : TRUE };
static bool_t NativeOpen(GFILE *f, const char *fname, const char *mode) {
	FILE *fd;

	if (!(fd = fopen(fname, mode)))
		return FALSE;
	f->vmt = &FsNativeVMT;
	f->obj = (void *)fd;
	return TRUE;
}
static void NativeClose(GFILE *f) { fclose((FILE *)f->obj); }
static int NativeRead(GFILE *f, char *buf, int size) { return fread(buf, 1, size, (FILE *)f->obj); }
static int NativeWrite(GFILE *f, char *buf, int size) { return fwrite(buf, 1, size, (FILE *)f->obj); }
static bool_t NativeSetpos(GFILE *f, long int pos) {
	if (fseek((FILE *)f->obj, pos, SEEK_SET)) return FALSE;
	f->pos = pos;
	return TRUE;
}
static long int NativeGetsize(GFILE *f) {
	struct stat st;
	if (fstat(fileno((FILE *)f->obj), &st)) return -1;
	return st.st_size;
}
static bool_t NativeEof(GFILE *f) { return feof((FILE *)f->obj) ? TRUE : FALSE; }
