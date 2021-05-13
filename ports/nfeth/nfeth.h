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


/*
 *   ARP packet structure.
 */

typedef  struct arp_pkt {
     uint16  hardware_space;    /* Hardware address space identifier        */
     uint16  protocol_space;    /* Protocol address space identifier        */
     uint8   hardware_len;      /* Length of hardware address               */
     uint8   protocol_len;      /* Length of protocol address               */
     uint16  op_code;           /* Operation Code                           */
     uint8   src_ether[ETH_ALEN];      /* Sender's hardware address                */
     uint32  src_ip;            /* Sender's protocol address                */
     uint8   dest_ether[ETH_ALEN];     /* Target's hardware address                */
     uint32  dest_ip;           /* Target's protocol address                */
 } ARP;

#define  ARP_HARD_ETHER    1



/*--------------------------------------------------------------------------*/


/*
 *   ARP cache entry.
 */

typedef  struct arp_entry {
     int16   valid;             /* Validity flag                            */
     uint32  ip_addr;           /* IP address                               */
     uint8   ether[ETH_ALEN];   /* EtherNet station address                 */
     struct arp_entry  *next;   /* Address of next ARP in chain             */
 } ARP_ENTRY;



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

typedef struct  {
	PORT generic;
	int ethX;
	uint8 address[ETH_ALEN];
	uint32 ip_host;
} MYPORT;

#define MAX_ETH		4

int16 xmit_dgram(MYPORT *port, IP_DGRAM *dgram, uint8 *data);
int16 fetch_dgram(MYPORT *port, IP_DGRAM ** dgram);
void recve_dgram(MYPORT *port, uint8 *data, int16 length);
void deplete_queue(IP_DGRAM **queue);
void arp_init(void);
void send_block(int ethX, void *cp, short len);


#endif /* ETHER_H */
