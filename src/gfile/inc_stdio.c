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
 * Stdio Emulation Routines
 ********************************************************/

size_t gstdioRead(void * ptr, size_t size, size_t count, FILE *f) {
	return gfileRead(f, ptr, size*count)/size;
}

size_t gstdioWrite(const void * ptr, size_t size, size_t count, FILE *f) {
	return gfileWrite(f, ptr, size*count)/size;
}

int gstdioSeek(FILE *f, size_t offset, int origin) {
	switch(origin) {
	case SEEK_SET:
		break;
	case SEEK_CUR:
		offset += f->pos;
		break;
	case SEEK_END:
		offset += gfileGetSize(f);
		break;
	default:
		return -1;
	}
	return gfileSetPos(f, offset) ? 0 : -1;
}

int gstdioGetpos(FILE *f, long int *pos) {
	if (!(f->flags & GFILEFLG_OPEN))
		return -1;
	*pos = f->pos;
	return 0;
}
