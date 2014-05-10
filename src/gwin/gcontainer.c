/*
 * This file is subject to the terms of the GFX License. If a copy of
 * the license was not distributed with this file, you can obtain one at:
 *
 *              http://ugfx.org/license.html
 */

/**
 * @file    src/gwin/gcontainer.c
 * @brief   GWIN sub-system container code.
 *
 * @defgroup Containers Containers
 * @ingroup GWIN
 *
 * @{
 */

#include "gfx.h"

#if GFX_USE_GWIN && GWIN_NEED_CONTAINERS

#include "src/gwin/class_gwin.h"

void _gcontainerInit(void)
{
}

void _gcontainerDeinit(void)
{
}

GHandle _gcontainerCreate(GDisplay *g, GContainerObject *pgc, const GWidgetInit *pInit, const gcontainerVMT *vmt) {
	if (!(pgc = (GContainerObject *)_gwidgetCreate(g, (GWidgetObject *)pgc, pInit, &vmt->gw)))
		return 0;

	pgc->g.flags |= GWIN_FLG_CONTAINER;

	return 	&pgc->g;
}

void _gcontainerDestroy(GHandle gh) {
	GHandle		child;

	while((child = gwinGetFirstChild(gh)))
		gwinDestroy(child);
	_gwidgetDestroy(gh);
}

void _gcontainerRedraw(GHandle gh) {
	GHandle		child;

	if (!(gh->flags & GWIN_FLG_SYSVISIBLE))
		return;

	((GWidgetObject *)gh)->fnDraw((GWidgetObject *)gh, ((GWidgetObject *)gh)->fnParam);

	for(child = gwinGetFirstChild(gh); child; child = gwinGetSibling(child))
		gwinRedraw(child);
}

void _gcontainerUpdate(GHandle gh) {
	GHandle		child;

	if (!(gh->flags & GWIN_FLG_SYSVISIBLE))
		return;

	#if GDISP_NEED_CLIP
		gdispGSetClip(gh->display, gh->x, gh->y, gh->width, gh->height);
	#endif
	((GWidgetObject *)gh)->fnDraw((GWidgetObject *)gh, ((GWidgetObject *)gh)->fnParam);

	for(child = gwinGetFirstChild(gh); child; child = gwinGetSibling(child))
		gwinRedraw(child);
}

void _gwinRecurse(GHandle gh, bool_t (*fn)(GHandle gh)) {
	if (fn(gh) && (gh->flags & GWIN_FLG_CONTAINER)) {
		// Apply to this windows children
		for(gh = gwinGetFirstChild(gh); gh; gh = gwinGetSibling(gh)) {
			// Only recurse when we have to. Otherwise apply it directly
			if ((gh->flags & GWIN_FLG_CONTAINER))
				_gwinRecurse(gh, fn);
			else
				fn(gh);
		}
	}
}

GHandle gwinGetFirstChild(GHandle gh) {
	GHandle		child;

	for(child = gwinGetNextWindow(0); child; child = gwinGetNextWindow(child))
		if (child->parent == gh)
			return child;
	return 0;
}

GHandle gwinGetSibling(GHandle gh) {
	GHandle		child;

	for(child = gwinGetNextWindow(gh), gh = gh->parent; child; child = gwinGetNextWindow(child))
		if (child->parent == gh)
			return child;
	return 0;
}

#endif /* GFX_USE_GWIN && GWIN_NEED_CONTAINERS */
/** @} */

/*-----------------------------------------------
 * The simplest container type - a container
 *-----------------------------------------------
 *
 * @defgroup Containers Containers
 * @ingroup GWIN
 *
 * @{
 */

#if GFX_USE_GWIN && GWIN_NEED_CONTAINER

static void DrawSimpleContainer(GWidgetObject *gw, void *param) {
	(void)	param;
	gdispGFillArea(gw->g.display, gw->g.x, gw->g.y, gw->g.width, gw->g.height, gw->pstyle->background);
}

// The container VMT table
static const gcontainerVMT containerVMT = {
	{
		{
			"Container",				// The classname
			sizeof(GContainerObject),	// The object size
			_gcontainerDestroy,			// The destroy routine
			_gcontainerRedraw,			// The redraw routine
			0,							// The after-clear routine
		},
		DrawSimpleContainer,			// The default drawing routine
		#if GINPUT_NEED_MOUSE
			{
				0, 0, 0,				// No mouse
			},
		#endif
		#if GINPUT_NEED_TOGGLE
			{
				0, 0, 0, 0, 0,			// No toggles
			},
		#endif
		#if GINPUT_NEED_DIAL
			{
				0, 0, 0, 0,				// No dials
			},
		#endif
	},
	0,									// Adjust the relative position of a child (optional)
	0,									// Adjust the size of a child (optional)
	0,									// A child has been added (optional)
	0,									// A child has been deleted (optional)
};

GHandle gwinGContainerCreate(GDisplay *g, GContainerObject *gw, const GWidgetInit *pInit) {
	if (!(gw = (GContainerObject *)_gcontainerCreate(g, gw, pInit, &containerVMT)))
		return 0;

	gwinSetVisible((GHandle)gw, pInit->g.show);
	return (GHandle)gw;
}

#endif
/** @} */
