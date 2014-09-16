/*
 * This file is subject to the terms of the GFX License. If a copy of
 * the license was not distributed with this file, you can obtain one at:
 *
 *              http://ugfx.org/license.html
 */

/**
 * @file    src/ginput/ginput_mouse.c
 * @brief   GINPUT mouse/touch code.
 *
 * @defgroup Mouse Mouse
 * @ingroup GINPUT
 * @{
 */
#include "gfx.h"

#if (GFX_USE_GINPUT && GINPUT_NEED_MOUSE) || defined(__DOXYGEN__)

#include "driver_mouse.h"

#if !GINPUT_TOUCH_NOCALIBRATE
	#if !defined(GFX_USE_GDISP) || !GFX_USE_GDISP
		#error "GINPUT: GFX_USE_GDISP must be defined when mouse or touch calibration is required"
	#endif

	#include <string.h>							// Required for memcpy

	#define CALIBRATION_FONT			"* Double"
	#define CALIBRATION_FONT2			"* Narrow"
	#define CALIBRATION_TEXT			"Calibration"
	#define CALIBRATION_ERROR_TEXT		"Failed - Please try again!"
	#define CALIBRATION_SAME_TEXT		"Error: Same Reading - Check Driver!"
	#define CALIBRATION_BACKGROUND		Blue
	#define CALIBRATION_COLOR1			White
	#define CALIBRATION_COLOR2			RGB2COLOR(184,158,131)

#endif

static GTIMER_DECL(MouseTimer);

