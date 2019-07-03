/*********************************************************************/
/*                                                                   */
/*     Low Level Port : EtherNet Network Interface                   */
/*                                                                   */
/*                                                                   */
/*      Version 0.1                        vom 26. Januar 1998       */
/*                                                                   */
/*      Modul zur Installation und Aktivierung der Ports             */
/*                                                                   */
/*********************************************************************/


#include <tos.h>
#include <stdio.h>
#include <string.h>

#include "transprt.h"
#include "port.h"

#include "ether.h"

#define  M_YEAR    18
#define  M_MONTH   6
#define  M_DAY     18
#define  VERSION   "00.20"


#ifdef __GNUC__
#define _BasPag _base
#endif

#ifndef GNU_ASM_NAME
#ifdef __GNUC__
#define GNU_ASM_NAME(x) __asm__(x)
#else
#define GNU_ASM_NAME(x)
#endif
#endif


long check_hardware(void) GNU_ASM_NAME("check_hardware");
long cache_on(void) GNU_ASM_NAME("cache_on");
long cache_off(void) GNU_ASM_NAME("cache_off");
int bus_error(void) GNU_ASM_NAME("bus_error");
long berr_off(void) GNU_ASM_NAME("berr_off");

uint16 *rdp GNU_ASM_NAME("rdp");
uint16 *rap GNU_ASM_NAME("rap");
long _cpu GNU_ASM_NAME("_cpu");
void *memory GNU_ASM_NAME("memory");

TPL *tpl;
STX *stx;

PORT my_port = {
	"EtherNet",
	L_SER_BUS,
	FALSE,
	0L,
	0xffffffffUL,
	0xffffffffUL,
	1500,
	1500,
	0L,
	NULL,
	0L,
	NULL,
	0,
	NULL,
	NULL
};

static DRIVER my_driver;
uint8 address[6];
BAB *this_xmit;

static TMD *tmd_array;
static RMD *rmd_array;
static BAB xmit_bab[8];
static BAB recve_bab[32];
static BAB *this_recve;
static long cookie;
static uint16 ck_flag;
static uint16 gtype;
static char *hardware[6];
static int16 ctrl_type = -1;

static char const fault[] = "ETHER.STX : STinG extension module. Only to be started by STinG !\r\n";



static long read_cookie(void)
{
	long *work;

	ck_flag = FALSE;

	work = *(long **) 0x5a0L;
	if (work == NULL)
		return 0;

	for (; *work != 0; work += 2)
		if (*work == cookie)
		{
			ck_flag = TRUE;
			return *++work;
		}

	return -1;
}


static int get_cookie(long which, long *value)
{
	cookie = which;
	*value = Supexec(read_cookie);

	return ck_flag;
}


static void cdecl my_send(PORT *port)
{
	IP_DGRAM *dgram;

	if (port != &my_port || my_port.active == 0)
		return;

	cache_off();

	while ((this_xmit->buffer.xmit_buff->status & DS_OWN) == 0)
	{
		if (fetch_dgram(&dgram) == FALSE)
			return;
		if (!xmit_dgram(dgram, this_xmit))
			continue;
		*rdp = CSR0_TDMD;
		this_xmit = this_xmit->next_bab;
	}

	cache_on();
}


static void cdecl my_receive(PORT *port)
{
	if (port != &my_port || my_port.active == 0)
		return;

	cache_off();

	while ((this_recve->buffer.recve_buff->status & DS_OWN) == 0)
	{
		recve_dgram(this_recve);
		this_recve = this_recve->next_bab;
	}

	cache_on();
}


static long check_start(void)
{
	return (long) (*rdp & CSR0_IDON);
}


static long finish_start(void)
{
	if ((*rdp & CSR0_IDON) == 0)
	{
		*rdp = CSR0_STOP;
		return FALSE;
	}

	*rdp = CSR0_STRT;
	*rdp = CSR0_IDON;

	this_recve = &recve_bab[0];
	this_xmit = &xmit_bab[0];

	return TRUE;
}


static long stop_lance(void)
{
	cache_off();
	this_recve = &recve_bab[0];
	this_xmit = &xmit_bab[0];

	*rap = 0;
	*rdp |= CSR0_STOP;

	cache_on();
	return 0;
}


