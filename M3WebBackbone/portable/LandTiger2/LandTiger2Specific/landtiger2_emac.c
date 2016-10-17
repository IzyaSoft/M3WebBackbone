#include "landtiger2_emac.h"
#include "LPC17xx.h"

static unsigned short *rptr;
static unsigned short *tptr;

/*
// Rx Descriptor data array
static RX_Desc rxDescriptor[NUM_RX_FRAG];

// Rx Status data array - Must be 8-Byte aligned
#if defined ( __CC_ARM   )
static __align(8) RX_Stat rxStatus[NUM_RX_FRAG];
#elif defined ( __ICCARM__ )
#pragma data_alignment=8
static RX_Stat rxStatus[NUM_RX_FRAG];
#elif defined   (  __GNUC__  )
static __attribute__ ((aligned (8))) RX_Stat rxStatus[NUM_RX_FRAG];
#endif
// Tx Descriptor data array
static TX_Desc txDescriptor[NUM_TX_FRAG];
// Tx Status data array
static TX_Stat txStatus[NUM_TX_FRAG];
// EMAC local DMA buffers

// Rx buffer data
static uint32_t rxBuffer[NUM_RX_FRAG][ETH_MAX_FLEN>>2];
// Tx buffer data
static uint32_t txBuffer[NUM_TX_FRAG][ETH_MAX_FLEN>>2];
*/
static int32_t write_PHY (int32_t phyReg, int32_t value)
{
    uint32_t tout;

    LPC_EMAC->MADR = DP83848C_DEF_ADR | phyReg;
    LPC_EMAC->MWTD = value;

    /* Wait until operation completed */
    for (tout = 0; tout < MII_WR_TOUT; tout++)
    {
        if ((LPC_EMAC->MIND & MIND_BUSY) == 0)
            return 0;
    }
    return -1;
}

static int32_t read_PHY (byte phyReg)
{
    uint32_t tout;

    LPC_EMAC->MADR = DP83848C_DEF_ADR | phyReg;
    LPC_EMAC->MCMD = MCMD_READ;

    /* Wait until operation completed */
    for (tout = 0; tout < MII_RD_TOUT; tout++)
    {
        if((LPC_EMAC->MIND & MIND_BUSY) == 0)
        {
        	LPC_EMAC->MCMD = 0;
            return (LPC_EMAC->MRDD);
        }
    }
    return -1;
}

void rx_descr_init (void)
{
    /* Initialize Receive Descriptor and Status array. */

/*	for (uint32_t i = 0; i < NUM_RX_FRAG; i++)
	{
		rxDescriptor[i].Packet = (uint32_t)&rxBuffer[i];
		rxDescriptor[i].Ctrl = RCTRL_INT | (ETH_MAX_FLEN - 1);
		rxStatus[i].Info = 0;
		rxStatus[i].HashCRC = 0;
	}*/

	for (uint32_t i = 0; i < NUM_RX_FRAG; i++)
	{
		RX_DESC_PACKET(i) = RX_BUF(i);
	    RX_DESC_CTRL(i) = RCTRL_INT | (ETH_FRAG_SIZE-1);
	    RX_STAT_INFO(i) = 0;
	    RX_STAT_HASHCRC(i) = 0;
	}

    /* Set EMAC Receive Descriptor Registers. */
	//LPC_EMAC->RxDescriptor = (uint32_t)&rxDescriptor[0];
	//LPC_EMAC->RxStatus = (uint32_t)&rxStatus[0];

	LPC_EMAC->RxDescriptorNumber = NUM_RX_FRAG - 1;
    LPC_EMAC->RxDescriptor = RX_DESC_BASE;
    LPC_EMAC->RxStatus = RX_STAT_BASE;
	/* Rx Descriptors Point to 0 */
	LPC_EMAC->RxConsumeIndex  = 0;
}

void tx_descr_init (void)
{
    for (uint32_t i = 0; i < NUM_TX_FRAG; i++)
    {
        TX_DESC_PACKET(i) = TX_BUF(i);
        TX_DESC_CTRL(i) = 0;
        TX_STAT_INFO(i) = 0;
    }

  /* Set EMAC Transmit Descriptor Registers. */
  LPC_EMAC->TxDescriptor = TX_DESC_BASE;
  LPC_EMAC->TxStatus = TX_STAT_BASE;
  LPC_EMAC->TxDescriptorNumber = NUM_TX_FRAG-1;

  /* Tx Descriptors Point to 0 */
  LPC_EMAC->TxProduceIndex  = 0;
}


// configure port-pins for use with LAN-controller,
// reset it and send the configuration-sequence

