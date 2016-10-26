#ifndef HARDFAULT_DIAGNOSTIC_H
#define HARDFAULT_DIAGNOSTIC_H

#include <stdint.h>

#define vHardFaultHandler HardFault_Handler
#define vMemManageHandler MemManage_Handler

void checkCFSRValue(uint32_t CFSRValue);
void checkBusError(uint32_t CFSRValue, uint32_t regValue);
void checkMemoryManagementError(uint32_t CFSRValue, uint32_t regValue);
void getRegistersFromStack(uint32_t* pulFaultStackAddress, uint32_t faultSource);

#endif
