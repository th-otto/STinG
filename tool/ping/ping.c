/*********************************************************************/
/*                                                                   */
/*     STinG : Ping Network Tool                                     */
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
#include <mint/arch/nf_ops.h>

#include "transprt.h"
#include "ping.h"
#include "icmp.h"

#define	IN_CLASSD(a)		((((a)) & 0xf0000000UL) == 0xe0000000UL)
#define	IN_MULTICAST(a)		IN_CLASSD(a)

TPL *tpl;
static DRV_LIST *sting_drivers;
static uint16 sent;
static uint16 received;
static uint16 min;
static uint16 ave;
static uint16 max;
static uint16 num_packets;
static int interval;
static uint32 host;
static char alert[200];
static int show_ping_echo;
static _WORD next_info;
static OBJECT *main_dialog;
#define NUM_INFOS (INFO_4 - INFO_1 + 1)
static char info_buf[NUM_INFOS][200];
static volatile int info_dirty;
static GRECT gr;



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


static void set_str(OBJECT *tree, _WORD obj, const char *str)
{
	TEDINFO *ted = tree[obj].ob_spec.tedinfo;
	strncpy(ted->te_ptext, str, ted->te_txtlen - 1);
}


static long fetch_clock(void)
{
	return (*(long *) 0x4baL);
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


static int send_echo(void)
{
	uint16 buffer[16];
	static uint16 sequence = 0;
	long *pl;
	int16 ret;
	
	buffer[0] = 0xaffeu;
	buffer[1] = sequence++;
	pl = (long *)&buffer[2];
	*pl = Supexec(fetch_clock);

	buffer[4] = 0xa5a5u;
	buffer[5] = 0x5a5au;
	buffer[6] = 0x0f0fu;
	buffer[7] = 0xf0f0u;

	if ((ret = ICMP_send(host, ICMP_ECHO, 0, buffer, 32)) < 0)
	{
		sprintf(alert, rs_frstr(STING_STRERROR), get_err_text(ret));
		do_alert(alert);
		return TRUE;
	}

	sent++;

	return FALSE;
}


static int16 cdecl receive_echo(IP_DGRAM *datagram)
{
	uint16 *data;
	uint16 delay;
	uint16 sequence;
	static char ip[20];
	int i;
	
	data = datagram->pkt_data;

	if (data[0] != (ICMP_ECHOREPLY << 8) || data[2] != 0xaffeu)
		return FALSE;

	received++;
	sequence = data[3];
	delay = (uint16) (fetch_clock() - *(long *) &data[4]);

	if (min > delay)
		min = delay;
	if (max < delay)
		max = delay;
	ave += delay;

	if (show_ping_echo)
	{
		sprintf(ip, "%d.%d.%d.%d",
			(int)(datagram->hdr.ip_src >> 24) & 0xff,
			(int)(datagram->hdr.ip_src >> 16) & 0xff,
			(int)(datagram->hdr.ip_src >> 8) & 0xff,
			(int)(datagram->hdr.ip_src) & 0xff);
		if (next_info == (NUM_INFOS - 1))
		{
			for (i = 1; i < NUM_INFOS; i++)
				strcpy(info_buf[i - 1], info_buf[i]);
		} else
		{
			next_info++;
		}
		sprintf(info_buf[next_info], "%5d %-15s %5u %3u %5lu", datagram->pkt_length, ip, sequence, datagram->hdr.ttl, delay * 5L);
		/*
		 * this function is called from the timer interrupt,
		 * and we must not call AES functions here.
		 * Update of the dialog has to be done below.
		 */
		info_dirty++;
	}
	
	ICMP_discard(datagram);

	return TRUE;
}


static int show_echos(OBJECT *tree, long delay)
{
	_WORD events;
	_WORD mox, moy, key, dummy;
	_WORD msg[8];
	long timeout;
	
	for (;;)
	{
		if (delay > 100)
			timeout = 100;
		else
			timeout = delay;
		events = evnt_multi(MU_TIMER | MU_BUTTON | MU_KEYBD,
			1, 1, 1,
			0, 0, 0, 0, 0,
			0, 0, 0, 0, 0,
			msg,
			timeout,
			&mox, &moy, &dummy, &dummy, &key, &dummy);
		if (info_dirty)
		{
			info_dirty = 0;
			objc_draw_grect(tree, INFO_BOX, MAX_DEPTH, &gr);
		}
		if (events & MU_BUTTON)
		{
			evnt_button(1, 1, 0, &mox, &moy, &dummy, &dummy);
			if (objc_find(tree, ROOT, MAX_DEPTH, mox, moy) == CANCEL)
				return TRUE;
		}
		if (events & MU_KEYBD)
		{
			if ((key & 0xff) == 0x1b) /* Esc */
				return TRUE;
		}
		delay -= timeout;
		if (delay <= 0)
			break;
	}
	return FALSE;
}


static void run_ping(OBJECT *tree)
{
	uint16 count;
	int done;
	
	sent = received = 0;
	min = 50000u;
	ave = max = 0;

	show_ping_echo = TRUE;
	
	done = FALSE;
	graf_mouse(BUSY_BEE, NULL);
	
	if (num_packets == 0)
		num_packets = 65534U;
	
	for (count = 0; !done && count < num_packets; count++)
	{
		done = send_echo();
		if (!done)
			done = show_echos(tree, interval);
	}

	/*
	 * if not all answers have been received yet,
	 * wait a little bit more
	 */
	for (count = 0; count < 10 && received != sent; count++)
		done = show_echos(tree, 200);

	show_ping_echo = FALSE;

	sprintf(alert, rs_frstr(FIRST), sent, received, (sent - received) * 100L / sent);
	set_str(tree, INFOLINE1, alert);

	if (received)
		sprintf(alert, rs_frstr(SECOND), min * 5L, ave * 5L / received, max * 5L);
	else
		alert[0] = '\0';
	set_str(tree, INFOLINE2, alert);
}


static void do_ping_dialog(void)
{
	OBJECT *tree;
	_WORD button;
	int i;
	
	tree = rs_tree(PING);
	main_dialog = tree;

	if (!ICMP_handler(receive_echo, HNDLR_SET))
	{
		do_alert(rs_frstr(NO_HANDLER));
		return;
	}
	set_str(tree, MODULE, tpl->module);
	set_str(tree, AUTHOR, tpl->author);
	set_str(tree, VERSION, tpl->version);
	set_str(tree, HOST, "127.0.0.1");
	set_str(tree, INTERVAL, "1000");
	set_str(tree, INFOLINE1, "");
	set_str(tree, INFOLINE2, "");

	wind_update(BEG_UPDATE);

	form_center_grect(tree, &gr);
	form_dial_grect(FMD_START, &gr, &gr);

	for (i = 0; i < NUM_INFOS; i++)
		tree[INFO_1 + i].ob_spec.free_string = info_buf[i];
	next_info = -1;
	
	for (;;)
	{
		objc_draw_grect(tree, ROOT, MAX_DEPTH, &gr);
		graf_mouse(ARROW, NULL);
		button = form_do(tree, HOST);
		tree[button].ob_state &= ~OS_SELECTED;
		if (button == CANCEL)
			break;
		if (button != START)
			continue;
		num_packets = atoi(tree[NUM].ob_spec.tedinfo->te_ptext);
		interval = atoi(tree[INTERVAL].ob_spec.tedinfo->te_ptext);
		if (inet_aton(tree[HOST].ob_spec.tedinfo->te_ptext, &host) == 0)
		{
			graf_mouse(BUSY_BEE, NULL);
			if (resolve(tree[HOST].ob_spec.tedinfo->te_ptext, NULL, &host, 1) <= 0)
			{
				do_alert(rs_frstr(INVALID_ADDR));
				continue;
			}
		}
		if (IN_MULTICAST(host))
		{
			do_alert(rs_frstr(NO_MULTICAST));
			continue;
		}
		set_str(tree, INFOLINE1, "");
		set_str(tree, INFOLINE2, "");
		objc_draw_grect(tree, INFOLINE1, MAX_DEPTH, &gr);
		objc_draw_grect(tree, INFOLINE2, MAX_DEPTH, &gr);
		run_ping(tree);
	}
	
	form_dial_grect(FMD_FINISH, &gr, &gr);

	wind_update(END_UPDATE);

	ICMP_handler(receive_echo, HNDLR_REMOVE);
}


static void gem_program(void)
{
	sting_drivers = (DRV_LIST *) Supexec(get_sting_cookie);

	if (sting_drivers == 0)
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
	if (tpl == NULL)
	{
		do_alert(rs_frstr(NO_MODULE));
		return;
	}
	if (tpl->ICMP_send == 0)
	{
		do_alert(rs_frstr(STIK_TOO_OLD));
		return;
	}
	do_ping_dialog();
}


int main(void)
{
	appl_init();

	if (rsrc_load("ping.rsc") == 0)
		do_alert("[1][ No RSC File !  ][ Hmpf ]");
	else
		gem_program();

	appl_exit();
	return 0;
}
