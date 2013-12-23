// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// $Id:$
//
// Copyright (C) 1993-1996 by id Software, Inc.
//
// This source is available for distribution and/or modification
// only under the terms of the DOOM Source Code License as
// published by id Software. All rights reserved.
//
// The source is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// FITNESS FOR A PARTICULAR PURPOSE. See the DOOM Source Code License
// for more details.
//
// $Log:$
//
// DESCRIPTION:
//
//-----------------------------------------------------------------------------

#include "gfx.h"

#include "doomdef.h"
#include "m_misc.h"
#include "i_video.h"
#include "i_sound.h"

#include "d_net.h"
#include "g_game.h"

#ifdef __GNUG__
#pragma implementation "i_system.h"
#endif
#include "i_system.h"




int	mb_used = 6;


void
I_Tactile
( int	on,
  int	off,
  int	total )
{
  // UNUSED.
  on = off = total = 0;
}

ticcmd_t	emptycmd;
ticcmd_t*	I_BaseTiccmd(void)
{
    return &emptycmd;
}


int  I_GetHeapSize (void)
{
    return mb_used*1024*1024;
}

byte* I_ZoneBase (int*	size)
{
    *size = mb_used*1024*1024;
    return (byte *) gfxAlloc (*size);
}



//
// I_GetTime
// returns time in 1/70th second tics
//
int  I_GetTime (void)
{
	systemticks_t	tdiv;

	tdiv = gfxMillisecondsToTicks(1000*256/TICRATE);
	return (gfxSystemTicks()<<8)/tdiv;
}



//
// I_Init
//
void I_Init (void)
{
    I_InitSound();
    //  I_InitGraphics();
}

//
// I_Quit
//
void I_Quit (void)
{
    D_QuitNetGame ();
    I_ShutdownSound();
    I_ShutdownMusic();
    M_SaveDefaults ();
    I_ShutdownGraphics();
    gfxExit();
}

void I_WaitVBL(int count)
{
    gfxSleepMilliseconds(1000/TICRATE);
}

void I_BeginRead(void)
{
}

void I_EndRead(void)
{
}

byte*	I_AllocLow(int length)
{
    byte*	mem;
        
    mem = (byte *)gfxAlloc (length);
    memset (mem,0,length);
    return mem;
}


//
// I_Error
//
extern boolean demorecording;

void I_Error (char *error, ...)
{
#if 0
    va_list	argptr;

    // Message first.
    va_start (argptr,error);
    fprintf (stderr, "Error: ");
    vfprintf (stderr,error,argptr);
    fprintf (stderr, "\n");
    va_end (argptr);

    fflush( stderr );
#endif

    // Shutdown. Here might be other errors.
    if (demorecording)
	G_CheckDemoStatus();

    D_QuitNetGame ();
    I_ShutdownGraphics();
    
    gfxHalt(error);
}

#include "doom1_wad.h"

#define F1NAME		"doom1.wad"
#define	F1SIZE		4196020
#define F1HANDLE	313			// Some random number > 2
static size_t		f1pos;

int I_HaveFile(char *fname) {
	return !strcmp(fname, F1NAME);
}

int I_FileSize(int handle) {
	return handle == F1HANDLE ? F1SIZE : 0;
}

int I_FileRead(int handle, char *buf, int len) {
	if (handle != F1HANDLE || len <= 0) return 0;
	if (f1pos + len > F1SIZE)
		len = F1SIZE - f1pos;
	memcpy(buf, doom1_wad+f1pos, len);
	return len;
}
void I_FilePos(int handle, int pos) {
	if (handle != F1HANDLE) return;
	if (pos > F1SIZE)
		pos = F1SIZE;
	f1pos = pos;
}
int I_FileOpenRead(char *fname) {
	return strcmp(fname, F1NAME) ? -1 : F1HANDLE;
}
void I_FileClose(int handle) {
}
void *I_Realloc(void *p, int nsize) {
	return gfxRealloc(p, 0 /* Oops - we don't know this */, nsize);
}
