/*
 *			SNTP.C
 *
 * Simple NTP Client.
 *
 * Status -
 *	Public Domain.  Distribution Unlimited.
 */

#ifdef __GNUC__
# define _GNU_SOURCE
#endif
#include <stdio.h>
#include <stddef.h>
#include <errno.h>
#include <signal.h>
#include <stdint.h>
#include <time.h>
#include <sys/time.h>
#include <stdlib.h>
#include <unistd.h>
#include <poll.h>
#include <limits.h>
#include <string.h>
#include <ctype.h>
#include <mint/ssystem.h>
#include <mint/arch/nf_ops.h>

#ifndef FALSE
#  define FALSE 0
#  define TRUE  1
#endif

#include "icmp.h"
#include <mint/mintbind.h>
#include <netdb.h>
#undef SIGINT
#define SIGINT 2
#undef SIGQUIT
#define SIGQUIT 3
#undef SIGCONT
#define SIGCONT 19
#ifdef __PUREC__
#define random() rand()
#endif
#ifdef __GNUC__
#include <gem.h>
#else
#include <aes.h>
#endif
#include "sntp.h"
#include "transprt.h"
#include "layer.h"
#include "adaptrsc.h"

#ifdef __PUREC__
char *mint_strerror(int errnum);
#else
#define mint_strerror(err) strerror(err)
#endif


static char hostname[MAXHOSTNAMELEN];

static char alert[200];
static _WORD next_info;
static OBJECT *main_dialog;
#define NUM_INFOS (INFO_4 - INFO_1 + 1)
static char info_buf[NUM_INFOS][200];
static volatile int info_dirty;
static GRECT gr;
static long timeout;
static int sockfd;
static struct sockaddr_storage whereto;
static enum {
	MODE_MINTNET,
	MODE_STING,
} mode;
TPL *tpl;
STX *stx;
static DRV_LIST *sting_drivers;

/* seconds between 1.1.1900 (NTP epoch) and 1.1.1970 (unix epoch *) */
#define NTP_TIMESTAMP_DELTA 2208988800ul
#define NTP_PORT 123

#define LI(packet)   (uint8_t) ((packet.li_vn_mode & 0xC0) >> 6)	/* (li   & 11 000 000) >> 6 */
#define VN(packet)   (uint8_t) ((packet.li_vn_mode & 0x38) >> 3)	/* (vn   & 00 111 000) >> 3 */
#define MODE(packet) (uint8_t) ((packet.li_vn_mode & 0x07) >> 0)	/* (mode & 00 000 111) >> 0 */

/* Structure that defines the 48 byte NTP packet protocol. */
/* Version 1: https://datatracker.ietf.org/doc/html/rfc1059 */
/* Version 3: https://datatracker.ietf.org/doc/html/rfc1305 */
/* Version 4: https://datatracker.ietf.org/doc/html/rfc5905 */

typedef struct
{
	uint8_t li_vn_mode;				/* Eight bits. li, vn, and mode. */
	/* li.   Two bits.   Leap indicator. */
	/* vn.   Three bits. Version number of the protocol. */
	/* mode. Three bits. Client will pick mode 3 for client. */

	uint8_t stratum;				/* Eight bits. Stratum level of the local clock. */
	uint8_t poll;					/* Eight bits. Maximum interval between successive messages. */
	uint8_t precision;				/* Eight bits. Precision of the local clock. */

	uint32_t rootDelay;				/* 32 bits. Total round trip delay time. */
	uint32_t rootDispersion;		/* 32 bits. Max error aloud from primary clock source. */
	uint32_t refid;					/* 32 bits. Reference clock identifier. */

	uint32_t refTm_s;				/* 32 bits. Reference time-stamp seconds. */
	uint32_t refTm_f;				/* 32 bits. Reference time-stamp fraction of a second. */

	uint32_t origTm_s;				/* 32 bits. Originate time-stamp seconds. */
	uint32_t origTm_f;				/* 32 bits. Originate time-stamp fraction of a second. */

	uint32_t rxTm_s;				/* 32 bits. Received time-stamp seconds. */
	uint32_t rxTm_f;				/* 32 bits. Received time-stamp fraction of a second. */

	uint32_t txTm_s;				/* 32 bits and the most important field the client cares about. Transmit time-stamp seconds. */
	uint32_t txTm_f;				/* 32 bits. Transmit time-stamp fraction of a second. */

} ntp_packet;						/* Total: 384 bits or 48 bytes. */

