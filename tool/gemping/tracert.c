/*
 * Copyright (c) 1988, 1989, 1991, 1994, 1995, 1996, 1997, 1998, 1999, 2000
 *	The Regents of the University of California.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that: (1) source code distributions
 * retain the above copyright notice and this paragraph in its entirety, (2)
 * distributions including binary code include the above copyright notice and
 * this paragraph in its entirety in the documentation or other materials
 * provided with the distribution, and (3) all advertising materials mentioning
 * features or use of this software display the following acknowledgement:
 * ``This product includes software developed by the University of California,
 * Lawrence Berkeley Laboratory and its contributors.'' Neither the name of
 * the University nor the names of its contributors may be used to endorse
 * or promote products derived from this software without specific prior
 * written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

/*
 * traceroute host  - trace the route ip packets follow going to "host".
 *
 * Attempt to trace the route an ip packet would follow to some
 * internet host.  We find out intermediate hops by launching probe
 * packets with a small ttl (time to live) then listening for an
 * icmp "time exceeded" reply from a gateway.  We start our probes
 * with a ttl of one and increase by one until we get an icmp "port
 * unreachable" (which means we got to "host") or hit a max (which
 * defaults to 30 hops & can be changed with the -m flag).  Three
 * probes (change with -q flag) are sent at each ttl setting and a
 * line is printed showing the ttl, address of the gateway and
 * round trip time of each probe.  If the probe answers come from
 * different gateways, the address of each responding system will
 * be printed.  If there is no response within a 5 sec. timeout
 * interval (changed with the -w flag), a "*" is printed for that
 * probe.
 *
 * Probe packets are UDP format.  We don't want the destination
 * host to process them so the destination port is set to an
 * unlikely value (if some clod on the destination is using that
 * value, it can be changed with the -p flag).
 *
 * A sample use might be:
 *
 *     [yak 71]% traceroute nis.nsf.net.
 *     traceroute to nis.nsf.net (35.1.1.48), 30 hops max, 56 byte packet
 *      1  helios.ee.lbl.gov (128.3.112.1)  19 ms  19 ms  0 ms
 *      2  lilac-dmc.Berkeley.EDU (128.32.216.1)  39 ms  39 ms  19 ms
 *      3  lilac-dmc.Berkeley.EDU (128.32.216.1)  39 ms  39 ms  19 ms
 *      4  ccngw-ner-cc.Berkeley.EDU (128.32.136.23)  39 ms  40 ms  39 ms
 *      5  ccn-nerif22.Berkeley.EDU (128.32.168.22)  39 ms  39 ms  39 ms
 *      6  128.32.197.4 (128.32.197.4)  40 ms  59 ms  59 ms
 *      7  131.119.2.5 (131.119.2.5)  59 ms  59 ms  59 ms
 *      8  129.140.70.13 (129.140.70.13)  99 ms  99 ms  80 ms
 *      9  129.140.71.6 (129.140.71.6)  139 ms  239 ms  319 ms
 *     10  129.140.81.7 (129.140.81.7)  220 ms  199 ms  199 ms
 *     11  nic.merit.edu (35.1.1.48)  239 ms  239 ms  239 ms
 *
 * Note that lines 2 & 3 are the same.  This is due to a buggy
 * kernel on the 2nd hop system -- lbl-csam.arpa -- that forwards
 * packets with a zero ttl.
 *
 * A more interesting example is:
 *
 *     [yak 72]% traceroute allspice.lcs.mit.edu.
 *     traceroute to allspice.lcs.mit.edu (18.26.0.115), 30 hops max
 *      1  helios.ee.lbl.gov (128.3.112.1)  0 ms  0 ms  0 ms
 *      2  lilac-dmc.Berkeley.EDU (128.32.216.1)  19 ms  19 ms  19 ms
 *      3  lilac-dmc.Berkeley.EDU (128.32.216.1)  39 ms  19 ms  19 ms
 *      4  ccngw-ner-cc.Berkeley.EDU (128.32.136.23)  19 ms  39 ms  39 ms
 *      5  ccn-nerif22.Berkeley.EDU (128.32.168.22)  20 ms  39 ms  39 ms
 *      6  128.32.197.4 (128.32.197.4)  59 ms  119 ms  39 ms
 *      7  131.119.2.5 (131.119.2.5)  59 ms  59 ms  39 ms
 *      8  129.140.70.13 (129.140.70.13)  80 ms  79 ms  99 ms
 *      9  129.140.71.6 (129.140.71.6)  139 ms  139 ms  159 ms
 *     10  129.140.81.7 (129.140.81.7)  199 ms  180 ms  300 ms
 *     11  129.140.72.17 (129.140.72.17)  300 ms  239 ms  239 ms
 *     12  * * *
 *     13  128.121.54.72 (128.121.54.72)  259 ms  499 ms  279 ms
 *     14  * * *
 *     15  * * *
 *     16  * * *
 *     17  * * *
 *     18  ALLSPICE.LCS.MIT.EDU (18.26.0.115)  339 ms  279 ms  279 ms
 *
 * (I start to see why I'm having so much trouble with mail to
 * MIT.)  Note that the gateways 12, 14, 15, 16 & 17 hops away
 * either don't send ICMP "time exceeded" messages or send them
 * with a ttl too small to reach us.  14 - 17 are running the
 * MIT C Gateway code that doesn't send "time exceeded"s.  God
 * only knows what's going on with 12.
 *
 * The silent gateway 12 in the above may be the result of a bug in
 * the 4.[23]BSD network code (and its derivatives):  4.x (x <= 3)
 * sends an unreachable message using whatever ttl remains in the
 * original datagram.  Since, for gateways, the remaining ttl is
 * zero, the icmp "time exceeded" is guaranteed to not make it back
 * to us.  The behavior of this bug is slightly more interesting
 * when it appears on the destination system:
 *
 *      1  helios.ee.lbl.gov (128.3.112.1)  0 ms  0 ms  0 ms
 *      2  lilac-dmc.Berkeley.EDU (128.32.216.1)  39 ms  19 ms  39 ms
 *      3  lilac-dmc.Berkeley.EDU (128.32.216.1)  19 ms  39 ms  19 ms
 *      4  ccngw-ner-cc.Berkeley.EDU (128.32.136.23)  39 ms  40 ms  19 ms
 *      5  ccn-nerif35.Berkeley.EDU (128.32.168.35)  39 ms  39 ms  39 ms
 *      6  csgw.Berkeley.EDU (128.32.133.254)  39 ms  59 ms  39 ms
 *      7  * * *
 *      8  * * *
 *      9  * * *
 *     10  * * *
 *     11  * * *
 *     12  * * *
 *     13  rip.Berkeley.EDU (128.32.131.22)  59 ms !  39 ms !  39 ms !
 *
 * Notice that there are 12 "gateways" (13 is the final
 * destination) and exactly the last half of them are "missing".
 * What's really happening is that rip (a Sun-3 running Sun OS3.5)
 * is using the ttl from our arriving datagram as the ttl in its
 * icmp reply.  So, the reply will time out on the return path
 * (with no notice sent to anyone since icmp's aren't sent for
 * icmp's) until we probe with a ttl that's at least twice the path
 * length.  I.e., rip is really only 7 hops away.  A reply that
 * returns with a ttl of 1 is a clue this problem exists.
 * Traceroute prints a "!" after the time if the ttl is <= 1.
 * Since vendors ship a lot of obsolete (DEC's Ultrix, Sun 3.x) or
 * non-standard (HPUX) software, expect to see this problem
 * frequently and/or take care picking the target host of your
 * probes.
 *
 * Other possible annotations after the time are !H, !N, !P (got a host,
 * network or protocol unreachable, respectively), !S or !F (source
 * route failed or fragmentation needed -- neither of these should
 * ever occur and the associated gateway is busted if you see one).  If
 * almost all the probes result in some kind of unreachable, traceroute
 * will give up and exit.
 *
 * Notes
 * -----
 * This program must be run by root or be setuid.  (I suggest that
 * you *don't* make it setuid -- casual use could result in a lot
 * of unnecessary traffic on our poor, congested nets.)
 *
 * This program requires a kernel mod that does not appear in any
 * system available from Berkeley:  A raw ip socket using proto
 * IPPROTO_RAW must interpret the data sent as an ip datagram (as
 * opposed to data to be wrapped in a ip datagram).  See the README
 * file that came with the source to this program for a description
 * of the mods I made to /sys/netinet/raw_ip.c.  Your mileage may
 * vary.  But, again, ANY 4.x (x < 4) BSD KERNEL WILL HAVE TO BE
 * MODIFIED TO RUN THIS PROGRAM.
 *
 * The udp port usage may appear bizarre (well, ok, it is bizarre).
 * The problem is that an icmp message only contains 8 bytes of
 * data from the original datagram.  8 bytes is the size of a udp
 * header so, if we want to associate replies with the original
 * datagram, the necessary information must be encoded into the
 * udp header (the ip id could be used but there's no way to
 * interlock with the kernel's assignment of ip id's and, anyway,
 * it would have taken a lot more kernel hacking to allow this
 * code to set the ip id).  So, to allow two or more users to
 * use traceroute simultaneously, we use this task's pid as the
 * source port (the high bit is set to move the port number out
 * of the "likely" range).  To keep track of which probe is being
 * replied to (so times and/or hop counts don't get confused by a
 * reply that was delayed in transit), we increment the destination
 * port number before each probe.
 *
 * Don't use this as a coding example.  I was trying to find a
 * routing problem and this code sort-of popped out after 48 hours
 * without sleep.  I was amazed it ever compiled, much less ran.
 *
 * I stole the idea for this program from Steve Deering.  Since
 * the first release, I've learned that had I attended the right
 * IETF working group meetings, I also could have stolen it from Guy
 * Almes or Matt Mathis.  I don't know (or care) who came up with
 * the idea first.  I envy the originators' perspicacity and I'm
 * glad they didn't keep the idea a secret.
 *
 * Tim Seaver, Ken Adelman and C. Philip Wood provided bug fixes and/or
 * enhancements to the original distribution.
 *
 * I've hacked up a round-trip-route version of this that works by
 * sending a loose-source-routed udp datagram through the destination
 * back to yourself.  Unfortunately, SO many gateways botch source
 * routing, the thing is almost worthless.  Maybe one day...
 *
 *  -- Van Jacobson (van@ee.lbl.gov)
 *     Tue Dec 20 03:50:13 PST 1988
 */