Bool InitializeEthernetMAC(EMAC_CFG_Type* emacConfig)
{
    // Initializes the EMAC ethernet controller
    uint32_t regv, tout, id1, id2;

    /* Power Up the EMAC controller. */
    LPC_SC->PCONP |= 0x40000000;

    /* For the first silicon rev.'-' ID P1.6 should be set, but on rev. 'A' and later, P1.6 should NOT be set. */
    LPC_PINCON->PINSEL2 = 0x50150105;

    LPC_PINCON->PINSEL3 = (LPC_PINCON->PINSEL3 & ~0x0000000F) | 0x00000005;

    /* Reset all EMAC internal modules. */
    LPC_EMAC->MAC1 = MAC1_RES_TX | MAC1_RES_MCS_TX | MAC1_RES_RX | MAC1_RES_MCS_RX | MAC1_SIM_RES | MAC1_SOFT_RES;
    // MAC_COMMAND = CR_REG_RES | CR_TX_RES | CR_RX_RES;
    LPC_EMAC->Command = CR_REG_RES | CR_TX_RES | CR_RX_RES | CR_PASS_RUNT_FRM;
  
    /* A short delay after reset. */
    for (tout = 100; tout; tout--);

    /* Initialize MAC control registers. */
    LPC_EMAC->MAC1 = MAC1_PASS_ALL;
    LPC_EMAC->MAC2 = MAC2_CRC_EN | MAC2_PAD_EN;
    //  MAC2 = MAC2_CRC_EN | MAC2_PAD_EN | MAC2_VLAN_PAD_EN;

    LPC_EMAC->MAXF = ETH_MAX_FLEN;
    LPC_EMAC->CLRT = CLRT_DEF;
    LPC_EMAC->IPGR = IPGR_DEF;

    /* Enable Reduced MII interface. */
    LPC_EMAC->Command = CR_RMII | CR_PASS_RUNT_FRM;

    /* Put the DP83848C in reset mode */
    write_PHY (PHY_REG_BMCR, 0x8000);

    /* Wait for hardware reset to end. */
    for (tout = 0; tout < 0x100000; tout++)
    {
        regv = read_PHY(PHY_REG_BMCR);
        if (!(regv & 0x8000))
            break;    /* Reset complete */
    }

    if(regv & 0x8000)
        return FALSE;

    /* Check if this is a DP83848C PHY. */
    id1 = read_PHY(PHY_REG_IDR1);
    id2 = read_PHY(PHY_REG_IDR2);

    if (((id1 << 16) | (id2 & 0xFFF0)) == DP83848C_ID)
    {
        /* Configure the PHY device */

        /* Use autonegotiation about the link speed. */
        write_PHY(PHY_REG_BMCR, PHY_AUTO_NEG);
        /* Wait to complete Auto_Negotiation. */
        for (tout = 0; tout < 0x100000; tout++)
        {
            regv = read_PHY(PHY_REG_BMSR);
            if (regv & 0x0020)
                break; /* Autonegotiation Complete. */
        }
    }

    if (!(regv & 0x0020))
    	return FALSE;

    /* Check the link status. */
    for (tout = 0; tout < 0x10000; tout++)
    {
        regv = read_PHY(PHY_REG_STS);
        if (regv & 0x0001)
            break;/* Link is on. */
    }

    if(!(regv & 0x0001))
    	return FALSE;

    // todo: umv : settings for spedd mode & duplex from parameter

    /* Configure Full/Half Duplex mode. */
    if (regv & 0x0004)
    {
        /* Full duplex is enabled. */
        LPC_EMAC->MAC2    |= MAC2_FULL_DUP;
        LPC_EMAC->Command |= CR_FULL_DUP;
        LPC_EMAC->IPGT     = IPGT_FULL_DUP;
    }
    else LPC_EMAC->IPGT = IPGT_HALF_DUP;/* Half duplex mode. */

    /* Configure 100MBit/10MBit mode. */
    if (regv & 0x0002)
    {
        /* 10MBit mode. */
        LPC_EMAC->SUPP = 0;
    }
    else
    {
        /* 100MBit mode. */
        LPC_EMAC->SUPP = SUPP_SPEED;
    }

    /* Set the Ethernet MAC Address registers */
    LPC_EMAC->SA0 = (emacConfig->pbEMAC_Addr[0] << 8) | emacConfig->pbEMAC_Addr[1];
    LPC_EMAC->SA1 = (emacConfig->pbEMAC_Addr[2] << 8) | emacConfig->pbEMAC_Addr[3];
    LPC_EMAC->SA2 = (emacConfig->pbEMAC_Addr[4] << 8) | emacConfig->pbEMAC_Addr[5];

    /* Initialize Tx and Rx DMA Descriptors */
    rx_descr_init();
    tx_descr_init();

    /* Receive Broadcast and Perfect Match Packets */
    LPC_EMAC->RxFilterCtrl = RFC_BCAST_EN | RFC_PERFECT_EN;

    /* Enable EMAC interrupts. */
    LPC_EMAC->IntEnable = INT_RX_DONE | INT_TX_DONE;

    /* Reset all interrupts */
    LPC_EMAC->IntClear  = 0xFFFF;

    /* Enable receive and transmit mode of MAC Ethernet core */
    LPC_EMAC->Command |= (CR_RX_EN | CR_TX_EN);
    LPC_EMAC->MAC1 |= MAC1_REC_EN;
    return TRUE;
}

