/*********************************************************************/
/*                                                                   */
/*     STinG : API and IP kernel package                             */
/*                                                                   */
/*                                                                   */
/*      Version 1.1                         from 8. Januar 1997      */
/*                                                                   */
/*      Module for Application Programming Interface, Dummy calls    */
/*                                                                   */
/*********************************************************************/


#include <tos.h>
#include <stdio.h>
#include <string.h>

#include "globdefs.h"


#define  NUM_LAYER   2

#undef get_dftab
#undef ETM_exec


typedef struct driver
{
	char magic[10];
	DRV_HDR *cdecl (*get_drvfunc) (const char *);
	int16 cdecl (*ETM_exec) (const char *);
	CONFIG *cfg;
	BASPAG *basepage;
	DRV_HDR *layer[NUM_LAYER];
} GENERIC;

extern GENERIC cookie;

CONFIG conf;




static DRV_HDR *cdecl get_drv_func(const char *drv_name)
{
	int count;

	for (count = 0; count < NUM_LAYER; count++)
		if (strcmp(cookie.layer[count]->module, drv_name) == 0)
			return cookie.layer[count];

	return NULL;
}


static int16 cdecl ETM_exec(const char *module)
{
	UNUSED(module);
	return 0;
}


static int16 cdecl TCP_open(uint32 rem_host, uint16 rem_port, uint16 tos, uint16 buff_size)
{
	UNUSED(rem_host);
	UNUSED(rem_port);
	UNUSED(tos);
	UNUSED(buff_size);
	return E_UNREACHABLE;
}


static int16 cdecl TCP_close(int16 connec, int16 mode, int16 *result)
{
	UNUSED(connec);
	UNUSED(mode);
	UNUSED(result);
	return E_BADHANDLE;
}


static int16 cdecl TCP_send(int16 connec, const void *buffer, int16 length)
{
	UNUSED(connec);
	UNUSED(buffer);
	UNUSED(length);
	return E_BADHANDLE;
}


static int16 cdecl TCP_wait_state(int16 connec, int16 state, int16 timeout)
{
	UNUSED(connec);
	UNUSED(state);
	UNUSED(timeout);
	return E_BADHANDLE;
}


static int16 cdecl TCP_ack_wait(int16 connec, int16 timeout)
{
	UNUSED(connec);
	UNUSED(timeout);
	return E_BADHANDLE;
}


static int16 cdecl TCP_info(int16 connec, TCPIB *tcp_info)
{
	UNUSED(connec);
	UNUSED(tcp_info);
	return E_BADHANDLE;
}


static int16 cdecl UDP_open(uint32 rem_host, uint16 rem_port)
{
	UNUSED(rem_host);
	UNUSED(rem_port);
	return E_UNREACHABLE;
}


static int16 cdecl UDP_close(int16 connec)
{
	UNUSED(connec);
	return E_BADHANDLE;
}


static int16 cdecl UDP_send(int16 connec, const void *buffer, int16 length)
{
	UNUSED(connec);
	UNUSED(buffer);
	UNUSED(length);
	return E_BADHANDLE;
}


static int16 cdecl CNkick(int16 connec)
{
	void *anonymous;
	CN_FUNCS *entry;

	if (handle_lookup(connec, &anonymous, &entry) == 0)
		return E_BADHANDLE;

	return (*entry->CNkick) (anonymous);
}


static int16 cdecl CNbyte_count(int16 connec)
{
	void *anonymous;
	CN_FUNCS *entry;

	if (handle_lookup(connec, &anonymous, &entry) == 0)
		return E_BADHANDLE;

	return (*entry->CNbyte_count) (anonymous);
}


static int16 cdecl CNget_char(int16 connec)
{
	void *anonymous;
	CN_FUNCS *entry;

	if (handle_lookup(connec, &anonymous, &entry) == 0)
		return E_BADHANDLE;

	return (*entry->CNget_char) (anonymous);
}


static NDB *cdecl CNget_NDB(int16 connec)
{
	void *anonymous;
	CN_FUNCS *entry;

	if (handle_lookup(connec, &anonymous, &entry) == 0)
		return NULL;

	return (*entry->CNget_NDB) (anonymous);
}


static int16 cdecl CNget_block(int16 connec, void *buffer, int16 length)
{
	void *anonymous;
	CN_FUNCS *entry;

	if (handle_lookup(connec, &anonymous, &entry) == 0)
		return E_BADHANDLE;

	return (*entry->CNget_block) (anonymous, buffer, length);
}


