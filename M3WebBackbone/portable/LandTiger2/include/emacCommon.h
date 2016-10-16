#ifndef EMAC_COMMON_H
#define EMAC_COMMON_H

#include <stdint.h>

/**
 * @brief RX Descriptor structure type definition
 */
typedef struct {
	uint32_t Packet;	/**< Receive Packet Descriptor */
	uint32_t Ctrl;		/**< Receive Control Descriptor */
} RX_Desc;

/**
 * @brief RX Status structure type definition
 */
typedef struct {
	uint32_t Info;		/**< Receive Information Status */
	uint32_t HashCRC;	/**< Receive Hash CRC Status */
} RX_Stat;

/**
 * @brief TX Descriptor structure type definition
 */
typedef struct {
	uint32_t Packet;	/**< Transmit Packet Descriptor */
	uint32_t Ctrl;		/**< Transmit Control Descriptor */
} TX_Desc;

/**
 * @brief TX Status structure type definition
 */
typedef struct {
   uint32_t Info;		/**< Transmit Information Status */
} TX_Stat;


/**
 * @brief TX Data Buffer structure definition
 */
typedef struct {
	uint32_t ulDataLen;			/**< Data length */
	uint32_t *pbDataBuf;		/**< A word-align data pointer to data buffer */
} EMAC_PACKETBUF_Type;

/**
 * @brief EMAC configuration structure definition
 */
typedef struct {
	uint32_t	Mode;						/**< Supported EMAC PHY device speed, should be one of the following:
											- EMAC_MODE_AUTO
											- EMAC_MODE_10M_FULL
											- EMAC_MODE_10M_HALF
											- EMAC_MODE_100M_FULL
											- EMAC_MODE_100M_HALF
											*/
	uint8_t 	*pbEMAC_Addr;				/**< Pointer to EMAC Station address that contains 6-bytes
											of MAC address, it must be sorted in order (bEMAC_Addr[0]..[5])
											*/
}EMAC_CFG_Type;

#endif