#include <sys/param.h>
#include <sys/file.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/time.h>
#ifdef __MINT__
#include <mint/sysctl.h>
#else
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#pragma GCC diagnostic ignored "-Wcpp"
#include <sys/sysctl.h>
#pragma GCC diagnostic warning "-Wcpp"
#endif

#include <net/if.h>
#include <netinet/in_systm.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <netinet/udp.h>
#include <arpa/inet.h>

#ifdef __PUREC__
char *mint_strerror(int errnum);
#else
#define mint_strerror(err) strerror(err)
#endif


#define USE_UDP 1


#ifdef linux
#define ICMP_FILTER			1

struct icmp_filter
{
	unsigned int data;
};
#endif

#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <poll.h>

/* rfc1716 */
#ifndef ICMP_UNREACH_FILTER_PROHIB
#define ICMP_UNREACH_FILTER_PROHIB	13	/* admin prohibited filter */
#endif
#ifndef ICMP_UNREACH_HOST_PRECEDENCE
#define ICMP_UNREACH_HOST_PRECEDENCE	14	/* host precedence violation */
#endif
#ifndef ICMP_UNREACH_PRECEDENCE_CUTOFF
#define ICMP_UNREACH_PRECEDENCE_CUTOFF	15	/* precedence cutoff */
#endif

#include "ifaddrl.h"
#include "as.h"

#ifdef __PUREC__
#pragma warn -stv /* for inet_ntoa() */
#define random() rand()
#ifndef EMSGSIZE
#define EMSGSIZE 302
#endif
#endif

/* Maximum number of gateways (include room for one noop) */
#define NGATEWAYS ((int)((MAX_IPOPTLEN - IPOPT_MINOFF - 1) / sizeof(u_int32_t)))

#ifndef MAXHOSTNAMELEN
#define MAXHOSTNAMELEN	64
#endif

/* Host name and address list */
struct hostinfo
{
	char *name;
	int n;
	u_int32_t *addrs;
};

/* Data section of the probe packet */
struct outdata
{
	u_char seq;							/* sequence number of this packet */
	u_char ttl;							/* ttl packet left with */
	struct tv32
	{
		int32_t tv32_sec;
		int32_t tv32_usec;
	} tv;								/* time packet left */
};

#ifdef __MINT__
#include <netinet/ip_var.h>
#include <netinet/udp_var.h>
#else
/*
 * Overlay for ip header used by other protocols (tcp, udp).
 */
struct ipovly
{
	uint32_t ih_next;
	uint32_t ih_prev;					/* for protocol sequence q's */
	u_char ih_x1;						/* (unused) */
	u_char ih_pr;						/* protocol */
	short ih_len;						/* protocol length */
	struct in_addr ih_src;				/* source internet address */
	struct in_addr ih_dst;				/* destination internet address */
};

/*
 * UDP kernel structures and variables.
 */
struct udpiphdr
{
	struct ipovly ui_i;					/* overlaid ip structure */
	struct udphdr ui_u;					/* udp header */
};

#define	ui_x1		ui_i.ih_x1
#define	ui_pr		ui_i.ih_pr
#define	ui_len		ui_i.ih_len
#define	ui_src		ui_i.ih_src
#define	ui_dst		ui_i.ih_dst
#define	ui_sport	ui_u.uh_sport
#define	ui_dport	ui_u.uh_dport
#define	ui_ulen		ui_u.uh_ulen
#define	ui_sum		ui_u.uh_sum
#endif

char version[] = "1.4a12";

#ifndef HAVE_ICMP_NEXTMTU
/* Path MTU Discovery (RFC1191) */
struct my_pmtu
{
	u_int16_t ipm_void;
	u_int16_t ipm_nextmtu;
};
#endif

static u_char packet[512];				/* last inbound (icmp) packet */

static struct ip *outip;				/* last output (udp) packet */
static struct udphdr *outudp;			/* last output (udp) packet */
static void *outmark;					/* packed location of struct outdata */
static struct outdata outsetup;			/* setup and copy for alignment */

static struct icmp *outicmp;			/* last output (icmp) packet */

/* loose source route gateway list (including room for final destination) */
static u_int32_t gwlist[NGATEWAYS + 1];

static int recvsock;					/* receive (icmp) socket file descriptor */
static int sndsock;						/* send (udp/icmp) socket file descriptor */

static struct sockaddr whereto;			/* Who to try to reach */
static struct sockaddr wherefrom;		/* Who we are */
static long packlen;					/* total length of packet */
static long minpacket;					/* min ip packet size */

#if !defined(IP_MAXPACKET)
#define	IP_MAXPACKET	(64 * 1024L)
#endif
static long maxpacket = IP_MAXPACKET;	/* max ip packet size */
static int printed_ttl = 0;
static int pmtu;						/* Path MTU Discovery (RFC1191) */
static long pausemsecs;

static const char *prog;
static char *source;
static char *hostname;
static char *device;

static int nprobes = 3;
static int max_ttl;
static int first_ttl = 1;
static u_int16_t ident;
static in_port_t port = 32768U + 666;	/* start udp dest port # for probe packets */

static int options;						/* socket options */
static int verbose;
static long waittime = 5;				/* time to wait for response (in seconds) */
static int nflag;						/* print addresses numerically */
static int dump;
static int Mflag;						/* show MPLS labels if any */
static int as_path;						/* print as numbers for each hop */
static char *as_server = NULL;
static void *asn;
static int useicmp;						/* use icmp echo instead of udp packets */

#ifdef CANT_HACK_CKSUM
static int doipcksum = 0;				/* don't calculate ip checksums by default */
#else
static int doipcksum = 1;				/* calculate ip checksums by default */
#endif
static int optlen;						/* length of ip options */

static int const mtus[] = {
	17914,
	8166,
	4464,
	4352,
	2048,
	2002,
	1536,
	1500,
	1492,
	1480,
	1280,
	1006,
	576,
	552,
	544,
	512,
	508,
	296,
	68,
	0
};

static const int *mtuptr = &mtus[0];
static int mtudisc = 0;
static int nextmtu;						/* from ICMP error, set by packet_ok(), might be 0 */

#ifndef LAUGHTER
static u_int16_t uh_sport;
#endif


/*
 * Checksum routine for Internet Protocol family headers (C Version)
 */
