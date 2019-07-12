
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
#include <stdlib.h>

#include "transprt.h"
#include "layer.h"

#include "tcp.h"


#define  M_YEAR    1999
#define  M_MONTH   12
#define  M_DAY     1
#define  M_VERSION "01.35"
#define  M_AUTHOR  "Peter Rottengatter|     &  STinG Evolution Team"


void           _appl_yield (void);
void           wait_flag (int16 *semaphore);
void           rel_flag (int16 *semaphore);
long           dis_intrpt (void);
long           en_intrpt (void);
int16         req_flag (int16 *semaphore);
int32 cdecl get_sr(void *);
int32 cdecl set_sr(void *);

void    cdecl  timer_function (void);
int16         timer_work (CONNEC *connec);
int16          poll_receive (CONNEC *connec);
int16   cdecl  do_ICMP (IP_DGRAM *dgram);
void           send_sync (CONNEC *connec);
void           close_self (CONNEC *connec, int16 reason);
void           flush_queue (NDB **queue);
int16          receive (CONNEC *connec, uint8 *buffer, int16 *length, int16 flag);
int16          categorize (CONNEC *connec);
int32 cdecl unlink_connect(void *param);

int16   cdecl  TCP_handler (IP_DGRAM *dgram);
void           do_output (CONNEC *connec);
void          destroy_conn (CONNEC *connec);

long           get_sting_cookie (void);
int16          install (void);
uint16         read_word (char *string);
int32 cdecl next_port (void *);
int16   cdecl  my_TCP_open (uint32 rem_host, uint16 rem_port, uint16 tos, uint16 size);
int16   cdecl  my_TCP_close (int16 connec, int16 mode, int16 *result);
int16   cdecl  my_TCP_send (int16 connec, void *buffer, int16 length);
int16   cdecl  my_TCP_wait_state (int16 connec, int16 state, int16 timeout);
int16   cdecl  my_TCP_ack_wait (int16 connec, int16 timeout);
int16   cdecl  my_TCP_info (int16 connec, TCPIB *block);
int16   cdecl  my_CNkick (void *connec);
int16   cdecl  my_CNbyte_count (void *connec);
int16   cdecl  my_CNget_char (void *connec);
NDB *   cdecl  my_CNget_NDB (void *connec);
int16   cdecl  my_CNget_block (void *connec, void *buffer, int16 length);
CIB *   cdecl  my_CNgetinfo (void *connec);
int16   cdecl  my_CNgets (void *connec, char *buffer, int16 length, char delimiter);
void abort_conn(CONNEC *connec);
uint32 inet_addr(const char *cp);


DRV_LIST  *sting_drivers;
TPL       *tpl;
STX       *stx;
TCP_CONF  my_conf  =  {{  "TCP", M_VERSION, 0x10400L, ((M_YEAR - 1980) << 9) | (M_MONTH << 5) | M_DAY, 
                          M_AUTHOR, 0, NULL, NULL },
                          2000, 2000, 1500, 64, 6000, 0, 0, 0 };
CN_FUNCS  cn_vectors = {  my_CNkick, my_CNbyte_count, my_CNget_char, my_CNget_NDB,
                          my_CNget_block, my_CNgetinfo, my_CNgets   };
uint16    last_port;
CONNEC    *root_list = NULL;
char      fault[] = "TCP.STX : STinG extension module. Only to be started by STinG !\r\n";



void  main (argc, argv)

int   argc;
char  *argv[];

{
   if (argc != 2) {
        Cconws (fault);   return;
      }
   if (strcmp (argv[1], "STinG_Load") != 0) {
        Cconws (fault);   return;
      }

   sting_drivers = (DRV_LIST *) Supexec (get_sting_cookie);

   if (sting_drivers == 0L)   return;

   if (strcmp (sting_drivers->magic, MAGIC) != 0)
        return;

   tpl = (TPL *) (*sting_drivers->get_dftab) (TRANSPORT_DRIVER);
   stx = (STX *) (*sting_drivers->get_dftab) (MODULE_DRIVER);

   if (tpl != (TPL *) NULL && stx != (STX *) NULL) {
        if (install())
             Ptermres (_PgmSize, 0);
      }
 }