#if !GINPUT_TOUCH_NOCALIBRATE
	/*
	static inline void CalibrationSetIdentity(MouseCalibration *c) {
		c->ax = 1;
		c->bx = 0;
		c->cx = 0;
		c->ay = 0;
		c->by = 1;
		c->cy = 0;
	}
	*/

	static inline void CalibrationCrossDraw(GMouse *m, const point *pp) {
		gdispGDrawLine(m->display, pp->x-15, pp->y, pp->x-2, pp->y, CALIBRATION_COLOR1);
		gdispGDrawLine(m->display, pp->x+2, pp->y, pp->x+15, pp->y, CALIBRATION_COLOR1);
		gdispGDrawLine(m->display, pp->x, pp->y-15, pp->x, pp->y-2, CALIBRATION_COLOR1);
		gdispGDrawLine(m->display, pp->x, pp->y+2, pp->x, pp->y+15, CALIBRATION_COLOR1);
		gdispGDrawLine(m->display, pp->x-15, pp->y+15, pp->x-7, pp->y+15, CALIBRATION_COLOR2);
		gdispGDrawLine(m->display, pp->x-15, pp->y+7, pp->x-15, pp->y+15, CALIBRATION_COLOR2);
		gdispGDrawLine(m->display, pp->x-15, pp->y-15, pp->x-7, pp->y-15, CALIBRATION_COLOR2);
		gdispGDrawLine(m->display, pp->x-15, pp->y-7, pp->x-15, pp->y-15, CALIBRATION_COLOR2);
		gdispGDrawLine(m->display, pp->x+7, pp->y+15, pp->x+15, pp->y+15, CALIBRATION_COLOR2);
		gdispGDrawLine(m->display, pp->x+15, pp->y+7, pp->x+15, pp->y+15, CALIBRATION_COLOR2);
		gdispGDrawLine(m->display, pp->x+7, pp->y-15, pp->x+15, pp->y-15, CALIBRATION_COLOR2);
		gdispGDrawLine(m->display, pp->x+15, pp->y-15, pp->x+15, pp->y-7, CALIBRATION_COLOR2);
	}

	static inline void CalibrationCrossClear(GMouse *m, const point *pp) {
		gdispGFillArea(m->display, pp->x - 15, pp->y - 15, 32, 32, CALIBRATION_BACKGROUND);
	}

	static inline void CalibrationTransform(GMouseReading *pt, const GMouseCalibration *c) {
		pt->x = (coord_t) (c->ax * pt->x + c->bx * pt->y + c->cx);
		pt->y = (coord_t) (c->ay * pt->x + c->by * pt->y + c->cy);
	}

	static inline void CalibrationCalculate(GMouse *m, const point *cross, const point *points) {
		float		dx;
		coord_t		c0, c1, c2;
		(void)		m;

		// Work on x values
		c0 = cross[0].x;
		c1 = cross[1].x;
		c2 = cross[2].x;

		#if GDISP_NEED_CONTROL
			if (!(gmvmt(m)->d.flags & GMOUSE_VFLG_SELFROTATION)) {
				/* Convert all cross points back to GDISP_ROTATE_0 convention
				 * before calculating the calibration matrix.
				 */
				switch(gdispGGetOrientation(m->display)) {
				case GDISP_ROTATE_90:
					c0 = cross[0].y;
					c1 = cross[1].y;
					c2 = cross[2].y;
					break;
				case GDISP_ROTATE_180:
					c0 = c1 = c2 = gdispGGetWidth(m->display) - 1;
					c0 -= cross[0].x;
					c1 -= cross[1].x;
					c2 -= cross[2].x;
					break;
				case GDISP_ROTATE_270:
					c0 = c1 = c2 = gdispGGetHeight(m->display) - 1;
					c0 -= cross[0].y;
					c1 -= cross[1].y;
					c2 -= cross[2].y;
					break;
                default:
                    break;
				}
			}
		#endif

		/* Compute all the required determinants */
		dx  = (float)(points[0].x - points[2].x) * (float)(points[1].y - points[2].y)
				- (float)(points[1].x - points[2].x) * (float)(points[0].y - points[2].y);

		m->caldata.ax = ((float)(c0 - c2) * (float)(points[1].y - points[2].y)
							- (float)(c1 - c2) * (float)(points[0].y - points[2].y)) / dx;
		m->caldata.bx = ((float)(c1 - c2) * (float)(points[0].x - points[2].x)
							- (float)(c0 - c2) * (float)(points[1].x - points[2].x)) / dx;
		m->caldata.cx = (c0 * ((float)points[1].x * (float)points[2].y - (float)points[2].x * (float)points[1].y)
							- c1 * ((float)points[0].x * (float)points[2].y - (float)points[2].x * (float)points[0].y)
							+ c2 * ((float)points[0].x * (float)points[1].y - (float)points[1].x * (float)points[0].y)) / dx;

		// Work on y values
		c0 = cross[0].y;
		c1 = cross[1].y;
		c2 = cross[2].y;

		#if GDISP_NEED_CONTROL
			if (!(gmvmt(m)->d.flags & GMOUSE_VFLG_SELFROTATION)) {
				switch(gdispGGetOrientation(m->display)) {
				case GDISP_ROTATE_90:
					c0 = c1 = c2 = gdispGGetWidth(m->display) - 1;
					c0 -= cross[0].x;
					c1 -= cross[1].x;
					c2 -= cross[2].x;
					break;
				case GDISP_ROTATE_180:
					c0 = c1 = c2 = gdispGGetHeight(m->display) - 1;
					c0 -= cross[0].y;
					c1 -= cross[1].y;
					c2 -= cross[2].y;
					break;
				case GDISP_ROTATE_270:
					c0 = cross[0].x;
					c1 = cross[1].x;
					c2 = cross[2].x;
					break;
                default:
                    break;
				}
			}
		#endif

		m->caldata.ay = ((float)(c0 - c2) * (float)(points[1].y - points[2].y)
							- (float)(c1 - c2) * (float)(points[0].y - points[2].y)) / dx;
		m->caldata.by = ((float)(c1 - c2) * (float)(points[0].x - points[2].x)
							- (float)(c0 - c2) * (float)(points[1].x - points[2].x)) / dx;
		m->caldata.cy = (c0 * ((float)points[1].x * (float)points[2].y - (float)points[2].x * (float)points[1].y)
							- c1 * ((float)points[0].x * (float)points[2].y - (float)points[2].x * (float)points[0].y)
							+ c2 * ((float)points[0].x * (float)points[1].y - (float)points[1].x * (float)points[0].y)) / dx;
	}
#endif

