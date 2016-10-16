/*
 * FreeRTOS+TCP Labs Build 160919 (C) 2016 Real Time Engineers ltd.
 * Authors include Hein Tibosch and Richard Barry
 *
 *******************************************************************************
 ***** NOTE ******* NOTE ******* NOTE ******* NOTE ******* NOTE ******* NOTE ***
 ***                                                                         ***
 ***                                                                         ***
 ***   FREERTOS+TCP IS STILL IN THE LAB (mainly because the FTP and HTTP     ***
 ***   demos have a dependency on FreeRTOS+FAT, which is only in the Labs    ***
 ***   download):                                                            ***
 ***                                                                         ***
 ***   FreeRTOS+TCP is functional and has been used in commercial products   ***
 ***   for some time.  Be aware however that we are still refining its       ***
 ***   design, the source code does not yet quite conform to the strict      ***
 ***   coding and style standards mandated by Real Time Engineers ltd., and  ***
 ***   the documentation and testing is not necessarily complete.            ***
 ***                                                                         ***
 ***   PLEASE REPORT EXPERIENCES USING THE SUPPORT RESOURCES FOUND ON THE    ***
 ***   URL: http://www.FreeRTOS.org/contact  Active early adopters may, at   ***
 ***   the sole discretion of Real Time Engineers Ltd., be offered versions  ***
 ***   under a license other than that described below.                      ***
 ***                                                                         ***
 ***                                                                         ***
 ***** NOTE ******* NOTE ******* NOTE ******* NOTE ******* NOTE ******* NOTE ***
 *******************************************************************************
 *
 * FreeRTOS+TCP can be used under two different free open source licenses.  The
 * license that applies is dependent on the processor on which FreeRTOS+TCP is
 * executed, as follows:
 *
 * If FreeRTOS+TCP is executed on one of the processors listed under the Special
 * License Arrangements heading of the FreeRTOS+TCP license information web
 * page, then it can be used under the terms of the FreeRTOS Open Source
 * License.  If FreeRTOS+TCP is used on any other processor, then it can be used
 * under the terms of the GNU General Public License V2.  Links to the relevant
 * licenses follow:
 *
 * The FreeRTOS+TCP License Information Page: http://www.FreeRTOS.org/tcp_license
 * The FreeRTOS Open Source License: http://www.FreeRTOS.org/license
 * The GNU General Public License Version 2: http://www.FreeRTOS.org/gpl-2.0.txt
 *
 * FreeRTOS+TCP is distributed in the hope that it will be useful.  You cannot
 * use FreeRTOS+TCP unless you agree that you use the software 'as is'.
 * FreeRTOS+TCP is provided WITHOUT ANY WARRANTY; without even the implied
 * warranties of NON-INFRINGEMENT, MERCHANTABILITY or FITNESS FOR A PARTICULAR
 * PURPOSE. Real Time Engineers Ltd. disclaims all conditions and terms, be they
 * implied, expressed, or statutory.
 *
 * 1 tab == 4 spaces!
 *
 * http://www.FreeRTOS.org
 * http://www.FreeRTOS.org/plus
 * http://www.FreeRTOS.org/labs
 *
 */

#include "Zynq/x_emacpsif.h"
#include "Zynq/x_topology.h"
#include "xstatus.h"

#include "xparameters.h"
#include "xparameters_ps.h"
#include "xil_exception.h"
#include "xil_mmu.h"*/

#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"
#include "semphr.h"

/* FreeRTOS+TCP includes. */
#include "FreeRTOS_IP.h"
#include "FreeRTOS_Sockets.h"
#include "FreeRTOS_IP_Private.h"
#include "NetworkBufferManagement.h"

#include "uncached_memory.h"

/* Two defines used to set or clear the EMAC interrupt */
#define INTC_BASE_ADDR		XPAR_SCUGIC_CPU_BASEADDR
#define INTC_DIST_BASE_ADDR	XPAR_SCUGIC_DIST_BASEADDR



