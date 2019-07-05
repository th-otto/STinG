/*********************************************************************/
/*                                                                   */
/*     High Level Protokoll : TCP                                    */
/*                                                                   */
/*                                                                   */
/*      Version 1.0                         from 5. March 1997       */
/*                                                                   */
/*      Modul zur Installation und API                               */
/*                                                                   */
/*********************************************************************/


#include <tos.h>
#include <stdio.h>
#include <string.h>

#include "transprt.h"
#include "layer.h"

#include "tcp.h"


#define  M_YEAR    1999
#define  M_MONTH   1
#define  M_DAY     2
#define  M_VERSION "01.15"

#ifdef __GNUC__
# define _BasPag _base
#endif


TPL *tpl;
STX *stx;

CONNEC *root_list;

static uint16 last_port;
static char const fault[] = "TCP.STX : STinG extension module. Only to be started by STinG !\r\n";

TCP_CONF my_conf = {
	{
		"TCP",
		M_VERSION,
		0x10400L,
		((M_YEAR - 1980) << 9) | (M_MONTH << 5) | M_DAY,
		"Peter Rottengatter",
		0,
		NULL,
		NULL
	},
	2000, 2000, 50, 64, 200, 0, 0, 0
};



static int16 cdecl my_CNkick(void *connec)
{
	CONNEC *conn = connec;
	int16 error;
	uint16 smooth;

	if ((error = poll_receive(connec)) < 0)
		return error;

	wait_flag(&conn->sema);

	smooth = conn->rtrp.smooth > 1 ? conn->rtrp.smooth : 1;

	conn->rtrn.mode = FALSE;
	conn->rtrn.backoff = 0;
	conn->rtrn.start = TIMER_now();
	conn->rtrn.timeout = 2 * smooth;

	conn->flags |= FORCE;

	do_output(conn);
	rel_flag(&conn->sema);

	return E_NORMAL;
}


static int16 cdecl my_CNbyte_count(void *connec)
{
	CONNEC *conn = connec;
	int16 error;

	if ((error = poll_receive(connec)) < 0)
		return error;

	switch (categorize(conn))
	{
	case C_END:
	case C_CLSD:
		return E_EOF;
	case C_LISTEN:
		return E_LISTEN;
	case C_DEFAULT:
		return E_NODATA;
	}

	return conn->recve.count;
}


static int16 cdecl my_CNget_char(void *connec)
{
	CONNEC *conn = connec;
	int16 error;
	int16 length = 1;
	uint8 character;

	if ((error = poll_receive(connec)) < 0)
		return error;

	switch (categorize(conn))
	{
	case C_END:
	case C_CLSD:
		return E_EOF;
	case C_LISTEN:
		return E_LISTEN;
	case C_DEFAULT:
		return E_NODATA;
	}

	receive(conn, &character, &length, TRUE);

	return length ? (int16) character : E_NODATA;
}


static NDB *cdecl my_CNget_NDB(void *connec)
{
	NDB *ndb;
	int16 flag = categorize(connec);

	if (poll_receive(connec) < 0)
		return NULL;

	if (flag != C_READY && flag != C_FIN)
		return NULL;

	flag = -1;
	receive(connec, (uint8 *) & ndb, &flag, FALSE);

	if (flag < 0)
		return NULL;
	ndb->next = NULL;
	return ndb;
}


static int16 cdecl my_CNget_block(void *connec, void *buffer, int16 length)
{
	CONNEC *conn = connec;
	int16 error;

	if ((error = poll_receive(connec)) < 0)
		return error;

	switch (categorize(conn))
	{
	case C_END:
	case C_CLSD:
		return E_EOF;
	case C_LISTEN:
		return E_LISTEN;
	case C_DEFAULT:
		return E_NODATA;
	}

	if (length <= 0)
		return 0;

	if (length > conn->recve.count)
		return E_NODATA;

	receive(conn, buffer, &length, FALSE);

	return length;
}


static CIB *cdecl my_CNgetinfo(void *connec)
{
	CONNEC *conn = connec;
	CIB *cib;

	if (conn->info == NULL)
	{
		if ((conn->info = (CIB *) KRmalloc(sizeof(CIB))) == NULL)
			return NULL;
		conn->info->status = 0;
	}

	cib = conn->info;

	cib->protocol = P_TCP;
	cib->address.lport = conn->local_port;
	cib->address.rport = conn->remote_port;
	cib->address.rhost = conn->remote_IP_address;
	cib->address.lhost = conn->local_IP_address;

	return cib;
}


