/**
 * This file has a different license to the rest of the GFX system.
 * You can copy, modify and distribute this file as you see fit.
 * You do not need to publish your source modifications to this file.
 * The only thing you are not permitted to do is to relicense it
 * under a different license.
 */

#ifndef _GFXCONF_H
#define _GFXCONF_H

/* The operating system to use - one of these must be defined */
//#define GFX_USE_OS_CHIBIOS		TRUE
//#define GFX_USE_OS_WIN32		FALSE
//#define GFX_USE_OS_POSIX		FALSE

/* GFX sub-systems to turn on */
#define GFX_USE_GDISP			TRUE
#define GFX_USE_GMISC			TRUE

/* Features for the GDISP sub-system. */
#define GDISP_NEED_AUTOFLUSH	FALSE
#define GDISP_NEED_VALIDATION	TRUE
#define GDISP_NEED_CLIP			FALSE
#define GDISP_NEED_TEXT			FALSE
#define GDISP_NEED_CIRCLE		FALSE
#define GDISP_NEED_ELLIPSE		FALSE
#define GDISP_NEED_ARC			FALSE
#define GDISP_NEED_SCROLL		FALSE
#define GDISP_NEED_PIXELREAD	FALSE
#define GDISP_NEED_CONTROL		FALSE
#define GDISP_NEED_MULTITHREAD  FALSE
#define GDISP_NEED_STREAMING	TRUE

/* Builtin Fonts */
#define GDISP_INCLUDE_FONT_UI2          FALSE

#define GFX_USE_GMISC				TRUE
#define GMISC_NEED_FIXEDTRIG		FALSE
#define GMISC_NEED_FASTTRIG			FALSE
#define GMISC_NEED_INVSQRT			TRUE
//#define GDISP_INVSQRT_MIXED_ENDIAN	TRUE
//#define GDISP_INVSQRT_REAL_SLOW		TRUE

#endif /* _GFXCONF_H */