long  get_sting_cookie()

{
   long  *work;

   for (work = * (long **) 0x5a0L; *work != 0L; work += 2)
        if (*work == 'STiK')
             return (*++work);

   return (0L);
 }


int16  install()

{
   LAYER  *layers;
   int16  count;
   char   *config;

   if (! ICMP_handler (do_ICMP, HNDLR_SET))
        return (FALSE);

   if (! IP_handler (P_TCP, TCP_handler, HNDLR_SET)) {
        ICMP_handler (do_ICMP, HNDLR_REMOVE);
        return (FALSE);
      }

   if (! TIMER_call (timer_function, HNDLR_SET)) {
        ICMP_handler (do_ICMP, HNDLR_REMOVE);
        IP_handler (P_TCP, TCP_handler, HNDLR_REMOVE);
        return (FALSE);
      }

   if (PRTCL_announce (P_TCP)) {
        ICMP_handler (do_ICMP, HNDLR_REMOVE);
        IP_handler (P_TCP, TCP_handler, HNDLR_REMOVE);
        TIMER_call (timer_function, HNDLR_REMOVE);
        return (FALSE);
      }

   my_conf.generic.basepage = _BasPag;

   query_chains (NULL, NULL, (void **) & layers);

   while (layers->next)
        layers = layers->next;

   layers->next = & my_conf.generic;

   tpl->TCP_open       = my_TCP_open;
   tpl->TCP_close      = my_TCP_close;
   tpl->TCP_send       = my_TCP_send;
   tpl->TCP_wait_state = my_TCP_wait_state;
   tpl->TCP_ack_wait   = my_TCP_ack_wait;
   tpl->TCP_info       = my_TCP_info;

   config = getvstr ("TCP_PORT");
   if (config[1]) {
        my_conf.generic.flags &= 0xffff0000ul;
        my_conf.generic.flags |= read_word (config);
      }
   config = getvstr ("TCP_ICMP");
   my_conf.generic.flags &= 0xfffefffful;
   my_conf.generic.flags |= (config[0] != '0') ? 0x10000ul : 0ul;

   config = getvstr ("MSS");
   if (config[1])
        my_conf.mss        = read_word (config);
   config = getvstr ("RCV_WND");
   if (config[1])
        my_conf.rcv_window = read_word (config);
   config = getvstr ("DEF_RTT");
   if (config[1])
        my_conf.def_rtt    = read_word (config);
   config = getvstr ("DEF_TTL");
   if (config[1])
        my_conf.def_ttl    = read_word (config);

   if ((last_port = my_conf.generic.flags & 0xfffful) >= 30000)
        last_port = 29999;

   my_conf.max_slt = 4 * my_conf.def_rtt;

   return (TRUE);
 }


uint16  read_word (string)

char  *string;

{
   uint16  result = 0;

   while (*string == ' ')
        string++;

   while ('0' <= *string && *string <= '9')
        result = result * 10 + (*string++ - '0');

   return (result);
 }


int32 cdecl next_port(void *param)
{
   CONNEC  *connect;

   (void)param;
   for (;;) {
        last_port++;

        if (last_port > 32765 || last_port < (my_conf.generic.flags & 0xfffful))
             last_port = my_conf.generic.flags & 0xfffful;

        for (connect = root_list; connect; connect = connect->next)
             if (connect->local_port == last_port)
                  break;
        if (connect)   continue;

        return (last_port);
      }
 }


int16  cdecl  my_TCP_open (rem_host, rem_port, tos, buff_size)

uint32  rem_host;
uint16  rem_port, tos;
uint16  buff_size;

