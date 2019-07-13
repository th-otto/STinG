/*********************************************************************/
/*                                                                   */
/*     Low Level Port : Serielle Schnittstellen                      */
/*                                                                   */
/*                                                                   */
/*      Version 1.2                       vom 13. Januar 1997        */
/*                                                                   */
/*      Modul zur Installation und Aktivierung der Ports             */
/*                                                                   */
/*********************************************************************/


#include <tos.h>
#include <stdio.h>
#include <string.h>

#include "transprt.h"
#include "port.h"
#include <mint/sysvars.h>

#include "serial.h"


#define  M_YEAR    2000
#define  M_MONTH   8
#define  M_DAY     31
#define  M_VERSION "01.21"
#define  M_AUTHOR  "Peter Rottengatter|     &  STinG Evolution Team"


TPL *tpl;
STX *stx;

SERIAL_PORT *my_ports = NULL;
int space;
static int magx;




static PORT const init_dummy = {
	NULL,           /* name */
	L_SER_PTP,      /* type */
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

static RSVF_DEV *rsvf_head;

static MAPTAB *do_flush;
static int ck_flag;
static int scc;
static int has_drv_u;
static long cookie;
static long handle;
static char device[20] = "U:\\DEV\\";
static char const fault[] = "SERIAL.STX : STinG extension module. Only to be started by STinG !\r\n";



static long read_cookie(void)
{
	long *work;

	ck_flag = FALSE;

	if (*(long **) 0x5a0L == NULL)
		return 0;

	for (work = *(long **) 0x5a0L; *work != 0L; work += 2)
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


static void init_port(int index, const char *name, int bios, const char *gemdos, MAPTAB *handler)
{
	my_ports[index].generic = init_dummy;
	my_ports[index].generic.name = name;
	my_ports[index].bios_addr = bios;
	my_ports[index].gemdos = gemdos;
	my_ports[index].handler = handler;
	my_ports[index].vjhc = NULL;
	my_ports[index].send_buffer = NULL;
	my_ports[index].recve_buffer = NULL;

	my_ports[index].is_magx = FALSE;
	my_ports[index].iocntl = NULL;
	my_ports[index].ppp.pap_id[0] = my_ports[index].ppp.pap_passwd[0] = '\0';

	init_ppp(&my_ports[index]);
}


static void add_standard_ports(void)
{
	BCONMAP *map_ptr;
	long machine;

	map_ptr = (BCONMAP *) Bconmap(-2);

	if (!get_cookie(0x5F4D4348L, &machine))	/* '_MCH' */
		machine = 0;

	switch ((int)(machine >> 16))
	{
	case 0:
		space = 1;
		break;
	case 1:
		space = (machine & 0xffffL) == 16 ? 3 : 1;
		break;
	case 2:
		space = 4;
		break;
	case 3:
		space = 3;
		break;
	default:
		space = 1;
		break;
	}

	if ((my_ports = (SERIAL_PORT *) Malloc(space * sizeof(SERIAL_PORT))) == NULL)
		return;

	switch ((int)(machine >> 16))
	{
	case 0: /* ST */
		init_port(0, "Modem 1", 6, "MODEM1", map_ptr->maptab + 0);
		break;
	case 1: /* STE/MSTE */
		if ((machine & 0xffffL) == 16)
		{
			init_port(1, "Modem 2", 7, "MODEM2", map_ptr->maptab + 1);
			init_port(2, "Ser.2/LAN", 8, "SERIAL2", map_ptr->maptab + 2);
		}
		init_port(0, "Modem 1", 6, "MODEM1", map_ptr->maptab);
		break;
	case 2: /* TT */
		init_port(0, "Modem 1", 6, "MODEM1", map_ptr->maptab + 0);
		init_port(1, "Modem 2", 7, "MODEM2", map_ptr->maptab + 1);
		init_port(2, "Serial 1", 8, "SERIAL1", map_ptr->maptab + 2);
		init_port(3, "Ser.2/LAN", 9, "SERIAL2", map_ptr->maptab + 3);
		break;
	case 3: /* Falcon */
	case 5: /* Aranym */
		init_port(0, "Modem 1", 6, "MODEM1", map_ptr->maptab + 0);
		init_port(1, "Modem 2", 7, "MODEM2", map_ptr->maptab + 1);
		init_port(2, "LAN", 8, "LAN", map_ptr->maptab + 2);
		break;
	default:
		init_port(0, "Modem 1", 6, "MODEM1", map_ptr->maptab + 0);
		break;
	}
}


static int find_rsvf_name(int bios, SERIAL_PORT *port, int lan)
{
	RSVF_DEV *walk;

	walk = rsvf_head;

	while (walk->miscell)
	{
		if ((walk->flags & RSVF_DEVICE) == 0)
		{
			walk = walk->miscell;
			continue;
		}
		if (walk->bios != 3 && walk->bios == bios && (lan || strcmp(walk->miscell, "LAN")))
		{
			port->gemdos = walk->miscell;
			if (strcmp(walk->miscell, "MIDI") == 0)
				port->generic.name = "Midi";
			else
				port->generic.name = *port->generic.name ? port->generic.name : walk->miscell;
			if (walk->flags & RSVF_MXDDEV)
			{
				port->iocntl = (*((void ***) walk->miscell - 1))[7];
			}
			if (!(walk->flags & RSVF_BIOS))
			{
				port->is_magx = TRUE;
				if (!magx)
					return FALSE;
			}
			return TRUE;
		}
		walk++;
	}

	return FALSE;
}


static void add_rsvf_ports(void)
{
	SERIAL_PORT *temp;
	BCONMAP *map_ptr;
	MAPTAB *act_map;
	int rest;
	int bios;

	map_ptr = (BCONMAP *) Bconmap(-2);

	if (!get_cookie(0x52535646L, (long *) &rsvf_head))	/* 'RSVF' */
		rsvf_head = NULL;

	if ((rest = map_ptr->maptabsize - space) == 0 || rsvf_head == NULL)
		return;

	if ((temp = (SERIAL_PORT *) Malloc((space + rest) * sizeof(SERIAL_PORT))) == NULL)
		return;

	memcpy(temp, my_ports, space * sizeof(SERIAL_PORT));

	Mfree(my_ports);
	my_ports = temp;

	for (bios = 0; bios < space; bios++)
		find_rsvf_name(my_ports[bios].bios_addr, &my_ports[bios], TRUE);

	for (act_map = map_ptr->maptab + space, bios = 6 + space; rest > 0; rest--, bios++)
	{
		init_port(space, "", bios, "", act_map++);
		if (find_rsvf_name(bios, &my_ports[space], FALSE))
			space++;
	}
}


static long do_Fopen(void)
{
	OSHEADER *oshdr = *(OSHEADER **) 0x4f2L;
	BASPAG **process;
	BASPAG *old;

	if (oshdr->os_version >= 0x0102)
		process = (BASPAG **)oshdr->p_run;
	else
		process = (BASPAG **) ((oshdr->os_conf >> 1) == 4 ? 0x873cL : 0x602cL);

	old = *process;
	*process = _BasPag;

	handle = Fopen(device, FO_RW | O_NDELAY);

	*process = old;

	return 0;
}


static long do_Fclose(void)
{
	OSHEADER *oshdr = *(OSHEADER **) 0x4f2L;
	BASPAG **process;
	BASPAG *old;

	if (oshdr->os_version >= 0x0102)
		process = (BASPAG **)oshdr->p_run;
	else
		process = (BASPAG **) ((oshdr->os_conf >> 1) == 4 ? 0x873cL : 0x602cL);

	old = *process;
	*process = _BasPag;

	Fclose((int) handle);

	*process = old;

	return 0;
}


static void deplete_queue(IP_DGRAM **queue)
{
	IP_DGRAM *walk;
	IP_DGRAM *next;

	for (walk = *queue; walk; walk = next)
	{
		next = walk->next;
		IP_discard(walk, TRUE);
	}

	*queue = NULL;
}


#pragma GCC diagnostic ignored "-Wcast-function-type"

static long flush(void)
{
	while (execute((short cdecl(*)(short)) do_flush->Bconstat) != 0)
		execute((short cdecl(*)(short)) do_flush->Bconin);

	return 0;
}


static int16 cdecl my_set_state(PORT *port, int16 state)
{
	SERIAL_PORT *serial;

	if (port->driver != &my_driver)
		return FALSE;

	serial = (SERIAL_PORT *) port;
	port->flags &= ~(FLG_DONE | FLG_SUCCESS);

	if (state)
	{
		if (serial->send_buffer == NULL)
			if ((serial->send_buffer = KRmalloc(8192L)) == NULL)
				return FALSE;
		if (serial->recve_buffer == NULL)
			if ((serial->recve_buffer = KRmalloc(8192L)) == NULL)
			{
				KRfree(serial->send_buffer);
				serial->send_buffer = NULL;
				return FALSE;
			}
		if (port->flags & FLG_VJHC)
		{
			if (serial->vjhc == NULL)
				if ((serial->vjhc = KRmalloc(sizeof(VJHC))) == NULL)
				{
					KRfree(serial->send_buffer);
					serial->send_buffer = NULL;
					KRfree(serial->recve_buffer);
					serial->recve_buffer = NULL;
					return FALSE;
				}
			init_vjhc(serial->vjhc, MAX_STATES);
		}
		if ((port->flags & FLG_PRTCL) != 0)
		{
			if (!open_ppp(serial))
			{
				KRfree(serial->vjhc);
				serial->vjhc = NULL;
				KRfree(serial->send_buffer);
				serial->send_buffer = NULL;
				KRfree(serial->recve_buffer);
				serial->recve_buffer = NULL;
				return FALSE;
			}
		} else
		{
			port->flags |= FLG_DONE | FLG_SUCCESS;
		}
		if (strcmp(port->name, "Ser.2/LAN") == 0)
		{
			if (has_drv_u)
			{
				serial->gemdos = (port->flags & FLG_LANBIT) ? "LAN" : "SERIAL2";
			} else
			{
				scc = Giaccess(0, 14);
				(port->flags & FLG_LANBIT) ? Offgibit(0x7f) : Ongibit(0x80);
			}
		}
		if (has_drv_u)
		{
			strcpy(&device[7], serial->gemdos);
			Supexec(do_Fopen);
			if (handle < 0)
				return FALSE;
			serial->handle = (int) handle;
		}
		serial->send_length = serial->send_index = 0;
		serial->recve_length = serial->recve_index = 0;
	} else
	{
		if ((port->flags & FLG_PRTCL) != 0)
			close_ppp(serial);
		if (!has_drv_u)
		{
			if (strcmp(port->name, "Ser.2/LAN") == 0)
				(scc & 0x80) ? Ongibit(0x80) : Offgibit(0x7f);
		} else
		{
			handle = serial->handle;
			Supexec(do_Fclose);
		}
		if (serial->send_buffer)
		{
			KRfree(serial->send_buffer);
			serial->send_buffer = NULL;
		}
		if (serial->recve_buffer)
		{
			KRfree(serial->recve_buffer);
			serial->recve_buffer = NULL;
		}
		if (serial->vjhc)
		{
			KRfree(serial->vjhc);
			serial->vjhc = NULL;
		}
		deplete_queue(&port->send);
		deplete_queue(&port->receive);
	}

	do_flush = serial->handler;
	Supexec(flush);

	return TRUE;
}


static int16 cdecl my_cntrl(PORT *port, uint32 argument, int16 code)
{
	SERIAL_PORT *serial;
	int16 status;
	int16 result = E_FNAVAIL;

	serial = (SERIAL_PORT *) port;
	status = ((port->flags & FLG_PRTCL) && port->active) ? TRUE : FALSE;

	switch (code)
	{
	case CTL_SERIAL_SET_PRTCL:
		if ((argument & FLG_LANBIT) && strcmp(port->name, "Ser.2/LAN"))
			return E_PARAMETER;
		if (argument & ~(FLG_PRTCL | FLG_VJHC | FLG_LANBIT | FLG_DNS))
			return E_PARAMETER;
		port->flags = (port->flags & ~(FLG_PRTCL | FLG_VJHC | FLG_LANBIT | FLG_DNS)) | argument;
		result = E_NORMAL;
		break;

	case CTL_SERIAL_GET_PRTCL:
		*(uint16 *) argument = (uint16) port->flags & (FLG_PRTCL | FLG_VJHC | FLG_LANBIT | FLG_DNS);
		result = E_NORMAL;
		break;

	case CTL_SERIAL_SET_LOGGING:
		if (*(long *) argument)
		{
			serial->log_buffer = *(char **) argument;
			serial->log_len = (uint16) ((uint32 *) argument)[1];
			serial->log_ptr = 0;
			port->flags |= FLG_LOGGING;
		} else
		{
			*(uint32 *) argument = serial->log_ptr;
			port->flags &= ~FLG_LOGGING;
		}
		result = E_NORMAL;
		break;

	case CTL_SERIAL_SET_AUTH:
		if (argument)
		{
#if 0
			serial->ppp.pap_ack = ((char **) argument)[0];
			serial->ppp.pap_nak = ((char **) argument)[1];
			serial->ppp.pap_auth = &((char **) argument)[2];
#else
			serial->ppp.pap_auth = &((char **) argument)[0];
#endif
			port->flags |= FLG_REQU_AUTH;
		} else
		{
#if 0
			serial->ppp.pap_ack = serial->ppp.pap_nak = NULL;
#endif
			serial->ppp.pap_auth = NULL;
			port->flags &= ~FLG_REQU_AUTH;
		}
		result = E_NORMAL;
		break;

	case CTL_SERIAL_SET_PAP:
		if (((char **) argument)[0] == NULL || ((char **) argument)[1] == NULL)
		{
			port->flags &= ~FLG_ALLOW_PAP;
		} else
		{
			if (strlen(((char **) argument)[0]) > 127 || strlen(((char **) argument)[1]) > 127)
				return E_BIGBUF;
			strcpy(&serial->ppp.pap_id[0], ((char **) argument)[0]);
			strcpy(&serial->ppp.pap_passwd[0], ((char **) argument)[1]);
			port->flags |= FLG_ALLOW_PAP;
		}
		result = E_NORMAL;
		break;

	case CTL_SERIAL_INQ_STATE:
		if (*((int16 *) argument) == 0)
		{
			if ((port->flags & FLG_DONE) != 0)
				*((int16 *) argument) = (port->flags & FLG_SUCCESS) ? 1 : -1;
			else
				*((int16 *) argument) = 0;
		} else
		{
			/* return PPP status */ ;
		}
		result = E_NORMAL;
		break;

	case CTL_GENERIC_SET_MTU:
		if (argument < 68 || argument > port->max_mtu)
			return E_PARAMETER;
		if (status)
		{
			close_ppp(serial);
			port->mtu = (uint16) argument;
			open_ppp(serial);
		} else
		{
			port->mtu = (uint16) argument;
		}
		result = E_NORMAL;
		break;

	case CTL_GENERIC_SET_IP:
		if (status)
		{
			close_ipcp(serial);
			port->ip_addr = argument;
			open_ipcp(serial);
		} else
		{
			port->ip_addr = argument;
		}
		result = E_NORMAL;
		break;
	}

	return result;
}


DRIVER my_driver = {
	my_set_state,
	my_cntrl,
	my_send,
	my_receive,
	"Serial",
	M_VERSION,
	((M_YEAR - 1980) << 9) | (M_MONTH << 5) | M_DAY,
	M_AUTHOR,
	NULL,
	NULL
};

static int install(void)
{
	PORT *ports;
	DRIVER *driver;
	int count;
	long value;

	magx = get_cookie(0x4d616758L, &value); /* 'MagX' */
	
	if (Bconmap(0) != 0)
		return FALSE;

	add_standard_ports();
	add_rsvf_ports();

	if (my_ports == NULL)
		return FALSE;

	if (!TIMER_call(ppp_timer, HNDLR_SET))
		return FALSE;

	has_drv_u = (Drvmap() & 0x00100000L) != 0;

	query_chains(&ports, &driver, NULL);

	my_driver.basepage = _BasPag;

	while (ports->next)
		ports = ports->next;

	for (count = 0; count < space; count++)
	{
		if (count)
			my_ports[count - 1].generic.next = &my_ports[count].generic;
		my_ports[count].generic.next = NULL;
		my_ports[count].generic.driver = &my_driver;
	}
	ports->next = &my_ports[0].generic;

	while (driver->next)
		driver = driver->next;

	driver->next = &my_driver;

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

	if (!get_cookie(STIK_COOKIE_MAGIC, (long *) &sting_drivers))
		return 1;

	if (sting_drivers == NULL)
		return 1;

	if (strcmp(sting_drivers->magic, STIK_DRVR_MAGIC) != 0)
		return 1;

	tpl = (TPL *) (*sting_drivers->get_dftab) (TRANSPORT_DRIVER);
	stx = (STX *) (*sting_drivers->get_dftab) (MODULE_DRIVER);

	if (tpl != NULL && stx != NULL)
	{
		if (install())
		{
			Ptermres(_PgmSize, 0);
		} else
		{
			if (my_ports)
				Mfree(my_ports);
		}
	}
	
	return 1;
}