static int16 cdecl my_CNgets(void *connec, char *buffer, int16 length, char delimiter)
{
	CONNEC *conn = connec;
	NDB *walk;
	int16 error;
	int16 count;
	int16 amount;
	char *search;

	if ((error = poll_receive(connec)) < 0)
		return error;

	switch (categorize(conn))
	{
	case C_END:
	case C_CLSD:
		return E_EOF;
	case C_LISTEN:
		return E_LISTEN;
	case C_DEFAULT:
		return E_NODATA;
	}

	if (conn->recve.count == 0)
		return E_NODATA;

	if (length <= 1)
		return E_BIGBUF;

	wait_flag(&conn->sema);

	for (walk = conn->recve.queue, amount = 0; walk != NULL; walk = walk->next)
	{
		search = walk->ndata;
		for (count = 0; count < walk->len && amount < length; count++, amount++)
		{
			if (*search++ == delimiter)
			{
				amount++;
				rel_flag(&conn->sema);
				receive(conn, (uint8 *)buffer, &amount, FALSE);
				buffer[--amount] = '\0';
				return amount;
			}
		}
		if (amount == length)
		{
			rel_flag(&conn->sema);
			return E_BIGBUF;
		}
	}

	rel_flag(&conn->sema);

	return E_NODATA;
}


static CN_FUNCS cn_vectors = {
	my_CNkick,
	my_CNbyte_count,
	my_CNget_char,
	my_CNget_NDB,
	my_CNget_block,
	my_CNgetinfo,
	my_CNgets
};


static int16 next_port(void)
{
	CONNEC *connect;

	Supexec(dis_intrpt);

	for (;;)
	{
		last_port++;

		if (last_port > 32765 || last_port < (my_conf.generic.flags & 0xfffful))
			last_port = my_conf.generic.flags & 0xfffful;

		for (connect = root_list; connect; connect = connect->next)
			if (connect->local_port == last_port)
				break;
		if (connect)
			continue;

		Supexec(en_intrpt);
		return last_port;
	}
}


static int16 cdecl my_TCP_open(uint32 rem_host, uint16 rem_port, uint16 tos, uint16 buff_size)
{
	CAB *cab;
	CONNEC *connect;
	uint32 lcl_host = 0;
	uint32 aux_ip;
	uint32 error;
	uint16 act_pass;
	uint16 lport;
	uint16 rport;
	uint16 mtu;
	uint16 max_mss = 32768u;
	int16 window;
	int16 ttl;
	int16 handle;

	if (rem_host == 0L && (rem_port == TCP_ACTIVE || rem_port == TCP_PASSIVE))
		rem_port = next_port();

	if (rem_port != TCP_ACTIVE && rem_port != TCP_PASSIVE)
	{
		if (rem_host)
		{
			act_pass = TCP_ACTIVE;
			lport = next_port();
			rport = rem_port;
		} else
		{
			act_pass = TCP_PASSIVE;
			lport = rem_port;
			rport = 0;
		}
	} else
	{
		cab = (CAB *) rem_host;
		act_pass = rem_port;
		rem_host = cab->rhost;
		rport = cab->rport;
		lcl_host = cab->lhost;
		lport = cab->lport ? cab->lport : next_port();
	}

	if (rem_host != 0L)
	{
		if (PRTCL_get_parameters(rem_host, &aux_ip, &ttl, &mtu) != E_NORMAL)
			return E_UNREACHABLE;
		lcl_host = lcl_host ? lcl_host : aux_ip;
		max_mss = mtu - sizeof(IP_HDR) - sizeof(TCP_HDR);
	} else
	{
		if (act_pass == TCP_ACTIVE)
			return E_PARAMETER;
	}

	ttl = my_conf.def_ttl;
	window = buff_size > 0 ? buff_size : my_conf.rcv_window;

	if ((connect = (CONNEC *) KRmalloc(sizeof(CONNEC))) == NULL)
		return E_NOMEM;

	if ((handle = PRTCL_request(connect, &cn_vectors)) == -1)
	{
		KRfree(connect);
		return E_NOMEM;
	}

	connect->remote_IP_address = rem_host;
	connect->remote_port = rport;
	connect->local_IP_address = lcl_host;
	connect->local_port = lport;
	connect->flags = 0;
	connect->mss = my_conf.mss < max_mss ? my_conf.mss : max_mss;
	connect->mtu = mtu;
	connect->tos = tos;
	connect->ttl = ttl;
	connect->info = NULL;
	connect->reason = 0;
	connect->net_error = 0;

	connect->send.window = window;
	connect->send.bufflen = window;
	connect->send.total = 0;
	connect->send.count = 0;
	connect->send.queue = NULL;
	connect->recve.window = my_conf.rcv_window;
	connect->recve.reseq = NULL;
	connect->recve.count = 0;
	connect->recve.queue = NULL;

	connect->rtrn.start = TIMER_now();
	connect->rtrn.timeout = 2 * my_conf.def_rtt;
	connect->rtrn.mode = FALSE;
	connect->rtrn.backoff = 0;
	connect->rtrp.mode = FALSE;
	connect->rtrp.smooth = my_conf.def_rtt;

	connect->sema = -1;
	connect->pending = NULL;
	connect->result = NULL;

	Supexec(dis_intrpt);
	connect->next = root_list;
	root_list = connect;
	Supexec(en_intrpt);

	if (act_pass == TCP_ACTIVE)
	{
		send_sync(connect);
		connect->state = TSYN_SENT;
		my_conf.con_out++;
		do_output(connect);
	} else
	{
		connect->state = TLISTEN;
	}

	rel_flag(&connect->sema);

	if ((error = connect->net_error) == 0)
		return handle;

	Supexec(dis_intrpt);
	root_list = connect->next;
	Supexec(en_intrpt);

	KRfree(connect);
	PRTCL_release(handle);

	return error;
}