{
   CAB     *cab;
   CONNEC  *connect;
   uint32  lcl_host = 0L, aux_ip, error;
   uint16  act_pass, lport, rport, mtu, max_mss = 32768u;
   int16   window, ttl, handle;

   if (rem_host == 0L && (rem_port == TCP_ACTIVE || rem_port == TCP_PASSIVE))
        rem_port = protect_exec(NULL, next_port);

   if (rem_port != TCP_ACTIVE && rem_port != TCP_PASSIVE) {
        if (rem_host) {
             act_pass = TCP_ACTIVE;   lport = protect_exec(NULL, next_port);  rport = rem_port;
           }
          else {
             act_pass = TCP_PASSIVE;  lport = rem_port;     rport = 0;
           }
      }
     else {
        cab = (CAB *) rem_host;
        act_pass = rem_port;
        rem_host = cab->rhost;   rport =  cab->rport;
        lcl_host = cab->lhost;   lport = (cab->lport) ? cab->lport : protect_exec(NULL, next_port);
      }

   if (rem_host != 0L) {
        if (PRTCL_get_parameters (rem_host, & aux_ip, & ttl, & mtu) != E_NORMAL)
             return (E_UNREACHABLE);
        lcl_host = (lcl_host) ? lcl_host : aux_ip;
        max_mss = mtu - sizeof (IP_HDR) - sizeof (TCP_HDR);
      }
     else {
        if (act_pass == TCP_ACTIVE)   return (E_PARAMETER);
      }

   ttl = my_conf.def_ttl;
   window = (buff_size > 0) ? buff_size : my_conf.rcv_window;

   if ((connect = (CONNEC *) KRmalloc (sizeof (CONNEC))) == NULL)
        return (E_NOMEM);

   if ((handle = PRTCL_request (connect, & cn_vectors)) == -1) {
        KRfree (connect);
        return (E_NOMEM);
      }

   connect->handle  = handle;
   connect->act_pass = act_pass;
   connect->remote_IP_address_orig  = rem_host;
   connect->rport_orig = rport;
   connect->local_IP_address_orig = lcl_host;
   connect->lport_orig = lport;
   connect->remote_IP_address = rem_host;
   connect->remote_port       = rport;
   connect->local_IP_address  = lcl_host;
   connect->local_port        = lport;
   connect->flags             = 0;
   connect->mss               = (my_conf.mss < max_mss) ? my_conf.mss : max_mss;
   connect->mtu               = mtu;
   connect->tos               = tos;
   connect->ttl               = ttl;
   connect->info              = NULL;
   connect->reason            = 0;
   connect->net_error         = 0;

   connect->send.window       = window;
   connect->send.bufflen      = window;
   connect->o140 = 0x430;
   connect->o142 = 0xffff;
   connect->send.total        = 0;
   connect->send.count        = 0;
   connect->send.queue        = NULL;
   connect->send.start = TIMER_now() - 1500;

   connect->recve.window      = my_conf.rcv_window;
   connect->recve.reseq       = NULL;
   connect->recve.count       = 0;
   connect->recve.queue       = NULL;

   connect->rtrp.mode         = FALSE;
   connect->rtrp.smooth       = my_conf.def_rtt;
   connect->rtrn.mode         = FALSE;
   if (connect->rtrp.smooth < 100)
      connect->rtrp.smooth = 100;
   else if (connect->rtrp.smooth > 30000)
      connect->rtrp.smooth = 30000;
      
   connect->rtrn.timeout      = 2 * connect->rtrp.smooth;
   connect->rtrn.start        = TIMER_now();
   connect->rtrn.backoff      = 0;

   connect->sema              = -1;
   connect->pending           = NULL;
   connect->result            = NULL;

   /* BUG: not protected */
   connect->next = root_list;
   root_list     = connect;

   if (act_pass == TCP_ACTIVE) {
        send_sync (connect);
        connect->state = TSYN_SENT;
        my_conf.con_out++;
        do_output (connect);
      }
     else
        connect->state = TLISTEN;

   rel_flag (& connect->sema);

   if ((error = connect->net_error) == 0)
        return (handle);

   protect_exec(connect, unlink_connect);

   KRfree (connect);   PRTCL_release (handle);

   return (error);
 }


int16  cdecl  my_TCP_close (connec, mode, result)

int16  connec, mode, *result;

