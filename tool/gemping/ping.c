/*
 *			P I N G . C
 *
 * Using the InterNet Control Message Protocol (ICMP) "ECHO" facility,
 * measure round-trip-delays and packet loss across network paths.
 *
 * Author -
 *	Mike Muuss
 *	U. S. Army Ballistic Research Laboratory
 *	December, 1983
 * Modified at Uc Berkeley
 * Record Route and verbose headers - Phil Dykstra, BRL, March 1988.
 * Multicast options (ttl, if, loop) - Steve Deering, Stanford, August 1988.
 * ttl, duplicate detection - Cliff Frost, UCB, April 1989
 * Pad pattern - Cliff Frost (from Tom Ferrin, UCSF), April 1989
 *
 * Status -
 *	Public Domain.  Distribution Unlimited.
 *
 * Bugs -
 *	More statistics could always be gathered.
 *	This program has to run SUID to ROOT to access the ICMP socket.
 */

#include <stdio.h>
#include <stddef.h>
#include <errno.h>
#include <signal.h>
#include <stdint.h>
#include <sys/time.h>
#include <stdlib.h>
#include <unistd.h>
#include <poll.h>
#include <limits.h>
#include <math.h>
#include <string.h>
#include <ctype.h>

#ifndef FALSE
#  define FALSE 0
#  define TRUE  1
#endif

#if (defined(__MINT__) || defined(__PUREC__)) && !defined(__atarist__)
#define __atarist__ 1
#endif

#ifdef __atarist__
#include "icmp.h"
#include <mint/mintbind.h>
#include <netdb.h>
#ifdef __PUREC__
char *mint_strerror(int errnum);
#else
#define mint_strerror(err) strerror(err)
#endif
/*
 * need to use the MiNT definitions,
 * regardless what's defined in signal.h
 */
#undef SIGINT
#define SIGINT 2
#undef SIGQUIT
#define SIGQUIT 3
#undef SIGCONT
#define SIGCONT 19
#ifdef __PUREC__
#define random() rand()
#endif
#else
#include <sys/socket.h>
#include <sys/param.h>
#include <netinet/in_systm.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <arpa/inet.h>
#include <netdb.h>
#endif

#define FLOOD_INTVL	10000L				/* default flood output interval */
#define	MAXPACKET	(IP_MAXPACKET-60-8)	/* max packet size */
#define F_VERBOSE	0x0001
#define F_QUIET		0x0002				/* minimize all output */
#define F_SEMI_QUIET	0x0004			/* ignore our ICMP errors */
#define F_FLOOD		0x0008				/* flood-ping */
#define	F_RECORD_ROUTE	0x0010			/* record route */
#define F_SOURCE_ROUTE	0x0020			/* loose source route */
#define F_PING_FILLED	0x0040			/* is buffer filled with user data? */
#define F_PING_RANDOM	0x0080			/* use random data */
#define	F_NUMERIC	0x0100				/* do not do gethostbyaddr() calls */
#define F_TIMING	0x0200				/* room for a timestamp */
#define F_DF		0x0400				/* set IP DF bit */
#define F_SOURCE_ADDR	0x0800			/* set source IP address/interface */
#define F_ONCE		0x1000				/* exit(0) after receiving 1 reply */
#define F_MCAST		0x2000				/* multicast target */
#define F_MCAST_NOLOOP	0x4000			/* no multicast loopback */
#define F_AUDIBLE	0x8000				/* audible output */
#define F_TIMING64	0x10000L			/* 64 bit time, nanoseconds */

/* MAX_DUP_CHK is the number of bits in received table, the
 *	maximum number of received sequence numbers we can track to check
 *	for duplicates.
 */
#define MAX_DUP_CHK     (8 * 2048)
static uint8_t rcvd_tbl[MAX_DUP_CHK / 8];
static int nrepeats = 0;

#define A(seq)	rcvd_tbl[(seq/8)%sizeof(rcvd_tbl)]	/* byte in array */
#define B(seq)	(1 << (seq & 0x07))		/* bit in byte */
#define SET(seq) (A(seq) |= B(seq))
#define CLR(seq) (A(seq) &= (~B(seq)))
#define TST(seq) (A(seq) & B(seq))

struct tv32
{
	int32_t tv32_sec;
	int32_t tv32_usec;
};


static uint8_t *packet;
static int packlen;
static long pingflags = 0;
static int options;
static int pongflags = 0;
static char *fill_pat;

static int s;							/* Socket file descriptor */

#define PHDR_LEN sizeof(struct tv32)	/* size of timestamp header */
#define PHDR64_LEN sizeof(struct timeval)	/* size of timestamp header */
static struct sockaddr_in whereto;
static struct sockaddr_in send_addr;	/* Who to ping */
static struct sockaddr_in src_addr;		/* from where */
static struct sockaddr_in loc_addr;		/* 127.1 */
static int datalen;						/* How much data */
static int phdrlen;

static sigset_t blockmask;
static sigset_t enablemask;				/* signal masks */

static char const progname[] = "ping";

static char hostname[MAXHOSTNAMELEN];

static struct
{
	struct ip o_ip;
	char o_opt[MAX_IPOPTLEN];
	union
	{
		uint8_t u_buf[MAXPACKET + offsetof(struct icmp, icmp_data)];
		struct icmp u_icmp;
	} o_u;
} out_pack;

#define	opack_icmp	out_pack.o_u.u_icmp
static struct ip *opack_ip;

static uint8_t optspace[MAX_IPOPTLEN];	/* record route space */
static unsigned int optlen;

static int npackets;					/* total packets to send */
static int preload;						/* number of packets to "preload" */
static int ntransmitted;				/* output sequence # = #sent */
static int ident;						/* our ID, in network byte order */

static int nreceived;					/* # of packets we got back */

static long interval;					/* interval between packets, in usecs */
static struct timeval interval_tv;
static long tmin = 999999L;
static long tmax = 0;
static long tsum = 0;					/* sum of all times */
static long tsumsq = 0;
static long maxwait = 0;

static long bufspace;

static struct timeval now;
static struct timeval clear_cache;
static struct timeval last_tx;
static struct timeval next_tx;
static struct timeval first_tx;
static struct timeval last_rx;
static struct timeval first_rx;
static int lastrcvd = 1;				/* last ping sent has been received */

static struct timeval jiggle_time;
static int jiggle_cnt;
static int total_jiggled;
static int jiggle_direction = -1;


static void clk_gettime(struct timeval *tv)
{
	gettimeofday(tv, NULL);
}


static void blocksignals(void)
{
#ifdef __atarist__
	Psigsetmask(blockmask);
#else
	if (sigprocmask(SIG_SETMASK, &blockmask, NULL) == -1)
	{
		fprintf(stderr, "%s: blocksignals: sigprocmask: %s\n", progname, mint_strerror(errno));
	}
#endif
}


static void enablesignals(void)
{
#ifdef __atarist__
	Psigsetmask(enablemask);
#else
	if (sigprocmask(SIG_SETMASK, &enablemask, NULL) == -1)
	{
		fprintf(stderr, "%s: enablesignals: sigprocmask: %s\n", progname, mint_strerror(errno));
	}
#endif
}


static void rnd_fill(void)
{
	static uint32_t rnd;
	int i;

	for (i = phdrlen; i < datalen; i++)
	{
		rnd = (3141592621UL * rnd + 663896637UL);
		opack_icmp.icmp_data[i] = rnd >> 24;
	}
}


