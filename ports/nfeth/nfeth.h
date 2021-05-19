/*
 *      ether.h             (c) Peter Rottengatter  1997
 *                              perot@pallas.amp.uni-hannover.de
 *
 *      Included into the ETHER.STX source code files
 */

#ifndef ETHER_H
#define ETHER_H


/*
 * Hardware address length, 6 bytes for Ethernet
 */
#define ETH_ALEN 6

typedef struct  {
	PORT generic;
	int ethX;
	uint8 address[ETH_ALEN];
	uint32 ip_host;
} MYPORT;

#define MAX_ETH		4


/*--------------------------------------------------------------------------*/

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
int16 launch_arp(uint32 ip_address, uint32 src_ip, uint8 address[ETH_ALEN], ETH_HDR *ethhdr);
int16 process_arp(uint32 ip_addr, uint8 address[ETH_ALEN], uint8 *buffer, ETH_HDR *ethptr);
void arp_init(void);


/*--------------------------------------------------------------------------*/

int16 xmit_dgram(MYPORT *port, IP_DGRAM *dgram, uint8 *data);
int16 fetch_dgram(MYPORT *port, IP_DGRAM ** dgram);
void recve_dgram(MYPORT *port, uint8 *data, int16 length);
void deplete_queue(IP_DGRAM **queue);
void arp_init(void);
void send_block(int ethX, void *cp, short len);


#endif /* ETHER_H */
