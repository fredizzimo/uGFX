/*
 * This file is subject to the terms of the GFX License. If a copy of
 * the license was not distributed with this file, you can obtain one at:
 *
 *              http://ugfx.org/license.html
 */

/**
 * @file    src/gwin/console.c
 * @brief   GWIN sub-system console code.
 */

#include "gfx.h"

#if GFX_USE_GWIN && GWIN_NEED_CONSOLE

#include <string.h>

#include "gwin/class_gwin.h"

#define GWIN_CONSOLE_USE_CLEAR_LINES			TRUE
#define GWIN_CONSOLE_USE_FILLED_CHARS			FALSE

// some temporary warning
#if GWIN_CONSOLE_USE_HISTORY
	#warning "This feature is work in progress and does currently contain a lot of bugs."
#endif

/*
 * Stream interface implementation. The interface is write only
 */

#if GFX_USE_OS_CHIBIOS && GWIN_CONSOLE_USE_BASESTREAM
	#define Stream2GWindow(ip)		((GHandle)(((char *)(ip)) - (size_t)(&(((GConsoleObject *)0)->stream))))

	static size_t GWinStreamWrite(void *ip, const uint8_t *bp, size_t n) { gwinPutCharArray(Stream2GWindow(ip), (const char *)bp, n); return RDY_OK; }
	static size_t GWinStreamRead(void *ip, uint8_t *bp, size_t n) {	(void)ip; (void)bp; (void)n; return 0; }
	static msg_t GWinStreamPut(void *ip, uint8_t b) { gwinPutChar(Stream2GWindow(ip), (char)b); return RDY_OK; }
	static msg_t GWinStreamGet(void *ip) {(void)ip; return RDY_OK; }
	static msg_t GWinStreamPutTimed(void *ip, uint8_t b, systime_t time) { (void)time; gwinPutChar(Stream2GWindow(ip), (char)b); return RDY_OK; }
	static msg_t GWinStreamGetTimed(void *ip, systime_t timeout) { (void)ip; (void)timeout; return RDY_OK; }
	static size_t GWinStreamWriteTimed(void *ip, const uint8_t *bp, size_t n, systime_t time) { (void)time; gwinPutCharArray(Stream2GWindow(ip), (const char *)bp, n); return RDY_OK; }
	static size_t GWinStreamReadTimed(void *ip, uint8_t *bp, size_t n, systime_t time) { (void)ip; (void)bp; (void)n; (void)time; return 0; }

	struct GConsoleWindowVMT_t {
		_base_asynchronous_channel_methods
	};

	static const struct GConsoleWindowVMT_t GWindowConsoleVMT = {
		GWinStreamWrite,
		GWinStreamRead,
		GWinStreamPut,
		GWinStreamGet,
		GWinStreamPutTimed,
		GWinStreamGetTimed,
		GWinStreamWriteTimed,
		GWinStreamReadTimed
	};
#endif

#if GWIN_CONSOLE_USE_HISTORY
	static void CustomRedraw(GWindowObject *gh) {
		#define gcw		((GConsoleObject *)gh)

		uint16_t i;

		// loop through buffer and don't add it again	
		gcw->store = FALSE;
		for (i = 0; i < gcw->last_char; i++) {
			gwinPutChar(gh, gcw->buffer[i]);
		}
		gcw->store = TRUE;
	
		#undef gcw
	}
#endif

static void AfterClear(GWindowObject *gh) {
	#define gcw		((GConsoleObject *)gh)
	gcw->cx = 0;
	gcw->cy = 0;

	#if GWIN_CONSOLE_USE_HISTORY
		// issue an overflow, this is some kind
		// of emptying the buffer
		gcw->last_char = gcw->size;
	#endif

	#undef gcw		
}

static const gwinVMT consoleVMT = {
	"Console",				// The classname
	sizeof(GConsoleObject),	// The object size
	0,						// The destroy routine
	#if GWIN_CONSOLE_USE_HISTORY
		CustomRedraw,		// The redraw routine (custom)
	#else
		0,					// The redraw routine (default)
	#endif
	AfterClear,				// The after-clear routine
};

GHandle gwinGConsoleCreate(GDisplay *g, GConsoleObject *gc, const GWindowInit *pInit) {
	if (!(gc = (GConsoleObject *)_gwindowCreate(g, &gc->g, pInit, &consoleVMT, 0)))
		return 0;

	#if GFX_USE_OS_CHIBIOS && GWIN_CONSOLE_USE_BASESTREAM
		gc->stream.vmt = &GWindowConsoleVMT;
	#endif

	#if GWIN_CONSOLE_USE_HISTORY
		gc->buffer = 0;
		gc->size = 0;
		gc->last_char = 0;
	#endif

	gc->cx = 0;
	gc->cy = 0;

	gwinSetVisible((GHandle)gc, pInit->show);

	return (GHandle)gc;
}