static CIB *cdecl CNgetinfo(int16 connec)
{
	void *anonymous;
	CN_FUNCS *entry;

	if (handle_lookup(connec, &anonymous, &entry) == 0)
		return NULL;

	return (*entry->CNgetinfo) (anonymous);
}


static int16 cdecl CNgets(int16 connec, char *buffer, int16 length, char delimiter)
{
	void *anonymous;
	CN_FUNCS *entry;

	if (handle_lookup(connec, &anonymous, &entry) == 0)
		return E_BADHANDLE;

	return (*entry->CNgets) (anonymous, buffer, length, delimiter);
}


static int16 cdecl resolve(const char *domain, char **real_domain, uint32 *ip_list, int16 ip_num)
{
	UNUSED(domain);
	UNUSED(real_domain);
	UNUSED(ip_list);
	UNUSED(ip_num);
	return E_CANTRESOLVE;
}


static void cdecl serial_dummy(void)
{
	/* Do really nothing, as these functions are obsolete ! */
}


static int16 cdecl carrier_detect(void)
{
	return 1;
}


static void cdecl house_keep(void)
{
	/* Do really nothing, as this function is obsolete ! */
}


static TPL tpll = {
	TRANSPORT_DRIVER,
	"Peter Rottengatter",
	TCP_DRIVER_VERSION,
	KRmalloc,
	KRfree,
	KRgetfree,
	KRrealloc,
	get_error_text,
	getvstr,
	carrier_detect,
	TCP_open,
	TCP_close,
	TCP_send,
	TCP_wait_state,
	TCP_ack_wait,
	UDP_open,
	UDP_close,
	UDP_send,
	CNkick,
	CNbyte_count,
	CNget_char,
	CNget_NDB,
	CNget_block,
	house_keep,
	resolve,
	serial_dummy,
	serial_dummy,
	set_flag,
	clear_flag,
	CNgetinfo,
	on_port,
	off_port,
	setvstr,
	query_port,
	CNgets,
	ICMP_send,
	ICMP_handler,
	ICMP_discard,
	TCP_info,
	cntrl_port
};

static STX stxl = {
	MODULE_DRIVER,
	"Peter Rottengatter",
	STX_LAYER_VERSION,
	set_dgram_ttl,
	check_dgram_ttl,
	routing_table,
	set_sysvars,
	query_chains,
	IP_send,
	IP_fetch,
	IP_handler,
	IP_discard,
	PRTCL_announce,
	PRTCL_get_parameters,
	PRTCL_request,
	PRTCL_release,
	PRTCL_lookup,
	TIMER_call,
	TIMER_now,
	TIMER_elapsed,
	protect_exec,
	get_route_entry,
	set_route_entry
};

GENERIC cookie = {
	"STiKmagic",
	get_drv_func,
	ETM_exec,
	&conf,
	NULL,
	{ (DRV_HDR *) &tpll, (DRV_HDR *) &stxl }
};
static long my_jar[8] = { 0L, 4L };

long init_cookie(void)
{
	int cnt_cookie;
	long *work;
	long *jar;
	long *new;

	conf.new_cookie = FALSE;

	if ((work = *(long **) 0x5a0L) == NULL)
	{
		conf.new_cookie = TRUE;
		*(long **) 0x5a0L = &my_jar[0];
	}

	for (work = *(long **) 0x5a0L, cnt_cookie = 0; *work != 0L; work += 2, cnt_cookie++)
		if (*work == 0x5354694bUL) /* 'STiK' */
			return -1;

	if (work[1] - 1 <= cnt_cookie)
	{
		if ((jar = (long *) Malloc((cnt_cookie + 8) * 2 * sizeof(long))) == NULL)
			return -1;
		for (work = *(long **) 0x5a0L, new = jar; *work != 0L; work += 2, new += 2)
			new[0] = work[0], new[1] = work[1];
		new[0] = 0L;
		new[1] = cnt_cookie + 8;
		work = new;
		*(long **) 0x5a0L = jar;
		conf.new_cookie = TRUE;
	}

	work[2] = work[0];
	work[0] = 0x5354694bUL; /* 'STiK' */
	work[3] = work[1];
	work[1] = (long) &cookie;

	cookie.basepage = _BasPag;

	return 0;
}
