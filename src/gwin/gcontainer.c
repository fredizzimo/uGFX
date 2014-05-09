/*
 * This file is subject to the terms of the GFX License. If a copy of
 * the license was not distributed with this file, you can obtain one at:
 *
 *              http://ugfx.org/license.html
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
