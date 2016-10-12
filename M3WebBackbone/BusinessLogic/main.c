#include "LPC17xx.h"
#include "core_cm3.h"
#include "FreeRTOSConfig.h"
#include "FreeRTOSFATConfig.h"
#include "projdefs.h"
#include "portmacro.h"
#include "FreeRTOSIPConfig.h"


#include "configurationManager.h"
//#include "FreeRTOS_TCP_server.h"

// global variables
const uint8_t ETHERNET_MAC_ADDRESS[] = {0xAA, 0x33, 0x00, 0x66, 0x22, 0xEE};

const uint8_t APP_DEFAULT_IP_ADDRESS[] = {192, 168, 200, 5};
const uint8_t APP_DEFAULT_NETMASK[] = {255, 255, 255, 0};

const uint8_t APP_DEFAULT_GATEWAY[] = {192, 168, 200, 1};
const uint8_t APP_DEFAULT_NAMESERVER[] = {192, 168, 200, 1};

int main()
{
	InitializeClocks();

	// IP initialization in FreeRTOS
	//FreeRTOS_IPInit(APP_DEFAULT_IP_ADDRESS, APP_DEFAULT_NETMASK, APP_DEFAULT_GATEWAY, APP_DEFAULT_NAMESERVER, ETHERNET_MAC_ADDRESS);
	while(1)
	{

	}
    return 0;
}
