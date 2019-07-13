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

#include "transprt.h"
#include "layer.h"
#include "tracrout.h"


#define  ICMP_DEST_UNREACH    3
#define  ICMP_DU_PRTCL        2
#define  ICMP_DU_PORT         3
#define  ICMP_TTL_EXCEED     11

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
static char const not_there[] = "[1][ |  STinG is not loaded or enabled !   ][ Hmmm ]";
static char const corrupted[] = "[1][ |  STinG structures corrupted !   ][ Oooops ]";
static char const found_it[] = "[3][ |  Driver \'%s\',|  by %s, found,   |  version %s.][ Okay ]";
static char const no_module[] = "[1][ |  STinG Transport Driver not found !   ][ Grmbl ]";
static char const format[] = "[0][ |  Hop #%u :| |     Address %u.%u.%u.%u   ][ Okay ]";
static char const timeout[] = "[1][ |  Timeout awaiting   | |    response  !][ Retry | Cancel ]";



static int16 cdecl receive_echo(IP_DGRAM *datagram)
{
	UDP_PCKT *packet;
	uint8 *icmp;

	icmp = datagram->pkt_data;

	if (*icmp != ICMP_TTL_EXCEED && *icmp != ICMP_DEST_UNREACH)
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


static void fetch_parameters(void)
{
	OBJECT *tree;
	char *txt;
	_WORD x, y, w, h, c_x, c_y;

	graf_mouse(ARROW, NULL);

	rsrc_gaddr(R_TREE, TRACROUT, &tree);

	wind_update(BEG_UPDATE);

	form_center(tree, &x, &y, &w, &h);
	c_x = x + w / 2;
	c_y = y + h / 2;
	form_dial(FMD_START, c_x, c_y, 0, 0, x, y, w, h);
	form_dial(FMD_GROW, c_x, c_y, 0, 0, x, y, w, h);

	*tree[HOST].ob_spec.tedinfo->te_ptext = '\0';
	strcpy(tree[MAX_TTL].ob_spec.tedinfo->te_ptext, "30");

	objc_draw(tree, ROOT, MAX_DEPTH, x, y, w, h);
	form_do(tree, HOST);

	form_dial(FMD_SHRINK, c_x, c_y, 0, 0, x, y, w, h);
	form_dial(FMD_FINISH, c_x, c_y, 0, 0, x, y, w, h);

	wind_update(END_UPDATE);

	max_ttl = atoi(tree[MAX_TTL].ob_spec.tedinfo->te_ptext);
	txt = tree[HOST].ob_spec.tedinfo->te_ptext;
	txt[12] = '\0';
	h = atoi(&txt[9]);
	txt[9] = '\0';
	w = atoi(&txt[6]);
	txt[6] = '\0';
	y = atoi(&txt[3]);
	txt[3] = '\0';
	x = atoi(&txt[0]);
	host = ((uint32) x << 24) | ((uint32) y << 16) | ((uint32) w << 8) | (uint32) h;
}


static void do_some_work(void)
{
	uint16 *walk;
	uint16 *dgram;
	uint16 wait;
	int16 what;
	uint32 chksum = 0;

	fetch_parameters();

	if (host == 0)
		return;

	if (PRTCL_get_parameters(host, &packet.ps_hdr.src_ip, NULL, NULL) != E_NORMAL)
	{
		form_alert(1, "[1][ |   No Route !   ][ Hmmm ]");
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
		return;

	for (count = 1; count <= max_ttl; count++)
	{
		if ((dgram = KRmalloc(sizeof(UDP_PCKT))) == NULL)
		{
			form_alert(1, "[1][ |   Out of internal memory !   ][ Ooops ]");
			ICMP_handler(receive_echo, HNDLR_REMOVE);
			return;
		}
		memcpy(dgram, &packet.udp_packet, sizeof(UDP_PCKT));

		response = 0;

		what = IP_send(packet.ps_hdr.src_ip, packet.ps_hdr.dest_ip, 0, TRUE, count, P_UDP,
					   32768u + count, dgram, (int16) sizeof(UDP_PCKT), NULL, 0);

		if (what == E_NOMEM || what == E_UNREACHABLE)
		{
			form_alert(1, "[1][ |   Problem sending packet !   ][ Hmmm ]");
			KRfree(dgram);
			ICMP_handler(receive_echo, HNDLR_REMOVE);
			return;
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
			if (form_alert(1, timeout) == 1)
			{
				count--;
				break;
			}
			ICMP_handler(receive_echo, HNDLR_REMOVE);
			return;
		case ICMP_TTL_EXCEED:
			sprintf(alert, format, count, hop_a, hop_b, hop_c, hop_d);
			form_alert(1, alert);
			break;
		case ICMP_DEST_UNREACH:
			form_alert(1, (code == ICMP_DU_PRTCL || code == ICMP_DU_PORT) ?
					   "[3][ |   We've reached| |     the destination !  ][ Great ]" :
					   "[1][ |   We can't get to| |     the destination !  ][ Pity ]");
			ICMP_handler(receive_echo, HNDLR_REMOVE);
			return;
		}
	}
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
		form_alert(1, not_there);
		return;
	}
	if (strcmp(sting_drivers->magic, STIK_DRVR_MAGIC) != 0)
	{
		form_alert(1, corrupted);
		return;
	}

	tpl = (TPL *) (*sting_drivers->get_dftab) (TRANSPORT_DRIVER);
	stx = (STX *) (*sting_drivers->get_dftab) (MODULE_DRIVER);

	if (tpl != NULL)
	{
		sprintf(alert, found_it, tpl->module, tpl->author, tpl->version);
		form_alert(1, alert);
		do_some_work();
	} else
	{
		form_alert(1, no_module);
	}
}


int main(void)
{
	appl_init();

	if (rsrc_load("TRACROUT.RSC") == 0)
		form_alert(1, "[1][ No RSC File !  ][ Hmpf ]");
	else
		gem_program();

	appl_exit();
	return 0;
}