#if( ipconfigPACKET_FILLER_SIZE != 2 )
	#error Please define ipconfigPACKET_FILLER_SIZE as the value '2'
#endif
#define TX_OFFSET				ipconfigPACKET_FILLER_SIZE

/* Defined in NetworkInterface.c */
extern TaskHandle_t xEMACTaskHandle;

/*
	pxDMA_tx_buffers: these are character arrays, each one is big enough to hold 1 MTU.
	The actual TX buffers are located in uncached RAM.
*/
static unsigned char *pxDMA_tx_buffers[ ipconfigNIC_N_TX_DESC ] = { NULL };

/*
	pxDMA_rx_buffers: these are pointers to 'NetworkBufferDescriptor_t'.
	Once a message has been received by the EMAC, the descriptor can be passed
	immediately to the IP-task.
*/
static NetworkBufferDescriptor_t *pxDMA_rx_buffers[ ipconfigNIC_N_RX_DESC ] = { NULL };

/*
	The FreeRTOS+TCP port is using a fixed 'topology', which is declared in
	./portable/NetworkInterface/Zynq/NetworkInterface.c
*/
extern struct xtopology_t xXTopology;

static SemaphoreHandle_t xTXDescriptorSemaphore = NULL;

/*
	The FreeRTOS+TCP port does not make use of "src/xemacps_bdring.c".
	In stead 'struct xemacpsif_s' has a "head" and a "tail" index.
	"head" is the next index to be written, used.
	"tail" is the next index to be read, freed.
*/

int is_tx_space_available( xemacpsif_s *xemacpsif )
{
int freecount = 0;
int tail = xemacpsif->txTail;
int head = xemacpsif->txHead;

	/* Returns the number of TX descriptors available */
	if( head == tail )
	{
		/* Normally in a circular buffer, "head == tail" would mean: the buffer is empty.
		However, for the EMAC, it may also mean that the buffer is totally filled.
		When "TXBUF_USED" is set, it means that the TX descriptor is available. */
		if( ( xemacpsif->txSegments[ head ].flags & XEMACPS_TXBUF_USED_MASK ) != 0 )
		{
			freecount = ipconfigNIC_N_TX_DESC;
		}
	}
	else
	{
		freecount = ipconfigNIC_N_TX_DESC + tail - head;
		if( freecount >= ipconfigNIC_N_TX_DESC )
		{
			freecount -= ipconfigNIC_N_TX_DESC;
		}
	}

	return freecount;
}

int is_rx_packets_available( xemacpsif_s *xemacpsif )
{
int filledcount = 0;
int head = xemacpsif->rxHead;
int tail = xemacpsif->rxTail;

	/* This function is used less than the above function:
	it returns the number of RX buffers that have been read
	and need a new network buffer descriptor */
	if( head == tail )
	{
		/* Same as for the TX: "head == tail" can either mean that the buffer is empty,
		or the buffer is totally filled. */
		if( ( xemacpsif->rxSegments[ head ].address & XEMACPS_RXBUF_NEW_MASK ) != 0 )
		{
			filledcount = ipconfigNIC_N_RX_DESC;
		}
	}
	else
	{
		filledcount = ipconfigNIC_N_RX_DESC + tail - head;
		if( filledcount >= ipconfigNIC_N_RX_DESC )
		{
			filledcount -= ipconfigNIC_N_RX_DESC;
		}
	}

	return filledcount;
}

