/*********************************************************************/
/*                                                                   */
/*     STinG : TraceRoute Network Tool                               */
/*                                                                   */
/*                                                                   */
/*      Version 1.0                        from 16. Januar 1997      */
/*                                                                   */
/*********************************************************************/


#ifdef __GNUC__
#include <gem.h>
#else
#include <aes.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "transprt.h"
#include "layer.h"
#include "tracrout.h"
#include "icmp.h"


#define  UDP_PORT         65530u


typedef struct ps_hdr
{
	uint32 src_ip;
	uint32 dest_ip;
	uint8 zero;
	uint8 protocol;
	uint16 udp_len;
} PS_HDR;

typedef struct udp_pckt
{
	uint16 src_port;
	uint16 dest_port;
	uint16 length;
	uint16 chk_sum;
	uint16 ident;
} UDP_PCKT;

typedef struct pudp_hdr
{
	PS_HDR ps_hdr;
	UDP_PCKT udp_packet;
} PUDP;




TPL *tpl;
STX *stx;

static DRV_LIST *sting_drivers;
PUDP packet;
static uint16 max_ttl;
static uint16 count;
static uint16 response;
static uint16 code;
static uint16 hop_a;
static uint16 hop_b;
static uint16 hop_c;
static uint16 hop_d;
static uint32 host;
static char alert[200];


static char *rs_frstr(_WORD num)
{
	char *str = NULL;
	rsrc_gaddr(R_STRING, num, &str);
	return str;
}


static OBJECT *rs_tree(_WORD num)
{
	OBJECT *tree = NULL;
	rsrc_gaddr(R_TREE, num, &tree);
	return tree;
}




static int16 cdecl receive_echo(IP_DGRAM *datagram)
{
	UDP_PCKT *packet;
	uint8 *icmp;

	icmp = datagram->pkt_data;

	if (*icmp != ICMP_TIME_EXCEEDED && *icmp != ICMP_DEST_UNREACH)
		return FALSE;

	if (((IP_HDR *) & icmp[8])->protocol != P_UDP)
		return FALSE;

	packet = (UDP_PCKT *) (&icmp[8] + ((IP_HDR *) & icmp[8])->hd_len * 4);

	if (packet->src_port != 0 || packet->dest_port != UDP_PORT)
		return FALSE;

	response = icmp[0];
	code = icmp[1];

	hop_a = (uint16) ((datagram->hdr.ip_src >> 24) & 0xff);
	hop_b = (uint16) ((datagram->hdr.ip_src >> 16) & 0xff);
	hop_c = (uint16) ((datagram->hdr.ip_src >> 8) & 0xff);
	hop_d = (uint16) (datagram->hdr.ip_src & 0xff);

	ICMP_discard(datagram);

	return TRUE;
}


static void set_str(OBJECT *tree, _WORD obj, const char *str)
{
	TEDINFO *ted = tree[obj].ob_spec.tedinfo;
	strncpy(ted->te_ptext, str, ted->te_txtlen - 1);
}


/*
 * Check whether "cp" is a valid ascii representation
 * of an Internet address and convert to a binary address.
 * Returns 1 if the address is valid, 0 if not.
 * This replaces inet_addr, the return value from which
 * cannot distinguish between failure and a local broadcast address.
 */
