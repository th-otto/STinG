/*********************************************************************/
/*                                                                   */
/*     Low Level Port : MidiNet Network Interface                    */
/*                                                                   */
/*      Version 0.1                      vom 28. November 1996       */
/*                                                                   */
/*********************************************************************/


#include <stdio.h>
#include <string.h>

#include "transprt.h"
#include "port.h"

#ifdef __GNUC__
extern unsigned long _PgmSize;
#endif

#define  M_YEAR    1998
#define  M_MONTH   6
#define  M_DAY     18
#define  M_VERSION "00.01"

TPL *tpl;
STX *stx;

#ifndef UNUSED
# define UNUSED(x) ((void)(x))
#endif

#ifdef __GNUC__
#define _BasPag _base
#endif


static PORT my_port = {
	"MidiNet",      /* name */
	L_SER_RING,     /* type */
	FALSE,          /* active */
	0,              /* flags */
	0xffffffffUL,   /* ip_addr */
	0xffffffffUL,   /* sub_mask */
	4096,           /* mtu */
	4096,           /* max_mtu */
	0,              /* stat_sd_data */
	NULL,           /* send */
	0,              /* stat_rcv_data */
	NULL,           /* receive */
	0,              /* stat_dropped */
	NULL,           /* driver */
	NULL            /* next */
};

static char const fault[] = "MIDI.STX : STinG extension module. Only to be started by STinG !\r\n";



static void cdecl my_send(PORT *port)
{
	if (port != &my_port)
		return;
}


static void cdecl my_receive(PORT *port)
{
	if (port != &my_port)
		return;
}


static int16 cdecl my_set_state(PORT *port, int16 state)
{
	UNUSED(state);
	if (port != &my_port)
		return FALSE;

	return TRUE;
}


static int16 cdecl my_cntrl(PORT *port, uint32 argument, int16 code)
{
	UNUSED(port);
	UNUSED(argument);
	UNUSED(code);
	return E_FNAVAIL;
}


static DRIVER my_driver = {
	my_set_state,
	my_cntrl,
	my_send,
	my_receive,
	"MidiNet",
	M_VERSION,
	((M_YEAR - 1980) << 9) | (M_MONTH << 5) | M_DAY, "Unknown Programmer",
	NULL, NULL
};


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


static int16 install(void)
{
	PORT *ports;
	DRIVER *driver;

	query_chains(&ports, &driver, NULL);

	my_port.driver = &my_driver;
	my_driver.basepage = _BasPag;

	while (ports->next)
		ports = ports->next;

	ports->next = &my_port;

	while (driver->next)
		driver = driver->next;

	driver->next = &my_driver;

	return TRUE;
}


int main(void)
{
	DRV_LIST *sting_drivers;

	if (strcmp(_BasPag->p_cmdlin, "\012STinG_Load") != 0)
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
