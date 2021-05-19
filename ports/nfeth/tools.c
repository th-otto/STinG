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
	uint8 ether[ETH_ALEN];
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

	if (arp_cache(ip_address, ether, FALSE))
		length = send_dgram(port, dgram, ether, data);
	else
		length = launch_arp(ip_address, port->generic.ip_addr, port->address, (ETH_HDR *)data);

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

	port->generic.stat_rcv_data += length;
	switch (ethptr->type)
	{
	case TYPE_IP:
		retrieve_dgram(port, &ethptr->data[0], length);
		break;
	case TYPE_ARP:
		length = process_arp(port->generic.ip_addr, port->address, ethptr->data, ethptr);
		if (length != 0)
		{
			send_block(port->ethX, ethptr, length);
			port->generic.stat_sd_data += length;
		}
		break;
	}
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
