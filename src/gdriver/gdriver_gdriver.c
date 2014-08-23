/*
 * This file is subject to the terms of the GFX License. If a copy of
 * the license was not distributed with this file, you can obtain one at:
 *
 *              http://ugfx.org/license.html
 */

#include "gfx.h"

#if GFX_USE_GDRIVER

#include "sys_defs.h"

// Define the tables to hold the driver instances.
static GDriver *dhead;

// Definition that allows getting addresses of structures
typedef const struct GDriverVMT const	VMT_EL[1];

// The system initialization.
void _gdriverInit(void) {
	#if GFX_USE_GDISP
	{
		// Both GDISP_CONTROLLER_LIST and GDISP_CONTROLLER_DISPLAYS are defined - create the required numbers of each controller
		#if defined(GDISP_CONTROLLER_LIST) && defined(GDISP_CONTROLLER_DISPLAYS)
			int		i, cnt;


			extern VMT_EL							GDISP_CONTROLLER_LIST;
			static const struct GDriverVMT const *	dclist[GDISP_TOTAL_CONTROLLERS] = {GDISP_CONTROLLER_LIST};
			static const unsigned					dnlist[GDISP_TOTAL_CONTROLLERS] = {GDISP_CONTROLLER_DISPLAYS};
			for(i = 0; i < GDISP_TOTAL_CONTROLLERS; i++) {
				for(cnt = dnlist[i]; cnt; cnt--)
					gdriverRegister(dclist[i]);
			}

		// Only GDISP_CONTROLLER_LIST is defined - create one of each controller
		#elif defined(GDISP_CONTROLLER_LIST)
			int		i;


			extern VMT_EL							GDISP_CONTROLLER_LIST;
			static const struct GDriverVMT const *	dclist[GDISP_TOTAL_CONTROLLERS] = {GDISP_CONTROLLER_LIST};
			for(i = 0; i < GDISP_TOTAL_CONTROLLERS; i++)
				gdriverRegister(dclist[i]);

		// Only GDISP_TOTAL_DISPLAYS is defined - create the required number of the one controller
		#elif GDISP_TOTAL_DISPLAYS > 1
			int		cnt;

			extern VMT_EL GDISPVMT_OnlyOne;
			for(cnt = 0; cnt < GDISP_TOTAL_DISPLAYS; cnt++)
				gdriverRegister(GDISPVMT_OnlyOne);

		// One and only one display
		#else
			extern VMT_EL GDISPVMT_OnlyOne;
			gdriverRegister(GDISPVMT_OnlyOne);
		#endif
	}
	#endif

	// Drivers not loaded yet
	// GINPUT_NEED_MOUSE
	// GINPUT_NEED_DIAL
	// GINPUT_NEED_TOGGLE
	// GINPUT_NEED_KEYBOARD
	// GINPUT_NEED_STRING
	// GFX_USE_GBLOCK
}

// The system de-initialization.
void _gdriverDeinit(void) {
	while(dhead)
		gdriverUnRegister(dhead);
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

	return pd;
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
		driver->vmt->deinit(driver);

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

GDriver *gdriverGetNext(uint16_t type, GDriver *driver) {
	driver = driver ? driver->driverchain : dhead;

	while(driver && driver->vmt->type != type)
		driver = driver->driverchain;

	return driver;
}

#endif /* GFX_USE_GDRIVER */
