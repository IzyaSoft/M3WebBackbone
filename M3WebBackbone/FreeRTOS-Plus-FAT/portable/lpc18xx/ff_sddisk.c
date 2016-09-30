/*
 * FreeRTOS+FAT Labs Build 160919 (C) 2016 Real Time Engineers ltd.
 * Authors include James Walmsley, Hein Tibosch and Richard Barry
 *
 *******************************************************************************
 ***** NOTE ******* NOTE ******* NOTE ******* NOTE ******* NOTE ******* NOTE ***
 ***                                                                         ***
 ***                                                                         ***
 ***   FREERTOS+FAT IS STILL IN THE LAB:                                     ***
 ***                                                                         ***
 ***   This product is functional and is already being used in commercial    ***
 ***   products.  Be aware however that we are still refining its design,    ***
 ***   the source code does not yet fully conform to the strict coding and   ***
 ***   style standards mandated by Real Time Engineers ltd., and the         ***
 ***   documentation and testing is not necessarily complete.                ***
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
 * FreeRTOS+FAT can be used under two different free open source licenses.  The
 * license that applies is dependent on the processor on which FreeRTOS+FAT is
 * executed, as follows:
 *
 * If FreeRTOS+FAT is executed on one of the processors listed under the Special
 * License Arrangements heading of the FreeRTOS+FAT license information web
 * page, then it can be used under the terms of the FreeRTOS Open Source
 * License.  If FreeRTOS+FAT is used on any other processor, then it can be used
 * under the terms of the GNU General Public License V2.  Links to the relevant
 * licenses follow:
 *
 * The FreeRTOS+FAT License Information Page: http://www.FreeRTOS.org/fat_license
 * The FreeRTOS Open Source License: http://www.FreeRTOS.org/license
 * The GNU General Public License Version 2: http://www.FreeRTOS.org/gpl-2.0.txt
 *
 * FreeRTOS+FAT is distributed in the hope that it will be useful.  You cannot
 * use FreeRTOS+FAT unless you agree that you use the software 'as is'.
 * FreeRTOS+FAT is provided WITHOUT ANY WARRANTY; without even the implied
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

/* Standard includes. */
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <stdio.h>

/* LPC18xx includes. */
#include "chip.h"
#include "board.h"

/* FreeRTOS includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "portmacro.h"

/* FreeRTOS+FAT includes. */
#include "ff_sddisk.h"
#include "ff_sys.h"

/*_RB_ Definitions require comments to say what and why. */
#define sdSIGNATURE 0x41404342

/*_RB_ IO manager memory size needs to be configurable. */
#define sdIOMAN_MEM_SIZE	4096
#define sdHUNDRED_64_BIT	100ULL

/*-----------------------------------------------------------*/

/*_RB_ Functions require comment blocks. */
static void prvSDMMCSetupWakeup( void *pvInfo );
static uint32_t prvSDMMCWait( void );
static void prvSDMMCDelay_ms( uint32_t time );
static void prvInitialiseCardInfo( void );
static int32_t prvSDMMC_Init( void );
static BaseType_t prvSDDiskMount( FF_Disk_t *pxDisk, BaseType_t xPartitionNumber );
static int32_t prvFFRead( uint8_t *pucBuffer, uint32_t ulSectorNumber, uint32_t ulSectorCount, FF_Disk_t *pxDisk );
static int32_t prvFFWrite( uint8_t *pucBuffer, uint32_t ulSectorNumber, uint32_t ulSectorCount, FF_Disk_t *pxDisk );
BaseType_t prvSDDiskDelete( FF_Disk_t *pxDisk );

/*-----------------------------------------------------------*/

/*_RB_ Variables require a comment block where appropriate. */
static int32_t lSDDetected = 0;
static mci_card_struct xCardInfo;
static volatile int32_t lSDIOWaitExit;
static uint32_t	ulSDCardStatus;
static SemaphoreHandle_t xSDCardSemaphore;
static SemaphoreHandle_t xPlusFATMutex;

/*-----------------------------------------------------------*/

#warning Update to make read and write functions static and make use of the FF_Disk_t type.

