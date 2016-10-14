#include "configurationManager.h"

#ifdef LPC1768
#include "system_LPC17xx.h"
#endif

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


void InitializeLPC1768Clock()
{
    SystemInit();
}

void InitializeLPC1768GPIO()
{

}
