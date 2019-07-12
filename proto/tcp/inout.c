/*********************************************************************/
/*                                                                   */
/*     High Level Protokoll : TCP                                    */
/*                                                                   */
/*                                                                   */
/*      Version 1.0                     from 19. February 1997       */
/*                                                                   */
/*      Modul fÅr Input / Output Funktionen                          */
/*                                                                   */
/*********************************************************************/


#include <tos.h>
#include <stdio.h>
#include <string.h>

#include "transprt.h"
#include "layer.h"

#include "tcp.h"


uint16 tcp_id = 0;



void update_wind(CONNEC *connec, TCP_HDR *tcph)
{
	uint32 acked;

	if ((int32) tcph->acknowledge - (int32) connec->send.next > 0)
	{
		connec->flags |= FORCE;
		return;
	}

	if ((int32) tcph->sequence - (int32) connec->send.lwup_seq > 0 || ((tcph->sequence == connec->send.lwup_seq) &&
																	   ((int32) tcph->acknowledge -
																		(int32) connec->send.lwup_ack >= 0)))
	{
		if (connec->send.window == 0 && tcph->window != 0)
			connec->send.ptr = connec->send.unack;
		connec->send.window = tcph->window;
		connec->send.lwup_seq = tcph->sequence;
		connec->send.lwup_ack = tcph->acknowledge;
	}

	if ((int32) tcph->acknowledge - (int32) connec->send.unack <= 0)
		return;

	acked = (int32) tcph->acknowledge - (int32) connec->send.unack;

	if (connec->send.ini_sequ == connec->send.unack &&
		(connec->state == TSYN_SENT || connec->state == TSYN_RECV))
	{
		connec->send.count--;
		acked--;
	}

	if ((connec->flags & FLAG40) &&
		(uint32)connec->send.count == acked &&
		acked != 0)
	{
		acked--;
		connec->send.count--;
	}

	pull_up(&connec->send.queue, NULL, acked);

	connec->send.count -= acked;
	connec->send.total -= acked;
	connec->send.unack = tcph->acknowledge;

	if (connec->send.unack != connec->send.next)
	{
		connec->rtrn.mode = TRUE;
		connec->rtrn.start = TIMER_now();
		connec->rtrn.timeout = 2L * connec->rtrp.smooth;
		connec->rtrn.backoff = 0;
	} else
	{
		connec->rtrn.mode = FALSE;
	}

	if ((int32) connec->send.ptr - (int32) connec->send.unack < 0)
		connec->send.ptr = connec->send.unack;

	connec->flags &= ~RETRAN;
}


uint16 pull_up(NDB **queue, char *buffer, uint16 length)
{
	NDB *temp;
	uint16 avail;
	uint16 accu = 0;

	while (length > 0 && *queue != NULL)
	{
		avail = (*queue)->len < length ? (*queue)->len : length;
		if (buffer)
		{
			memcpy(buffer, (*queue)->ndata, avail);
			buffer += avail;
		}
		(*queue)->len -= avail;
		(*queue)->ndata += avail;
		if ((*queue)->len == 0)
		{
			KRfree((*queue)->ptr);
			temp = *queue;
			*queue = (*queue)->next;
			KRfree(temp);
		}
		accu += avail;
		length -= avail;
	}

	return accu;
}


