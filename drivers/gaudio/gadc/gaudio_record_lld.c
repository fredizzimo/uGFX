/*
 * This file is subject to the terms of the GFX License. If a copy of
 * the license was not distributed with this file, you can obtain one at:
 *
 *              http://ugfx.org/license.html
 */

/**
 * @file    drivers/gaudio/gadc/gaudio_record_lld.c
 * @brief   GAUDIO - Record Driver file for using the cpu ADC (via GADC).
 *
 * @addtogroup GAUDIO
 *
 * @{
 */

/**
 * We are now implementing the driver - pull in our channel table
 * from the board definitions.
 */
#define GAUDIO_RECORD_IMPLEMENTATION


#include "gfx.h"

#if GFX_USE_GAUDIO && GAUDIO_NEED_RECORD

/* Double check the GADC system is turned on */
#if !GFX_USE_GADC
	#error "GAUDIO - The GADC driver for GAUDIO requires GFX_USE_GADC to be TRUE"
#endif

/* Include the driver defines */
#include "src/gaudio/driver_record.h"

/*===========================================================================*/
/* External declarations.                                                    */
/*===========================================================================*/

void gaudin_lld_init(const gaudin_params *paud) {
	/* Setup the high speed GADC */
	gadcHighSpeedInit(gaudin_lld_physdevs[paud->channel], paud->frequency, paud->buffer, paud->bufcount, paud->samplesPerEvent);

	/* Register ourselves for ISR callbacks */
	gadcHighSpeedSetISRCallback(GAUDIN_ISR_CompleteI);

	/**
	 * The gadc driver handles any errors for us by restarting the transaction so there is
	 * no need for us to setup anything for GAUDIN_ISR_ErrorI()
	 */
}

void gaudin_lld_start(void) {
	gadcHighSpeedStart();
}

void gaudin_lld_stop(void) {
	gadcHighSpeedStop();
}

#endif /* GFX_USE_GAUDIO && GAUDIO_NEED_RECORD */
/** @} */
