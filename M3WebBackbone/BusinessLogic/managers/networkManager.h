#ifndef NETWORK_MANAGER_H
#define NETWORK_MANAGER_H

#include <stdint.h>
#include "data/networkConfig.h"

void initNetworks(struct NetworkConfig* config);
void initEthernetChip();
void initLwIp(struct NetworkConfig* config);

#endif
