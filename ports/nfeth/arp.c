#include <stdio.h>
#include <string.h>

#include "transprt.h"
#include "port.h"

#include "nfeth.h"

/*
 *   ARP cache entry.
 */

typedef  struct arp_entry {
     int16   valid;             /* Validity flag                            */
     uint32  ip_addr;           /* IP address                               */
     uint8   ether[ETH_ALEN];   /* EtherNet station address                 */
     struct arp_entry  *next;   /* Address of next ARP in chain             */
 } ARP_ENTRY;

#define  ARP_NUM     32



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
#define ARP_REQUEST 1
#define ARP_REPLY   2

#define  ARP_HARD_ETHER    1


static ARP_ENTRY arp_entries[ARP_NUM];
static ARP_ENTRY *cache;


/*--------------------------------------------------------------------------*/

int16 arp_cache(uint32 ip_addr, uint8 ether[ETH_ALEN], int update)
{
	ARP_ENTRY *walk;
	ARP_ENTRY **previous;

	for (walk = *(previous = &cache); walk; walk = *(previous = &walk->next))
	{
		if (walk->valid)
		{
			if (walk->ip_addr == ip_addr)
				break;
		}
	}

	if (!walk)
		return FALSE;

	*previous = walk->next;
	walk->next = cache;
	cache = walk;

	if (update)
	{
		walk->ether[0] = ether[0];
		walk->ether[1] = ether[1];
		walk->ether[2] = ether[2];
		walk->ether[3] = ether[3];
		walk->ether[4] = ether[4];
		walk->ether[5] = ether[5];
	} else
	{
		ether[0] = walk->ether[0];
		ether[1] = walk->ether[1];
		ether[2] = walk->ether[2];
		ether[3] = walk->ether[3];
		ether[4] = walk->ether[4];
		ether[5] = walk->ether[5];
	}

	return TRUE;
}

/*--------------------------------------------------------------------------*/

int16 launch_arp(uint32 ip_address, uint32 src_ip, uint8 address[ETH_ALEN], ETH_HDR *ethptr)
{
	ARP *arp;

	memset(ethptr->destination, 0xff, ETH_ALEN);
	memcpy(ethptr->source, address, ETH_ALEN);
	ethptr->type = htons(TYPE_ARP);

	arp = (ARP *) &ethptr->data[0];
	arp->hardware_space = ARP_HARD_ETHER;
	arp->hardware_len = ETH_ALEN;
	arp->protocol_space = TYPE_IP;
	arp->protocol_len = 4;
	arp->op_code = ARP_REQUEST;
	memcpy(arp->src_ether, address, ETH_ALEN);
	memset(arp->dest_ether, 0xff, ETH_ALEN);
	arp->src_ip = src_ip;
	arp->dest_ip = ip_address;

	memset(arp + 1, 0, 60 - sizeof(ETH_HDR) - sizeof(ARP));
	return sizeof(ETH_HDR) + sizeof(ARP);
}

/*--------------------------------------------------------------------------*/

static void arp_enter(uint32 ip_addr, uint8 ether_addr[ETH_ALEN])
{
	ARP_ENTRY *walk;
	ARP_ENTRY **previous;

	for (walk = *(previous = &cache); walk->next; walk = *(previous = &walk->next))
		;

	*previous = NULL;
	walk->valid = TRUE;
	walk->ip_addr = ip_addr;
	memcpy(walk->ether, ether_addr, ETH_ALEN);

	walk->next = cache;
	cache = walk;
}

/*--------------------------------------------------------------------------*/

int16 process_arp(uint32 ip_addr, uint8 address[ETH_ALEN], uint8 *buffer, ETH_HDR *ethptr)
{
	ARP *arp;
	int16 update = FALSE;
	int16 length;

	arp = (ARP *) buffer;

	if (arp->hardware_space != ARP_HARD_ETHER || arp->hardware_len != ETH_ALEN)
		return 0;
	if (arp->protocol_space != TYPE_IP || arp->protocol_len != 4)
		return 0;

	if (arp_cache(arp->src_ip, arp->src_ether, TRUE))
	{
		update = TRUE;
	}

	if (arp->dest_ip != ip_addr)
		return 0;

	if (update == FALSE)
		arp_enter(arp->src_ip, arp->src_ether);

	if (arp->op_code == ARP_REPLY)
		return 0;

	arp->dest_ip = arp->src_ip;
	memcpy(arp->dest_ether, arp->src_ether, ETH_ALEN);
	arp->src_ip = ip_addr;
	memcpy(arp->src_ether, address, ETH_ALEN);
	arp->op_code = ARP_REPLY;

	memcpy(ethptr->destination, arp->dest_ether, ETH_ALEN);
	memcpy(ethptr->source, arp->src_ether, ETH_ALEN);
	ethptr->type = TYPE_ARP;
	memcpy(ethptr->data, arp, sizeof(ARP));

	length = sizeof(ETH_HDR) + sizeof(ARP);
	length = length > 60 ? length : 60;
	
	return length;
}

/*--------------------------------------------------------------------------*/

void arp_init(void)
{
	int16 count;

	for (count = 0; count < ARP_NUM; count++)
	{
		arp_entries[count].valid = FALSE;
		arp_entries[count].next = &arp_entries[count + 1];
	}
	arp_entries[ARP_NUM - 1].next = NULL;

	cache = &arp_entries[0];
}
