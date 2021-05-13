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


#include <stdio.h>
#include <string.h>

#include "transprt.h"
#include "port.h"

#include "nfeth.h"
#include <mint/arch/nf_ops.h>
#include "nfethapi.h"

#define  M_YEAR    2021
#define  M_MONTH   5
#define  M_DAY     9
#define  M_VERSION "00.10"


#ifdef __GNUC__
#define _BasPag _base
extern unsigned long _PgmSize;
#endif

#ifndef GNU_ASM_NAME
#ifdef __GNUC__
#define GNU_ASM_NAME(x) __asm__(x)
#else
#define GNU_ASM_NAME(x)
#endif
#endif


TPL *tpl;
STX *stx;

static MYPORT my_port[MAX_ETH] = {
	{
		{
		"eth0",         /* name */
		L_SER_BUS,      /* type */
		FALSE,          /* active */
		0,              /* flags */
		0,              /* ip_addr */
		0xffffffffUL,   /* sub_mask */
		1500,           /* mtu */
		1500,           /* max_mtu */
		0,              /* stat_sd_data */
		NULL,           /* send */
		0,              /* stat_rcv_data */
		NULL,           /* receive */
		0,              /* stat_dropped */
		NULL,           /* driver */
		NULL            /* next */
		},
		0,
		{ 0, 0, 0, 0, 0, 0 },
		0
	},
	{
		{
		"eth1",         /* name */
		L_SER_BUS,      /* type */
		FALSE,          /* active */
		0,              /* flags */
		0,              /* ip_addr */
		0xffffffffUL,   /* sub_mask */
		1500,           /* mtu */
		1500,           /* max_mtu */
		0,              /* stat_sd_data */
		NULL,           /* send */
		0,              /* stat_rcv_data */
		NULL,           /* receive */
		0,              /* stat_dropped */
		NULL,           /* driver */
		NULL            /* next */
		},
		1,
		{ 0, 0, 0, 0, 0, 0 },
		0
	},
	{
		{
		"eth2",         /* name */
		L_SER_BUS,      /* type */
		FALSE,          /* active */
		0,              /* flags */
		0,              /* ip_addr */
		0xffffffffUL,   /* sub_mask */
		1500,           /* mtu */
		1500,           /* max_mtu */
		0,              /* stat_sd_data */
		NULL,           /* send */
		0,              /* stat_rcv_data */
		NULL,           /* receive */
		0,              /* stat_dropped */
		NULL,           /* driver */
		NULL            /* next */
		},
		2,
		{ 0, 0, 0, 0, 0, 0 },
		0
	},
	{
		{
		"eth3",         /* name */
		L_SER_BUS,      /* type */
		FALSE,          /* active */
		0,              /* flags */
		0,              /* ip_addr */
		0xffffffffUL,   /* sub_mask */
		1500,           /* mtu */
		1500,           /* max_mtu */
		0,              /* stat_sd_data */
		NULL,           /* send */
		0,              /* stat_rcv_data */
		NULL,           /* receive */
		0,              /* stat_dropped */
		NULL,           /* driver */
		NULL            /* next */
		},
		3,
		{ 0, 0, 0, 0, 0, 0 },
		0
	},
};

#define IS_MY_PORT(p) ((p) >= &my_port[0] && (p) < &my_port[MAX_ETH])

/* old handler */
extern void *old_interrupt GNU_ASM_NAME("old_interrupt");
/* interrupt wrapper routine */
void my_interrupt (void) GNU_ASM_NAME("my_interrupt");

/* the C routine handling the interrupt */
void cdecl nfeth_interrupt(void) GNU_ASM_NAME("nfeth_interrupt");



#define BUFFER_SIZE 1536L

static char *hardware[] = { "Emulator", NULL };
static int16 ctrl_type = 0;


/* ================================================================ */
static unsigned long nfEtherID;
static long cdecl (*nf_call)(long id, ...);

static long get_nfapi_version(void)
{
	return nf_call(ETH(GET_VERSION));
}

static unsigned long get_int_level(void)
{
	return nf_call(ETH(XIF_INTLEVEL));
}