{
   CONNEC  *conn;
   int16   retval = E_NOROUTINE;
   int32 now;
   int32 start;
   int16 error;

   now = TIMER_now();

   if ((conn = PRTCL_lookup (connec, & cn_vectors)) == NULL)
        return (E_BADHANDLE);

   if (mode >= 0) {
        if (conn->result != NULL) {
             conn->result = NULL;   return (E_NORMAL);
           }
        if (mode >= 1000)
            mode = 999;
      }
     else
        conn->result = result;

   if (conn->flags & CLOSING)
   {
       if (conn->state != TCLOSED)
           close_self(conn, E_BADCLOSE);
       return E_BADCLOSE;
   }
   conn->close.start = now;
   conn->close.timeout = 1000000L;

   switch (conn->state) {
      case TLISTEN :
      case TSYN_SENT :
        if ((error = req_flag(&conn->sema)) != 0 &&
            !(conn->flags & DEFERRED) &&
            !(protect_exec(0, get_sr) & 0x2000))
        {
        	start = TIMER_now();
        	while ((error = req_flag(&conn->sema)) != 0 && TIMER_elapsed(start) < 1000)
        		_appl_yield();
        }
        if (error)
        	return E_LOCKED;
        close_self (conn, E_NORMAL);
        conn->flags |= CLOSING;
        rel_flag (& conn->sema);
        retval = E_NORMAL;
        break;
      case TSYN_RECV :
      case TESTABLISH :
      case TCLOSE_WAIT :
        if ((error = req_flag(&conn->sema)) != 0 &&
            !(conn->flags & DEFERRED) &&
            !(protect_exec(0, get_sr) & 0x2000))
        {
        	start = TIMER_now();
        	while ((error = req_flag(&conn->sema)) != 0 && TIMER_elapsed(start) < 1000)
        		_appl_yield();
        }
        if (error)
        	return E_LOCKED;
        conn->flags |= FLAG40;
        ++conn->send.count;
        conn->state = conn->state == TCLOSE_WAIT ? TLAST_ACK : TFIN_WAIT1;
        do_output(conn);
        rel_flag(&conn->sema);
        if (!(conn->flags & DEFERRED) && mode >= 0)
        {
        	if (conn->recve.count != 0)
        	{
        		retval = mode != 0 ? E_CNTIMEOUT : E_NORMAL;
        		abort_conn(conn);
        		my_conf.generic.stat_dropped++;
	        	close_self(conn, retval);
		        destroy_conn(conn);
		        return retval;
        	} else
        	{
        		retval = E_NODATA;
        		conn->result = &retval;
        		conn->flags |= CLOSING | DISCARD;
        		while (retval == E_NODATA && TIMER_elapsed(now) < mode * 1000L)
        		{
	        		if (!(protect_exec(NULL, get_sr) & 0x2000))
	        			_appl_yield();
	        	}
	        	conn->result = NULL;
	        	if (retval == E_NODATA)
	        	{
	        		retval = mode != 0 ? E_CNTIMEOUT : E_NORMAL;
	        	}
        	}
		} else
		{
			conn->flags |= CLOSING;
			if (conn->flags & DEFERRED)
				retval = E_LOCKED;
			else
				retval = E_NODATA;
		}
		break;
      case TCLOSED :
		conn->flags &= ~CLOSING;
        retval = conn->reason == 0 ? E_EOF : conn->reason;
        if (result)
        	*result = retval;
        destroy_conn(conn);
        return retval;
      case TFIN_WAIT1 :
      case TFIN_WAIT2 :
      case TCLOSING :
      case TLAST_ACK :
      case TTIME_WAIT :
        close_self(conn, E_BADCLOSE);
        conn->flags |= CLOSING;
        retval = E_BADCLOSE;
		break;
      }

   if (mode < 0 && result != NULL)
        *result = retval;

   return (retval);
 }


int16  cdecl  my_TCP_send (connec, buffer, length)

int16  connec, length;
void   *buffer;

