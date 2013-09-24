/*
 * This file is subject to the terms of the GFX License. If a copy of
 * the license was not distributed with this file, you can obtain one at:
 *
 *              http://ugfx.org/license.html
 */

/**
 * @file    drivers/multiple/Win32/gdisp_lld.c
 * @brief   GDISP Graphics Driver subsystem low level driver source for Win32.
 */

#include "gfx.h"

#if GFX_USE_GDISP

#define GDISP_LLD_DECLARATIONS
#include "gdisp/lld/gdisp_lld.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <windows.h>
#include <wingdi.h>
#include <assert.h>

#ifndef GDISP_SCREEN_WIDTH
	#define GDISP_SCREEN_WIDTH	640
#endif
#ifndef GDISP_SCREEN_HEIGHT
	#define GDISP_SCREEN_HEIGHT	480
#endif

#if GINPUT_NEED_TOGGLE
	/* Include toggle support code */
	#include "ginput/lld/toggle.h"
#endif

#if GINPUT_NEED_MOUSE
	/* Include mouse support code */
	#include "ginput/lld/mouse.h"
#endif

/*===========================================================================*/
/* Driver local routines    .                                                */
/*===========================================================================*/

#define WIN32_USE_MSG_REDRAW	FALSE
#if GINPUT_NEED_TOGGLE
	#define WIN32_BUTTON_AREA		16
#else
	#define WIN32_BUTTON_AREA		0
#endif

#define APP_NAME "GDISP"

#define COLOR2BGR(c)	((((c) & 0xFF)<<16)|((c) & 0xFF00)|(((c)>>16) & 0xFF))
#define BGR2COLOR(c)	COLOR2BGR(c)

static HWND winRootWindow = NULL;
static HDC dcBuffer = NULL;
static HBITMAP dcBitmap = NULL;
static HBITMAP dcOldBitmap;
static volatile bool_t isReady = FALSE;
static coord_t	wWidth, wHeight;

#if GINPUT_NEED_MOUSE
	static coord_t	mousex, mousey;
	static uint16_t	mousebuttons;
#endif
#if GINPUT_NEED_TOGGLE
	static uint8_t	toggles = 0;
#endif