#if GFX_USE_OS_CHIBIOS && GWIN_CONSOLE_USE_BASESTREAM
	BaseSequentialStream *gwinConsoleGetStream(GHandle gh) {
		if (gh->vmt != &consoleVMT)
			return 0;

		return (BaseSequentialStream *)&(((GConsoleObject *)(gh))->stream);
	}
#endif

#if GWIN_CONSOLE_USE_HISTORY
	bool_t gwinConsoleSetBuffer(GHandle gh, void* buffer, size_t size) {
		#define gcw		((GConsoleObject *)gh)

		uint8_t buf_width, buf_height, fp, fy;

		if (gh->vmt != &consoleVMT)
			return FALSE;

		// assign buffer or allocate new one
		if (buffer == 0) {
			(void)size;

			// calculate buffer size
			fy = gdispGetFontMetric(gh->font, fontHeight);
			fp = gdispGetFontMetric(gh->font, fontMinWidth);
			buf_height = (gh->height / fy);
			buf_width = (gh->width / fp);
			
			if ((gcw->buffer = (char*)gfxAlloc(buf_width * buf_height)) == 0)
				return FALSE;
			gcw->size = buf_width * buf_height;

		} else {

			if (size <= 0)
				return FALSE;
			gcw->buffer = (char*)buffer;
			gcw->size = size;

		}

		gcw->last_char = 0;

		return TRUE;
		
		#undef gcw
	}
#endif

void gwinPutChar(GHandle gh, char c) {
	#define gcw		((GConsoleObject *)gh)
	uint8_t			width, fy, fp;

	if (gh->vmt != &consoleVMT || !gh->font)
		return;

	#if GWIN_CONSOLE_USE_HISTORY
		// buffer overflow check
		if (gcw->last_char >= gcw->size)
			gcw->last_char = 0;

		// store new character in buffer
		if (gcw->store && gcw->buffer != 0)
			gcw->buffer[gcw->last_char++] = c;

		// only render new character and don't issue a complete redraw (performance...)
		if (!gwinGetVisible(gh))
			return;
	#endif

	fy = gdispGetFontMetric(gh->font, fontHeight);
	fp = gdispGetFontMetric(gh->font, fontCharPadding);

	#if GDISP_NEED_CLIP
		gdispGSetClip(gh->display, gh->x, gh->y, gh->width, gh->height);
	#endif
	
	if (c == '\n') {
		gcw->cx = 0;
		gcw->cy += fy;
		// We use lazy scrolling here and only scroll when the next char arrives
	} else if (c == '\r') {
		// gcw->cx = 0;
	} else {
		width = gdispGetCharWidth(c, gh->font) + fp;
		if (gcw->cx + width >= gh->width) {
			gcw->cx = 0;
			gcw->cy += fy;
		}

		if (gcw->cy + fy > gh->height) {
#if GDISP_NEED_SCROLL
			/* scroll the console */
			gdispGVerticalScroll(gh->display, gh->x, gh->y, gh->width, gh->height, fy, gh->bgcolor);
			/* reset the cursor to the start of the last line */
			gcw->cx = 0;
			gcw->cy = (((coord_t)(gh->height/fy))-1)*fy;
#else
			/* clear the console */
			gdispGFillArea(gh->display, gh->x, gh->y, gh->width, gh->height, gh->bgcolor);
			/* reset the cursor to the top of the window */
			gcw->cx = 0;
			gcw->cy = 0;
#endif
		}

#if GWIN_CONSOLE_USE_CLEAR_LINES
		/* clear to the end of the line */
		if (gcw->cx == 0)
			gdispGFillArea(gh->display, gh->x, gh->y + gcw->cy, gh->width, fy, gh->bgcolor);
#endif
#if GWIN_CONSOLE_USE_FILLED_CHARS
		gdispGFillChar(gh->display, gh->x + gcw->cx, gh->y + gcw->cy, c, gh->font, gh->color, gh->bgcolor);
#else
		gdispGDrawChar(gh->display, gh->x + gcw->cx, gh->y + gcw->cy, c, gh->font, gh->color);
#endif

		/* update cursor */
		gcw->cx += width;
	}
	#undef gcw
}

void gwinPutString(GHandle gh, const char *str) {
	while(*str)
		gwinPutChar(gh, *str++);
}

void gwinPutCharArray(GHandle gh, const char *str, size_t n) {
	while(n--)
		gwinPutChar(gh, *str++);
}

#include <stdarg.h>