static u_int16_t in_cksum2(uint16_t seed, uint16_t *addr, long len)
{
	long nleft = len;
	const uint16_t *w = addr;
	uint16_t answer;
	uint32_t sum = seed;

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
		sum += htons(*(const uint8_t *) w << 8);

	/*
	 * add back carry outs from top 16 bits to low 16 bits
	 */
	sum = (sum >> 16) + (sum & 0xffff);	/* add hi 16 to low 16 */
	sum += (sum >> 16);					/* add carry */
	answer = sum;						/* truncate to 16 bits */
	return answer;
}


static u_int16_t in_cksum(u_int16_t *addr, long len)
{
	return ~in_cksum2(0, addr, len);
}


/*
 * ICMP Extension Structure Header (RFC4884).
 */
#ifndef ICMP_EXT_VERSION
#define ICMP_EXT_VERSION	2
#define ICMP_EXT_OFFSET		128

struct icmp_ext_hdr {
#if __BYTE_ORDER == __ORDER_BIG_ENDIAN__
	unsigned int version:4;
	unsigned int rsvd1:4;
	unsigned int rsvd2:8;
#else
	unsigned int rsvd2:8;
	unsigned int rsvd1:4;
	unsigned int version:4;
#endif
	uint16_t checksum;
};

/*
 * ICMP Extension Object Header (RFC4884).
 */
struct icmp_ext_obj_hdr {
	uint16_t length;
	uint8_t class_num;
	uint8_t c_type;
};
#endif

#ifdef ICMP_EXT_VERSION

/*
 * Support for ICMP extensions
 *
 * http://www.ietf.org/proceedings/01aug/I-D/draft-ietf-mpls-icmp-02.txt
 */
#define MPLS_STACK_ENTRY_CLASS 1
#define MPLS_STACK_ENTRY_C_TYPE 1

/*
struct mpls_header
{
#if BYTE_ORDER == BIG_ENDIAN
	uint32_t label:20;
	unsigned char exp:3;
	unsigned char s:1;
	unsigned char ttl:8;
#else
	unsigned char ttl:8;
	unsigned char s:1;
	unsigned char exp:3;
	uint32_t label:20;
#endif
};
*/

static void decode_extensions(unsigned char *buf, int ip_len)
{
	struct icmp_ext_hdr *cmn_hdr;
	struct icmp_ext_obj_hdr *obj_hdr;
	uint32_t mpls;
	size_t datalen;
	size_t obj_len;
	struct ip *ip;

	ip = (struct ip *) buf;

	if (ip_len < (int) ((ip->ip_hl << 2) + 8 + ICMP_EXT_OFFSET + sizeof(struct icmp_ext_hdr)))
	{
		/*
		 * No support for ICMP extensions on this host
		 */
		return;
	}

	/*
	 * Move forward to the start of the ICMP extensions, if present
	 */
	buf += (ip->ip_hl << 2) + 8 + ICMP_EXT_OFFSET;
	cmn_hdr = (struct icmp_ext_hdr *) buf;

	if (cmn_hdr->version != ICMP_EXT_VERSION)
	{
		/*
		 * Unknown version
		 */
		return;
	}

	datalen = ip_len - ((u_char *) cmn_hdr - (u_char *) ip);

	/*
	 * Check the checksum, cmn_hdr->checksum == 0 means no checksum'ing
	 * done by sender.
	 *
	 * If the checksum is ok, we'll get 0, as the checksum is calculated
	 * with the checksum field being 0'd.
	 */
	if (ntohs(cmn_hdr->checksum) && in_cksum((u_int16_t *) cmn_hdr, datalen) != 0)
	{
		return;
	}

	buf += sizeof(*cmn_hdr);
	datalen -= sizeof(*cmn_hdr);

	while (datalen >= sizeof(struct icmp_ext_obj_hdr))
	{
		obj_hdr = (struct icmp_ext_obj_hdr *) buf;
		obj_len = ntohs(obj_hdr->length);

		/*
		 * Sanity check the length field
		 */
		if (obj_len > datalen)
			return;

		datalen -= obj_len;

		/*
		 * Move past the object header
		 */
		buf += sizeof(struct icmp_ext_obj_hdr);
		obj_len -= sizeof(struct icmp_ext_obj_hdr);

		switch (obj_hdr->class_num)
		{
		case MPLS_STACK_ENTRY_CLASS:
			switch (obj_hdr->c_type)
			{
			case MPLS_STACK_ENTRY_C_TYPE:
				while (obj_len >= sizeof(uint32_t))
				{
					mpls = ntohl(*(uint32_t *) buf);

					buf += sizeof(uint32_t);
					obj_len -= sizeof(uint32_t);

					printf(" [MPLS: Label %ld Exp %d Ttl %u]", (unsigned long)(mpls >> 12), ((int)mpls >> 9) & 7, (int)mpls & 0xff);
				}
				if (obj_len > 0)
				{
					/*
					 * Something went wrong, and we're at
					 * a unknown offset into the packet,
					 * ditch the rest of it.
					 */
					return;
				}
				break;
			default:
				/*
				 * Unknown object, skip past it
				 */
				buf += ntohs(obj_hdr->length) - sizeof(struct icmp_ext_obj_hdr);
				break;
			}
			break;

		default:
			/*
			 * Unknown object, skip past it
			 */
			buf += ntohs(obj_hdr->length) - sizeof(struct icmp_ext_obj_hdr);
			break;
		}
	}
}
#endif


static void freehostinfo(struct hostinfo *hi)
{
	if (hi->name != NULL)
	{
		free(hi->name);
		hi->name = NULL;
	}
	free(hi->addrs);
	free(hi);
}


static struct hostinfo *gethostinfo(char *hostname)
{
	int n;
	struct hostent *hp;
	struct hostinfo *hi;
	char **p;
	u_int32_t *ap;
	struct in_addr addr;

	if (strlen(hostname) > MAXHOSTNAMELEN)
	{
		fprintf(stderr, "%s: hostname \"%.32s...\" is too long\n", prog, hostname);
		return NULL;
	}
	hi = calloc(1, sizeof(*hi));
	if (hi == NULL)
	{
		fprintf(stderr, "%s: calloc: %s\n", prog, strerror(errno));
		return NULL;
	}
	if (inet_aton(hostname, &addr) != 0)
	{
		hi->name = strdup(hostname);
		if (!hi->name)
		{
			fprintf(stderr, "%s: strdup: %s\n", prog, strerror(errno));
			return NULL;
		}
		hi->n = 1;
		hi->addrs = calloc(1, sizeof(hi->addrs[0]));
		if (hi->addrs == NULL)
		{
			fprintf(stderr, "%s: calloc: %s\n", prog, strerror(errno));
			return NULL;
		}
		hi->addrs[0] = addr.s_addr;
		return hi;
	}

	hp = gethostbyname(hostname);
	if (hp == NULL)
	{
		fprintf(stderr, "%s: unknown host %s\n", prog, hostname);
		return NULL;
	}
	if (hp->h_addrtype != AF_INET || hp->h_length != 4)
	{
		fprintf(stderr, "%s: bad host %s\n", prog, hostname);
		return NULL;
	}
	hi->name = strdup(hp->h_name);
	if (!hi->name)
	{
		fprintf(stderr, "%s: strdup: %s\n", prog, strerror(errno));
		return NULL;
	}
	for (n = 0, p = hp->h_addr_list; *p != NULL; ++n, ++p)
		continue;
	hi->n = n;
	hi->addrs = calloc(n, sizeof(hi->addrs[0]));
	if (hi->addrs == NULL)
	{
		fprintf(stderr, "%s: calloc: %s\n", prog, strerror(errno));
		return NULL;
	}
	for (ap = hi->addrs, p = hp->h_addr_list; *p != NULL; ++ap, ++p)
		memcpy(ap, *p, sizeof(*ap));
	return hi;
}


static int getaddr(u_int32_t *ap, char *hostname)
{
	struct hostinfo *hi;

	hi = gethostinfo(hostname);
	if (hi == NULL)
		return 0;
	*ap = hi->addrs[0];
	freehostinfo(hi);
	return 1;
}


static long deltaT(struct timeval *t1p, struct timeval *t2p)
{
	long dt;

	dt = (t2p->tv_sec - t1p->tv_sec) * 1000000L + (t2p->tv_usec - t1p->tv_usec);
	return dt;
}


static void setsin(struct sockaddr_in *sin, u_int32_t addr)
{
	memset(sin, 0, sizeof(*sin));
#ifdef HAVE_SOCKADDR_SA_LEN
	sin->sin_len = sizeof(*sin);
#endif
	sin->sin_family = AF_INET;
	sin->sin_addr.s_addr = addr;
}


