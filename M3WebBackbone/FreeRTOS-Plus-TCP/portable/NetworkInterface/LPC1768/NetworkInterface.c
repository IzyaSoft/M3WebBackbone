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
#include "lpc17xx_emac.h"

static TaskHandle_t xEMACTaskHandle;

static void prvEMACHandlerTask(void *pvParameters)
{
    // function of LPC1768 EMAC Handling
	// todo: umv: check interrupts flags: RxDone, TxDone
	// todo: umv: if RxDone pass data from buffers to IP stack via xSendEventStructToIpTask

	/*	EMAC_PACKETBUF_Type pDataStruct;
		uint8_t ethernetBuffer [EMAC_ETH_MAX_FLEN] = {};
		pDataStruct.pbDataBuf = ethernetBuffer;
		pDataStruct.ulDataLen = EMAC_ETH_MAX_FLEN;
		EMAC_ReadPacketBuffer(&pDataStruct);
		for (x = 0; x < ipconfigNUM_NETWORK_BUFFER_DESCRIPTORS; x++)
		{
		    pxNetworkBuffers[x].pucEthernetBuffer = pDataStruct.pbDataBuf + ipBUFFER_PADDING;
		    *( ( unsigned * ) ram_buffer ) = ( unsigned ) ( &pxNetworkBuffers[ x ] );
		    ram_buffer += UNIT_SIZE;
		}*/
}

BaseType_t xNetworkInterfaceInitialise( void )
{
	EMAC_CFG_Type emacConfig;
	emacConfig.Mode = ETHERNET_MODE;
	emacConfig.pbEMAC_Addr = ETHERNET_MAC_ADDRESS;
	xTaskCreate(prvEMACHandlerTask, "LPC1768EMAC", configEMAC_TASK_STACK_SIZE, NULL, configMAX_PRIORITIES - 1, &xEMACTaskHandle);
	Status result = EMAC_Init(&emacConfig);
	if(result == SUCCESS)
		return pdTRUE;
	return  pdFALSE;
}

BaseType_t xNetworkInterfaceOutput(NetworkBufferDescriptor_t * const pxNetworkBuffer, BaseType_t xReleaseAfterSend)
{
	EMAC_PACKETBUF_Type txBuffer;
	txBuffer.pbDataBuf = pxNetworkBuffer->pucEthernetBuffer;
	txBuffer.ulDataLen = pxNetworkBuffer->xDataLength; // Maybe should add +1 for CRC
	EMAC_WritePacketBuffer(&txBuffer);
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


