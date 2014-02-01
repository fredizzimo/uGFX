/*
 * This file is subject to the terms of the GFX License. If a copy of
 * the license was not distributed with this file, you can obtain one at:
 *
 *              http://ugfx.org/license.html
 */

/**
 * @file    src/gos/freertos.c
 * @brief   GOS FreeRTOS Operating System support.
 */
#include "gfx.h"
#include <string.h>

#if INCLUDE_vTaskDelay != 1
	#error "GOS: INCLUDE_vTaskDelay must be defined in FreeRTOSConfig.h"
#endif

#if configUSE_MUTEXES != 1
	#error "GOS: configUSE_MUTEXES must be defined in FreeRTOSConfig.h"
#endif

#if configUSE_COUNTING_SEMAPHORES  != 1
	#error "GOS: configUSE_COUNTING_SEMAPHORES must be defined in FreeRTOSConfig.h"
#endif

void _gosInit(void)
{
	// The user must call vTaskStartScheduler() himself before he calls gfxInit(). 
}

void* gfxRealloc(void *ptr, size_t oldsz, size_t newsz)
{
	void *np;

	if (newsz <= oldsz)
		return ptr;

	np = gfxAlloc(newsz);
	if (!np)
		return 0;

	if (oldsz) {
		memcpy(np, ptr, oldsz);
		vPortFree(ptr);
	}

	return np;
}

void gfxSleepMilliseconds(delaytime_t ms)
{
	// Implement this
}

void gfxSleepMicroseconds(delaytime_t ms)
{
	// Implement this
}

portTickType MS2ST(portTickType ms)
{
	// Verify this

	uint64_t	val;

	if(configTICK_RATE_HZ == 1000) {	// gain time because no test to do in most case
		return ms;
	}

	val = ms;
	val *= configTICK_RATE_HZ;
	val += 999;
	val /= 1000;

	return val;
}

void gfxSemInit(gfxSem* psem, semcount_t val, semcount_t limit)
{
	if (val > limit)
		val = limit;

	psem->counter = val;
	psem->limit = limit;
	psem->sem = xSemaphoreCreateCounting(limit,val);

	vTraceSetSemaphoreName(psem->sem, "uGFXSema"); // for FreeRTOS+Trace debug
}

void gfxSemDestroy(gfxSem* psem)
{
	vSemaphoreDelete(psem->sem);
}

bool_t gfxSemWait(gfxSem* psem, delaytime_t ms)
{
	psem->counter--;

	if(xSemaphoreTake(psem->sem, MS2ST(ms)) == pdPASS)
		return TRUE;

	psem->counter++;

	return FALSE;
}

void gfxSemSignal(gfxSem* psem)
{
	taskENTER_CRITICAL();

	if(psem->counter < psem->limit) {
		psem->counter++;
		xSemaphoreGive(psem->sem);
	}

	taskYIELD();
	taskEXIT_CRITICAL();
}

void gfxSemSignalI(gfxSem* psem)
{
	portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;

	if(psem->counter < psem->limit) {
		psem->counter++;
		xSemaphoreGiveFromISR(psem->sem,&xHigherPriorityTaskWoken);
	}
}

gfxThreadHandle gfxThreadCreate(void *stackarea, size_t stacksz, threadpriority_t prio, DECLARE_THREAD_FUNCTION((*fn),p), void *param)
{
	xTaskHandle task = NULL;
	stacksz = (size_t)stackarea;

	if (stacksz < configMINIMAL_STACK_SIZE)
		stacksz = configMINIMAL_STACK_SIZE;

	if (xTaskCreate(fn, (signed char*)"uGFX_TASK", stacksz, param, prio, &task )!= pdPASS) {
		for (;;);
	}

	return task;
}

#endif /* GFX_USE_OS_FREERTOS */
/** @} */
#endif /* USE_UGFX */
