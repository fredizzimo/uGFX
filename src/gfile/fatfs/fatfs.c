/*
 * This file is subject to the terms of the GFX License. If a copy of
 * the license was not distributed with this file, you can obtain one at:
 *
 *              http://ugfx.org/license.html
 */

/**
 * @file    src/gfile/fatfs/fatfs.c
 * @brief   GFILE FATFS wrapper.
 *
 */

#include "gfx.h"

#if GFX_USE_GFILE && GFILE_NEED_FATFS

#include "src/ff.c"
#include "src/option/unicode.c"

#endif // GFX_USE_GFILE && GFILE_NEED_FATFS