static int16 cdecl my_TCP_close(int16 connec, int16 mode, int16 *result)
{
	CONNEC *conn;
	int16 retval = E_NOROUTINE;

	if ((conn = PRTCL_lookup(connec, &cn_vectors)) == NULL)
		return E_BADHANDLE;

	if (mode >= 0)
	{
		if (conn->result != NULL)
		{
			conn->result = NULL;
			return E_NORMAL;
		}
		if (mode >= 1000)
			return E_PARAMETER;
	} else
	{
		conn->result = result;
	}

	switch (conn->state)
	{
	case TLISTEN:
	case TSYN_SENT:
		wait_flag(&conn->sema);
		close_self(conn, E_NORMAL);
		rel_flag(&conn->sema);
		retval = E_NORMAL;
		break;
	case TSYN_RECV:
	case TESTABLISH:
	case TCLOSE_WAIT:
		if (mode >= 0)
		{
			retval = fuldup_close(conn, 1000L * mode);
			PRTCL_release(connec);
		} else
			retval = halfdup_close(conn);
		break;
	case TFIN_WAIT1:
	case TFIN_WAIT2:
	case TCLOSING:
	case TLAST_ACK:
	case TTIME_WAIT:
	case TCLOSED:
		retval = E_BADCLOSE;
		break;
	}

	if (mode < 0 && result != NULL)
		*result = retval;

	return retval;
}


static int16 cdecl my_TCP_send(int16 connec, const void *buffer, int16 length)
{
	CONNEC *conn;
	NDB *ndb;
	NDB *walk;
	uint8 *data;
	int16 error;

	if ((conn = PRTCL_lookup(connec, &cn_vectors)) == NULL)
		return E_BADHANDLE;

	error = conn->net_error;

	if (error < 0)
	{
		conn->net_error = 0;
		return error;
	}

	switch (categorize(conn))
	{
	case C_FIN:
	case C_CLSD:
		return E_EOF;
	case C_LISTEN:
		return E_LISTEN;
	case C_DEFAULT:
		return E_NODATA;
	}

	if (conn->send.bufflen - conn->send.total < length)
		return E_OBUFFULL;

	if ((ndb = (NDB *) KRmalloc(sizeof(NDB))) == NULL)
		return E_NOMEM;

	if ((data = (uint8 *) KRmalloc(length)) == NULL)
	{
		KRfree(ndb);
		return E_NOMEM;
	}

	ndb->ptr = ndb->ndata = (char *)data;
	ndb->len = length;
	ndb->next = NULL;
	memcpy(data, buffer, length);

	wait_flag(&conn->sema);

	if (conn->send.queue)
	{
		for (walk = conn->send.queue; walk->next; walk = walk->next) ;
		walk->next = ndb;
	} else
	{
		conn->send.queue = ndb;
	}

	conn->send.count += length;
	conn->send.total += length;
	do_output(conn);

	rel_flag(&conn->sema);

	return E_NORMAL;
}


static int16 cdecl my_TCP_wait_state(int16 connec, int16 state, int16 timeout)
{
	CONNEC *conn;
	int16 err;
	uint32 timer;
	int32 time_out;

	if ((conn = PRTCL_lookup(connec, &cn_vectors)) == NULL)
		return E_BADHANDLE;

	err = conn->net_error;

	if (err < 0)
	{
		conn->net_error = 0;
		return err;
	}

	timer = TIMER_now();
	time_out = 1000L * timeout;

	while (conn->state != state)
	{
		if (TIMER_elapsed(timer) >= time_out)
		{
			return E_CNTIMEOUT;
		}
		_appl_yield();
	}

	return E_NORMAL;
}