/*
 * Convert an ICMP "type" field to a printable string.
 */
static const char *pr_type(u_char t)
{
	static const char *const ttab[] = {
		"Echo Reply", "ICMP 1", "ICMP 2", "Dest Unreachable",
		"Source Quench", "Redirect", "ICMP 6", "ICMP 7",
		"Echo", "ICMP 9", "ICMP 10", "Time Exceeded",
		"Param Problem", "Timestamp", "Timestamp Reply", "Info Request",
		"Info Reply"
	};

	if (t > 16)
		return "OUT-OF-RANGE";

	return ttab[t];
}


static void resize_packet(void)
{
	if (useicmp)
	{
		outicmp->icmp_cksum = 0;
		outicmp->icmp_cksum = in_cksum((u_int16_t *) outicmp, packlen - (sizeof(*outip) + optlen));
		if (outicmp->icmp_cksum == 0)
			outicmp->icmp_cksum = 0xffff;
	} else
	{
		outudp->uh_ulen = htons((u_int16_t) (packlen - (sizeof(*outip) + optlen)));
	}
}


/*
 * Received ICMP unreachable (fragmentation required and DF set).
 * If the ICMP error was from a "new" router, it'll contain the next-hop
 * MTU that we should use next.  Otherwise we'll just keep going in the
 * mtus[] table, trying until we hit a valid MTU.
 */
static void frag_err(void)
{
	int i;

	if (nextmtu > 0 && nextmtu < packlen)
	{
		printf("\nfragmentation required and DF set, next hop MTU = %d\n", nextmtu);
		packlen = nextmtu;
		for (i = 0; mtus[i] > 0; i++)
		{
			if (mtus[i] < nextmtu)
			{
				mtuptr = &mtus[i];		/* next one to try */
				break;
			}
		}
	} else
	{
		printf("\nfragmentation required and DF set. ");
		if (nextmtu)
			printf("\nBogus next hop MTU = %d > last MTU = %ld. ", nextmtu, packlen);
		packlen = *mtuptr++;
		printf("Trying new MTU = %ld\n", packlen);
	}
	resize_packet();
}


static int packet_ok(u_char *buf, ssize_t cc, struct sockaddr_in *from, int seq)
{
	struct icmp *icp;
	u_char type;
	u_char code;
	int hlen;

#ifndef ARCHAIC
	struct ip *ip;

	ip = (struct ip *) buf;
	hlen = ip->ip_hl << 2;
	if (cc < hlen + ICMP_MINLEN)
	{
		if (verbose)
			printf("packet too short (%ld bytes) from %s\n", (long) cc, inet_ntoa(from->sin_addr));
		return 0;
	}
	cc -= hlen;
	icp = (struct icmp *) (buf + hlen);
#else
	icp = (struct icmp *) buf;
#endif

	if (in_cksum((u_int16_t *) icp, htons(ip->ip_len) - hlen))
		fprintf(stderr, "Icmp checksum is wrong\n");

	type = icp->icmp_type;
	code = icp->icmp_code;

	/* Path MTU Discovery (RFC1191) */
	if (code != ICMP_UNREACH_NEEDFRAG)
	{
		pmtu = 0;
	} else
	{
#ifdef HAVE_ICMP_NEXTMTU
		pmtu = ntohs(icp->icmp_nextmtu);
#else
		struct my_pmtu *ppmtu = (struct my_pmtu *) &icp->icmp_void;

		pmtu = ntohs(ppmtu->ipm_nextmtu);
#endif
	}

	if ((type == ICMP_TIMXCEED && code == ICMP_TIMXCEED_INTRANS) || type == ICMP_UNREACH || type == ICMP_ECHOREPLY)
	{
		struct ip *hip;
		struct udphdr *up;
		struct icmp *hicmp;

		hip = &icp->icmp_ip;
		hlen = hip->ip_hl << 2;

		nextmtu = ntohs(icp->icmp_nextmtu);	/* for frag_err() */

		if (useicmp)
		{
			/* XXX */
			if (type == ICMP_ECHOREPLY && icp->icmp_id == htons(ident) && icp->icmp_seq == htons(seq))
				return -2;

			hicmp = (struct icmp *) ((u_char *) hip + hlen);
			/* XXX 8 is a magic number */
			if (hlen + 8 <= cc &&
				hip->ip_p == IPPROTO_ICMP && hicmp->icmp_id == htons(ident) && hicmp->icmp_seq == htons(seq))
				return type == ICMP_TIMXCEED ? -1 : code + 1;
		} else
		{
			up = (struct udphdr *) ((u_char *) hip + hlen);
			/* XXX 8 is a magic number */
			if (hlen + 12 <= cc &&
				hip->ip_p == IPPROTO_UDP &&
#ifdef LAUGHTER
				up->uh_sport == htons(ident) &&
#else
				up->uh_sport == uh_sport &&
#endif
				up->uh_dport == htons(port + seq))
				return type == ICMP_TIMXCEED ? -1 : code + 1;
		}
	}
#ifndef ARCHAIC
	if (verbose)
	{
		long i;
		uint32_t *lp = (u_int32_t *) & icp->icmp_ip;

		printf("\n%ld bytes from %s to ", (long) cc, inet_ntoa(from->sin_addr));
		printf("%s: icmp type %d (%s) code %d\n", inet_ntoa(ip->ip_dst), type, pr_type(type), icp->icmp_code);
		for (i = 4; i < cc; i += sizeof(*lp))
			printf("%2ld: x%8.8lx\n", i, (unsigned long)*lp++);
	}
#endif
	return 0;
}


/*
 * Construct an Internet address representation.
 * If the nflag has been supplied, give
 * numeric value, otherwise try for symbolic name.
 */
static char *inetname(struct in_addr in)
{
	char *cp;
	struct hostent *hp;
	static int first = 1;
	static char domain[MAXHOSTNAMELEN + 1];
	static char line[MAXHOSTNAMELEN + 1];

	if (first && !nflag)
	{
		first = 0;
		if (gethostname(domain, MAXHOSTNAMELEN) < 0)
			domain[0] = '\0';
		else
		{
			cp = strchr(domain, '.');
			if (cp == NULL)
			{
				hp = gethostbyname(domain);
				if (hp != NULL)
					cp = strchr(hp->h_name, '.');
			}
			if (cp == NULL)
				domain[0] = '\0';
			else
			{
				++cp;
				strncpy(domain, cp, sizeof(domain) - 1);
				domain[sizeof(domain) - 1] = '\0';
			}
		}
	}
	if (!nflag && in.s_addr != INADDR_ANY)
	{
		hp = gethostbyaddr((char *) &in, sizeof(in), AF_INET);
		if (hp != NULL)
		{
			if ((cp = strchr(hp->h_name, '.')) != NULL && strcmp(cp + 1, domain) == 0)
				*cp = '\0';
			strncpy(line, hp->h_name, sizeof(line) - 1);
			line[sizeof(line) - 1] = '\0';
			return line;
		}
	}
	return inet_ntoa(in);
}


static void print(u_char *buf, ssize_t cc, struct sockaddr_in *from)
{
	struct ip *ip;
	int hlen;
	char addr[INET_ADDRSTRLEN];

	ip = (struct ip *) buf;
	hlen = ip->ip_hl << 2;
	cc -= hlen;

	strcpy(addr, inet_ntoa(from->sin_addr));

	if (as_path)
		printf(" [AS%u]", as_lookup(asn, addr, AF_INET));

	if (nflag)
		printf(" %s", addr);
	else
		printf(" %s (%s)", inetname(from->sin_addr), addr);

	if (verbose)
		printf(" %ld bytes to %s", (long) cc, inet_ntoa(ip->ip_dst));
}


static void dump_packet(void)
{
	const uint8_t *p;
	int i;
	int len;

	fprintf(stderr, "packet data:");

#if USE_UDP
	p = useicmp ? (const uint8_t *) outicmp : (const uint8_t *) outudp;
	len = (int)(packlen - (sizeof(*outip) + optlen));
#else
	p = (const uint8_t *) outip;
	len = (int)packlen;
#endif
	for (i = 0; i < len; i++)
	{
		if ((i % 24) == 0)
			fprintf(stderr, "\n ");
		fprintf(stderr, " %02x", *p++);
	}
	fprintf(stderr, "\n");
}


