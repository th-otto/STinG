/*********************************************************************/
/*                                                                   */
/*     Low Level Port : Serielle Schnittstellen                      */
/*                                                                   */
/*                                                                   */
/*      Version 1.1                       vom 31. Januar 1997        */
/*                                                                   */
/*      Modul zum Datentransfer                                      */
/*                                                                   */
/*********************************************************************/


#include <tos.h>
#include <stdio.h>
#include <string.h>

#include "transprt.h"
#include "port.h"

#include "serial.h"
#include "ppp_fcs.h"


#undef FIONREAD
#undef FIONWRITE
#define		FIONREAD		(('F' << 8) | 1)
#define		FIONWRITE		(('F' << 8) | 2)

static uint8 ppp_header[4] = { PPP_ADDR, PPP_CNTRL, 0, 0 };



static int16 ppp_out(uint8 *destination, uint16 *fcs, uint32 send_accm, uint8 *source, uint16 length)
{
	int16 count = 0;
	uint8 byte;

	while (length--)
	{
		*fcs = (*fcs >> 8) ^ fcs_tab[(*fcs ^ (byte = *source++)) & 0xff];

		if (byte < 32)
		{
			if (((1L << byte) & send_accm) == 0)
				*destination++ = byte;
			else
				*destination++ = PPP_ESC, *destination++ = byte ^ 0x20, count++;
			count++;
		} else
		{
			switch (byte)
			{
			case PPP_FLAG:
				*destination++ = PPP_ESC;
				*destination++ = PPP_FLAG ^ 0x20;
				count += 2;
				break;
			case PPP_ESC:
				*destination++ = PPP_ESC;
				*destination++ = PPP_ESC ^ 0x20;
				count += 2;
				break;
			default:
				*destination++ = byte;
				count++;
				break;
			}
		}
	}

	return count;
}


static void ppp_do_cp(SERIAL_PORT *port)
{
	uint8 *work;
	uint8 chksum[2];
	uint8 *data;
	uint16 fcs;
	uint16 count;
	uint16 packet;

	work = port->send_buffer;

	for (count = 0; count < port->ppp.cp_send_len; count += packet + 2)
	{
		*work++ = PPP_FLAG;
		data = port->ppp.cp_send_data + count;
		packet = (data[4] << 8) | data[5];
		ppp_header[2] = data[0];
		ppp_header[3] = data[1];
		fcs = 0xffffu;
		work += ppp_out(work, &fcs, 0xffffffffuL, ppp_header, 4);
		work += ppp_out(work, &fcs, 0xffffffffuL, &data[2], packet);
		fcs ^= 0xffffu;
		chksum[0] = (uint8) (fcs & 0x00ff);
		chksum[1] = (uint8) ((fcs >> 8) & 0x00ff);
		work += ppp_out(work, &fcs, 0xffffffffuL, chksum, 2);
	}

	*work++ = PPP_FLAG;
	port->send_length = work - port->send_buffer;
	KRfree(port->ppp.cp_send_data);
	port->ppp.cp_send_len = 0;
}


static int16 slip_out(uint8 *destination, uint8 *source, int16 length)
{
	int count = 0;

	while (length--)
	{
		switch (*source)
		{
		case SLIP_END:
			*destination++ = SLIP_ESC;
			*destination++ = SLIP_DATEND;
			count += 2;
			break;
		case SLIP_ESC:
			*destination++ = SLIP_ESC;
			*destination++ = SLIP_DATESC;
			count += 2;
			break;
		default:
			*destination++ = *source;
			count++;
			break;
		}
		source++;
	}

	return count;
}


