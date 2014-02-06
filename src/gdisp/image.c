/*
 * This file is subject to the terms of the GFX License. If a copy of
 * the license was not distributed with this file, you can obtain one at:
 *
 *              http://ugfx.org/license.html
 */

/**
 * @file    src/gdisp/image.c
 * @brief   GDISP generic image code.
 *
 * @defgroup Image Image
 * @ingroup GDISP
 */
#include "gfx.h"

#if GFX_USE_GDISP && GDISP_NEED_IMAGE

#include <string.h>

static gdispImageError imageOpen(gdispImage *img) {
	gdispImageError err;

	img->bgcolor = White;
	for(img->fns = ImageHandlers; img->fns < ImageHandlers+sizeof(ImageHandlers)/sizeof(ImageHandlers[0]); img->fns++) {
		err = img->fns->open(img);
		if (err != GDISP_IMAGE_ERR_BADFORMAT) {
			if (!(err & GDISP_IMAGE_ERR_UNRECOVERABLE))
				return err;
			goto unrecoverable;
		}
		img->io.fns->seek(&img->io, 0);
	}

	err = GDISP_IMAGE_ERR_BADFORMAT;
	img->type = GDISP_IMAGE_TYPE_UNKNOWN;

unrecoverable:
	img->fns->close(img);
	img->flags = 0;
	img->fns = 0;
	img->priv = 0;
	return err;
}

gdispImageError
		DEPRECATED("If possible please use gdispImageOpenFile(), gdispImageOpenMemory() or gdispImageOpenBaseFileStream() instead")
		gdispImageOpen(gdispImage *img) {
	return imageOpen(img);
}

#if GDISP_NEED_IMAGE_NATIVE
	extern gdispImageError gdispImageOpen_NATIVE(gdispImage *img);
	extern void gdispImageClose_NATIVE(gdispImage *img);
	extern gdispImageError gdispImageCache_NATIVE(gdispImage *img);
	extern gdispImageError gdispGImageDraw_NATIVE(GDisplay *g, gdispImage *img, coord_t x, coord_t y, coord_t cx, coord_t cy, coord_t sx, coord_t sy);
	extern delaytime_t gdispImageNext_NATIVE(gdispImage *img);
#endif

#if GDISP_NEED_IMAGE_GIF
	extern gdispImageError gdispImageOpen_GIF(gdispImage *img);
	extern void gdispImageClose_GIF(gdispImage *img);
	extern gdispImageError gdispImageCache_GIF(gdispImage *img);
	extern gdispImageError gdispGImageDraw_GIF(GDisplay *g, gdispImage *img, coord_t x, coord_t y, coord_t cx, coord_t cy, coord_t sx, coord_t sy);
	extern delaytime_t gdispImageNext_GIF(gdispImage *img);
#endif

#if GDISP_NEED_IMAGE_BMP
	extern gdispImageError gdispImageOpen_BMP(gdispImage *img);
	extern void gdispImageClose_BMP(gdispImage *img);
	extern gdispImageError gdispImageCache_BMP(gdispImage *img);
	extern gdispImageError gdispGImageDraw_BMP(GDisplay *g, gdispImage *img, coord_t x, coord_t y, coord_t cx, coord_t cy, coord_t sx, coord_t sy);
	extern delaytime_t gdispImageNext_BMP(gdispImage *img);
#endif

#if GDISP_NEED_IMAGE_JPG
	extern gdispImageError gdispImageOpen_JPG(gdispImage *img);
	extern void gdispImageClose_JPG(gdispImage *img);
	extern gdispImageError gdispImageCache_JPG(gdispImage *img);
	extern gdispImageError gdispGImageDraw_JPG(GDisplay *g, gdispImage *img, coord_t x, coord_t y, coord_t cx, coord_t cy, coord_t sx, coord_t sy);
	extern delaytime_t gdispImageNext_JPG(gdispImage *img);
#endif

