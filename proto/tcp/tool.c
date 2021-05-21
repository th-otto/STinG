/*********************************************************************/
/*                                                                   */
/*     High Level Protokoll : TCP                                    */
/*                                                                   */
/*                                                                   */
/*      Version 1.0                         from 4. March 1997       */
/*                                                                   */
/*      Modul fÅr Werkzeuge                                          */
/*                                                                   */
/*********************************************************************/


#include <stdio.h>

#include "transprt.h"
#include "layer.h"

#include "tcp.h"
#include "icmp.h"


static uint32 ini_sequ;
static uint32 ini_sequ_next;



static void flush_queue(NDB **queue)
{
	NDB *walk;
	NDB *temp;

	if (*queue == NULL)
		return;

	walk = *queue;
	*queue = NULL;
	for (; walk; walk = temp)
	{
		temp = walk->next;
		KRfree(walk->ptr);
		KRfree(walk);
	}
}


int32 cdecl unlink_connect(void *connec)
{
	CONNEC *work;
	CONNEC **previous;

	for (work = *(previous = &root_list); work; work = *(previous = &work->next))
	{
		if (work == connec)
			break;
	}

	if (work)
		*previous = work->next;
	return (int32) previous;
}


void destroy_conn(CONNEC *connec)
{
	IP_DGRAM *ip_walk;
	IP_DGRAM *ip_next;
	RESEQU *rsq_walk;
	RESEQU *rsq_next;

	protect_exec(connec, unlink_connect);
	flush_queue(&connec->send.queue);
	flush_queue(&connec->recve.queue);

	for (ip_walk = connec->pending; ip_walk; ip_walk = ip_next)
	{
		ip_next = ip_walk->next;
		IP_discard(ip_walk, TRUE);
	}

	for (rsq_walk = connec->recve.reseq; rsq_walk; rsq_walk = rsq_next)
	{
		rsq_next = rsq_walk->next;
		KRfree(rsq_walk->hdr);
		KRfree(rsq_walk);
	}

	PRTCL_release(connec->handle);
	KRfree(connec->info);
	KRfree(connec);
}


void abort_conn(CONNEC *connec)
{
	TCP_HDR *hdr;

	if ((hdr = (TCP_HDR *) KRmalloc(sizeof(TCP_HDR))) == NULL)
		return;

	my_conf.resets++;

	hdr->src_port = connec->local_port;
	hdr->dest_port = connec->remote_port;

	hdr->sequence = connec->send.next;
	hdr->acknowledge = 0;

	hdr->urgent = hdr->ack = hdr->push = hdr->sync = hdr->fin = FALSE;
	hdr->reset = TRUE;

	hdr->offset = hdr->resvd = hdr->window = hdr->chksum = hdr->urg_ptr = 0;

	hdr->chksum = check_sum(connec->local_IP_address, connec->remote_IP_address, hdr, sizeof(TCP_HDR));

	IP_send(connec->local_IP_address, connec->remote_IP_address, connec->tos, FALSE,
			connec->ttl, P_TCP, tcp_id++, (uint8 *) hdr, sizeof(TCP_HDR), NULL, 0);
}


static int32 cdecl timer_work(void *param)
{
	CONNEC *connec = param;
	IP_DGRAM *walk;
	IP_DGRAM *next;

	if (req_flag(&connec->sema) != 0)
		return TRUE;

	if (connec->pending)
	{
		for (walk = get_pending(&connec->pending); walk; walk = next)
		{
			next = walk->next;
			do_arrive(connec, walk);
			IP_discard(walk, TRUE);
		}
	}

	if (connec->rtrn.mode)
	{
		if ((uint32)TIMER_elapsed(connec->rtrn.start) > connec->rtrn.timeout)
		{
			connec->rtrn.mode = FALSE;
			if (connec->state != TTIME_WAIT)
			{
				connec->send.ptr = connec->send.unack;
				connec->flags |= RETRAN;
				connec->rtrn.start = TIMER_now();
				connec->rtrn.timeout = connec->rtrp.smooth << ++connec->rtrn.backoff;
				if (connec->send.window <= connec->o140)
					connec->o142 = connec->send.window >> 1;
				else
					connec->o142 = connec->o140 >> 1;
				if (connec->o142 < connec->mss)
					connec->o142 = connec->mss;
				connec->o140 = connec->mss;
				do_output(connec);
			} else
			{
				close_self(connec, E_NORMAL);
			}
		}
	}

	if ((connec->flags & FLAG20) && (connec->flags & FORCE))
	{
		uint32 elapsed;

		if ((elapsed = TIMER_elapsed(connec->send.start)) >= 500 ||
			elapsed >= (connec->rtrp.smooth >> 1))
		{
			do_output(connec);
		}
	}

	if (connec->flags & CLOSING)
	{
		if (connec->state == TCLOSED)
		{
			destroy_conn(connec);
			return FALSE;
		}
		if (connec->state == TTIME_WAIT)
		{
			if (TIMER_elapsed(connec->send.start) > 2 * my_conf.max_slt)
			{
				close_self(connec, E_NORMAL);
				destroy_conn(connec);
				return FALSE;
			}
			if (connec->result != NULL)
			{
				*connec->result = E_NORMAL;
				connec->result = NULL;
			}
		}
		if ((uint32)TIMER_elapsed(connec->close.start) > connec->close.timeout)
		{
			abort_conn(connec);
			close_self(connec, E_CNTIMEOUT);
			destroy_conn(connec);
			return FALSE;
		}
		if (connec->net_error != 0)
		{
			close_self(connec, E_CNTIMEOUT);
			destroy_conn(connec);
			return FALSE;
		}
	}

	connec->last_work = TIMER_now();
	rel_flag(&connec->sema);

	return TRUE;
}


