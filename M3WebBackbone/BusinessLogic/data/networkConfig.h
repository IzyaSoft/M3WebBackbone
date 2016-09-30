#ifndef NETWORK_CONFIG_H
#define NETWORK_CONFIG_H

#include <stdint.h>

struct IpV4Parameter
{
	union
	{
		uint8_t _byte1;
		uint8_t _byte2;
		uint8_t _byte3;
		uint8_t _byte4;
	};
	uint32_t _dword;
};

struct NetworkConfig
{
	struct IpV4Parameter _ipV4Address;
	struct IpV4Parameter _networkMask;
	struct IpV4Parameter _defaultGateway;
};

#endif