#if GDISP_NEED_IMAGE_PNG
	extern gdispImageError gdispImageOpen_PNG(gdispImage *img);
	extern void gdispImageClose_PNG(gdispImage *img);
	extern gdispImageError gdispImageCache_PNG(gdispImage *img);
	extern gdispImageError gdispGImageDraw_PNG(GDisplay *g, gdispImage *img, coord_t x, coord_t y, coord_t cx, coord_t cy, coord_t sx, coord_t sy);
	extern delaytime_t gdispImageNext_PNG(gdispImage *img);
#endif

/* The structure defining the routines for image drawing */
typedef struct gdispImageHandlers {
	gdispImageError	(*open)(gdispImage *img);			/* The open function */
	void			(*close)(gdispImage *img);			/* The close function */
	gdispImageError	(*cache)(gdispImage *img);			/* The cache function */
	gdispImageError	(*draw)(GDisplay *g,
							gdispImage *img,
							coord_t x, coord_t y,
							coord_t cx, coord_t cy,
							coord_t sx, coord_t sy);	/* The draw function */
	delaytime_t		(*next)(gdispImage *img);			/* The next frame function */
} gdispImageHandlers;

static gdispImageHandlers ImageHandlers[] = {
	#if GDISP_NEED_IMAGE_NATIVE
		{	gdispImageOpen_NATIVE,	gdispImageClose_NATIVE,
			gdispImageCache_NATIVE,	gdispGImageDraw_NATIVE,	gdispImageNext_NATIVE,
		},
	#endif
	#if GDISP_NEED_IMAGE_GIF
		{	gdispImageOpen_GIF,		gdispImageClose_GIF,
			gdispImageCache_GIF,	gdispGImageDraw_GIF,	gdispImageNext_GIF,
		},
	#endif
	#if GDISP_NEED_IMAGE_BMP
		{	gdispImageOpen_BMP,		gdispImageClose_BMP,
			gdispImageCache_BMP,	gdispGImageDraw_BMP,	gdispImageNext_BMP,
		},
	#endif
	#if GDISP_NEED_IMAGE_JPG
		{	gdispImageOpen_JPG,		gdispImageClose_JPG,
			gdispImageCache_JPG,	gdispGImageDraw_JPG,	gdispImageNext_JPG,
		},
	#endif
	#if GDISP_NEED_IMAGE_PNG
		{	gdispImageOpen_PNG,		gdispImageClose_PNG,
			gdispImageCache_PNG,	gdispGImageDraw_PNG,	gdispImageNext_PNG,
		},
	#endif
};

static size_t ImageGFileRead(struct gdispImageIO *pio, void *buf, size_t len) {
	if (!pio->fd) return 0;
	len = gfileRead((GFILE *)pio->fd, buf, len);
	if ((int)len < 0) len = 0;
	pio->pos += len;
	return len;
}

static void ImageGFileSeek(struct gdispImageIO *pio, size_t pos) {
	if (!pio->fd) return;
	if (pio->pos != pos) {
		gfileSetPos((GFILE *)pio->fd, pos);
		pio->pos = pos;
	}
}

static void ImageGFileClose(struct gdispImageIO *pio) {
	if (!pio->fd) return;
	gfileClose((GFILE *)pio->fd);
	pio->fd = 0;
	pio->pos = 0;
}

static const gdispImageIOFunctions ImageGFileFunctions =
	{ ImageGFileRead, ImageGFileSeek, ImageGFileClose };

#if GFILE_NEED_MEMFS
	gdispImageError gdispImageOpenMemory(gdispImage *img, const void *memimage) {
		img->io.fns = &ImageGFileFunctions;
		img->io.pos = 0;
		img->io.fd = gfileOpenMemory(memimage, "rb");
		if (!img->io.fd)
			return GDISP_IMAGE_ERR_NOSUCHFILE;
		return imageOpen(img);
	}

	bool_t DEPRECATED("Use gdispImageOpenMemory() instead. GFX_USE_GFILE, GFILE_NEED_MEMFS must also be TRUE") gdispImageSetMemoryReader(gdispImage *img, const void *memimage) {
		img->io.fns = &ImageGFileFunctions;
		img->io.pos = 0;
		img->io.fd = gfileOpenMemory(memimage, "rb");
		return img->io.fd != 0;
	}
#endif