static int16 wrap_ip(SERIAL_PORT *port)
{
	IP_DGRAM *dgram;
	VJHC *comp;
	int16 compressed;
	int16 type;
	int16 offs;
	int16 hdr;
	int16 len;
	int16 posi;
	uint32 accm;
	uint16 fcs = 0xffffu;
	uint8 *work;
	uint8 chksum[2];

	port->send_length = 0;

	do
	{
		if ((dgram = port->generic.send) == NULL)
			return FALSE;
		port->generic.send = dgram->next;
	} while (check_dgram_ttl(dgram) != E_NORMAL);

	compressed = ((port->generic.flags & FLG_VJHC) != 0) ? TRUE : FALSE;
	work = port->send_buffer;

	comp = port->vjhc;

	if ((port->generic.flags & FLG_PRTCL) == 0)
	{
		*work++ = SLIP_END;
		if (compressed)
		{
			switch (type = vjhc_compress(dgram, comp))
			{
			case VJHC_TYPE_IP:
			case VJHC_TYPE_UNCOMPR_TCP:
				*((uint8 *) & dgram->hdr) |= type;
				work += slip_out(work, (uint8 *) & dgram->hdr, 20);
				work += slip_out(work, dgram->options, dgram->opt_length);
				work += slip_out(work, dgram->pkt_data, dgram->pkt_length);
				break;
			case VJHC_TYPE_COMPR_TCP:
				comp->header[comp->begin] |= VJHC_TYPE_COMPR_TCP;
				work += slip_out(work, comp->header + comp->begin, comp->length);
				offs = ((TCP_HDR *) dgram->pkt_data)->offset << 2;
				work += slip_out(work, (uint8 *) dgram->pkt_data + offs, dgram->pkt_length - offs);
				break;
			}
		} else
		{
			work += slip_out(work, (uint8 *) & dgram->hdr, 20);
			work += slip_out(work, dgram->options, dgram->opt_length);
			work += slip_out(work, dgram->pkt_data, dgram->pkt_length);
		}
		*work++ = SLIP_END;
	} else
	{
		hdr = ppp_header[2] = 0;
		len = posi = 4;
		if ((port->generic.flags & FLG_PRTCL_COMP) != 0)
		{
			len -= 1;
			posi -= 1;
		}
		if ((port->generic.flags & FLG_A_C_COMP) != 0)
		{
			len -= 2;
			hdr += 2;
		}
		*work++ = PPP_FLAG;
		accm = port->ppp.send_accm;
		if (compressed)
		{
			switch (type = vjhc_compress(dgram, comp))
			{
			case VJHC_TYPE_IP:
			case VJHC_TYPE_UNCOMPR_TCP:
				ppp_header[posi - 1] = type == VJHC_TYPE_IP ? PPP_IP : PPP_VJHC_UNC;
				work += ppp_out(work, &fcs, accm, &ppp_header[hdr], len);
				work += ppp_out(work, &fcs, accm, (uint8 *) & dgram->hdr, 20);
				work += ppp_out(work, &fcs, accm, dgram->options, dgram->opt_length);
				work += ppp_out(work, &fcs, accm, dgram->pkt_data, dgram->pkt_length);
				break;
			case VJHC_TYPE_COMPR_TCP:
				ppp_header[posi - 1] = PPP_VJHC_C;
				work += ppp_out(work, &fcs, accm, &ppp_header[hdr], len);
				work += ppp_out(work, &fcs, accm, comp->header + comp->begin, comp->length);
				offs = ((TCP_HDR *) dgram->pkt_data)->offset << 2;
				work += ppp_out(work, &fcs, accm, (uint8 *) dgram->pkt_data + offs, dgram->pkt_length - offs);
				break;
			}
		} else
		{
			ppp_header[posi - 1] = PPP_IP;
			work += ppp_out(work, &fcs, accm, &ppp_header[hdr], len);
			work += ppp_out(work, &fcs, accm, (uint8 *) & dgram->hdr, 20);
			work += ppp_out(work, &fcs, accm, dgram->options, dgram->opt_length);
			work += ppp_out(work, &fcs, accm, dgram->pkt_data, dgram->pkt_length);
		}
		fcs ^= 0xffffu;
		chksum[0] = (uint8) (fcs & 0x00ff);
		chksum[1] = (uint8) ((fcs >> 8) & 0x00ff);
		work += ppp_out(work, &fcs, accm, chksum, 2);
		*work++ = PPP_FLAG;
	}

	IP_discard(dgram, TRUE);
	port->send_length = work - port->send_buffer;

	return TRUE;
}


static int16 fetch_datagram(SERIAL_PORT *port)
{
	port->send_length = port->send_index = 0;

	if ((port->generic.flags & FLG_PRTCL) && port->ppp.cp_send_len)
	{
		ppp_do_cp(port);
	} else
	{
		if (!wrap_ip(port))
			return FALSE;
	}

	port->generic.stat_sd_data += port->send_length;

	return TRUE;
}


void cdecl my_send(PORT *port)
{
	SERIAL_PORT *serial;
	uint8 *walk;
	int16 remain;
	int32 ready;
	int32 ret;

	if (port->driver != &my_driver || !port->active)
		return;

	serial = (SERIAL_PORT *) port;
	if (serial->is_magx)
	{
		ready = 0;
		Fcntl(serial->handle, (long)&ready, FIONWRITE);
	} else
	{
		ready = bconmap_bcostat(serial->handler);
	}

	if (ready != 0)
	{
		walk = serial->send_buffer + serial->send_index;
		remain = serial->send_length - serial->send_index;
		do
		{
			if (remain == 0)
			{
				if (!fetch_datagram(serial))
					return;
				walk = serial->send_buffer;
				serial->send_index = 0;
				remain = serial->send_length;
			}
			if (serial->is_magx)
				ret = Fwrite(serial->handle, remain, walk);
			else
				ret = bconmap_write(serial->handler, remain, walk);
			if ((short) ret <= 0)
				break;
			walk += (short)ret;
		} while ((remain -= (short)ret) >= 0);

		serial->send_index = serial->send_length - remain;
	}
}