static int fill(void)
{
	int i;
	int j;
	int k;
	char *cp;
	int pat[16];

	for (cp = fill_pat; *cp != '\0'; cp++)
	{
		if (!isxdigit((unsigned char) *cp))
			break;
	}
	if (cp == fill_pat || *cp != '\0' || (cp - fill_pat) > 16 * 2)
	{
		fflush(stdout);
		fprintf(stderr, "%s: \"-p %s\": patterns must be specified with 1-32 hex digits\n", progname, fill_pat);
		return FALSE;
	}

	i = sscanf(fill_pat,
			   "%2x%2x%2x%2x%2x%2x%2x%2x%2x%2x%2x%2x%2x%2x%2x%2x",
			   &pat[0], &pat[1], &pat[2], &pat[3],
			   &pat[4], &pat[5], &pat[6], &pat[7],
			   &pat[8], &pat[9], &pat[10], &pat[11], &pat[12], &pat[13], &pat[14], &pat[15]);

	for (k = phdrlen, j = 0; k < datalen; k++)
	{
		opack_icmp.icmp_data[k] = pat[j];
		if (++j >= i)
			j = 0;
	}

	if (!(pingflags & F_QUIET))
	{
		printf("PATTERN: 0x");
		for (j = 0; j < i; j++)
			printf("%02x", (uint8_t) opack_icmp.icmp_data[phdrlen + j]);
		printf("\n");
	}

	return TRUE;
}



static void timevaladd(struct timeval *t1, struct timeval *t2, struct timeval *res)
{
	res->tv_sec = t1->tv_sec + t2->tv_sec;
	if ((res->tv_usec = t1->tv_usec + t2->tv_usec) >= 1000000L)
	{
		res->tv_sec++;
		res->tv_usec -= 1000000L;
	}
}


/*
 * compute the difference of two timevals in useconds
 */
static long diffsec(struct timeval *timenow, struct timeval *then)
{
	if (timenow->tv_sec == 0)
		return -1000000L;
	return (timenow->tv_sec - then->tv_sec) * 1000000L + (timenow->tv_usec - then->tv_usec);
}


static void usec_to_timeval(long usec, struct timeval *tp)
{
	tp->tv_sec = usec / 1000000L;
	tp->tv_usec = usec - tp->tv_sec * 1000000L;
}


static long timeval_to_sec(const struct timeval *tp)
{
	return tp->tv_sec * 1000000L + tp->tv_usec;
}



static void jiggle_flush(int nl)		/* new line if there are dots */
{
	int serrno = errno;

	if (jiggle_cnt > 0)
	{
		total_jiggled += jiggle_cnt;
		jiggle_direction = 1;
		do
		{
			putchar('.');
		} while (--jiggle_cnt > 0);

	} else if (jiggle_cnt < 0)
	{
		total_jiggled -= jiggle_cnt;
		jiggle_direction = -1;
		do
		{
			putchar('\b');
		} while (++jiggle_cnt < 0);
	}

	if (nl)
	{
		if (total_jiggled != 0)
			putchar('\n');
		total_jiggled = 0;
		jiggle_direction = -1;
	}

	fflush(stdout);
	fflush(stderr);
	jiggle_time = now;
	errno = serrno;
}


/* jiggle the cursor for flood-ping
 */
static void jiggle(int delta)
{
	long dt;

	if (pingflags & F_QUIET)
		return;

	/* do not back up into messages */
	if (total_jiggled + jiggle_cnt + delta < 0)
		return;

	jiggle_cnt += delta;

	/* flush the FLOOD dots when things are quiet
	 * or occassionally to make the cursor jiggle.
	 */
	dt = diffsec(&last_tx, &jiggle_time);
	if (dt > 200000L || (dt >= 150000L && delta * jiggle_direction < 0))
		jiggle_flush(0);
}


/*
 * Print statistics.
 * Heavily buffered STDIO is used here, so that all the statistics
 * will be written with 1 sys-write call.  This is nice when more
 * than one copy of the program is running on a terminal;  it prevents
 * the statistics output from becomming intermingled.
 */
static void summary(int header)
{
	jiggle_flush(1);

	if (header)
		printf("\n----%s PING Statistics----\n", hostname);
	printf("%d packets transmitted, ", ntransmitted);
	printf("%d packets received, ", nreceived);
	if (nrepeats)
		printf("+%d duplicates, ", nrepeats);
	if (ntransmitted)
	{
		if (nreceived > ntransmitted)
		{
			printf("-- somebody's duplicating packets!");
		} else
		{
			printf("%ld%% packet loss", (((ntransmitted - nreceived) * 100L) / ntransmitted));
		}
	}
	printf("\n");
	if (nreceived && (pingflags & (F_TIMING | F_TIMING64)))
	{
		long n = nreceived + nrepeats;
		long avg = tsum / n;
		long variance = 0;

		if (n > 1)
			variance = sqrt((tsumsq - n * (double)avg * (double)avg) / (n - 1));

		printf("round-trip min/avg/max/stddev = "
			"%ld.%03ld/%ld.%03ld/%ld.%03ld/%ld.%03ld ms\n",
			tmin / 1000L, tmin % 1000L, avg / 1000L, avg % 1000L, tmax / 1000L, tmax % 1000L, variance / 1000L, variance % 1000L);
		if (pingflags & F_FLOOD)
		{
			long r = diffsec(&last_rx, &first_rx);
			long t = diffsec(&last_tx, &first_tx);

			if (r == 0)
				r = 1000;
			if (t == 0)
				t = 1000;
			t = (ntransmitted * 1000L) / t;
			r = (nreceived * 1000L) / r;
			printf("  %ld.%03ld packets/sec sent,  %ld.%03ld packets/sec received\n", t / 1000, t % 1000, r / 1000, r % 1000);
		}
	}
}


/*
 * Print statistics and give up.
 */
#ifdef __atarist__
static void __CDECL finish(long sig)
#else
static void finish(int sig)
#endif
{
	(void)sig;
#ifdef __atarist__
	Psignal(SIGQUIT, 0);
#else
#ifdef SIGINFO
	signal(SIGINFO, SIG_DFL);
#else
	signal(SIGQUIT, SIG_DFL);
#endif
#endif

	summary(1);
	exit(nreceived > 0 ? 0 : 2);
}


/*
 * On the first SIGINT, allow any outstanding packets to dribble in
 */
#ifdef __atarist__
static void __CDECL prefinish(long sig)
#else
static void prefinish(int sig)
#endif
{
	if (lastrcvd						/* quit now if caught up */
		|| nreceived == 0)				/* or if remote is dead */
		finish(0);

#ifdef __atarist__
	Psignal(sig, finish);
#else
	signal(sig, finish);				/* do this only the 1st time */
#endif

	if (npackets > ntransmitted)		/* let the normal limit work */
		npackets = ntransmitted;
}


/*
 * Print statistics when SIGINFO is received.
 */
#ifdef __atarist__
static void __CDECL prtsig(long sig)
{
	(void)sig;
	summary(0);
	Psignal(SIGQUIT, prtsig);
}
#else
static void prtsig(int sig)
{
	(void)sig;
	summary(0);
#ifndef SIGINFO
	signal(SIGQUIT, prtsig);
#endif
}
#endif


