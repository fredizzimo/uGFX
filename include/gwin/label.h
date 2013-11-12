/*
 * This file is subject to the terms of the GFX License. If a copy of
 * the license was not distributed with this file, you can obtain one at:
 *
 *              http://ugfx.org/license.html
 */

/**
 * @file	include/gwin/label.h
 * @brief	GWIN label widget header file.
 *
 * @defgroup Label Label
 * @ingroup GWIN
 *
 * @details		GWIN allos it to create an label widget. The widget
 *				takes no user input.
 *
 * @pre			GFX_USE_GDISP must be set to TRUE in your gfxconf.h
 * @pre			GFX_USE_GWIN must be set to TRUE in your gfxconf.h
 * @pre			GDISP_NEED_TEXT must be set to TRUE in your gfxconf.h
 * @pre			GWIN_NEED_LABEL must be set to TRUE in your gfxconf.h
 * @pre			The fonts you want to use must be enabled in your gfxconf.h
 *
 * @{
 */

#ifndef _GWIN_LABEL_H
#define _GWIN_LABEL_H

// This file is included within "gwin/gwin.h"

// An label window
typedef struct GLabelObject {
	GWidgetObject	w;
} GLabelObject;

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief				Create a label widget.
 * @details				A label widget is a simple window which has a static text.
 * 
 * @param[in] g			The GDisplay to display this window on
 * @param[in] widget	The label structure to initialise. If this is NULL, the structure is dynamically allocated.
 * @param[in] pInit		The initialisation parameters to use.
 *
 * @return				NULL if there is no resultat drawing area, otherwise the widget handle.
 *
 * @api
 */
GHandle gwinGLabelCreate(GDisplay *g, GLabelObject *widget, GWidgetInit *pInit);
#define gwinLabelCreate(w, pInit)			gwinGLabelCreate(GDISP, w, pInit)

/**
 * @brief				Border settings for the default rendering routine
 *
 * @param[in] gh		The widget handle (must be a list handle)
 * @param[in] border	Shall a border be rendered?
 *
 * @api
 */
void gwinLabelSetBorder(GHandle gh, bool_t border);

#ifdef __cplusplus
}
#endif

#endif // _GWIN_LABEL_H
/** @} */