int16 cdecl timer_function(IP_DGRAM *dgram)
{
	CONNEC *connect;
	CONNEC *next;

	(void)dgram;
	for (connect = root_list; connect; connect = next)
	{
		next = connect->next;
		timer_work(connect);
	}
	return 0;
}


int16 poll_receive(CONNEC *connec)
{
	int16 error;

	error = connec->net_error;

	if (error < 0)
	{
		connec->net_error = E_NORMAL;
		return error;
	}

	if (TIMER_elapsed(connec->last_work) < 1200)
		return 0;

	return protect_exec(connec, timer_work) ? E_NORMAL : E_NOCONNECTION;
}


int16 cdecl do_ICMP(IP_DGRAM *dgram)
{
	IP_HDR *ip;
	TCP_HDR *tcp;
	CONNEC *connect;
	uint8 type;
	uint8 code;

	if ((my_conf.generic.flags & PROTO_DO_ICMP) == 0)
		return FALSE;

	type = *(uint8 *) dgram->pkt_data;
	code = *((uint8 *) dgram->pkt_data + 1);

	if (type != ICMP_UNREACH && type != ICMP_SOURCEQUENCH && type != ICMP_TIMXCEED)
		return FALSE;

	ip = (IP_HDR *) ((uint8 *) dgram->pkt_data + 8);

	if (ip->protocol != P_TCP)
		return FALSE;

	tcp = (TCP_HDR *) ((uint8 *) ip + ip->hd_len * 4);

	for (connect = root_list; connect; connect = connect->next)
	{
		if (tcp->src_port != connect->local_port)
			continue;
		if (tcp->dest_port != connect->remote_port)
			continue;
		if (ip->ip_src != connect->local_IP_address)
			continue;
		if (ip->ip_dest != connect->remote_IP_address)
			continue;
		break;
	}

	if (connect == NULL)
	{
		ICMP_discard(dgram);
		return TRUE;
	}

	if (!sequ_within_range(tcp->sequence, connect->send.unack, connect->send.next))
	{
		ICMP_discard(dgram);
		return TRUE;
	}

	if (connect->info)
		connect->info->status = ((uint16) type << 8) | code;

	if (connect->state == TSYN_SENT || connect->state == TSYN_RECV)
	{
		connect->net_error = E_CONNECTFAIL;
		close_self(connect, connect->net_error);
	} else
	{
		switch (type)
		{
		case ICMP_UNREACH:
			connect->net_error = E_UNREACHABLE;
			break;
		case ICMP_SOURCEQUENCH:
			connect->net_error = E_CNTIMEOUT;
			break;
		case ICMP_TIMXCEED:
			connect->net_error = E_TTLEXCEED;
			break;
		}
	}

	ICMP_discard(dgram);
	return TRUE;
}


void send_sync(CONNEC *connec)
{
	ini_sequ += 250052L;
	ini_sequ_next = (TIMER_now() << 8) + ini_sequ;
	connec->send.ini_sequ = ini_sequ_next;
	connec->rtrp.sequ = connec->send.lwup_ack = connec->send.unack = connec->send.ini_sequ;
	connec->send.ptr = connec->send.next = connec->send.ini_sequ;
	connec->send.count++;
	connec->flags |= FORCE;
}


void process_sync(CONNEC *connec, IP_DGRAM *dgram)
{
	uint16 max_mss;

	connec->flags |= FORCE;

	if (PRECMASK(dgram->hdr.tos) > PRECMASK(connec->tos))
		connec->tos = dgram->hdr.tos;
	else
		connec->tos = (dgram->hdr.tos & 0x1f) | PRECMASK(connec->tos);

	connec->send.lwup_seq = ((TCP_HDR *) dgram->pkt_data)->sequence;
	connec->send.window = ((TCP_HDR *) dgram->pkt_data)->window;
	connec->recve.next = ((TCP_HDR *) dgram->pkt_data)->sequence;

	process_options(connec, dgram);

	max_mss = connec->mtu - sizeof(IP_HDR) - sizeof(TCP_HDR);
	if (connec->mss > max_mss)
		connec->mss = max_mss;
	connec->o140 = connec->mss * 2;
	connec->o142 = 0xffff;
}


