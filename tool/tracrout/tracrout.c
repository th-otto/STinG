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
#include <time.h>

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
} UDP_PCKT;

typedef struct pudp_hdr
{
	PS_HDR ps_hdr;
	UDP_PCKT udp_packet;
} PUDP;




STX *stx;
TPL *tpl;

static DRV_LIST *sting_drivers;
PUDP packet;
static uint16 max_ttl;
static uint16 waittime;
static uint16 count;
static int16 response;
static uint16 code;
static uint16 hop_a;
static uint16 hop_b;
static uint16 hop_c;
static uint16 hop_d;
static uint16 ttl;
static uint32 host;
static char alert[200];
static GRECT gr;
static OBJECT *main_dialog;
#define NUM_INFOS (INFO_4 - INFO_1 + 1)
static char info_buf[NUM_INFOS][200];
static _WORD next_info;
static volatile int info_dirty;
static int32 now;
static int32 elapsed;


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




/*
 * next 2 functions are from StinG only,
 * and are not supported by STiK/Gluestik
 */
static int32 safe_TIMER_now(void)
{
	if (stx)
		return TIMER_now();
	return clock() * 5;
}


static int32 safe_TIMER_elapsed(int32 then)
{
	if (stx)
		return TIMER_elapsed(then);
	return (clock() * 5) - then;
}


