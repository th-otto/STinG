/*
 *      ether.h             (c) Peter Rottengatter  1997
 *                              perot@pallas.amp.uni-hannover.de
 *
 *      Included into the ETHER.STX source code files
 */

#ifndef ETHER_H
#define ETHER_H



/*--------------------------------------------------------------------------*/


/*
 * Hardware address length, 6 bytes for Ethernet
 */
#define ETH_ALEN 6

/*
 *   Ethernet packet header.
 */

typedef  struct eth_hdr {
     uint8   destination[ETH_ALEN];    /* Destination hardware address             */
     uint8   source[ETH_ALEN];         /* Source hardware address                  */
     uint16  type;              /* Ethernet protocol type                   */
     uint8   data[0];           /* Data block                               */
 } ETH_HDR;

#define  TYPE_IP      0x0800
#define  TYPE_ARP     0x0806
#define  TYPE_RARP    0x8035



/*--------------------------------------------------------------------------*/

/* ARP cache */

int16 arp_cache(uint32 ip_addr, uint8 ether[ETH_ALEN], int update);
int16 launch_arp(uint32 ip_address, uint32 src_ip, uint8 address[ETH_ALEN], ETH_HDR *ethptr);
int16 process_arp(uint32 ip_addr, uint8 address[ETH_ALEN], uint8 *buffer, ETH_HDR *ethptr);
void arp_init(void);


/*--------------------------------------------------------------------------*/


/*
 *   Definitions of bits in registers 0 and 3.
 */

#define  CSR0_INIT        0x0001
#define  CSR0_STRT        0x0002
#define  CSR0_STOP        0x0004
#define  CSR0_TDMD        0x0008
#define  CSR0_TXON        0x0010
#define  CSR0_RXON        0x0020
#define  CSR0_INEA        0x0040
#define  CSR0_INTR        0x0080
#define  CSR0_IDON        0x0100
#define  CSR0_TINT        0x0200
#define  CSR0_RINT        0x0400
#define  CSR0_MERR        0x0800
#define  CSR0_MISS        0x1000
#define  CSR0_CERR        0x2000
#define  CSR0_BABL        0x4000
#define  CSR0_ERR         0x8000

#define  CSR3_BCON        0x0001
#define  CSR3_ACON        0x0002
#define  CSR3_BSWP        0x0004



/*--------------------------------------------------------------------------*/


/*
 *   LANCE initialisation block.
 */

typedef  struct rbd {
     uint16    dra_low;         /* Address of descriptor (lower 16 bit)     */
     uint8     length;          /* Two's logarithm of the number of entries */
     uint8     dra_high;        /* Address of descriptor (higher 8 bit)     */
 } DRP;

typedef  struct lance_init {
     uint16  mode;              /* Mode word                                */
     uint8   addr[ETH_ALEN];    /* Hardware address of this Etherport       */
     uint32  ladrf[2];          /* Logical address filter for multicasting  */
     DRP     rdrp;              /* Pointer to receive descriptor ring       */
     DRP     tdrp;              /* Pointer to transmit descriptor ring      */
 } LANCE_INIT;



/*--------------------------------------------------------------------------*/


/*
 *   Receive descriptor ring entry.
 */

typedef  struct rmd {
     uint16    addr_low;        /* Address of data block (lower 16 bit)     */
     uint8     status;          /* Eigth error and status flags             */
     uint8     addr_high;       /* Address of data block (higher 8 bit)     */
     unsigned  ones    : 4;     /* Four bits being one                      */
     unsigned  bcount  : 12;    /* Available space in this data block       */
     unsigned  zeros   : 4;     /* Four bits being zero                     */
     unsigned  mcount  : 12;    /* Amount of received data                  */
 } RMD;



/*--------------------------------------------------------------------------*/


/*
 *   Transmit descriptor ring entry.
 */