void emacps_check_tx( xemacpsif_s *xemacpsif )
{
int tail = xemacpsif->txTail;
int head = xemacpsif->txHead;

	if( head != tail )
	{
		for( ; ; )
		{
			if( ( xemacpsif->txSegments[ tail ].flags & XEMACPS_TXBUF_USED_MASK ) == 0 )
			{
				/* The driver is still waiting for the EMAC to sent this message.
				When done, "TXBUF_USED" will be set. */
				break;
			}
#if( ipconfigZERO_COPY_TX_DRIVER != 0 )
#warning ipconfigZERO_COPY_TX_DRIVER is defined
			{
			void *pvBuffer = pxDMA_tx_buffers[ tail ];
			NetworkBufferDescriptor_t *pxBuffer;

				if( pvBuffer != NULL )
				{
					pxDMA_tx_buffers[ tail ] = NULL;
					pxBuffer = pxPacketBuffer_to_NetworkBuffer( pvBuffer );
					if( pxBuffer != NULL )
					{
						vReleaseNetworkBufferAndDescriptor( pxBuffer );
					}
					else
					{
						FreeRTOS_printf( ( "emacps_check_tx: Can not find network buffer\n" ) );
					}
				}
			}
#endif
			/* Clear all but the "used" and "wrap" bits. */
			if( tail < ipconfigNIC_N_TX_DESC - 1 )
			{
				xemacpsif->txSegments[ tail ].flags = XEMACPS_TXBUF_USED_MASK;
			}
			else
			{
				xemacpsif->txSegments[ tail ].flags = XEMACPS_TXBUF_USED_MASK | XEMACPS_TXBUF_WRAP_MASK;
			}
			
			/* Tell the counting semaphore that one more TX descriptor is available. */
			xSemaphoreGive( xTXDescriptorSemaphore );
			if( ++tail == ipconfigNIC_N_TX_DESC )
			{
				tail = 0;
			}
			if( tail == head )
			{
				break;
			}
		}
		xemacpsif->txTail = tail;
	}
	return;
}

void emacps_send_handler(void *arg)
{
xemacpsif_s   *xemacpsif;
BaseType_t xHigherPriorityTaskWoken = pdFALSE;

	xemacpsif = (xemacpsif_s *)(arg);

	/* In this port for FreeRTOS+TCP, the EMAC interrupts will only set a bit in
	"isr_events". The task in NetworkInterface will wake-up and do the necessary work.
	*/
	xemacpsif->isr_events |= EMAC_IF_TX_EVENT;
	xemacpsif->txBusy = pdFALSE;

	if( xEMACTaskHandle != NULL )
	{
		vTaskNotifyGiveFromISR( xEMACTaskHandle, &xHigherPriorityTaskWoken );
	}

	portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
}

static BaseType_t xValidLength( BaseType_t xLength )
{
BaseType_t xReturn;

	if( ( xLength >= ( BaseType_t ) sizeof( struct xARP_PACKET ) ) && ( ( ( uint32_t ) xLength ) <= ipTOTAL_ETHERNET_FRAME_SIZE ) )
	{
		xReturn = pdTRUE;
	}
	else
	{
		xReturn =  pdFALSE;
	}

	return xReturn;
}