{
   CONNEC  *conn;
   NDB     *ndb, *walk;
   uint8   *data;
   int16   error;
   int16   error2;

   if ((conn = PRTCL_lookup (connec, & cn_vectors)) == NULL)
        return (E_BADHANDLE);

   error = conn->net_error;

   if (error < 0) {
        conn->net_error = 0;   return (error);
      }

   switch (categorize (conn)) {
      case C_FIN :
      case C_CLSD :      return (E_EOF);
      case C_LISTEN :    return (E_LISTEN);
      case C_DEFAULT :   return (E_NODATA);
      }

   if (conn->send.bufflen - conn->send.total < length)
        return (E_OBUFFULL);

   if ((ndb = (NDB *) KRmalloc (sizeof (NDB))) == NULL)
        return (E_NOMEM);

   if ((data = (uint8 *) KRmalloc (length)) == NULL) {
        KRfree (ndb);
        return (E_NOMEM);
      }

   ndb->ptr  = ndb->ndata = data;
   ndb->len  = length;
   ndb->next = NULL;
   memcpy (data, buffer, length);

    if ((error2 = req_flag(&conn->sema)) != 0 &&
        !(conn->flags & DEFERRED) &&
        !(protect_exec(0, get_sr) & 0x2000))
    {
    	int32 now = TIMER_now();
    	while ((error2 = req_flag(&conn->sema)) != 0 && TIMER_elapsed(now) < 1000)
    		_appl_yield();
    }
    if (error2)
    {
    	KRfree(data);
    	KRfree(ndb);
    	return E_LOCKED;
	}

   if (conn->send.queue) {
        for (walk = conn->send.queue; walk->next; walk = walk->next);
        walk->next = ndb;
      }
     else {
        conn->send.queue = ndb;
      }

   conn->send.count += length;
   conn->send.total += length;
   do_output (conn);

   rel_flag (& conn->sema);

   return (E_NORMAL);
 }


int16  cdecl  my_TCP_wait_state (int16 connec, int16 state, int16 timeout)
{
   CONNEC  *conn;
   int16   err;
   uint32  timer, time_out;

   if ((conn = PRTCL_lookup (connec, & cn_vectors)) == NULL)
        return (E_BADHANDLE);

   err = conn->net_error;

   if (err < 0) {
        conn->net_error = 0;   return (err);
      }

   time_out = 1000L * timeout;
   if (time_out != 0 && (conn->flags & DEFERRED))
   		return E_PARAMETER;

   timer    = TIMER_now();
   while (conn->state != state) {
        if (TIMER_elapsed (timer) >= time_out) {
             return (E_CNTIMEOUT);
           }
        if (!(protect_exec(NULL, get_sr) & 0x2000))
	        _appl_yield();
	    err = conn->net_error;

	    if (err < 0) {
	        conn->net_error = 0;   return (err);
	    }
      }

   return (E_NORMAL);
 }


int16  cdecl  my_TCP_ack_wait (int16 connec, int16 timeout)
{
   CONNEC  *conn;
   int16   err;
   uint32  timer;

   if ((conn = PRTCL_lookup (connec, & cn_vectors)) == NULL)
        return (E_BADHANDLE);

   err = conn->net_error;

   if (err < 0) {
        conn->net_error = 0;   return (err);
      }

   if (timeout != 0 && (conn->flags & DEFERRED))
   		return E_PARAMETER;

   timer = TIMER_now();

   while (conn->send.total > 0) {
        if (TIMER_elapsed (timer) >= timeout) {
             return (E_CNTIMEOUT);
           }
        if (!(protect_exec(NULL, get_sr) & 0x2000))
        	_appl_yield();
	    err = conn->net_error;

	    if (err < 0) {
	        conn->net_error = 0;   return (err);
	    }
      }

   return (E_NORMAL);
 }


int16  cdecl  my_TCP_info (connec, block)

int16  connec;
TCPIB  *block;

{
	CONNEC *conn;
	uint32 request;
	int16 error;
   
	if ((conn = PRTCL_lookup(connec, &cn_vectors)) == NULL)
		return E_BADHANDLE;

	if ((long)block <= 0 || (request = block->request) > TCPI_mask)
		return E_PARAMETER;

	if (request & TCPI_defer)
		conn->flags |= DEFERRED;
	if (request & TCPI_state)
		block->state = conn->state;
	if (request & TCPI_unacked)
		block->unacked = conn->send.count;
	if (request & TCPI_srtt)
		block->srtt = conn->rtrp.smooth;
	
	error = conn->net_error;
	if (error < 0)
		conn->net_error = 0;
	else
		error = TCPI_bits;
	return error;
}