gdispImageError gdispImageOpenFile(gdispImage *img, const char *filename) {
	img->io.fns = &ImageGFileFunctions;
	img->io.pos = 0;
	img->io.fd = gfileOpen(filename, "rb");
	if (!img->io.fd)
		return GDISP_IMAGE_ERR_NOSUCHFILE;
	return imageOpen(img);
}

#if defined(WIN32) || GFX_USE_OS_WIN32 || GFX_USE_OS_LINUX || GFX_USE_OS_OSX
	bool_t DEPRECATED("Please use gdispImageOpenFile() instead. GFX_USE_GFILE must also be TRUE") gdispImageSetFileReader(gdispImage *img, const char *filename) {
		img->io.fns = &ImageGFileFunctions;
		img->io.pos = 0;
		img->io.fd = gfileOpen(filename, "rb");
		return img->io.fd != 0;
	}
#endif

#if GFILE_NEED_CHIBIOSFS && GFX_USE_OS_CHIBIOS
	bool_t DEPRECATED("Use gdispImageOpenBaseFileStream() instead. GFX_USE_GFILE, GFILE_NEED_CHIBIOSFS must also be TRUE") gdispImageSetBaseFileStreamReader(gdispImage *img, void *BaseFileStreamPtr) {
		img->io.fns = &ImageGFileFunctions;
		img->io.pos = 0;
		img->io.fd = gfileOpenBaseFileStream(BaseFileStreamPtr, "rb");
		return img->io.fd != 0;
	}

	gdispImageError gdispImageOpenBaseFileStream(gdispImage *img, void *BaseFileStreamPtr) {
		img->io.fns = &ImageGFileFunctions;
		img->io.pos = 0;
		img->io.fd = gfileOpenBaseFileStream(BaseFileStreamPtr, "rb");
		if (!img->io.fd)
			return GDISP_IMAGE_ERR_NOSUCHFILE;
		return imageOpen(img);
	}

#endif

void gdispImageClose(gdispImage *img) {
	if (img->fns)
		img->fns->close(img);
	else
		img->io.fns->close(&img->io);
	img->type = GDISP_IMAGE_TYPE_UNKNOWN;
	img->flags = 0;
	img->fns = 0;
	img->priv = 0;
}

bool_t gdispImageIsOpen(gdispImage *img) {
	return img->type != GDISP_IMAGE_TYPE_UNKNOWN && img->fns != 0;
}

void gdispImageSetBgColor(gdispImage *img, color_t bgcolor) {
	img->bgcolor = bgcolor;
}

gdispImageError gdispImageCache(gdispImage *img) {
	if (!img->fns) return GDISP_IMAGE_ERR_BADFORMAT;
	return img->fns->cache(img);
}

gdispImageError gdispGImageDraw(GDisplay *g, gdispImage *img, coord_t x, coord_t y, coord_t cx, coord_t cy, coord_t sx, coord_t sy) {
	if (!img->fns) return GDISP_IMAGE_ERR_BADFORMAT;
	return img->fns->draw(g, img, x, y, cx, cy, sx, sy);
}

delaytime_t gdispImageNext(gdispImage *img) {
	if (!img->fns) return GDISP_IMAGE_ERR_BADFORMAT;
	return img->fns->next(img);
}

// Helper Routines
void *gdispImageAlloc(gdispImage *img, size_t sz) {
	#if GDISP_NEED_IMAGE_ACCOUNTING
		void *ptr;

		ptr = gfxAlloc(sz);
		if (ptr) {
			img->memused += sz;
			if (img->memused > img->maxmemused)
				img->maxmemused = img->memused;
		}
		return ptr;
	#else
		(void) img;
		return gfxAlloc(sz);
	#endif
}

void gdispImageFree(gdispImage *img, void *ptr, size_t sz) {
	#if GDISP_NEED_IMAGE_ACCOUNTING
		gfxFree(ptr);
		img->memused -= sz;
	#else
		(void) img;
		(void) sz;
		gfxFree(ptr);
	#endif
}

#endif /* GFX_USE_GDISP && GDISP_NEED_IMAGE */
/** @} */