/*
 *  Return an ASCII host address
 *  as a dotted quad and optionally with a hostname
 */
static char *pr_addr(struct in_addr *addr)	/* in network order */
{
	struct hostent *hp;
	static char buf[MAXHOSTNAMELEN + 4 + 16 + 1];

	if ((pingflags & F_NUMERIC) || (hp = gethostbyaddr((char *) addr, sizeof(*addr), AF_INET)) == NULL)
	{
		strcpy(buf, inet_ntoa(*addr));
	} else
	{
		sprintf(buf, "%s (%s)", hp->h_name, inet_ntoa(*addr));
	}

	return buf;
}


/*
 * Print an ASCII host address starting from a string of bytes.
 */
static void pr_saddr(uint8_t *cp)
{
	n_long l;
	struct in_addr addr;

	l = (uint8_t) * ++cp;
	l = (l << 8) + (uint8_t) * ++cp;
	l = (l << 8) + (uint8_t) * ++cp;
	l = (l << 8) + (uint8_t) * ++cp;
	addr.s_addr = htonl(l);
	printf("\t%s", l == 0 ? "0.0.0.0" : pr_addr(&addr));
}


static int								/* 0=do not print it */
ck_pr_icmph(struct icmp *icp, struct sockaddr_in *from, int cc, int override)	/* 1=override VERBOSE if interesting */
{
	int hlen;
	struct ip ipb;
	struct ip *ip = &ipb;
	struct icmp icp2b;
	struct icmp *icp2 = &icp2b;
	int res;

	if (pingflags & F_VERBOSE)
	{
		res = 1;
		jiggle_flush(1);
	} else
	{
		res = 0;
	}

	memcpy(ip, icp->icmp_data, sizeof(*ip));
	hlen = ip->ip_hl << 2;
	if (ip->ip_p == IPPROTO_ICMP && hlen + 6 <= cc)
	{
		memcpy(icp2, &icp->icmp_data[hlen], sizeof(*icp2));
		if (icp2->icmp_id == ident)
		{
			if (!res && override && (pingflags & (F_QUIET | F_SEMI_QUIET)) == 0)
			{
				jiggle_flush(1);
				printf("%d bytes from %s: ", cc, pr_addr(&from->sin_addr));
				res = 1;
			}
		}
	}

	return res;
}


/*
 *  Print an IP header with options.
 */
static void pr_iph(struct icmp *icp, int cc)
{
	int hlen;
	uint8_t *cp;
	struct ip ipb;
	struct ip *ip = &ipb;

	memcpy(ip, icp->icmp_data, sizeof(*ip));

	hlen = ip->ip_hl << 2;
	cp = (uint8_t *) & icp->icmp_data[20];	/* point to options */

	printf("\n Vr HL TOS  Len   ID Flg  off TTL Pro  cks      Src	     Dst\n");
	printf("  %1x  %1x  %02x %04x %04x", ip->ip_v, ip->ip_hl, ip->ip_tos, ip->ip_len, ip->ip_id);
	printf("   %1x %04x", ((ip->ip_off) & 0xe000) >> 13, (ip->ip_off) & 0x1fff);
	printf("  %02x  %02x %04x", ip->ip_ttl, ip->ip_p, ip->ip_sum);
	printf(" %15s ", inet_ntoa(*(struct in_addr *) &ip->ip_src.s_addr));
	printf(" %s ", inet_ntoa(*(struct in_addr *) &ip->ip_dst.s_addr));
	/* dump any option bytes */
	while (hlen-- > 20 && cp < (uint8_t *) icp + cc)
	{
		printf("%02x", *cp++);
	}
}


/*
 *  Dump some info on a returned (via ICMP) IP packet.
 */
static void pr_retip(struct icmp *icp, int cc)
{
	int hlen;
	uint8_t *cp;
	struct ip ipb;
	struct ip *ip = &ipb;

	memcpy(ip, icp->icmp_data, sizeof(*ip));

	if (pingflags & F_VERBOSE)
		pr_iph(icp, cc);

	hlen = ip->ip_hl << 2;
	cp = (uint8_t *) & icp->icmp_data[hlen];

	if (ip->ip_p == IPPROTO_TCP)
	{
		if (pingflags & F_VERBOSE)
			printf("\n  TCP: from port %u, to port %u", (*cp * 256 + *(cp + 1)), (*(cp + 2) * 256 + *(cp + 3)));
	} else if (ip->ip_p == IPPROTO_UDP)
	{
		if (pingflags & F_VERBOSE)
			printf("\n  UDP: from port %u, to port %u", (*cp * 256 + *(cp + 1)), (*(cp + 2) * 256 + *(cp + 3)));
	} else if (ip->ip_p == IPPROTO_ICMP)
	{
		struct icmp icp2;

		memcpy(&icp2, cp, sizeof(icp2));
		if (icp2.icmp_type == ICMP_ECHO)
		{
			if (pingflags & F_VERBOSE)
				printf("\n  ID=%u icmp_seq=%u",
							  ntohs((uint16_t) icp2.icmp_id), ntohs((uint16_t) icp2.icmp_seq));
			else
				printf(" for icmp_seq=%u", ntohs((uint16_t) icp2.icmp_seq));
		}
	}
}


/*
 *  Print a descriptive string about an ICMP header other than an echo reply.
 */