static int16 cdecl receive_echo(IP_DGRAM *datagram)
{
	UDP_PCKT *packet;
	uint8 *icmp;
	IP_HDR *ip;
	int16 pkt_length;
	int16 hlen;

	icmp = datagram->pkt_data;
	pkt_length = datagram->pkt_length;
	
	if (pkt_length < ICMP_MINLEN)
		return FALSE;
	
	if ((icmp[0] == ICMP_TIMXCEED /* && icmp[1] == ICMP_TIMXCEED_INTRANS */) ||
		icmp[0] == ICMP_UNREACH ||
		icmp[0] == ICMP_ECHOREPLY)
	{
		ip = (IP_HDR *) &icmp[ICMP_MINLEN];
		pkt_length -= ICMP_MINLEN;
		if (pkt_length < 20 || ip->protocol != P_UDP)
			return FALSE;
	
		hlen = ip->hd_len * 4;
		packet = (UDP_PCKT *) ((char *)ip + hlen);
		if (pkt_length < hlen + (int)sizeof(*packet))
			return FALSE;
		if (packet->src_port != 0 || packet->dest_port != UDP_PORT)
			return FALSE;
	
		response = icmp[0];
		code = icmp[1];
	
		hop_a = (uint16) ((datagram->hdr.ip_src >> 24) & 0xff);
		hop_b = (uint16) ((datagram->hdr.ip_src >> 16) & 0xff);
		hop_c = (uint16) ((datagram->hdr.ip_src >> 8) & 0xff);
		hop_d = (uint16) (datagram->hdr.ip_src & 0xff);
		ttl = datagram->hdr.ttl;
		elapsed = safe_TIMER_elapsed(now);

		ICMP_discard(datagram);
	
		return TRUE;
	}
	return FALSE;
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


/*
 * Checksum routine for Internet Protocol family headers (C Version)
 */
static uint16 in_cksum(uint16 *addr, long len)
{
	long nleft = len;
	const uint16 *w = addr;
	uint16 answer;
	uint32 sum = 0;

	/*
	 *  Our algorithm is simple, using a 32 bit accumulator (sum),
	 *  we add sequential 16 bit words to it, and at the end, fold
	 *  back all the carry bits from the top 16 bits into the lower
	 *  16 bits.
	 */
	while (nleft > 1)
	{
		sum += *w++;
		nleft -= 2;
	}

	/* mop up an odd byte, if necessary */
	if (nleft == 1)
		sum += htons(*(const uint8 *) w << 8);

	/*
	 * add back carry outs from top 16 bits to low 16 bits
	 */
	sum = (sum >> 16) + (sum & 0xffff);	/* add hi 16 to low 16 */
	sum += (sum >> 16);					/* add carry */
	answer = sum;						/* truncate to 16 bits */
	return ~answer;
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


static char *next_info_buf(void)
{
	int i;
	
	if (next_info == (NUM_INFOS - 1))
	{
		for (i = 1; i < NUM_INFOS; i++)
			strcpy(info_buf[i - 1], info_buf[i]);
	} else
	{
		next_info++;
	}
	info_dirty++;
	return info_buf[next_info];
}


static int dialog_aborted(void)
{
	_WORD events;
	_WORD mox, moy, key, dummy;
	_WORD msg[8];
	
	events = evnt_multi(MU_TIMER | MU_BUTTON | MU_KEYBD,
		1, 1, 1,
		0, 0, 0, 0, 0,
		0, 0, 0, 0, 0,
		msg,
		100,
		&mox, &moy, &dummy, &dummy, &key, &dummy);
	if (events & MU_BUTTON)
	{
		if (objc_find(main_dialog, ROOT, MAX_DEPTH, mox, moy) == CANCEL)
		{
			main_dialog[CANCEL].ob_state |= OS_SELECTED;
			objc_draw_grect(main_dialog, CANCEL, MAX_DEPTH, &gr);
			evnt_button(1, 1, 0, &mox, &moy, &dummy, &dummy);
			main_dialog[CANCEL].ob_state &= ~OS_SELECTED;
			objc_draw_grect(main_dialog, CANCEL, MAX_DEPTH, &gr);
			return TRUE;
		}
	}
	if (events & MU_KEYBD)
	{
		if ((key & 0xff) == 0x1b) /* Esc */
		{
			/*
			 * at least XaAES has problems to react to the key,
			 * so you typically have to keep it pressed, until it is
			 * recognized. Swallow any keys that may been generated
			 * by key-repeat in the meantime.
			 */
			do
			{
				events = evnt_multi(MU_TIMER | MU_KEYBD,
					1, 1, 1,
					0, 0, 0, 0, 0,
					0, 0, 0, 0, 0,
					msg,
					0,
					&mox, &moy, &dummy, &dummy, &key, &dummy);
			} while (events & MU_KEYBD);
			return TRUE;
		}
	}
	return FALSE;
}


static void do_traceroute_dialog(void)
{
	OBJECT *tree;
	_WORD button;
	uint16 *dgram;
	uint16 wait;
	int16 what;
	int i;
	int got_there;
	int unreachable;
	int dots, maxdots;
	
	tree = rs_tree(TRACROUT);
	main_dialog = tree;

	if (!ICMP_handler(receive_echo, HNDLR_SET))
	{
		do_alert(rs_frstr(NO_HANDLER));
		return;
	}

	set_str(tree, MODULE, tpl->module);
	set_str(tree, AUTHOR, tpl->author);
	set_str(tree, VERSION, tpl->version);
	set_str(tree, HOST, "");
	set_str(tree, MAX_TTL, "30");
	set_str(tree, WAITTIME, "5");

	wind_update(BEG_UPDATE);

	form_center_grect(tree, &gr);
	form_dial_grect(FMD_START, &gr, &gr);

	for (i = 0; i < NUM_INFOS; i++)
		tree[INFO_1 + i].ob_spec.free_string = info_buf[i];
	next_info = -1;
	maxdots = (int)strlen(tree[DOTS].ob_spec.free_string);
	*tree[DOTS].ob_spec.free_string = '\0';
	
	for (;;)
	{
		*tree[DOTS].ob_spec.free_string = '\0';
		objc_draw_grect(tree, ROOT, MAX_DEPTH, &gr);
		graf_mouse(ARROW, NULL);
		button = form_do(tree, HOST);
		tree[button].ob_state &= ~OS_SELECTED;
		if (button == CANCEL)
			break;
		if (button != START)
			continue;

		if (inet_aton(tree[HOST].ob_spec.tedinfo->te_ptext, &host) == 0)
		{
			graf_mouse(BUSY_BEE, NULL);
			if (resolve(tree[HOST].ob_spec.tedinfo->te_ptext, NULL, &host, 1) <= 0)
			{
				do_alert(rs_frstr(UNKNOWN_HOST));
				continue;
			}
		}
		max_ttl = atoi(tree[MAX_TTL].ob_spec.tedinfo->te_ptext);
		waittime = atoi(tree[WAITTIME].ob_spec.tedinfo->te_ptext);
		/* * 10 because we wait for 100 ms in the event loop below */
		waittime *= 10;

		if (stx && PRTCL_get_parameters(host, &packet.ps_hdr.src_ip, NULL, NULL) != E_NORMAL)
		{
			do_alert(rs_frstr(NO_ROUTE));
			continue;
		}

		dots = 0;
		
		packet.ps_hdr.dest_ip = host;
		packet.ps_hdr.zero = 0;
		packet.ps_hdr.protocol = P_UDP;
		packet.ps_hdr.udp_len = (int16) sizeof(UDP_PCKT);
		packet.udp_packet.src_port = 0;
		packet.udp_packet.dest_port = UDP_PORT;
		packet.udp_packet.length = (int16) sizeof(UDP_PCKT);
		packet.udp_packet.chk_sum = 0;
	
#if 0
		{
			uint32 chksum = 0;
			uint16 *walk;

			for (walk = (uint16 *) &packet, count = 0; count < sizeof(packet) / 2; walk++, count++)
				chksum += *walk;
			packet.udp_packet.chk_sum = ~(uint16) ((chksum & 0xffffL) + ((chksum >> 16) & 0xffffL));
		}
#else
		packet.udp_packet.chk_sum = in_cksum((uint16 *) &packet, sizeof(packet));
#endif
	
		for (count = 1; count <= max_ttl; count++)
		{
			got_there = 0;
			unreachable = 0;
			
			graf_mouse(BUSY_BEE, NULL);
			if ((dgram = KRmalloc(sizeof(UDP_PCKT))) == NULL)
			{
				do_alert(rs_frstr(NO_MEMORY));
				break;
			}
			memcpy(dgram, &packet.udp_packet, sizeof(UDP_PCKT));
	
			response = -1;
			now = safe_TIMER_now();
			elapsed = -1;
			what = IP_send(packet.ps_hdr.src_ip, packet.ps_hdr.dest_ip, 0, TRUE, count, P_UDP,
						   32768u + count, dgram, (int16) sizeof(UDP_PCKT), NULL, 0);
	
			if (what == E_NOMEM || what == E_UNREACHABLE)
			{
				do_alert(rs_frstr(PROB_SEND_PACKET));
				KRfree(dgram);
				break;
			}
	
			for (wait = 0; wait < waittime; wait++)
			{
				if (dialog_aborted())
				{
					count = max_ttl;
					break;
				}
				if (response >= 0)
					break;
			}

			if (response >= 0)
			{
				sprintf(alert, "%u.%u.%u.%u", hop_a, hop_b, hop_c, hop_d);
				sprintf(next_info_buf(), "%5u %-21s %3u %5lu", count, alert, ttl, elapsed);
				objc_draw_grect(tree, INFO_BOX, MAX_DEPTH, &gr);
			}			
			switch (response)
			{
			case -1:
				if (dots < maxdots)
				{
					tree[DOTS].ob_spec.free_string[dots++] = '*';
					tree[DOTS].ob_spec.free_string[dots] = '\0';
					objc_draw_grect(tree, DOTS, MAX_DEPTH, &gr);
				}
				break;
			case ICMP_ECHOREPLY:
				++got_there;
				break;
			case ICMP_TIME_EXCEEDED:
				break;
			case ICMP_UNREACH:
				switch (code)
				{
				case ICMP_UNREACH_PORT:
				case ICMP_UNREACH_PROTOCOL:
					got_there++;
					break;
				default:
					unreachable++;
					break;
				}
				count = max_ttl;
				break;
			}
		}
	}

	form_dial_grect(FMD_FINISH, &gr, &gr);

	wind_update(END_UPDATE);

	ICMP_handler(receive_echo, HNDLR_REMOVE);
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

	if (tpl == NULL)
	{
		do_alert(rs_frstr(NO_MODULE));
		return;
	}
	/*
	 * still needed for IP_send :(
	 */
	if (stx == NULL)
	{
		do_alert(rs_frstr(NO_STIK));
		return;
	}
	do_traceroute_dialog();
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
