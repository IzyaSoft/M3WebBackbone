/* Hardware*/
#include "driverConfig.h"
#include "hal.h"
/* FreeRTOS includes. */
#include <FreeRTOS.h>
#include "task.h"
#include "timers.h"
#include "queue.h"
#include "semphr.h"
#include "diagnostic.h"
#include "pingManager.h"
#include "networkEstablishHelper.h"
/* Free RTOS Configs*/
#include "FreeRTOSConfig.h"
#include "FreeRTOSFATConfig.h"
#include "FreeRTOSIPConfig.h"

/* Application*/
#include "networkManager.h"
#include "staticAllocationImpl.h"
#include "FreeRTOS_TCP_server.h"
/* Debug */
#include "semihosting.h"

#define WEB_SERVER_STACK_SIZE 512
#define WEB_SERVER_TASK_PRIORITY configMAX_PRIORITIES / 2

struct NetworkConfiguration networkConfiguration;

//extern void initialise_monitor_handles(void); /* prototype */
// global variables
/*const uint8_t ETHERNET_MAC_ADDRESS[] = {0x00, 0x33, 0x11, 0x66, 0x22, 0xEE};
// ip settings
const uint8_t APP_DEFAULT_IP_ADDRESS[] = {192, 168, 200, 5};
const uint8_t APP_DEFAULT_NETMASK[] = {255, 255, 255, 0};
const uint8_t APP_DEFAULT_GATEWAY[] = {192, 168, 200, 1};
const uint8_t APP_DEFAULT_NAMESERVER[] = {192, 168, 200, 1};*/
// Task Handle
static TaskHandle_t xWebServerTaskHandle = NULL;
static TaskHandle_t xLedBlinkTaskHandle = NULL;

void prvWebServerTask(void *pvParameters);
void prvLedBlinkTask(void *pvParameters);

int main()
{
	// Debug
    // disableRAMWriteBufferization();
	// Hardware Initialization
    ConfigureSystemClock();
    printf("m3webbackbone started:%s \r\n", "v0.9beta");
    // IP initialization in FreeRTOS
    GetNetworkConfiguration(&networkConfiguration);
    FreeRTOS_IPInit(networkConfiguration._ipAddress, networkConfiguration._netmask, networkConfiguration._gateway, networkConfiguration._nameServer, networkConfiguration._macAddress);
    // Tasks
    BaseType_t result = xTaskCreate(prvWebServerTask, "M3WebServer", WEB_SERVER_STACK_SIZE, NULL, tskIDLE_PRIORITY, &xWebServerTaskHandle);
    result = xTaskCreate(prvLedBlinkTask, "Blink", configMINIMAL_STACK_SIZE, NULL, 2, &xLedBlinkTaskHandle);

    // to do: add other tasks
    vTaskStartScheduler();
    // start schedule
    for(;;) { }
    return 0;
}

void prvLedBlinkTask(void *pvParameters)
{
    //#define LED1_OPTION 0xA0
    //#define LED2_OPTION 0x0F
    #define LED_PORT_NUMBER 2
    #define LED_PORT_MASK   0x000000FF
    uint32_t ledValue = 0x0F;
    const TickType_t xPauseTime = pdMS_TO_TICKS(100UL);
    ConfigureLedPort(LED_PORT_NUMBER, LED_PORT_MASK, ledValue);

    //LPC_GPIO2->FIODIR |= 0x000000ff;  //P2.0...P2.7 Output LEDs on PORT2 defined as Output

    for(;;)
    {
    	//LPC_GPIO2->FIOPIN = 0x00;
        //LPC_GPIO2->FIOPIN |= LED1_OPTION;
        vTaskDelay(xPauseTime);
        ledValue = 0xF0;
        SetLedsValue(LED_PORT_NUMBER, ledValue);
        //LPC_GPIO2->FIOPIN &= LED1_OPTION;
        //LPC_GPIO2->FIOPIN |= LED2_OPTION;
        vTaskDelay(xPauseTime);
        ledValue = 0x0F;
        SetLedsValue(LED_PORT_NUMBER, ledValue);
    }
}

void prvWebServerTask(void *pvParameters)
{
    TCPServer_t *pxTCPServer = NULL;
    const TickType_t xInitialBlockTime = pdMS_TO_TICKS(5000UL);

    /* A structure that defines the servers to be created.  Which servers are
    included in the structure depends on the mainCREATE_HTTP_SERVER and
    mainCREATE_FTP_SERVER settings at the top of this file. */
    static const struct xSERVER_CONFIG xServerConfiguration[] =
    {
        /* Server type, port number, backlog, root dir. */
        {eSERVER_HTTP, 80, 12, configHTTP_ROOT},
        /* Server type, port number, backlog, root dir. */
        //{eSERVER_FTP, 21, 12, ""}
    };

    /* Remove compiler warning about unused parameter. */
    ( void ) pvParameters;

	/* Initialization is completed, rising priority*/
    vTaskPrioritySet(xWebServerTaskHandle, WEB_SERVER_TASK_PRIORITY);

    /* Wait until the network is up before creating the servers.  The notification is given from the network event hook. */
    ulTaskNotifyTake(pdTRUE, xInitialBlockTime);

    /* Create the servers defined by the xServerConfiguration array above. */
    pxTCPServer = FreeRTOS_CreateTCPServer(xServerConfiguration, sizeof(xServerConfiguration) / sizeof(xServerConfiguration[0]));
    configASSERT(pxTCPServer);

    /* Run the HTTP and/or FTP servers, as configured above. */
    for( ;; )
    {
        FreeRTOS_TCPServerWork(pxTCPServer, xInitialBlockTime);
    }

    vTaskDelete(NULL);
}

void vApplicationStackOverflowHook(TaskHandle_t pxTask, signed char *pcTaskName)
{
	( void ) pcTaskName;
	( void ) pxTask;

	/* Run time stack overflow checking is performed if
	configCHECK_FOR_STACK_OVERFLOW is defined to 1 or 2.  This hook
	function is called if a stack overflow is detected. */
	taskDISABLE_INTERRUPTS();
	for( ;; );
}
/*-----------------------------------------------------------*/

void vApplicationMallocFailedHook( void )
{
	/* vApplicationMallocFailedHook() will only be called if
	configUSE_MALLOC_FAILED_HOOK is set to 1 in FreeRTOSConfig.h.  It is a hook
	function that will get called if a call to pvPortMalloc() fails.
	pvPortMalloc() is called internally by the kernel whenever a task, queue,
	timer or semaphore is created.  It is also called by various parts of the
	demo application.  If heap_1.c or heap_2.c are used, then the size of the
	heap available to pvPortMalloc() is defined by configTOTAL_HEAP_SIZE in
	FreeRTOSConfig.h, and the xPortGetFreeHeapSize() API function can be used
	to query the size of free heap space that remains (although it does not
	provide information on how the remaining heap might be fragmented). */
	taskDISABLE_INTERRUPTS();
	for( ;; );
}