static int setsockoptl(int fd, int level, int optname, int32_t val)
{
	if (setsockopt(fd, level, optname, &val, sizeof(val)) < 0)
	{
		fprintf(stderr, "%s: setsockopt %ld: %s\n", prog, (long)val, mint_strerror(errno));
		return 0;
	}
	return 1;
}


static int send_probe(int seq, int ttl, struct timeval *tp)
{
	ssize_t cc;
	long oldmtu = packlen;

  again:
#ifdef BYTESWAP_IP_HDR
	outip->ip_len = htons((uint16_t)packlen);
#else
	outip->ip_len = (uint16_t)packlen;
#endif
	outip->ip_ttl = ttl;
#ifdef linux
	/* Do not fiddle with ID, it must be unique
	   and only kernel is allowed to make it. --ANK
	 */
	outip->ip_id = 0;
#else
#ifndef __hpux
	outip->ip_id = htons(ident + seq);
#endif
#endif

#ifdef LAUGHTER
	/* The comment below has nothing to do with reality and
	   udp cksum has nothing to do with ip one. --ANK
	 */
	/*
	 * In most cases, the kernel will recalculate the ip checksum.
	 * But we must do it anyway so that the udp checksum comes out
	 * right.
	 */
	if (doipcksum)
	{
		outip->ip_sum = in_cksum((u_int16_t *) outip, sizeof(*outip) + optlen);
		if (outip->ip_sum == 0)
			outip->ip_sum = 0xffff;
	}
#endif

	/* Payload */
	outsetup.seq = seq;
	outsetup.ttl = ttl;
	outsetup.tv.tv32_sec = htonl(tp->tv_sec);
	outsetup.tv.tv32_usec = htonl(tp->tv_usec);
	memcpy(outmark, &outsetup, sizeof(outsetup));

	if (useicmp)
		outicmp->icmp_seq = htons(seq);
	else
		outudp->uh_dport = htons(port + seq);

	/* (We can only do the checksum if we know our ip address) */
	if (useicmp)
	{
		/* Always calculate checksum for icmp packets */
		outicmp->icmp_cksum = 0;
		outicmp->icmp_cksum = in_cksum((u_int16_t *) outicmp, packlen - (sizeof(*outip) + optlen));
		if (outicmp->icmp_cksum == 0)
			outicmp->icmp_cksum = 0xffff;
	} else if (doipcksum)
	{
#ifdef FULL_CRAP_I_WONDER_WHY_LBNL_FOLKS_DID_IT
		struct ip tip;
		struct udpiphdr *ui;

		/* Checksum (we must save and restore ip header) */
		tip = *outip;
		ui = (struct udpiphdr *) outip;
		ui->ui_next = 0;
		ui->ui_prev = 0;
		ui->ui_x1 = 0;
		ui->ui_len = outudp->uh_ulen;
		outudp->uh_sum = 0;
		outudp->uh_sum = in_cksum((u_int16_t *) ui, packlen);
		if (outudp->uh_sum == 0)
			outudp->uh_sum = 0xffff;
		*outip = tip;
#else
		struct udpmagichdr
		{
			struct in_addr src;
			struct in_addr dst;
			u_char zero;
			u_char proto;
			u_int16_t len;
		} h, saved, *hptr;

		hptr = ((struct udpmagichdr *) outudp) - 1;
		h.src = outip->ip_src;
		h.dst = outip->ip_dst;
		h.zero = 0;
		h.proto = IPPROTO_UDP;
		h.len = outudp->uh_ulen;
		saved = *hptr;
		*hptr = h;
		outudp->uh_sum = 0;
		outudp->uh_sum = in_cksum((u_int16_t *) hptr, ntohs(outudp->uh_ulen) + sizeof(*hptr));
		if (outudp->uh_sum == 0)
			outudp->uh_sum = 0xffff;
		*hptr = saved;
#endif
	}

	/* XXX undocumented debugging hack */
	if (verbose > 1)
	{
		const u_int16_t *sp;
		int nshorts;
		int i;

		sp = (u_int16_t *) outip;
		nshorts = (int)(packlen / sizeof(u_int16_t));
		i = 0;
		printf("[ %ld bytes", packlen);
		while (--nshorts >= 0)
		{
			if ((i++ % 8) == 0)
				printf("\n\t");
			printf(" %04x", ntohs(*sp++));
		}
		if (packlen & 1)
		{
			if ((i % 8) == 0)
				printf("\n\t");
			printf(" %02x", *(const u_char *) sp);
		}
		printf("]\n");
	}

#if !defined(IP_HDRINCL) && defined(IP_TTL)
	if (setsockoptl(sndsock, IPPROTO_IP, IP_TTL, ttl) == 0)
		return 0;
#endif
	if (dump)
		dump_packet();

#if USE_UDP
	cc = sendto(sndsock, useicmp ? (char *) outicmp : (char *) outudp,
				packlen - (sizeof(*outip) + optlen), 0, &whereto, sizeof(whereto));
	if (cc > 0)
		cc += sizeof(*outip) + optlen;
#else
	cc = sendto(sndsock, outip, packlen, 0, &whereto, sizeof(whereto));
#endif
	if (cc < 0 || cc != packlen)
	{
		if (cc < 0)
		{
			/*
			 * An errno of EMSGSIZE means we're writing too big a
			 * datagram for the interface.  We have to just
			 * decrease the packet size until we find one that
			 * works.
			 *
			 * XXX maybe we should try to read the outgoing if's
			 * mtu?
			 */
			if (errno == EMSGSIZE)
			{
				packlen = *mtuptr++;
				resize_packet();
				goto again;
			}
			fprintf(stderr, "%s: sendto: %s\n", prog, mint_strerror(errno));
		}

		printf("%s: wrote %s %ld chars, ret=%ld\n", prog, hostname, packlen, (long) cc);
		fflush(stdout);
	}
	if (oldmtu != packlen)
	{
		printf("message too big, trying new MTU = %ld\n", packlen);
		printed_ttl = 0;
	}
	if (!printed_ttl)
	{
		printf("%2d ", ttl);
		printed_ttl = 1;
	}
	
	return 1;
}


/*
 * Subtract 2 timeval structs:  out = out - in.
 * Out is assumed to be >= in.
 */
static void tvsub(struct timeval *out, struct timeval *in)
{
	if ((out->tv_usec -= in->tv_usec) < 0)
	{
		--out->tv_sec;
		out->tv_usec += 1000000L;
	}
	out->tv_sec -= in->tv_sec;
}


static ssize_t wait_for_reply(int sock, struct sockaddr_in *fromp, const struct timeval *tp)
{
	struct pollfd set[1];
	struct timeval now;
	struct timeval wait;
	ssize_t cc = 0;
	socklen_t fromlen = sizeof(*fromp);
	int retval;

	set[0].fd = sock;
	set[0].events = POLLIN;

	wait.tv_sec = tp->tv_sec + waittime;
	wait.tv_usec = tp->tv_usec;
	gettimeofday(&now, NULL);
	tvsub(&wait, &now);

	if (wait.tv_sec < 0)
	{
		wait.tv_sec = 0;
		wait.tv_usec = 0;
	}

	retval = poll(set, 1, wait.tv_sec * 1000 + wait.tv_usec / 1000);
	if (retval < 0)
	{
		/* If we continue, we probably just flood the remote host. */
		fprintf(stderr, "%s: poll: %s\n", prog, mint_strerror(errno));
		exit(EXIT_FAILURE);
	}
	if (retval > 0)
		cc = recvfrom(sock, (char *) packet, sizeof(packet), 0, (struct sockaddr *) fromp, &fromlen);

	return cc;
}


static int find_local_ip(struct sockaddr_in *from, struct sockaddr_in *to)
{
	int sock;
	struct sockaddr_in help;
	socklen_t help_len;

	sock = socket(AF_INET, SOCK_DGRAM, 0);
	if (sock < 0)
		return 0;

	help.sin_family = AF_INET;
	/*
	 * At this point the port number doesn't matter
	 * since it only has to be greater than zero.
	 */
	help.sin_port = 42;
	help.sin_addr.s_addr = to->sin_addr.s_addr;
	if (connect(sock, (struct sockaddr *) &help, sizeof(help)) < 0)
	{
		close(sock);
		return 0;
	}

	help_len = sizeof(help);
	if (getsockname(sock, (struct sockaddr *) &help, &help_len) < 0 ||
		help_len != sizeof(help) || help.sin_addr.s_addr == INADDR_ANY)
	{
		close(sock);
		return 0;
	}

	close(sock);
	setsin(from, help.sin_addr.s_addr);
	return 1;
}