int16 trim_segm(CONNEC *connec, IP_DGRAM *dgram, RESEQU **block, int16 make_resequ)
{
	TCP_HDR *hdr;
	uint32 wind_beg, wind_end;
	int32 dupes;
	int32 excess;
	int16 dat_len;
	int16 seq_len;
	uint8 *data;

	if (make_resequ)
	{
		if ((*block = KRmalloc(sizeof(RESEQU))) == NULL)
			return FALSE;
		(*block)->tos = dgram->hdr.tos;
		(*block)->hdr = hdr = (TCP_HDR *) dgram->pkt_data;
		(*block)->data = (uint8 *) dgram->pkt_data + hdr->offset * 4;
		(*block)->data_len = dgram->pkt_length - hdr->offset * 4;
	}

	hdr = (*block)->hdr;
	data = (*block)->data;
	dat_len = (*block)->data_len;

	seq_len = dat_len;

	if (hdr->sync)
		seq_len++;
	if (hdr->fin)
		seq_len++;

	wind_beg = connec->recve.next;
	wind_end = wind_beg + connec->recve.window - 1;

	if (connec->recve.window == 0)
	{
		if (hdr->sequence != connec->recve.next)
			return FALSE;
		if (seq_len != 0)
			return FALSE;
		return TRUE;
	}

	if (sequ_within_range(hdr->sequence, wind_beg, wind_end))
	{
		;
	} else
	{
		if (sequ_within_range(hdr->sequence + seq_len - 1, wind_beg, wind_end))
			;
		else if (sequ_within_range(wind_beg, hdr->sequence, hdr->sequence + seq_len - 1))
			;
		else
			return FALSE;
	}

	if ((dupes = connec->recve.next - hdr->sequence) > 0)
	{
		if (hdr->sync)
		{
			dupes--;
			hdr->sync = FALSE;
			hdr->sequence++;
		}
		data += dupes;
		dat_len -= dupes;
		hdr->sequence += dupes;
	}

	excess = (hdr->sequence + dat_len) - (connec->recve.next + connec->recve.window);

	if (excess > 0)
	{
		if (hdr->fin)
		{
			excess -= 1;
			hdr->fin = FALSE;
		}
		dat_len -= excess;
	}

	(*block)->data = data;
	(*block)->data_len = dat_len;

	return TRUE;
}


void add_resequ(CONNEC *connec, RESEQU *block)
{
	RESEQU *work;

	if (connec->recve.reseq != NULL)
	{
		if ((int32) block->hdr->sequence - (int32) connec->recve.reseq->hdr->sequence >= 0)
		{
			for (work = connec->recve.reseq; work->next; work = work->next)
			{
				if ((int32) block->hdr->sequence - (int32) work->next->hdr->sequence < 0)
					break;
			}
			block->next = work->next;
			work->next = block;
			return;
		}
	}

	block->next = connec->recve.reseq;
	connec->recve.reseq = block;
}


static uint8 *prep_segment(CONNEC *connec, TCP_HDR *hdr, uint16 *length, uint16 offset, uint16 size)
{
	NDB *work;
	uint16 *walk;
	uint16 chunk;
	uint8 *mem;
	uint8 *ptr;

	*length = sizeof(TCP_HDR) + (hdr->sync ? 4 : 0) + size;

	if ((mem = KRmalloc(*length)) == NULL)
		return NULL;

	hdr->src_port = connec->local_port;
	hdr->dest_port = connec->remote_port;
	hdr->offset = hdr->sync ? 6 : 5;
	hdr->resvd = 0;
	hdr->window = connec->recve.window;
	hdr->urg_ptr = 0;

	if (hdr->sync)
	{
		walk = (uint16 *) (mem + sizeof(TCP_HDR));
		*walk++ = 0x0204;
		*walk++ = connec->mss;
		if (offset != 0)
			--offset;
	}
	memcpy(mem, hdr, sizeof(TCP_HDR));

	if (size > 0)
	{
		ptr = mem + sizeof(TCP_HDR) + (hdr->sync ? 4 : 0);

		for (work = connec->send.queue; work != NULL; work = work->next)
		{
			if (work->len > offset)
				break;
			offset -= work->len;
		}

		for (; work != NULL && size > 0; work = work->next, offset = 0)
		{
			chunk = (work->len - offset < size) ? work->len - offset : size;
			size -= chunk;
			memcpy(ptr, work->ndata + offset, chunk);
			ptr += chunk;
		}
	}

	((TCP_HDR *) mem)->chksum = 0;

	((TCP_HDR *) mem)->chksum =
		check_sum(connec->local_IP_address, connec->remote_IP_address, (TCP_HDR *) mem, *length);

	connec->recve.lst_win = connec->recve.window;

	return mem;
}