static long create_lance_structs(void)
{
	LANCE_INIT *init;
	TMD *twalk;
	RMD *rwalk;
	uint8 *buffer;
	int16 count;
	int16 length;

	init = (LANCE_INIT *) memory;
	tmd_array = (TMD *) (((long) (init + 1) + 7) & 0xfffffff8ul);
	rmd_array = (RMD *) (((long) &tmd_array[8] + 7) & 0xfffffff8ul);
	buffer = (uint8 *) &rmd_array[32];

	length = 1536;

	*rap = 0;
	*rdp = CSR0_STOP;

	for (count = 0, rwalk = rmd_array; count < 32; count++, rwalk++)
	{
		rwalk->addr_high = 0;
		rwalk->addr_low = (uint16) ((long) buffer & 0xffffL);
		rwalk->status = DS_STP | DS_ENP | DS_OWN;
		rwalk->ones = 0xf;
		rwalk->bcount = -length;
		rwalk->zeros = 0;
		rwalk->mcount = 0;
		recve_bab[count].buffer.recve_buff = rwalk;
		recve_bab[count].data = (ETH_HDR *) buffer;
		recve_bab[count].next_bab = &recve_bab[count < 31 ? count + 1 : 0];
		buffer += length;
	}
	this_recve = &recve_bab[0];

	for (count = 0, twalk = tmd_array; count < 8; count++, twalk++)
	{
		twalk->addr_high = 0;
		twalk->addr_low = (uint16) ((long) buffer & 0xffffL);
		twalk->status = DS_STP | DS_ENP;
		twalk->ones = 0xf;
		twalk->bcount = 0;
		twalk->buffer = twalk->uflow = twalk->reserv = twalk->lcoll = twalk->lcar = FALSE;
		twalk->tdr = 0;
		twalk->retry = FALSE;
		xmit_bab[count].buffer.xmit_buff = twalk;
		xmit_bab[count].data = (ETH_HDR *) buffer;
		xmit_bab[count].next_bab = &xmit_bab[count < 7 ? count + 1 : 0];
		buffer += length;
	}
	this_xmit = &xmit_bab[0];

	init->mode = 0;
	address[0] = init->addr[1];
	address[1] = init->addr[0];
	address[2] = init->addr[3];
	address[3] = init->addr[2];
	address[4] = init->addr[5];
	address[5] = init->addr[4];
	init->ladrf[0] = 0;
	init->ladrf[1] = 0;

	init->rdrp.dra_low = (long) rmd_array & 0xfffful;
	init->rdrp.dra_high = 0;
	init->rdrp.length = 5 << 5;

	init->tdrp.dra_low = (long) tmd_array & 0xfffful;
	init->tdrp.dra_high = 0;
	init->tdrp.length = 3 << 5;

	*rap = 1;
	*rdp = (long) init & 0xfffful;
	*rap = 2;
	*rdp = 0;
	*rap = 3;
	*rdp = CSR3_BSWP;

	*rap = 0;
	*rdp = CSR0_INIT;

	return 0;
}


static int16 cdecl my_set_state(PORT *port, int16 state)
{
	int32 now;

	if (port != &my_port)
		return FALSE;

	if (state)
	{
		if (Supexec(check_hardware) == FALSE)
			return FALSE;
		Supexec(cache_off);
		Supexec(create_lance_structs);
		now = TIMER_now();
		while (TIMER_elapsed(now) < 500 && Supexec(check_start) == 0)
			;
		now = Supexec(finish_start);
		Supexec(cache_on);
		if (now == FALSE)
			return FALSE;
		arp_init();
	} else
	{
		Supexec(stop_lance);
		deplete_queue(&my_port.send);
		deplete_queue(&my_port.receive);
	}

	return TRUE;
}


