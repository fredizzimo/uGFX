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
 * The Memory pointer file-system VMT
 ********************************************************/

#include <string.h>

static int MEMRead(GFILE *f, char *buf, int size);
static int MEMWrite(GFILE *f, char *buf, int size);
static bool_t MEMSetpos(GFILE *f, long int pos);

static const GFILEVMT FsMemVMT = {
	0,													// next
	GFSFLG_SEEKABLE|GFSFLG_WRITEABLE,					// flags
	0,													// prefix
	0, 0, 0, 0,
	0, 0, MEMRead, MEMWrite,
	MEMSetpos, 0, 0,
};

static int MEMRead(GFILE *f, char *buf, int size) {
	memset(buf, ((char *)f->fd)+f->pos, size);
	return size;
}
static int MEMWrite(GFILE *f, char *buf, int size) {
	memset(((char *)f->fd)+f->pos, buf, size);
	return size;
}
static bool_t MEMSetpos(GFILE *f, long int pos) {
	return TRUE;
}
