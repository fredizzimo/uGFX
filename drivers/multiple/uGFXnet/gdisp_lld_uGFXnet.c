/*
 * This file is subject to the terms of the GFX License. If a copy of
 * the license was not distributed with this file, you can obtain one at:
 *
 *              http://ugfx.org/license.html
 */

/**
 * @file    drivers/multiple/Win32/gdisp_lld.c
 * @brief   GDISP Graphics Driver subsystem low level driver source for uGFX network display.
 */
#include "gfx.h"

#if GFX_USE_GDISP

#define GDISP_DRIVER_VMT			GDISPVMT_uGFXnet
#include "../drivers/multiple/uGFXnet/gdisp_lld_config.h"
#include "gdisp/lld/gdisp_lld.h"

#ifndef GDISP_SCREEN_WIDTH
	#define GDISP_SCREEN_WIDTH	640
#endif
#ifndef GDISP_SCREEN_HEIGHT
	#define GDISP_SCREEN_HEIGHT	480
#endif
#ifndef GDISP_GFXNET_PORT
	#define GDISP_GFXNET_PORT	13001
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

#define GDISP_FLG_HASMOUSE			(GDISP_FLG_DRIVER<<0)
#define GDISP_FLG_CONNECTED			(GDISP_FLG_DRIVER<<1)
#define GDISP_FLG_HAVEDATA			(GDISP_FLG_DRIVER<<2)

#if GINPUT_NEED_MOUSE
	/* Include mouse support code */
	#include "ginput/lld/mouse.h"
#endif

/*===========================================================================*/
/* Driver local routines    .                                                */
/*===========================================================================*/

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

#define GNETCODE_VERSION		0x0100		// V1.0

typedef struct netPriv {
	SOCKET			netfd;					// The current socket
	unsigned		databytes;				// How many bytes have been read
	uint16_t		data[2];				// Buffer for storing data read.
	#if GINPUT_NEED_MOUSE
		coord_t		mousex, mousey;
		uint16_t	mousebuttons;
	#endif
} netPriv;

static gfxThreadHandle	hThread;
static GDisplay *		mouseDisplay;

