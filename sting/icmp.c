/*********************************************************************/
/*                                                                   */
/*     STinG : API and IP kernel package                             */
/*                                                                   */
/*                                                                   */
/*      Version 1.0                      from 23. November 1996      */
/*                                                                   */
/*      Module for InterNet Control Message Protocol                 */
/*                                                                   */
/*********************************************************************/


#include <stdio.h>
#include <string.h>

#include "globdefs.h"


#define  M_YEAR    1996
#define  M_MONTH   11
#define  M_DAY     23
#define  M_VERSION "01.00"


LAYER icmp_desc = {
	"ICMP",
	M_VERSION,
	0L,
	((M_YEAR - 1980) << 9) | (M_MONTH << 5) | M_DAY,
	"Peter Rottengatter",
	0,
	NULL,
	NULL
};

static uint16 icmp_id = 0;



static uint16 layer_checksum(struct icmp_header *header, int16 length)
{
	uint32 chksum;
	uint16 *walk;
	uint16 count;

	header->checksum = 0;

	chksum = 0;

	for (walk = (uint16 *) header, count = 0; count < length / 2; walk++, count++)
		chksum += *walk;

	if (length & 1)
		chksum += (uint16) (*(uint8 *) walk) << 8;

	chksum = (chksum & 0xffffL) + ((chksum >> 16) & 0xffffL);

	return ~(uint16) ((chksum & 0x10000L) ? chksum + 1 : chksum);
}


int16 ICMP_reply(uint8 type, uint8 code, IP_DGRAM *dgram, uint32 supple)
{
	IP_DGRAM *walk;
	IP_DGRAM **previous;
	int32 time_stamp;
	uint32 ip;
	uint16 length;
	uint16 status;
	uint8 *packet;
	struct icmp_header *header;

	ip = dgram->hdr.ip_src;

	if (ip == 0L || (ip >> 24) == 0xe0 || dgram->hdr.frag_ofst)
	{
		ICMP_discard(dgram);
		return FALSE;
	}

	switch (type)
	{
	case ICMP_ECHO_REPLY:
		code = 0;
		break;
	case ICMP_DEST_UNREACH:
	case ICMP_SRC_QUENCH:
	case ICMP_REDIRECT:
	case ICMP_TIME_EXCEED:
	case ICMP_PARAMETER:
		length = 8 + dgram->hdr.hd_len * 4 + 8;
		if ((packet = KRmalloc(length)) == NULL)
		{
			icmp_desc.stat_dropped++;
			ICMP_discard(dgram);
			return FALSE;
		}
		memcpy(packet + 8, &dgram->hdr, 20);
		memcpy(packet + 28, dgram->options, dgram->opt_length);
		if (dgram->pkt_data)
			memcpy(packet + 28 + dgram->opt_length, dgram->pkt_data, 8);
		*((uint32 *) packet + 1) = supple;
		if (dgram->pkt_data)
			KRfree(dgram->pkt_data);
		dgram->pkt_data = packet;
		dgram->pkt_length = length;
		break;
	case ICMP_STAMP_REPLY:
		code = 0;
		if ((time_stamp = sting_clock + ((int32) icmp_desc.flags >> 16) * 60000L) < 0)
			time_stamp += MAX_CLOCK;
		if (time_stamp >= MAX_CLOCK)
			time_stamp -= MAX_CLOCK;
		*((uint32 *) dgram->pkt_data + 3) = time_stamp;
		*((uint32 *) dgram->pkt_data + 4) = time_stamp;
		dgram->pkt_length = 20;
		break;
	case ICMP_MASK_REPLY:
		code = 0;
		*((uint32 *) dgram->pkt_data + 2) = ((PORT *)dgram->recvd)->sub_mask;
		dgram->pkt_length = 12;
		break;
	default:
		icmp_desc.stat_dropped++;
		ICMP_discard(dgram);
		return FALSE;
	}

	header = (struct icmp_header *)dgram->pkt_data;
	header->type = type;
	header->code = code;

	header->checksum = layer_checksum(header, dgram->pkt_length);

	dgram->hdr.length = dgram->hdr.hd_len * 4 + dgram->pkt_length;
	dgram->hdr.ident = icmp_id++;
	dgram->hdr.dont_frg = TRUE;
	dgram->hdr.more_frg = FALSE;
	dgram->hdr.frag_ofst = 0;
	dgram->hdr.ttl = conf.ttl + 1;
	dgram->hdr.protocol = P_ICMP;
	dgram->hdr.ip_src = ((PORT *)dgram->recvd)->ip_addr;
	dgram->hdr.ip_dest = ip;

	dgram->hdr.hdr_chksum = 0;
	dgram->hdr.hdr_chksum = check_sum(&dgram->hdr, dgram->options, dgram->opt_length);

	dgram->timeout = sting_clock + dgram->hdr.ttl * 1000L - 1;
	dgram->next = NULL;

	if (dgram->timeout >= MAX_CLOCK)
		dgram->timeout -= MAX_CLOCK;

	status = lock_exec(0);

	for (walk = *(previous = &my_port.receive); walk; walk = *(previous = &walk->next))
		;
	*previous = dgram;

	lock_exec(status);

	return TRUE;
}


