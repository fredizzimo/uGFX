/*
 * This file is subject to the terms of the GFX License. If a copy of
 * the license was not distributed with this file, you can obtain one at:
 *
 *              http://ugfx.org/license.html
 */

#include "gfx.h"

#ifndef GDISP_GFXNET_PORT
	#define GDISP_GFXNET_PORT	13001
#endif
#ifndef GDISP_GFXNET_HOST
	#define GDISP_GFXNET_HOST	"127.0.0.1"
#endif

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#if defined(WIN32) || GFX_USE_OS_WIN32
	#include <winsock.h>

	static void StopSockets(void) {
		WSACleanup();
	}
	static void StartSockets(void) {
		WSADATA wsaData;
		if (WSAStartup(MAKEWORD(2,2), &wsaData) != 0)
			gfxHalt("GDISP: uGFXnet - WSAStartup failed");
		atexit(StopSockets);
	}

#else
	#include <sys/types.h>
	#include <sys/socket.h>
	#include <netinet/in.h>
	#include <arpa/inet.h>
	#include <netdb.h>

	#define closesocket(fd)			close(fd)
	#define ioctlsocket(fd,cmd,arg)	ioctl(fd,cmd,arg)
	#define StartSockets()
	#ifndef SOCKET
		#define SOCKET	int
	#endif
#endif

#define GNETCODE_VERSION		0x0100		// V1.0

// All commands are sent in 16 bit blocks (2 bytes) in network order (BigEndian)
#define GNETCODE_INIT			0xFFFF		// Followed by version,width,height,pixelformat,hasmouse
#define GNETCODE_FLUSH			0x0000		// No following data
#define GNETCODE_PIXEL			0x0001		// Followed by x,y,color
#define GNETCODE_FILL			0x0002		// Followed by x,y,cx,cy,color
#define GNETCODE_BLIT			0x0003		// Followed by x,y,cx,cy,bits
#define GNETCODE_READ			0x0004		// Followed by x,y - Response is 0x0004,color
#define GNETCODE_SCROLL			0x0005		// Followed by x,y,cx,cy,lines
#define GNETCODE_CONTROL		0x0006		// Followed by what,data - Response is 0x0006,0x0000 (fail) or 0x0006,0x0001 (success)
#define GNETCODE_MOUSE_X		0x0007		// This is only ever received - never sent. Response is 0x0007,x
#define GNETCODE_MOUSE_Y		0x0008		// This is only ever received - never sent. Response is 0x0008,y
#define GNETCODE_MOUSE_B		0x0009		// This is only ever received - never sent. Response is 0x0009,buttons
#define GNETCODE_KILL			0xFFFE		// This is only ever received - never sent. Response is 0xFFFE,retcode


static GListener				gl;
static SOCKET					netfd = (SOCKET)-1;

static DECLARE_THREAD_STACK(waNetThread, 512);
static DECLARE_THREAD_FUNCTION(NetThread, param) {
	GEventMouse				*pem;
	uint16_t				cmd[2];
	uint16_t				lbuttons;
	coord_t					lx, ly;

	// Initialize the mouse and the listener.
	geventListenerInit(&gl);
	geventAttachSource(&gl, ginputGetMouse(0), GLISTEN_MOUSEDOWNMOVES|GLISTEN_MOUSEMETA);
	lbuttons = 0;
	lx = ly = -1;

	while(1) {
		// Get a (mouse) event
		pem = (GEventMouse *)geventEventWait(&gl, TIME_INFINITE);
		if (pem->type != GEVENT_MOUSE && pem->type != GEVENT_TOUCH)
			continue;

		// Nothing to do if the socket is not open
		if (netfd == (SOCKET)-1)
			continue;

		// Transfer mouse data that has changed
		if (lx != pem->x) {
			lx = pem->x;
			cmd[0] = htons(GNETCODE_MOUSE_X);
			cmd[1] = htons(lx);
			send(netfd, cmd, sizeof(cmd), 0);
		}
		if (ly != pem->y) {
			ly = pem->y;
			cmd[0] = htons(GNETCODE_MOUSE_Y);
			cmd[1] = htons(ly);
			send(netfd, cmd, sizeof(cmd), 0);
		}
		if (lbuttons != pem->current_buttons) {
			lbuttons = pem->current_buttons;
			cmd[0] = htons(GNETCODE_MOUSE_B);
			cmd[1] = htons(lbuttons);
			send(netfd, cmd, sizeof(cmd), 0);
		}
	}
}

int main(void) {
	coord_t		width, height;
	font_t		font;
	uint16_t	cmd[2];

    // Initialize and clear the display
    gfxInit();
	font = gdispOpenFont("UI2");

	// Open the connection
	// TODO

	// Start the mouse thread
	// TODO

	// Process incoming instructions
	// TODO

}