static LRESULT myWindowProc(HWND hWnd,	UINT Msg, WPARAM wParam, LPARAM lParam)
{
	HDC			dc;
	PAINTSTRUCT	ps;
	#if GINPUT_NEED_TOGGLE
		HBRUSH		hbrOn, hbrOff;
		HPEN		pen;
		RECT		rect;
		HGDIOBJ		old;
		POINT 		p;
		coord_t		pos;
		uint8_t		bit;
	#endif

	switch (Msg) {
	case WM_CREATE:
		break;
	case WM_LBUTTONDOWN:
		#if GINPUT_NEED_MOUSE
			if ((coord_t)HIWORD(lParam) < wHeight) {
				mousebuttons |= GINPUT_MOUSE_BTN_LEFT;
				goto mousemove;
			}
		#endif
		#if GINPUT_NEED_TOGGLE
			bit = 1 << ((coord_t)LOWORD(lParam)*8/wWidth);
			toggles ^= bit;
			rect.left = 0;
			rect.right = wWidth;
			rect.top = wHeight;
			rect.bottom = wHeight + WIN32_BUTTON_AREA;
			InvalidateRect(hWnd, &rect, FALSE);
			UpdateWindow(hWnd);
			#if GINPUT_TOGGLE_POLL_PERIOD == TIME_INFINITE
				ginputToggleWakeup();
			#endif
		#endif
		break;
	case WM_LBUTTONUP:
		#if GINPUT_NEED_TOGGLE
			if ((toggles & 0x0F)) {
				toggles &= ~0x0F;
				rect.left = 0;
				rect.right = wWidth;
				rect.top = wHeight;
				rect.bottom = wHeight + WIN32_BUTTON_AREA;
				InvalidateRect(hWnd, &rect, FALSE);
				UpdateWindow(hWnd);
				#if GINPUT_TOGGLE_POLL_PERIOD == TIME_INFINITE
					ginputToggleWakeup();
				#endif
			}
		#endif
		#if GINPUT_NEED_MOUSE
			if ((coord_t)HIWORD(lParam) < wHeight) {
				mousebuttons &= ~GINPUT_MOUSE_BTN_LEFT;
				goto mousemove;
			}
		#endif
		break;
#if GINPUT_NEED_MOUSE
	case WM_MBUTTONDOWN:
		if ((coord_t)HIWORD(lParam) < wHeight) {
			mousebuttons |= GINPUT_MOUSE_BTN_MIDDLE;
			goto mousemove;
		}
		break;
	case WM_MBUTTONUP:
		if ((coord_t)HIWORD(lParam) < wHeight) {
			mousebuttons &= ~GINPUT_MOUSE_BTN_MIDDLE;
			goto mousemove;
		}
		break;
	case WM_RBUTTONDOWN:
		if ((coord_t)HIWORD(lParam) < wHeight) {
			mousebuttons |= GINPUT_MOUSE_BTN_RIGHT;
			goto mousemove;
		}
		break;
	case WM_RBUTTONUP:
		if ((coord_t)HIWORD(lParam) < wHeight) {
			mousebuttons &= ~GINPUT_MOUSE_BTN_RIGHT;
			goto mousemove;
		}
		break;
	case WM_MOUSEMOVE:
		if ((coord_t)HIWORD(lParam) >= wHeight)
			break;
	mousemove:
		mousex = (coord_t)LOWORD(lParam); 
		mousey = (coord_t)HIWORD(lParam); 
		#if GINPUT_MOUSE_POLL_PERIOD == TIME_INFINITE
			ginputMouseWakeup();
		#endif
		break;
#endif
	case WM_SYSKEYDOWN:
	case WM_KEYDOWN:
	case WM_SYSKEYUP:
	case WM_KEYUP:
		break;
	case WM_CHAR:
	case WM_DEADCHAR:
	case WM_SYSCHAR:
	case WM_SYSDEADCHAR:
		break;
	case WM_PAINT:
		dc = BeginPaint(hWnd, &ps);
		BitBlt(dc, ps.rcPaint.left, ps.rcPaint.top,
			ps.rcPaint.right - ps.rcPaint.left,
			(ps.rcPaint.bottom > wHeight ? wHeight : ps.rcPaint.bottom) - ps.rcPaint.top,
			dcBuffer, ps.rcPaint.left, ps.rcPaint.top, SRCCOPY);
		#if GINPUT_NEED_TOGGLE
			if (ps.rcPaint.bottom >= wHeight) {
				pen = CreatePen(PS_SOLID, 1, COLOR2BGR(Black));
				hbrOn = CreateSolidBrush(COLOR2BGR(Blue));
				hbrOff = CreateSolidBrush(COLOR2BGR(Gray));
				old = SelectObject(dc, pen);
				MoveToEx(dc, 0, wHeight, &p);
				LineTo(dc, wWidth, wHeight);
				for(pos = 0, bit=1; pos < wWidth; pos=rect.right, bit <<= 1) {
					rect.left = pos;
					rect.right = pos + wWidth/8;
					rect.top = wHeight;
					rect.bottom = wHeight + WIN32_BUTTON_AREA;
					FillRect(dc, &rect, (toggles & bit) ? hbrOn : hbrOff);
					if (pos > 0) {
						MoveToEx(dc, rect.left, rect.top, &p);
						LineTo(dc, rect.left, rect.bottom);
					}
				}
				DeleteObject(hbrOn);
				DeleteObject(hbrOff);
				SelectObject(dc, old);
			}
		#endif
		EndPaint(hWnd, &ps);
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		SelectObject(dcBuffer, dcOldBitmap);
		DeleteDC(dcBuffer);
		DeleteObject(dcBitmap);
		winRootWindow = NULL;
		break;
	default:
		return DefWindowProc(hWnd, Msg, wParam, lParam);
	}
	return 0;
}

