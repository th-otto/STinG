/*********************************************************************/
/*                                                                   */
/*     Low Level Port : EtherNet Network Interface                   */
/*                                                                   */
/*                                                                   */
/*      Version 0.1                        vom 26. Januar 1998       */
/*                                                                   */
/*      Modul fuer Verschiedenes                                     */
/*                                                                   */
/*********************************************************************/


#include <stdio.h>
#include <string.h>

#include "transprt.h"
#include "port.h"

#include "nfeth.h"


#define  ARP_NUM     32
#define ARP_REQUEST 1
#define ARP_REPLY   2


static ARP_ENTRY arp_entries[ARP_NUM];
static ARP_ENTRY *cache;



static int16 arp_cache(uint32 ip_addr, ARP_ENTRY **entry)
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

	*entry = walk;

	return TRUE;
}


static int16 launch_arp(MYPORT *port, uint32 ip_address, uint8 *data)
{
	ETH_HDR *ethptr;
	ARP *arp;

	ethptr = (ETH_HDR *)data;
	memset(ethptr->destination, 0xff, ETH_ALEN);
	memcpy(ethptr->source, port->address, ETH_ALEN);
	ethptr->type = htons(TYPE_ARP);

	arp = (ARP *) &ethptr->data[0];
	arp->hardware_space = ARP_HARD_ETHER;
	arp->hardware_len = ETH_ALEN;
	arp->protocol_space = TYPE_IP;
	arp->protocol_len = 4;
	arp->op_code = ARP_REQUEST;
	memcpy(arp->src_ether, port->address, ETH_ALEN);
	memset(arp->dest_ether, 0xff, ETH_ALEN);
	arp->src_ip = port->generic.ip_addr;
	arp->dest_ip = ip_address;

	memset(arp + 1, 0, 60 - sizeof(ETH_HDR) - sizeof(ARP));
	return sizeof(ETH_HDR) + sizeof(ARP);
}


static int16 send_dgram(MYPORT *port, IP_DGRAM *dgram, uint8 ether[ETH_ALEN], uint8 *data)
{
	ETH_HDR *ethptr;
	uint8 *work;

	ethptr = (ETH_HDR *)data;
	memcpy(ethptr->destination, ether, ETH_ALEN);
	memcpy(ethptr->source, port->address, ETH_ALEN);
	ethptr->type = TYPE_IP;

	work = &ethptr->data[0];
	memcpy(work, &dgram->hdr, sizeof(IP_HDR));
	work += sizeof(IP_HDR);
	memcpy(work, dgram->options, dgram->opt_length);
	work += dgram->opt_length;
	memcpy(work, dgram->pkt_data, dgram->pkt_length);
	work += dgram->pkt_length;

	return (int16) (work - (uint8 *) ethptr);
}


int16 xmit_dgram(MYPORT *port, IP_DGRAM *dgram, uint8 *data)
{
	ARP_ENTRY *entry;
	int16 length;
	uint32 network;
	uint32 ip_address;

	network = port->generic.ip_addr & port->generic.sub_mask;

	if ((dgram->hdr.ip_dest & port->generic.sub_mask) == network)
	{
		ip_address = dgram->hdr.ip_dest;
	} else
	{
		if ((dgram->ip_gateway & port->generic.sub_mask) != network)
		{
			port->generic.stat_dropped++;
			IP_discard(dgram, TRUE);
			return 0;
		} else
		{
			ip_address = dgram->ip_gateway;
		}
	}

	if (arp_cache(ip_address, &entry))
		length = send_dgram(port, dgram, entry->ether, data);
	else
		length = launch_arp(port, ip_address, data);

	length = length > 60 ? length : 60;

	IP_discard(dgram, TRUE);
	port->generic.stat_sd_data += length;

	return length;
}


int16 fetch_dgram(MYPORT *port, IP_DGRAM **dgram)
{
	do
	{
		if ((*dgram = port->generic.send) == NULL)
			return FALSE;
		port->generic.send = (*dgram)->next;
	} while (check_dgram_ttl(*dgram) != E_NORMAL);

	return TRUE;
}


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