void make_IP_dgram(uint8 *buffer, int16 buff_len, IP_DGRAM **dgram)
{
	IP_DGRAM *temp;

	*dgram = NULL;

	if (buff_len < (int16)sizeof(IP_HDR))
		return;

	if ((temp = KRmalloc(sizeof(IP_DGRAM))) == NULL)
		return;

	memcpy(&temp->hdr, buffer, sizeof(IP_HDR));
	buffer += sizeof(IP_HDR);

	if (temp->hdr.length > buff_len || temp->hdr.hd_len < 5 || (temp->hdr.hd_len << 2) > buff_len)
	{
		KRfree(temp);
		return;
	}

	temp->options = KRmalloc(temp->opt_length = (temp->hdr.hd_len << 2) - sizeof(IP_HDR));
	temp->pkt_data = KRmalloc(temp->pkt_length = temp->hdr.length - (temp->hdr.hd_len << 2));

	if (temp->options == NULL || temp->pkt_data == NULL)
	{
		IP_discard(temp, TRUE);
		return;
	}

	memcpy(temp->options, buffer, temp->opt_length);
	memcpy(temp->pkt_data, buffer + temp->opt_length, temp->pkt_length);

	*dgram = temp;
}


static int16 slip_unwrap(SERIAL_PORT *port, int16 *type)
{
	uint8 *p_read;
	uint8 *p_write;
	uint8 *p_last;
	uint8 first;
	int16 error = FALSE;

	p_last = (p_read = p_write = port->recve_buffer) + port->recve_length;

	if (*p_read == SLIP_END)
		p_read++;

	while (p_read < p_last)
	{
		if (*p_read == SLIP_ESC)
		{
			switch (*++p_read)
			{
			case SLIP_DATEND:
				*p_read = SLIP_END;
				break;
			case SLIP_DATESC:
				*p_read = SLIP_ESC;
				break;
			default:
				error = TRUE;
				break;
			}
		}
		*p_write++ = *p_read++;
	}

	if (*p_read != SLIP_END)
		error = TRUE;

	first = *port->recve_buffer;

	if (!error)
	{
		if (first & 0x80u)
		{
			*type = VJHC_TYPE_COMPR_TCP;
		} else
		{
			if (first >= 0x70u)
			{
				*type = VJHC_TYPE_UNCOMPR_TCP;
				*port->recve_buffer &= ~0x30u;
			} else
			{
				*type = VJHC_TYPE_IP;
			}
		}
	} else
	{
		*type = VJHC_TYPE_ERROR;
	}

	return (int16) (p_write - port->recve_buffer);
}


static int16 ppp_unwrap(SERIAL_PORT *port, uint16 *protocol, uint8 **data)
{
	uint8 *p_read;
	uint8 *p_write;
	uint8 *p_last;
	uint16 fcs = 0xffffu;

	p_last = (p_read = p_write = port->recve_buffer) + port->recve_length;

	if (*p_read == PPP_FLAG)
		p_read++;

	while (p_read < p_last)
	{
		if (*p_read < 32)
		{
			if (((1L << *p_read) & port->ppp.recve_accm) != 0)
			{
				p_read++;
				continue;
			}
		}
		if (*p_read == PPP_ESC)
		{
			if (*++p_read == PPP_FLAG)
				return 0;
			*p_read ^= 0x20;
		}
		fcs = (fcs >> 8) ^ fcs_tab[(fcs ^ *p_read) & 0xff];
		*p_write++ = *p_read++;
	}

	if (fcs != 0xf0b8 || p_write - port->recve_buffer < 6)
		return 0;

	p_read = port->recve_buffer;
	p_write -= 3;

	if (p_read[0] == PPP_ADDR && p_read[1] == PPP_CNTRL)
		p_read += 2;

	if ((*p_read & 0x01) == 0)
		*protocol = (*p_read << 8) | *(p_read + 1);
	else
		*protocol = *p_read;

	*data = p_read + ((*p_read & 0x01) ? 1 : 2);

	if ((*protocol & 0x01) == 0)
		return 0;

	return (int16) (p_write - p_read);
}


