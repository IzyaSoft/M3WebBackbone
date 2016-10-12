#ifndef ETHERNET_MAC_CONFIG_H
#define ETHERNET_MAC_CONFIG_H

#include "lpc17xx_emac.h"

#define ETHERNET_MODE EMAC_MODE_AUTO

extern const uint8_t ETHERNET_MAC_ADDRESS[];

extern const uint8_t APP_DEFAULT_IP_ADDRESS[];
extern const uint8_t APP_DEFAULT_NETMASK[];

extern const uint8_t APP_DEFAULT_GATEWAY[];
extern const uint8_t APP_DEFAULT_NAMESERVER[];

#endif
