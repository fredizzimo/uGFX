/*
 * This file is subject to the terms of the GFX License. If a copy of
 * the license was not distributed with this file, you can obtain one at:
 *
 *              http://ugfx.org/license.html
 */

/**
 * @file    include/gfile/gfile.h
 * @brief   GFILE - File IO Routines header file.
 *
 * @addtogroup GFILE
 *
 * @brief	Module which contains Operating system independent FILEIO
 *
 * @{
 */

#ifndef _GFILE_H
#define _GFILE_H

#include "gfx.h"

#if GFX_USE_GMISC || defined(__DOXYGEN__)

/*===========================================================================*/
/* Type definitions                                                          */
/*===========================================================================*/

/**
 * @brief	A file pointer
 */

typedef struct GFILE {
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
} GFILE;

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
	bool_t		del(const char *fname);
	bool_t		exists(const char *fname);
	long int	filesize(const char *fname);
	bool_t		ren(const char *oldname, const char *newname);
	bool_t		open(GFILE *f, const char *fname);
	void		close(GFILE *f);
	int			read(GFILE *f, char *buf, int size);
	int			write(GFILE *f, const char *buf, int size);
	bool_t		setpos(GFILE *f, long int pos);
	long int	getsize(GFILE *f);
	bool_t		eof(GFILE *f);
} GFILEVMT;

typedef void	GFILE;

extern GFILE *gfileStdIn;
extern GFILE *gfileStdErr;
extern GFILE *gfileStdOut;

/*===========================================================================*/
/* External declarations.                                                    */
/*===========================================================================*/

//TODO
//FILE * tmpfile ( void );		// Auto-deleting
//char * tmpnam ( char * str );
//L_tmpnam - Minimum length for temporary file name
//FILENAME_MAX - Maximum length of file names (constant )
// FOPEN_MAX - Potential limit of simultaneous open streams (constant )
// TMP_MAX - Number of temporary files (constant )
//FILE * freopen ( const char * filename, const char * mode, FILE * stream );
//setbuf
//setvbuf
//fflush
//fscanf
//scanf
//sscanf
//vscanf
//vsscanf
//fgetc
//fgets
//fputc
//fputs
//getc
//getchar
//puts
//ungetc
//void perror (const char * str);

#ifdef __cplusplus
extern "C" {
#endif

	bool_t		gfileExists(const char *fname);
	bool_t		gfileDelete(const char *fname);
	long int	gfileGetFilesize(const char *fname);
	bool_t		gfileRename(const char *oldname, const char *newname);
	GFILE *		gfileOpen(const char *fname, const char *mode);
	void		gfileClose(GFILE *f);
	size_t		gfileRead(GFILE *f, char *buf, size_t len);
	size_t		gfileWrite(GFILE *f, const char *buf, size_t len);
	long int	gfileGetPos(GFILE *f);
	bool_t		gfileSetPos(GFILE *f, long int pos);
	long int	gfileGetSize(GFILE *f);
	bool_t		gfileEOF(GFILE *f);

	#if GFILE_NEED_PRINTG
		int vfnprintg(GFILE *f, int maxlen, const char *fmt, va_list arg);
		int fnprintg(GFILE *f, int maxlen, const char *fmt, ...);
		#define vfprintg(f,m,a)			vfnprintg(f,0,m,a)
		#define fprintg(f,m,...)		fnprintg(f,0,m,...)
		#define vprintg(m,a)			vfnprintg(gfileStdOut,0,m,a)
		#define printg(m,...)			fnprintg(gfileStdOut,0,m,...)

		#if GFILE_NEED_STRINGS
			int vsnprintg(char *buf, int maxlen, const char *fmt, va_list arg);
			int snprintg(char *buf, int maxlen, const char *fmt, ...);
			#define vsprintg(s,m,a)			vsnprintg(s,0,m,a)
			#define sprintg(s,m,...)		snprintg(s,0,m,...)
		#endif
	#endif


	#if GFILE_NEED_STDIO && !defined(GFILE_IMPLEMENTATION)
		#define FILE					GFILE
		#define fopen(n,m)				gfileOpen(n,m)
		#define fclose(f)				gfileClose(f)
		size_t gstdioRead(void * ptr, size_t size, size_t count, FILE *f);
		size_t gstdioWrite(const void * ptr, size_t size, size_t count, FILE *f);
		#define fread(p,sz,cnt,f)		gstdioRead(p,sz,cnt,f)
		#define fwrite(p,sz,cnt,f)		gstdioWrite(p,sz,cnt,f)
		int gstdioSeek(FILE *f, size_t offset, int origin);
		#define fseek(f,ofs,org)		gstdioSeek(f,ofs,org)
			#define SEEK_SET	0
			#define SEEK_CUR	1
			#define SEEK_END	2
		#define remove(n)				(!gfileDelete(n))
		#define rename(o,n)				(!gfileRename(o,n))
		#define fflush(f)				(0)
		#define ftell(f)				gfileGetPos(f)
		#define fpos_t					long int
		int gstdioGetpos(FILE *f, long int *pos);
		#define fgetpos(f,pos)			gstdioGetpos(f,pos)
		#define fsetpos(f, pos)			(!gfileSetPos(f, *pos))
		#define rewind(f)				gfileSetPos(f, 0);
		#define clearerr(f)				(0)
		#define feof(f)					gfileEOF(f)
		//#define ferror(f)				(0)

		#define vfprintf(f,m,a)			vfnprintg(f,0,m,a)
		#define fprintf(f,m,...)		fnprintg(f,0,m,...)
		#define vprintf(m,a)			vfnprintg(gfileStdOut,0,m,a)
		#define printf(m,...)			fnprintg(gfileStdOut,0,m,...)
		#define vsnprintf(s,n,m,a)		vsnprintg(s,n,m,a)
		#define snprintf(s,n,m,...)		snprintg(s,n,m,...)
		#define vsprintf(s,m,a)			vsnprintg(s,0,m,a)
		#define sprintf(s,m,...)		snprintg(s,0,m,...)
	#endif

#ifdef __cplusplus
}
#endif

#endif /* GFX_USE_MISC */

#endif /* _GMISC_H */
/** @} */