static void process_datagram(SERIAL_PORT *port)
{
	IP_DGRAM *dgram;
	IP_DGRAM *walk;
	IP_DGRAM **previous;
	uint8 *data;
	int16 len;
	int16 compression;
	int16 type;
	uint16 protocol;

	compression = (port->generic.flags & FLG_VJHC) ? TRUE : FALSE;
	port->generic.stat_rcv_data += port->recve_length;

	if (port->generic.flags & FLG_PRTCL)
	{
		len = ppp_unwrap(port, &protocol, &data);
		if (len > 0)
		{
			switch (protocol)
			{
			case PPP_IP:
				compression = FALSE;
				break;
			case PPP_VJHC_UNC:
				compression = TRUE;
				type = VJHC_TYPE_UNCOMPR_TCP;
				break;
			case PPP_VJHC_C:
				compression = TRUE;
				type = VJHC_TYPE_COMPR_TCP;
				break;
			default:
				ppp_control(port, protocol, data, len);
				return;
			}
		} else
		{
			compression = TRUE;
			type = VJHC_TYPE_ERROR;
			data = NULL;
		}
	} else
	{
		data = port->recve_buffer;
		len = slip_unwrap(port, &type);
	}

	if ((port->generic.flags & FLG_PRTCL) != 0 && port->ppp.ipcp.state != PPP_OPENED)
	{
		port->generic.stat_dropped++;
		return;
	}

	if (!compression)
		make_IP_dgram(data, len, &dgram);
	else
		vjhc_uncompress(data, len, type, port->vjhc, &dgram);

	if (dgram == NULL)
	{
		port->generic.stat_dropped++;
		return;
	}

	dgram->recvd = &port->generic;
	dgram->next = NULL;
	set_dgram_ttl(dgram);

	previous = &port->generic.receive;

	for (walk = *previous; walk; walk = *(previous = &walk->next))
		;
	*previous = dgram;
}


void cdecl my_receive(PORT *port)
{
	SERIAL_PORT *serial;
	uint8 *walk, mark;
	uint8 *end, *recve_buffer;
	int32 remain;

	if (port->driver != &my_driver || !port->active)
		return;

	serial = (SERIAL_PORT *) port;

	if (serial->is_magx)
	{
		remain = 0;
		Fcntl(serial->handle, (long) &remain, FIONREAD);
	} else
	{
		remain = bconmap_bconstat(serial->handler);
	}

	if (remain != 0)
	{
		walk = serial->recve_buffer + serial->recve_index;
		remain = serial->recve_buffer + 8190 - walk;
		
		if (serial->is_magx)
			remain = Fread(serial->handle, remain, walk);
		else
			remain = bconmap_read(serial->handler, remain, walk);

		if (remain <= 0)
			return;
		mark = ((serial->generic.flags & FLG_PRTCL) == 0) ? SLIP_END : PPP_FLAG;
		end = walk + remain;
		recve_buffer = serial->recve_buffer;
		do {
			if (*walk++ == mark)
			{
				if ((walk - serial->recve_buffer) > 1)
				{
					serial->recve_length = (int)(walk - serial->recve_buffer - 1);
					process_datagram(serial);
				}
				serial->recve_buffer = walk;
			}
		} while (walk < end);
		if (recve_buffer < serial->recve_buffer)
		{
			int count;
			
			serial->recve_index = (int) (walk - serial->recve_buffer);
			if (serial->recve_index != 0)
			{
				count = serial->recve_index;
				walk = recve_buffer;
				end = serial->recve_buffer;
				while (--count >= 0)
					*walk++ = *end++;
			}
			serial->recve_buffer = recve_buffer;
			return;
		}
		if ((serial->recve_index = (int) (walk - recve_buffer)) >= 8190)
		{
			serial->generic.stat_dropped++;
			serial->recve_index = 0;
		}
	}
}


#if 0
long bconmap_bconstat(MAPTAB *handler)
{
	IOREC *iorec = handler->iorec;
	unsigned short size;
	
	size = iorec->ibuftl - iorec->ibufhd;
	if ((unsigned short)iorec->ibuftl < (unsigned short)iorec->ibufhd)
		size += (unsigned short)iorec->ibufsiz;
	return size;
}


long bconmap_bcostat(MAPTAB *handler)
{
	IOREC *iorec = handler->iorec;
	unsigned short size;
	
	size = (unsigned short)iorec[1].ibufhd - (unsigned short)iorec[1].ibuftl;
	if ((unsigned short)iorec[1].ibufhd <= (unsigned short)iorec[1].ibuftl)
		size += (unsigned short)iorec[1].ibufsiz;
	if (size >= 2u)
		size -= 2;
	return size;
}
#endif