XStatus emacps_send_message(xemacpsif_s *xemacpsif, NetworkBufferDescriptor_t *pxBuffer, int iReleaseAfterSend )
{
int head = xemacpsif->txHead;
int tail = xemacpsif->txTail;
int iHasSent = 0;
uint32_t ulBaseAddress = xemacpsif->emacps.Config.BaseAddress;
TickType_t xBlockTimeTicks = pdMS_TO_TICKS( 5000u );

	#if( ipconfigZERO_COPY_TX_DRIVER != 0 )
	{
		/* This driver wants to own all network buffers which are to be transmitted. */
		configASSERT( iReleaseAfterSend != pdFALSE );
	}
	#endif

	/* Open a do {} while ( 0 ) loop to be able to call break. */
	do
	{
	uint32_t flags = 0;

		if( xValidLength( pxBuffer->xDataLength ) != pdTRUE )
		{
			break;
		}

		if( xTXDescriptorSemaphore == NULL )
		{
			break;
		}

		if( xSemaphoreTake( xTXDescriptorSemaphore, xBlockTimeTicks ) != pdPASS )
		{
			FreeRTOS_printf( ( "emacps_send_message: Time-out waiting for TX buffer\n" ) );
			break;
		}

		if( ( head == tail ) && ( xemacpsif->txSegments[ head ].flags & XEMACPS_TXBUF_USED_MASK ) == 0 )
		{
			FreeRTOS_printf( ( "emacps_send_message: Next TX buffer still in use???\n" ) );
			break;
		}
#if( ipconfigZERO_COPY_TX_DRIVER != 0 )
		/* Pass the pointer (and its ownership) directly to DMA. */
		pxDMA_tx_buffers[ head ] = pxBuffer->pucEthernetBuffer;
		if( ucIsCachedMemory( pxBuffer->pucEthernetBuffer ) != 0 )
		{
			Xil_DCacheFlushRange( ( unsigned )pxBuffer->pucEthernetBuffer, pxBuffer->xDataLength );
		}
		/* Buffer has been transferred, do not release it. */
		iReleaseAfterSend = pdFALSE;
#else
		if( pxDMA_tx_buffers[ head ] == NULL )
		{
			FreeRTOS_printf( ( "emacps_send_message: pxDMA_tx_buffers[ %d ] == NULL\n", head ) );
			break;
		}
		/* Copy the message to unbuffered space in RAM. */
		memcpy( pxDMA_tx_buffers[ head ], pxBuffer->pucEthernetBuffer, pxBuffer->xDataLength );
#endif
		/* Packets will be sent one-by-one, so for each packet
		the TXBUF_LAST bit will be set. */
		flags |= XEMACPS_TXBUF_LAST_MASK;
		flags |= ( pxBuffer->xDataLength & XEMACPS_TXBUF_LEN_MASK );
		if( head == ( ipconfigNIC_N_TX_DESC - 1 ) )
		{
			flags |= XEMACPS_TXBUF_WRAP_MASK;
		}

		/* Copy the address of the buffer and set the flags. */
		xemacpsif->txSegments[ head ].address = ( uint32_t )pxDMA_tx_buffers[ head ];
		xemacpsif->txSegments[ head ].flags = flags;

		iHasSent = pdTRUE;
		if( ++head == ipconfigNIC_N_TX_DESC )
		{
			head = 0;
		}
		/* Update the TX-head index. These variable are declared volatile so they will be
		accessed as little as possible.	*/
		xemacpsif->txHead = head;
	} while( pdFALSE );

	if( iReleaseAfterSend != pdFALSE )
	{
		vReleaseNetworkBufferAndDescriptor( pxBuffer );
		pxBuffer = NULL;
	}

	/* Data Synchronization Barrier */
	dsb();

	if( iHasSent != pdFALSE )
	{
		/* Make STARTTX high */
		uint32_t ulValue = XEmacPs_ReadReg( ulBaseAddress, XEMACPS_NWCTRL_OFFSET);
		/* Start transmit */
		xemacpsif->txBusy = pdTRUE;
		XEmacPs_WriteReg( ulBaseAddress, XEMACPS_NWCTRL_OFFSET, ( ulValue | XEMACPS_NWCTRL_STARTTX_MASK ) );
	}
	dsb();

	return 0;
}