typedef  struct tmd {
     uint16    addr_low;        /* Address of data block (lower 16 bit)     */
     uint8     status;          /* Eigth error and status flags             */
     uint8     addr_high;       /* Address of data block (higher 8 bit)     */
     unsigned  ones    : 4;     /* Four bits being one                      */
     unsigned  bcount  : 12;    /* Amount of data in block to be sent       */
     unsigned  buffer  : 1;     /* Buffer-not-owned error flag              */
     unsigned  uflow   : 1;     /* Underflow error flag                     */
     unsigned  reserv  : 1;     /* Reserved (zero)                          */
     unsigned  lcoll   : 1;     /* Late-collision error flag                */
     unsigned  lcar    : 1;     /* Loss-of-carrier error flag               */
     unsigned  retry   : 1;     /* Retries-exceeded error flag              */
     unsigned  tdr     : 10;    /* Time domain reflectometry                */
 } TMD;



/*--------------------------------------------------------------------------*/


/*
 *   Status and error flags in descriptor ring entries.
 */

#define  DS_OWN     0x80
#define  DS_ERR     0x40

#define  DS_FRAM    0x20
#define  DS_OFLO    0x10
#define  DS_CRC     0x08
#define  DS_BUFF    0x04

#define  DS_AFCS    0x20
#define  DS_MORE    0x10
#define  DS_ONE     0x08
#define  DS_DEF     0x04

#define  DS_STP     0x02
#define  DS_ENP     0x01



/*--------------------------------------------------------------------------*/


/*
 *   Buffer access block.
 */

typedef  struct bab {
     union {
          RMD  *recve_buff;     /* Receive :  Receive Memory Descriptor     */
          TMD  *xmit_buff;      /* Transmit : Transmit Memory Descriptor    */
      } buffer;
     ETH_HDR     *data;         /* Address of data block                    */
     struct bab  *next_bab;     /* Address of next BAB in circular chain    */
 } BAB;



/*--------------------------------------------------------------------------*/


/*
 *   Hardware addresses.
 */

#define  PAM_RDP              (uint16 *) 0xFECFFFF0L
#define  PAM_RAP              (uint16 *) 0xFECFFFF2L
#define  PAM_MEMBOT             (void *) 0xFECF0000L
#define  PAM_IRQ

#define  RIEBL_MEGA_RDP       (uint16 *) 0x00FF7000L
#define  RIEBL_MEGA_RAP       (uint16 *) 0x00FF7002L
#define  RIEBL_MEGA_MEMBOT      (void *) 0x00E00000L
#define  RIEBL_MEGA_IRQ                  0x1d

#define  RIEBL_HACK_RDP       (uint16 *) 0x00FF7000L
#define  RIEBL_HACK_RAP       (uint16 *) 0x00FF7002L
#define  RIEBL_HACK_MEMBOT      (void *) 0x00D00000L
#define  RIEBL_HACK_IRQ                  0x1d

#define  RIEBL_MSTE_RDP       (uint16 *) 0x00C0FFF0L
#define  RIEBL_MSTE_RAP       (uint16 *) 0x00C0FFF2L
#define  RIEBL_MSTE_MEMBOT      (void *) 0x00C10000L
#define  RIEBL_MSTE_IRQ                  0x50

#define  RIEBL_TT_RDP         (uint16 *) 0xFE00FFF0L
#define  RIEBL_TT_RAP         (uint16 *) 0xFE00FFF2L
#define  RIEBL_TT_MEMBOT        (void *) 0xFE010000L
#define  RIEBL_TT_IRQ                    0x50



/*--------------------------------------------------------------------------*/

extern PORT my_port;
extern BAB *this_xmit;
extern uint8 address[ETH_ALEN];

int16 xmit_dgram(IP_DGRAM *dgram, BAB *txbab);
int16 send_dgram(IP_DGRAM *dgram, uint8 ether[ETH_ALEN], BAB *txbab);
int16 fetch_dgram(IP_DGRAM ** dgram);
void recve_dgram(BAB *rxbab);
void retrieve_dgram(uint8 *buffer, int16 length);
void deplete_queue(IP_DGRAM **queue);


#endif /* ETHER_H */
