/*
 * This file is subject to the terms of the GFX License. If a copy of
 * the license was not distributed with this file, you can obtain one at:
 *
 *              http://ugfx.org/license.html
 */

#include <string.h>
#include "../../gfx.h"

#if GFX_USE_GTRANS

static const transTable* _languageBase;
static const transTable* _languageCurrent;

void _gtransInit(void)
{
	_languageBase = 0;
	_languageCurrent = 0;
}

void _gtransDeinit(void)
{
}

const char* gtransString(const char* string)
{
	size_t i = 0;
	while (1) {
		if (i >= _languageBase->numEntries-1) {
			return 0;
		}

		if (strcmp(string, _languageBase->strings[i]) == 0) {
			break;
		}

		i++;
	}

	if (i >= _languageCurrent->numEntries-1) {
		return 0;
	}

	return _languageCurrent->strings[i];
}

const char* gtransIndex(unsigned index)
{
	if (!_languageCurrent) {
		return 0;
	}

	if (index >= _languageCurrent->numEntries) {
		return 0;
	}

	return _languageCurrent->strings[index];
}

void gtransSetBaseLanguage(const transTable* const translation)
{
	_languageBase = translation;
}

void gtransSetLanguage(const transTable* const translation)
{
	_languageCurrent = translation;
}

#endif /* GFX_USE_GTRANS */
