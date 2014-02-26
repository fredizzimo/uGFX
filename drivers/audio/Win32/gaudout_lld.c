/*
 * This file is subject to the terms of the GFX License. If a copy of
 * the license was not distributed with this file, you can obtain one at:
 *
 *              http://ugfx.org/license.html
 */

/**
 * @file    drivers/audio/Win32/gaudout_lld.c
 * @brief   GAUDOUT - Driver file for Win32.
 */

#include "gfx.h"

#if GFX_USE_GAUDOUT

/* Include the driver defines */
#include "src/gaudout/driver.h"

#undef Red
#undef Green
#undef Blue
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>
#include <mmsystem.h>

#define MAX_WAVE_HEADERS		2				// Larger numbers enable more buffering which is good for ensuring
												// there are no skips due to data not being available, however larger
												// numbers of buffers also create higher latency.

static HWAVEOUT		ah = 0;
static volatile int	nQueuedBuffers;
static bool_t		isRunning;
static WAVEHDR		WaveHdrs[MAX_WAVE_HEADERS];
static HANDLE		waveThread;
static DWORD		threadID;

/*
static void PrintWaveErrorMsg(DWORD err, TCHAR * str)
{
	#define BUFFERSIZE 128
	char	buffer[BUFFERSIZE];

	fprintf(stderr, "GAUDOUT: ERROR 0x%08X: %s\r\n", err, str);
	if (mciGetErrorString(err, &buffer[0], sizeof(buffer)))
		fprintf(stderr, "%s\r\n", &buffer[0]);
	else
		fprintf(stderr, "0x%08X returned!\r\n", err);
}
*/

/**************************** waveProc() *******************************
 * We don't use CALLBACK_FUNCTION because it is restricted to calling only
 * a few particular Windows functions, namely some of the time functions,
 * and a few of the Low Level MIDI API. If you violate this rule, your app can
 * hang inside of the callback). One of the Windows API that a callback can't
 * call is waveOutUnPrepareBuffer() which is what we need to use whenever we receive a
 * MM_WOM_DONE. My callback would need to defer that job to another thread
 * anyway, so instead just use CALLBACK_THREAD here instead.
 *************************************************************************/

static bool_t senddata(WAVEHDR *pwh) {
	GAudioData *paud;

	// Get the next data block to send
	if (!(paud = gaudoutGetDataBlockI()))
		return FALSE;

	// Prepare the wave header for Windows
	pwh->dwUser = (DWORD_PTR)paud;
	pwh->lpData = (LPSTR)(paud+1);			// The data is on the end of the structure
	pwh->dwBufferLength = paud->len;
	pwh->dwFlags = 0;
	pwh->dwLoops = 0;
	if (waveOutPrepareHeader(ah, pwh, sizeof(WAVEHDR))) {
		pwh->lpData = 0;
		fprintf(stderr, "GAUDOUT: Failed to prepare a buffer");
		return FALSE;
	}

	// Send it to windows
	if (waveOutWrite(ah, pwh, sizeof(WAVEHDR))) {
		pwh->lpData = 0;
		fprintf(stderr, "GAUDOUT: Failed to write the buffer");
		return FALSE;
	}

	nQueuedBuffers++;
	return TRUE;
}

static DWORD WINAPI waveProc(LPVOID arg) {
	MSG			msg;
	WAVEHDR		*pwh;
	(void)		arg;

	while (GetMessage(&msg, 0, 0, 0)) {
		switch (msg.message) {
			case MM_WOM_DONE:
				pwh = (WAVEHDR *)msg.lParam;

				// Windows - Let go!
				waveOutUnprepareHeader(ah, pwh, sizeof(WAVEHDR));

				// Give the buffer back to the Audio Free List
				gaudoutReleaseDataBlockI((GAudioData *)pwh->dwUser);
				pwh->lpData = 0;
				nQueuedBuffers--;

				// Try and get a new block
				if (isRunning)
					senddata(pwh);
                break;
		}
	}
	return 0;
}

/*===========================================================================*/
/* External declarations.                                                    */
/*===========================================================================*/

void gaudout_lld_deinit() {
	if (ah) {
		isRunning = FALSE;
		waveOutReset(ah);
		while(nQueuedBuffers) Sleep(1);
		waveOutClose(ah);
		ah = 0;
	}
}

bool_t gaudout_lld_init(uint16_t channel, uint32_t frequency) {
	WAVEFORMATEX	wfx;

	if (!waveThread) {
		if (!(waveThread = CreateThread(0, 0, (LPTHREAD_START_ROUTINE)waveProc, 0, 0, &threadID))) {
			fprintf(stderr, "GAUDOUT: Can't create WAVE play-back thread\n");
			return FALSE;
		}
		CloseHandle(waveThread);
	}

	wfx.wFormatTag = WAVE_FORMAT_PCM;
	wfx.nChannels = channel == GAUDOUT_STEREO ? 2 : 1;
	wfx.nSamplesPerSec = frequency;
	wfx.nBlockAlign = wfx.nChannels * sizeof(audout_sample_t);
	wfx.nAvgBytesPerSec = wfx.nSamplesPerSec * wfx.nBlockAlign;
	wfx.wBitsPerSample = sizeof(audout_sample_t) * 8;
	wfx.cbSize = 0;

	if (waveOutOpen(&ah, WAVE_MAPPER, &wfx, (DWORD_PTR)threadID, 0, CALLBACK_THREAD)) {
		fprintf(stderr, "GAUDOUT: Can't open WAVE play-back device\n");
		return FALSE;
	}

	return TRUE;
}

bool_t gaudout_lld_set_volume(uint8_t vol) {
	if (!ah)
		return FALSE;
	return waveOutSetVolume(ah, (((uint16_t)vol)<<8)|vol) != 0;
}

void gaudout_lld_start(void) {
	WAVEHDR		*pwh;

	if (!ah)
		return;

	isRunning = TRUE;
	while (nQueuedBuffers < MAX_WAVE_HEADERS) {
		// Find the empty one - there will always be at least one.
		for(pwh = WaveHdrs; pwh->lpData; pwh++);

		// Grab the next audio block from the Audio Out Queue
		if (!senddata(pwh))
			break;
	}
}

void gaudout_lld_stop(void) {
	isRunning = FALSE;
	if (ah)
		waveOutReset(ah);
}

#endif /* GFX_USE_GAUDOUT */
