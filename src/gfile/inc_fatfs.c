/*
 * This file is subject to the terms of the GFX License. If a copy of
 * the license was not distributed with this file, you can obtain one at:
 *
 *              http://ugfx.org/license.html
 */

/**
 * This file is included by src/gfile/gfile.c
 */

#include "ff.h"
#include "ffconf.h"

/********************************************************
 * The FAT file-system VMT
 ********************************************************/

static bool_t fatfsDel(const char* fname);
static bool_t fatfsExists(const char* fname);
static long int fatfsFileSize(const char* fname);
static bool_t fatfsRename(const char* oldname, const char* newname);
static bool_t fatfsOpen(GFILE* f, const char* fname);
static void fatfsClose(GFILE* f);
static int fatfsRead(GFILE* f, void* buf, int size);
static int fatfsWrite(GFILE* f, const void* buf, int size);
static bool_t fatfsSetPos(GFILE* f, long int pos);
static long int fatfsGetSize(GFILE* f);
static bool_t fatfsEOF(GFILE* f);
static bool_t fatfsMount(const char* drive);
static bool_t fatfsUnmount(const char* drive);
static bool_t fatfsSync(GFILE* f);

static const GFILEVMT FsFatFSVMT = {
	GFILE_CHAINHEAD,
	GFSFLG_WRITEABLE | GFSFLG_SEEKABLE,
	'F',
	fatfsDel,
	fatfsExists,
	fatfsFileSize,
	fatfsRename,
	fatfsOpen,
	fatfsClose,
	fatfsRead,
	fatfsWrite,
	fatfsSetPos,
	fatfsGetSize,
	fatfsEOF,
	fatfsMount,
	fatfsUnmount,
	fatfsSync
};

#undef GFILE_CHAINHEAD
#define GFILE_CHAINHEAD &FsFatFSVMT

// optimize these later on. Use an array to have multiple FatFS
static bool_t fatfs_mounted = FALSE;
static FATFS fatfs_fs;

static BYTE fatfs_flags2mode(GFILE* f)
{
	BYTE mode = 0;

	if (f->flags & GFILEFLG_READ)
		mode |= FA_READ;
	if (f->flags & GFILEFLG_WRITE)
		mode |= FA_WRITE;
	if (f->flags & GFILEFLG_APPEND)
		mode |= 0;  // ToDo
	if (f->flags & GFILEFLG_TRUNC)
		mode |= FA_CREATE_ALWAYS;

	/* ToDo - Complete */
	return mode;
}

static bool_t fatfsDel(const char* fname)
{
	FRESULT ferr;

	ferr = f_unlink( (const TCHAR*)fname );
	if (ferr != FR_OK)
		return FALSE;

	return TRUE;
}

static bool_t fatfsExists(const char* fname)
{
	FRESULT ferr;
	FILINFO fno;

	ferr = f_stat( (const TCHAR*)fname, &fno);
	if (ferr != FR_OK)
		return FALSE;

	return TRUE;
}

static long int fatfsFileSize(const char* fname)
{
	FRESULT ferr;
	FILINFO fno;

	ferr = f_stat( (const TCHAR*)fname, &fno );
	if (ferr != FR_OK)
		return 0;

	return (long int)fno.fsize;
}

static bool_t fatfsRename(const char* oldname, const char* newname)
{
	FRESULT ferr;

	ferr = f_rename( (const TCHAR*)oldname, (const TCHAR*)newname );
	if (ferr != FR_OK)
		return FALSE;

	return TRUE;
}

static bool_t fatfsOpen(GFILE* f, const char* fname)
{
	FIL* fd;

	#if !GFILE_NEED_NOAUTOMOUNT
		if (!fatfs_mounted && !fatfsMount(""))
			return FALSE;
	#endif

	if (!(fd = gfxAlloc(sizeof(FIL))))
		return FALSE;

	if (f_open(fd, fname, fatfs_flags2mode(f)) != FR_OK) {
		gfxFree(fd);
		f->obj = 0;

		return FALSE;
	}

	f->obj = (void*)fd;

	#if !GFILE_NEED_NOAUTOSYNC
		// no need to sync when not opening for write
		if (f->flags & GFILEFLG_WRITE) {
			f_sync( (FIL*)f->obj );
		}
	#endif

	return TRUE;	
}

static void fatfsClose(GFILE* f)
{
	if ((FIL*)f->obj != 0) { 
		gfxFree( (FIL*)f->obj );
		f_close( (FIL*)f->obj );
	}
}

static int fatfsRead(GFILE* f, void* buf, int size)
{
	int br;

	f_read( (FIL*)f->obj, buf, size, (UINT*)&br);

	return br;
}

static int fatfsWrite(GFILE* f, const void* buf, int size)
{
	int wr;

	f_write( (FIL*)f->obj, buf, size, (UINT*)&wr);
	#if !GFILE_NEED_NOAUTOSYNC
		f_sync( (FIL*)f->obj );
	#endif

	return wr;
}

static bool_t fatfsSetPos(GFILE* f, long int pos)
{
	FRESULT ferr;

	ferr = f_lseek( (FIL*)f->obj, (DWORD)pos );
	if (ferr != FR_OK)
		return FALSE;

	return TRUE;
}

static long int fatfsGetSize(GFILE* f)
{
	return (long int)f_size( (FIL*)f->obj );
}

static bool_t fatfsEOF(GFILE* f)
{
	if ( f_eof( (FIL*)f->obj ) != 0)
		return TRUE;
	else
		return FALSE;
}

static bool_t fatfsMount(const char* drive)
{
	FRESULT ferr;

	if (!fatfs_mounted) {
		ferr = f_mount(&fatfs_fs, drive, 1);
		if (ferr !=  FR_OK)
			return FALSE;
		fatfs_mounted = TRUE;
		return TRUE;
	}

	return FALSE;
}

static bool_t fatfsUnmount(const char* drive)
{
	(void)drive;

	if (fatfs_mounted) {
		// FatFS does not provide an unmount routine.
		fatfs_mounted = FALSE;
		return TRUE;
	}

	return FALSE;
}

static bool_t fatfsSync(GFILE *f)
{
	FRESULT ferr;

	ferr = f_sync( (FIL*)f->obj );
	if (ferr != FR_OK) {
		return FALSE;
	}

	return TRUE;
}