#ifdef USE_KERNEL_ROUTING_TABLE

/* This function currently only supports IPv4.  Someone who knows
 * more about multi-protocol socket stuff should take a look at this.
 * 
 * (But does it make any sense for traceroute to support other 
 * protocols?  Maybe IPv6...
 */
static struct ifaddrlist *search_routing_table(struct sockaddr_in *to, struct ifaddrlist *al, int n)
{
	struct ifaddrlist *first_if;
	FILE *fp;
	char buf[1024];
	char ifname[128];
	unsigned int route_dest;
	unsigned int mask;
	unsigned int best_mask;
	unsigned int dest_addr;
	unsigned int best_addr;
	unsigned int gateway;
	unsigned int use_gateway;
	unsigned int convs;

	/* How come using ntohl(to->sin_addr.s_addr) doesn't work here? */
	dest_addr = to->sin_addr.s_addr;

	fp = fopen("/proc/net/route", "r");
	if (fp == NULL)
	{
		return al;
	}

	/* Skip the first line (the column headings) */
	if (fgets(buf, sizeof(buf), fp) == NULL)
	{
		fclose(fp);
		return al;
	}

	best_mask = 0;
	best_addr = 0;
	use_gateway = 0;

	while (fgets(buf, sizeof(buf), fp) != NULL)
	{
		/* Field 1: interface name
		 * Field 2: dest addr
		 * Field 3: gateway addr
		 * Field 8: genmask 
		 */
		convs = sscanf(buf, "%s %x %x %*s %*s %*s %*s %x", ifname, &route_dest, &gateway, &mask);
		if (convs != 4)
		{
			/* format error .... */
			fclose(fp);
			return al;
		}

		if ((dest_addr & mask) == route_dest)
		{
			/* This routing entry applies to
			 * our destination addr
			 */
			if ((mask > best_mask) || (best_mask == 0))
			{
				/* And it is more specific than any
				 * previous match (or is the first match)
				 */
				best_mask = mask;
				best_addr = route_dest;
				use_gateway = gateway;
			}
		}
	}

	fclose(fp);

	/* If we don't find a match, we'll return the first entry */
	first_if = al;

	while (al < first_if + n)
	{
		/* Use different approach if we send through gateway */
		if (use_gateway)
		{
			if ((al->addr & al->mask) == (use_gateway & al->mask))
			{
				return al;
			}
		} else
		{
			/* Better way than comparing if names,
			   this works with aliased if:s too */
			if (best_addr == (al->addr & al->mask))
			{
				/* Got a match */
				return al;
			}
		}
		al++;
	}

	return first_if;
}
#endif


/* String to value with optional min and max. Handles decimal and hex. */
static long str2val(const char *str, const char *what, long mi, long ma)
{
	const char *cp;
	long val;
	char *ep;

	errno = 0;
	ep = NULL;
	if (str[0] == '0' && (str[1] == 'x' || str[1] == 'X'))
	{
		cp = str + 2;
		val = strtol(cp, &ep, 16);
	} else
	{
		val = strtol(str, &ep, 10);
	}
	if (errno || str[0] == '\0' || *ep != '\0')
	{
		fprintf(stderr, "%s: \"%s\" bad value for %s \n", prog, str, what);
		return mi - 1;
	}
	if (val < mi && mi >= 0)
	{
		fprintf(stderr, "%s: %s must be >= %ld\n", prog, what, mi);
		return mi - 1;
	}
	if (val > ma && ma >= 0)
	{
		fprintf(stderr, "%s: %s must be <= %ld\n", prog, what, ma);
		return mi - 1;
	}
	return val;
}


static void usage(void)
{
	fprintf(stderr, "Version %s\n", version);
	fprintf(stderr, "Usage: %s [-adDFPIlMnrvx] [-g gateway] [-i iface] \
[-f first_ttl]\n\t[-m max_ttl] [-p port] [-q nqueries] [-s src_addr] [-t tos]\n\t\
[-w waittime] [-z pausemsecs] [-A as_server] host [packetlen]\n", prog);
}


int main(int argc, char **argv)
{
	int op;
	int code;
	int n;
	char *cp;
	u_char *outp;
	u_int32_t *ap;
	struct sockaddr_in *from = (struct sockaddr_in *) &wherefrom;
	struct sockaddr_in *to = (struct sockaddr_in *) &whereto;
	struct hostinfo *hi;
	int ttl;
	int probe;
	int i;
	int seq = 0;
	int tos = 0;
	int settos = 0;
	int ttl_flag = 0;
	int lsrr = 0;
	u_int16_t off = 0;
	struct ifaddrlist *al;
	struct ifaddrlist *allist;
	char errbuf[132];

	if (argv[0] == NULL || argv[0][0] == '\0')
		prog = "traceroute";
	else if ((cp = strrchr(argv[0], '/')) != NULL)
		prog = cp + 1;
	else
		prog = argv[0];

	if ((recvsock = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP)) < 0)
	{
		fprintf(stderr, "%s: icmp socket: %s\n", prog, mint_strerror(errno));
		return EXIT_FAILURE;
	}

#ifdef IPCTL_DEFTTL
	{
		int mib[4] = { CTL_NET, PF_INET, IPPROTO_IP, IPCTL_DEFTTL };
		size_t size = sizeof(max_ttl);

		sysctl(mib, sizeof(mib) / sizeof(mib[0]), &max_ttl, &size, NULL, 0);
	}
#else
	max_ttl = 30;
#endif

	opterr = 0;
	while ((op = getopt(argc, argv, "aA:dDFPIMnlrvxf:g:i:m:p:q:s:t:w:z:")) != -1)
	{
		switch (op)
		{
		case 'a':
			as_path = 1;
			break;

		case 'A':
			as_path = 1;
			as_server = optarg;
			break;

		case 'd':
			options |= SO_DEBUG;
			break;

		case 'D':
			dump = 1;
			break;

		case 'f':
			first_ttl = (int)str2val(optarg, "first ttl", 1, 255);
			if (first_ttl <= 0)
				return EXIT_FAILURE;
			break;

		case 'F':
			off = IP_DF;
			break;

		case 'g':
			if (strlen(optarg) >= MAXHOSTNAMELEN)
			{
				fprintf(stderr, "%s: Gateway address too long\n", prog);
				return EXIT_FAILURE;
			}
			if (lsrr >= NGATEWAYS)
			{
				fprintf(stderr, "%s: No more than %d gateways\n", prog, NGATEWAYS);
				return EXIT_FAILURE;
			}
			if (!getaddr(gwlist + lsrr, optarg))
				return EXIT_FAILURE;
			++lsrr;
			break;

		case 'i':
			device = optarg;
			if (strlen(device) >= IF_NAMESIZE)
			{
				fprintf(stderr, "%s: Interface name too long\n", prog);
				return EXIT_FAILURE;
			}
			break;

		case 'I':
			++useicmp;
			break;

		case 'l':
			++ttl_flag;
			break;

		case 'm':
			max_ttl = (int)str2val(optarg, "max ttl", 1, 255);
			if (max_ttl <= 0)
				return EXIT_FAILURE;
			break;

		case 'M':
			Mflag = 1;
			break;

		case 'n':
			++nflag;
			break;

		case 'p':
			port = str2val(optarg, "port", 1, 65535L);
			if (port <= 0)
				return EXIT_FAILURE;
			break;

		case 'q':
			nprobes = (int)str2val(optarg, "nprobes", 1, -1);
			if (nprobes <= 0)
				return EXIT_FAILURE;
			break;

		case 'r':
			options |= SO_DONTROUTE;
			break;

		case 's':
			/*
			 * set the ip source address of the outbound
			 * probe (e.g., on a multi-homed host).
			 */
			source = optarg;
			if (strlen(source) >= MAXHOSTNAMELEN)
			{
				fprintf(stderr, "%s: Source address too long\n", prog);
				return EXIT_FAILURE;
			}
			break;

		case 't':
			tos = (int)str2val(optarg, "tos", 0, 255);
			if (tos < 0)
				return EXIT_FAILURE;
			++settos;
			break;

		case 'v':
			++verbose;
			break;

		case 'x':
			doipcksum = doipcksum == 0;
			break;

		case 'w':
			waittime = str2val(optarg, "wait time", 1, 24L * 60 * 60);
			if (waittime <= 0)
				return EXIT_FAILURE;
			break;

		case 'z':
			pausemsecs = str2val(optarg, "pause msecs", 0, 60L * 60 * 1000);
			if (pausemsecs < 0)
				return EXIT_FAILURE;
			break;

		case 'P':
			off = IP_DF;
			mtudisc = 1;
			break;

		default:
			usage();
			return EXIT_FAILURE;
		}
	}

	if (first_ttl > max_ttl)
	{
		fprintf(stderr, "%s: first ttl (%d) may not be greater than max ttl (%d)\n", prog, first_ttl, max_ttl);
		return EXIT_FAILURE;
	}

	if (lsrr > 0)
		optlen = (lsrr + 1) * (int)sizeof(gwlist[0]);
	minpacket = sizeof(*outip) + sizeof(struct outdata) + optlen;
	if (useicmp)
		minpacket += 8;					/* XXX magic number */
	else
		minpacket += sizeof(*outudp);
	packlen = minpacket;				/* minimum sized packet */

	if (mtudisc)
		packlen = *mtuptr++;

	/* Process destination and optional packet size */
	switch (argc - optind)
	{
	case 2:
		packlen = str2val(argv[optind + 1], "packet length", minpacket, maxpacket);
		if (packlen < minpacket)
			return EXIT_FAILURE;
		/* Fall through */

	case 1:
		hostname = argv[optind];
		if (strlen(hostname) >= MAXHOSTNAMELEN)
		{
			fprintf(stderr, "%s: Address too long\n", prog);
			return EXIT_FAILURE;
		}
		hi = gethostinfo(hostname);
		if (hi == NULL)
			return EXIT_FAILURE;
		setsin(to, hi->addrs[0]);
		if (hi->n > 1)
			fprintf(stderr, "%s: Warning: %s has multiple addresses; using %s\n", prog, hostname, inet_ntoa(to->sin_addr));
		hostname = hi->name;
		hi->name = NULL;
		freehostinfo(hi);
		break;

	default:
		usage();
		return EXIT_FAILURE;
	}

	/* This checking was moved here by oh3mqu+rpm@vip.fi */
	/* It was useless before packlen gets command line value */
	if (packlen == 0)
	{
		packlen = minpacket;			/* minimum sized packet */
	} else if (minpacket > packlen || packlen > maxpacket)
	{
		fprintf(stderr, "%s: packet size must be %ld <= s <= %ld\n", prog, minpacket, maxpacket);
		return EXIT_FAILURE;
	}

	setvbuf(stdout, NULL, _IONBF, 0);

	outip = (struct ip *) malloc(packlen);
	if (outip == NULL)
	{
		fprintf(stderr, "%s: malloc: %s\n", prog, strerror(errno));
		return EXIT_FAILURE;
	}
	memset(outip, 0, packlen);

	outip->ip_v = IPVERSION;
	if (settos)
		outip->ip_tos = tos;