int16  cdecl  my_CNkick (void *connec)
{
   CONNEC  *conn = connec;
   int16   error;
   int16   error2;

   if ((error = poll_receive (connec)) < 0)
        return (error);

    if ((error2 = req_flag(&conn->sema)) != 0 &&
        !(conn->flags & DEFERRED) &&
        !(protect_exec(0, get_sr) & 0x2000))
    {
    	int32 now = TIMER_now();
    	while ((error2 = req_flag(&conn->sema)) != 0 && TIMER_elapsed(now) < 1000)
    		_appl_yield();
    }
    if (error2)
    {
    	return E_LOCKED;
	}

   conn->rtrn.mode    = FALSE;
   conn->rtrn.backoff = 0;
   conn->rtrn.start   = TIMER_now();
   conn->rtrn.timeout = 2 * conn->rtrp.smooth;

   conn->flags |= FORCE;

   do_output (conn);
   rel_flag (& conn->sema);

   return (E_NORMAL);
 }


int16  cdecl  my_CNbyte_count (void *connec)
{
   CONNEC  *conn = connec;
   int16   error;

   if ((error = poll_receive (connec)) < 0)
        return (error);

   switch (categorize (conn)) {
      case C_END :
      case C_CLSD :      return (E_EOF);
      case C_LISTEN :    return (E_LISTEN);
      case C_DEFAULT :   return (E_NODATA);
      }

   return ((int16) conn->recve.count);
 }


int16  cdecl  my_CNget_char (void  *connec)
{
   CONNEC  *conn = connec;
   int16   error, length = 1;
   uint8   character;
   int16 error2;

   if ((error = poll_receive (connec)) < 0)
        return (error);

   switch (categorize (conn)) {
      case C_END :
      case C_CLSD :      return (E_EOF);
      case C_LISTEN :    return (E_LISTEN);
      case C_DEFAULT :   return (E_NODATA);
      }

    if ((error2 = req_flag(&conn->sema)) != 0 &&
        !(conn->flags & DEFERRED) &&
        !(protect_exec(0, get_sr) & 0x2000))
    {
    	int32 now = TIMER_now();
    	while ((error2 = req_flag(&conn->sema)) != 0 && TIMER_elapsed(now) < 1000)
    		_appl_yield();
    }
    if (error2)
    {
    	return E_LOCKED;
	}

   receive (conn, & character, & length, TRUE);
   rel_flag (& conn->sema);

   return ((length) ? (int16) character : E_NODATA);
 }


NDB *  cdecl  my_CNget_NDB (void *connec)
{
   CONNEC  *conn = connec;
   NDB    *ndb;
   int16  flag = categorize (connec);
	int16 error2;

   if (poll_receive (connec) < 0)
        return (NULL);

   if (flag != C_READY && flag != C_FIN)
        return (NULL);

   flag = -1;

    if ((error2 = req_flag(&conn->sema)) != 0 &&
        !(conn->flags & DEFERRED) &&
        !(protect_exec(0, get_sr) & 0x2000))
    {
    	int32 now = TIMER_now();
    	while ((error2 = req_flag(&conn->sema)) != 0 && TIMER_elapsed(now) < 1000)
    		_appl_yield();
    }
    if (error2)
    {
    	return (NDB *)E_LOCKED;
	}

   receive (conn, (uint8 *) & ndb, & flag, FALSE);
   rel_flag (& conn->sema);

   if (flag < 0)
        return (NULL);
     else {
        ndb->next = NULL;   return (ndb);
      }
 }


int16  cdecl  my_CNget_block (void *connec, void *buffer, int16 length)
{
   CONNEC  *conn = connec;
   int16   error;
   int16   error2;

   if ((error = poll_receive (connec)) < 0)
        return (error);

   switch (categorize (conn)) {
      case C_END :
      case C_CLSD :      return (E_EOF);
      case C_LISTEN :    return (E_LISTEN);
      case C_DEFAULT :   return (E_NODATA);
      }

   if (length <= 0)   return (0);

   if (length > conn->recve.count)
        return (E_NODATA);

    if ((error2 = req_flag(&conn->sema)) != 0 &&
        !(conn->flags & DEFERRED) &&
        !(protect_exec(0, get_sr) & 0x2000))
    {
    	int32 now = TIMER_now();
    	while ((error2 = req_flag(&conn->sema)) != 0 && TIMER_elapsed(now) < 1000)
    		_appl_yield();
    }
    if (error2)
    {
    	return E_LOCKED;
	}

   receive (conn, buffer, & length, FALSE);
   rel_flag (& conn->sema);

   return (length);
 }


