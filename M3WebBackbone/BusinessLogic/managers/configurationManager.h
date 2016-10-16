#ifndef CONFIGURATION_MANAGER_H
#define CONFIGURATION_MANAGER_H

#ifdef LPC1768
#include "LPC17xx.h"
#include "system_LPC17xx.h"
#endif
#include "core_cm3.h"

void InitializeClocks();
void InitializeGPIO();
void InitializeInterrupts(uint32_t value);

//LPC1768
void InitializeLPC1768Clock();
void InitializeLPC1768GPIO();

#endif