static void InitWindow(void) {
	HANDLE hInstance;
	WNDCLASS wc;
	RECT	rect;
	HDC		dc;

	hInstance = GetModuleHandle(NULL);

	wc.style           = CS_HREDRAW | CS_VREDRAW; // | CS_OWNDC;
	wc.lpfnWndProc     = (WNDPROC)myWindowProc;
	wc.cbClsExtra      = 0;
	wc.cbWndExtra      = 0;
	wc.hInstance       = hInstance;
	wc.hIcon           = LoadIcon(NULL, IDI_APPLICATION);
	wc.hCursor         = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground   = GetStockObject(WHITE_BRUSH);
	wc.lpszMenuName    = NULL;
	wc.lpszClassName   = APP_NAME;
	RegisterClass(&wc);

	rect.top = 0; rect.bottom = wHeight+WIN32_BUTTON_AREA;
	rect.left = 0; rect.right = wWidth;
	AdjustWindowRect(&rect, WS_OVERLAPPED|WS_CAPTION|WS_SYSMENU, 0);
	winRootWindow = CreateWindow(APP_NAME, "", WS_OVERLAPPED|WS_CAPTION|WS_SYSMENU, 0, 0,
			rect.right-rect.left, rect.bottom-rect.top, 0, 0, hInstance, NULL);
	assert(winRootWindow != NULL);


	GetClientRect(winRootWindow, &rect);
	wWidth = rect.right-rect.left;
	wHeight = rect.bottom - rect.top - WIN32_BUTTON_AREA;

	dc = GetDC(winRootWindow);
	dcBitmap = CreateCompatibleBitmap(dc, wWidth, wHeight);
	dcBuffer = CreateCompatibleDC(dc);
	ReleaseDC(winRootWindow, dc);
	dcOldBitmap = SelectObject(dcBuffer, dcBitmap);

	ShowWindow(winRootWindow, SW_SHOW);
	UpdateWindow(winRootWindow);
	isReady = TRUE;
}