static void GetMouseReading(GMouse *m) {
	GMouseReading	r;

	// Get the raw reading.
	gmvmt(m)->get(m, &r);
	m->flags &= ~GMOUSE_FLG_NEEDREAD;

	// If touch then calculate button 0 from z
	if ((gmvmt(m)->d.flags & GMOUSE_VFLG_TOUCH)) {
		if (gmvmt(m)->z_min <= gmvmt(m)->z_max) {
			if (r.z >= gmvmt(m)->z_touchon)			r.buttons |= GINPUT_MOUSE_BTN_LEFT;
			else if (r.z <= gmvmt(m)->z_touchoff)	r.buttons &= ~GINPUT_MOUSE_BTN_LEFT;
			else									return;				// bad transitional reading
		} else {
			if (r.z <= gmvmt(m)->z_touchon)			r.buttons |= GINPUT_MOUSE_BTN_LEFT;
			else if (r.z >= gmvmt(m)->z_touchoff)	r.buttons &= ~GINPUT_MOUSE_BTN_LEFT;
			else									return;				// bad transitional reading
		}
	}

	// Double check up & down events if needed
	if ((gmvmt(m)->d.flags & GMOUSE_VFLG_POORUPDOWN)) {
		// Are we in a transition test
		if ((m->flags & GMOUSE_FLG_INDELTA)) {
			if (!((r.buttons ^ m->r.buttons) & GINPUT_MOUSE_BTN_LEFT)) {
				// Transition failed
				m->flags &= ~GMOUSE_FLG_INDELTA;
				return;
			}
			// Transition succeeded
			m->flags &= ~GMOUSE_FLG_INDELTA;

		// Should we start a transition test
		} else if (((r.buttons ^ m->r.buttons) & GINPUT_MOUSE_BTN_LEFT)) {
			m->flags |= GMOUSE_FLG_INDELTA;
			return;
		}
	}

	// If the mouse is up we may need to keep our previous position
	if ((gmvmt(m)->d.flags & GMOUSE_VFLG_ONLY_DOWN) && !(r.buttons & GINPUT_MOUSE_BTN_LEFT)) {
		r.x = m->r.x;
		r.y = m->r.y;

	} else {
		coord_t			w, h;

		#if !GINPUT_TOUCH_NOCALIBRATE
			// Do we need to calibrate the reading?
			if ((m->flags & GMOUSE_FLG_CALIBRATE))
				CalibrationTransform(&r, &m->caldata);
		#endif

		// We can't clip or rotate if we don't have a display
		if (m->display) {

			// We now need display information
			w = gdispGGetWidth(m->display);
			h = gdispGGetHeight(m->display);

			#if GDISP_NEED_CONTROL
				// Do we need to rotate the reading to match the display
				if (!(gmvmt(m)->d.flags & GMOUSE_VFLG_SELFROTATION)) {
					coord_t		t;

					switch(gdispGGetOrientation(m->display)) {
						case GDISP_ROTATE_0:
							break;
						case GDISP_ROTATE_90:
							t = r.x;
							r.x = w - 1 - r.y;
							r.y = t;
							break;
						case GDISP_ROTATE_180:
							r.x = w - 1 - r.x;
							r.y = h - 1 - r.y;
							break;
						case GDISP_ROTATE_270:
							t = r.y;
							r.y = h - 1 - r.x;
							r.x = t;
							break;
						default:
							break;
					}
				}
			#endif

			// Do we need to clip the reading to the display
			if ((m->flags & GMOUSE_FLG_CLIP)) {
				if (r.x < 0)		r.x = 0;
				else if (r.x >= w)	r.x = w-1;
				if (r.y < 0)		r.y = 0;
				else if (r.y >= h)	r.y = h-1;
			}
		}
	}

	{
		const GMouseJitter	*pj;
		uint32_t			diff;

		// Are we in pen or finger mode
		pj = (m->flags & GMOUSE_FLG_FINGERMODE) ? &gmvmt(m)->finger_jitter : &gmvmt(m)->pen_jitter;

		// Is this just movement jitter
		if (pj->move > 0) {
			diff = (uint32_t)(r.x - m->r.x) * (uint32_t)(r.x - m->r.x) + (uint32_t)(r.y - m->r.y) * (uint32_t)(r.y - m->r.y);
			if (diff > (uint32_t)pj->move * (uint32_t)pj->move) {
				r.x = m->r.x;
				r.y = m->r.y;
			}
		}

		// Check if the click has moved outside the click area and if so cancel the click
		if (pj->click > 0 && (m->flags & GMOUSE_FLG_CLICK_TIMER)) {
			diff = (uint32_t)(r.x - m->clickpos.x) * (uint32_t)(r.x - m->clickpos.x) + (uint32_t)(r.y - m->clickpos.y) * (uint32_t)(r.y - m->clickpos.y);
			if (diff > (uint32_t)pj->click * (uint32_t)pj->click)
				m->flags &= ~GMOUSE_FLG_CLICK_TIMER;
		}
	}

	{
		GSourceListener	*psl;
		GEventMouse		*pe;
		unsigned 		meta;
		uint16_t		upbtns, dnbtns;

		// Calculate out new event meta value and handle CLICK and CXTCLICK
		dnbtns = r.buttons & ~m->r.buttons;
		upbtns = ~r.buttons & m->r.buttons;
		meta = GMETA_NONE;

		// Mouse down
		if ((dnbtns & (GINPUT_MOUSE_BTN_LEFT|GINPUT_MOUSE_BTN_RIGHT))) {
			m->clickpos.x = r.x;
			m->clickpos.y = r.y;
			m->clicktime = gfxSystemTicks();
			m->flags |= GMOUSE_FLG_CLICK_TIMER;
			if ((dnbtns & GINPUT_MOUSE_BTN_LEFT))
				meta |= GMETA_MOUSE_DOWN;
		}

		// Mouse up
		if ((upbtns & (GINPUT_MOUSE_BTN_LEFT|GINPUT_MOUSE_BTN_RIGHT))) {
			if ((upbtns & GINPUT_MOUSE_BTN_LEFT))
				meta |= GMETA_MOUSE_UP;
			if ((m->flags & GMOUSE_FLG_CLICK_TIMER)) {
				if ((upbtns & GINPUT_MOUSE_BTN_LEFT)
						#if GINPUT_TOUCH_CLICK_TIME != TIME_INFINITE
							&& gfxSystemTicks() - m->clicktime < gfxMillisecondsToTicks(GINPUT_TOUCH_CLICK_TIME)
						#endif
						)
					meta |= GMETA_MOUSE_CLICK;
				else
					meta |= GMETA_MOUSE_CXTCLICK;
				m->flags &= ~GMOUSE_FLG_CLICK_TIMER;
			}
		}

		// Send the event to the listeners that are interested.
		psl = 0;
		while ((psl = geventGetSourceListener((GSourceHandle)m, psl))) {
			if (!(pe = (GEventMouse *)geventGetEventBuffer(psl))) {
				// This listener is missing - save the meta events that have happened
				psl->srcflags |= meta;
				continue;
			}

			// If we haven't really moved (and there are no meta events) don't bother sending the event
			if (!meta && !psl->srcflags && !(psl->listenflags & GLISTEN_MOUSENOFILTER)
					&& r.x == m->r.x && r.y == m->r.y && r.buttons == m->r.buttons)
				continue;

			// Send the event if we are listening for it
			if (((r.buttons & GINPUT_MOUSE_BTN_LEFT) && (psl->listenflags & GLISTEN_MOUSEDOWNMOVES))
					|| (!(r.buttons & GINPUT_MOUSE_BTN_LEFT) && (psl->listenflags & GLISTEN_MOUSEUPMOVES))
					|| (meta && (psl->listenflags & GLISTEN_MOUSEMETA))) {
				pe->type = (gmvmt(m)->d.flags & GMOUSE_VFLG_TOUCH) ? GEVENT_TOUCH : GEVENT_MOUSE;
				pe->x = r.x;
				pe->y = r.y;
				pe->z = r.z;
				pe->current_buttons = r.buttons;
				pe->last_buttons = m->r.buttons;
				pe->meta = meta;
				if (psl->srcflags) {
					pe->current_buttons |= GINPUT_MISSED_MOUSE_EVENT;
					pe->meta |= psl->srcflags;
					psl->srcflags = 0;
				}
				pe->display = m->display;
				geventSendEvent(psl);
			}
		}
	}

	// Finally save the results
	m->r.x = r.x;
	m->r.y = r.y;
	m->r.z = r.z;
	m->r.buttons = r.buttons;
}