#ifdef BYTESWAP_IP_HDR
	outip->ip_len = htons((uint16_t) packlen);
	outip->ip_off = htons(off);
#else
	outip->ip_len = (uint16_t) packlen;
	outip->ip_off = off;
#endif
	outp = (u_char *) (outip + 1);
#ifdef HAVE_RAW_OPTIONS
	if (lsrr > 0)
	{
		u_char *optlist;

		optlist = outp;
		outp += optlen;

		/* final hop */
		gwlist[lsrr] = to->sin_addr.s_addr;

		outip->ip_dst.s_addr = gwlist[0];

		/* force 4 byte alignment */
		optlist[0] = IPOPT_NOP;
		/* loose source route option */
		optlist[1] = IPOPT_LSRR;
		i = lsrr * sizeof(gwlist[0]);
		optlist[2] = i + 3;
		/* Pointer to LSRR addresses */
		optlist[3] = IPOPT_MINOFF;
		memcpy(optlist + 4, gwlist + 1, i);
	} else
#endif
		outip->ip_dst = to->sin_addr;

	outip->ip_hl = (unsigned int)(outp - (u_char *) outip) >> 2;
	ident = (random() & 0xffff) | 0x8000;
	if (useicmp)
	{
		outip->ip_p = IPPROTO_ICMP;

		outicmp = (struct icmp *) outp;
		outicmp->icmp_type = ICMP_ECHO;
		outicmp->icmp_id = htons(ident);

		outmark = outp + 8;				/* XXX magic number */
	} else
	{
		outip->ip_p = IPPROTO_UDP;

		outudp = (struct udphdr *) outp;
#ifdef LAUGHTER
		outudp->uh_sport = htons(ident);
#else
		/* Avoid udp port conflicts! */
		if (!useicmp)
		{
			struct sockaddr_in s;
			socklen_t alen = sizeof(s);
			int lock_fd = socket(AF_INET, SOCK_DGRAM, 0);

			if (lock_fd < 0)
			{
				perror("socket");
				return EXIT_FAILURE;
			}
			memset(&s, 0, sizeof(s));
#if !defined(__MINT__) && !defined(__TOS__)
			if (bind(lock_fd, (struct sockaddr *) &s, sizeof(s)) == -1)
			{
				perror("bind");
				return EXIT_FAILURE;
			}
#endif
			if (getsockname(lock_fd, (struct sockaddr *) &s, &alen) == -1)
			{
				perror("getsockname");
				return EXIT_FAILURE;
			}
			uh_sport = s.sin_port;
			outudp->uh_sport = s.sin_port;
			/* DO NOT CLOSE LOCK SOCKET */
		}
#endif
		outudp->uh_ulen = htons((uint16_t) (packlen - (sizeof(*outip) + optlen)));
		outmark = outudp + 1;
	}

#if USE_UDP
	sndsock = socket(AF_INET, SOCK_DGRAM, useicmp ? IPPROTO_ICMP : IPPROTO_UDP);
#else
	sndsock = socket(AF_INET, SOCK_RAW, IPPROTO_RAW);
#endif
	if (sndsock < 0)
	{
		fprintf(stderr, "%s: raw socket: %s\n", prog, mint_strerror(errno));
		return EXIT_FAILURE;
	}

#if defined(IP_OPTIONS) && !defined(HAVE_RAW_OPTIONS)
	if (lsrr > 0)
	{
		u_char optlist[MAX_IPOPTLEN];

		/* final hop */
		gwlist[lsrr] = to->sin_addr.s_addr;
		++lsrr;

		/* force 4 byte alignment */
		optlist[0] = IPOPT_NOP;
		/* loose source route option */
		optlist[1] = IPOPT_LSRR;
		i = lsrr * (int)sizeof(gwlist[0]);
		optlist[2] = i + 3;
		/* Pointer to LSRR addresses */
		optlist[3] = IPOPT_MINOFF;
		memcpy(optlist + 4, gwlist, i);

		if (setsockopt(sndsock, IPPROTO_IP, IP_OPTIONS, optlist, i + sizeof(gwlist[0])) < 0)
		{
			fprintf(stderr, "%s: IP_OPTIONS: %s\n", prog, mint_strerror(errno));
			return EXIT_FAILURE;
		}
	}
#endif

#ifdef SO_SNDBUF
	if (setsockoptl(sndsock, SOL_SOCKET, SO_SNDBUF, packlen) == 0)
		return EXIT_FAILURE;
#endif
#ifdef IP_HDRINCL
	if (setsockoptl(sndsock, IPPROTO_IP, IP_HDRINCL, 1) == 0)
		return EXIT_FAILURE;
#else
#ifdef IP_TOS
	if (settos && setsockoptl(sndsock, IPPROTO_IP, IP_TOS, tos) == 0)
		return EXIT_FAILURE;
