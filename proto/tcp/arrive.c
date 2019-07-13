/*********************************************************************/
/*                                                                   */
/*     High Level Protokoll : TCP                                    */
/*                                                                   */
/*                                                                   */
/*      Version 1.2                          from 28. May 1997       */
/*                                                                   */
/*      Modul fÅr Segment Handler                                    */
/*                                                                   */
/*********************************************************************/


#include <stdio.h>
#include <string.h>

#include "transprt.h"
#include "layer.h"

#include "tcp.h"


int16 cdecl TCP_handler(IP_DGRAM *datagram)
{
	CONNEC *connect;
	CONNEC *option;
	TCP_HDR *hdr;
	IP_DGRAM *walk;
	uint16 len;
	uint16 max_mss;
	uint16 count;
	uint16 found;

	hdr = (TCP_HDR *) datagram->pkt_data;
	len = datagram->pkt_length;

	if (len < sizeof(TCP_HDR))
	{
		my_conf.generic.stat_dropped++;
		return TRUE;
	}

	if (check_sum(datagram->hdr.ip_src, datagram->hdr.ip_dest, hdr, len) != 0)
	{
		my_conf.generic.stat_dropped++;
		return TRUE;
	}

	for (option = NULL, count = 0, connect = root_list; connect; connect = connect->next)
	{
		if (hdr->dest_port == connect->local_port)
		{
			if (connect->remote_port == hdr->src_port &&
				connect->local_IP_address == datagram->hdr.ip_dest &&
				connect->remote_IP_address == datagram->hdr.ip_src)
			{
				if (connect->state != TLISTEN)
					break;
				found = 4;
			} else
			{
				found = 1;
				if (connect->remote_port != 0)
				{
					if (connect->remote_port != hdr->src_port)
						continue;
					found += found;
				}
				if (connect->local_IP_address != 0)
				{
					if (connect->local_IP_address != datagram->hdr.ip_dest)
						continue;
					found++;
				}
				if (connect->remote_IP_address != 0)
				{
					if (connect->remote_IP_address != datagram->hdr.ip_src)
						continue;
					found++;
				}
			}
			if (found > count)
			{
				count = found;
				option = connect;
			}
		}
	}

	if (connect == NULL && option != NULL)
	{
		if (option->state == TLISTEN)
		{
			if (hdr->sync)
			{
				connect = option;
				connect->local_IP_address = datagram->hdr.ip_dest;
				connect->remote_IP_address = datagram->hdr.ip_src;
				connect->remote_port = hdr->src_port;
				if (connect->info)
				{
					connect->info->address.lhost = connect->local_IP_address;
					connect->info->address.rhost = connect->remote_IP_address;
					connect->info->address.rport = connect->remote_port;
					connect->info->status = 0;
				}
				PRTCL_get_parameters(datagram->hdr.ip_src, NULL, &connect->ttl, &connect->mtu);
				max_mss = connect->mtu - sizeof(IP_HDR) - sizeof(TCP_HDR);
				connect->mss = connect->mss < max_mss ? connect->mss : max_mss;
			}
		}
	}

	if (connect == NULL)
	{
		send_reset(datagram);
		my_conf.generic.stat_dropped++;
		return TRUE;
	}
	if (connect->state == TCLOSED)
	{
		send_reset(datagram);
		my_conf.generic.stat_dropped++;
		return TRUE;
	}

	if ((walk = (IP_DGRAM *) KRmalloc(sizeof(IP_DGRAM))) == NULL)
	{
		my_conf.generic.stat_dropped++;
		return TRUE;
	}
	memcpy(walk, datagram, sizeof(IP_DGRAM));

	datagram->options = datagram->pkt_data = NULL;
	datagram = walk;

	datagram->next = NULL;

	if (connect->pending)
	{
		for (walk = connect->pending; walk->next; walk = walk->next)
			;
		walk->next = datagram;
	} else
	{
		connect->pending = datagram;
	}

	return TRUE;
}