static int32_t prvFFRead( uint8_t *pucBuffer, uint32_t ulSectorNumber, uint32_t ulSectorCount, FF_Disk_t *pxDisk )
{
int32_t iReturn;

	/*_RB_ Many of the comments in this file apply to other functions in the file. */

	if( ( pxDisk != NULL ) &&		/*_RB_ Could this be changed to an assert? */
		( ulSDCardStatus == 1 ) && 	/*_RB_ What does 1 mean?  It would be obvious and more maintainable as a definition [and below]*/
		( pxDisk->ulSignature == sdSIGNATURE ) &&
		( pxDisk->xStatus.bIsInitialised != 0 ) && /*_RB_ 0 was put in to prevent this being an implicit test, but what does 0 mean? */
		( ulSectorNumber < pxDisk->ulNumberOfSectors ) &&
		( ( pxDisk->ulNumberOfSectors - ulSectorNumber ) >= ulSectorCount ) )
	{

		iReturn = Chip_SDMMC_ReadBlocks( LPC_SDMMC, pucBuffer, ulSectorNumber, ulSectorCount );

		/*_RB_ I'm guessing 512 is a sector size, but that needs to be clear.
		Is it defined in a header somewhere?  If so we can do a search and
		replace in files on it as it seems to be used everywhere. */
		if( iReturn == ( ulSectorCount * 512 ) ) /*_RB_ Signed/unsigned mismatch (twice!) */
		{
			iReturn = FF_ERR_NONE;
		}
		else
		{
			/*_RB_ Signed number used to return bitmap (again below). */
			iReturn = ( FF_ERR_IOMAN_OUT_OF_BOUNDS_READ | FF_ERRFLAG );
		}
	}
	else
	{
		memset( ( void * ) pucBuffer, '\0', ulSectorCount * 512 );

		if( pxDisk->xStatus.bIsInitialised != 0 )
		{
			FreeRTOS_printf( ( "prvFFRead: warning: %lu + %lu > %lu\n", ulSectorNumber, ulSectorCount, pxDisk->ulNumberOfSectors ) );
		}

		iReturn = ( FF_ERR_IOMAN_OUT_OF_BOUNDS_READ | FF_ERRFLAG );
	}

	return iReturn;
}
/*-----------------------------------------------------------*/

static int32_t prvFFWrite( uint8_t *pucBuffer, uint32_t ulSectorNumber, uint32_t ulSectorCount, FF_Disk_t *pxDisk )
{
int32_t iReturn;

	if( ( ulSDCardStatus == 1 ) &&
		( pxDisk != NULL ) &&
		( pxDisk->ulSignature == sdSIGNATURE ) &&
		( pxDisk->xStatus.bIsInitialised != 0 ) &&
		( ulSectorNumber < pxDisk->ulNumberOfSectors ) &&
		( ( pxDisk->ulNumberOfSectors - ulSectorNumber ) >= ulSectorCount ) )
	{
		iReturn = Chip_SDMMC_WriteBlocks( LPC_SDMMC, pucBuffer, ulSectorNumber, ulSectorCount );

		if( iReturn == ( ulSectorCount * 512 ) ) /*_RB_ Signed/unsigned mismatch (twice!) */
		{
			iReturn = 0;
		}
		else
		{
			iReturn = ( FF_ERR_IOMAN_OUT_OF_BOUNDS_WRITE | FF_ERRFLAG );
		}
	}
	else
	{
		memset( ( void * ) pucBuffer, '\0', ulSectorCount * 512 );
		if( pxDisk->xStatus.bIsInitialised )
		{
			FreeRTOS_printf( ( "prvFFWrite: warning: %lu + %lu > %lu\n", ulSectorNumber, ulSectorCount, pxDisk->ulNumberOfSectors ) );
		}

		iReturn = ( FF_ERR_IOMAN_OUT_OF_BOUNDS_WRITE | FF_ERRFLAG );
	}

	return iReturn;
}
/*-----------------------------------------------------------*/

/*_RB_ Should the name have FF in it somewhere? */
void FF_SDDiskFlush( FF_Disk_t *pxDisk )
{
	if( ( pxDisk != NULL ) && ( pxDisk->xStatus.bIsInitialised != 0 ) && ( pxDisk->pxIOManager != NULL ) )
	{
		FF_FlushCache( pxDisk->pxIOManager );
	}
}
/*-----------------------------------------------------------*/