static unsigned long get_hw_addr(int ethX, uint8 *buffer, int len)
{
	return nf_call(ETH(XIF_GET_MAC), (unsigned long)ethX, buffer, (unsigned long)len);
}

static unsigned long nfInterrupt(short bit_mask)
{
	return nf_call(ETH(XIF_IRQ), (unsigned long)bit_mask);
}

static short read_packet_len(int ethX)
{
	return nf_call(ETH(XIF_READLENGTH), (unsigned long)ethX);
}

static void read_block(int ethX, void *cp, short len)
{
	nf_call(ETH(XIF_READBLOCK), (unsigned long)ethX, cp, (unsigned long)len);
}

void send_block(int ethX, void *cp, short len)
{
	nf_call(ETH(XIF_WRITEBLOCK), (unsigned long)ethX, cp, (unsigned long)len);
}

/* ================================================================ */


static long get_jar(void)
{
	return *((long *)0x5a0L);
}


static int get_cookie(long which, long *value)
{
	long *work;
	
	*value = 0;
	work = (long *)Supexec(get_jar);
	if (work)
	{
		for (; *work != 0; work += 2)
			if (*work == which)
			{
				*value = *++work;
				return TRUE;
			}
	}
	return FALSE;
}


static void cdecl my_send(PORT *_port)
{
	MYPORT *port = (MYPORT *)_port;
	IP_DGRAM *dgram;
	int16 length;
	static uint8 buffer[BUFFER_SIZE];

	if (!IS_MY_PORT(port) || port->generic.active == 0)
		return;

	for (;;)
	{
		if (fetch_dgram(port, &dgram) == FALSE)
			return;
		if ((length = xmit_dgram(port, dgram, buffer)) != 0)
			send_block(port->ethX, buffer, length);
	}
}


static void cdecl my_receive(PORT *port)
{
	(void)port;
	/* nothing to do here, since the driver is interrupt-driven */
}


static void recv_packet(MYPORT *port)
{
	short pktlen;
	static uint8 buffer[BUFFER_SIZE];

	if (!IS_MY_PORT(port) || port->generic.active == 0)
		return;

	/* read packet length (excluding 32 bit crc) */
	pktlen = read_packet_len(port->ethX);
	if (pktlen <= 0 || pktlen > BUFFER_SIZE)
	{
		return;
	}
	read_block(port->ethX, buffer, pktlen);
	/* and enqueue packet */
	recve_dgram(port, buffer, pktlen);
}


/*
 * interrupt routine
 */
void cdecl nfeth_interrupt (void)
{
	static int in_use = 0;
	int ethX;

	if (in_use)
		return; /* primitive locking */
	in_use++;

	for (ethX = 0; ethX < MAX_ETH; ethX++)
	{
		int this_dev_irq_bit = 1 << ethX;
		int irq_for_eth_bitmask = (int)nfInterrupt(0);
		if (this_dev_irq_bit & irq_for_eth_bitmask)
		{
			recv_packet(&my_port[ethX]);
			nfInterrupt(this_dev_irq_bit);
		}
	}
	in_use = 0;
}


static MYPORT *sup_port;
static long stop_driver(void)
{
	MYPORT *port = sup_port;

	nf_call(ETH(XIF_STOP), (unsigned long)port->ethX);
	
	return 0;
}


static long start_driver(void)
{
	MYPORT *port = sup_port;

	if (nf_call(ETH(XIF_START), (unsigned long)port->ethX) != 0)
		return FALSE;

	return TRUE;
}


static int16 cdecl my_set_state(PORT *_port, int16 state)
{
	MYPORT *port = (MYPORT *)_port;

	if (!IS_MY_PORT(port))
		return FALSE;

	sup_port = port;
	if (state)
	{
		if (Supexec(start_driver) == FALSE)
			return FALSE;
		arp_init();
	} else
	{
		Supexec(stop_driver);
		deplete_queue(&port->generic.send);
		deplete_queue(&port->generic.receive);
	}

	return TRUE;
}


