/*
 * FreeRTOSFATConfig.h
 *
 *  Created on: 30 сент. 2016 г.
 *      Author: michael
 */

#ifndef BUSINESSLOGIC_FREERTOSFATCONFIG_H_
#define BUSINESSLOGIC_FREERTOSFATCONFIG_H_

#define ffconfigBYTE_ORDER pdFREERTOS_LITTLE_ENDIAN
#define ffconfigHAS_CWD 1 // relatives path support
#define ffconfigCWD_THREAD_LOCAL_INDEX 5
#define ffconfigLFN_SUPPORT 0 // long file names disabled
#define ffconfigINCLUDE_SHORT_NAME 0 // DOS names ? disabled
#define ffconfigSHORTNAME_CASE 0 // short names switched off
#define ffconfigUNICODE_UTF16_SUPPORT 0
#define ffconfigUNICODE_UTF8_SUPPORT 1
#define ffconfigFAT12_SUPPORT 0
#define ffconfigOPTIMISE_UNALIGNED_ACCESS 1 // read/write files with 512bytes blocks
#define ffconfigCACHE_WRITE_THROUGH 0 // write directly without buffers
#define ffconfigWRITE_BOTH_FATS 1 // usage of two copies of FAT
#define ffconfigWRITE_FREE_COUNT 1 // writing free clusters values
#define ffconfigTIME_SUPPORT 0 // time stamps excluded
#define ffconfigREMOVABLE_MEDIA 1 // removable media such as SD-cards
#define ffconfigMOUNT_FIND_FREE 1 // checking free resources when disk is mounted
#define ffconfigFSINFO_TRUSTED 0 // i don't know
#define ffconfigPATH_CACHE 0 // do not cache relatives path (slower work)
#define ffconfigPATH_CACHE_DEPTH 0
#define ffconfigHASH_CACHE 0 // we won't use hashes due to simple files structure
#define ffcvonfigHASH_FUNCTION CRC8 // could be CRC16, but we don't use hashes
#define ffconfigMKDIR_RECURSIVE 1
//#define ffconfigBLKDEV_USES_SEM 1 // read and write works via additional semaphore
#define ffconfigMALLOC(size) pvPortMalloc(size) // we are using the same mem allocation function as free RTOS
#define ffconfigFREE(ptr) vPortFree(ptr)
#define ffconfig64_NUM_SUPPORT 0 // free size is 32-bit number (up to 4 Gb)
#define ffconfigMAX_PARTITIONS 8 // maximum number of partitions that could be recognized
#define ffconfigMAX_FILE_SYS 8 // maximum number of drives
#define ffconfigDRIVER_BUSY_SLEEP_MS 10 // sleep time if FF_ERR_DRIVER_BUSY
#define ffconfigFPRINTF_SUPPORT 0 // printf disabled
#define ffconfigFPRINTF_BUFFER_LENGTH 0
#define ffconfigINLINE_MEMORY_ACCESS 1 // don't know
#define ffconfigFAT_CHECK 0 // fat check disabled
#define ffconfigMAX_FILENAME 128 // probably 255 is better

#endif /* BUSINESSLOGIC_FREERTOSFATCONFIG_H_ */