static int								/* 0=printed nothing */
pr_icmph(struct icmp *icp, struct sockaddr_in *from, int cc)
{
	switch (icp->icmp_type)
	{
	case ICMP_UNREACH:
		if (!ck_pr_icmph(icp, from, cc, 1))
			return 0;
		switch (icp->icmp_code)
		{
		case ICMP_UNREACH_NET:
			printf("Destination Net Unreachable");
			break;
		case ICMP_UNREACH_HOST:
			printf("Destination Host Unreachable");
			break;
		case ICMP_UNREACH_PROTOCOL:
			printf("Destination Protocol Unreachable");
			break;
		case ICMP_UNREACH_PORT:
			printf("Destination Port Unreachable");
			break;
		case ICMP_UNREACH_NEEDFRAG:
			printf("frag needed and DF set.  Next MTU=%d", ntohs(icp->icmp_nextmtu));
			break;
		case ICMP_UNREACH_SRCFAIL:
			printf("Source Route Failed");
			break;
		case ICMP_UNREACH_NET_UNKNOWN:
			printf("Unreachable unknown net");
			break;
		case ICMP_UNREACH_HOST_UNKNOWN:
			printf("Unreachable unknown host");
			break;
		case ICMP_UNREACH_ISOLATED:
			printf("Unreachable host isolated");
			break;
		case ICMP_UNREACH_NET_PROHIB:
			printf("Net prohibited access");
			break;
		case ICMP_UNREACH_HOST_PROHIB:
			printf("Host prohibited access");
			break;
		case ICMP_UNREACH_TOSNET:
			printf("Bad TOS for net");
			break;
		case ICMP_UNREACH_TOSHOST:
			printf("Bad TOS for host");
			break;
		case 13:
			printf("Communication prohibited");
			break;
		case 14:
			printf("Host precedence violation");
			break;
		case 15:
			printf("Precedence cutoff");
			break;
		default:
			printf("Bad Destination Unreachable Code: %d", icp->icmp_code);
			break;
		}
		/* Print returned IP header information */
		pr_retip(icp, cc);
		break;

	case ICMP_SOURCEQUENCH:
		if (!ck_pr_icmph(icp, from, cc, 1))
			return 0;
		printf("Source Quench");
		pr_retip(icp, cc);
		break;

	case ICMP_REDIRECT:
		if (!ck_pr_icmph(icp, from, cc, 1))
			return 0;
		switch (icp->icmp_code)
		{
		case ICMP_REDIRECT_NET:
			printf("Redirect Network");
			break;
		case ICMP_REDIRECT_HOST:
			printf("Redirect Host");
			break;
		case ICMP_REDIRECT_TOSNET:
			printf("Redirect Type of Service and Network");
			break;
		case ICMP_REDIRECT_TOSHOST:
			printf("Redirect Type of Service and Host");
			break;
		default:
			printf("Redirect--Bad Code: %d", icp->icmp_code);
			break;
		}
		printf(" New router addr: %s", pr_addr(&icp->icmp_hun.ih_gwaddr));
		pr_retip(icp, cc);
		break;

	case ICMP_ECHO:
		if (!ck_pr_icmph(icp, from, cc, 0))
			return 0;
		printf("Echo Request: ID=%d seq=%d", ntohs(icp->icmp_id), ntohs(icp->icmp_seq));
		break;

	case ICMP_ECHOREPLY:
		/* displaying other's pings is too noisey */
#if 0
		if (!ck_pr_icmph(icp, from, cc, 0))
			return 0;
		printf("Echo Reply: ID=%d seq=%d", ntohs(icp->icmp_id), ntohs(icp->icmp_seq));
		break;
#else
		return 0;
#endif

	case ICMP_ROUTERADVERT:
		if (!ck_pr_icmph(icp, from, cc, 0))
			return 0;
		printf("Router Discovery Advert");
		break;

	case ICMP_ROUTERSOLICIT:
		if (!ck_pr_icmph(icp, from, cc, 0))
			return 0;
		printf("Router Discovery Solicit");
		break;

	case ICMP_TIMXCEED:
		if (!ck_pr_icmph(icp, from, cc, 1))
			return 0;
		switch (icp->icmp_code)
		{
		case ICMP_TIMXCEED_INTRANS:
			printf("Time To Live exceeded");
			break;
		case ICMP_TIMXCEED_REASS:
			printf("Frag reassembly time exceeded");
			break;
		default:
			printf("Time exceeded, Bad Code: %d", icp->icmp_code);
			break;
		}
		pr_retip(icp, cc);
		break;

	case ICMP_PARAMPROB:
		if (!ck_pr_icmph(icp, from, cc, 1))
			return 0;
		printf("Parameter problem: pointer = 0x%02x", icp->icmp_hun.ih_pptr);
		pr_retip(icp, cc);
		break;

	case ICMP_TSTAMP:
		if (!ck_pr_icmph(icp, from, cc, 0))
			return 0;
		printf("Timestamp");
		break;

	case ICMP_TSTAMPREPLY:
		if (!ck_pr_icmph(icp, from, cc, 0))
			return 0;
		printf("Timestamp Reply");
		break;

	case ICMP_IREQ:
		if (!ck_pr_icmph(icp, from, cc, 0))
			return 0;
		printf("Information Request");
		break;

	case ICMP_IREQREPLY:
		if (!ck_pr_icmph(icp, from, cc, 0))
			return 0;
		printf("Information Reply");
		break;

	case ICMP_MASKREQ:
		if (!ck_pr_icmph(icp, from, cc, 0))
			return 0;
		printf("Address Mask Request");
		break;

	case ICMP_MASKREPLY:
		if (!ck_pr_icmph(icp, from, cc, 0))
			return 0;
		printf("Address Mask Reply");
		break;

	default:
		if (!ck_pr_icmph(icp, from, cc, 0))
			return 0;
		printf("Bad ICMP type: %d", icp->icmp_type);
		if (pingflags & F_VERBOSE)
			pr_iph(icp, cc);
	}

	return 1;
}


/* Compute the IP checksum
 *	This assumes the packet is less than 32K long.
 */
static uint16_t in_cksum(uint16_t *p, unsigned int len)
{
	uint32_t sum = 0;
	int nwords = len >> 1;

	while (nwords-- != 0)
		sum += *p++;

	if (len & 1)
	{
		union
		{
			uint16_t w;
			uint8_t c[2];
		} u;

		u.c[0] = *(uint8_t *) p;
		u.c[1] = 0;
		sum += u.w;
	}

	/* end-around-carry */
	sum = (sum >> 16) + (sum & 0xffff);
	sum += (sum >> 16);
	return (~sum);
}


/*
 * Compose and transmit an ICMP ECHO REQUEST packet.  The IP packet
 * will be added on by the kernel.  The ID field is our UNIX process ID,
 * and the sequence number is an ascending integer.  The first phdrlen bytes
 * of the data portion are used to hold a UNIX "timeval" struct in VAX
 * byte-order, to compute the round-trip time, or a UNIX "timeval" in native
 * format.
 */
static void pinger(void)
{
	struct tv32 tv32;
	int i;
	int cc;
	long waittime;
	long numskip;

	opack_icmp.icmp_code = 0;
	opack_icmp.icmp_seq = htons((uint16_t) (ntransmitted));

	opack_icmp.icmp_type = ICMP_ECHO;
	opack_icmp.icmp_id = ident;

	if (pingflags & F_TIMING)
	{
		tv32.tv32_sec = (uint32_t) htonl(now.tv_sec);
		tv32.tv32_usec = htonl(now.tv_usec);
		(void) memcpy(&opack_icmp.icmp_data[0], &tv32, sizeof(tv32));
	} else if (pingflags & F_TIMING64)
	{
		memcpy(&opack_icmp.icmp_data[0], &now, sizeof(now));
	}

	cc = ICMP_MINLEN;
	if (datalen > cc)
		cc = datalen;
	cc += (int)PHDR_LEN;
	opack_icmp.icmp_cksum = 0;
	opack_icmp.icmp_cksum = in_cksum((uint16_t *) & opack_icmp, cc);

	cc += opack_ip->ip_hl << 2;
	opack_ip->ip_len = cc;
	i = sendto(s, (char *) opack_ip, cc, 0, (struct sockaddr *) &send_addr, sizeof(struct sockaddr_in));
	if (i != cc)
	{
		jiggle_flush(1);
		if (i < 0)
			fprintf(stderr, "%s: sendto: %s\n", progname, mint_strerror(errno));
		else
			fprintf(stderr, "%s: wrote %s %d chars, ret=%d\n", progname, hostname, cc, i);
		fflush(stderr);
	}
	lastrcvd = 0;

	CLR(ntransmitted);
	ntransmitted++;

	last_tx = now;
	if (next_tx.tv_sec == 0)
	{
		first_tx = now;
		next_tx = now;
	}

	/* Transmit regularly, at always the same microsecond in the
	 * second when going at one packet per second.
	 * If we are at most 100 ms behind, send extras to get caught up.
	 * Otherwise, skip packets we were too slow to send.
	 */
	waittime = diffsec(&next_tx, &now);
	if (waittime < -1000000L)
	{
		/* very behind - forget about being precise */
		next_tx.tv_sec += (int) (-waittime / 1000000L);
	} else if (waittime < -100000L)
	{
		/* behind - skip a few */
		if (interval_tv.tv_sec == 0)
		{
			numskip = (long) (-waittime / interval_tv.tv_usec);
			next_tx.tv_usec += numskip * interval_tv.tv_usec;
			/*
			 * We can add at most one second's worth, but allow
			 * for tv_nsec reaching 2 billion just in case FP
			 * issues strike.
			 */
			while (next_tx.tv_usec >= 1000000L)
			{
				next_tx.tv_sec++;
				next_tx.tv_usec -= 1000000L;
			}
		} else
		{
			do
			{
				timevaladd(&next_tx, &interval_tv, &next_tx);
			} while (diffsec(&next_tx, &now) < -100000L);
		}
	} else if (waittime <= interval)
	{
		timevaladd(&next_tx, &interval_tv, &next_tx);
	}

	if (pingflags & F_FLOOD)
		jiggle(1);

	/* While the packet is going out, ready buffer for the next
	 * packet. Use a fast but not very good random number generator.
	 */
	if (pingflags & F_PING_RANDOM)
		rnd_fill();
}