static void process_arp(MYPORT *port, ETH_HDR *ethptr)
{
	ARP *arp;
	ARP_ENTRY *entry;
	int16 update = FALSE;
	int16 length;

	arp = (ARP *) ethptr->data;

	if (arp->hardware_space != ARP_HARD_ETHER || arp->hardware_len != ETH_ALEN)
		return;
	if (arp->protocol_space != TYPE_IP || arp->protocol_len != 4)
		return;

	if (arp_cache(arp->src_ip, &entry))
	{
		update = TRUE;
		memcpy(entry->ether, arp->src_ether, ETH_ALEN);
	}

	if (arp->dest_ip != port->generic.ip_addr)
		return;

	if (update == FALSE)
		arp_enter(arp->src_ip, arp->src_ether);

	if (arp->op_code == ARP_REPLY)
		return;

	arp->dest_ip = arp->src_ip;
	memcpy(arp->dest_ether, arp->src_ether, ETH_ALEN);
	arp->src_ip = port->generic.ip_addr;
	memcpy(arp->src_ether, port->address, ETH_ALEN);
	arp->op_code = ARP_REPLY;

	memcpy(ethptr->destination, arp->dest_ether, ETH_ALEN);
	memcpy(ethptr->source, arp->src_ether, ETH_ALEN);
	ethptr->type = TYPE_ARP;
	memcpy(ethptr->data, arp, sizeof(ARP));

	length = sizeof(ETH_HDR) + sizeof(ARP);
	length = length > 60 ? length : 60;

	send_block(port->ethX, ethptr, length);
	port->generic.stat_sd_data += length;
}


static void retrieve_dgram(MYPORT *port, uint8 *buffer, int16 length)
{
	IP_DGRAM *dgram;
	IP_DGRAM *walk;
	IP_DGRAM **previous;

	if ((dgram = KRmalloc(sizeof(IP_DGRAM))) == NULL)
	{
		port->generic.stat_dropped++;
		return;
	}

	memcpy(&dgram->hdr, buffer, sizeof(IP_HDR));
	buffer += sizeof(IP_HDR);

	if (dgram->hdr.length > length || dgram->hdr.hd_len < 5 || (dgram->hdr.hd_len << 2) > length)
	{
		KRfree(dgram);
		port->generic.stat_dropped++;
		return;
	}

	dgram->options = KRmalloc(dgram->opt_length = (dgram->hdr.hd_len << 2) - sizeof(IP_HDR));
	dgram->pkt_data = KRmalloc(dgram->pkt_length = dgram->hdr.length - (dgram->hdr.hd_len << 2));

	if (dgram->options == NULL || dgram->pkt_data == NULL)
	{
		IP_discard(dgram, TRUE);
		port->generic.stat_dropped++;
		return;
	}

	memcpy(dgram->options, buffer, dgram->opt_length);
	memcpy(dgram->pkt_data, buffer + dgram->opt_length, dgram->pkt_length);

	dgram->recvd = port;
	dgram->next = NULL;
	set_dgram_ttl(dgram);

	for (walk = *(previous = &port->generic.receive); walk; walk = *(previous = &walk->next))
		;
	*previous = dgram;
}


void recve_dgram(MYPORT *port, uint8 *data, int16 length)
{
	ETH_HDR *ethptr;

	ethptr = (ETH_HDR *)data;

	switch (ethptr->type)
	{
	case TYPE_IP:
		retrieve_dgram(port, &ethptr->data[0], length);
		break;
	case TYPE_ARP:
		process_arp(port, ethptr);
		break;
	}
	port->generic.stat_rcv_data += length;
}


void deplete_queue(IP_DGRAM **queue)
{
	IP_DGRAM *walk;
	IP_DGRAM *next;

	for (walk = *queue; walk; walk = next)
	{
		next = walk->next;
		IP_discard(walk, TRUE);
	}

	*queue = NULL;
}


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
