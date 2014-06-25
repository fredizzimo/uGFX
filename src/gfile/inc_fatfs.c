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

/*
 * ToDo:
 *
 *   - fatfsExists()
 *   - f_mount has to be called before the disk can be accessed
 *   - complete _flags2mode()
 *   - restructure provided diskio.c files
 *   - do full testing
 */
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

static const GFILEVMT FsFatFSVMT = {
	GFILE_CHAINHEAD,
	GFSFLG_SEEKABLE,
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
	fatfsEOF
};

#undef GFILE_CHAINHEAD
#define GFILE_CHAINHEAD &FsFatFSVMT

static void _flags2mode(GFILE* f, BYTE* mode)
{
	*mode = 0;

	if (f->flags & GFILEFLG_MUSTEXIST)
		*mode |= FA_READ;
	else if (f->flags & GFILEFLG_APPEND)
		*mode |= 0;	/* ToDO */
	else
		*mode |= FA_WRITE;

	/* ToDo - Complete */	
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
	(void)fname;

	/* ToDo */

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
	FIL* fd = 0;
	BYTE mode;
	FRESULT ferr;

	_flags2mode(f, &mode);

	ferr = f_open(fd, fname, mode);
	if (ferr != FR_OK)
		return FALSE;

	f->obj = (void*)fd;

	return TRUE;	
}

static void fatfsClose(GFILE* f)
{
	f_close( (FIL*)f->obj );
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
	return (long int)f_tell( (FIL*)f->obj );
}

static bool_t fatfsEOF(GFILE* f)
{
	if ( f_eof( (FIL*)f->obj ) != 0)
		return TRUE;
	else
		return FALSE;
}