static void rtrp_mode(CONNEC *conn)
{
	uint32 elapsed;

	conn->rtrp.mode = 0;
	if (!(conn->flags & RETRAN))
	{
		elapsed = TIMER_elapsed(conn->rtrp.start);
		conn->rtrp.smooth = (conn->rtrp.smooth * 7 + elapsed) / 8;
		if (conn->rtrp.smooth < 100)
		{
			conn->rtrp.timeout = 200;
		} else if (conn->rtrp.smooth > 30000L)
		{
			conn->rtrp.timeout = 60000L;
		} else
		{
			conn->rtrp.timeout = conn->rtrp.smooth * 2;
		}
		conn->rtrn.timeout = conn->rtrp.smooth * 2;
		conn->rtrn.start = TIMER_now();
		conn->rtrn.backoff = 0;
	}
}


void do_arrive(CONNEC *conn, IP_DGRAM *datagram)
{
	TCP_HDR *hdr;
	RESEQU *net_data;
	RESEQU temp;
	NDB *ndb;
	NDB *work;
	int16 stored;
	int16 trim;
	uint16 len;

	hdr = (TCP_HDR *) datagram->pkt_data;
	len = datagram->pkt_length;

	switch (conn->state)
	{
	case TCLOSED:
		conn->rtrp.mode = 0;
		send_reset(datagram);
		conn->net_error = E_UA;
		my_conf.generic.stat_dropped++;
		return;
	case TLISTEN:
		if (hdr->reset)
		{
			return;
		}
		if (hdr->ack)
		{
			conn->rtrp.mode = 0;
			send_reset(datagram);
			my_conf.generic.stat_dropped++;
			return;
		}
		if (hdr->sync)
		{
			process_sync(conn, datagram);
			send_sync(conn);
			my_conf.con_in++;
			conn->state = TSYN_RECV;
			if (len - hdr->offset * 4 > 0 || hdr->fin)
				break;
			++conn->recve.next;
			do_output(conn);
		} else
		{
			my_conf.generic.stat_dropped++;
		}
		return;
	case TSYN_SENT:
		if (hdr->ack)
		{
			if (!sequ_within_range(hdr->acknowledge, conn->send.ini_sequ + 1, conn->send.next))
			{
				conn->rtrp.mode = 0;
				send_reset(datagram);
				conn->net_error = E_UA;
				my_conf.generic.stat_dropped++;
				return;
			}
		}
		if (hdr->reset)
		{
			if (hdr->ack)
			{
				conn->rtrp.mode = 0;
				close_self(conn, E_RRESET);
				conn->net_error = E_REFUSE;
			}
			return;
		}
		if (hdr->sync)
		{
			process_sync(conn, datagram);
		}
#if 0 /* causes immediate transfer aborts with some providers */
		if (hdr->ack && PREC(datagram->hdr.tos) != PREC(conn->tos))
		{
			conn->rtrp.mode = 0;
			send_reset(datagram);
			conn->net_error = E_UA;
			my_conf.generic.stat_dropped++;
			return;
		}
#endif
		if (hdr->sync)
		{
			if (hdr->ack)
			{
				update_wind(conn, hdr);
				conn->state = TESTABLISH;
			} else
			{
				conn->state = TSYN_RECV;
			}
			if (len - hdr->offset * 4 > 0 || hdr->fin)
				break;
			if (conn->rtrp.mode)
				rtrp_mode(conn);
			++conn->recve.next;
			do_output(conn);
		}
		if (hdr->ack && conn->o140 < 0xffff)
		{
			if (conn->o140 <= conn->o142)
			{
				conn->o140 += conn->mss;
			} else
			{
				if (conn->smooth_start == 0 ||
					(uint32)TIMER_elapsed(conn->smooth_start) > conn->rtrp.smooth)
				{
					conn->smooth_start = TIMER_now();
					conn->o148 = 0;
				}
				if (conn->o148 < conn->mss)
				{
					uint16 tmp = (conn->mss * conn->mss) / conn->o140;
					if (conn->o148 + tmp > conn->mss)
						tmp = conn->mss - conn->o148;
					conn->o140 += tmp;
					conn->o148 += tmp;
				}
			}
		}
		return;
	}

	if (!hdr->sync)
		process_options(conn, datagram);

	if (!trim_segm(conn, datagram, &net_data, TRUE))
	{
		if (!hdr->reset)
		{
			conn->flags |= FORCE;
			conn->rtrp.mode = 0;
			do_output(conn);
		}
		KRfree(net_data);
		return;
	}
	datagram->pkt_data = NULL;

	if (conn->rtrp.mode && ((int32)hdr->acknowledge - (int32)conn->rtrp.sequ) >= 0)
		rtrp_mode(conn);
	if (hdr->ack && conn->o140 < 0xffff)
	{
		if (conn->o140 <= conn->o142)
		{
			conn->o140 += conn->mss;
		} else
		{
			if (conn->smooth_start == 0 ||
				(uint32)TIMER_elapsed(conn->smooth_start) > conn->rtrp.smooth)
			{
				conn->smooth_start = TIMER_now();
				conn->o148 = 0;
			}
			if (conn->o148 < conn->mss)
			{
				uint16 tmp = (conn->mss * conn->mss) / conn->o140;
				if (conn->o148 + tmp > conn->mss)
					tmp = conn->mss - conn->o148;
				conn->o140 += tmp;
				conn->o148 += tmp;
			}
		}
	}

	if ((conn->flags & DISCARD) && net_data->data_len != 0)
	{
		KRfree(net_data->hdr);
		KRfree(net_data);
		my_conf.generic.stat_dropped++;
		abort_conn(conn);
		close_self(conn, E_CNTIMEOUT);
		conn->net_error = E_CNTIMEOUT;
		return;
	}
	if ((conn->flags & CLOSING) && net_data->hdr->reset)
	{
		KRfree(net_data->hdr);
		KRfree(net_data);
		close_self(conn, E_CNTIMEOUT);
		conn->net_error = E_CNTIMEOUT;
		return;
	}

	if (hdr->sequence != conn->recve.next && (net_data->data_len != 0 || hdr->fin))
	{
		add_resequ(conn, net_data);
		return;
	}

	for (;;)
	{
		hdr = net_data->hdr;
		len = (uint16) (net_data->data + net_data->data_len - (uint8 *) net_data->hdr);
		stored = FALSE;

		if (hdr->reset)
		{
			if (conn->state == TSYN_RECV && conn->act_pass == TCP_PASSIVE)
			{
				conn->local_IP_address = conn->local_IP_address_orig;
				conn->local_port = conn->lport_orig;
				conn->remote_IP_address = conn->remote_IP_address_orig;
				conn->remote_port = conn->rport_orig;
				conn->state = TLISTEN;
			} else
			{
				close_self(conn, E_RRESET);
				conn->net_error = E_RRESET;
			}
			KRfree(net_data->hdr);
			KRfree(net_data);
			return;
		}

		if (
#if 0 /* causes immediate transfer aborts with some providers */
		PREC(net_data->tos) != PREC(conn->tos) ||
#endif
			hdr->sync)
		{
			datagram->pkt_data = hdr;
			datagram->pkt_length = len;
			KRfree(net_data);
			send_reset(datagram);
			conn->net_error = E_UA;
			my_conf.generic.stat_dropped++;
			return;
		}

		if (!hdr->ack)
		{
			KRfree(net_data->hdr);
			KRfree(net_data);
			my_conf.generic.stat_dropped++;
			return;
		}

		switch (conn->state)
		{
		case TSYN_RECV:
			if (sequ_within_range(hdr->acknowledge, conn->send.unack + 1, conn->send.next))
			{
				update_wind(conn, hdr);
				conn->state = TESTABLISH;
			} else
			{
				datagram->pkt_data = hdr;
				datagram->pkt_length = len;
				KRfree(net_data);
				send_reset(datagram);
				conn->net_error = E_UA;
				my_conf.generic.stat_dropped++;
				return;
			}
			break;
		case TESTABLISH:
		case TCLOSE_WAIT:
		case TFIN_WAIT2:
			update_wind(conn, hdr);
			break;
		case TFIN_WAIT1:
			update_wind(conn, hdr);
			if (conn->send.count == 0)
				conn->state = TFIN_WAIT2;
			break;
		case TCLOSING:
			update_wind(conn, hdr);
			if (conn->send.count == 0)
			{
				conn->state = TTIME_WAIT;
				conn->rtrn.mode = TRUE;
				conn->rtrn.start = TIMER_now();
				conn->rtrn.timeout = 2 * my_conf.max_slt;
			}
			break;
		case TLAST_ACK:
			update_wind(conn, hdr);
			if (conn->send.count == 0)
			{
				close_self(conn, E_NORMAL);
				KRfree(net_data->hdr);
				KRfree(net_data);
				return;
			}
			/* fall through */
		case TTIME_WAIT:
			conn->flags |= FORCE;
			conn->rtrn.mode = TRUE;
			conn->rtrn.start = TIMER_now();
			conn->rtrn.timeout = 2 * my_conf.max_slt;
			break;
		}

		if (net_data->data_len != 0)
		{
			switch (conn->state)
			{
			case TSYN_RECV:
			case TESTABLISH:
			case TFIN_WAIT1:
			case TFIN_WAIT2:
				temp = *net_data;
				ndb = (NDB *) net_data;
				ndb->ptr = (char *) temp.hdr;
				ndb->ndata = (char *) temp.data;
				ndb->len = temp.data_len;
				ndb->next = NULL;
				if (conn->recve.queue)
				{
					for (work = conn->recve.queue; work->next; work = work->next)
						;
					work->next = ndb;
				} else
				{
					conn->recve.queue = ndb;
				}
				conn->recve.count += ndb->len;
				conn->recve.next += ndb->len;
				conn->recve.window -= ndb->len;
				conn->flags |= FORCE;
				stored = TRUE;
				break;
			}
		}

		if (hdr->fin)
		{
			conn->flags |= FORCE;
			switch (conn->state)
			{
			case TSYN_RECV:
			case TESTABLISH:
				conn->recve.next++;
				conn->state = TCLOSE_WAIT;
				break;
			case TFIN_WAIT1:
				if (conn->send.count != 0)
				{
					conn->recve.next++;
					conn->state = TCLOSING;
					break;
				}
				/* fall through */
			case TFIN_WAIT2:
				conn->recve.next++;
				conn->state = TTIME_WAIT;
				conn->rtrn.mode = TRUE;
				conn->rtrn.start = TIMER_now();
				conn->rtrn.timeout = 2 * my_conf.max_slt;
				break;
			case TCLOSE_WAIT:
			case TCLOSING:
			case TLAST_ACK:
				break;
			case TTIME_WAIT:
				conn->rtrn.mode = TRUE;
				conn->rtrn.start = TIMER_now();
				conn->rtrn.timeout = 2 * my_conf.max_slt;
				break;
			}
		}

		if (!stored)
		{
			KRfree(net_data->hdr);
			KRfree(net_data);
		}

		for (trim = FALSE; conn->recve.reseq != NULL;)
		{
			if ((int32) conn->recve.next - (int32) conn->recve.reseq->hdr->sequence < 0)
				break;
			net_data = conn->recve.reseq;
			conn->recve.reseq = net_data->next;
			if (trim_segm(conn, NULL, &net_data, FALSE))
			{
				trim = TRUE;
				break;
			}
			KRfree(net_data->hdr);
			KRfree(net_data);
		}
		if (!trim)
			break;
	}

	do_output(conn);
}