static void pr_pack_sub(int cc, char *addr, int seqno, int dupflag, int ttl, long triptime)
{
	jiggle_flush(1);

	if (pingflags & F_FLOOD)
		return;

	printf("%d bytes from %s: icmp_seq=%u", cc, addr, seqno);
	if (dupflag)
		printf(" DUP!");
	printf(" ttl=%d", ttl);
	if (pingflags & (F_TIMING | F_TIMING64))
	{
		printf(" time=%ld.%03ld ms", triptime / 1000, triptime % 1000);
	}

	/*
	 * Send beep to stderr, since that's more likely than stdout
	 * to go to a terminal..
	 */
	if ((pingflags & F_AUDIBLE) && !dupflag)
		fprintf(stderr, "\a");
}


/*
 * Print out the packet, if it came from us.  This logic is necessary
 * because ALL readers of the ICMP socket get a copy of ALL ICMP packets
 * which arrive ('tis only fair).  This permits multiple copies of this
 * program to be run without having intermingled output (or statistics!).
 */
static void pr_pack(uint8_t *buf, int tot_len, struct sockaddr_in *from)
{
	struct ip *ip;
	struct icmp *icp;
	int i;
	int j;
	int net_len;
	uint8_t *cp;
	static int old_rrlen;
	static char old_rr[MAX_IPOPTLEN];
	int hlen;
	int dupflag = 0;
	int dumped;
	long triptime = 0;

#define PR_PACK_SUB() {if (!dumped) {			\
	dumped = 1;					\
	pr_pack_sub(net_len, inet_ntoa(from->sin_addr),	\
		    ntohs((uint16_t)icp->icmp_seq),	\
		    dupflag, ip->ip_ttl, triptime);}}

	/* Check the IP header */
	ip = (struct ip *) buf;
	hlen = ip->ip_hl << 2;
	if (tot_len < hlen + ICMP_MINLEN)
	{
		if (pingflags & F_VERBOSE)
		{
			jiggle_flush(1);
			printf("packet too short (%d bytes) from %s\n", tot_len, inet_ntoa(from->sin_addr));
		}
		return;
	}

	/* Now the ICMP part */
	dumped = 0;
	net_len = tot_len - hlen;
	icp = (struct icmp *) (buf + hlen);
	if (icp->icmp_type == ICMP_ECHOREPLY && icp->icmp_id == ident)
	{

		if (icp->icmp_seq == htons((uint16_t) (ntransmitted - 1)))
			lastrcvd = 1;
		last_rx = now;
		if (first_rx.tv_sec == 0)
			first_rx = last_rx;
		nreceived++;
		if (pingflags & (F_TIMING | F_TIMING64))
		{
			struct timeval tv;

			if (pingflags & F_TIMING)
			{
				struct tv32 tv32;

				memcpy(&tv32, icp->icmp_data, sizeof(tv32));
				tv.tv_sec = (uint32_t) ntohl(tv32.tv32_sec);
				tv.tv_usec = ntohl(tv32.tv32_usec);
			} else if (pingflags & F_TIMING64)
			{
				memcpy(&tv, icp->icmp_data, sizeof(tv));
			} else
			{
				memset(&tv, 0, sizeof(tv));	/* XXX: gcc */
			}

			triptime = diffsec(&last_rx, &tv);
			tsum += triptime;
			tsumsq += triptime * triptime;
			if (triptime < tmin)
				tmin = triptime;
			if (triptime > tmax)
				tmax = triptime;
		}

		if (TST(ntohs((uint16_t) icp->icmp_seq)))
		{
			nrepeats++;
			nreceived--;
			dupflag = 1;
		} else
		{
			SET(ntohs((uint16_t) icp->icmp_seq));
		}

		if (tot_len != opack_ip->ip_len)
		{
			PR_PACK_SUB();
			switch (opack_ip->ip_len - tot_len)
			{
			case MAX_IPOPTLEN:
				if ((pongflags & F_RECORD_ROUTE) != 0)
					break;
				if ((pingflags & F_RECORD_ROUTE) == 0)
					goto out;
				pongflags |= F_RECORD_ROUTE;
				printf("\nremote host does not support record route");
				break;
			case 8:
				if ((pongflags & F_SOURCE_ROUTE) != 0)
					break;
				if ((pingflags & F_SOURCE_ROUTE) == 0)
					goto out;
				pongflags |= F_SOURCE_ROUTE;
				printf("\nremote host does not support source route");
				break;
			default:
			  out:
				printf("\nwrong total length %d instead of %d", tot_len, opack_ip->ip_len);
				break;
			}
		}

		if (!dupflag)
		{
			static uint16_t last_seqno = 0xffff;
			uint16_t seqno = ntohs((uint16_t) icp->icmp_seq);
			uint16_t gap = seqno - (last_seqno + 1);

			if (gap > 0 && gap < 0x8000 && (pingflags & F_VERBOSE))
			{
				printf("[*** sequence gap of %u packets from %u ... %u ***]\n", gap,
							  (uint16_t) (last_seqno + 1), (uint16_t) (seqno - 1));
				if (pingflags & F_QUIET)
					summary(0);
			}

			if (gap < 0x8000)
				last_seqno = seqno;
		}

		if (pingflags & F_QUIET)
			return;

		if (!(pingflags & F_FLOOD))
			PR_PACK_SUB();

		/* check the data */
		if ((size_t) (tot_len - hlen) >
			offsetof(struct icmp, icmp_data) + datalen
			&& !(pingflags & F_PING_RANDOM)
			&& memcmp(icp->icmp_data + phdrlen, opack_icmp.icmp_data + phdrlen, datalen - phdrlen))
		{
			for (i = phdrlen; i < datalen; i++)
			{
				if (icp->icmp_data[i] != opack_icmp.icmp_data[i])
					break;
			}
			PR_PACK_SUB();
			printf("\nwrong data byte #%d should have been"
						  " %#x but was %#x", i - phdrlen,
						  (uint8_t) opack_icmp.icmp_data[i], (uint8_t) icp->icmp_data[i]);
			for (i = phdrlen; i < datalen; i++)
			{
				if ((i % 16) == 0)
					printf("\n\t");
				printf("%2x ", (uint8_t) icp->icmp_data[i]);
			}
		}

	} else
	{
		if (!pr_icmph(icp, from, net_len))
			return;
		dumped = 2;
	}

	/* Display any IP options */
	cp = buf + sizeof(struct ip);
	while (hlen > (int) sizeof(struct ip))
	{
		switch (*cp)
		{
		case IPOPT_EOL:
			hlen = 0;
			break;
		case IPOPT_LSRR:
			hlen -= 2;
			j = *++cp;
			++cp;
			j -= IPOPT_MINOFF;
			if (j <= 0)
				continue;
			if (dumped <= 1)
			{
				j = ((j + 3) / 4) * 4;
				hlen -= j;
				cp += j;
				break;
			}
			PR_PACK_SUB();
			printf("\nLSRR: ");
			for (;;)
			{
				pr_saddr(cp);
				cp += 4;
				hlen -= 4;
				j -= 4;
				if (j <= 0)
					break;
				putchar('\n');
			}
			break;
		case IPOPT_RR:
			j = *++cp;					/* get length */
			i = *++cp;					/* and pointer */
			hlen -= 2;
			if (i > j)
				i = j;
			i -= IPOPT_MINOFF;
			if (i <= 0)
				continue;
			if (dumped <= 1)
			{
				if (i == old_rrlen && !memcmp(cp, old_rr, i))
				{
					if (dumped)
						printf("\t(same route)");
					j = ((i + 3) / 4) * 4;
					hlen -= j;
					cp += j;
					break;
				}
				old_rrlen = i;
				memcpy(old_rr, cp, i);
			}
			if (!dumped)
			{
				jiggle_flush(1);
				printf("RR: ");
				dumped = 1;
			} else
			{
				printf("\nRR: ");
			}
			for (;;)
			{
				pr_saddr(cp);
				cp += 4;
				hlen -= 4;
				i -= 4;
				if (i <= 0)
					break;
				putchar('\n');
			}
			break;
		case IPOPT_NOP:
			if (dumped <= 1)
				break;
			PR_PACK_SUB();
			printf("\nNOP");
			break;
		default:
			PR_PACK_SUB();
			printf("\nunknown option 0x%x", *cp);
			break;
		}
		hlen--;
		cp++;
	}

	if (dumped)
	{
		putchar('\n');
		fflush(stdout);
	} else
	{
		jiggle(-1);
	}
}