/* Initialise the SDIO driver and mount an SD card */
FF_Disk_t *FF_SDDiskInit( const char *pcName )
{
FF_Error_t xFFError;
int32_t	lPartitionNumber = 0;
FF_Disk_t * pxDisk;

	ulSDCardStatus = prvSDMMC_Init();
	if( ulSDCardStatus == 1 ) /*_RB_ What does 1 mean?  Can it be replaced with pdPASS? */
	{
		pxDisk = ( FF_Disk_t * ) pvPortMalloc( sizeof( *pxDisk ) );

		if( pxDisk != NULL )
		{
		FF_CreationParameters_t xParameters;

			if( xPlusFATMutex == NULL)
			{
				xPlusFATMutex = xSemaphoreCreateRecursiveMutex();
			}
			/* Initialise the created disk structure. */
			memset( pxDisk, '\0', sizeof( *pxDisk ) );
			pxDisk->ulNumberOfSectors = xCardInfo.card_info.blocknr;
			pxDisk->ulSignature = sdSIGNATURE;

			memset( &xParameters, '\0', sizeof( xParameters ) );
			xParameters.pucCacheMemory = NULL;
			xParameters.ulMemorySize = sdIOMAN_MEM_SIZE;
			xParameters.ulSectorSize = 512;
			xParameters.fnWriteBlocks = prvFFWrite;
			xParameters.fnReadBlocks = prvFFRead;
			xParameters.pxDisk = pxDisk;

			/* prvFFRead()/prvFFWrite() are not re-entrant and must be protected with
			the use of a semaphore. */
			xParameters.xBlockDeviceIsReentrant = pdFALSE;

			/* The semaphore will be uesd to protect critical sections in the +FAT driver,
			and also to avoid concurrent calls to prvFFRead()/prvFFWrite() from different tasks. */
			xParameters.pvSemaphore = ( void * ) xPlusFATMutex;

			/* Allocate an IO manager to use with the disk. */
			pxDisk->pxIOManager = FF_CreateIOManger( &xParameters, &xFFError );

			if( pxDisk->pxIOManager != NULL )
			{
				pxDisk->xStatus.bIsInitialised = pdTRUE;

				/*_RB_ Should I replace 0s with pdFAILs here? */
				if( prvSDDiskMount( pxDisk, lPartitionNumber ) == 0 )
				{
					/* The IO manager is deleted with the disk. */
					prvSDDiskDelete( pxDisk );
					pxDisk = NULL;
				}
				else
				{
					if( pcName == NULL )
					{
						pcName = "/";
					}
					FF_FS_Add( pcName, pxDisk );
				}
			}
			else
			{
				FreeRTOS_printf( ( "FF_SDDiskInit: FF_CreateIOManger: %s\n", ( const char * ) FF_GetErrMessage( xFFError ) ) );

				/* Remove compiler warnings if FreeRTOS_printf() is not
				defined. */
				( void ) xFFError;

				/* Cannot allocate the IO manager so free the disk again. */
				prvSDDiskDelete( pxDisk );
				pxDisk = NULL;
			}
		}
		else
		{
			FreeRTOS_printf( ( "FF_SDDiskInit: Malloc failed\n" ) );
		}
	}
	else
	{
		FreeRTOS_printf( ( "FF_SDDiskInit: prvSDMMC_Init failed\n" ) );
		pxDisk = NULL;
	}

	return pxDisk;
}
/*-----------------------------------------------------------*/