static int inet_aton(const char *cp, uint32 *addr)
{
	uint32 val;
	int base;
	int n;
	unsigned char c;
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
		if (!isdigit(c))
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
			if (isascii(c) && isdigit(c))
			{
				val = (val * base) + (c - '0');
				c = *++cp;
			} else if (base == 16 && isascii(c) && isxdigit(c))
			{
				val = (val << 4) |
					(c + 10 - (islower(c) ? 'a' : 'A'));
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
	if (c != '\0' && (!isascii(c) || !isspace(c)))
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
		*addr = htonl(val);

	return 1;
}


static int fetch_parameters(void)
{
	OBJECT *tree;
	GRECT gr;
	_WORD button;

	graf_mouse(ARROW, NULL);

	rsrc_gaddr(R_TREE, TRACROUT, &tree);

	wind_update(BEG_UPDATE);

	form_center_grect(tree, &gr);
	form_dial_grect(FMD_START, &gr, &gr);

	set_str(tree, MODULE, tpl->module);
	set_str(tree, AUTHOR, tpl->author);
	set_str(tree, VERSION, tpl->version);
	set_str(tree, HOST, "");
	set_str(tree, MAX_TTL, "30");

	objc_draw_grect(tree, ROOT, MAX_DEPTH, &gr);
	button = form_do(tree, HOST);

	form_dial_grect(FMD_FINISH, &gr, &gr);

	wind_update(END_UPDATE);

	if (button != OK)
		return 0;
	max_ttl = atoi(tree[MAX_TTL].ob_spec.tedinfo->te_ptext);
	if (inet_aton(tree[HOST].ob_spec.tedinfo->te_ptext, &host) == 0)
		return 0;
	return 1;
}


static _WORD do_alert(const char *str)
{
	_WORD ret;
	_WORD message[8];
	_WORD dummy;
	
	graf_mouse(ARROW, NULL);
	ret = form_alert(1, str);
	/*
	 * let other windows redraw in multitask-AES
	 */
	evnt_multi(MU_MESAG | MU_TIMER,
		0, 0, 0,
		0, 0, 0, 0, 0,
		0, 0, 0, 0, 0,
		message,
		0,
		&dummy, &dummy, &dummy, &dummy, &dummy, &dummy);
	
	return ret;
}


static void do_some_work(void)
{
	uint16 *walk;
	uint16 *dgram;
	uint16 wait;
	int16 what;
	uint32 chksum = 0;

	if (fetch_parameters() == 0)
		return;

	if (PRTCL_get_parameters(host, &packet.ps_hdr.src_ip, NULL, NULL) != E_NORMAL)
	{
		do_alert(rs_frstr(NO_ROUTE));
		return;
	}

	packet.ps_hdr.dest_ip = host;
	packet.ps_hdr.zero = 0;
	packet.ps_hdr.protocol = P_UDP;
	packet.ps_hdr.udp_len = (int16) sizeof(UDP_PCKT);
	packet.udp_packet.src_port = 0;
	packet.udp_packet.dest_port = UDP_PORT;
	packet.udp_packet.length = (int16) sizeof(UDP_PCKT);
	packet.udp_packet.chk_sum = 0;
	packet.udp_packet.ident = 0xaffaU;

	for (walk = (uint16 *) & packet, count = 0; count < sizeof(PUDP) / 2; walk++, count++)
		chksum += *walk;

	packet.udp_packet.chk_sum = ~(uint16) ((chksum & 0xffffL) + ((chksum >> 16) & 0xffffL));

	if (!ICMP_handler(receive_echo, HNDLR_SET))
	{
		do_alert(rs_frstr(NO_HANDLER));
		return;
	}

	for (count = 1; count <= max_ttl; count++)
	{
		graf_mouse(BUSY_BEE, NULL);
		if ((dgram = KRmalloc(sizeof(UDP_PCKT))) == NULL)
		{
			do_alert(rs_frstr(NO_MEMORY));
			break;
		}
		memcpy(dgram, &packet.udp_packet, sizeof(UDP_PCKT));

		response = 0;

		what = IP_send(packet.ps_hdr.src_ip, packet.ps_hdr.dest_ip, 0, TRUE, count, P_UDP,
					   32768u + count, dgram, (int16) sizeof(UDP_PCKT), NULL, 0);

		if (what == E_NOMEM || what == E_UNREACHABLE)
		{
			do_alert(rs_frstr(PROB_SEND_PACKET));
			KRfree(dgram);
			break;
		}

		for (wait = 0; wait < 200; wait++)
		{
			evnt_timer(100);
			if (response)
				break;
		}

		switch (response)
		{
		case 0:
			if (do_alert(rs_frstr(TIMEOUT)) == 1)
			{
				count--;
				break;
			}
			count = max_ttl;
			break;
		case ICMP_TIME_EXCEEDED:
			sprintf(alert, rs_frstr(HOP_RESULT), count, hop_a, hop_b, hop_c, hop_d);
			do_alert(alert);
			break;
		case ICMP_DEST_UNREACH:
			do_alert(rs_frstr(code == ICMP_DU_PRTCL || code == ICMP_DU_PORT ? DEST_REACHED : NO_DEST));
			count = max_ttl;
			break;
		}
	}

	ICMP_handler(receive_echo, HNDLR_REMOVE);

	graf_mouse(ARROW, NULL);
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


static void gem_program(void)
{
	sting_drivers = (DRV_LIST *) Supexec(get_sting_cookie);

	if (sting_drivers == 0L)
	{
		do_alert(rs_frstr(NOT_THERE));
		return;
	}
	if (strcmp(sting_drivers->magic, STIK_DRVR_MAGIC) != 0)
	{
		do_alert(rs_frstr(CORRUPTED));
		return;
	}

	tpl = (TPL *) (*sting_drivers->get_dftab) (TRANSPORT_DRIVER);
	stx = (STX *) (*sting_drivers->get_dftab) (MODULE_DRIVER);

	if (tpl != NULL)
	{
		if (stx == NULL)
		{
			do_alert(rs_frstr(NO_STIK));
		} else
		{
			do_some_work();
		}
	} else
	{
		do_alert(rs_frstr(NO_MODULE));
	}
}


int main(void)
{
	appl_init();

	if (rsrc_load("tracrout.rsc") == 0)
		do_alert("[1][ No RSC File !  ][ Hmpf ]");
	else
		gem_program();

	appl_exit();
	return 0;
}
