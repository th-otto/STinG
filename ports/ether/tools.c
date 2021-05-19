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

#include "ether.h"



int16 xmit_dgram(IP_DGRAM *dgram, BAB *txbab)
{
	uint8 ether[ETH_ALEN];
	int16 length;
	uint32 network;
	uint32 ip_address;

	network = my_port.ip_addr & my_port.sub_mask;

	if ((dgram->hdr.ip_dest & my_port.sub_mask) == network)
	{
		ip_address = dgram->hdr.ip_dest;
	} else
	{
		if ((dgram->ip_gateway & my_port.sub_mask) != network)
		{
			my_port.stat_dropped++;
			IP_discard(dgram, TRUE);
			return FALSE;
		} else
		{
			ip_address = dgram->ip_gateway;
		}
	}

	if (arp_cache(ip_address, ether, FALSE))
		length = send_dgram(dgram, ether, txbab);
	else
		length = launch_arp(ip_address, my_port.ip_addr, address, txbab->data);

	length = length > 60 ? length : 60;
	txbab->buffer.xmit_buff->bcount = -length;
	txbab->buffer.xmit_buff->status |= DS_OWN;

	IP_discard(dgram, TRUE);
	my_port.stat_sd_data += length;

	return TRUE;
}


int16 send_dgram(IP_DGRAM *dgram, uint8 ether[ETH_ALEN], BAB *txbab)
{
	ETH_HDR *ethptr;
	uint8 *work;

	ethptr = txbab->data;
	memcpy(ethptr->destination, ether, ETH_ALEN);
	memcpy(ethptr->source, address, ETH_ALEN);
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


int16 fetch_dgram(IP_DGRAM **dgram)
{
	do
	{
		if ((*dgram = my_port.send) == NULL)
			return FALSE;
		my_port.send = (*dgram)->next;
	} while (check_dgram_ttl(*dgram) != E_NORMAL);

	return TRUE;
}


void recve_dgram(BAB *rxbab)
{
	ETH_HDR *ethptr;
	int16 length;

	length = rxbab->buffer.recve_buff->mcount;
	ethptr = rxbab->data;

	if ((rxbab->buffer.recve_buff->status & (DS_ERR | DS_STP | DS_ENP)) == (DS_STP | DS_ENP))
	{
		my_port.stat_rcv_data += length;
		switch (ethptr->type)
		{
		case TYPE_IP:
			retrieve_dgram(&ethptr->data[0], length);
			break;
		case TYPE_ARP:
			length = process_arp(my_port.ip_addr, address, ethptr->data, this_xmit->data);
			if (length != 0)
			{
				this_xmit->buffer.xmit_buff->bcount = -length;
				this_xmit->buffer.xmit_buff->status |= DS_OWN;
			
				this_xmit = this_xmit->next_bab;
				my_port.stat_sd_data += length;
			}
			break;
		}
	} else
	{
		my_port.stat_dropped++;
	}

	rxbab->buffer.recve_buff->status |= DS_OWN;
}


void retrieve_dgram(uint8 *buffer, int16 length)
{
	IP_DGRAM *dgram;
	IP_DGRAM *walk;
	IP_DGRAM **previous;

	if ((dgram = KRmalloc(sizeof(IP_DGRAM))) == NULL)
	{
		my_port.stat_dropped++;
		return;
	}

	memcpy(&dgram->hdr, buffer, sizeof(IP_HDR));
	buffer += sizeof(IP_HDR);

	if (dgram->hdr.length > length || dgram->hdr.hd_len < 5 || (dgram->hdr.hd_len << 2) > length)
	{
		KRfree(dgram);
		my_port.stat_dropped++;
		return;
	}

	dgram->options = KRmalloc(dgram->opt_length = (dgram->hdr.hd_len << 2) - sizeof(IP_HDR));
	dgram->pkt_data = KRmalloc(dgram->pkt_length = dgram->hdr.length - (dgram->hdr.hd_len << 2));

	if (dgram->options == NULL || dgram->pkt_data == NULL)
	{
		IP_discard(dgram, TRUE);
		my_port.stat_dropped++;
		return;
	}

	memcpy(dgram->options, buffer, dgram->opt_length);
	memcpy(dgram->pkt_data, buffer + dgram->opt_length, dgram->pkt_length);

	dgram->recvd = &my_port;
	dgram->next = NULL;
	set_dgram_ttl(dgram);

	for (walk = *(previous = &my_port.receive); walk; walk = *(previous = &walk->next))
		;
	*previous = dgram;
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