int16 cdecl ICMP_process(IP_DGRAM *dgram)
{
	FUNC_LIST *walk;
	struct icmp_header *header;
	uint16 checksum;
		
	header = (struct icmp_header *)dgram->pkt_data;

	checksum = header->checksum;
	if (checksum != layer_checksum(header, dgram->pkt_length))
	{
		icmp_desc.stat_dropped++;
		ICMP_discard(dgram);
		return TRUE;
	}

	switch (header->type)
	{
	case ICMP_ECHO:
		ICMP_reply(ICMP_ECHO_REPLY, 0, dgram, 0L);
		break;
	case ICMP_STAMP_REQU:
		ICMP_reply(ICMP_STAMP_REPLY, 0, dgram, 0L);
		break;
	case ICMP_MASK_REQU:
		if ((icmp_desc.flags & 1) == 0)
			break;
		ICMP_reply(ICMP_MASK_REPLY, 0, dgram, 0L);
		break;
	default:
		for (walk = conf.icmp; walk; walk = walk->next)
		{
			if (walk->handler(dgram))
				break;
		}
		if (walk == NULL)
		{
			icmp_desc.stat_dropped++;
			ICMP_discard(dgram);
		}
	}

	return TRUE;
}


int16 cdecl ICMP_send(uint32 dest, uint8 type, uint8 code, const void *data, uint16 dat_length)
{
	uint16 length;
	uint8 *packet;
	struct icmp_header *header;

	if (dest == 0 || (dest >> 24) == 0xe0)
		return E_BADDNAME;

	if ((packet = KRmalloc((length = 4 + dat_length) + 1)) == NULL)
		return E_NOMEM;

	header = (struct icmp_header *)packet;
	memcpy(packet + 4, data, dat_length);
	packet[length] = '\0';
	header->type = type;
	header->code = code;

	header->checksum = layer_checksum(header, length);

	if (IP_send(0, dest, 0, 1, conf.ttl, P_ICMP, icmp_id++, packet, length, NULL, 0) != E_NORMAL)
	{
		KRfree(packet);
		return E_NOMEM;
	}
	return E_NORMAL;
}


int16 cdecl ICMP_handler(int16 cdecl (*handler)(IP_DGRAM *), int16 flag)
{
	FUNC_LIST *walk;
	FUNC_LIST *previous;
	FUNC_LIST *this;
	FUNC_LIST *prev_this;

	this = prev_this = previous = NULL;

	for (walk = conf.icmp; walk; walk = walk->next)
	{
		if (walk->handler == handler)
			this = walk, prev_this = previous;
		previous = walk;
	}

	switch (flag)
	{
	case HNDLR_SET:
	case HNDLR_FORCE:
		if (this != NULL)
			return FALSE;
		if ((this = KRmalloc(sizeof(FUNC_LIST))) == NULL)
			return FALSE;
		this->handler = handler;
		this->next = conf.icmp;
		conf.icmp = this;
		return TRUE;
	case HNDLR_REMOVE:
		if (this == NULL)
			return FALSE;
		if (prev_this)
			prev_this->next = this->next;
		else
			conf.icmp = this->next;
		KRfree(this);
		return TRUE;
	case HNDLR_QUERY:
		return this ? TRUE : FALSE;
	}

	return FALSE;
}


void cdecl ICMP_discard(IP_DGRAM *dgram)
{
	IP_discard(dgram, TRUE);
}