void emacps_set_rx_buffers( xemacpsif_s *xemacpsif, u32 ulCount )
{
	NetworkBufferDescriptor_t *pxBuffer;
	unsigned int FreeBds;
	int tail = xemacpsif->rxTail;
	int head = xemacpsif->rxHead;

	if( ulCount != 0 )
	{
		FreeBds = ulCount;
	}
	else
	{
		FreeBds = is_rx_packets_available( NULL );
	}

	while( FreeBds-- != 0 )
	{
		pxBuffer = pxGetNetworkBufferWithDescriptor( ipTOTAL_ETHERNET_FRAME_SIZE, ( TickType_t ) 0 );
		if( pxBuffer == NULL )
		{
			FreeRTOS_printf( ("unable to alloc pbuf in recv_handler\n" ) );
			dsb();
			return;
		}

		dsb();

		if( ( xemacpsif->rxSegments[ tail ].address & XEMACPS_RXBUF_NEW_MASK ) == 0 )
		{
			FreeRTOS_printf( ("Buffer %d too far?\n", tail ) );
		}
		if( pxDMA_rx_buffers[ tail ] != NULL )
		{
			FreeRTOS_printf( ("Buffer %d not used?\n", tail ) );
		}
		pxDMA_rx_buffers[ tail ] = ( void * )pxBuffer;
		if( ucIsCachedMemory( pxBuffer->pucEthernetBuffer ) != 0 )
		{
			Xil_DCacheInvalidateRange( ( ( uint32_t )pxBuffer->pucEthernetBuffer ) - ipconfigPACKET_FILLER_SIZE, (unsigned)ipTOTAL_ETHERNET_FRAME_SIZE );
		}
		{
			uint32_t addr = ( ( uint32_t )pxBuffer->pucEthernetBuffer ) & XEMACPS_RXBUF_ADD_MASK;
			if( tail == ( ipconfigNIC_N_RX_DESC - 1 ) )
			{
				addr |= XEMACPS_RXBUF_WRAP_MASK;
			}
			/* Clearing 'XEMACPS_RXBUF_NEW_MASK'       0x00000001 *< Used bit.. */
			xemacpsif->rxSegments[ tail ].address = addr;
			xemacpsif->rxSegments[ tail ].flags = 0;
		}
		if( ++tail == ipconfigNIC_N_RX_DESC )
		{
			tail = 0;
		}
		if( tail == head )
		{
			break;
		}
	}
	xemacpsif->rxTail = tail;
}

void emacps_recv_handler(void *arg)
{
	xemacpsif_s *xemacpsif;
	BaseType_t xHigherPriorityTaskWoken = pdFALSE;

	xemacpsif = (xemacpsif_s *)(arg);
	xemacpsif->isr_events |= EMAC_IF_RX_EVENT;

	if( xEMACTaskHandle != NULL )
	{
		vTaskNotifyGiveFromISR( xEMACTaskHandle, &xHigherPriorityTaskWoken );
	}

	portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
}

static NetworkBufferDescriptor_t *ethMsg = NULL;
static NetworkBufferDescriptor_t *ethLast = NULL;

static void passEthMessages( void )
{
IPStackEvent_t xRxEvent;

	xRxEvent.eEventType = eNetworkRxEvent;
	xRxEvent.pvData = ( void * ) ethMsg;

	if( xSendEventStructToIPTask( &xRxEvent, ( TickType_t ) 1000 ) != pdPASS )
	{
		/* The buffer could not be sent to the stack so	must be released again.
		This is a deferred handler taskr, not a real interrupt, so it is ok to
		use the task level function here. */
		do
		{
			NetworkBufferDescriptor_t *xNext = ethMsg->pxNextBuffer;
			vReleaseNetworkBufferAndDescriptor( ethMsg );
			ethMsg = xNext;
		} while( ethMsg != NULL );

		iptraceETHERNET_RX_EVENT_LOST();
		FreeRTOS_printf( ( "passEthMessages: Can not queue return packet!\n" ) );
	}

	ethMsg = ethLast = NULL;
}

