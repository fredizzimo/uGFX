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
 * The virtual string file VMT
 ********************************************************/

#include <string.h>

// Special String VMT
static int StringRead(GFILE *f, void *buf, int size) {
	int		res;
	char	*p;

	p = ((char *)f->obj) + f->pos;
	for(res = 0; res < size && *p; res++, p++, buf = ((char *)buf)+1)
		((char *)buf)[0] = *p;
	return res;
}
static int StringWrite(GFILE *f, const void *buf, int size) {
	if ((f->flags & GFILEFLG_APPEND)) {
		while(((char *)f->obj)[f->pos])
			f->pos++;
	}
	memcpy(((char *)f->obj)+f->pos, buf, size);
	((char *)f->obj)[f->pos+size] = 0;
	return size;
}
static const GFILEVMT StringVMT = {
	0,								// next
	0,								// flags
	'_',							// prefix
	0, 0, 0, 0,
	0, 0, StringRead, StringWrite,
	0, 0, 0,
	0, 0, 0,
	#if GFILE_NEED_FILELISTS
		0, 0, 0,
	#endif
};

static void gfileOpenStringFromStaticGFILE(GFILE *f, char *str) {
	if ((f->flags & GFILEFLG_TRUNC))
		str[0] = 0;
	f->vmt = &StringVMT;
	f->obj = str;
	f->pos = 0;
	f->flags |= GFILEFLG_OPEN|GFILEFLG_CANSEEK;
}

GFILE *gfileOpenString(char *str, const char *mode) {
	GFILE	*f;

	// Get an empty file and set the flags
	if (!(f = findemptyfile(mode)))
		return 0;

	// File is open - fill in all the details
	gfileOpenStringFromStaticGFILE(f, str);
	return f;
}