char *masquerade_port = "Masquerade";

CIB *  cdecl  my_CNgetinfo (void *connec)
{
   CONNEC  *conn = connec;
   CIB     *cib;

   if (conn->info == NULL) {
        if ((conn->info = (CIB *) KRmalloc (sizeof (CIB))) == NULL)
             return (NULL);
        conn->info->status = 0;
      }

   cib = conn->info;

   cib->protocol = P_TCP;
   cib->address.lport = conn->local_port;
   cib->address.rport = conn->remote_port;
   cib->address.rhost = conn->remote_IP_address;
   if ((cib->address.lhost = conn->local_IP_address) == 0)
   {
		const char *config;
		
		config = getvstr("FORCED_IP");
		if (strlen(config) > 6)
		{
			cib->address.lhost = inet_addr(config);
		} else
		{
			if (query_port(masquerade_port))
			{
				cntrl_port(masquerade_port, (uint32)&cib->address.lhost, CTL_MASQUE_GET_REALIP);
			} else
			{
				if (cib->address.rhost != 0)
				{
					PRTCL_get_parameters(cib->address.rhost, &cib->address.lhost, NULL, NULL);
				} else
				{
					PRTCL_get_parameters(0x0A00FF49UL, &cib->address.lhost, NULL, NULL);
				}
			}
		}
   }

   return (cib);
 }


uint32 inet_addr(const char *cp)
{
	uint32 ip_a, ip_b, ip_c, ip_d;

	ip_a = (uint32)atoi(cp);
	cp = strchr(cp, '.');
	if (cp == NULL)
		return 0;
	++cp;
	ip_b = (uint32)atoi(cp);
	cp = strchr(cp, '.');
	if (cp == NULL)
		return 0;
	++cp;
	ip_c = (uint32)atoi(cp);
	cp = strchr(cp, '.');
	if (cp == NULL)
		return 0;
	++cp;
	ip_d = (uint32)atoi(cp);
	return (ip_a << 24) | (ip_b << 16) | (ip_c << 8) | ip_d;
}


int16  cdecl  my_CNgets (void *connec, char *buffer, int16 length, char delimiter)
{
   CONNEC  *conn = connec;
   NDB     *walk;
   int16   error, count, amount;
   uint8   *search;
   int error2;

   if ((error = poll_receive (connec)) < 0)
        return (error);

   switch (categorize (conn)) {
      case C_END :
      case C_CLSD :      return (E_EOF);
      case C_LISTEN :    return (E_LISTEN);
      case C_DEFAULT :   return (E_NODATA);
      }

   if (conn->recve.count == 0)   return (E_NODATA);

   if (length <= 1)   return (E_BIGBUF);

    if ((error2 = req_flag(&conn->sema)) != 0 &&
        !(conn->flags & DEFERRED) &&
        !(protect_exec(0, get_sr) & 0x2000))
    {
    	int32 now = TIMER_now();
    	while ((error2 = req_flag(&conn->sema)) != 0 && TIMER_elapsed(now) < 1000)
    		_appl_yield();
    }
    if (error2)
    {
    	return E_LOCKED;
	}

   for (walk = conn->recve.queue, amount = 0; walk != NULL; walk = walk->next) {
        search = walk->ndata;
        for (count = 0; count < walk->len && amount < length; count++, amount++) {
             if (*search++ == delimiter) {
                  amount++;
                  receive (conn, buffer, & amount, FALSE);
                  rel_flag (& conn->sema);
                  buffer[--amount] = '\0';
                  return (amount);
                }
           }
        if (amount == length) {
             rel_flag (& conn->sema);   return (E_BIGBUF);
           }
      }

   rel_flag (& conn->sema);

   return (E_NODATA);
 }