static DECLARE_THREAD_STACK(waNetThread, 512);
static DECLARE_THREAD_FUNCTION(NetThread, param) {
	SOCKET				listenfd, fdmax, i, clientfd;
	int					len;
	unsigned			disp;
	fd_set				master, read_fds;
    struct sockaddr_in	serv_addr;
    struct sockaddr_in	clientaddr;
    GDisplay *			g;
    netPriv *			priv;
	(void)param;

	// Start the sockets layer
	StartSockets();

	/* clear the master and temp sets */
	FD_ZERO(&master);
	FD_ZERO(&read_fds);

	if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) == (SOCKET)-1)
		gfxHalt("GDISP: uGFXnet - Socket failed");

	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	serv_addr.sin_port = htons(GDISP_GFXNET_PORT);

	if (bind(listenfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) == -1)
		gfxHalt("GDISP: uGFXnet - Bind failed");

    if (listen(listenfd, 10) == -1)
		gfxHalt("GDISP: uGFXnet - Listen failed");


    /* add the listener to the master set */
    FD_SET(listenfd, &master);

    /* keep track of the biggest file descriptor */
    fdmax = listenfd; /* so far, it's this one*/

    /* loop */
    for(;;) {
		/* copy it */
		read_fds = master;
		if (select(fdmax+1, &read_fds, 0, 0, 0) == -1)
			gfxHalt("GDISP: uGFXnet - Select failed");

		// Run through the existing connections looking for data to be read
		for(i = 0; i <= fdmax; i++) {
			if(!FD_ISSET(i, &read_fds))
				continue;

			// Handle new connections
			if(i == listenfd) {
				len = sizeof(clientaddr);
				if((clientfd = accept(listenfd, (struct sockaddr *)&clientaddr, &len)) == (SOCKET)-1)
					gfxHalt("GDISP: uGFXnet - Accept failed");

				// Look for a display that isn't connected
				for(disp = 0; disp < GDISP_TOTAL_DISPLAYS; disp++) {
					if (!(g = gdispGetDisplay(disp)))
						continue;
					#if GDISP_TOTAL_CONTROLLERS > 1
						// Ignore displays for other controllers
						if (g->vmt != &GDISPVMT_uGFXnet)
							continue;
					#endif
					if (!(g->flags & GDISP_FLG_CONNECTED))
						break;
				}

				// Was anything found?
				if (disp >= GDISP_TOTAL_DISPLAYS) {
					// No Just close the connection
					closesocket(clientfd);
					//printf(New connection from %s on socket %d rejected as all displays are already connected\n", inet_ntoa(clientaddr.sin_addr), clientfd);
					continue;
				}

				// Save the descriptor
				FD_SET(clientfd, &master);
				if (clientfd > fdmax) fdmax = clientfd;
				priv = g->priv;
				memset(priv, 0, sizeof(netPriv));
				priv->netfd = clientfd;
				g->flags |= GDISP_FLG_CONNECTED;
				//printf(New connection from %s on socket %d allocated to display %u\n", inet_ntoa(clientaddr.sin_addr), clientfd, disp+1);

				// Send the initialisation data (2 words at a time)
				priv->data[0] = htons(GNETCODE_INIT);
				priv->data[1] = htons(GNETCODE_VERSION);
				send(clientfd, (const char *)priv->data, 4, 0);
				priv->data[0] = htons(GDISP_SCREEN_WIDTH);
				priv->data[1] = htons(GDISP_SCREEN_HEIGHT);
				send(clientfd, (const char *)priv->data, 4, 0);
				priv->data[0] = htons(GDISP_LLD_PIXELFORMAT);
				priv->data[1] = htons((g->flags & GDISP_FLG_HASMOUSE) ? 1 : 0);
				send(clientfd, (const char *)priv->data, 4, 0);
				continue;
			}

			// Handle data from a client

			// Look for a display that is connected and the socket descriptor matches
			for(disp = 0; disp < GDISP_TOTAL_DISPLAYS; disp++) {
				if (!(g = gdispGetDisplay(disp)))
					continue;
				#if GDISP_TOTAL_CONTROLLERS > 1
				// Ignore displays for other controllers
					if (g->vmt != &GDISPVMT_uGFXnet)
						continue;
				#endif
				priv = g->priv;
				if ((g->flags & GDISP_FLG_CONNECTED) && priv->netfd == i)
					break;
			}
			if (disp >= GDISP_TOTAL_DISPLAYS)
				gfxHalt("GDISP: uGFXnet - Got data from unrecognized connection");

			if ((g->flags & GDISP_FLG_HAVEDATA)) {
				// The higher level is still processing the previous data.
				//	Give it a chance to run by coming back to this data.
				gfxSleepMilliseconds(1);
				continue;
			}

			/* handle data from a client */
			if ((len = recv(i, ((char *)priv->data)+priv->databytes, sizeof(priv->data)-priv->databytes, 0)) <= 0) {
				// Socket closed or in error state
				g->flags &= ~GDISP_FLG_CONNECTED;
				memset(priv, 0, sizeof(netPriv));
				closesocket(i);
				FD_CLR(i, &master);
				continue;
			}

			// Do we have a full reply yet
			if (priv->databytes < sizeof(priv->data))
				continue;

			// Process the data received
			switch(priv->data[0]) {
			#if GINPUT_NEED_MOUSE
				case GNETCODE_MOUSE_X:		priv->mousex = priv->data[1];			break;
				case GNETCODE_MOUSE_Y:		priv->mousey = priv->data[1];			break;
				case GNETCODE_MOUSE_B:		priv->mousebuttons = priv->data[1];		break;
			#endif
			case GNETCODE_CONTROL:
			case GNETCODE_READ:
				priv->databytes = 0;
				g->flags |= GDISP_FLG_HAVEDATA;
				break;
			case GNETCODE_KILL:
				gfxHalt("GDISP: uGFXnet - Display sent KILL command");
				break;

			default:
				// Just ignore unrecognised data
				break;
			}
		}
	}
    return 0;
}

/*===========================================================================*/
/* Driver exported functions.                                                */
/*===========================================================================*/

