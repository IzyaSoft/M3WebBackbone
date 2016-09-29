#include "main.h"
#include "data/networkConfig.h"
#include "usr/networkManager.h"

/* Library includes. */
#include "LPC17xx.h"
#include "lpc17xx_emac.h"
#include "lpc17xx_gpio.h"

/* lwIP includes. */
#include "lwip/tcpip.h"


int main(void)
{
	// todo: umv: read config from flash
	struct NetworkConfig networkConfig;
	networkConfig._ipV4Address._byte1 = DEFAULT_IP_BYTE1;
	networkConfig._ipV4Address._byte2 = DEFAULT_IP_BYTE2;
	networkConfig._ipV4Address._byte3 = DEFAULT_IP_BYTE3;
	networkConfig._ipV4Address._byte4 = DEFAULT_IP_BYTE4;

	networkConfig._networkMask._byte1 = DEFAULT_NETMASK_BYTE1;
	networkConfig._networkMask._byte2 = DEFAULT_NETMASK_BYTE2;
	networkConfig._networkMask._byte3 = DEFAULT_NETMASK_BYTE3;
	networkConfig._networkMask._byte4 = DEFAULT_NETMASK_BYTE4;

	networkConfig._defaultGateway._byte1 = DEFAULT_GATEWAY_BYTE1;
	networkConfig._defaultGateway._byte2 = DEFAULT_GATEWAY_BYTE2;
	networkConfig._defaultGateway._byte3 = DEFAULT_GATEWAY_BYTE3;
	networkConfig._defaultGateway._byte4 = DEFAULT_GATEWAY_BYTE4;

	// modules initialization ...
	// 1. Init clocks
	// 2. Init IO ports
	// 3. Init other peripherials
	// 4. Init network settings

    initNetworks(&networkConfig);

    while(1)
    {
    }
}
