#include "configurationManager.h"

// common functions
void InitializeClocks()
{
#ifdef LPC1768
	InitializeLPC1768Clock();
#endif
}

void InitializeGPIO()
{
#ifdef LPC1768
	SystemCoreClockUpdate();
	InitializeLPC1768GPIO();
#endif
}

void InitializeInterrupts(uint32_t value)
{
	NVIC_SetPriorityGrouping(value);
}

// Hardware specific functions
void InitializeLPC1768Clock()
{
    SystemInit();
}

void InitializeLPC1768GPIO()
{

}
