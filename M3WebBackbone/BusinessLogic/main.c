/* FreeRTOS includes. */
#include <FreeRTOS.h>
#include "task.h"
#include "timers.h"
#include "queue.h"
#include "semphr.h"

/* Free RTOS Configs*/
#include "FreeRTOSConfig.h"
#include "FreeRTOSFATConfig.h"
#include "FreeRTOSIPConfig.h"

/* Hardware*/
#include "LPC17xx.h"
#include "core_cm3.h"

/* Application*/
#include "staticAllocationImpl.h"
#include "configurationManager.h"
#include "FreeRTOS_TCP_server.h"

#define mainWEB_SERVER_STACK_SIZE 4096

// global variables
const uint8_t ETHERNET_MAC_ADDRESS[] = {0xAA, 0x33, 0x00, 0x66, 0x22, 0xEE};
// ip settings
const uint8_t APP_DEFAULT_IP_ADDRESS[] = {192, 168, 200, 5};
const uint8_t APP_DEFAULT_NETMASK[] = {255, 255, 255, 0};
const uint8_t APP_DEFAULT_GATEWAY[] = {192, 168, 200, 1};
const uint8_t APP_DEFAULT_NAMESERVER[] = {192, 168, 200, 1};
// Task Handle
static TaskHandle_t xServerWorkTaskHandle = NULL;

void prvServerWorkTask(void *pvParameters);

int main()
{
	InitializeClocks();

	// IP initialization in FreeRTOS
	FreeRTOS_IPInit(APP_DEFAULT_IP_ADDRESS, APP_DEFAULT_NETMASK, APP_DEFAULT_GATEWAY, APP_DEFAULT_NAMESERVER, ETHERNET_MAC_ADDRESS);
	BaseType_t result = xTaskCreate(prvServerWorkTask, "M3WebServer", mainWEB_SERVER_STACK_SIZE, NULL, tskIDLE_PRIORITY, &xServerWorkTaskHandle);
	// to do: add other tasks
	vTaskStartScheduler();
	while(1) { }
    return 0;
}

void prvServerWorkTask(void *pvParameters)
{

}