int emacps_check_rx( xemacpsif_s *xemacpsif )
{
NetworkBufferDescriptor_t *pxBuffer;
int rx_bytes;
volatile int msgCount = 0;
int head = xemacpsif->rxHead;

	/* There seems to be an issue (SI# 692601), see comments below. */
	resetrx_on_no_rxdata(xemacpsif);

	/* This FreeRTOS+TCP driver shall be compiled with the option
	"ipconfigUSE_LINKED_RX_MESSAGES" enabled.  It allows the driver to send a
	chain of RX messages within one message to the IP-task.	*/
	for( ; ; )
	{
		if( ( ( xemacpsif->rxSegments[ head ].address & XEMACPS_RXBUF_NEW_MASK ) == 0 ) ||
			( pxDMA_rx_buffers[ head ] == NULL ) )
		{
			break;
		}

		pxBuffer = ( NetworkBufferDescriptor_t * )pxDMA_rx_buffers[ head ];

		/* Just avoiding to use or refer to the same buffer again */
		pxDMA_rx_buffers[ head ] = NULL;

		/*
		 * Adjust the buffer size to the actual number of bytes received.
		 */
		rx_bytes = xemacpsif->rxSegments[ head ].flags & XEMACPS_RXBUF_LEN_MASK;

		pxBuffer->xDataLength = rx_bytes;

		if( ucIsCachedMemory( pxBuffer->pucEthernetBuffer ) != 0 )
		{
			Xil_DCacheInvalidateRange( ( ( uint32_t )pxBuffer->pucEthernetBuffer ) - ipconfigPACKET_FILLER_SIZE, (unsigned)rx_bytes );
		}

		/* store it in the receive queue, where it'll be processed by a
		different handler. */
		iptraceNETWORK_INTERFACE_RECEIVE();
		pxBuffer->pxNextBuffer = NULL;

		if( ethMsg == NULL )
		{
			// Becomes the first message
			ethMsg = pxBuffer;
		}
		else if( ethLast != NULL )
		{
			// Add to the tail
			ethLast->pxNextBuffer = pxBuffer;
		}

		ethLast = pxBuffer;
		msgCount++;

		if( ++head == ipconfigNIC_N_RX_DESC )
		{
			head = 0;
		}
	}
	xemacpsif->rxHead = head;

	if( ethMsg != NULL )
	{
		passEthMessages( );
	}

	if( msgCount != 0 )
	{
		/* Some packets (network buffer descriptors) have been passed to the
		IP-stack.  Now allocate new descriptors and put them in place. */
		emacps_set_rx_buffers( xemacpsif, msgCount );
	}

	return msgCount;
}

void clean_dma_txdescs(xemacpsif_s *xemacpsif)
{
int index;
unsigned char *ucTxBuffer;

	/* Clear all TX descriptors and assign uncached memory to each descriptor.
	"tx_space" points to the first available TX buffer. */
	ucTxBuffer = xemacpsif->tx_space;

	for( index = 0; index < ipconfigNIC_N_TX_DESC; index++ )
	{
		xemacpsif->txSegments[ index ].address = ( uint32_t )ucTxBuffer;
		xemacpsif->txSegments[ index ].flags = XEMACPS_TXBUF_USED_MASK;
#if( ipconfigZERO_COPY_TX_DRIVER != 0 )
		pxDMA_tx_buffers[ index ] = ( void* )NULL;
#else
		pxDMA_tx_buffers[ index ] = ( void* )( ucTxBuffer + TX_OFFSET );
#endif
		ucTxBuffer += xemacpsif->uTxUnitSize;
	}
	xemacpsif->txSegments[ ipconfigNIC_N_TX_DESC - 1 ].flags =
		XEMACPS_TXBUF_USED_MASK | XEMACPS_TXBUF_WRAP_MASK;
}

