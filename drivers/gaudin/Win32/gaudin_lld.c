/*
 * This file is subject to the terms of the GFX License. If a copy of
 * the license was not distributed with this file, you can obtain one at:
 *
 *              http://ugfx.org/license.html
 */

/**
 * @file    drivers/gaudin/Win32/gaudin_lld.c
 * @brief   GAUDIN - Driver file for Win32.
 */

#include "gfx.h"

#if GFX_USE_GAUDIN

/* Include the driver defines */
#include "gaudin/lld/gaudin_lld.h"

#undef Red
#undef Green
#undef Blue
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>
#include <mmsystem.h>

static HWAVEIN		ah;
static volatile int	nUsedBuffers;
static WAVEHDR		*pWaveHdrs;
static HANDLE		waveInThread;
static DWORD		threadID;

/*
static void PrintWaveErrorMsg(DWORD err, TCHAR * str)
{
	#define BUFFERSIZE 128
	char	buffer[BUFFERSIZE];

	fprintf(stderr, "GAUDIN: ERROR 0x%08X: %s\r\n", err, str);
	if (mciGetErrorString(err, &buffer[0], sizeof(buffer)))
		fprintf(stderr, "%s\r\n", &buffer[0]);
	else
		fprintf(stderr, "0x%08X returned!\r\n", err);
}
*/

/**************************** waveInProc() *******************************
 * We don't use CALLBACK_FUNCTION because it is restricted to calling only
 * a few particular Windows functions, namely some of the time functions,
 * and a few of the Low Level MIDI API. If you violate this rule, your app can
 * hang inside of the callback). One of the Windows API that a callback can't
 * call is waveInAddBuffer() which is what we need to use whenever we receive a
 * MM_WIM_DATA. My callback would need to defer that job to another thread
 * anyway, so instead just use CALLBACK_THREAD here instead.
 *************************************************************************/

DWORD WINAPI waveInProc(LPVOID arg) {
	MSG		msg;
	bool_t	isRecording;
	(void)	arg;

	isRecording = FALSE;
	while (GetMessage(&msg, 0, 0, 0)) {
		switch (msg.message) {
			case MM_WIM_DATA:
				GAUDIN_ISR_CompleteI((audin_sample_t *)((WAVEHDR *)msg.lParam)->lpData, ((WAVEHDR *)msg.lParam)->dwBytesRecorded/sizeof(audin_sample_t));

				/* Are we still recording? */
				if (isRecording) {
					/* Yes. Now we need to requeue this buffer so the driver can use it for another block of audio
					 * data. NOTE: We shouldn't need to waveInPrepareHeader() a WAVEHDR that has already been prepared once.
					 * Note: We are assuming here that both the application can still access the buffer while
					 * it is on the queue.
					 */
					waveInAddBuffer(ah, (WAVEHDR *)msg.lParam, sizeof(WAVEHDR));

				} else {
					/* We aren't recording, so another WAVEHDR has been returned to us after recording has stopped.
					 * When we get all of them back, DoneAll will be equal to how many WAVEHDRs we queued
					 */
					nUsedBuffers--;
					waveInUnprepareHeader(ah, (WAVEHDR *)msg.lParam, sizeof(WAVEHDR));
				}

                break;

			case MM_WIM_OPEN:
				isRecording = TRUE;
                break;

			case MM_WIM_CLOSE:
				isRecording = FALSE;
				break;
		}
	}
	return 0;
}

/*===========================================================================*/
/* External declarations.                                                    */
/*===========================================================================*/

void gaudin_lld_init(const gaudin_params *paud) {
//	uint16_t		channel;
//	uint32_t		frequency;
//	audin_sample_t	*buffer;
//	size_t			bufcount;
//	size_t			samplesPerEvent;
	WAVEFORMATEX	wfx;
	size_t			spaceleft;
	audin_sample_t	*p;
	WAVEHDR			*phdr;
	size_t			nBuffers;
	size_t			sz;

	if (!waveInThread) {
		if (!(waveInThread = CreateThread(0, 0, (LPTHREAD_START_ROUTINE)waveInProc, 0, 0, &threadID))) {
			fprintf(stderr, "GAUDIN: Can't create WAVE recording thread\n");
			return;
		}
		CloseHandle(waveInThread);
	}

	nUsedBuffers = 0;

	wfx.wFormatTag = WAVE_FORMAT_PCM;
	wfx.nChannels = paud->channel == GAUDIN_STEREO ? 2 : 1;
	wfx.nSamplesPerSec = paud->frequency;
	wfx.nBlockAlign = wfx.nChannels * sizeof(audin_sample_t);
	wfx.nAvgBytesPerSec = wfx.nSamplesPerSec * wfx.nBlockAlign;
	wfx.wBitsPerSample = sizeof(audin_sample_t) * 8;
	wfx.cbSize = 0;

	if (waveInOpen(&ah, WAVE_MAPPER, &wfx, (DWORD_PTR)threadID, (DWORD_PTR)paud, CALLBACK_THREAD)) {
		fprintf(stderr, "GAUDIN: Can't open WAVE recording device\n");
		return;
	}

	/* We need to allocate a wave header for each buffer */
	nBuffers = (paud->bufcount + paud->samplesPerEvent - 1) / paud->samplesPerEvent;
	if (!(pWaveHdrs = gfxAlloc(nBuffers * sizeof(WAVEHDR)))) {
		fprintf(stderr, "GAUDIN: Buffer header allocation failed\n");
		return;
	}

	/* Prepare each buffer and send to the wavein device */
	spaceleft = paud->bufcount;
	for(p = paud->buffer, phdr = pWaveHdrs, spaceleft = paud->bufcount; spaceleft; p += sz, phdr++, spaceleft -= sz) {
		sz = spaceleft > paud->samplesPerEvent ? paud->samplesPerEvent : spaceleft;
		phdr->dwBufferLength = sz * sizeof(audin_sample_t);
		phdr->lpData = (LPSTR)p;
		phdr->dwFlags = 0;
		if (!waveInPrepareHeader(ah, phdr, sizeof(WAVEHDR))
				&& !waveInAddBuffer(ah, phdr, sizeof(WAVEHDR)))
			nUsedBuffers++;
		else
			fprintf(stderr, "GAUDIN: Buffer prepare failed\n");
	}
	if (!nUsedBuffers)
		fprintf(stderr, "GAUDIN: Failed to prepare any buffers\n");
}

void gadc_lld_start(void) {
	if (nUsedBuffers)
		waveInStart(ah);
}

void gadc_lld_stop(void) {
	waveInReset(ah);
	while(nUsedBuffers) Sleep(1);
	if (pWaveHdrs) {
		gfxFree(pWaveHdrs);
		pWaveHdrs = 0;
	}
}

#endif /* GFX_USE_GAUDIN */
