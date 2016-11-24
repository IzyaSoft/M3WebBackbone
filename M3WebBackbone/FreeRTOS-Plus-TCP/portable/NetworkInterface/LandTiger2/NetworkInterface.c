/* Standard includes. */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
/* FreeRTOS includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
/* FreeRTOS+TCP includes. */
#include "FreeRTOS_IP.h"
#include "FreeRTOS_Sockets.h"
#include "FreeRTOS_IP_Private.h"
#include "NetworkBufferManagement.h"
#include "NetworkInterface.h"
// Hardware
#include "hal.h"
#include "ethernetDriver.h"
#include "LPC17xx.h"         //todo: umv: get rid of this
// Debug
#include "debugPrintFunctions.h"

#define MAX_TX_ATTEMPTS 10
#define TX_CHECK_BUFFER_TIME (pdMS_TO_TICKS(2UL))
#define EthernetIrqHandler ENET_IRQHandler

static TaskHandle_t eMACTaskHandle;
static SemaphoreHandle_t xEthernetMACRxEventSemaphore = NULL;

/* The queue used to communicate Ethernet events with the IP task. */
extern QueueHandle_t xNetworkEventQueue;
extern struct NetworkConfiguration networkConfiguration;

static void prvEMACTask(void *pvParameters)
{
    const TickType_t xPauseTime = pdMS_TO_TICKS(1UL);
    size_t dataLength;
    const uint16_t cRCLength = 4;
    NetworkBufferDescriptor_t* networkBuffer;
    IPStackEvent_t rxEvent = {eNetworkRxEvent, NULL};
    struct EthernetBuffer rxBuffer;

    for(;;)
    {
        /* Wait for the EMAC interrupt to indicate that another packet has been
         * received.  The while() loop is only needed if INCLUDE_vTaskSuspend is
         * set to 0 in FreeRTOSConfig.h. */
        while(xSemaphoreTake(xEthernetMACRxEventSemaphore, portMAX_DELAY) == pdFALSE);
        //printf("Semaphore captured ... \n\r");
        /* At least one packet has been received. */
        while(CheckIsDataAvailable())
        {
            //printf("After index check ... \n\r");
            // Obtain the length, minus the CRC.  The CRC is four bytes but the length is already minus 1.
            dataLength = (size_t) CheckAvailableDataSize();//GetReceivedDataSize() - (cRCLength - 1);
            if(dataLength > 0)
            {
                networkBuffer = pxGetNetworkBufferWithDescriptor(0, (TickType_t )0);
                networkBuffer->xDataLength = dataLength;
                rxBuffer._buffer = networkBuffer->pucEthernetBuffer;
                rxBuffer._bufferCapacity = networkBuffer->xDataLength;
                Read(&rxBuffer);

                rxEvent.pvData = (void *) networkBuffer;

                // printf("Received data: ");
                // printStringHexSymbols(networkBuffer->pucEthernetBuffer, dataLength, -1);

                // Data was received and stored.  Send a message to the IP task to let it know.
                if(xSendEventStructToIPTask(&rxEvent, (TickType_t)0) == pdFAIL)
                {
                    vReleaseNetworkBufferAndDescriptor(networkBuffer);
                    iptraceETHERNET_RX_EVENT_LOST();
                }
                else
                {
                    iptraceETHERNET_RX_EVENT_LOST();
                }
                vReleaseNetworkBufferAndDescriptor(networkBuffer);
                iptraceNETWORK_INTERFACE_RECEIVE();
            }
            //UpdateRxConsumeIndex();
            vTaskDelay(xPauseTime);
        }

    }
    vTaskDelete(NULL);
}

BaseType_t xStartEmacTask()
{
    return xTaskCreate(prvEMACTask, "LANDTIGER2EMAC", configEMAC_TASK_STACK_SIZE, NULL, configMAX_PRIORITIES - 2, NULL); //&eMACTaskHandle);
}

BaseType_t xNetworkInterfaceInitialise( void )
{
    vSemaphoreCreateBinary(xEthernetMACRxEventSemaphore);
    // Interrupts configure
    LPC_EMAC->IntEnable &= ~(INT_TX_DONE);
    NVIC_SetPriority(ENET_IRQn, configEMAC_INTERRUPT_PRIORITY);
    NVIC_EnableIRQ(ENET_IRQn);
    struct EthernetConfiguration ethernetConfiguration;
    GetNetworkConfiguration(&networkConfiguration);
    unsigned char revertedMac[] =
    {
        networkConfiguration._macAddress[5],
        networkConfiguration._macAddress[4],
        networkConfiguration._macAddress[3],
        networkConfiguration._macAddress[2],
        networkConfiguration._macAddress[1],
        networkConfiguration._macAddress[0]
    };
    ethernetConfiguration._macAddress = revertedMac;
    ethernetConfiguration._useAutoNegotiation = 1;

    unsigned char result = InitializeEthrernet(&ethernetConfiguration);
    xStartEmacTask();
    return result != 0;
}

BaseType_t xNetworkInterfaceOutput(NetworkBufferDescriptor_t * const pxNetworkBuffer, BaseType_t xReleaseAfterSend)
{
    BaseType_t result = pdFAIL;
    struct EthernetBuffer txBuffer;
    /* Attempt to obtain access to a Tx buffer. */
    for(uint32_t x = 0; x < MAX_TX_ATTEMPTS; x++)
    {
        if( CheckTransmissionAvailable() == 1)  //todo add check!
        {
            //todo: umv: packetization
            if( pxNetworkBuffer->xDataLength < ETH_MAX_FLEN )
            {
                txBuffer._buffer = pxNetworkBuffer->pucEthernetBuffer;
                txBuffer._bufferCapacity = pxNetworkBuffer->xDataLength;
                txBuffer._storedBytes = pxNetworkBuffer->xDataLength;
                Write(&txBuffer);
                iptraceNETWORK_INTERFACE_TRANSMIT();
                result = pdPASS;
                break;
            }
        }
        else vTaskDelay(TX_CHECK_BUFFER_TIME);
    }

    vReleaseNetworkBufferAndDescriptor(pxNetworkBuffer);
    return pdPASS;
}

void vNetworkInterfaceAllocateRAMToBuffers(NetworkBufferDescriptor_t pxNetworkBuffers[ ipconfigNUM_NETWORK_BUFFER_DESCRIPTORS ] )
{
    // In our first version of driver we are using BufferAllocation_2, therefore we do, n't need to setup this function
}

BaseType_t xGetPhyLinkStatus(void)
{
    return pdTRUE;
}

void EthernetIrqHandler()
{
     uint32_t interruptCause;

     while((interruptCause = LPC_EMAC->IntStatus) != 0)
     {
         //printf("Interrupt raised ... \n\r");
         /* Clear the interrupt. */
         LPC_EMAC->IntClear = interruptCause;

         /* Clear fatal error conditions.  NOTE:  The driver does not clear all
          * errors, only those actually experienced.  For future reference, range
          * errors are not actually errors so can be ignored. */
         if((interruptCause & INT_TX_UNDERRUN) != 0)
             LPC_EMAC->Command |= CR_TX_RES;

         /* Unblock the deferred interrupt handler task if the event was an Rx. */
         if((interruptCause & INT_RX_DONE) != 0)
             xSemaphoreGiveFromISR(xEthernetMACRxEventSemaphore, NULL);
     }

     /* ulInterruptCause is used for convenience here.  A context switch is
      * wanted, but coding portEND_SWITCHING_ISR( 1 ) would likely result in a
      * compiler warning. */
     portEND_SWITCHING_ISR(interruptCause);
}