XStatus init_dma(xemacpsif_s *xemacpsif)
{
	NetworkBufferDescriptor_t *pxBuffer;

	int iIndex;
	UBaseType_t xRxSize;
	UBaseType_t xTxSize;
	struct xtopology_t *xtopologyp = &xXTopology;

	xRxSize = ipconfigNIC_N_RX_DESC * sizeof( xemacpsif->rxSegments[ 0 ] );

	xTxSize = ipconfigNIC_N_TX_DESC * sizeof( xemacpsif->txSegments[ 0 ] );

	/* Also round-up to 4KB */
	xemacpsif->uTxUnitSize = ( ipTOTAL_ETHERNET_FRAME_SIZE + 0x1000ul ) & ~0xffful;
	/*
	 * We allocate 65536 bytes for RX BDs which can accommodate a
	 * maximum of 8192 BDs which is much more than any application
	 * will ever need.
	 */
	xemacpsif->rxSegments = ( struct xBD_TYPE * )( pucGetUncachedMemory ( xRxSize )  );
	xemacpsif->txSegments = ( struct xBD_TYPE * )( pucGetUncachedMemory ( xTxSize ) );
	xemacpsif->tx_space   = ( unsigned char *   )( pucGetUncachedMemory ( ipconfigNIC_N_TX_DESC * xemacpsif->uTxUnitSize ) );

	/* These variables will be used in XEmacPs_Start (see src/xemacps.c). */
	xemacpsif->emacps.RxBdRing.BaseBdAddr = ( uint32_t ) xemacpsif->rxSegments;
	xemacpsif->emacps.TxBdRing.BaseBdAddr = ( uint32_t ) xemacpsif->txSegments;

	if( xTXDescriptorSemaphore == NULL )
	{
		xTXDescriptorSemaphore = xSemaphoreCreateCounting( ( UBaseType_t ) ipconfigNIC_N_TX_DESC - 1, ( UBaseType_t ) ipconfigNIC_N_TX_DESC - 1 );
		configASSERT( xTXDescriptorSemaphore );
	}
	/*
	 * Allocate RX descriptors, 1 RxBD at a time.
	 */
	for( iIndex = 0; iIndex < ipconfigNIC_N_RX_DESC; iIndex++ )
	{
		pxBuffer = pxDMA_rx_buffers[ iIndex ];
		if( pxBuffer == NULL )
		{
			pxBuffer = pxGetNetworkBufferWithDescriptor( ipTOTAL_ETHERNET_FRAME_SIZE, ( TickType_t ) 0 );
			if( pxBuffer == NULL )
			{
				FreeRTOS_printf( ("Unable to allocate a network buffer in recv_handler\n" ) );
				return -1;
			}
		}

		xemacpsif->rxSegments[ iIndex ].flags = 0;
		xemacpsif->rxSegments[ iIndex ].address = ( ( uint32_t )pxBuffer->pucEthernetBuffer ) & XEMACPS_RXBUF_ADD_MASK;

		pxDMA_rx_buffers[ iIndex ] = pxBuffer;
		/* Make sure this memory is not in cache for now. */
		if( ucIsCachedMemory( pxBuffer->pucEthernetBuffer ) != 0 )
		{
			Xil_DCacheInvalidateRange( ( ( uint32_t )pxBuffer->pucEthernetBuffer ) - ipconfigPACKET_FILLER_SIZE,
				(unsigned)ipTOTAL_ETHERNET_FRAME_SIZE );
		}
	}

	xemacpsif->rxSegments[ ipconfigNIC_N_RX_DESC - 1 ].address |= XEMACPS_RXBUF_WRAP_MASK;

	memset( xemacpsif->tx_space, '\0', ipconfigNIC_N_TX_DESC * xemacpsif->uTxUnitSize );

	clean_dma_txdescs( xemacpsif );

	{
		uint32_t value;
		value = XEmacPs_ReadReg( xemacpsif->emacps.Config.BaseAddress, XEMACPS_DMACR_OFFSET );

		// 1xxxx: Attempt to use INCR16 AHB bursts
		value = ( value & ~( XEMACPS_DMACR_BLENGTH_MASK ) ) | XEMACPS_DMACR_INCR16_AHB_BURST;
#if( ipconfigDRIVER_INCLUDED_TX_IP_CHECKSUM != 0 )
		value |= XEMACPS_DMACR_TCPCKSUM_MASK;
#else
#warning Are you sure the EMAC should not calculate outgoing checksums?
		value &= ~XEMACPS_DMACR_TCPCKSUM_MASK;
#endif
		XEmacPs_WriteReg( xemacpsif->emacps.Config.BaseAddress, XEMACPS_DMACR_OFFSET, value );
	}
	{
		uint32_t value;
		value = XEmacPs_ReadReg( xemacpsif->emacps.Config.BaseAddress, XEMACPS_NWCFG_OFFSET );

		/* Network buffers are 32-bit aligned + 2 bytes (because ipconfigPACKET_FILLER_SIZE = 2 ).
		Now tell the EMAC that received messages should be stored at "address + 2". */
		value = ( value & ~XEMACPS_NWCFG_RXOFFS_MASK ) | 0x8000;

#if( ipconfigDRIVER_INCLUDED_RX_IP_CHECKSUM != 0 )
		value |= XEMACPS_NWCFG_RXCHKSUMEN_MASK;
#else
#warning Are you sure the EMAC should not calculate incoming checksums?
		value &= ~XEMACPS_NWCFG_RXCHKSUMEN_MASK;
#endif
		XEmacPs_WriteReg( xemacpsif->emacps.Config.BaseAddress, XEMACPS_NWCFG_OFFSET, value );
	}

	/*
	 * Connect the device driver handler that will be called when an
	 * interrupt for the device occurs, the handler defined above performs
	 * the specific interrupt processing for the device.
	 */
	XScuGic_RegisterHandler(INTC_BASE_ADDR, xtopologyp->scugic_emac_intr,
		(Xil_ExceptionHandler)XEmacPs_IntrHandler,
		(void *)&xemacpsif->emacps);
	/*
	 * Enable the interrupt for emacps.
	 */
	EmacEnableIntr( );

	return 0;
}