static void MousePoll(void *param) {
	GMouse *	m;
	(void) 		param;

	for(m = (GMouse *)gdriverGetNext(GDRIVER_TYPE_MOUSE, 0); m; m = (GMouse *)gdriverGetNext(GDRIVER_TYPE_MOUSE, (GDriver *)m)) {
		if (!(gmvmt(m)->d.flags & GMOUSE_VFLG_NOPOLL) || (m->flags & GMOUSE_FLG_NEEDREAD))
			GetMouseReading(m);
	}
}

void _gmouseInit(void) {
	// GINPUT_MOUSE_DRIVER_LIST is defined - create each driver instance
	#if defined(GINPUT_MOUSE_DRIVER_LIST)
		{
			int		i;

			extern GDriverVMTList					GINPUT_MOUSE_DRIVER_LIST;
			static const struct GDriverVMT const *	dclist[] = {GINPUT_MOUSE_DRIVER_LIST};
			static const unsigned					dnlist[] = {GDISP_CONTROLLER_DISPLAYS};

			for(i = 0; i < sizeof(dclist)/sizeof(dclist[0]); i++)
				gdriverRegister(dclist[i]);
		}

	// One and only one display
	#else
		{
			extern GDriverVMTList					GINPUTMOUSEVMT_OnlyOne;

			gdriverRegister(GINPUTMOUSEVMT_OnlyOne);
		}
	#endif
}