#endif
#endif
	if (options & SO_DEBUG)
		setsockoptl(sndsock, SOL_SOCKET, SO_DEBUG, 1);
	if (options & SO_DONTROUTE)
		setsockoptl(sndsock, SOL_SOCKET, SO_DONTROUTE, 1);

	/* Get the interface address list */
	n = ifaddrlist(&allist, errbuf);
	al = allist;
	if (n < 0)
	{
		fprintf(stderr, "%s: ifaddrlist: %s\n", prog, errbuf);
		return EXIT_FAILURE;
	}
	if (n == 0)
	{
		fprintf(stderr, "%s: Can't find any network interfaces\n", prog);
		return EXIT_FAILURE;
	}

	/* Look for a specific device */
	if (device != NULL)
	{
		for (i = n; i > 0; --i, ++al)
			if (strcmp(device, al->device) == 0)
				break;
		if (i <= 0)
		{
			fprintf(stderr, "%s: Can't find interface %s\n", prog, device);
			return EXIT_FAILURE;
		}
	}

	/* Determine our source address */
	if (source == NULL)
	{
#ifdef USE_KERNEL_ROUTING_TABLE
		/* Search the kernel routing table for a match with the
		 * destination address.  Then use that interface.  If
		 * there is no match, default to using the first 
		 * interface found.
		 */
		al = search_routing_table(to, allist, n);
		setsin(from, al->addr);
#else
		/*
		 * If a device was specified, use the interface address.
		 * Otherwise, try to determine our source address.
		 * Warn if there are more than one.
		 */
		setsin(from, al->addr);
		if (n > 1 && device == NULL && !find_local_ip(from, to))
		{
			fprintf(stderr,
					"%s: Warning: Multiple interfaces found; using %s @ %s\n",
					prog, inet_ntoa(from->sin_addr), al->device);
		}
#endif
	} else
	{
		hi = gethostinfo(source);
		source = hi->name;
		hi->name = NULL;
		/*
		 * If the device was specified make sure it
		 * corresponds to the source address specified.
		 * Otherwise, use the first address (and warn if
		 * there are more than one).
		 */
		if (device == NULL)
		{
			/*
			 * Use the first interface found.
			 * Warn if there are more than one.
			 */
			setsin(from, hi->addrs[0]);
			if (hi->n > 1)
				fprintf(stderr,
						"%s: Warning: %s has multiple addresses; using %s\n", prog, source, inet_ntoa(from->sin_addr));
		} else
		{
			/*
			 * Make sure the source specified matches the
			 * interface address.
			 */
			for (i = hi->n, ap = hi->addrs; i > 0; --i, ++ap)
				if (*ap == al->addr)
					break;
			if (i <= 0)
			{
				fprintf(stderr, "%s: %s is not on interface %s\n", prog, source, device);
				return EXIT_FAILURE;
			}
			setsin(from, *ap);
		}
		freehostinfo(hi);
	}

	/* Revert to non-privileged user after opening sockets */
	setgid(getgid());
	setuid(getuid());

	/* 
	 * If not root, make sure source address matches a local interface.
	 * (The list of addresses produced by ifaddrlist() automatically
	 * excludes interfaces that are marked down and/or loopback.)
	 */
	if (getuid() != 0)
	{
		al = allist;
		for (i = n; i > 0; --i, ++al)
			if (from->sin_addr.s_addr == al->addr)
				break;
		if (i <= 0)
		{
			fprintf(stderr, "%s: %s is not a valid local address and you are not superuser.\n", prog, inet_ntoa(from->sin_addr));
			return EXIT_FAILURE;
		}
	}

	outip->ip_src = from->sin_addr;
#ifndef IP_HDRINCL
	if (bind(sndsock, (struct sockaddr *) from, sizeof(*from)) < 0)
	{
		fprintf(stderr, "%s: bind: %s\n", prog, mint_strerror(errno));
		return EXIT_FAILURE;
	}
#endif

#ifdef linux
	if (bind(sndsock, (struct sockaddr *) from, sizeof(*from)) < 0)
	{
		fprintf(stderr, "%s: bind: %s\n", prog, mint_strerror(errno));
		return EXIT_FAILURE;
	}
	if (bind(recvsock, (struct sockaddr *) from, sizeof(*from)) < 0)
	{
		fprintf(stderr, "%s: bind ICMP socket: %s\n", prog, mint_strerror(errno));
		return EXIT_FAILURE;
	}
	{
		struct icmp_filter filt;

		filt.data = ~((1 << ICMP_TIMXCEED) | (1 << ICMP_UNREACH));
		if (useicmp)
			filt.data &= ~(1 << ICMP_ECHOREPLY);
		if (setsockopt(recvsock, SOL_RAW, ICMP_FILTER, (char *) &filt, sizeof(filt)) == -1)
			perror("WARNING: setsockopt(ICMP_FILTER)");
	}
#endif

	if (as_path)
	{
		asn = as_setup(as_server);
		if (asn == NULL)
		{
			fprintf(stderr, "%s: Warning: as_setup failed, AS# lookups disabled\n", prog);
			fflush(stderr);
			as_path = 0;
		}
	}

	fprintf(stderr, "%s to %s (%s)", prog, hostname, inet_ntoa(to->sin_addr));
	if (source)
		fprintf(stderr, " from %s", source);
	fprintf(stderr, ", %d hops max, %ld byte packets\n", max_ttl, packlen);
	fflush(stderr);

	for (ttl = first_ttl; ttl <= max_ttl; ++ttl)
	{
		u_int32_t lastaddr = 0;
		int gotlastaddr = 0;
		int got_there = 0;
		int unreachable = 0;
		int sentfirst = 0;

	  again:
		printed_ttl = 0;
		for (probe = 0; probe < nprobes; ++probe)
		{
			ssize_t cc;
			struct timeval t1;
			struct timeval t2;
			struct ip *ip;

			if (sentfirst && pausemsecs > 0)
				usleep(pausemsecs * 1000L);
			gettimeofday(&t1, NULL);
			++seq;
			if (!useicmp && htons(port + seq) == 0)
				seq++;
			if (send_probe(seq, ttl, &t1) == 0)
				return EXIT_FAILURE;
			++sentfirst;
			while ((cc = wait_for_reply(recvsock, from, &t1)) != 0)
			{
				long dt;
				
				gettimeofday(&t2, NULL);
				/*
				 * Since we'll be receiving all ICMP
				 * messages to this host above, we may
				 * never end up with cc=0, so we need
				 * an additional termination check.
				 */
				if (t2.tv_sec - t1.tv_sec > waittime)
				{
					cc = 0;
					break;
				}
				i = packet_ok(packet, cc, from, seq);
				/* Skip short packet */
				if (i == 0)
					continue;
				if (!gotlastaddr || from->sin_addr.s_addr != lastaddr)
				{
					if (gotlastaddr)
						printf("\n   ");
					print(packet, cc, from);
					lastaddr = from->sin_addr.s_addr;
					++gotlastaddr;
				}
				ip = (struct ip *) packet;
				dt = deltaT(&t1, &t2);
				printf("  %ld.%03ld ms", dt / 1000, dt % 1000);
				if (ttl_flag)
					printf(" (ttl = %d)", ip->ip_ttl);
				if (i == -2)
				{
#ifndef ARCHAIC
					if (ip->ip_ttl <= 1)
						printf(" !");
#endif
					++got_there;
					break;
				}

				/* time exceeded in transit */
				if (i == -1)
					break;
				code = i - 1;
				switch (code)
				{
				case ICMP_UNREACH_PORT:
#ifndef ARCHAIC
					if (ip->ip_ttl <= 1)
						printf(" !");
#endif
					++got_there;
					break;

				case ICMP_UNREACH_NET:
					++unreachable;
					printf(" !N");
					break;

				case ICMP_UNREACH_NET_PROHIB:
					++unreachable;
					printf(" !n");
					break;

				case ICMP_UNREACH_HOST:
					++unreachable;
					printf(" !H");
					break;

				case ICMP_UNREACH_HOST_PROHIB:
					++unreachable;
					printf(" !h");
					break;

				case ICMP_UNREACH_PROTOCOL:
					++got_there;
					printf(" !P");
					break;

				case ICMP_UNREACH_NEEDFRAG:
					if (mtudisc)
					{
						frag_err();
						goto again;
					} else
					{
						++unreachable;
						printf(" !F-%d", pmtu);
					}
					break;

				case ICMP_UNREACH_SRCFAIL:
					++unreachable;
					printf(" !S");
					break;

				case ICMP_UNREACH_FILTER_PROHIB:
					++unreachable;
					printf(" !X");
					break;

				case ICMP_UNREACH_HOST_PRECEDENCE:
					++unreachable;
					printf(" !V");
					break;

				case ICMP_UNREACH_PRECEDENCE_CUTOFF:
					++unreachable;
					printf(" !C");
					break;

				default:
					++unreachable;
					printf(" !<%d>", code);
					break;
				}
				break;
			}
			if (cc == 0)
			{
				printf(" *");
			} else if (cc && probe == nprobes - 1 && Mflag)
			{
#ifdef ICMP_EXT_VERSION
				decode_extensions(packet, (int)cc);
#endif
			}
			fflush(stdout);
		}
		putchar('\n');
		if (got_there || (unreachable > 0 && unreachable >= nprobes - 1))
			break;
	}

	if (as_path)
		as_shutdown(asn);

	return EXIT_SUCCESS;
}