static BaseType_t prvSDDiskMount( FF_Disk_t *pxDisk, BaseType_t xPartitionNumber )
{
FF_Error_t xFFError;
BaseType_t xReturn;

	pxDisk->xStatus.bPartitionNumber = xPartitionNumber;

	/* Mount the partition. */
	xFFError = FF_Mount( pxDisk, xPartitionNumber );

	if( FF_isERR( xFFError ) != 0 )
	{
		/*_RB_ Out of curiosity - is it possible to omit error strings (configure
		then out)?  They could potentially take up quite a bit of space. */
		FreeRTOS_printf( ( "prvSDDiskMount: %s\n", ( const char * ) FF_GetErrMessage( xFFError ) ) );
		xReturn = 0;
	}
	else
	{
		pxDisk->xStatus.bIsMounted = pdTRUE;
/*
		pxDisk->pxIOManager->xPartition.ulPartitionSize = pxDisk->ulNumberOfSectors;
*/
		FreeRTOS_printf( ( "****** Full-FAT initialized %lu sectors\n", pxDisk->pxIOManager->xPartition.ulTotalSectors ) );
		FF_SDDiskShowPartition( pxDisk );
		xReturn = 1;
	}

	return xReturn;
}
/*-----------------------------------------------------------*/

/* Get a pointer to IOMAN, which can be used for all FreeRTOS+FAT functions */
FF_IOManager_t *sddisk_ioman( FF_Disk_t *pxDisk )
{
FF_IOManager_t * pxReturn;

	if( ( pxDisk != NULL ) && ( pxDisk->xStatus.bIsInitialised != 0 ) )
	{
		pxReturn = pxDisk->pxIOManager;
	}
	else
	{
		pxReturn = NULL;
	}

	return pxReturn;
}
/*-----------------------------------------------------------*/

/* Release all resources */
BaseType_t prvSDDiskDelete( FF_Disk_t *pxDisk )
{
	if( pxDisk != NULL )
	{
		pxDisk->ulSignature = 0;
		pxDisk->xStatus.bIsInitialised = 0;
		if( pxDisk->pxIOManager != NULL )
		{
			FF_DeleteIOManager( pxDisk->pxIOManager );
		}

		vPortFree( pxDisk );
	}

	return 1;
}
/*-----------------------------------------------------------*/

BaseType_t FF_SDDiskShowPartition( FF_Disk_t *pxDisk )
{
FF_Error_t Error;
uint64_t ullFreeSize, ullFreeSectors;
int32_t lPercentFree;
FF_IOManager_t *pxIOManager;
const char *pcTypeName = "unknown type";
BaseType_t xReturn;

	if( pxDisk != NULL )
	{
		pxIOManager = pxDisk->pxIOManager;
		FreeRTOS_printf( ( "Reading FAT and calculating Free Space\n" ) );

		switch( pxIOManager->xPartition.ucType )
		{
			case FF_T_FAT12:
				pcTypeName = "FAT12";
				break;

			case FF_T_FAT16:
				pcTypeName = "FAT16";
				break;

			case FF_T_FAT32:
				pcTypeName = "FAT32";
				break;
		}

		ullFreeSize = FF_GetFreeSize( pxIOManager, &Error );

		ullFreeSectors = pxIOManager->xPartition.ulFreeClusterCount * pxIOManager->xPartition.ulSectorsPerCluster;
		lPercentFree = ( int32_t ) ( ( ( sdHUNDRED_64_BIT * ullFreeSectors ) + ( pxIOManager->xPartition.ulDataSectors / 2 ) ) / pxIOManager->xPartition.ulDataSectors );

		FreeRTOS_printf( ( "Partition Nr   %8u\n", pxDisk->xStatus.bPartitionNumber ) );
		FreeRTOS_printf( ( "Type           %8u (%s)\n", pxIOManager->xPartition.ucType, pcTypeName ) );
		FreeRTOS_printf( ( "VolLabel       '%8s' \n", pxIOManager->xPartition.pcVolumeLabel ) );
		FreeRTOS_printf( ( "TotalSectors   %8lu\n", pxIOManager->xPartition.ulTotalSectors ) );
		FreeRTOS_printf( ( "Size           %8lu\n", 512U * pxIOManager->xPartition.ulDataSectors ) );
		FreeRTOS_printf( ( "FreeSize       %8lu ( %ld perc free )\n", ( uint32_t ) ullFreeSize, lPercentFree ) );

		xReturn = 1;
	}
	else
	{
		xReturn = 0;
	}

	return xReturn;
}
/*-----------------------------------------------------------*/

