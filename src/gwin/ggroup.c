/*
 * This file is subject to the terms of the GFX License. If a copy of
 * the license was not distributed with this file, you can obtain one at:
 *
 *              http://ugfx.org/license.html
 */

#include "gfx.h"

#if GFX_USE_GWIN && GWIN_NEED_GROUPS

#include "gwin/class_gwin.h"

GHandle _ggroupCreate(GDisplay *g, GGroupObject *go, const GWindowInit *pInit, const ggroupVMT *vmt) {
	if (!(go = (GGroupObject *)_gwindowCreate(g, &go->g, pInit, &vmt->g, GWIN_FLG_GROUP|GWIN_FLG_ENABLED)))
		return NULL;

	go->parent = NULL;
	go->sibling = NULL;
	go->child = NULL;

	return &go->g;
}

#endif /* GFX_USE_GWIN && GWIN_NEED_GROUPS */