LLDSPEC bool_t gdisp_lld_init(GDisplay *g) {
	netPriv	*	priv;

	// Initialise the receiver thread (if it hasn't been done already)
	if (!hThread) {
		hThread = gfxThreadCreate(waNetThread, sizeof(waNetThread), HIGH_PRIORITY, NetThread, 0);
		gfxThreadClose(hThread);
	}

	// Only turn on mouse on the first window for now
	#if GINPUT_NEED_MOUSE
		if (!g->controllerdisplay) {
			mouseDisplay = g;
			g->flags |= GDISP_FLG_HASMOUSE;
		}
	#endif

	// Create a private area for this window
	if (!(priv = (netPriv *)gfxAlloc(sizeof(netPriv))))
		gfxHalt("GDISP: uGFXnet - Memory allocation failed");
	memset(priv, 0, sizeof(netPriv));
	g->priv = priv;
	g->board = 0;			// no board interface for this controller

	// Initialise the GDISP structure
	g->g.Orientation = GDISP_ROTATE_0;
	g->g.Powermode = powerOn;
	g->g.Backlight = 100;
	g->g.Contrast = 50;
	g->g.Width = GDISP_SCREEN_WIDTH;
	g->g.Height = GDISP_SCREEN_HEIGHT;

	return TRUE;
}

#if GDISP_HARDWARE_FLUSH
	LLDSPEC void gdisp_lld_flush(GDisplay *g) {
		netPriv	*	priv;
		uint16_t	buf[1];

		if (!(g->flags & GDISP_FLG_CONNECTED))
			return;

		priv = g->priv;
		buf[0] = htons(GNETCODE_FLUSH);
		send(priv->netfd, (const char *)buf, sizeof(buf), 0);
	}
#endif

#if GDISP_HARDWARE_DRAWPIXEL
	LLDSPEC void gdisp_lld_draw_pixel(GDisplay *g) {
		netPriv	*	priv;
		uint16_t	buf[4];

		if (!(g->flags & GDISP_FLG_CONNECTED))
			return;

		priv = g->priv;
		buf[0] = htons(GNETCODE_PIXEL);
		buf[1] = htons(g->p.x);
		buf[2] = htons(g->p.y);
		buf[3] = htons(COLOR2NATIVE(g->p.color));
		send(priv->netfd, (const char *)buf, sizeof(buf), 0);
	}
#endif

/* ---- Optional Routines ---- */

#if GDISP_HARDWARE_FILLS
	LLDSPEC void gdisp_lld_fill_area(GDisplay *g) {
		netPriv	*	priv;
		uint16_t	buf[6];

		if (!(g->flags & GDISP_FLG_CONNECTED))
			return;

		priv = g->priv;
		buf[0] = htons(GNETCODE_FILL);
		buf[1] = htons(g->p.x);
		buf[2] = htons(g->p.y);
		buf[3] = htons(g->p.cx);
		buf[4] = htons(g->p.cy);
		buf[5] = htons(COLOR2NATIVE(g->p.color));
		send(priv->netfd, (const char *)buf, sizeof(buf), 0);
	}
#endif

#if GDISP_HARDWARE_BITFILLS
	LLDSPEC void gdisp_lld_blit_area(GDisplay *g) {
		netPriv	*	priv;
		pixel_t	*	buffer;
		uint16_t	buf[5];
		coord_t		x, y;

		if (!(g->flags & GDISP_FLG_CONNECTED))
			return;

		// Make everything relative to the start of the line
		buffer = g->p.ptr;
		buffer += g->p.x2*g->p.y1;
		
		priv = g->priv;
		buf[0] = htons(GNETCODE_BLIT);
		buf[1] = htons(g->p.x);
		buf[2] = htons(g->p.y);
		buf[3] = htons(g->p.cx);
		buf[4] = htons(g->p.cy);
		send(priv->netfd, (const char *)buf, sizeof(buf), 0);

		for(y = 0; y < g->p.cy; y++, buffer += g->p.x2 - g->p.cx) {
			for(x = 0; x < g->p.cx; x++, buffer++) {
				buf[0] = COLOR2NATIVE(buffer[0]);
				send(priv->netfd, (const char *)buf, sizeof(buf[0]), 0);
			}
		}
	}
#endif

#if GDISP_HARDWARE_PIXELREAD
	LLDSPEC	color_t gdisp_lld_get_pixel_color(GDisplay *g) {
		netPriv	*	priv;
		uint16_t	buf[3];
		color_t		data;

		if (!(g->flags & GDISP_FLG_CONNECTED))
			return 0;

		priv = g->priv;
		buf[0] = htons(GNETCODE_READ);
		buf[1] = htons(g->p.x);
		buf[2] = htons(g->p.y);
		send(priv->netfd, (const char *)buf, sizeof(buf), 0);

		// Now wait for a reply
		while(!(g->flags & GDISP_FLG_HAVEDATA) || priv->data[0] != GNETCODE_READ)
			gfxSleepMilliseconds(1);
		
		data = NATIVE2COLOR(priv->data[1]);
		g->flags &= ~GDISP_FLG_HAVEDATA;

		return data;
	}