void process_options(CONNEC *connec, IP_DGRAM *dgram)
{
	uint8 *work;
	uint8 *limit;
	uint16 new_mss;

	work = (uint8 *) ((TCP_HDR *) dgram->pkt_data + 1);
	limit = (uint8 *) dgram->pkt_data + ((TCP_HDR *) dgram->pkt_data)->offset * 4;

	while (limit > work)
	{
		switch (*work)
		{
		case 0:
			work = limit;
			break;
		case 1:
			work++;
			break;
		case 2:
			new_mss = ((uint16) work[2] << 8) | ((uint16) work[3]);
			connec->mss = new_mss < connec->mss ? new_mss : connec->mss;
			work += work[1];
			break;
		default:
			work += work[1];
			break;
		}
	}
}


void send_reset(IP_DGRAM *dgram)
{
	TCP_HDR *hdr;
	uint16 ports;

	if ((hdr = (TCP_HDR *) dgram->pkt_data)->reset)
		return;

	my_conf.resets++;

	ports = hdr->src_port;
	hdr->src_port = hdr->dest_port;
	hdr->dest_port = ports;

	if (hdr->ack)
	{
		hdr->sequence = hdr->acknowledge;
		hdr->acknowledge = 0;
		hdr->ack = hdr->sync = hdr->push = hdr->urgent = FALSE;
	} else
	{
		hdr->ack = TRUE;
		hdr->acknowledge = hdr->sequence + dgram->pkt_length - hdr->offset * 4;
		hdr->sequence = 0;
		if (hdr->sync)
			hdr->acknowledge++;
		if (hdr->fin)
			hdr->acknowledge++;
		hdr->sync = hdr->push = hdr->urgent = FALSE;
	}
	hdr->reset = TRUE;

	hdr->window = hdr->chksum = hdr->urg_ptr = 0;
	hdr->offset = 5;

	hdr->chksum = check_sum(dgram->hdr.ip_dest, dgram->hdr.ip_src, hdr, sizeof(TCP_HDR));

	IP_send(dgram->hdr.ip_dest, dgram->hdr.ip_src, dgram->hdr.tos, FALSE,
			my_conf.def_ttl, P_TCP, tcp_id++, (uint8 *) hdr, sizeof(TCP_HDR), NULL, 0);

	dgram->pkt_data = NULL;
}


void close_self(CONNEC *connec, int16 reason)
{
	RESEQU *work;
	RESEQU *temp;

	/*
	 * BUG: calling this directly does not work;
	 * it will only set the mask to be restored
	 * when returning from protect_exec, but does not
	 * generate a priviledge violation when called from user mode.
	 * As a result, it will corrupt the stack.
	 */
	/* uint16 save_sr = cli(NULL); */

	connec->rtrn.mode = FALSE;
	connec->rtrp.mode = FALSE;
	connec->reason = reason;

	work = connec->recve.reseq;
	connec->recve.reseq = NULL;
	connec->state = TCLOSED;

	if (connec->result != NULL)
	{
		if (connec->send.count == 0 && reason == E_NORMAL)
			*connec->result = E_NORMAL;
		else
			*connec->result = reason == E_NORMAL ? E_EOF : reason;
	}
	/* set_sr((void *)(int32)save_sr); */

	for (; work; work = temp)
	{
		temp = work->next;
		KRfree(work->hdr);
		KRfree(work);
	}
}


int16 receive(CONNEC *connec, uint8 *buffer, int16 *length, int16 getchar)
{
	NDB *ndb;

	if (*length >= 0)
	{
		if (*length > connec->recve.count)
			*length = connec->recve.count;
		if (getchar)
		{
			pull_char(connec->recve.queue, (char *)buffer, *length);
			return connec->recve.count;
		}
		pull_up(&connec->recve.queue, (char *)buffer, *length);
	} else
	{
		if ((ndb = connec->recve.queue) == NULL)
		{
			*length = -1;
		} else
		{
			*length = ndb->len;
			*(NDB **) buffer = ndb;
			connec->recve.queue = ndb->next;
		}
	}

	if (*length > 0)
	{
		connec->recve.count -= *length;
		connec->recve.window += *length;

		if (connec->recve.window == *length ||
			(connec->recve.lst_win < connec->mss && connec->recve.window >= connec->mss) ||
			(int32)(connec->recve.next + connec->recve.window - (connec->recve.lst_next + connec->recve.lst_win)) >= connec->mss)
		{
			connec->flags |= FORCE;
			do_output(connec);
		}
	}

	return connec->recve.count;
}


int16 categorize(CONNEC *connec)
{
	switch (connec->state)
	{
	case TLISTEN:
		return C_LISTEN;
	case TSYN_SENT:
	case TSYN_RECV:
	case TESTABLISH:
		return C_READY;
	case TFIN_WAIT1:
	case TFIN_WAIT2:
		return C_FIN;
	case TCLOSE_WAIT:
		return connec->recve.count ? C_READY : C_END;
	case TCLOSED:
	case TCLOSING:
	case TLAST_ACK:
	case TTIME_WAIT:
		return connec->recve.count ? C_FIN : C_CLSD;
	}

	return C_DEFAULT;
}