static int doit(void)
{
	int cc;
	struct sockaddr_in from;
	socklen_t fromlen;
	long sec;
	long last;
	long d_last;
	struct pollfd fdmaskp[1];

	clk_gettime(&clear_cache);
	if (maxwait != 0)
	{
		last = timeval_to_sec(&clear_cache) + maxwait;
		d_last = 0;
	} else
	{
		last = 0;
		d_last = 24 * 60 * 60 * 1000L;
	}

	do
	{
		clk_gettime(&now);

		if (last != 0)
			d_last = last - timeval_to_sec(&now);

		if (ntransmitted < npackets && d_last > 0)
		{
			/* send if within 100 usec or late for next packet */
			sec = diffsec(&next_tx, &now);
			if (sec <= 100 || (lastrcvd && (pingflags & F_FLOOD)))
			{
				pinger();
				sec = diffsec(&next_tx, &now);
			}
			if (sec < 0)
				sec = 0;
			if (d_last < sec)
				sec = d_last;
		} else
		{
			/* For the last response, wait twice as long as the
			 * worst case seen, or 10 times as long as the
			 * maximum interpacket interval, whichever is longer.
			 */
			sec = 2 * tmax;
			if (10 * interval > sec)
				sec = 10 * interval;
			sec -= diffsec(&now, &last_tx);
			if (d_last < sec)
				sec = d_last;
			if (sec <= 0)
				break;
		}

		fdmaskp[0].fd = s;
		fdmaskp[0].events = POLLIN;

		enablesignals();
		cc = poll(fdmaskp, 1, sec / 1000);
		blocksignals();

		if (cc <= 0)
		{
			if (cc < 0)
			{
				if (errno == EINTR)
					continue;
				jiggle_flush(1);
				fprintf(stderr, "%s: poll: %s\n", progname, mint_strerror(errno));
				return FALSE;
			}
			continue;
		}

		fromlen = sizeof(from);
		cc = recvfrom(s, (char *) packet, packlen, 0, (struct sockaddr *) &from, &fromlen);
		if (cc < 0)
		{
			if (errno != EINTR)
			{
				jiggle_flush(1);
				fprintf(stderr, "%s: recvfrom: %s\n", progname, mint_strerror(errno));
				fflush(stderr);
			}
			continue;
		}
		clk_gettime(&now);
		pr_pack(packet, cc, &from);

	} while (nreceived < npackets && (nreceived == 0 || !(pingflags & F_ONCE)));
	
	return TRUE;
}


static int gethost(const char *arg, const char *name, struct sockaddr_in *sa, char *realname, int realname_len)
{
	struct hostent *hp;

	memset(sa, 0, sizeof(*sa));
	sa->sin_family = AF_INET;

	/* If it is an IP address, try to convert it to a name to
	 * have something nice to display.
	 */
	if (inet_aton(name, &sa->sin_addr) != 0)
	{
		if (realname)
		{
			if (pingflags & F_NUMERIC)
				hp = 0;
			else
				hp = gethostbyaddr((char *) &sa->sin_addr, sizeof(sa->sin_addr), AF_INET);
			strncpy(realname, hp ? hp->h_name : name, realname_len - 1);
			realname[realname_len - 1] = '\0';
		}
		return TRUE;
	}

	hp = gethostbyname(name);
	if (hp == NULL)
	{
		fprintf(stderr, "%s: Cannot resolve \"%s\" (%s)\n", progname, name, hstrerror(h_errno));
		return FALSE;
	}

	if (hp->h_addrtype != AF_INET)
	{
		fprintf(stderr, "%s: %s only supported with IP\n", progname, arg);
		return FALSE;
	}

	memcpy(&sa->sin_addr, hp->h_addr, sizeof(sa->sin_addr));

	if (realname)
	{
		strncpy(realname, hp->h_name, realname_len - 1);
		realname[realname_len - 1] = '\0';
	}
	
	return TRUE;
}



static void usage(void)
{
	fprintf(stderr, "usage: \n"
				   "%s [-aCDdfLnoPQqRrv] [-c count] [-g gateway] [-h host]"
				   " [-I addr] [-i interval]\n"
				   "     [-l preload] [-p pattern] [-s size] [-T ttl] [-t tos]"
				   " [-w maxwait] host\n", progname);
}