void _gmouseDeinit(void) {
}

GSourceHandle ginputGetMouse(uint16_t instance) {
    GMouse      *m;
	#if GINPUT_MOUSE_NEED_CALIBRATION
		GCalibration		*pc;
	#endif

    if (!(m = (GMouse *)gdriverGetInstance(GDRIVER_TYPE_MOUSE, instance)))
        return 0;

	// Make sure we have a valid mouse display
	if (!m->display)
		m->display = GDISP;

	// Do we need to initialise the mouse subsystem?
	if (!(m->flags & FLG_INIT_DONE)) {
		ginput_lld_mouse_init();

		#if GINPUT_MOUSE_NEED_CALIBRATION
			#if GINPUT_MOUSE_LLD_CALIBRATION_LOADSAVE
				if (!m->fnloadcal) {
					m->fnloadcal = ginput_lld_mouse_calibration_load;
					m->flags &= ~FLG_CAL_FREE;
				}
				if (!m->fnsavecal)
					m->fnsavecal = ginput_lld_mouse_calibration_save;
			#endif
			if (m->fnloadcal && (pc = (Calibration *)m->fnloadcal(instance))) {
				memcpy(&m->caldata, pc, sizeof(m->caldata));
				m->flags |= (FLG_CAL_OK|FLG_CAL_SAVED);
				if ((m->flags & FLG_CAL_FREE))
					gfxFree((void *)pc);
			} else if (instance == 9999) {
				CalibrationSetIdentity(&m->caldata);
				m->flags |= (FLG_CAL_OK|FLG_CAL_SAVED|FLG_CAL_RAW);
			} else
				ginputCalibrateMouse(instance);
		#endif

		// Get the first reading
		m->last_buttons = 0;
		get_calibrated_reading(&m->t);

		// Mark init as done and start the Poll timer
		m->flags |= FLG_INIT_DONE;
		gtimerStart(&MouseTimer, MousePoll, 0, TRUE, GINPUT_MOUSE_POLL_PERIOD);
	}

	// Return our structure as the handle
	return (GSourceHandle)m;
}

void ginputSetMouseDisplay(uint16_t instance, GDisplay *g) {
	if (instance)
		return;

	m->display = g ? g : GDISP;
}

GDisplay *ginputGetMouseDisplay(uint16_t instance) {
	if (instance)
		return 0;

	return m->display;
}

bool_t ginputGetMouseStatus(uint16_t instance, GEventMouse *pe) {
	// Win32 threads don't seem to recognise priority and/or pre-emption
	// so we add a sleep here to prevent 100% polled applications from locking up.
	gfxSleepMilliseconds(1);

	if (instance || (m->flags & (FLG_INIT_DONE|FLG_IN_CAL)) != FLG_INIT_DONE)
		return FALSE;

	pe->type = GINPUT_MOUSE_EVENT_TYPE;
	pe->x = m->t.x;
	pe->y = m->t.y;
	pe->z = m->t.z;
	pe->current_buttons = m->t.buttons;
	pe->last_buttons = m->last_buttons;
	if (pe->current_buttons & ~pe->last_buttons & GINPUT_MOUSE_BTN_LEFT)
		pe->meta = GMETA_MOUSE_DOWN;
	else if (~pe->current_buttons & pe->last_buttons & GINPUT_MOUSE_BTN_LEFT)
		pe->meta = GMETA_MOUSE_UP;
	else
		pe->meta = GMETA_NONE;
	return TRUE;
}

