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
    TCPServer_t *pxTCPServer = NULL;
    const TickType_t xInitialBlockTime = pdMS_TO_TICKS( 5000UL );
    const TickType_t xSDCardInsertDelay = pdMS_TO_TICKS( 1000UL );

    /* A structure that defines the servers to be created.  Which servers are
    included in the structure depends on the mainCREATE_HTTP_SERVER and
    mainCREATE_FTP_SERVER settings at the top of this file. */
    static const struct xSERVER_CONFIG xServerConfiguration[] =
    {
        #if( mainCREATE_HTTP_SERVER == 1 )
        /* Server type,        port number,    backlog,     root dir. */
        { eSERVER_HTTP,     80,             12,         configHTTP_ROOT },
        #endif

        #if( mainCREATE_FTP_SERVER == 1 )
        /* Server type,        port number,    backlog,     root dir. */
        { eSERVER_FTP,      21,             12,         "" }
        #endif
    };


    /* Remove compiler warning about unused parameter. */
    ( void ) pvParameters;

    /* Can't serve web pages or start an FTP session until the card is
    inserted.  The Xplained Pro hardware cannot generate an interrupt when
    the card is inserted - so periodically look for the card until it is inserted. */

/*    while( ( pxDisk = FF_SDDiskInit( mainSD_CARD_DISK_NAME ) ) == NULL )
    {
        vTaskDelay( xSDCardInsertDelay );
    }*/

    //FreeRTOS_printf( ( "SD card detected, creating example files." ) );
    //prvCreateExampleFiles();
    //FreeRTOS_printf( ( "Example files created." ) );

    /* A disk is going to be created, so register the example file CLI
    commands (which are very basic). */
    //vRegisterFileSystemCLICommands();

    /* The priority of this task can be raised now the disk has been initialised. */
    //vTaskPrioritySet( NULL, mainTCP_SERVER_TASK_PRIORITY );

    /* Wait until the network is up before creating the servers.  The notification is given from the network event hook. */
    //ulTaskNotifyTake( pdTRUE, portMAX_DELAY );

    /* Create the servers defined by the xServerConfiguration array above. */
    //pxTCPServer = FreeRTOS_CreateTCPServer(xServerConfiguration, sizeof( xServerConfiguration ) / sizeof( xServerConfiguration[ 0 ] ) );
    //configASSERT( pxTCPServer );

    //for( ;; )
    //{
        /* The Xplained pro board does not define a pin for detecting the
        removal of the SD card, so periodically check for the cards
        presence. */
        //prvSDCardDetect();

        /* Run the HTTP and/or FTP servers, as configured above. */
        //FreeRTOS_TCPServerWork( pxTCPServer, xInitialBlockTime );
    //}
}
