#ifndef STATIC_ALLOCATION_IMPL_H
#define STATIC_ALLOCATION_IMPL_H

#include "FreeRTOS.h"

#if configSUPPORT_STATIC_ALLOCATION
void vApplicationGetIdleTaskMemory(StaticTask_t **ppxIdleTaskTCBBuffer, StackType_t **ppxIdleTaskStackBuffer,
		                           uint32_t *pulIdleTaskStackSize);
#endif

#if configSUPPORT_STATIC_ALLOCATION && configUSE_TIMERS
void vApplicationGetTimerTaskMemory(StaticTask_t **ppxTimerTaskTCBBuffer, StackType_t **ppxTimerTaskStackBuffer,
		                            uint32_t *pulTimerTaskStackSize);
#endif


#endif