#endif

#if GDISP_NEED_SCROLL && GDISP_HARDWARE_SCROLL
	LLDSPEC void gdisp_lld_vertical_scroll(GDisplay *g) {
		netPriv	*	priv;
		uint16_t	buf[6];

		if (!(g->flags & GDISP_FLG_CONNECTED))
			return;

		priv = g->priv;
		buf[0] = htons(GNETCODE_FILL);
		buf[1] = htons(g->p.x);
		buf[2] = htons(g->p.y);
		buf[3] = htons(g->p.cx);
		buf[4] = htons(g->p.cy);
		buf[5] = htons(g->p.y1);
		send(priv->netfd, (const char *)buf, sizeof(buf), 0);
	}
#endif

#if GDISP_NEED_CONTROL && GDISP_HARDWARE_CONTROL
	LLDSPEC void gdisp_lld_control(GDisplay *g) {
		netPriv	*	priv;
		uint16_t	buf[3];
		bool_t		allgood;

		if (!(g->flags & GDISP_FLG_CONNECTED))
			return;

		// Check if we might support the code
		switch(g->p.x) {
		case GDISP_CONTROL_ORIENTATION:
			if (g->g.Orientation == (orientation_t)g->p.ptr)
				return;
			break;
		case GDISP_CONTROL_POWER:
			if (g->g.Powermode == (powermode_t)g->p.ptr)
				return;
			break;
		case GDISP_CONTROL_BACKLIGHT:
			if (g->g.Backlight == (uint16_t)(int)g->p.ptr)
				return;
			if ((uint16_t)(int)g->p.ptr > 100)
				g->p.ptr = (void *)100;
			break;
		default:
			return;
		}

		// Send the command
		priv = g->priv;
		buf[0] = htons(GNETCODE_CONTROL);
		buf[1] = htons(g->p.x);
		buf[2] = htons((uint16_t)(int)g->p.ptr);
		send(priv->netfd, (const char *)buf, sizeof(buf), 0);

		// Now wait for a reply
		while(!(g->flags & GDISP_FLG_HAVEDATA) || priv->data[0] != GNETCODE_CONTROL)
			gfxSleepMilliseconds(1);

		// Extract the return status
		allgood = priv->data[1] ? TRUE : FALSE;
		g->flags &= ~GDISP_FLG_HAVEDATA;

		// Do nothing more if the operation failed
		if (!allgood) return;

		// Update the local stuff
		switch(g->p.x) {
		case GDISP_CONTROL_ORIENTATION:
			switch((orientation_t)g->p.ptr) {
				case GDISP_ROTATE_0:
				case GDISP_ROTATE_180:
					g->g.Width = GDISP_SCREEN_WIDTH;
					g->g.Height = GDISP_SCREEN_HEIGHT;
					break;
				case GDISP_ROTATE_90:
				case GDISP_ROTATE_270:
					g->g.Height = GDISP_SCREEN_WIDTH;
					g->g.Width = GDISP_SCREEN_HEIGHT;
					break;
				default:
					return;
			}
			g->g.Orientation = (orientation_t)g->p.ptr;
			break;
		case GDISP_CONTROL_POWER:
			g->g.Powermode = (powermode_t)g->p.ptr;
			break;
		case GDISP_CONTROL_BACKLIGHT:
			g->g.Backlight = (uint16_t)(int)g->p.ptr;
			break;
		}
	}
#endif

#if GINPUT_NEED_MOUSE
	void ginput_lld_mouse_init(void) {}
	void ginput_lld_mouse_get_reading(MouseReading *pt) {
		GDisplay *	g;
		netPriv	*	priv;

		g = mouseDisplay;
		priv = g->priv;

		pt->x = priv->mousex;
		pt->y = priv->mousey;
		pt->z = (priv->mousebuttons & GINPUT_MOUSE_BTN_LEFT) ? 100 : 0;
		pt->buttons = priv->mousebuttons;
	}
#endif /* GINPUT_NEED_MOUSE */

#endif /* GFX_USE_GDISP */