int main(int argc, char *argv[])
{
	int c;
	int i;
	long on = 1;
	int hostind = 0;
	long l;
	int len = -1;
	int compat = 0;
	long ttl = 0;
	uint32_t tos = 0;
	char *p;

#ifdef SIGINFO
	struct sigaction sa;
#endif

#ifdef __atarist__
	Pdomain(1);
#endif

	if ((s = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP)) < 0)
	{
		fprintf(stderr, "%s: Cannot create socket: %s\n", progname, mint_strerror(errno));
		return EXIT_FAILURE;
	}

	if (setuid(getuid()) == -1)
	{
		fprintf(stderr, "%s: setuid: %s\n", progname, mint_strerror(errno));
		return EXIT_FAILURE;
	}

	while ((c = getopt(argc, argv, "ac:CdDfg:h:i:I:l:Lnop:PqQrRs:t:T:vw:")) != -1)
	{
		switch (c)
		{
		case 'a':
			pingflags |= F_AUDIBLE;
			break;
		case 'C':
			compat = 1;
			break;
		case 'c':
			l = strtol(optarg, &p, 0);
			if (*p != '\0' || l <= 0)
			{
				fprintf(stderr, "%s: Bad/invalid number of packets: %s\n", progname, optarg);
				return EXIT_FAILURE;
			}
#if INT_MAX < LONG_MAX
			if (l > INT_MAX)
			{
				fprintf(stderr, "%s: Too many packets to count: %ld\n", progname, l);
				return EXIT_FAILURE;
			}
#endif
			npackets = (int)l;
			break;
		case 'D':
			pingflags |= F_DF;
			break;
		case 'd':
			options |= SO_DEBUG;
			break;
		case 'f':
			pingflags |= F_FLOOD;
			break;
		case 'h':
			hostind = optind - 1;
			break;
		case 'i':						/* wait between sending packets */
			interval = strtol(optarg, &p, 0);
			if (*p != '\0' || interval <= 0)
			{
				fprintf(stderr, "%s: Bad/invalid interval: %s\n", progname, optarg);
				return EXIT_FAILURE;
			}
			/*
			 * In order to avoid overflowing the microseconds
			 * argument of poll() the interval must be less than
			 * INT_MAX/1000. Limit it to one second less than
			 * that to be safe.
			 */
			if (interval >= LONG_MAX / 1000 - 1)
			{
				fprintf(stderr, "%s: Timing interval %ld too large\n", progname, interval);
				return EXIT_FAILURE;
			}
			break;
		case 'l':
			l = strtol(optarg, &p, 0);
			if (*p != '\0' || l < 0)
			{
				fprintf(stderr, "%s: Bad/invalid preload value: %s\n", progname, optarg);
				return EXIT_FAILURE;
			}
#if INT_MAX < LONG_MAX
			if (l > INT_MAX)
			{
				fprintf(stderr, "%s: Too many preload packets: %ld\n", progname, l);
				return EXIT_FAILURE;
			}
#endif
			preload = (int)l;
			break;
		case 'n':
			pingflags |= F_NUMERIC;
			break;
		case 'o':
			pingflags |= F_ONCE;
			break;
		case 'p':						/* fill buffer with user pattern */
			if (pingflags & F_PING_RANDOM)
			{
				fprintf(stderr, "%s: Only one of -P and -p allowed\n", progname);
				return EXIT_FAILURE;
			}
			pingflags |= F_PING_FILLED;
			fill_pat = optarg;
			break;
		case 'P':
			if (pingflags & F_PING_FILLED)
			{
				fprintf(stderr, "%s: Only one of -P and -p allowed\n", progname);
				return EXIT_FAILURE;
			}
			pingflags |= F_PING_RANDOM;
			break;
		case 'q':
			pingflags |= F_QUIET;
			break;
		case 'Q':
			pingflags |= F_SEMI_QUIET;
			break;
		case 'r':
			options |= SO_DONTROUTE;
			break;
		case 's':						/* size of packet to send */
			l = strtol(optarg, &p, 0);
			if (*p != '\0' || l < 0)
			{
				fprintf(stderr, "%s: Bad/invalid packet size: %s\n", progname, optarg);
				return EXIT_FAILURE;
			}
			if ((unsigned long)l > MAXPACKET)
			{
				fprintf(stderr, "%s: packet size is too large\n", progname);
				return EXIT_FAILURE;
			}
			len = (int) l;
			break;
		case 'v':
			pingflags |= F_VERBOSE;
			break;
		case 'R':
			pingflags |= F_RECORD_ROUTE;
			break;
		case 'L':
			pingflags |= F_MCAST_NOLOOP;
			break;
		case 't':
			tos = strtoul(optarg, &p, 0);
			if (*p != '\0' || tos > 0xFF)
			{
				fprintf(stderr, "%s: bad tos value: %s\n", progname, optarg);
				return EXIT_FAILURE;
			}
			break;
		case 'T':
			l = strtol(optarg, &p, 0);
			if (*p != '\0' || l > 255 || l <= 0)
			{
				fprintf(stderr, "%s: ttl out of range: %s\n", progname, optarg);
				return EXIT_FAILURE;
			}
			ttl = (uint8_t) l;			/* cannot check >255 otherwise */
			break;
		case 'I':
			pingflags |= F_SOURCE_ADDR;
			if (gethost("-I", optarg, &src_addr, 0, 0) == FALSE)
				return EXIT_FAILURE;
			break;
		case 'g':
			pingflags |= F_SOURCE_ROUTE;
			gethost("-g", optarg, &send_addr, 0, 0);
			break;
		case 'w':
			maxwait = strtol(optarg, &p, 0);
			if (*p != '\0' || maxwait <= 0)
			{
				fprintf(stderr, "%s: Bad/invalid maxwait time: %s\n", progname, optarg);
				return EXIT_FAILURE;
			}
			break;
		default:
			usage();
			return EXIT_FAILURE;
		}
	}

	if (interval == 0)
		interval = (pingflags & F_FLOOD) ? FLOOD_INTVL : 1000000L;
	if ((pingflags & F_FLOOD) && getuid() != 0)
	{
		fprintf(stderr, "%s: Must be superuser to use -f\n", progname);
		return EXIT_FAILURE;
	}
	if (interval < 1000000L && getuid() != 0)
	{
		fprintf(stderr, "%s: Must be superuser to use < 1 sec ping interval\n", progname);
		return EXIT_FAILURE;
	}
	if (preload > 0 && getuid() != 0)
	{
		fprintf(stderr, "%s: Must be superuser to use -l\n", progname);
		return EXIT_FAILURE;
	}
	
	usec_to_timeval(interval, &interval_tv);
	if (interval_tv.tv_sec == 0 && interval_tv.tv_usec == 0)
	{
		fprintf(stderr, "%s: Packet interval must be at least 1 ns\n", progname);
		return EXIT_FAILURE;
	}

	if ((pingflags & (F_AUDIBLE | F_FLOOD)) == (F_AUDIBLE | F_FLOOD))
		fprintf(stderr, "%s: Sorry, no audible output for flood pings\n", progname);

	if (npackets != 0)
	{
		npackets += preload;
	} else
	{
		npackets = INT_MAX;
	}

	if (hostind == 0)
	{
		if (optind != argc - 1)
		{
			usage();
			return EXIT_FAILURE;
		}
		hostind = optind;
	} else if (hostind >= argc - 1)
	{
		usage();
		return EXIT_FAILURE;
	}

	gethost("", argv[hostind], &whereto, hostname, (int)sizeof(hostname));
	if (IN_MULTICAST(ntohl(whereto.sin_addr.s_addr)))
		pingflags |= F_MCAST;
	if (!(pingflags & F_SOURCE_ROUTE))
		memcpy(&send_addr, &whereto, sizeof(send_addr));

	loc_addr.sin_family = AF_INET;
	loc_addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);

	if (len != -1)
		datalen = len;
	else
		datalen = 64 - (int)PHDR_LEN;
	if (!compat && datalen >= (int) PHDR64_LEN)
	{									/* can we time them? */
		pingflags |= F_TIMING64;
		phdrlen = (int)PHDR64_LEN;
	} else if (datalen >= (int) PHDR_LEN)
	{									/* can we time them? */
		pingflags |= F_TIMING;
		phdrlen = (int)PHDR_LEN;
	} else
	{
		phdrlen = 0;
	}

	packlen = datalen + 60 + 76;		/* MAXIP + MAXICMP */
	if ((packet = malloc(packlen)) == NULL)
	{
		fprintf(stderr, "%s: Can't allocate %d bytes: %s\n", progname, packlen, mint_strerror(errno));
		return EXIT_FAILURE;
	}

	if (pingflags & F_PING_FILLED)
	{
		if (fill() == FALSE)
			return EXIT_FAILURE;
	} else if (pingflags & F_PING_RANDOM)
	{
		rnd_fill();
	} else
	{
		for (i = phdrlen; i < datalen; i++)
			opack_icmp.icmp_data[i] = i;
	}

	ident = random() & 0xFFFF;

	if (options & SO_DEBUG)
	{
		if (setsockopt(s, SOL_SOCKET, SO_DEBUG, (char *) &on, sizeof(on)) == -1)
			fprintf(stderr, "%s: Can't turn on socket debugging: %s\n", progname, mint_strerror(errno));
	}
	if (options & SO_DONTROUTE)
	{
		if (setsockopt(s, SOL_SOCKET, SO_DONTROUTE, (char *) &on, sizeof(on)) == -1)
			fprintf(stderr, "%s: SO_DONTROUTE: %s\n", progname, mint_strerror(errno));
	}

	if (pingflags & F_SOURCE_ROUTE)
	{
		optspace[IPOPT_OPTVAL] = IPOPT_LSRR;
		optspace[IPOPT_OLEN] = optlen = 7;
		optspace[IPOPT_OFFSET] = IPOPT_MINOFF;
		memcpy(&optspace[IPOPT_MINOFF - 1], &whereto.sin_addr, sizeof(whereto.sin_addr));
		optspace[optlen++] = IPOPT_NOP;
	}
	if (pingflags & F_RECORD_ROUTE)
	{
		optspace[optlen + IPOPT_OPTVAL] = IPOPT_RR;
		optspace[optlen + IPOPT_OLEN] = (MAX_IPOPTLEN - 1 - optlen);
		optspace[optlen + IPOPT_OFFSET] = IPOPT_MINOFF;
		optlen = MAX_IPOPTLEN;
	}
	/* this leaves opack_ip 0(mod 4) aligned */
	opack_ip = (struct ip *) ((char *) &out_pack.o_ip + sizeof(out_pack.o_opt) - optlen);
	memcpy(opack_ip + 1, optspace, optlen);

	if (setsockopt(s, IPPROTO_IP, IP_HDRINCL, (char *) &on, sizeof(on)) < 0)
	{
		fprintf(stderr, "%s: Can't set special IP header: %s\n", progname, mint_strerror(errno));
		return EXIT_FAILURE;
	}

	opack_ip->ip_v = IPVERSION;
	opack_ip->ip_hl = (unsigned int)(sizeof(struct ip) + optlen) >> 2;
	opack_ip->ip_tos = (unsigned char)tos;
	opack_ip->ip_off = (pingflags & F_DF) ? IP_DF : 0;
	opack_ip->ip_ttl = ttl ? ttl : MAXTTL;
	opack_ip->ip_p = IPPROTO_ICMP;
	opack_ip->ip_src = src_addr.sin_addr;
	opack_ip->ip_dst = send_addr.sin_addr;

	if (pingflags & F_MCAST)
	{
		if (pingflags & F_MCAST_NOLOOP)
		{
			long loop = 0;

			/*
			 * Note: MiNT 1.19 has a bug and expects a value len of 1,
			 * but fetches a long from the argument
			 */
			if (setsockopt(s, IPPROTO_IP, IP_MULTICAST_LOOP, (char *) &loop, 1) < 0)
			{
				fprintf(stderr, "%s: Can't disable multicast loopback: %s\n", progname, mint_strerror(errno));
				return EXIT_FAILURE;
			}
		}

		if (ttl != 0 && setsockopt(s, IPPROTO_IP, IP_MULTICAST_TTL, (char *) &ttl, sizeof(ttl)) < 0)
		{
			fprintf(stderr, "%s: Can't set multicast time-to-live: %s\n", progname, mint_strerror(errno));
			return EXIT_FAILURE;
		}
		
		if ((pingflags & F_SOURCE_ADDR) &&
			setsockopt(s, IPPROTO_IP, IP_MULTICAST_IF,
							   &src_addr.sin_addr, sizeof(src_addr.sin_addr)) < 0)
		{
			fprintf(stderr, "%s: Can't set multicast source interface: %s\n", progname, mint_strerror(errno));
			return EXIT_FAILURE;
		}
	} else if (pingflags & F_SOURCE_ADDR)
	{
		if (setsockopt(s, IPPROTO_IP, IP_MULTICAST_IF, &src_addr.sin_addr, sizeof(src_addr.sin_addr)) < 0)
		{
			fprintf(stderr, "%s: Can't set source interface/address: %s\n", progname, mint_strerror(errno));
			return EXIT_FAILURE;
		}
	}

	printf("PING %s (%s): %d data bytes\n", hostname, inet_ntoa(whereto.sin_addr), datalen);

	/* When pinging the broadcast address, you can get a lot
	 * of answers.  Doing something so evil is useful if you
	 * are trying to stress the ethernet, or just want to
	 * fill the arp cache to get some stuff for /etc/ethers.
	 */
	bufspace = IP_MAXPACKET;
	while (0 > setsockopt(s, SOL_SOCKET, SO_RCVBUF, (char *) &bufspace, sizeof(bufspace)))
	{
		if ((bufspace -= 4096) <= 0)
		{
			fprintf(stderr, "%s: Cannot set the receive buffer size: %s\n", progname, mint_strerror(errno));
			return EXIT_FAILURE;
		}
	}

	/* make it possible to send giant probes, but do not worry now
	 * if it fails, since we probably won't send giant probes.
	 */
	setsockopt(s, SOL_SOCKET, SO_SNDBUF, (char *) &bufspace, sizeof(bufspace));