bool_t ginputCalibrateMouse(uint16_t instance) {
	#if GINPUT_TOUCH_NOCALIBRATE

		(void) instance;

		return FALSE;
	#else

		const coord_t height  =  gdispGGetHeight(MouseConfig.display);
		const coord_t width  =  gdispGGetWidth(MouseConfig.display);
		#if GINPUT_MOUSE_CALIBRATE_EXTREMES
			const MousePoint cross[]  =  {{0, 0},
									{(width - 1) , 0},
									{(width - 1) , (height - 1)},
									{(width / 2), (height / 2)}}; /* Check point */
		#else
			const MousePoint cross[]  =  {{(width / 4), (height / 4)},
									{(width - (width / 4)) , (height / 4)},
									{(width - (width / 4)) , (height - (height / 4))},
									{(width / 2), (height / 2)}}; /* Check point */
		#endif
		MousePoint points[4];
		const MousePoint	*pc;
		MousePoint *pt;
		int32_t px, py;
		unsigned i, j;
		font_t	font1, font2;
		#if GINPUT_MOUSE_MAX_CALIBRATION_ERROR >= 0
			unsigned	err;
		#endif

		if (instance || (MouseConfig.flags & FLG_IN_CAL))
			return FALSE;

		font1 = gdispOpenFont(GINPUT_MOUSE_CALIBRATION_FONT);
		font2 = gdispOpenFont(GINPUT_MOUSE_CALIBRATION_FONT2);

		MouseConfig.flags |= FLG_IN_CAL;
		gtimerStop(&MouseTimer);
		MouseConfig.flags &= ~(FLG_CAL_OK|FLG_CAL_SAVED|FLG_CAL_RAW);

		#if GDISP_NEED_CLIP
			gdispGSetClip(MouseConfig.display, 0, 0, width, height);
		#endif

		#if GINPUT_MOUSE_MAX_CALIBRATION_ERROR >= 0
			while(1) {
		#endif
				gdispGClear(MouseConfig.display, Blue);

				gdispGFillStringBox(MouseConfig.display, 0, 5, width, 30, GINPUT_MOUSE_CALIBRATION_TEXT, font1,  White, Blue, justifyCenter);

				for(i = 0, pt = points, pc = cross; i < GINPUT_MOUSE_CALIBRATION_POINTS; i++, pt++, pc++) {
					CalibrationCrossDraw(pc);

					do {

						/* Wait for the mouse to be pressed */
						while(get_raw_reading(&MouseConfig.t), !(MouseConfig.t.buttons & GINPUT_MOUSE_BTN_LEFT))
							gfxSleepMilliseconds(20);

						/* Average all the samples while the mouse is down */
						for(px = py = 0, j = 0;
								gfxSleepMilliseconds(20),			/* Settling time between readings */
								get_raw_reading(&MouseConfig.t),
								(MouseConfig.t.buttons & GINPUT_MOUSE_BTN_LEFT);
								j++) {
							px += MouseConfig.t.x;
							py += MouseConfig.t.y;
						}

					} while(!j);

					pt->x = px / j;
					pt->y = py / j;

					CalibrationCrossClear(pc);

					if (i >= 1 && pt->x == (pt-1)->x && pt->y == (pt-1)->y) {
						gdispGFillStringBox(MouseConfig.display, 0, 35, width, 40, GINPUT_MOUSE_CALIBRATION_SAME_TEXT, font2,  Red, Yellow, justifyCenter);
						gfxSleepMilliseconds(5000);
						gdispGFillArea(MouseConfig.display, 0, 35, width, 40, Blue);
					}

				}

				/* Apply 3 point calibration algorithm */
				CalibrationCalculate(&MouseConfig, cross, points);

				 /* Verification of correctness of calibration (optional) :
				 *  See if the 4th point (Middle of the screen) coincides with the calibrated
				 *  result. If point is within +/- Squareroot(ERROR) pixel margin, then successful calibration
				 *  Else, start from the beginning.
				 */
		#if GINPUT_MOUSE_MAX_CALIBRATION_ERROR >= 0
				/* Transform the co-ordinates */
				MouseConfig.t.x = points[3].x;
				MouseConfig.t.y = points[3].y;
				CalibrationTransform(&MouseConfig.t, &MouseConfig.caldata);
				_tsOrientClip(&MouseConfig.t, MouseConfig.display, FALSE);

				/* Calculate the delta */
				err = (MouseConfig.t.x - cross[3].x) * (MouseConfig.t.x - cross[3].x) +
					(MouseConfig.t.y - cross[3].y) * (MouseConfig.t.y - cross[3].y);

				if (err <= GINPUT_MOUSE_MAX_CALIBRATION_ERROR * GINPUT_MOUSE_MAX_CALIBRATION_ERROR)
					break;

				gdispGFillStringBox(MouseConfig.display, 0, 35, width, 40, GINPUT_MOUSE_CALIBRATION_ERROR_TEXT, font2,  Red, Yellow, justifyCenter);
				gfxSleepMilliseconds(5000);
			}
		#endif

		// Restart everything
		gdispCloseFont(font1);
		gdispCloseFont(font2);
		MouseConfig.flags |= FLG_CAL_OK;
		MouseConfig.last_buttons = 0;
		get_calibrated_reading(&MouseConfig.t);
		MouseConfig.flags &= ~FLG_IN_CAL;
		if ((MouseConfig.flags & FLG_INIT_DONE))
			gtimerStart(&MouseTimer, MousePoll, 0, TRUE, GINPUT_MOUSE_POLL_PERIOD);

		// Save the calibration data (if possible)
		if (MouseConfig.fnsavecal) {
			MouseConfig.fnsavecal(instance, (const uint8_t *)&MouseConfig.caldata, sizeof(MouseConfig.caldata));
			MouseConfig.flags |= FLG_CAL_SAVED;
		}

		// Clear the screen using the GWIN default background color
		#if GFX_USE_GWIN
			gdispGClear(MouseConfig.display, gwinGetDefaultBgColor());
		#else
			gdispGClear(MouseConfig.display, Black);
		#endif

		return TRUE;
	#endif
}

