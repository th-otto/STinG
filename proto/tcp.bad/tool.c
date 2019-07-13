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


static uint32 ini_sequ;
static CONNEC *global;
static volatile signed char global_sema = 0;



static int16 discard(CONNEC *connec)
{
	NDB *ndb;

	if ((ndb = connec->recve.queue) == NULL)
		return TRUE;

	connec->recve.queue = ndb->next;
	connec->recve.count -= ndb->len;
	connec->recve.window += ndb->len;

	connec->flags |= FORCE;
	do_output(connec);

	KRfree(ndb->ptr);
	KRfree(ndb);

	return FALSE;
}


static void flush_queue(NDB **queue)
{
	NDB *walk;
	NDB *temp;

	if (*queue == NULL)
		return;

	for (walk = *queue; walk; walk = temp)
	{
		KRfree(walk->ptr);
		temp = walk->next;
		KRfree(walk);
	}
	*queue = NULL;
}


static void destroy_conn(CONNEC *connec)
{
	CONNEC *work;
	CONNEC **previous;
	IP_DGRAM *ip_walk;
	IP_DGRAM *ip_next;
	RESEQU *rsq_walk;
	RESEQU *rsq_next;

	Supexec(dis_intrpt);

	for (work = *(previous = &root_list); work; work = *(previous = &work->next))
	{
		if (work == connec)
			break;
	}

	if (work)
		*previous = work->next;

	Supexec(en_intrpt);

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

	KRfree(connec->info);
	KRfree(connec);
}


static void abort_conn(CONNEC *connec)
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


static int16 timer_work(CONNEC *connec)
{
	IP_DGRAM *walk;
	IP_DGRAM *next;
	uint16 backoff;
	uint16 smooth;

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
				connec->rtrn.backoff++;
				backoff = (int16) connec->rtrn.backoff * connec->rtrn.backoff;
				smooth = (connec->rtrp.smooth > 1) ? connec->rtrp.smooth : 1;
				connec->send.ptr = connec->send.unack;
				connec->flags |= RETRAN;
				connec->rtrn.start = TIMER_now();
				connec->rtrn.timeout = 2L * backoff * smooth;
				do_output(connec);
			} else
			{
				close_self(connec, E_NORMAL);
			}
		}
	}

	if (connec->flags & CLOSING)
	{
		if (discard(connec))
		{
			if (connec->state == TCLOSED && (connec->flags & BLOCK) == 0)
			{
				destroy_conn(connec);
				return FALSE;
			}
		}
		if ((uint32)TIMER_elapsed(connec->close.start) > connec->close.timeout)
		{
			abort_conn(connec);
			close_self(connec, E_CNTIMEOUT);
		}
	}

	connec->last_work = TIMER_now();
	rel_flag(&connec->sema);

	return TRUE;
}


int16 cdecl timer_function(IP_DGRAM *dgram)
{
	CONNEC *connect, *next;

	(void)dgram;
	for (connect = root_list; connect; connect = next)
	{
		next = connect->next;
		timer_work(connect);
	}
	return 0;
}


static long poll_doit(void)
{
	CONNEC *connec;

	connec = global;

	rel_flag(&global_sema);
	return timer_work(connec);
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

	wait_flag(&global_sema);
	global = connec;

	Supexec(poll_doit);

	return E_NORMAL;
}


int16 cdecl do_ICMP(IP_DGRAM *dgram)
{
	IP_HDR *ip;
	TCP_HDR *tcp;
	CONNEC *connect;
	uint8 type;
	uint8 code;

	if ((my_conf.generic.flags & 0x10000ul) == 0)
		return FALSE;

	type = *(uint8 *) dgram->pkt_data;
	code = *((uint8 *) dgram->pkt_data + 1);

	if (type != 3 && type != 4 && type != 11)
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

	if (!sequ_within(tcp->sequence, connect->send.unack, connect->send.next))
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
		case 3:
			connect->net_error = E_UNREACHABLE;
			break;
		case 4:
			connect->net_error = E_CNTIMEOUT;
			break;
		case 11:
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
	connec->send.ini_sequ = ini_sequ;
	connec->rtrp.sequ = connec->send.lwup_ack = connec->send.unack = connec->send.ini_sequ;
	connec->send.ptr = connec->send.next = connec->send.ini_sequ;
	connec->send.count++;
	connec->flags |= FORCE;
}


