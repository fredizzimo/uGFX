/*
 * Copyright (c) 2012, 2013, Joel Bodenmann aka Tectu <joel@unormal.org>
 * Copyright (c) 2012, 2013, Andrew Hannam aka inmarket
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *    * Redistributions of source code must retain the above copyright
 *      notice, this list of conditions and the following disclaimer.
 *    * Redistributions in binary form must reproduce the above copyright
 *      notice, this list of conditions and the following disclaimer in the
 *      documentation and/or other materials provided with the distribution.
 *    * Neither the name of the <organization> nor the
 *      names of its contributors may be used to endorse or promote products
 *      derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL <COPYRIGHT HOLDER> BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "gfx.h"

static const char* EnglishStrings[] = {
    "Welcome",
    "The number %s has the value %d",
    "Goodbye"
};
static const transTable EnglishTranslation = { sizeof(EnglishStrings)/sizeof(EnglishStrings[0]), EnglishStrings };

static const char* GermanStrings[] = {
    "Herzlich Willkommen",
    "Die Zahl %s hat den Wert %d",
    "Auf Wiedersehen"
};
static const transTable GermanTranslation = { sizeof(GermanStrings)/sizeof(GermanStrings[0]), GermanStrings };

int main(void)
{
    size_t i, j;
    font_t font;

    gfxInit();
    gdispClear(Silver);

    font = gdispOpenFont("*");

    gtransSetBaseLanguage(&EnglishTranslation);
    gtransSetLanguage(&GermanTranslation);

    gtransSetLanguage(&EnglishTranslation);
    i = 0;
    for (j = 0; j < EnglishTranslation.numEntries; j++) {
        gdispFillStringBox(20+300*i, 35*j, 300, 35, gtransIndex(j), font, Black, Silver, justifyLeft);
    }

    gtransSetLanguage(&GermanTranslation);
    i = 1;
    for (j = 0; j < EnglishTranslation.numEntries; j++) {
        gdispFillStringBox(20+300*i, 35*j, 300, 35, gtransIndex(j), font, Black, Silver, justifyLeft);
    }

    gdispFillStringBox(20, 300, 300, 25, gtransString("Welcome"), font, Black, Silver, justifyLeft);

	while (TRUE) {
		gfxSleepMilliseconds(500);
	}

	return 0;
}