static DECLARE_THREAD_STACK(waWindowThread, 1024);
static DECLARE_THREAD_FUNCTION(WindowThread, param) {
	(void)param;
	MSG msg;

	InitWindow();
	do {
		gfxSleepMilliseconds(1);
		while(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	} while (msg.message != WM_QUIT);
	ExitProcess(0);
	return msg.wParam;
}

/*===========================================================================*/
/* Driver exported functions.                                                */
/*===========================================================================*/

/**
 * @brief   Low level GDISP driver initialization.
 * @return	TRUE if successful, FALSE on error.
 *
 * @notapi
 */
LLDSPEC bool_t gdisp_lld_init(GDISPDriver *g) {
	RECT			rect;
	gfxThreadHandle	hth;

	/* Set the window dimensions */
	GetWindowRect(GetDesktopWindow(), &rect);
	wWidth = rect.right - rect.left;
	wHeight = rect.bottom - rect.top - WIN32_BUTTON_AREA;
	if (wWidth > GDISP_SCREEN_WIDTH)
		wWidth = GDISP_SCREEN_WIDTH;
	if (wHeight > GDISP_SCREEN_HEIGHT)
		wHeight = GDISP_SCREEN_HEIGHT;

	/* Initialise the window */
	if (!(hth = gfxThreadCreate(waWindowThread, sizeof(waWindowThread), HIGH_PRIORITY, WindowThread, 0))) {
		fprintf(stderr, "Cannot create window thread\n");
		exit(-1);
	}
	gfxThreadClose(hth);
	while (!isReady)
		Sleep(1);

	/* Initialise the GDISP structure to match */
	g->g.Orientation = GDISP_ROTATE_0;
	g->g.Powermode = powerOn;
	g->g.Backlight = 100;
	g->g.Contrast = 50;
	g->g.Width = wWidth;
	g->g.Height = wHeight;
	return TRUE;
}

#if GDISP_HARDWARE_DRAWPIXEL
	LLDSPEC void gdisp_lld_draw_pixel(GDISPDriver *g) {
		HDC			dcScreen;
		int			x, y;
		COLORREF	color;
	
		color = COLOR2BGR(g->p.color);
	
		#if GDISP_NEED_CONTROL
			switch(g->g.Orientation) {
			case GDISP_ROTATE_0:
				x = g->p.x;
				y = g->p.y;
				break;
			case GDISP_ROTATE_90:
				x = g->p.y;
				y = g->g.Width - 1 - g->p.x;
				break;
			case GDISP_ROTATE_180:
				x = g->g.Width - 1 - g->p.x;
				y = g->g.Height - 1 - g->p.y;
				break;
			case GDISP_ROTATE_270:
				x = g->g.Height - 1 - g->p.y;
				y = g->p.x;
				break;
			}
		#else
			x = g->p.x;
			y = g->p.y;
		#endif

		// Draw the pixel on the screen and in the buffer.
		dcScreen = GetDC(winRootWindow);
		SetPixel(dcScreen, x, y, color);
		SetPixel(dcBuffer, x, y, color);
		ReleaseDC(winRootWindow, dcScreen);
	}
#endif

/* ---- Optional Routines ---- */

#if GDISP_HARDWARE_FILLS
	LLDSPEC void gdisp_lld_fill_area(GDISPDriver *g) {
		HDC			dcScreen;
		RECT		rect;
		HBRUSH		hbr;
		COLORREF	color;

		color = COLOR2BGR(g->p.color);
		#if GDISP_NEED_CONTROL
			switch(g->g.Orientation) {
			case GDISP_ROTATE_0:
				rect.top = g->p.y;
				rect.bottom = rect.top + g->p.cy;
				rect.left = g->p.x;
				rect.right = rect.left + g->p.cx;
				break;
			case GDISP_ROTATE_90:
				rect.bottom = g->g.Width - g->p.x;
				rect.top = rect.bottom - g->p.cx;
				rect.left = g->p.y;
				rect.right = rect.left + g->p.cy;
				break;
			case GDISP_ROTATE_180:
				rect.bottom = g->g.Height - g->p.y;
				rect.top = rect.bottom - g->p.cy;
				rect.right = g->g.Width - g->p.x;
				rect.left = rect.right - g->p.cx;
				break;
			case GDISP_ROTATE_270:
				rect.top = g->p.x;
				rect.bottom = rect.top + g->p.cx;
				rect.right = g->g.Height - g->p.y;
				rect.left = rect.right - g->p.cy;
				break;
			}
		#else
			rect.top = g->p.y;
			rect.bottom = rect.top + g->p.cy;
			rect.left = g->p.x;
			rect.right = rect.left + g->p.cx;
		#endif

		hbr = CreateSolidBrush(color);

		dcScreen = GetDC(winRootWindow);
		FillRect(dcScreen, &rect, hbr);
		FillRect(dcBuffer, &rect, hbr);
		ReleaseDC(winRootWindow, dcScreen);

		DeleteObject(hbr);
	}
#endif

#if GDISP_HARDWARE_BITFILLS && GDISP_NEED_CONTROL
	static pixel_t *rotateimg(GDISPDriver *g, const pixel_t *buffer) {
		pixel_t	*dstbuf;
		pixel_t	*dst;
		const pixel_t	*src;
		size_t	sz;
		coord_t	i, j;

		// Shortcut.
		if (g->g.Orientation == GDISP_ROTATE_0 && g->p.x1 == 0 && g->p.cx == g->p.x2)
			return (pixel_t *)buffer;
		
		// Allocate the destination buffer
		sz = (size_t)g->p.cx * (size_t)g->p.cy;
		if (!(dstbuf = (pixel_t *)malloc(sz * sizeof(pixel_t))))
			return 0;
		
		// Copy the bits we need
		switch(g->g.Orientation) {
		case GDISP_ROTATE_0:
			for(dst = dstbuf, src = buffer+g->p.x1, j = 0; j < g->p.cy; j++, src += g->p.x2 - g->p.cx)
				for(i = 0; i < g->p.cx; i++)
					*dst++ = *src++;
			break;
		case GDISP_ROTATE_90:
			for(src = buffer+g->p.x1, j = 0; j < g->p.cy; j++, src += g->p.x2 - g->p.cx) {
				dst = dstbuf+sz-g->p.cy+j;
				for(i = 0; i < g->p.cx; i++, dst -= g->p.cy)
					*dst = *src++;
			}
			break;
		case GDISP_ROTATE_180:
			for(dst = dstbuf+sz, src = buffer+g->p.x1, j = 0; j < g->p.cy; j++, src += g->p.x2 - g->p.cx)
				for(i = 0; i < g->p.cx; i++)
					*--dst = *src++;
			break;
		case GDISP_ROTATE_270:
			for(src = buffer+g->p.x1, j = 0; j < g->p.cy; j++, src += g->p.x2 - g->p.cx) {
				dst = dstbuf+g->p.cy-j-1;
				for(i = 0; i < g->p.cx; i++, dst += g->p.cy)
					*dst = *src++;
			}
			break;
		}
		return dstbuf;
	}
#endif
	
#if GDISP_HARDWARE_BITFILLS
	void gdisp_lld_blit_area(GDISPDriver *g) {
		BITMAPV4HEADER bmpInfo;
		HDC			dcScreen;
		pixel_t	*	buffer;
		#if GDISP_NEED_CONTROL
			RECT		rect;
			pixel_t	*	srcimg;
		#endif

		// Make everything relative to the start of the line
		buffer = g->p.ptr;
		buffer += g->p.x2*g->p.y1;
		
		memset(&bmpInfo, 0, sizeof(bmpInfo));
		bmpInfo.bV4Size = sizeof(bmpInfo);
		bmpInfo.bV4Planes = 1;
		bmpInfo.bV4BitCount = sizeof(pixel_t)*8;
		bmpInfo.bV4AlphaMask = 0;
		bmpInfo.bV4RedMask		= RGB2COLOR(255,0,0);
		bmpInfo.bV4GreenMask	= RGB2COLOR(0,255,0);
		bmpInfo.bV4BlueMask		= RGB2COLOR(0,0,255);
		bmpInfo.bV4V4Compression = BI_BITFIELDS;
		bmpInfo.bV4XPelsPerMeter = 3078;
		bmpInfo.bV4YPelsPerMeter = 3078;
		bmpInfo.bV4ClrUsed = 0;
		bmpInfo.bV4ClrImportant = 0;
		bmpInfo.bV4CSType = 0; //LCS_sRGB;

		#if GDISP_NEED_CONTROL
			bmpInfo.bV4SizeImage = (g->p.cy*g->p.cx) * sizeof(pixel_t);
			srcimg = rotateimg(g, buffer);
			if (!srcimg) return;
			
			switch(g->g.Orientation) {
			case GDISP_ROTATE_0:
				bmpInfo.bV4Width = g->p.cx;
				bmpInfo.bV4Height = -g->p.cy; /* top-down image */
				rect.top = g->p.y;
				rect.bottom = rect.top+g->p.cy;
				rect.left = g->p.x;
				rect.right = rect.left+g->p.cx;
				break;
			case GDISP_ROTATE_90:
				bmpInfo.bV4Width = g->p.cy;
				bmpInfo.bV4Height = -g->p.cx; /* top-down image */
				rect.bottom = g->g.Width - g->p.x;
				rect.top = rect.bottom-g->p.cx;
				rect.left = g->p.y;
				rect.right = rect.left+g->p.cy;
				break;
			case GDISP_ROTATE_180:
				bmpInfo.bV4Width = g->p.cx;
				bmpInfo.bV4Height = -g->p.cy; /* top-down image */
				rect.bottom = g->g.Height-1 - g->p.y;
				rect.top = rect.bottom-g->p.cy;
				rect.right = g->g.Width - g->p.x;
				rect.left = rect.right-g->p.cx;
				break;
			case GDISP_ROTATE_270:
				bmpInfo.bV4Width = g->p.cy;
				bmpInfo.bV4Height = -g->p.cx; /* top-down image */
				rect.top = g->p.x;
				rect.bottom = rect.top+g->p.cx;
				rect.right = g->g.Height - g->p.y;
				rect.left = rect.right-g->p.cy;
				break;
			}
			dcScreen = GetDC(winRootWindow);
			SetDIBitsToDevice(dcBuffer, rect.left, rect.top, rect.right-rect.left, rect.bottom-rect.top, 0, 0, 0, rect.bottom-rect.top, srcimg, (BITMAPINFO*)&bmpInfo, DIB_RGB_COLORS);
			SetDIBitsToDevice(dcScreen, rect.left, rect.top, rect.right-rect.left, rect.bottom-rect.top, 0, 0, 0, rect.bottom-rect.top, srcimg, (BITMAPINFO*)&bmpInfo, DIB_RGB_COLORS);
			ReleaseDC(winRootWindow, dcScreen);
			if (srcimg != buffer)
				free(srcimg);
			
		#else
			bmpInfo.bV4Width = g->p.x2;
			bmpInfo.bV4Height = -g->p.cy; /* top-down image */
			bmpInfo.bV4SizeImage = (g->p.cy*g->p.x2) * sizeof(pixel_t);
			dcScreen = GetDC(winRootWindow);
			SetDIBitsToDevice(dcBuffer, g->p.x, g->p.y, g->p.cx, g->p.cy, g->p.x1, 0, 0, g->p.cy, buffer, (BITMAPINFO*)&bmpInfo, DIB_RGB_COLORS);
			SetDIBitsToDevice(dcScreen, g->p.x, g->p.y, g->p.cx, g->p.cy, g->p.x1, 0, 0, g->p.cy, buffer, (BITMAPINFO*)&bmpInfo, DIB_RGB_COLORS);
			ReleaseDC(winRootWindow, dcScreen);
		#endif
	}
#endif

#if GDISP_HARDWARE_PIXELREAD
	LLDSPEC	color_t gdisp_lld_get_pixel_color(GDISPDriver *g) {
		COLORREF	color;

		#if GDISP_NEED_CONTROL
			switch(g->g.Orientation) {
			case GDISP_ROTATE_0:
				color = GetPixel(dcBuffer, g->p.x, g->p.y);
				break;
			case GDISP_ROTATE_90:
				color = GetPixel(dcBuffer, g->p.y, g->g.Width - 1 - g->p.x);
				break;
			case GDISP_ROTATE_180:
				color = GetPixel(dcBuffer, g->g.Width - 1 - g->p.x, g->g.Height - 1 - g->p.y);
				break;
			case GDISP_ROTATE_270:
				color = GetPixel(dcBuffer, g->g.Height - 1 - g->p.y, g->p.x);
				break;
			}
		#else
			color = GetPixel(dcBuffer, g->p.x, g->p.y);
		#endif
		
		return BGR2COLOR(color);
	}
#endif

#if GDISP_NEED_SCROLL && GDISP_HARDWARE_SCROLL
	void gdisp_lld_vertical_scroll(GDISPDriver *g) {
		HDC			dcScreen;
		RECT		rect;
		coord_t		lines;
		
		#if GDISP_NEED_CONTROL
			switch(GC->g.Orientation) {
			case GDISP_ROTATE_0:
				rect.top = g->p.y;
				rect.bottom = rect.top+g->p.cy;
				rect.left = g->p.x;
				rect.right = rect.left+g->p.cx;
				lines = -g->p.y1;
				goto vertical_scroll;
			case GDISP_ROTATE_90:
				rect.bottom = g->g.Width - g->p.x;
				rect.top = rect.bottom-g->p.cx;
				rect.left = g->p.y;
				rect.right = rect.left+g->p.cy;
				lines = -g->p.y1;
				goto horizontal_scroll;
			case GDISP_ROTATE_180:
				rect.bottom = g->g.Height - g->p.y;
				rect.top = rect.bottom-g->p.cy;
				rect.right = GC->g.Width - g->p.x;
				rect.left = rect.right-g->p.cx;
				lines = g->p.y1;
			vertical_scroll:
				if (lines > 0) {
					rect.bottom -= lines;
				} else {
					rect.top -= lines;
				}
				if (g->p.cy >= lines && g->p.cy >= -lines) {
					dcScreen = GetDC(winRootWindow);
					ScrollDC(dcBuffer, 0, lines, &rect, 0, 0, 0);
					ScrollDC(dcScreen, 0, lines, &rect, 0, 0, 0);
					ReleaseDC(winRootWindow, dcScreen);
				}
				break;
			case GDISP_ROTATE_270:
				rect.top = g->p.x;
				rect.bottom = rect.top+g->p.cx;
				rect.right = g->g.Height - g->p.y;
				rect.left = rect.right-g->p.cy;
				lines = g->p.y1;
			horizontal_scroll:
				if (lines > 0) {
					rect.right -= lines;
				} else {
					rect.left -= lines;
				}
				if (g->p.cy >= lines && g->p.cy >= -lines) {
					dcScreen = GetDC(winRootWindow);
					ScrollDC(dcBuffer, lines, 0, &rect, 0, 0, 0);
					ScrollDC(dcScreen, lines, 0, &rect, 0, 0, 0);
					ReleaseDC(winRootWindow, dcScreen);
				}
				break;
			}
		#else
			rect.top = g->p.y;
			rect.bottom = rect.top+g->p.cy;
			rect.left = g->p.x;
			rect.right = rect.left+g->p.cx;
			lines = -g->p.y1;
			if (lines > 0) {
				rect.bottom -= lines;
			} else {
				rect.top -= lines;
			}
			if (g->p.cy >= lines && g->p.cy >= -lines) {
				dcScreen = GetDC(winRootWindow);
				ScrollDC(dcBuffer, 0, lines, &rect, 0, 0, 0);
				ScrollDC(dcScreen, 0, lines, &rect, 0, 0, 0);
				ReleaseDC(winRootWindow, dcScreen);
			}
		#endif
	}
#endif

#if GDISP_NEED_CONTROL && GDISP_HARDWARE_CONTROL
	LLDSPEC void gdisp_lld_control(GDISPDriver *g) {
		switch(g->p.x) {
		case GDISP_CONTROL_ORIENTATION:
			if (g->g.Orientation == (orientation_t)g->p.ptr)
				return;
			switch((orientation_t)g->p.ptr) {
				case GDISP_ROTATE_0:
				case GDISP_ROTATE_180:
					g->g.Width = wWidth;
					g->g.Height = wHeight;
					break;
				case GDISP_ROTATE_90:
				case GDISP_ROTATE_270:
					g->g.Height = wWidth;
					g->g.Width = wHeight;
					break;
				default:
					return;
			}
			g->g.Orientation = (orientation_t)g->p.ptr;
			return;
/*
		case GDISP_CONTROL_POWER:
		case GDISP_CONTROL_BACKLIGHT:
		case GDISP_CONTROL_CONTRAST:
*/
		}
	}
#endif

#if GINPUT_NEED_MOUSE
	void ginput_lld_mouse_init(void) {}
	void ginput_lld_mouse_get_reading(MouseReading *pt) {
		pt->x = mousex;
		pt->y = mousey > wHeight ? wHeight : mousey;
		pt->z = (mousebuttons & GINPUT_MOUSE_BTN_LEFT) ? 100 : 0;
		pt->buttons = mousebuttons;
	}
#endif /* GINPUT_NEED_MOUSE */

#if GINPUT_NEED_TOGGLE
	const GToggleConfig GInputToggleConfigTable[GINPUT_TOGGLE_CONFIG_ENTRIES] = {
		{0,	0xFF, 0x00, 0},
	};
	void ginput_lld_toggle_init(const GToggleConfig *ptc) { (void) ptc; }
	unsigned ginput_lld_toggle_getbits(const GToggleConfig *ptc) { (void) ptc; return toggles; }
#endif /* GINPUT_NEED_MOUSE */

#endif /* GFX_USE_GDISP */