static int fetch_addresses(int type)
{
	int result = TRUE;
	long machine;

	switch (type)
	{
	case 0:
		rdp = PAM_RDP;
		rap = PAM_RAP;
		memory = PAM_MEMBOT;
		break;
	case 1:
		if (!get_cookie(0x5F4D4348L, &machine))	/* '_MCH' */
			machine = 0;
		if ((machine >> 16) != 2 && ((machine >> 16) != 1 || (machine & 0xffffL) != 16))
		{
			result = FALSE;
		} else
		{
			rdp = PAM_RDP;
			rap = PAM_RAP;
			memory = PAM_MEMBOT;
		}
		break;
	case 2:
		rdp = RIEBL_MEGA_RDP;
		rap = RIEBL_MEGA_RAP;
		memory = RIEBL_MEGA_MEMBOT;
		break;
	case 3:
		rdp = RIEBL_HACK_RDP;
		rap = RIEBL_HACK_RAP;
		memory = RIEBL_HACK_MEMBOT;
		break;
	case 4:
		if (!get_cookie(0x5F4D4348L, &machine))	/* '_MCH' */
			machine = 0;
		switch ((int)(machine >> 16))
		{
		case 1:
			if ((machine & 0xffffL) != 16)
			{
				result = FALSE;
			} else
			{
				rdp = RIEBL_MSTE_RDP;
				rap = RIEBL_MSTE_RAP;
				memory = RIEBL_MSTE_MEMBOT;
			}
			break;
		case 2:
			rdp = RIEBL_TT_RDP;
			rap = RIEBL_TT_RAP;
			memory = RIEBL_TT_MEMBOT;
			break;
		default:
			result = FALSE;
			break;
		}
		break;
	default:
		result = FALSE;
		break;
	}

	gtype = type;

	return result;
}


static int16 cdecl my_cntrl(PORT *port, uint32 argument, int16 code)
{
	int16 result = E_NORMAL;

	if (port != &my_port)
		return E_PARAMETER;

	if (bus_error())
	{
		berr_off();
		return E_UNREACHABLE;
	}

	switch (code)
	{
	case CTL_ETHER_SET_MAC:
		if (memory)
			memcpy(&((LANCE_INIT *) memory)->addr[0], (uint8 *) argument, 6);
		break;
	case CTL_ETHER_GET_MAC:
		if (memory)
			memcpy((uint8 *) argument, &((LANCE_INIT *) memory)->addr[0], 6);
		break;
	case CTL_ETHER_INQ_SUPPTYPE:
		*((char ***) argument) = &hardware[0];
		break;
	case CTL_ETHER_SET_TYPE:
		ctrl_type = argument & 7;
		if (fetch_addresses(ctrl_type) == FALSE)
			result = E_PARAMETER;
		break;
	case CTL_ETHER_GET_TYPE:
		*((int16 *) argument) = ctrl_type;
		break;
	default:
		result = E_FNAVAIL;
		break;
	}

	berr_off();

	return result;
}


static DRIVER my_driver = {
	my_set_state,
	my_cntrl,
	my_send,
	my_receive,
	"EtherNet",
	VERSION,
	(M_YEAR << 9) | (M_MONTH << 5) | M_DAY,
	"Peter Rottengatter",
	NULL, NULL
};


static void install(void)
{
	PORT *ports;
	DRIVER *driver;

	if (!get_cookie(0x5F435055L, &_cpu))	/* '_CPU' */
		_cpu = 0;

	query_chains(&ports, &driver, NULL);

	my_port.driver = &my_driver;
	my_port.driver->basepage = _BasPag;

	while (ports->next)
		ports = ports->next;

	ports->next = &my_port;

	while (driver->next)
		driver = driver->next;

	driver->next = &my_driver;

	hardware[0] = "PAMs EMega";
	hardware[1] = "PAMs VME";
	hardware[2] = "Riebl Mega";
	hardware[3] = "Riebl Mega (Mod.)";
	hardware[4] = "Riebl VME";
	hardware[5] = NULL;
}


int main(int argc, char **argv)
{
	DRV_LIST *sting_drivers;

	if (argc != 2 || strcmp(argv[1], "STinG_Load") != 0)
	{
		(void) Cconws(fault);
		return 1;
	}

	if (!get_cookie(STIK_COOKIE_MAGIC, (long *) &sting_drivers))
		return 1;

	if (sting_drivers == 0)
		return 1;

	if (strcmp(sting_drivers->magic, STIK_DRVR_MAGIC) != 0)
		return 1;

	tpl = (TPL *) (*sting_drivers->get_dftab) (TRANSPORT_DRIVER);
	stx = (STX *) (*sting_drivers->get_dftab) (MODULE_DRIVER);

	if (tpl != NULL && stx != NULL)
	{
		install();
		Ptermres(_PgmSize, 0);
	}
	
	return 1;
}