int32_t CheckPhysicalState(uint32_t parameter)
{
	regv = read_PHY(PHY_REG_BMSR);
    return -1;
}

void WriteData()
{

}

void ReadData()
{

}

/*
// reads a word in little-endian byte order from RX_BUFFER

unsigned short ReadFrame_EMAC(void)
{
  return (*rptr++);
}

// reads a word in big-endian byte order from RX_FRAME_PORT
// (useful to avoid permanent byte-swapping while reading
// TCP/IP-data)

unsigned short ReadFrameBE_EMAC(void)
{
  unsigned short ReturnValue;

  ReturnValue = SwapBytes (*rptr++);
  return (ReturnValue);
}


// copies bytes from frame port to MCU-memory
// NOTES: * an odd number of byte may only be transfered
//          if the frame is read to the end!
//        * MCU-memory MUST start at word-boundary

void CopyFromFrame_EMAC(void *Dest, unsigned short Size)
{
  unsigned short * piDest;                       // Keil: Pointer added to correct expression

  piDest = Dest;                                 // Keil: Line added
  while (Size > 1) {
    *piDest++ = ReadFrame_EMAC();
    Size -= 2;
  }
  
  if (Size) {                                         // check for leftover byte...
    *(unsigned char *)piDest = (char)ReadFrame_EMAC();// the LAN-Controller will return 0
  }                                                   // for the highbyte
}

// does a dummy read on frame-I/O-port
// NOTE: only an even number of bytes is read!

void DummyReadFrame_EMAC(unsigned short Size)    // discards an EVEN number of bytes
{                                                // from RX-fifo
  while (Size > 1) {
    ReadFrame_EMAC();
    Size -= 2;
  }
}

// Reads the length of the received ethernet frame and checks if the 
// destination address is a broadcast message or not
// returns the frame length
unsigned short StartReadFrame(void) {
  unsigned short RxLen;
  unsigned int idx;

  idx = LPC_EMAC->RxConsumeIndex;
  RxLen = (RX_STAT_INFO(idx) & RINFO_SIZE) - 3;
  rptr = (unsigned short *)RX_DESC_PACKET(idx);
  return(RxLen);
}

void EndReadFrame(void) {
  unsigned int idx;

  // DMA free packet.
  idx = LPC_EMAC->RxConsumeIndex;
  if (++idx == NUM_RX_FRAG) idx = 0;
  LPC_EMAC->RxConsumeIndex = idx;
}

unsigned int CheckFrameReceived(void) {             // Packet received ?

  if (LPC_EMAC->RxProduceIndex != LPC_EMAC->RxConsumeIndex) // more packets received ?		
    return(1);
  else 
    return(0);
}

// requests space in EMAC memory for storing an outgoing frame

void RequestSend(unsigned short FrameSize)
{
  unsigned int idx;

  idx  = LPC_EMAC->TxProduceIndex;
  tptr = (unsigned short *)TX_DESC_PACKET(idx);
  TX_DESC_CTRL(idx) = FrameSize | TCTRL_LAST;
}

// check if ethernet controller is ready to accept the
// frame we want to send

unsigned int Rdy4Tx(void)
{
  return (1);   // the ethernet controller transmits much faster
}               // than the CPU can load its buffers


// writes a word in little-endian byte order to TX_BUFFER
void WriteFrame_EMAC(unsigned short Data)
{
  *tptr++ = Data;
}

// copies bytes from MCU-memory to frame port
// NOTES:  an odd number of byte may only be transfered
//          if the frame is written to the end!
//         MCU-memory MUST start at word-boundary

void CopyToFrame_EMAC(void *source, uint32_t size)
{
  unsigned short * piSource;
  unsigned int idx;

  piSource = source;
  size = (size + 1) & 0xFFFE;    // round Size up to next even number
  while (size > 0) {
    WriteFrame_EMAC(*piSource++);
    size -= 2;
  }

  idx = LPC_EMAC->TxProduceIndex;
  if (++idx == NUM_TX_FRAG) idx = 0;
  LPC_EMAC->TxProduceIndex = idx;
}
*/
