/*
 * This file is subject to the terms of the GFX License. If a copy of
 * the license was not distributed with this file, you can obtain one at:
 *
 *              http://ugfx.org/license.html
 */

#include "gfx.h"

#if GFX_NEED_GDRIVER

// Some HACKS as these aren't supported yet
#ifndef GINPUT_NEED_STRING
	#define GINPUT_NEED_STRING	FALSE
#endif
#ifndef GFX_USE_GBLOCK
	#define GFX_USE_GBLOCK	FALSE
#endif

// Define the tables to hold the driver instances.
static GDriver *dhead;

// The system initialization.
void _gdriverInit(void) {
	const GDriverAutoStart const *pa;
	int		cnt;

	#if GFX_USE_GDISP
		#ifdef GDISP_DRIVER_LIST
			{
				static const struct {
					const struct GDISPVMT const *	vmt;
					int								instances;
				} drivers[] = {GDISP_DRIVER_LIST};

				for(pa = drivers; pa < &drivers[sizeof(drivers)/sizeof(drivers[0])]) {
					for(cnt = pa->instances; cnt; cnt--)
						gdriverRegister(pa->vmt);
				}
			}
		#else
			extern struct GDISPVMT GDISP_VMT;
			gdriverRegister((GDriver *)&GDISP_VMT);
		#endif
	#endif
}


GDriver *gdriverRegister(const GDriverVMT *vmt) {
	GDriver *pd;
	GDriver *dtail;
	int		dinstance, sinstance;

	// Loop to find the driver instance and the system instance numbers
	dinstance = sinstance = 0;
	for(pd = dhead; pd; dtail = pd, pd = pd->driverchain) {
		if (pd->vmt == vmt)
			dinstance++;
		if (pd->vmt->type == vmt->type)
			sinstance++;
	}

	// Get a new driver instance of the correct size and initialize it
	pd = gfxAlloc(vmt->objsize);
	if (!pd)
		return 0;
	pd->driverchain = 0;
	pd->vmt = vmt;
	if (vmt->init && !vmt->init(pd, dinstance, sinstance)) {
		gfxFree(pd);
		return 0;
	}

	// Add it to the driver chain
	if (dhead)
		dtail->driverchain = pd;
	else
		dhead = pd;
}

void gdriverUnRegister(GDriver *driver) {
	GDriver		*pd;

	// Safety
	if (!driver)
		return;

	// Remove it from the list of drivers
	if (dhead == driver)
		dhead = driver->driverchain;
	else {
		for(pd = dhead; pd->driverchain; pd = pd->driverchain) {
			if (pd->driverchain == driver) {
				pd->driverchain = driver->driverchain;
				break;
			}
		}
	}

	// Call the deinit()
	if (driver->vmt->deinit)
		driver->vmt->dinit(driver);

	// Cleanup
	gfxFree(driver);
}

GDriver *gdriverGetInstance(uint16_t type, int instance) {
	GDriver		*pd;
	int			sinstance;

	// Loop to find the system instance
	sinstance = 0;
	for(pd = dhead; pd; pd = pd->driverchain) {
		if (pd->vmt->type == type) {
			if (sinstance == instance)
				return pd;
			sinstance++;
		}
	}
	return 0;
}

int gdriverInstanceCount(uint16_t type) {
	GDriver		*pd;
	int			sinstance;

	// Loop to count the system instances
	sinstance = 0;
	for(pd = dhead; pd; pd = pd->driverchain) {
		if (pd->vmt->type == type)
			sinstance++;
	}
	return sinstance;
}

#endif /* GFX_NEED_GDRIVER */