void SDIO_IRQHandler( void )
{
BaseType_t xHigherPriorityTaskWoken = pdFALSE;

	/* All SD based register handling is done in the callback function. The SDIO
	interrupt is not enabled as part of this driver and needs to be
	enabled/disabled in the callbacks or application as needed. This is to allow
	flexibility with IRQ handling for applications and RTOSes. */

	/* Set wait exit flag to tell wait function we are ready. In an RTOS, this
	would trigger wakeup of a thread waiting for the IRQ. */
	NVIC_DisableIRQ( SDIO_IRQn );
	xSemaphoreGiveFromISR( xSDCardSemaphore, &xHigherPriorityTaskWoken );
	lSDIOWaitExit = 1;
	portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
}
/*-----------------------------------------------------------*/

/* Sets up the SD event driven wakeup */
static void prvSDMMCSetupWakeup( void *pvInfo )
{
uint32_t *ulWaitStatus = ( uint32_t * ) pvInfo;

	/* Wait for IRQ - for an RTOS, you would pend on an event here with a IRQ
	based wakeup. */
	/*_RB_ Don't understand why this is spinning on a block time of 0.  Is it
	really meant to be != pdFALSE? */
	while( xSemaphoreTake( xSDCardSemaphore, 0 ) != pdFALSE )
	{
	}

	NVIC_ClearPendingIRQ( SDIO_IRQn );
	lSDIOWaitExit = 0;
	Chip_SDIF_SetIntMask( LPC_SDMMC, *ulWaitStatus );
	NVIC_EnableIRQ( SDIO_IRQn );
}
/*-----------------------------------------------------------*/

static uint32_t prvSDMMCWait( void )
{
uint32_t ulStatus;

	/*_RB_ 2000 needs to be defined and use pdMS_TO_TICKS so the delay period
	remains constant no matter how the end user sets configTICK_RATE_MS. */
	xSemaphoreTake( xSDCardSemaphore, 2000 );

	ulStatus = Chip_SDIF_GetIntStatus( LPC_SDMMC );
	if( ( ( ulStatus & MCI_INT_CMD_DONE ) == 0 ) || ( lSDIOWaitExit == 0 ) )
	{
		FreeRTOS_printf( ( "Wait SD: int32_t %ld ulStatus 0x%02lX\n", lSDIOWaitExit, ulStatus ) );
	}

	return ulStatus;
}
/*-----------------------------------------------------------*/

static void prvSDMMCDelay_ms( uint32_t ulTime )
{
	/* In an RTOS, the thread would sleep allowing other threads to run.
	For standalone operation, just spin on a timer */
	vTaskDelay( pdMS_TO_TICKS( ulTime ) );
}
/*-----------------------------------------------------------*/

static int32_t prvSDMMC_Init( void )
{
int32_t lSDCardStatus;

	if( xSDCardSemaphore == NULL )
	{
		xSDCardSemaphore = xSemaphoreCreateBinary();
		configASSERT( xSDCardSemaphore );
		xSemaphoreGive( xSDCardSemaphore );
	}

	prvInitialiseCardInfo();
	NVIC_SetPriority( SDIO_IRQn, configSD_INTERRUPT_PRIORITY );

	/*_RB_ Board_SDMMC_Init() is library specific code that is also specific to
	the target development board.The SDMMC peripheral should be initialised from
	the application code before this code is called. */
	Board_SDMMC_Init();
	Chip_SDIF_Init( LPC_SDMMC );
	lSDDetected = !Chip_SDIF_CardNDetect( LPC_SDMMC );

	Chip_SDIF_PowerOn( LPC_SDMMC );
	lSDCardStatus = Chip_SDMMC_Acquire( LPC_SDMMC, &xCardInfo );
	FreeRTOS_printf( ( "Acquire: %ld\n", lSDCardStatus ) );
	return lSDCardStatus;
}
/*-----------------------------------------------------------*/

static void prvInitialiseCardInfo( void )
{
	memset( &xCardInfo, 0, sizeof( xCardInfo ) );
	xCardInfo.card_info.evsetup_cb = prvSDMMCSetupWakeup;
	xCardInfo.card_info.waitfunc_cb = prvSDMMCWait;
	xCardInfo.card_info.msdelay_func = prvSDMMCDelay_ms;
}
