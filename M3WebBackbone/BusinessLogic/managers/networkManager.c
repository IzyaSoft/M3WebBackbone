#include "networkManager.h"
/*#include "lwip/tcpip.h"
#include "lwip/netif.h"
#include "netif/ethernetif.h"


void initNetworks(struct NetworkConfig* config)
{
    initEthernetChip();
    initLwIp(config);
}

void initEthernetChip()
{
     // LPC1768 contains onboard ethernet chip, therefore this method is empty.
}

void initLwIp(struct NetworkConfig* config)
{
	struct netif xnetif;
    // ip_addr_t ipAddress;
    // ip_addr_t networkMask;
    // ip_addr_t gateway;

    tcpip_init(NULL, NULL);
    //IP4_ADDR(&ipAddress, config->_ipV4Address._byte1,);
    netif_add(&xnetif, &config->_ipV4Address._dword, &config->_networkMask._dword, &config->_defaultGateway._dword, NULL, &ethernetif_init, &tcpip_input);
}*/
