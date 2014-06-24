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

static bool_t fatfsDel(const char* fname)
{

}

static bool_t fatfsExists(const char* fname)
{

}

static long int fatfsFileSize(const char* fname)
{

}

static bool_t fatfsRename(const char* oldname, const char* newname)
{

}

static bool_t fatfsOpen(GFILE* f, const char* fname)
{

}

static void fatfsClose(GFILE* f)
{

}

static int fatfsRead(GFILE* f, void* buf, int size)
{

}

static int fatfsWrite(GFILE* f, const void* buf, int size)
{

}

static bool_t fatfsSetPos(GFILE* f, long int pos)
{

}

static long int fatfsGetSize(GFILE* f)
{

}

static bool_t fatfsEOF(GFILE* f)
{

}