static ntp_packet packet;

#define LOCAL_TIME_FORMAT "%a %b %d %H:%M:%S %Z"
#define GMT_TIME_FORMAT   "%a %b %d %H:%M:%S GMT"

/* How to multiply by 4294.967296 quickly (and not quite exactly)
 * without using floating point or greater than 32-bit integers.
 * If you want to fix the last 12 microseconds of error, add in
 * (2911*(x))>>28)
 */
#define NTPFRAC(x) (4294 * (x) + ((1981 * (x)) >> 11))

/* The reverse of the above, needed if we want to set our microsecond
 * clock (via clock_settime) based on the incoming time in NTP format.
 * Basically exact.
 */
#define USEC(x) (((x) >> 12) - 759 * ((((x) >> 10) + 32768L) >> 16))



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


static _WORD do_alert(const char *str, int lockscreen)
{
	_WORD ret;
	_WORD message[8];
	_WORD dummy;
	
	graf_mouse(ARROW, NULL);
	if (lockscreen)
	{
		wind_update(END_UPDATE);
		wind_update(END_MCTRL);
	}
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
	
	if (lockscreen)
	{
		wind_update(BEG_UPDATE);
		wind_update(BEG_MCTRL);
	}
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


static void clk_gettime(struct timeval *tv)
{
	gettimeofday(tv, NULL);
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


static int dialog_aborted(long delay)
{
	_WORD events;
	_WORD mox, moy, key, dummy;
	_WORD msg[8];
	
	events = evnt_multi(MU_TIMER | MU_BUTTON | MU_KEYBD,
		1, 1, 1,
		0, 0, 0, 0, 0,
		0, 0, 0, 0, 0,
		msg,
		delay,
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


static int dots, maxdots;
static void print_dots(void)
{
	if (dots < maxdots)
	{
		main_dialog[DOTS].ob_spec.free_string[dots++] = '*';
		main_dialog[DOTS].ob_spec.free_string[dots] = '\0';
		objc_draw_grect(main_dialog, DOTS, MAX_DEPTH, &gr);
	}
}


static int gethost(const char *name, struct sockaddr_storage *sa, char *realname, int realname_len)
{
	struct hostent *hp;
	struct sockaddr_in *sin;
	
	sin = (struct sockaddr_in *)sa;
	memset(sa, 0, sizeof(*sa));
	sa->ss_family = AF_INET;

	/* If it is an IP address, try to convert it to a name to
	 * have something nice to display.
	 */
	if (inet_aton(name, &sin->sin_addr) != 0)
	{
		if (realname)
		{
			hp = gethostbyaddr((char *) &sin->sin_addr, sizeof(sin->sin_addr), AF_INET);
			strncpy(realname, hp ? hp->h_name : name, realname_len - 1);
			realname[realname_len - 1] = '\0';
		}
		return TRUE;
	}

	hp = gethostbyname(name);
	if (hp == NULL)
	{
		return FALSE;
	}

	if (hp->h_addrtype != AF_INET)
	{
		return FALSE;
	}

	memcpy(&sin->sin_addr, hp->h_addr, sizeof(sin->sin_addr));
	sin->sin_family = hp->h_addrtype;

	if (realname)
	{
		strncpy(realname, hp->h_name, realname_len - 1);
		realname[realname_len - 1] = '\0';
	}
	
	return TRUE;
}


static int ntp_mintnet(void)
{
	int cc;
	long msec;
	long dottime;
	struct pollfd fds[1];
	struct timeval tv;
	socklen_t slen;
	struct sockaddr_in *sin = (struct sockaddr_in *)&whereto;
	
	if (!gethost(main_dialog[HOST].ob_spec.tedinfo->te_ptext, &whereto, hostname, (int)sizeof(hostname)) ||
		sin->sin_addr.s_addr == INADDR_ANY)
	{
		do_alert(rs_frstr(INVALID_ADDR), TRUE);
		return FALSE;
	}
	sin->sin_port = htons(NTP_PORT);

	fds[0].fd = sockfd;
	fds[0].events = POLLIN;

	for (;;)
	{
		/* Create and zero out the packet. All 48 bytes worth. */
		memset(&packet, 0, sizeof(ntp_packet));
	
		/* Set the first byte's bits to 00,011,011 for li = 0, vn = 3, and mode = 3. The rest will be left set to zero. */
		packet.li_vn_mode = 0x1b;
	
		clk_gettime(&tv);
		packet.txTm_s = htonl(tv.tv_sec + NTP_TIMESTAMP_DELTA);
		packet.txTm_f = htonl(packet.txTm_f);
		
		/* Send it the NTP packet it wants. If n == -1, it failed. */
		cc = sendto(sockfd, &packet, sizeof(packet), 0, (struct sockaddr *)sin, sizeof(*sin));
		if (cc < 0)
		{
			sprintf(alert, rs_frstr(AL_SEND_ERROR), -errno, mint_strerror(errno));
			do_alert(alert, TRUE);
			return FALSE;
		}
	
		msec = timeout;
		dottime = 0;
		for (;;)
		{
			if (msec >= 100)
			{
				cc = poll(fds, 1, 100);
				msec -= 100;
			} else
			{
				cc = poll(fds, 1, msec);
				msec = 0;
			}
			if (cc <= 0)
			{
				if (cc < 0)
				{
					if (errno == EINTR)
						continue;
					sprintf(alert, rs_frstr(AL_POLL_ERROR), mint_strerror(errno));
					do_alert(alert, TRUE);
					return FALSE;
				}
				if (dialog_aborted(0))
					return FALSE;
				if (msec <= 0)
					return FALSE;
			} else
			{
				if (fds[0].revents & (POLLERR|POLLHUP|POLLNVAL))
				{
					do_alert(rs_frstr(HANGUP), TRUE);
					return FALSE;
				}
				if (fds[0].revents & (POLLIN | POLLRDNORM | POLLRDBAND))
					break;
			}
			dottime += 100;
			if (dottime >= 1000)
			{
				print_dots();
				dottime = 0;
			}
		}
	
		slen = sizeof(*sin);
		cc = recvfrom(sockfd, &packet, sizeof(packet), 0, (struct sockaddr *)sin, &slen);
		if (cc < 0)
		{
			if (errno != EINTR)
			{
				sprintf(alert, rs_frstr(AL_RECV_ERROR), mint_strerror(errno));
				do_alert(alert, TRUE);
			}
			return FALSE;
		}
		
		if (cc == 0)
			continue;
		
		if (packet.stratum == 0)
		{
			uint32_t refid = ntohl(packet.refid);
			char kod[5];
			kod[0] = refid >> 24;
			kod[1] = refid >> 16;
			kod[2] = refid >> 8;
			kod[3] = refid >> 0;
			kod[4] = 0;
			sprintf(alert, rs_frstr(KOD), kod);
			return FALSE;
		}

		if (packet.txTm_s == 0)
		{
			continue;
		}

		packet.txTm_s = ntohl(packet.txTm_s);
		packet.txTm_f = ntohl(packet.txTm_f);

		return TRUE;
	}
}


static int ntp_sting(void)
{
	struct sockaddr_in *sin;
	struct timeval tv;
	int handle;
	int32 starttime;
	int32 dottime;
	CIB *cib;
	int cc;
	
	sin = (struct sockaddr_in *)&whereto;
	if (inet_aton(main_dialog[HOST].ob_spec.tedinfo->te_ptext, &sin->sin_addr) == 0)
	{
		if (resolve(main_dialog[HOST].ob_spec.tedinfo->te_ptext, NULL, &sin->sin_addr.s_addr, 1) <= 0 ||
			sin->sin_addr.s_addr == INADDR_ANY)
		{
			do_alert(rs_frstr(INVALID_ADDR), TRUE);
			return FALSE;
		}
	}
	
	if ((handle = UDP_open(sin->sin_addr.s_addr, NTP_PORT)) < 0)
	{
		sprintf(alert, rs_frstr(AL_UDP_OPEN), get_err_text(handle));
		do_alert(alert, TRUE);
		return FALSE;
	}
	
	for (;;)
	{
		/* Create and zero out the packet. All 48 bytes worth. */
		memset(&packet, 0, sizeof(ntp_packet));
	
		/* Set the first byte's bits to 00,011,011 for li = 0, vn = 3, and mode = 3. The rest will be left set to zero. */
		packet.li_vn_mode = 0x1b;
	
		clk_gettime(&tv);
		packet.txTm_s = htonl(tv.tv_sec + NTP_TIMESTAMP_DELTA);
		packet.txTm_f = htonl(packet.txTm_f);
		
		/* Send it the NTP packet it wants. If n == -1, it failed. */
		cc = UDP_send(handle, &packet, sizeof(packet));
		if (cc < 0)
		{
			sprintf(alert, rs_frstr(AL_SEND_ERROR), cc, get_err_text(cc));
			do_alert(alert, TRUE);
			UDP_close(handle);
			return FALSE;
		}
	
		starttime = TIMER_now();
		dottime = starttime;
		while (CNbyte_count(handle) < (int)sizeof(packet))
		{
			_appl_yield();
			if ((cib = CNgetinfo(handle)) != NULL)
			{
				if (cib->status)
				{
					sprintf(alert, rs_frstr(AL_POLL_ERROR), get_err_text(E_EOF));
					do_alert(alert, TRUE);
					UDP_close(handle);
					return FALSE;
				}
			}
			if (dialog_aborted(100))
			{
				UDP_close(handle);
				return FALSE;
			}
			if (TIMER_elapsed(starttime) >= timeout)
			{
				UDP_close(handle);
				return FALSE;
			}
			if (TIMER_elapsed(dottime) >= 1000)
			{
				print_dots();
				dottime = TIMER_now();
			}
		}
	
		cc = CNget_block(handle, &packet, sizeof(packet));
		if (cc < 0)
		{
			sprintf(alert, rs_frstr(AL_RECV_ERROR), get_err_text(cc));
			do_alert(alert, TRUE);
			UDP_close(handle);
			return FALSE;
		}
		
		if (cc < (int)sizeof(packet))
			continue;
		
		if (packet.stratum == 0)
		{
			uint32_t refid = ntohl(packet.refid);
			char kod[5];
			kod[0] = refid >> 24;
			kod[1] = refid >> 16;
			kod[2] = refid >> 8;
			kod[3] = refid >> 0;
			kod[4] = 0;
			sprintf(alert, rs_frstr(KOD), kod);
			UDP_close(handle);
			return FALSE;
		}

		if (packet.txTm_s == 0)
		{
			continue;
		}
		
		packet.txTm_s = ntohl(packet.txTm_s);
		packet.txTm_f = ntohl(packet.txTm_f);
		
		UDP_close(handle);
		return TRUE;
	}
}


static void set_str(OBJECT *tree, _WORD obj, const char *str)
{
	TEDINFO *ted = tree[obj].ob_spec.tedinfo;
	strncpy(ted->te_ptext, str, ted->te_txtlen - 1);
}


static int do_ping_dialog(void)
{
	int i;
	OBJECT *tree;
	_WORD button;
	int ret;
	time_t txTm;
	struct timeval tv;
	struct timezone tz;
	struct tm tm;
	long gmtoff;

	graf_mouse(ARROW, NULL);

	/*
	 * make sure global variable timezone is set
	 */
	time(&txTm);
	tm = *localtime(&txTm);
	tz.tz_minuteswest = -1;
	gettimeofday(&tv, &tz);
	gmtoff = -tz.tz_minuteswest;
	/*
	 * if MiNT is not running, tz cannot be set reliably.
	 */
	if (tz.tz_minuteswest == -1)
	{
#ifdef __PUREC__
		gmtoff = timezone / 60;
		if (tm.tm_isdst > 0)
			gmtoff += 60;
#else
		gmtoff = -timezone / 60;
		if (tm.tm_isdst > 0)
			gmtoff += 60;
#endif
	}
	
	tree = rs_tree(SNTP_DIALOG);
	main_dialog = tree;
	set_str(tree, HOST, "pool.ntp.org");
	switch (mode)
	{
	case MODE_STING:
		{
			const char *config;
			int mon1, day1, mon2, day2;
			
			set_str(tree, MODULE, tpl->module);
			set_str(tree, KERNEL_VERSION, tpl->version);
			config = getvstr("NTP_SERVER");
			if (config && config[1])
				set_str(tree, HOST, config);
			config = getvstr("TIME_ZONE");
			if (config && config[1])
			{
				gmtoff = strtol(config, NULL, 10);
				config = getvstr("TIME_SUMMER");
				if (config && sscanf(config, "%d.%d.%d.%d", &mon1, &day1, &mon2, &day2) == 4)
				{
					int mon = tm.tm_mon + 1;
					
					if ((mon > mon1 || (mon == mon1 && tm.tm_mday >= day1)) &&
						(mon < mon2 || (mon == mon2 && tm.tm_mday <= day2)))
						gmtoff += 60;
				}
			}
		}
		break;
	case MODE_MINTNET:
		set_str(tree, MODULE, "MiNTNet");
		if (Ssystem(S_KNAME, (long)hostname, sizeof(hostname)) != 0)
			*hostname = '\0';
		set_str(tree, KERNEL_VERSION, hostname);
		break;
	}

	set_str(tree, TIMEOUT, "5000");
	sprintf(alert, "+%02ld%02ld", gmtoff / 60, gmtoff % 60);
	set_str(tree, GMT_OFFSET, alert);
	maxdots = (int)strlen(tree[DOTS].ob_spec.free_string);
	tree[DOTS].ob_spec.free_string[0] = '\0';

	tm = *localtime(&txTm);
	strftime(alert, sizeof(alert), LOCAL_TIME_FORMAT, &tm);
	set_str(tree, CURRTIME, alert);

	wind_update(BEG_UPDATE);
	wind_update(BEG_MCTRL);

	form_center_grect(tree, &gr);
	form_dial_grect(FMD_START, &gr, &gr);

	for (i = 0; i < NUM_INFOS; i++)
		tree[INFO_1 + i].ob_spec.free_string = info_buf[i];
	next_info = -1;
	
	for (;;)
	{
		objc_draw_grect(tree, ROOT, MAX_DEPTH, &gr);
		button = form_do(tree, HOST);
		if (button == CANCEL)
			break;
		if (button == START)
		{
			tree[button].ob_state &= ~OS_SELECTED;
			objc_draw_grect(tree, button, MAX_DEPTH, &gr);

			timeout = strtol(tree[TIMEOUT].ob_spec.tedinfo->te_ptext, NULL, 0);
			if (timeout <= 0)
				timeout = 5000;
			gmtoff = strtol(tree[GMT_OFFSET].ob_spec.tedinfo->te_ptext, NULL, 10);
			gmtoff = ((gmtoff / 100) * 60) + (gmtoff % 100);
#ifdef __PUREC__
			/* Pure-C has it stored with wrong sign */
			timezone = gmtoff * 60;
			if (tm.tm_isdst > 0)
				timezone += 3600;
#else
			timezone = -gmtoff * 60;
			if (tm.tm_isdst > 0)
				timezone += 3600;
#endif
			strcpy(hostname, tree[HOST].ob_spec.tedinfo->te_ptext);
			
			graf_mouse(BUSY_BEE, NULL);
			
			switch (mode)
			{
			case MODE_MINTNET:
				ret = ntp_mintnet();
				break;
			case MODE_STING:
				ret = ntp_sting();
				break;
			default:
				ret = FALSE;
				break;
			}
			
			graf_mouse(ARROW, NULL);
			if (ret)
			{
				char *p;
				
				/* Extract the 32 bits that represent the time-stamp seconds (since NTP epoch) from when the packet left the server. */
				/* Subtract 70 years worth of seconds from the seconds since 1900. */
				/* This leaves the seconds since the UNIX epoch of 1970. */
				/* (1900)------------------(1970)**************************************(Time Packet Left the Server) */
			
				txTm = (time_t) (packet.txTm_s - NTP_TIMESTAMP_DELTA);
			
				/* Print the time we got from the server, accounting for local timezone and conversion from UTC time. */
				p = next_info_buf();
				tm = *gmtime(&txTm);
				strftime(p, sizeof(info_buf[0]), GMT_TIME_FORMAT, &tm);
				p = next_info_buf();
				tm = *localtime(&txTm);
				strftime(p, sizeof(info_buf[0]), LOCAL_TIME_FORMAT, &tm);
				
				if (tree[UPDATE_TIME].ob_state & OS_SELECTED)
				{
					tv.tv_sec = txTm;
					tv.tv_usec = USEC(packet.txTm_f);
					tz.tz_minuteswest = (int)-gmtoff;
					tz.tz_dsttime = -1;
					if (settimeofday(&tv, &tz) != 0)
					{
						do_alert(rs_frstr(AL_SETTIME), TRUE);
					} else
					{
						time(&txTm);
						tm = *localtime(&txTm);
						strftime(alert, sizeof(alert), LOCAL_TIME_FORMAT, &tm);
						set_str(tree, CURRTIME, alert);
					}
				}
			}
		}
	}
	
	form_dial_grect(FMD_FINISH, &gr, &gr);

	wind_update(END_MCTRL);
	wind_update(END_UPDATE);
	
	return TRUE;
}


static _WORD init_vwork(void)
{
	_WORD work_out[57];
	_WORD work_in[11];
	int i;
	_WORD dummy;
	
	aes_handle = graf_handle(&gl_wchar, &gl_hchar, &dummy, &dummy);
	for (i = 0; i < 10; i++)
		work_in[i] = 1;
	
	work_in[10] = 2;
	vdi_handle = aes_handle;
	v_opnvwk(work_in, &vdi_handle, work_out);
	
	return vdi_handle;
}


static int init_resource(void)
{
	_WORD hor_3d;
	_WORD ver_3d;

	if (init_vwork() == 0)
		return FALSE;
	aes_flags = get_aes_info(&aes_font, &aes_height, &hor_3d, &ver_3d);
	
	substitute_objects(rs_tree(0), NUM_OBS, aes_flags, NULL, NULL);
	
	return TRUE;
}


static int gem_program(void)
{
	if ((sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) >= 0)
	{
		mode = MODE_MINTNET;
	} else if ((sting_drivers = (DRV_LIST *) Supexec(get_sting_cookie)) != NULL)
	{
		if (strcmp(sting_drivers->magic, STIK_DRVR_MAGIC) != 0)
		{
			do_alert(rs_frstr(CORRUPTED), FALSE);
			return FALSE;
		}
		tpl = (TPL *) (*sting_drivers->get_dftab) (TRANSPORT_DRIVER);
		stx = (STX *) (*sting_drivers->get_dftab) (MODULE_DRIVER);
		if (tpl == NULL || stx == NULL)
		{
			do_alert(rs_frstr(NO_MODULE), FALSE);
			return FALSE;
		}
		mode = MODE_STING;
	} else
	{
		do_alert(rs_frstr(NO_NETDRIVER), FALSE);
		return FALSE;
	}

	if (init_resource() == FALSE)
		return FALSE;

	do_ping_dialog();
	return TRUE;
}


int main(void)
{
	int exit_code = EXIT_SUCCESS;

	Pdomain(1);
	appl_init();

	if (rsrc_load("sntp.rsc") == 0)
		do_alert("[1][ No RSC File !  ][ Hmpf ]", FALSE);
	else if (gem_program() == FALSE)
		exit_code = EXIT_FAILURE;

	appl_exit();

	return exit_code;
}