/*
 * Check whether "cp" is a valid ascii representation
 * of an Internet address and convert to a binary address.
 * Returns 1 if the address is valid, 0 if not.
 * This replaces inet_addr, the return value from which
 * cannot distinguish between failure and a local broadcast address.
 */

static unsigned long inet_aton(const char *cp, uint32 *addr)
{
	unsigned long val;
	int base;
	int n;
	char c;
	unsigned long parts[4];
	unsigned long *pp = parts;

	c = *cp;
	for (;;)
	{
		/*
		 * Collect number up to ``.''.
		 * Values are specified as for C:
		 * 0x=hex, 0=octal, isdigit=decimal.
		 */
		if (!(c >= '0' && c <= '9'))
			return 0;
		base = 10;
		if (c == '0')
		{
			c = *++cp;
			if (c == 'x' || c == 'X')
				base = 16, c = *++cp;
			else
				base = 8;
		}
		val = 0;
		for (;;)
		{
			if (c >= 'A' && c <= 'F')
				c += 'a' - 'A';
			if (c >= '0' && c <= '9')
			{
				val = (val * base) + (c - '0');
				c = *++cp;
			} else if (base == 16 && c >= 'a' && c <= 'f')
			{
				val = (val << 4) | (c + 10 - 'a');
				c = *++cp;
			} else
				break;
		}
		if (c == '.')
		{
			/*
			 * Internet format:
			 *	a.b.c.d
			 *	a.b.c	(with c treated as 16 bits)
			 *	a.b	(with b treated as 24 bits)
			 */
			if (pp >= parts + 3)
				return 0;
			*pp++ = val;
			c = *++cp;
		} else
			break;
	}
	/*
	 * Check for trailing characters.
	 */
	if (c != '\0' && !(c == ' ' || c == '\t' || c == '\n' || c == '\r'))
		return 0;

	/*
	 * Concoct the address according to
	 * the number of parts specified.
	 */
	n = (int)(pp - parts + 1);
	switch (n)
	{
	case 0:
		return 0;		/* initial nondigit */

	case 1:				/* a -- 32 bits */
		break;

	case 2:				/* a.b -- 8.24 bits */
		if (parts[0] > 0xff || val > 0xffffffUL)
			return 0;
		val |= parts[0] << 24;
		break;

	case 3:				/* a.b.c -- 8.8.16 bits */
		if (parts[0] > 0xff || parts[1] > 0xff || val > 0xffffUL)
			return 0;
		val |= (parts[0] << 24) | (parts[1] << 16);
		break;

	case 4:				/* a.b.c.d -- 8.8.8.8 bits */
		if (parts[0] > 0xff || parts[1] > 0xff || parts[2] > 0xff || val > 0xff)
			return 0;
		val |= (parts[0] << 24) | (parts[1] << 16) | (parts[2] << 8);
		break;
	}
	if (addr)
		*addr = val;

	return 1;
}


static int16 cdecl my_cntrl(PORT *_port, uint32 argument, int16 code)
{
	MYPORT *port = (MYPORT *)_port;
	int16 result = E_NORMAL;
	uint32 *data;

	if (!IS_MY_PORT(port))
		return E_PARAMETER;

	switch (code)
	{
	case CTL_ETHER_SET_MAC:
		/* MAC cannot be changed to something different than configured in emulator */
		if (memcmp(port->address, (void *) argument, ETH_ALEN) != 0)
			result = E_LOCKED;
		break;
	case CTL_ETHER_GET_MAC:
		memcpy((void *) argument, port->address, ETH_ALEN);
		break;
	case CTL_ETHER_INQ_SUPPTYPE:
		*((char ***) argument) = hardware;
		break;
	case CTL_ETHER_SET_TYPE:
		ctrl_type = argument & 7;
		if (ctrl_type != 0)
			result = E_PARAMETER;
		break;
	case CTL_ETHER_GET_TYPE:
		*((int16 *) argument) = ctrl_type;
		break;
	case CTL_ETHER_GET_STAT:
		data = (uint32 *)argument;
		*data++ = port->generic.ip_addr;
		*data++ = port->ip_host;
		*data++ = port->generic.sub_mask;
		break;
	case CTL_GENERIC_SET_IP:
		/* IP cannot be changed to something different than configured in emulator */
		if (argument != port->generic.ip_addr)
			result = E_LOCKED;
		break;
	case CTL_GENERIC_SET_MASK:
		/* netmask cannot be changed to something different than configured in emulator */
		if (argument != port->generic.sub_mask)
			result = E_LOCKED;
		break;
	default:
		result = E_FNAVAIL;
		break;
	}

	return result;
}


