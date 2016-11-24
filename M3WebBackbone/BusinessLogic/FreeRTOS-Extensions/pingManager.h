#ifndef PING_MANAGER_H
#define PING_MANAGER_H


#include "FreeRTOS.h"
#include "queue.h"
#include "list.h"
#include "FreeRTOS_IP.h"

//extern QueueHandle_t xPingReplyQueue;

void vApplicationPingReplyHook(ePingReplyStatus_t eStatus, uint16_t usIdentifier);
//BaseType_t vSendPing(const int8_t *pcIPAddress);

#endif