#define MAX_FILLER 11
#define FLOAT_PRECISION 100000

static char *ltoa_wd(char *p, long num, unsigned radix, long divisor) {
	int i;
	char *q;

	if (!divisor) divisor = num;

	q = p + MAX_FILLER;
	do {
		i = (int)(num % radix);
		i += '0';
		if (i > '9')
		  i += 'A' - '0' - 10;
		*--q = i;
		num /= radix;
	} while ((divisor /= radix) != 0);

	i = (int)(p + MAX_FILLER - q);
	do {
		*p++ = *q++;
	} while (--i);

	return p;
}

#if GWIN_CONSOLE_USE_FLOAT
	static char *ftoa(char *p, double num) {
		long l;
		unsigned long precision = FLOAT_PRECISION;

		l = num;
		p = ltoa_wd(p, l, 10, 0);
		*p++ = '.';
		l = (num - l) * precision;
		return ltoa_wd(p, l, 10, precision / 10);
	}
#endif

void gwinPrintf(GHandle gh, const char *fmt, ...) {
	va_list ap;
	char *p, *s, c, filler;
	int i, precision, width;
	bool_t is_long, left_align;
	long l;
	#if GWIN_CONSOLE_USE_FLOAT
		float f;
		char tmpbuf[2*MAX_FILLER + 1];
	#else
		char tmpbuf[MAX_FILLER + 1];
	#endif

	if (gh->vmt != &consoleVMT || !gh->font)
		return;

	va_start(ap, fmt);
	while (TRUE) {
		c = *fmt++;
		if (c == 0) {
			va_end(ap);
			return;
		}
		if (c != '%') {
			gwinPutChar(gh, c);
			continue;
		}

		p = tmpbuf;
		s = tmpbuf;
		left_align = FALSE;
		if (*fmt == '-') {
			fmt++;
			left_align = TRUE;
		}
		filler = ' ';
		if (*fmt == '.') {
			fmt++;
			filler = '0';
		}
		width = 0;

		while (TRUE) {
			c = *fmt++;
			if (c >= '0' && c <= '9')
				c -= '0';
			else if (c == '*')
				c = va_arg(ap, int);
			else
				break;
			width = width * 10 + c;
		}
		precision = 0;
		if (c == '.') {
			while (TRUE) {
				c = *fmt++;
				if (c >= '0' && c <= '9')
					c -= '0';
				else if (c == '*')
					c = va_arg(ap, int);
				else
					break;
				precision = precision * 10 + c;
			}
		}
		/* Long modifier.*/
		if (c == 'l' || c == 'L') {
			is_long = TRUE;
			if (*fmt)
				c = *fmt++;
		}
		else
			is_long = (c >= 'A') && (c <= 'Z');

		/* Command decoding.*/
		switch (c) {
		case 'c':
			filler = ' ';
			*p++ = va_arg(ap, int);
			break;
		case 's':
			filler = ' ';
			if ((s = va_arg(ap, char *)) == 0)
				s = "(null)";
			if (precision == 0)
				precision = 32767;
			for (p = s; *p && (--precision >= 0); p++);
			break;
		case 'D':
		case 'd':
			if (is_long)
				l = va_arg(ap, long);
			else
				l = va_arg(ap, int);
			if (l < 0) {
				*p++ = '-';
				l = -l;
			}
			p = ltoa_wd(p, l, 10, 0);
			break;
		#if GWIN_CONSOLE_USE_FLOAT
			case 'f':
				f = (float) va_arg(ap, double);
				if (f < 0) {
					*p++ = '-';
					f = -f;
				}
				p = ftoa(p, f);
				break;
		#endif
		case 'X':
		case 'x':
			c = 16;
			goto unsigned_common;
		case 'U':
		case 'u':
			c = 10;
			goto unsigned_common;
		case 'O':
		case 'o':
			c = 8;
		unsigned_common:
			if (is_long)
				l = va_arg(ap, long);
			else
				l = va_arg(ap, int);
			p = ltoa_wd(p, l, c, 0);
			break;
		default:
			*p++ = c;
			break;
		}

		i = (int)(p - s);
		if ((width -= i) < 0)
			width = 0;
		if (left_align == FALSE)
			width = -width;
		if (width < 0) {
			if (*s == '-' && filler == '0') {
				gwinPutChar(gh, *s++);
				i--;
			}
			do {
				gwinPutChar(gh, filler);
			} while (++width != 0);
		}
		while (--i >= 0)
			gwinPutChar(gh, *s++);
		while (width) {
			gwinPutChar(gh, filler);
			width--;
		}
	}
}

#endif /* GFX_USE_GWIN && GWIN_NEED_CONSOLE */