static DRIVER my_driver = {
	my_set_state,
	my_cntrl,
	my_send,
	my_receive,
	"NFEther",
	M_VERSION,
	((M_YEAR - 1980) << 9) | (M_MONTH << 5) | M_DAY,
	"Thorsten Otto",
	NULL, NULL
};


static void failure(const char *reason)
{
	(void) Cconws("\r\nNFETH.STX : ");
	(void) Cconws(reason);
	(void) Cconws("\r\n");
}


static int install(void)
{
	MYPORT *port;
	PORT *ports;
	DRIVER *driver;
	char buffer[128];
	int ethX;
	int anyfound;
	
	query_chains(&ports, &driver, NULL);

	/*
	 * first check for any configured driver.
	 * We must not link anything to the driver or port lists
	 * if we are going to terminate
	 */
	anyfound = 0;
	for (ethX = 0, port = my_port; ethX < MAX_ETH; ethX++, port++)
	{
		port->ethX = ethX;
		if (!get_hw_addr(port->ethX, port->address, ETH_ALEN))
		{
			port->generic.ip_addr = 0;
		} else
		{
			anyfound++;
			nf_call(ETH(XIF_GET_IPATARI), (unsigned long)port->ethX, buffer, sizeof(buffer));
			inet_aton(buffer, &port->generic.ip_addr);
			nf_call(ETH(XIF_GET_NETMASK), (unsigned long)port->ethX, buffer, sizeof(buffer));
			inet_aton(buffer, &port->generic.sub_mask);
			nf_call(ETH(XIF_GET_IPHOST), (unsigned long)port->ethX, buffer, sizeof(buffer));
			inet_aton(buffer, &port->ip_host);
		}
	}
	if (anyfound == 0)
	{
		failure("no drivers configured");
		return FALSE;
	}
	
	/*
	 * now install the ports
	 */
	while (ports->next)
	{
		ports = ports->next;
	}
	for (ethX = 0, port = my_port; ethX < MAX_ETH; ethX++, port++)
	{
		if (port->generic.ip_addr != 0)
		{
			ports->next = &port->generic;
			port->generic.driver = &my_driver;
			ports = ports->next;
		}
	}
	
	/*
	 * install the driver
	 */
	while (driver->next)
	{
		driver = driver->next;
	}
	driver->next = &my_driver;
	my_driver.basepage = _BasPag;

	/*
	 * Install the interface interrupt.
	 */
	old_interrupt = (void *)Setexc(0x60 / 4 + get_int_level(), my_interrupt);

	return TRUE;
}


int main(void)
{
	DRV_LIST *sting_drivers;
	struct nf_ops *nf_ops;

	if (strcmp(_BasPag->p_cmdlin, "\012STinG_Load") != 0)
	{
		failure("STinG extension module. Only to be started by STinG");
		return 1;
	}

	/* get the Ethernet NatFeat ID */
	nfEtherID = 0;
	nf_ops = nf_init();
	if (nf_ops)
		nfEtherID = nf_ops->get_id("ETHERNET");

	if (nfEtherID == 0)
	{
		failure("not installed - NatFeat not found");
		return 1;
	}
	/* safe the nf_call pointer */
	nf_call = nf_ops->call;

	/* compare the version */
	if (Supexec(get_nfapi_version) != NFETH_NFAPI_VERSION )
	{
		failure("not installed - version mismatch");
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
		if (install())
		{
			Ptermres(_PgmSize, 0);
		}
	}

	return 1;
}