/*
 * resetrx_on_no_rxdata():
 *
 * It is called at regular intervals through the API xemacpsif_resetrx_on_no_rxdata
 * called by the user.
 * The EmacPs has a HW bug (SI# 692601) on the Rx path for heavy Rx traffic.
 * Under heavy Rx traffic because of the HW bug there are times when the Rx path
 * becomes unresponsive. The workaround for it is to check for the Rx path for
 * traffic (by reading the stats registers regularly). If the stats register
 * does not increment for sometime (proving no Rx traffic), the function resets
 * the Rx data path.
 *
 */

void resetrx_on_no_rxdata(xemacpsif_s *xemacpsif)
{
	unsigned long regctrl;
	unsigned long tempcntr;

	tempcntr = XEmacPs_ReadReg( xemacpsif->emacps.Config.BaseAddress, XEMACPS_RXCNT_OFFSET );
	if ( ( tempcntr == 0 ) && ( xemacpsif->last_rx_frms_cntr == 0 ) )
	{
		regctrl = XEmacPs_ReadReg(xemacpsif->emacps.Config.BaseAddress,
				XEMACPS_NWCTRL_OFFSET);
		regctrl &= (~XEMACPS_NWCTRL_RXEN_MASK);
		XEmacPs_WriteReg(xemacpsif->emacps.Config.BaseAddress,
				XEMACPS_NWCTRL_OFFSET, regctrl);
		regctrl = XEmacPs_ReadReg(xemacpsif->emacps.Config.BaseAddress, XEMACPS_NWCTRL_OFFSET);
		regctrl |= (XEMACPS_NWCTRL_RXEN_MASK);
		XEmacPs_WriteReg(xemacpsif->emacps.Config.BaseAddress, XEMACPS_NWCTRL_OFFSET, regctrl);
	}
	xemacpsif->last_rx_frms_cntr = tempcntr;
}

void EmacDisableIntr(void)
{
	XScuGic_DisableIntr(INTC_DIST_BASE_ADDR, xXTopology.scugic_emac_intr);
}

void EmacEnableIntr(void)
{
	XScuGic_EnableIntr(INTC_DIST_BASE_ADDR, xXTopology.scugic_emac_intr);
}

