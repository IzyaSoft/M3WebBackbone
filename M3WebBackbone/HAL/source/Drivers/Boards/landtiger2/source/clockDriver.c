#include "hal.h"
#include "LPC17xx.h"

void (*externalSysTickHandler)();

#ifndef SYSTICKHANDLER_REDEFINED
void SysTick_Handler()
{
	(*externalSysTickHandler)();
}
#endif

void ConfigureSystemClock()
{
    SystemInit();
}

void SetSysTickHandler(void (*sysTickHandler)(), uint32_t systemClockDivider)
{
    SysTick_Config(SystemCoreClock / systemClockDivider);
    externalSysTickHandler = sysTickHandler;
}
