/*
 * This file is subject to the terms of the GFX License. If a copy of
 * the license was not distributed with this file, you can obtain one at:
 *
 *              http://ugfx.org/license.html
 */

/**
 * @file    src/gtrans/gtrans.h
 *
 * @addtogroup GTRANS
 *
 * @brief	Module to allow changing the language of an application dynamically during run-time.
 *
 * @{
 */

#ifndef _TRANS_H
#define _TRANS_H

#include "../../gfx.h"

#if GFX_USE_GTRANS || defined(__DOXYGEN__)

typedef struct transTable {
	unsigned numEntries;
	const char** strings;
} transTable;

#ifdef __cplusplus
extern "C" {
#endif

const char* gtransString(const char* string);
const char* gtransIndex(unsigned index);
void gtransSetBaseLanguage(const transTable* const translation);
void gtransSetLanguage(const transTable* const translation);

#ifdef __cplusplus
}
#endif

#endif /* GFX_USE_GTRANS */

#endif /* _TRANS_H */
/** @} */

