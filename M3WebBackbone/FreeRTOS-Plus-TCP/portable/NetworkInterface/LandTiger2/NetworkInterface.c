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

static TaskHandle_t eMACTaskHandle;

static void prvEMACTask(void *pvParameters)
{
	const TickType_t xInitialBlockTime = pdMS_TO_TICKS(50UL);
    // function of LPC1768 EMAC Handling
	// todo: umv: check interrupts flags: RxDone, TxDone
	// todo: umv: if RxDone pass data from buffers to IP stack via xSendEventStructToIpTask
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
	EMAC_CFG_Type emacConfig;
	emacConfig.Mode = ETHERNET_MODE;
	emacConfig.pbEMAC_Addr = ETHERNET_MAC_ADDRESS;

	Bool result = InitializeEthernetMAC(&emacConfig);
	return result == TRUE;
}

BaseType_t xNetworkInterfaceOutput(NetworkBufferDescriptor_t * const pxNetworkBuffer, BaseType_t xReleaseAfterSend)
{
	//EMAC_PACKETBUF_Type txBuffer;
	//txBuffer.pbDataBuf = pxNetworkBuffer->pucEthernetBuffer;
	//txBuffer.ulDataLen = pxNetworkBuffer->xDataLength; // Maybe should add +1 for CRC
	//EMAC_WritePacketBuffer(&txBuffer);
	return pdTRUE;  // should think about return value
}

void vNetworkInterfaceAllocateRAMToBuffers(NetworkBufferDescriptor_t pxNetworkBuffers[ ipconfigNUM_NETWORK_BUFFER_DESCRIPTORS ] )
{
    // In our first version of driver we are using BufferAllocation_2, therefore we don't need to setup this function
	// todo: umv: make this function
}

BaseType_t xGetPhyLinkStatus( void )
{
    return pdTRUE;
}