/* Set the routines to save and fetch calibration data.
 * This function should be called before first calling ginputGetMouse() for a particular instance
 *	as the ginputGetMouse() routine may attempt to fetch calibration data and perform a startup calibration if there is no way to get it.
 *	If this is called after ginputGetMouse() has been called and the driver requires calibration storage, it will immediately save the data is has already obtained.
 * The 'requireFree' parameter indicates if the fetch buffer must be free()'d to deallocate the buffer provided by the Fetch routine.
 */
void ginputSetMouseCalibrationRoutines(uint16_t instance, GMouseCalibrationSaveRoutine fnsave, GMouseCalibrationLoadRoutine fnload, bool_t requireFree) {
	#if GINPUT_MOUSE_NEED_CALIBRATION
		if (instance)
			return;

		MouseConfig.fnloadcal = fnload;
		MouseConfig.fnsavecal = fnsave;
		if (requireFree)
			MouseConfig.flags |= FLG_CAL_FREE;
		else
			MouseConfig.flags &= ~FLG_CAL_FREE;
		#if GINPUT_MOUSE_LLD_CALIBRATION_LOADSAVE
			if (!MouseConfig.fnloadcal) {
				MouseConfig.fnloadcal = ginput_lld_mouse_calibration_load;
				MouseConfig.flags &= ~FLG_CAL_FREE;
			}
			if (!MouseConfig.fnsavecal)
				MouseConfig.fnsavecal = ginput_lld_mouse_calibration_save;
		#endif
		if (MouseConfig.fnsavecal && (MouseConfig.flags & (FLG_CAL_OK|FLG_CAL_SAVED)) == FLG_CAL_OK) {
			MouseConfig.fnsavecal(instance, (const uint8_t *)&MouseConfig.caldata, sizeof(MouseConfig.caldata));
			MouseConfig.flags |= FLG_CAL_SAVED;
		}
	#else
		(void)instance, (void)fnsave, (void)fnload, (void)requireFree;
	#endif
}

/* Test if a particular mouse instance requires routines to save its calibration data. */
bool_t ginputRequireMouseCalibrationStorage(uint16_t instance) {
	if (instance)
		return FALSE;

	#if GINPUT_MOUSE_NEED_CALIBRATION && !GINPUT_MOUSE_LLD_CALIBRATION_LOADSAVE
		return TRUE;
	#else
		return FALSE;
	#endif
}

/* Wake up the mouse driver from an interrupt service routine (there may be new readings available) */
void ginputMouseWakeup(GMouse *m) {
	m->flags |= GMOUSE_FLG_NEEDREAD;
	gtimerJab(&MouseTimer);
}

/* Wake up the mouse driver from an interrupt service routine (there may be new readings available) */
void ginputMouseWakeupI(GMouse *m) {
	m->flags |= GMOUSE_FLG_NEEDREAD;
	gtimerJabI(&MouseTimer);
}

#endif /* GFX_USE_GINPUT && GINPUT_NEED_MOUSE */
/** @} */
