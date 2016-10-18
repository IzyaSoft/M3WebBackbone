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

//LPC1768 Include
#include "landtiger2_emac.h"

#define MAX_TX_ATTEMPTS 10
#define TX_CHECK_BUFFER_TIME (pdMS_TO_TICKS(2UL))
#define EthernetIrqHandler ENET_IRQHandler

static TaskHandle_t eMACTaskHandle;
static SemaphoreHandle_t xEthernetMACRxEventSemaphore = NULL;

static void prvEMACTask(void *pvParameters)
{
	const TickType_t xInitialBlockTime = pdMS_TO_TICKS(50UL);
	for(;;)
	{
		// if no data then sleep
        vTaskDelay(xInitialBlockTime);
	}
	vTaskDelete(NULL);
}

BaseType_t xStartEmacTask()
{
	return xTaskCreate(prvEMACTask, "LPC1768EMAC", configEMAC_TASK_STACK_SIZE, NULL, configMAX_PRIORITIES - 3, &eMACTaskHandle);
}

BaseType_t xNetworkInterfaceInitialise( void )
{
	vSemaphoreCreateBinary(xEthernetMACRxEventSemaphore);
	// Interrupts configure
	NVIC_SetPriority(ENET_IRQn, configEMAC_INTERRUPT_PRIORITY);
	NVIC_EnableIRQ(ENET_IRQn);
	EMAC_CFG_Type emacConfig;
	emacConfig.Mode = ETHERNET_MODE;
	emacConfig.pbEMAC_Addr = ETHERNET_MAC_ADDRESS;

	Bool result = InitializeEthernetMAC(&emacConfig);
	return result == TRUE;
}

BaseType_t xNetworkInterfaceOutput(NetworkBufferDescriptor_t * const pxNetworkBuffer, BaseType_t xReleaseAfterSend)
{
    /* Attempt to obtain access to a Tx buffer. */
    for(uint32_t x = 0; x < MAX_TX_ATTEMPTS; x++)
    {
        if(CheckTransmitIndex())
        {
            //todo: umv: packetization
        	if( pxNetworkBuffer->xDataLength < ETH_MAX_FLEN )
            {
                EMAC_PACKETBUF_Type txBuffer;
                txBuffer.pbDataBuf = pxNetworkBuffer->pucEthernetBuffer;
                txBuffer.ulDataLen = pxNetworkBuffer->xDataLength; // Maybe should add +1 for CRC
                WriteData(&txBuffer);
                return pdTRUE;
            }
        }
        else vTaskDelay(TX_CHECK_BUFFER_TIME);
    }
	return pdFALSE;
}

void vNetworkInterfaceAllocateRAMToBuffers(NetworkBufferDescriptor_t pxNetworkBuffers[ ipconfigNUM_NETWORK_BUFFER_DESCRIPTORS ] )
{
    // In our first version of driver we are using BufferAllocation_2, therefore we don't need to setup this function
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
         /* Clear the interrupt. */
    	 LPC_EMAC->IntClear = interruptCause;

    	 /* Clear fatal error conditions.  NOTE:  The driver does not clear all
    	  * errors, only those actually experienced.  For future reference, range
    	  * errors are not actually errors so can be ignored. */
    	 if((interruptCause & EMAC_INT_TX_UNDERRUN) != 0)
             LPC_EMAC->Command |= EMAC_CR_TX_RES;

         /* Unblock the deferred interrupt handler task if the event was an Rx. */
         if((interruptCause & EMAC_INT_RX_DONE) != 0)
             xSemaphoreGiveFromISR(xEthernetMACRxEventSemaphore, NULL);
     }

     /* ulInterruptCause is used for convenience here.  A context switch is
      * wanted, but coding portEND_SWITCHING_ISR( 1 ) would likely result in a
      * compiler warning. */
     portEND_SWITCHING_ISR(interruptCause);
}