void process_sync(CONNEC *connec, IP_DGRAM *dgram)
{
	uint16 max_mss;

	connec->flags |= FORCE;

	if (PREC(dgram->hdr.tos) > PREC(connec->tos))
		connec->tos = dgram->hdr.tos;

	connec->send.lwup_seq = ((TCP_HDR *) dgram->pkt_data)->sequence;
	connec->send.window = ((TCP_HDR *) dgram->pkt_data)->window;
	connec->recve.next = ((TCP_HDR *) dgram->pkt_data)->sequence + 1;

	process_options(connec, dgram);

	max_mss = connec->mtu - sizeof(IP_HDR) - sizeof(TCP_HDR);
	if (connec->mss > max_mss)
		connec->mss = max_mss;
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
		case 1:
			work++;
			break;
		case 2:
			new_mss = ((uint16) work[2] << 8) | ((uint16) work[3]);
			connec->mss = (new_mss < connec->mss) ? new_mss : connec->mss;
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

	hdr->offset = hdr->window = hdr->chksum = hdr->urg_ptr = 0;

	hdr->chksum = check_sum(dgram->hdr.ip_dest, dgram->hdr.ip_src, hdr, sizeof(TCP_HDR));

	IP_send(dgram->hdr.ip_dest, dgram->hdr.ip_src, dgram->hdr.tos, FALSE,
			my_conf.def_ttl, P_TCP, tcp_id++, (uint8 *) hdr, sizeof(TCP_HDR), NULL, 0);

	dgram->pkt_data = NULL;
}


int16 sequ_within(uint32 actual, uint32 low, uint32 high)
{
	if (0 <= (int32) high - (int32) low)
	{
		if (0 <= (int32) actual - (int32) low && 0 <= (int32) high - (int32) actual)
			return TRUE;
	} else
	{
		if (0 >= (int32) actual - (int32) low && 0 >= (int32) high - (int32) actual)
			return TRUE;
	}
	return FALSE;
}


void close_self(CONNEC *connec, int16 reason)
{
	RESEQU *work;
	RESEQU *temp;

	connec->rtrn.mode = FALSE;
	connec->rtrp.mode = FALSE;
	connec->reason = reason;

	for (work = connec->recve.reseq; work; work = temp)
	{
		KRfree(work->hdr);
		temp = work->next;
		KRfree(work);
	}
	connec->recve.reseq = NULL;

	connec->state = TCLOSED;

	if (connec->result)
	{
		if (connec->send.count == 0 && reason == E_NORMAL)
			*connec->result = E_NORMAL;
		else
			*connec->result = (reason == E_NORMAL) ? E_EOF : reason;
	}
	connec->flags |= DISCARD;
}


int16 halfdup_close(CONNEC *connec)
{
	wait_flag(&connec->sema);

	connec->send.count++;
	connec->state = (connec->state == TCLOSE_WAIT) ? TLAST_ACK : TFIN_WAIT1;
	do_output(connec);

	rel_flag(&connec->sema);

	return E_NODATA;
}


int16 fuldup_close(CONNEC *connec, int32 time_out, int16 block_flg)
{
	int16 result;

	wait_flag(&connec->sema);

	connec->send.count++;
	connec->state = (connec->state == TCLOSE_WAIT) ? TLAST_ACK : TFIN_WAIT1;
	do_output(connec);

	connec->close.start = TIMER_now();
	connec->close.timeout = time_out;
	connec->flags |= CLOSING | (block_flg ? BLOCK : 0);

	rel_flag(&connec->sema);

	if (block_flg)
	{
		while (connec->state != TCLOSED)
			_appl_yield();
		result = connec->reason;
		destroy_conn(connec);
	} else
	{
		result = E_NORMAL;
	}

	return result;
}


int16 receive(CONNEC *connec, uint8 *buffer, int16 *length, int16 getchar)
{
	NDB *ndb;

	wait_flag(&connec->sema);

	if (*length >= 0)
	{
		if (*length > connec->recve.count)
			*length = connec->recve.count;
		pull_up(&connec->recve.queue, (char *)buffer, *length);
	} else
	{
		if ((ndb = connec->recve.queue) == NULL)
			*length = -1;
		else
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

		if ((connec->recve.lst_win < connec->mss && connec->recve.window >= connec->mss) ||
			connec->recve.window == *length || !getchar)
		{
			connec->flags |= FORCE;
			do_output(connec);
		}
	}

	rel_flag(&connec->sema);

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