#ifdef __atarist__
	Psignal(SIGINT, prefinish);
#else
	signal(SIGINT, prefinish);
#endif

	/*
	 * Set up two signal masks:
	 *    - blockmask blocks the signals we catch
	 *    - enablemask does not
	 */

#ifdef __atarist__
	enablemask = 0;
	blockmask = __sigmask(SIGQUIT);
	Psignal(SIGQUIT, prtsig);
	Psignal(SIGCONT, prtsig);
#else
	sigemptyset(&enablemask);
	sigemptyset(&blockmask);
	sigaddset(&blockmask, SIGINT);
#ifdef SIGINFO
	sigaddset(&blockmask, SIGINFO);
#else
	sigaddset(&blockmask, SIGQUIT);
#endif

#ifdef SIGINFO
	sa.sa_handler = prtsig;
	sa.sa_flags = SA_NOKERNINFO;
	sigemptyset(&sa.sa_mask);
	sigaction(SIGINFO, &sa, NULL);
#else
	signal(SIGQUIT, prtsig);
#endif
	signal(SIGCONT, prtsig);
#endif

	blocksignals();

	/* fire off them quickies */
	for (i = 0; i < preload; i++)
	{
		clk_gettime(&now);
		pinger();
	}

	if (doit() == FALSE)
		return EXIT_FAILURE;
	finish(0);

	return EXIT_SUCCESS;
}