static int16 cdecl my_TCP_ack_wait(int16 connec, int16 timeout)
{
	CONNEC *conn;
	int16 err;
	uint32 timer;

	if ((conn = PRTCL_lookup(connec, &cn_vectors)) == NULL)
		return E_BADHANDLE;

	err = conn->net_error;

	if (err < 0)
	{
		conn->net_error = 0;
		return err;
	}

	timer = TIMER_now();

	while (conn->send.total > 0)
	{
		if (TIMER_elapsed(timer) >= timeout)
		{
			return E_CNTIMEOUT;
		}
		_appl_yield();
	}

	return E_NORMAL;
}


static int16 cdecl my_TCP_info(int16 connec, TCPIB *block)
{
	CONNEC *conn;

	if ((conn = PRTCL_lookup(connec, &cn_vectors)) == NULL)
		return E_BADHANDLE;

	if (block == NULL)
		return E_PARAMETER;

	block->state = conn->state;

	return E_NORMAL;
}


static long get_sting_cookie(void)
{
	long *work;

	work = *(long **) 0x5a0L;
	if (work == 0)
		return 0;
	for (; *work != 0L; work += 2)
		if (*work == STIK_COOKIE_MAGIC)
			return *++work;

	return 0;
}


static uint16 read_word(const char *string)
{
	uint16 result = 0;

	while (*string == ' ')
		string++;

	while ('0' <= *string && *string <= '9')
		result = result * 10 + (*string++ - '0');

	return result;
}


static int16 install(void)
{
	LAYER *layers;
	const char *config;

	if (!ICMP_handler(do_ICMP, HNDLR_SET))
		return FALSE;

	if (!IP_handler(P_TCP, TCP_handler, HNDLR_SET))
	{
		ICMP_handler(do_ICMP, HNDLR_REMOVE);
		return FALSE;
	}

	if (!TIMER_call(timer_function, HNDLR_SET))
	{
		ICMP_handler(do_ICMP, HNDLR_REMOVE);
		IP_handler(P_TCP, TCP_handler, HNDLR_REMOVE);
		return FALSE;
	}

	if (PRTCL_announce(P_TCP))
	{
		ICMP_handler(do_ICMP, HNDLR_REMOVE);
		IP_handler(P_TCP, TCP_handler, HNDLR_REMOVE);
		TIMER_call(timer_function, HNDLR_REMOVE);
		return FALSE;
	}

	my_conf.generic.basepage = _BasPag;

	query_chains(NULL, NULL, &layers);

	while (layers->next)
		layers = layers->next;

	layers->next = &my_conf.generic;

	tpl->TCP_open = my_TCP_open;
	tpl->TCP_close = my_TCP_close;
	tpl->TCP_send = my_TCP_send;
	tpl->TCP_wait_state = my_TCP_wait_state;
	tpl->TCP_ack_wait = my_TCP_ack_wait;
	tpl->TCP_info = my_TCP_info;

	config = getvstr("TCP_PORT");
	if (config[1])
	{
		my_conf.generic.flags &= 0xffff0000ul;
		my_conf.generic.flags |= read_word(config);
	}
	config = getvstr("TCP_ICMP");
	my_conf.generic.flags &= 0xfffefffful;
	my_conf.generic.flags |= config[0] != '0' ? 0x10000ul : 0ul;

	config = getvstr("MSS");
	if (config[1])
		my_conf.mss = read_word(config);
	config = getvstr("RCV_WND");
	if (config[1])
		my_conf.rcv_window = read_word(config);
	config = getvstr("DEF_RTT");
	if (config[1])
		my_conf.def_rtt = read_word(config);
	config = getvstr("DEF_TTL");
	if (config[1])
		my_conf.def_ttl = read_word(config);

	if ((last_port = my_conf.generic.flags & 0xfffful) >= 30000)
		last_port = 29999;

	my_conf.max_slt = 4 * my_conf.def_rtt;

	return TRUE;
}


int main(int argc, char **argv)
{
	DRV_LIST *sting_drivers;

	if (argc != 2 || strcmp(argv[1], "STinG_Load") != 0)
	{
		(void) Cconws(fault);
		return 1;
	}

	sting_drivers = (DRV_LIST *) Supexec(get_sting_cookie);

	if (sting_drivers == 0)
		return 1;

	if (strcmp(sting_drivers->magic, STIK_DRVR_MAGIC) != 0)
		return 1;

	tpl = (TPL *) (*sting_drivers->get_dftab) (TRANSPORT_DRIVER);
	stx = (STX *) (*sting_drivers->get_dftab) (MODULE_DRIVER);

	if (tpl != NULL && stx != NULL)
	{
		if (install())
			Ptermres(_PgmSize, 0);
	}
	return 1;
}
