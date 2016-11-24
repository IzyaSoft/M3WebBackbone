#ifndef NETWORH_ESTABLISH_HELPER_H
#define NETWORH_ESTABLISH_HELPER_H

#include "FreeRTOS.h"
#include "list.h"
#include "FreeRTOS_IP.h"

void vApplicationIPNetworkEventHook(eIPCallbackEvent_t eNetworkEvent);

#endif
