/*
 * This file is subject to the terms of the GFX License. If a copy of
 * the license was not distributed with this file, you can obtain one at:
 *
 *              http://ugfx.org/license.html
 */

/**
 * @file    src/gadc/driver.h
 * @brief   GADC - Periodic ADC driver header file.
 *
 * @defgroup Driver Driver
 * @ingroup GADC
 * @{
 */

#ifndef _GADC_LLD_H
#define _GADC_LLD_H

#include "gfx.h"

#if GFX_USE_GADC || defined(__DOXYGEN__)

/*===========================================================================*/
/* Type definitions                                                          */
/*===========================================================================*/

/**
 * @brief				The structure passed to start a timer conversion
 * @note				We use the structure instead of parameters purely to save
 * 						interrupt stack space which is very limited in some platforms.
 * @{
 */
typedef struct GadcLldTimerData_t {
	uint32_t		physdev;		/* @< Which physical ADC devices/channels to use. Filled in by High Level Code */
	uint32_t		frequency;		/* @< The conversion frequency. Filled in by High Level Code */
	GDataBuffer		*pdata;			/* @< The buffer to put the ADC samples into. */
	bool_t			now;			/* @< Trigger the first conversion now rather than waiting for the first timer interrupt (if possible) */
	} GadcLldTimerData;
/* @} */

/**
 * @brief				The structure passed to start a non-timer conversion
 * @note				We use the structure instead of parameters purely to save
 * 						interrupt stack space which is very limited in some platforms.
 * @{
 */
typedef struct GadcLldNonTimerData_t {
	uint32_t		physdev;		/* @< A value passed to describe which physical ADC devices/channels to use. */
	adcsample_t		*buffer;		/* @< The static buffer to put the ADC samples into. */
	} GadcLldNonTimerData;
/* @} */

/**
 * @brief				This can be incremented by the low level driver if a timer interrupt is missed.
 * @details				Defined in the high level GADC code.
 *
 * @notapi
 */
extern volatile bool_t GADC_Timer_Missed;

/*===========================================================================*/
/* External declarations.                                                    */
/*===========================================================================*/

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief				These routines are the callbacks that the driver uses.
 * @details				Defined in the high level GADC code.
 *
 * @notapi
 * @{
 */
	/**
	 * @brief	The last conversion requested is now complete
	 */
	void gadcDataReadyI(void);

	/**
	 * @brief	The last conversion requested failed
	 */
	void gadcDataFailI(void);
/**
 * @}
 */

/**
 * @brief				Initialise the driver
 *
 * @api
 */
void gadc_lld_init(void);

/**
 * @brief				Start a periodic timer for high frequency conversions.
 *
 * @param[in] pgtd		The structure containing the sample frequency and physical device to use.
 *
 * @note				The exact meaning of physdev is hardware dependent. It describes the channels
 * 						the will be used later on when a "timer" conversion is actually scheduled.
 * @note				It is assumed that the timer is capable of free-running even when the ADC
 * 						is stopped or doing something else.
 * @details				When a timer interrupt occurs a conversion should start if there is a "timer" conversion
 * 						active.
 * @note				Timer interrupts occurring before @p gadc_lld_adc_timerI() has been called,
 * 						if  @p gadc_lld_adc_timerI() has been called quick enough, or while
 * 						a non-timer conversion is active should be ignored other than (optionally) incrementing
 * 						the GADC_Timer_Missed variable.
 * @note				The pdata and now members of the pgtd structure are now yet valid.
 *
 * @api
 */
void gadc_lld_start_timer(GadcLldTimerData *pgtd);

/**
 * @brief				Stop the periodic timer for high frequency conversions.
 * @details				Also stops any current "timer" conversion (but not a current "non-timer" conversion).
 *
 * @param[in] pgtd		The structure containing the sample frequency and physical device to use.
 *
 * @note				After this function returns there should be no more calls to @p gadcDataReadyI()
 * 						or @p gadcDataFailI() in relation to timer conversions.
 * @api
 */
void gadc_lld_stop_timer(GadcLldTimerData *pgtd);

/**
 * @brief				Start a set of "timer" conversions.
 * @details				Starts a series of conversions triggered by the timer.
 *
 * @param[in] pgtd		Contains the parameters for the timer conversion.
 *
 * @note				The exact meaning of physdev is hardware dependent. It is likely described in the
 * 						drivers gadc_lld_config.h
 * @note				The driver should call @p gadcDataReadyI() when it completes the operation
 * 						or @p gadcDataFailI() on an error.
 * @note				The high level code ensures that this is not called while a non-timer conversion is in
 * 						progress
 *
 * @iclass
 */
void gadc_lld_adc_timerI(GadcLldTimerData *pgtd);

/**
 * @brief				Start a "non-timer" conversion.
 * @details				Starts a single conversion now.
 *
 * @param[in] pgntd		Contains the parameters for the non-timer conversion.
 *
 * @note				The exact meaning of physdev is hardware dependent. It is likely described in the
 * 						drivers gadc_lld_config.h
 * @note				The driver should call @p gadcDataReadyI() when it completes the operation
 * 						or @p gadcDataFailI() on an error.
 * @note				The high level code ensures that this is not called while a timer conversion is in
 * 						progress
 *
 * @iclass
 */
void gadc_lld_adc_nontimerI(GadcLldNonTimerData *pgntd);

#ifdef __cplusplus
}
#endif

#endif /* GFX_USE_GADC */

#endif /* _GADC_LLD_H */
/** @} */
