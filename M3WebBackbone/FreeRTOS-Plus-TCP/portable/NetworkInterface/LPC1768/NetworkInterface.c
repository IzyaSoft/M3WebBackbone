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

BaseType_t xNetworkInterfaceInitialise( void )
{
	EMAC_CFG_Type emacConfig;
	emacConfig.Mode = ETHERNET_MODE;
	uint8_t macAddr[6] =  {0xAA, 0x33, 0x00, 0x66, 0x22, 0xEE}; //ETHERNET_MAC_ADDRESS;
	emacConfig.pbEMAC_Addr = macAddr;
	Status result = EMAC_Init(&emacConfig);
	if(result == SUCCESS)
		return pdTRUE;
	return  pdFALSE;
}

BaseType_t xNetworkInterfaceOutput( NetworkBufferDescriptor_t * const pxNetworkBuffer, BaseType_t xReleaseAfterSend )
{
	//EMAC_PACKETBUF_Type txBuffer;
	//txBufffer.pbDataBuf = pxNetworkBuffer->pucEthernetBuffer;
	//txBufffer.ulDataLen = pxNetworkBuffer->xDataLength; // Maybe should add +1 for CRC
	//EMAC_WritePacketBuffer(&txBuffer);
	return pdTRUE;  // should think about return value
}

void vNetworkInterfaceAllocateRAMToBuffers( NetworkBufferDescriptor_t pxNetworkBuffers[ ipconfigNUM_NETWORK_BUFFER_DESCRIPTORS ] )
{
    // should think about possible usage of Network
}

BaseType_t xGetPhyLinkStatus( void )
{
    return pdTRUE;
}