void do_output(CONNEC *connec)
{
	TCP_HDR hdr;
	int16 value;
	uint16 length;
	uint16 sent;
	uint16 usable_win;
	uint16 size;
	int16 raw_size;
	uint8 *block;
	uint16 mss;
	uint16 force;
	uint32 elapsed;

	mss = connec->mss;

	if (connec->state == TCLOSED || connec->state == TLISTEN)
		return;

	for (;;)
	{
		force = (connec->flags & FORCE) == 0;

		sent = connec->send.ptr - connec->send.unack;
		if (sent >= connec->send.window)
		{
			if (sent != 0)
				break;
			usable_win = 1;
		} else
		{
			usable_win = connec->send.window - sent;
		}
		if (usable_win > connec->o140)
			usable_win = connec->o140;

		size = connec->send.count - sent;
		if (force ||
			((elapsed = TIMER_elapsed(connec->send.start)) < (connec->rtrp.smooth >> 1) &&
			  elapsed < 500 &&
			  (connec->flags & 0) != 0)) /* WTF? */
		{
			if (size == 0 ||
				(sent != 0 && (size < mss || usable_win < mss)))
			{
				if (force == 0)
				{
					connec->flags |= FLAG20;
				}
				break;
			}
		}
		if (size > mss)
			size = mss;
		if (size > usable_win)
			size = usable_win;
		connec->flags &= ~FORCE;
		raw_size = size;

		hdr.urgent = hdr.push = hdr.reset = hdr.sync = hdr.fin = FALSE;
		hdr.ack = TRUE;
		hdr.sequence = connec->send.ptr;
		hdr.acknowledge = connec->recve.next;

		if (connec->send.ptr == connec->send.ini_sequ)
		{
			if (connec->state == TSYN_SENT || connec->state == TSYN_RECV)
			{
				hdr.sync = TRUE;
				if (connec->state == TSYN_SENT)
				{
					hdr.ack = FALSE;
					connec->rtrn.start = TIMER_now();
					connec->flags |= FLAG20;
				}
				if (--raw_size < 0)
					raw_size = 0;
			}
		}

		if (hdr.ack)
		{
			connec->flags &= ~FLAG20;
			connec->send.start = TIMER_now();
		}

		if ((connec->flags & FLAG40) &&
			connec->send.count - sent == size &&
			connec->send.count != 0)
		{
			hdr.fin = TRUE;
			if (--raw_size < 0)
				raw_size = 0;
			if (size == 0)
				hdr.sequence = connec->send.unack - 1;
		}

		if (raw_size != 0 && sent + size == connec->send.count)
			hdr.push = TRUE;

		connec->send.ptr += size;

		if ((int32) connec->send.ptr - (int32) connec->send.next > 0)
			connec->send.next = connec->send.ptr;

		if (size != 0)
		{
			if (!connec->rtrp.mode)
			{
				connec->rtrp.start = TIMER_now();
				connec->rtrp.mode = TRUE;
				connec->rtrp.sequ = connec->send.ptr;
			}
			if (!connec->rtrn.mode)
				connec->rtrn.mode = TRUE;
		} else
		{
			if (sent == 0)
				connec->rtrn.mode = FALSE;
		}

		if ((block = prep_segment(connec, &hdr, &length, sent, raw_size)) != NULL)
		{
			value = IP_send(connec->local_IP_address, connec->remote_IP_address, connec->tos,
							FALSE, connec->ttl, P_TCP, tcp_id++, block, length, NULL, 0);
			if (value != E_NORMAL)
				KRfree(block);
			if (value == E_UNREACHABLE)
				connec->net_error = E_UNREACHABLE;
		}
	}
}
