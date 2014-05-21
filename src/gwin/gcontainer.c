/*
 * This file is subject to the terms of the GFX License. If a copy of
 * the license was not distributed with this file, you can obtain one at:
 *
 *              http://ugfx.org/license.html
 */

/**
 * @file    src/gwin/gcontainer.c
 * @brief   GWIN sub-system container code
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

coord_t gwinGetInnerWidth(GHandle gh) {
	if (!(gh->flags & GWIN_FLG_CONTAINER))
		return 0;
	return gh->width - ((const gcontainerVMT *)gh->vmt)->LeftBorder(gh) - ((const gcontainerVMT *)gh->vmt)->RightBorder(gh);
}

coord_t gwinGetInnerHeight(GHandle gh) {
	if (!(gh->flags & GWIN_FLG_CONTAINER))
		return 0;
	return gh->height - ((const gcontainerVMT *)gh->vmt)->TopBorder(gh) - ((const gcontainerVMT *)gh->vmt)->BottomBorder(gh);
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

#if GWIN_CONTAINER_BORDER != GWIN_FIRST_CONTROL_FLAG
	#error "GWIN Container: - Flag definitions don't match"
#endif

static coord_t BorderSize(GHandle gh)	{ return (gh->flags & GWIN_CONTAINER_BORDER) ? 2 : 0; }

static void DrawSimpleContainer(GWidgetObject *gw, void *param) {
	(void)	param;
	gdispGFillArea(gw->g.display, gw->g.x, gw->g.y, gw->g.width, gw->g.height, gw->pstyle->background);
	if ((gw->g.flags & GWIN_CONTAINER_BORDER))
		gdispGDrawBox(gw->g.display, gw->g.x, gw->g.y, gw->g.width, gw->g.height, (gw->g.flags & GWIN_FLG_SYSENABLED) ? gw->pstyle->enabled.edge : gw->pstyle->disabled.edge);
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
	BorderSize,							// The size of the left border (mandatory)
	BorderSize,							// The size of the top border (mandatory)
	BorderSize,							// The size of the right border (mandatory)
	BorderSize,							// The size of the bottom border (mandatory)
	0,									// A child has been added (optional)
	0,									// A child has been deleted (optional)
};

GHandle gwinGContainerCreate(GDisplay *g, GContainerObject *gc, const GWidgetInit *pInit, uint32_t flags) {
	if (!(gc = (GContainerObject *)_gcontainerCreate(g, gc, pInit, &containerVMT)))
		return 0;

	gc->g.flags |= flags;

	gwinSetVisible((GHandle)gc, pInit->g.show);
	return (GHandle)gc;
}

#endif
