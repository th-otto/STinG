/*
 * Copyright (c) 1985, 1988 Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *	This product includes software developed by the University of
 *	California, Berkeley and its contributors.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include <unistd.h>
#include <stdio.h>
#include <ctype.h>
#include <errno.h>
#include <stdint.h>
#include <string.h>
#include <sys/time.h>

#if (defined(__MINT__) || defined(__PUREC__)) && !defined(__atarist__)
#define __atarist__ 1
#endif

#ifdef __atarist__
#include <netdb.h>
#include "icmp.h"
#include <mint/mintbind.h>
#include <mint/arch/nf_ops.h>
#define SIOCGIFCONF       (('S' << 8) | 12)       /* get iface list */
#define SIOCGIFNETMASK	(('S' << 8) | 21)	/* get iface network mask */
#define SIOCGIFADDR	(('S' << 8) | 15)	/* get iface address */
#ifndef ECONNRESET
#define	ECONNRESET		(316)		/* Connection reset by peer.  */
#endif
#ifndef ETIMEDOUT
#define ETIMEDOUT 320
#endif
#ifndef ECONNREFUSED
#define ECONNREFUSED 321
#endif
#else
#include <sys/param.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <arpa/nameser.h>
#include <netdb.h>
#include <resolv.h>
#include <sys/ioctl.h>
#include <net/if.h>						/* for struct ifconf */
#include <sockios.h>					/* for SIOC* */
#endif

#define NS_MAXDNAME	256	/* maximum domain name */
#define NS_QFIXEDSZ    4       /* #/bytes of fixed data in query */
#define NS_MAXLABEL	63	/* maximum length of domain label */
#define NS_RRFIXEDSZ	10	/* #/bytes of fixed data in r record */
#define NS_PACKETSZ    512     /* maximum packet size */

#undef _PATH_HOSTS
#define _PATH_HOSTS     "U:\\etc\\hosts"
#undef _PATH_HOSTCONF
#define _PATH_HOSTCONF  "U:\\etc\\host.conf"
#define _PATH_HOSTCONF2  "U:\\etc\\host.con"
#undef _PATH_RESCONF
#define _PATH_RESCONF   "U:\\etc\\resolv.conf"
#define _PATH_RESCONF2   "U:\\etc\\resolv.con"


#define	MAXALIASES	35
#define	MAXADDRS	35
#define MAXTRIMDOMAINS  4
#define HOSTDB		_PATH_HOSTS

#define SERVICE_NONE	0
#define SERVICE_BIND	1
#define SERVICE_HOSTS	2
#define SERVICE_NIS		3
#define SERVICE_MAX		3

#define CMD_ORDER	"order"
#define CMD_TRIMDOMAIN	"trim"
#define CMD_HMA		"multi"
#define CMD_SPOOF	"nospoof"
#define CMD_SPOOFALERT	"alert"
#define CMD_REORDER	"reorder"
#define CMD_ON		"on"
#define CMD_OFF		"off"
#define CMD_WARN	"warn"
#define CMD_NOWARN	"warn off"

#define ORD_BIND	"bind"
#define ORD_HOSTS	"hosts"
#define ORD_NIS		"nis"

#define ENV_HOSTCONF	"RESOLV_HOST_CONF"
#define ENV_SERVORDER	"RESOLV_SERV_ORDER"
#define ENV_SPOOF	"RESOLV_SPOOF_CHECK"
#define ENV_TRIM_OVERR	"RESOLV_OVERRIDE_TRIM_DOMAINS"
#define ENV_TRIM_ADD	"RESOLV_ADD_TRIM_DOMAINS"
#define ENV_HMA		"RESOLV_MULTI"
#define ENV_REORDER	"RESOLV_REORDER"

#define TOKEN_SEPARATORS " ,;:"

static int service_order[SERVICE_MAX + 1];
static int service_done = 0;

static char *h_addr_ptrs[MAXADDRS + 1];

static struct hostent host;
static char *host_aliases[MAXALIASES];
static char hostbuf[BUFSIZ + 1];
static struct in_addr host_addr;
static FILE *hostf = NULL;
static char hostaddr[MAXADDRS];
static char *host_addrs[2];
static int stayopen = 0;
static int hosts_multiple_addrs = 0;
static int spoof = 0;
static int spoofalert = 0;
static int reorder = 0;
static char *trimdomain[MAXTRIMDOMAINS];
static char trimdomainbuf[BUFSIZ];
static int numtrimdomains = 0;

#include <stdlib.h>

#if NS_PACKETSZ > 1024
#define	MAXPACKET	NS_PACKETSZ
#else
#define	MAXPACKET	1024
#endif


/*
 * Type values for resources and queries
 */
#define T_A		1		/* host address */
#define T_NS		2		/* authoritative server */
#define T_MD		3		/* mail destination */
#define T_MF		4		/* mail forwarder */
#define T_CNAME		5		/* connonical name */
#define T_SOA		6		/* start of authority zone */
#define T_MB		7		/* mailbox domain name */
#define T_MG		8		/* mail group member */
#define T_MR		9		/* mail rename name */
#define T_NULL		10		/* null resource record */
#define T_WKS		11		/* well known service */
#define T_PTR		12		/* domain name pointer */
#define T_HINFO		13		/* host information */
#define T_MINFO		14		/* mailbox information */
#define T_MX		15		/* mail routing information */
#define T_TXT		16		/* text strings */
#define T_RP		17		/* responsible serson */
#define T_AFSDB		18		/* AFS Data Base location */
#define T_X25		19		/* public switched data network id */
#define T_ISDN		20		/* integrated service digital network */
#define T_RT		21		/* route through */
#define T_NSAP		22		/* RFC 1348, RFC 1637 */
#define T_NSAP_PTR	23		/* RFC 1348, RFC 1637 */
#define T_SIG		24		/* signature, RFC 2065 */
#define	T_KEY		25		/* associated key, RFC 2065 */
#define T_PX		26		/* preference, RFC 1664 */
#define T_GPOS		27		/* geographical location, RFC 1712 */
#define T_AAAA		28		/* ipv6 host address */
#define T_LOC		29		/* location information, RFC 1876 */
	/* non standard */
#define T_UINFO		100		/* user (finger) information */
#define T_UID		101		/* user ID */
#define T_GID		102		/* group ID */
#define T_UNSPEC	103		/* Unspecified format (binary data) */
	/* Query type values which do not appear in resource records */
#define T_AXFR		252		/* transfer zone of authority */
#define T_MAILB		253		/* transfer mailbox records */
#define T_MAILA		254		/* transfer mail agent records */
#define T_ANY		255		/* wildcard match */

/*
 * Values for class field
 */

#define C_IN		1		/* the arpa internet */
#define C_CHAOS		3		/* for chaos net at MIT */
#define C_HS		4		/* for Hesiod name server at MIT */
	/* Query class values which do not appear in resource records */
#define C_ANY		255		/* wildcard match */


/*
 * Structure for query header, the order of the fields is machine and
 * compiler dependent, in our case, the bits within a byte are assignd
 * least significant first, while the order of transmition is most
 * significant first.  This requires a somewhat confusing rearrangement.
 */

typedef struct {
	unsigned short	id;		/* query identification number */
#if __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
			/* fields in third byte */
	unsigned int	qr:1;		/* response flag */
	unsigned int	opcode:4;	/* purpose of message */
	unsigned int	aa:1;		/* authoritive answer */
	unsigned int	tc:1;		/* truncated message */
	unsigned int	rd:1;		/* recursion desired */
			/* fields in fourth byte */
	unsigned int	ra:1;		/* recursion available */
	unsigned int	pr:1;		/* primary server required (non standard) */
	unsigned int	unused:2;	/* unused bits */
	unsigned int	rcode:4;	/* response code */
#else
			/* fields in third byte */
	unsigned int	rd:1;		/* recursion desired */
	unsigned int	tc:1;		/* truncated message */
	unsigned int	aa:1;		/* authoritive answer */
	unsigned int	opcode:4;	/* purpose of message */
	unsigned int	qr:1;		/* response flag */
			/* fields in fourth byte */
	unsigned int	rcode:4;	/* response code */
	unsigned int	unused:2;	/* unused bits */
	unsigned int	pr:1;		/* primary server required (non standard) */
	unsigned int	ra:1;		/* recursion available */
#endif
			/* remaining bytes */
	unsigned short	qdcount;	/* number of question entries */
	unsigned short	ancount;	/* number of answer entries */
	unsigned short	nscount;	/* number of authority entries */
	unsigned short	arcount;	/* number of resource entries */
} HEADER;

/*
 * Defines for handling compressed domain names
 */
#define INDIR_MASK	0xc0

/*
 * Structure for passing resource records around.
 */
struct rrec {
	short	r_zone;			/* zone number */
	short	r_class;		/* class number */
	short	r_type;			/* type number */
	unsigned long	r_ttl;			/* time to live */
	long	r_size;			/* size of data area */
					/* kr: was int */
	char	*r_data;		/* pointer to data */
};


typedef union
{
	HEADER hdr;
	uint8_t buf[MAXPACKET];
} querybuf;

typedef union
{
	long al;
	char ac;
} align;

int h_errno;

#define NAMESERVER_PORT	53

#define	MAXNS			3	/* max # name servers we'll track */
#define MAXDFLSRCH		3	/* # default domain levels to try */
#define	MAXDNSRCH		6	/* max # domains in search path */
#define	LOCALDOMAINPARTS	2	/* min levels in name that is "local" */

#define	RES_TIMEOUT		5	/* min. seconds between retries */


struct state {
	int	retrans;	 	/* retransmition time interval */
	int	retry;			/* number of times to retransmit */
	long options;		/* option flags - see below. */
	int	nscount;		/* number of name servers */
	struct sockaddr_in nsaddr_list[MAXNS];	/* address of name server */
#define	nsaddr nsaddr_list[0]		/* for backward compatibility */
	uint16_t id;		/* current packet id */
	char defdname[NS_MAXDNAME];	/* default domain */
	char *dnsrch[MAXDNSRCH+1];	/* components of domain to search */
};

static struct state _res;

 
/*
 * Currently defined opcodes
 */
#define QUERY		0x0		/* standard query */
#define IQUERY		0x1		/* inverse query */
#define STATUS		0x2		/* nameserver status query */
/*#define xxx		0x3		   0x3 reserved */
	/* non standard */
#define UPDATEA		0x9		/* add resource record */
#define UPDATED		0xa		/* delete a specific resource record */
#define UPDATEDA	0xb		/* delete all nemed resource record */
#define UPDATEM		0xc		/* modify a specific resource record */
#define UPDATEMA	0xd		/* modify all named resource record */

#define ZONEINIT	0xe		/* initial zone transfer */
#define ZONEREF		0xf		/* incremental zone referesh */

/*
 * Currently defined response codes
 */
#define NOERROR		0		/* no error */
#define FORMERR		1		/* format error */
#define SERVFAIL	2		/* server failure */
#define NXDOMAIN	3		/* non existent domain */
#define NOTIMP		4		/* not implemented */
#define REFUSED		5		/* query refused */
	/* non standard */
#define NOCHANGE	0xf		/* update failed to change db */

int __dn_skipname(const uint8_t *comp_dn, const uint8_t *eom);
int dn_expand(const uint8_t *msg, const uint8_t *eomorig, const uint8_t *comp_dn, char *exp_dn, int length);
int dn_comp(const char *exp_dn, uint8_t *comp_dn, int length, uint8_t **dnptrs, uint8_t **lastdnptr);

int res_search(const char *name, int class, int type, uint8_t *answer, int anslen);
int res_query(const char *name, int class, int type, uint8_t *answer, int anslen);
int res_mkquery(int op, const char *dname, int class, int type, char *data, int datalen, struct rrec *newrr, uint8_t *buf, int buflen);
char *__hostalias(const char *name);
int res_init(void);
int res_send(const uint8_t *buf, int buflen, uint8_t *answer, int anslen);
int res_querydomain(const char *name, const char *domain, int class, int type, uint8_t *answer, int anslen);



static void dotrimdomain(char *c)
{
	/* assume c points to the start of a host name; trim off any 
	   domain name matching any of the trimdomains */
	int d, l1, l2;

	for (d = 0; d < numtrimdomains; d++)
	{
		l1 = (int)strlen(trimdomain[d]);
		l2 = (int)strlen(c);
		if (l2 > l1 && strcasecmp(c + l2 - l1, trimdomain[d]) == 0)
			*(c + (l2 - l1)) = '\0';
	}
}


static struct hostent *trim_domains(struct hostent *h)
{
	if (numtrimdomains)
	{
		int i;

		dotrimdomain(h->h_name);
		for (i = 0; h->h_aliases[i]; i++)
			dotrimdomain(h->h_aliases[i]);
	}
	return h;
}

/* reorder_addrs -- by Tom Limoncelli
	Optimize order of an address list.

	gethostbyaddr() usually returns a list of addresses in some
	arbitrary order.  Most programs use the first one and throw the
	rest away.  This routine attempts to find a "best" address and
	swap it into the first position in the list.  "Best" is defined
	as "an address that is on a local subnet".  The search ends after
	one "best" address is found.  If no "best" address is found,
	nothing is changed.

	On first execution, a table is built of interfaces, netmasks,
	and mask'ed addresses.  This is to speed up future queries but
	may require you to reboot after changing internet addresses.
	(doesn't everyone reboot after changing internet addresses?)

	This routine should not be called if gethostbyaddr() is about
	to return only one address.

*/

/* Hal Stern (June 1992) of Sun claimed that more than 4 ethernets in a
Sun 4/690 would not work.  This variable is set to 10 to accomodate our
version of reality */
#define MAXINTERFACES (10)

static void reorder_addrs(struct hostent *h)
{
	static struct
	{
		char iname[16];
		in_addr_t address;
		in_addr_t netmask;
	} itab[MAXINTERFACES], *itp;
	static int numitab = -1;			/* number of used entries in itab */
	struct in_addr **r;					/* pointer to entry in address list */
	struct in_addr tmp;					/* pointer to entry in address list */
	int cnt;

	/***	itab[] contains masked addresses and netmask of each interface.
			numitab is -1 : table is empty.
			numitab is 0  : should never happen.
			numitab is 1,2,3,... :  number of valid entries in the table.
	***/
	if (!numitab)
		return;							/* no entries in table */
	if (numitab == -1)
	{									/* build the table */
		int fd;
		long err;
		struct ifconf ifs;
		struct ifreq ifbuf[MAXINTERFACES];
		struct ifreq *p;
		struct sockaddr_in *q;
		in_addr_t address, netmask;
		int endp;

		/* open a socket */
		fd = socket(PF_INET, SOCK_DGRAM, 0);
		if (fd == -1)
		{
			numitab = 0;
			return;
		}

		/**** get information about the first MAXINTERFACES interfaces ****/
		/* set up the ifconf structure */
		ifs.ifc_len = MAXINTERFACES * sizeof(struct ifreq);
		ifs.ifc_buf = ifbuf;
		/* get a list of interfaces */
		err = Fcntl(fd, &ifs, SIOCGIFCONF);
		if (err < 0)
			return;

		/**** cycle through each interface & get netmask & address ****/
		endp = (int)(ifs.ifc_len / sizeof(struct ifreq));
		itp = itab;
		for (p = ifs.ifc_req; endp; p++, endp--)
		{
			strcpy(itp->iname, p->ifr_name);	/* copy interface name */

			err = Fcntl(fd, p, SIOCGIFNETMASK);	/* get netmask */
			if (err < 0)
				continue;				/* error? skip this interface */
			q = (struct sockaddr_in *) &(p->ifr_addr);
			if (q->sin_family == AF_INET)
				netmask = q->sin_addr.s_addr;
			else
				continue;				/* not internet protocol? skip this interface */

			err = Fcntl(fd, p, SIOCGIFADDR);	/* get address */
			if (err < 0)
				continue;				/* error? skip this interface */
			q = (struct sockaddr_in *) &(p->ifr_addr);
			if (q->sin_family == AF_INET)
				address = q->sin_addr.s_addr;
			else
				continue;				/* not internet protocol? skip this interface */

			/* store the masked address and netmask in the table */
			address = address & netmask;	/* pre-mask the address */
			if (!address)
				continue;				/* funny address? skip this interface */
			itp->address = address;
			itp->netmask = netmask;

			if (numitab == -1)
				numitab = 0;			/* first time through */
			itp++;
			numitab++;
		}
		/**** clean up ****/
		Fclose(fd);
		/**** if we still don't have a table, leave */
		if (!numitab)
			return;
	}

							/**** loop through table for each (address,interface) combo ****/
	for (r = (struct in_addr **) (h->h_addr_list); *r; r++)
	{									/* loop through the addresses */
		for (itp = itab, cnt = numitab; cnt; itp++, cnt--)
		{								/* loop though the interfaces */
			if (((*r)->s_addr & itp->netmask) == itp->address)
			{							/* compare */
				/* We found a match.  Swap it into [0] */
				memcpy(&tmp, ((struct in_addr **) (h->h_addr_list))[0], sizeof(tmp));
				memcpy(((struct in_addr **) (h->h_addr_list))[0], (*r), sizeof(tmp));
				memcpy((*r), &tmp, sizeof(tmp));

				return;					/* found one, don't need to continue */
			}
		}
	}
}


static void init_services(void)
{
	char *cp;
	char *dp;
	char buf[BUFSIZ];
	int cc = 0;
	FILE *fd;
	char *tdp = trimdomainbuf;
	char *hostconf;

	if ((hostconf = getenv(ENV_HOSTCONF)) == NULL)
	{
		hostconf = _PATH_HOSTCONF;
	}
	if ((fd = fopen(hostconf, "r")) == NULL &&
		(fd = fopen(_PATH_HOSTCONF2, "r")) == NULL)
	{
		/* make some assumptions */
		service_order[0] = SERVICE_HOSTS;
		service_order[1] = SERVICE_BIND;
		service_order[2] = SERVICE_NONE;
	} else
	{
		while (fgets(buf, BUFSIZ, fd) != NULL)
		{
			if ((cp = strchr(buf, '\n')) != NULL)
				*cp = '\0';
			if (buf[0] == '#')
				continue;

#define checkbuf(b, cmd) (strncasecmp(b, cmd, strlen(cmd)) == 0)
#define bad_config_format(cmd) 

			if (checkbuf(buf, CMD_ORDER))
			{
				cp = strpbrk(buf, " \t");
				if (!cp)
				{
					bad_config_format(CMD_ORDER);
				} else
				{
					do
					{
						if (cc >= SERVICE_MAX)
							break;
						while (*cp == ' ' || *cp == '\t')
							cp++;
						dp = strpbrk(cp, TOKEN_SEPARATORS);
						if (dp)
							*dp = '\0';
						if (checkbuf(cp, ORD_BIND))
							service_order[cc++] = SERVICE_BIND;
						else if (checkbuf(cp, ORD_HOSTS))
							service_order[cc++] = SERVICE_HOSTS;
						else if (checkbuf(cp, ORD_NIS))
							service_order[cc++] = SERVICE_NIS;
						else
						{
							bad_config_format(CMD_ORDER);
						}

						if (dp)
							cp = ++dp;
					} while (dp != NULL);
					if (cc == 0)
					{
						bad_config_format(CMD_ORDER);
					}
				}
			} else if (checkbuf(buf, CMD_HMA))
			{
				if ((cp = strpbrk(buf, " \t")) != NULL)
				{
					while (*cp == ' ' || *cp == '\t')
						cp++;
					if (checkbuf(cp, CMD_ON))
						hosts_multiple_addrs = 1;
				} else
				{
					bad_config_format(CMD_HMA);
				}
			} else if (checkbuf(buf, CMD_SPOOF))
			{
				if ((cp = strpbrk(buf, " \t")) != NULL)
				{
					while (*cp == ' ' || *cp == '\t')
						cp++;
					if (checkbuf(cp, CMD_ON))
						spoof = 1;
				} else
				{
					bad_config_format(CMD_SPOOF);
				}
			} else if (checkbuf(buf, CMD_SPOOFALERT))
			{
				if ((cp = strpbrk(buf, " \t")) != NULL)
				{
					while (*cp == ' ' || *cp == '\t')
						cp++;
					if (checkbuf(cp, CMD_ON))
						spoofalert = 1;
				} else
				{
					bad_config_format(CMD_SPOOFALERT);
				}
			} else if (checkbuf(buf, CMD_REORDER))
			{
				if ((cp = strpbrk(buf, " \t")) != NULL)
				{
					while (*cp == ' ' || *cp == '\t')
						cp++;
					if (checkbuf(cp, CMD_ON))
						reorder = 1;
				} else
				{
					bad_config_format(CMD_REORDER);
				}
			} else if (checkbuf(buf, CMD_TRIMDOMAIN))
			{
				if (numtrimdomains < MAXTRIMDOMAINS)
				{
					if ((cp = strpbrk(buf, " \t")) != NULL)
					{
						while (*cp == ' ' || *cp == '\t')
							cp++;
						if (cp)
						{
							strcpy(tdp, cp);
							trimdomain[numtrimdomains++] = tdp;
							tdp += strlen(cp) + 1;
						} else
						{
							bad_config_format(CMD_TRIMDOMAIN);
						}
					} else
					{
						bad_config_format(CMD_TRIMDOMAIN);
					}
				}
			}
		}

		service_order[cc] = SERVICE_NONE;
		fclose(fd);
	}

	/* override service_order if environment variable */
	if ((cp = getenv(ENV_SERVORDER)) != NULL)
	{
		cc = 0;
		if ((cp = strtok(cp, TOKEN_SEPARATORS)) != NULL)
		{
			do
			{
				if (cc >= SERVICE_MAX)
					break;
				if (checkbuf(cp, ORD_BIND))
					service_order[cc++] = SERVICE_BIND;
				else if (checkbuf(cp, ORD_HOSTS))
					service_order[cc++] = SERVICE_HOSTS;
				else if (checkbuf(cp, ORD_NIS))
					service_order[cc++] = SERVICE_NIS;
			} while ((cp = strtok(NULL, TOKEN_SEPARATORS)) != NULL);
			service_order[cc] = SERVICE_NONE;
		}
	}

	/* override spoof if environment variable */
	if ((cp = getenv(ENV_SPOOF)) != NULL)
	{
		if (checkbuf(cp, CMD_WARN))
		{
			spoof = 1;
			spoofalert = 1;
		} else if (checkbuf(cp, CMD_OFF))
		{
			spoof = 0;
			spoofalert = 0;
		} else if (checkbuf(cp, CMD_NOWARN))
		{
			spoof = 1;
			spoofalert = 0;
		} else
		{
			spoof = 1;
		}
	}

	/* override hma if environment variable */
	if ((cp = getenv(ENV_HMA)) != NULL)
	{
		if (checkbuf(cp, CMD_ON))
		{
			hosts_multiple_addrs = 1;
		} else
		{
			hosts_multiple_addrs = 0;
		}
	}

	/* override reorder if environment variable */
	if ((cp = getenv(ENV_REORDER)) != NULL)
	{
		if (checkbuf(cp, CMD_ON))
		{
			reorder = 1;
		} else
		{
			reorder = 0;
		}
	}

	/* add trimdomains from environment variable */
	if ((cp = getenv(ENV_TRIM_ADD)) != NULL)
	{
		if ((cp = strtok(cp, TOKEN_SEPARATORS)) != NULL)
		{
			do
			{
				if (numtrimdomains < MAXTRIMDOMAINS)
				{
					strcpy(tdp, cp);
					trimdomain[numtrimdomains++] = tdp;
					tdp += strlen(cp) + 1;
				}
			} while ((cp = strtok(NULL, TOKEN_SEPARATORS)) != NULL);
		}
	}

	/* override trimdomains from environment variable */
	if ((cp = getenv(ENV_TRIM_OVERR)) != NULL)
	{
		numtrimdomains = 0;
		tdp = trimdomainbuf;
		if ((cp = strtok(cp, TOKEN_SEPARATORS)) != NULL)
		{
			do
			{
				if (numtrimdomains < MAXTRIMDOMAINS)
				{
					strcpy(tdp, cp);
					trimdomain[numtrimdomains++] = tdp;
					tdp += strlen(cp) + 1;
				}
			} while ((cp = strtok(NULL, TOKEN_SEPARATORS)) != NULL);
		}
	}

	service_done = 1;
}


/*
 * Skip over a compressed domain name. Return the size or -1.
 */
int __dn_skipname(const uint8_t *comp_dn, const uint8_t *eom)
{
	const uint8_t *cp;
	int n;

	cp = comp_dn;
	while (cp < eom && (n = *cp++) != 0)
	{
		/*
		 * check for indirection
		 */
		switch (n & INDIR_MASK)
		{
		case 0:						/* normal case, n == len */
			cp += n;
			continue;
		default:						/* illegal type */
			return -1;
		case INDIR_MASK:				/* indirection */
			cp++;
		}
		break;
	}
	return (int)(cp - comp_dn);
}


/*
 * Expand compressed domain name 'comp_dn' to full domain name.
 * 'msg' is a pointer to the begining of the message,
 * 'eomorig' points to the first location after the message,
 * 'exp_dn' is a pointer to a buffer of size 'length' for the result.
 * Return size of compressed name or -1 if there was an error.
 */
int dn_expand(const uint8_t *msg, const uint8_t *eomorig, const uint8_t *comp_dn, char *exp_dn, int length)
{
	const uint8_t *cp;
	char *dn;
	int n;
	int c;
	char *eom;
	int len = -1;
	int checked = 0;

	dn = exp_dn;
	cp = comp_dn;
	eom = exp_dn + length;

	/*
	 * fetch next label in domain name
	 */
	while ((n = *cp++) != 0)
	{
		/*
		 * Check for indirection
		 */
		switch (n & INDIR_MASK)
		{
		case 0:
			if (dn != exp_dn)
			{
				if (dn >= eom)
					return -1;
				*dn++ = '.';
			}
			if (dn + n >= eom)
				return -1;
			checked += n + 1;
			while (--n >= 0)
			{
				if ((c = *cp++) == '.')
				{
					if (dn + n + 2 >= eom)
						return -1;
					*dn++ = '\\';
				}
				*dn++ = c;
				if (cp >= eomorig)		/* out of range */
					return -1;
			}
			break;

		case INDIR_MASK:
			if (len < 0)
				len = (int)(cp - comp_dn + 1);
			cp = (u_char *) msg + (((n & 0x3f) << 8) | (*cp & 0xff));
			if (cp < msg || cp >= eomorig)	/* out of range */
				return -1;
			checked += 2;
			/*
			 * Check for loops in the compressed name;
			 * if we've looked at the whole message,
			 * there must be a loop.
			 */
			if (checked >= eomorig - msg)
				return -1;
			break;

		default:
			return -1;				/* flag error */
		}
	}
	*dn = '\0';
	if (len < 0)
		len = (int)(cp - comp_dn);
	return len;
}


/*
 * Search for expanded name from a list of previously compressed names.
 * Return the offset from msg if found or -1.
 * dnptrs is the pointer to the first name on the list,
 * not the pointer to the start of the message.
 */
static int dn_find(const char *exp_dn, uint8_t *msg, uint8_t **dnptrs, uint8_t **lastdnptr)
{
	const char *dn;
	uint8_t *cp;
	uint8_t **cpp;
	int n;
	uint8_t *sp;

	for (cpp = dnptrs; cpp < lastdnptr; cpp++)
	{
		dn = exp_dn;
		sp = cp = *cpp;
		while ((n = *cp++) != 0)
		{
			/*
			 * check for indirection
			 */
			switch (n & INDIR_MASK)
			{
			case 0:					/* normal case, n == len */
				while (--n >= 0)
				{
					if (*dn == '.')
						goto next;
					if (*dn == '\\')
						dn++;
					if ((uint8_t) *dn++ != *cp++)
						goto next;
				}
				if ((n = *dn++) == '\0' && *cp == '\0')
					return (int)(sp - msg);
				if (n == '.')
					continue;
				goto next;

			default:					/* illegal type */
				return -1;

			case INDIR_MASK:			/* indirection */
				cp = msg + (((n & 0x3f) << 8) | *cp);
				break;
			}
		}
		if (*dn == '\0')
			return (int)(sp - msg);
	  next:;
	}
	return -1;
}


/*
 * Compress domain name 'exp_dn' into 'comp_dn'.
 * Return the size of the compressed name or -1.
 * 'length' is the size of the array pointed to by 'comp_dn'.
 * 'dnptrs' is a list of pointers to previous compressed names. dnptrs[0]
 * is a pointer to the beginning of the message. The list ends with NULL.
 * 'lastdnptr' is a pointer to the end of the arrary pointed to
 * by 'dnptrs'. Side effect is to update the list of pointers for
 * labels inserted into the message as we compress the name.
 * If 'dnptr' is NULL, we don't try to compress names. If 'lastdnptr'
 * is NULL, we don't update the list.
 */
int dn_comp(const char *exp_dn, uint8_t *comp_dn, int length, uint8_t **dnptrs, uint8_t **lastdnptr)
{
	uint8_t *cp;
	const char *dn;
	int c, l;
	uint8_t **cpp = NULL;
	uint8_t **lpp = NULL;
	uint8_t *sp;
	uint8_t *eob;
	uint8_t *msg;

	dn = exp_dn;
	cp = comp_dn;
	eob = cp + length;
	if (dnptrs != NULL)
	{
		if ((msg = *dnptrs++) != NULL)
		{
			for (cpp = dnptrs; *cpp != NULL; cpp++)
				;
			lpp = cpp;					/* end of list to search */
		}
	} else
	{
		msg = NULL;
	}
	for (c = *dn++; c != '\0';)
	{
		/* look to see if we can use pointers */
		if (msg != NULL)
		{
			if ((l = dn_find(dn - 1, msg, dnptrs, lpp)) >= 0)
			{
				if (cp + 1 >= eob)
					return -1;
				*cp++ = (l >> 8) | INDIR_MASK;
				*cp++ = l % 256;
				return (int)(cp - comp_dn);
			}
			/* not found, save it */
			if (lastdnptr != NULL && cpp < lastdnptr - 1)
			{
				*cpp++ = cp;
				*cpp = NULL;
			}
		}
		sp = cp++;						/* save ptr to length byte */
		do
		{
			if (c == '.')
			{
				c = *dn++;
				break;
			}
			if (c == '\\')
			{
				if ((c = *dn++) == '\0')
					break;
			}
			if (cp >= eob)
			{
				if (msg != NULL)
					*lpp = NULL;
				return -1;
			}
			*cp++ = c;
		} while ((c = *dn++) != '\0');
		/* catch trailing '.'s but not '..' */
		if ((l = (int)(cp - sp - 1)) == 0 && c == '\0')
		{
			cp--;
			break;
		}
		if (l <= 0 || l > NS_MAXLABEL)
		{
			if (msg != NULL)
				*lpp = NULL;
			return -1;
		}
		*sp = l;
	}
	if (cp >= eob)
	{
		if (msg != NULL)
			*lpp = NULL;
		return -1;
	}
	*cp++ = '\0';
	return (int)(cp - comp_dn);
}


static uint16_t _getshort(uint8_t *msgp)
{
	uint8_t *p = msgp;
	uint16_t u;

	u = *p++ << 8;
	return u | *p;
}


#if 0 /* unused */
static uint32_t _getlong(uint8_t *msgp)
{
	uint8_t *p = msgp;
	uint32_t u;

	u = *p++; u <<= 8;
	u |= *p++; u <<= 8;
	u |= *p++; u <<= 8;
	return u | *p;
}
#endif


static struct hostent *getanswer(querybuf *answer, int anslen, int iquery)
{
	HEADER *hp;
	uint8_t *cp;
	int n;
	uint8_t *eom;
	char *bp;
	char **ap;
	int type;
	int class;
	int buflen;
	int ancount;
	int qdcount;
	int haveanswer;
	int getclass = C_ANY;
	char **hap;

	eom = answer->buf + anslen;
	/*
	 * find first satisfactory answer
	 */
	hp = &answer->hdr;
	ancount = ntohs(hp->ancount);
	qdcount = ntohs(hp->qdcount);
	bp = hostbuf;
	buflen = (int)sizeof(hostbuf);
	cp = answer->buf + sizeof(HEADER);
	if (qdcount)
	{
		if (iquery)
		{
			if ((n = dn_expand(answer->buf, eom, cp, bp, buflen)) < 0)
			{
				h_errno = NO_RECOVERY;
				return NULL;
			}
			cp += n + NS_QFIXEDSZ;
			host.h_name = bp;
			n = (int)strlen(bp) + 1;
			bp += n;
			buflen -= n;
		} else
		{
			cp += __dn_skipname(cp, eom) + NS_QFIXEDSZ;
		}
		while (--qdcount > 0)
			cp += __dn_skipname(cp, eom) + NS_QFIXEDSZ;
	} else if (iquery)
	{
		if (hp->aa)
			h_errno = HOST_NOT_FOUND;
		else
			h_errno = TRY_AGAIN;
		return NULL;
	}
	ap = host_aliases;
	*ap = NULL;
	host.h_aliases = host_aliases;
	hap = h_addr_ptrs;
	*hap = NULL;
	host.h_addr_list = h_addr_ptrs;
	haveanswer = 0;
	while (--ancount >= 0 && cp < eom)
	{
		if ((n = dn_expand(answer->buf, eom, cp, bp, buflen)) < 0)
			break;
		cp += n;
		type = _getshort(cp);
		cp += 2;
		class = _getshort(cp);
		/* skip TTL */
		cp += 6;
		/* get RDLENGTH */
		n = _getshort(cp);
		cp += 2;
		if (type == T_CNAME)
		{
			cp += n;
			if (ap >= &host_aliases[MAXALIASES - 1])
				continue;
			*ap++ = bp;
			n = (int)strlen(bp) + 1;
			bp += n;
			buflen -= n;
			continue;
		}
		if (iquery && type == T_PTR)
		{
			if ((n = dn_expand(answer->buf, eom, cp, bp, buflen)) < 0)
				break;
			cp += n;
			host.h_name = bp;
			return &host;
		}
		if (iquery || type != T_A)
		{
			cp += n;
			continue;
		}
		if (haveanswer)
		{
			if (n != (int) host.h_length)
			{
				cp += n;
				continue;
			}
			if (class != getclass)
			{
				cp += n;
				continue;
			}
		} else
		{
			host.h_length = n;
			getclass = class;
			host.h_addrtype = class == C_IN ? AF_INET : AF_UNSPEC;
			if (!iquery)
			{
				host.h_name = bp;
				bp += strlen(bp) + 1;
			}
		}

		bp += sizeof(align) - ((size_t) bp % sizeof(align));

		if (bp + n >= &hostbuf[sizeof(hostbuf)])
		{
			break;
		}
		*hap++ = bp;
		memcpy(bp, cp, n);
		bp += n;
		cp += n;
		haveanswer++;
	}
	if (haveanswer)
	{
		*ap = NULL;
		*hap = NULL;
		return &host;
	} else
	{
		h_errno = TRY_AGAIN;
		return NULL;
	}
}


static void _sethtent(void)
{
	if (hostf == NULL)
		hostf = fopen(_PATH_HOSTS, "r");
	else
		rewind(hostf);
}


static void _endhtent(void)
{
	if (hostf && !stayopen)
	{
		fclose(hostf);
		hostf = NULL;
	}
}


/* if hosts_multiple_addrs set, then gethtbyname behaves as follows:
 *  - for hosts with multiple addresses, return all addresses, such that
 *  the first address is most likely to be one on the same net as the
 *  host we're running on, if one exists. 
 *  - like the dns version of gethostsbyname, the alias field is empty
 *  unless the name being looked up is an alias itself, at which point the
 *  alias field contains that name, and the name field contains the primary
 *  name of the host. Unlike dns, however, this behavior will still take place
 *  even if the alias applies only to one of the interfaces. 
 *  - determining a "local" address to put first is dependant on the netmask 
 *  being such that the least significant network bit is more significant 
 *  than any host bit. Only strange netmasks will violate this. 
 *  - we assume addresses fit into u_longs. That's quite internet specific.
 *  - if the host we're running on is not in the host file, the address 
 *  shuffling will not take place.
 *                     - John DiMarco <jdd@cdf.toronto.edu>
 */
static struct hostent *_gethtbyname(const char *name)
{
	struct hostent *p;
	char **cp;
	char **hap;
	char **lhap;
	char *bp;
	char *lbp;
	socklen_t htbuflen;
	socklen_t locbuflen;
	int found = 0;
	int localfound = 0;
	char localname[MAXHOSTNAMELEN];

	static char htbuf[BUFSIZ + 1];		/* buffer for host addresses */
	static char locbuf[BUFSIZ + 1];		/* buffer for local hosts's addresses */
	static char *ht_addr_ptrs[MAXADDRS + 1];
	static char *loc_addr_ptrs[MAXADDRS + 1];
	static struct hostent ht;
	static char *aliases[MAXALIASES];
	static char namebuf[MAXHOSTNAMELEN];

	hap = ht_addr_ptrs;
	lhap = loc_addr_ptrs;
	*hap = NULL;
	*lhap = NULL;
	bp = htbuf;
	lbp = locbuf;
	htbuflen = (int)sizeof(htbuf);
	locbuflen = (int)sizeof(locbuf);

	aliases[0] = NULL;
	aliases[1] = NULL;
	strcpy(namebuf, name);

	gethostname(localname, sizeof(localname));

	_sethtent();
	while ((p = gethostent()) != NULL)
	{
		if (strcasecmp(p->h_name, name) == 0)
		{
			found++;
		} else
		{
			for (cp = p->h_aliases; *cp != 0; cp++)
				if (strcasecmp(*cp, name) == 0)
				{
					found++;
					aliases[0] = (char *) name;
					strcpy(namebuf, p->h_name);
				}
		}
		if (strcasecmp(p->h_name, localname) == 0)
		{
			localfound++;
		} else
		{
			for (cp = p->h_aliases; *cp != 0; cp++)
				if (strcasecmp(*cp, localname) == 0)
					localfound++;
		}

		if (found)
		{
			socklen_t n;

			if (!hosts_multiple_addrs)
			{
				/* original behaviour requested */
				_endhtent();
				return p;
			}
			n = p->h_length;

			ht.h_addrtype = p->h_addrtype;
			ht.h_length = p->h_length;

			if (n <= htbuflen)
			{
				/* add the found address to the list */
				memcpy(bp, p->h_addr_list[0], n);
				*hap++ = bp;
				*hap = NULL;
				bp += n;
				htbuflen -= n;
			}
			found = 0;
		}
		if (localfound)
		{
			socklen_t n = p->h_length;

			if (n <= locbuflen)
			{
				/* add the found local address to the list */
				memcpy(lbp, p->h_addr_list[0], n);
				*lhap++ = lbp;
				*lhap = NULL;
				lbp += n;
				locbuflen -= n;
			}
			localfound = 0;
		}
	}
	_endhtent();

	if (ht_addr_ptrs[0] == NULL)
	{
		return NULL;
	}

	ht.h_aliases = aliases;
	ht.h_name = namebuf;

	/* shuffle addresses around to ensure one on same net as local host 
	   is first, if exists */
	{
		/* "best" address is assumed to be the one with the greatest
		   number of leftmost bits matching any of the addresses of
		   the local host. This assumes a netmask in which all net
		   bits precede host bits. Usually but not always a fair 
		   assumption. */

		/* portability alert: assumption: iaddr fits in u_long.
		   This is really internet specific. */
		int i;
		int j;
		int best = 0;
		in_addr_t bestval = (in_addr_t) ~0;

		for (i = 0; loc_addr_ptrs[i]; i++)
		{
			for (j = 0; ht_addr_ptrs[j]; j++)
			{
				/* FIXME: What is h good for?  */
				in_addr_t t;
				in_addr_t l;
				in_addr_t h = 0;

				memcpy(&t, loc_addr_ptrs[i], ht.h_length);
				l = ntohl(t);
				memcpy(&t, ht_addr_ptrs[j], ht.h_length);
				t = l ^ h;

				if (t < bestval)
				{
					best = j;
					bestval = t;
				}
			}
		}
		if (best)
		{
			char *tmp;

			/* swap first and best address */
			tmp = ht_addr_ptrs[0];
			ht_addr_ptrs[0] = ht_addr_ptrs[best];
			ht_addr_ptrs[best] = tmp;
		}
	}

	ht.h_addr_list = ht_addr_ptrs;
	return &ht;
}



static void __putlong(uint32_t l, uint8_t *msgp)
{
	msgp[3] = l;
	msgp[2] = (l >>= 8);
	msgp[1] = (l >>= 8);
	msgp[0] = l >> 8;
}


static void __putshort(uint16_t s, uint8_t *msgp)
{
	msgp[1] = s;
	msgp[0] = s >> 8;
}


/*
 * Set up default settings.  If the configuration file exist, the values
 * there will have precedence.  Otherwise, the server address is set to
 * INADDR_ANY and the default domain name comes from the gethostname().
 *
 * As of 4.4 BSD the default name server address is 127.0.0.1. So we do.
 *
 * The configuration file should only be used if you want to redefine your
 * domain or run without a server on your machine.
 *
 * Return 0 if completes successfully, -1 on error
 */
int res_init(void)
{
	FILE *fp;
	char *cp;
	char **pp;
	int n;
	char buf[BUFSIZ];
	int nserv = 0;						/* number of nameserver records read from file */
	int haveenv = 0;
	int havesearch = 0;

	_res.retrans = RES_TIMEOUT;
	_res.retry = 4;
	_res.options = RES_DEFAULT;
	_res.nscount = 1;
	_res.nsaddr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
	_res.nsaddr.sin_family = htons(AF_INET);
	_res.nsaddr.sin_port = htons(NAMESERVER_PORT);

	/* Allow user to override the local domain definition */
	if ((cp = getenv("LOCALDOMAIN")) != NULL)
	{
		strncpy(_res.defdname, cp, sizeof(_res.defdname));
		haveenv++;
	}

	if ((fp = fopen(_PATH_RESCONF, "r")) != NULL ||
		(fp = fopen(_PATH_RESCONF2, "r")) != NULL)
	{
		/* read the config file */
		while (fgets(buf, (int)sizeof(buf), fp) != NULL)
		{
			/* read default domain name */
			if (strncmp(buf, "domain", sizeof("domain") - 1) == 0)
			{
				if (haveenv)			/* skip if have from environ */
					continue;
				cp = buf + sizeof("domain") - 1;
				while (*cp == ' ' || *cp == '\t')
					cp++;
				if ((*cp == '\0') || (*cp == '\n'))
					continue;
				(void) strncpy(_res.defdname, cp, sizeof(_res.defdname) - 1);
				if ((cp = strchr(_res.defdname, '\n')) != NULL)
					*cp = '\0';
				havesearch = 0;
				continue;
			}
			/* set search list */
			if (strncmp(buf, "search", sizeof("search") - 1) == 0)
			{
				if (haveenv)			/* skip if have from environ */
					continue;
				cp = buf + sizeof("search") - 1;
				while (*cp == ' ' || *cp == '\t')
					cp++;
				if ((*cp == '\0') || (*cp == '\n'))
					continue;
				(void) strncpy(_res.defdname, cp, sizeof(_res.defdname) - 1);
				if ((cp = strchr(_res.defdname, '\n')) != NULL)
					*cp = '\0';
				/*
				 * Set search list to be blank-separated strings
				 * on rest of line.
				 */
				cp = _res.defdname;
				pp = _res.dnsrch;
				*pp++ = cp;
				for (n = 0; *cp && pp < _res.dnsrch + MAXDNSRCH; cp++)
				{
					if (*cp == ' ' || *cp == '\t')
					{
						*cp = 0;
						n = 1;
					} else if (n)
					{
						*pp++ = cp;
						n = 0;
					}
				}
				/* null terminate last domain if there are excess */
				while (*cp != '\0' && *cp != ' ' && *cp != '\t')
					cp++;
				*cp = '\0';
				*pp++ = 0;
				havesearch = 1;
				continue;
			}
			/* read nameservers to query */
			if (strncmp(buf, "nameserver", sizeof("nameserver") - 1) == 0 && nserv < MAXNS)
			{
				cp = buf + sizeof("nameserver") - 1;
				while (*cp == ' ' || *cp == '\t')
					cp++;
				if (*cp == '\0' || *cp == '\n')
					continue;
				if ((_res.nsaddr_list[nserv].sin_addr.s_addr = inet_addr(cp)) == INADDR_NONE)
				{
					_res.nsaddr_list[nserv].sin_addr.s_addr = INADDR_ANY;
					continue;
				}
				_res.nsaddr_list[nserv].sin_family = AF_INET;
				_res.nsaddr_list[nserv].sin_port = htons(NAMESERVER_PORT);
				nserv++;
				continue;
			}
		}
		if (nserv > 1)
			_res.nscount = nserv;
		(void) fclose(fp);
	}
	if (_res.defdname[0] == 0)
	{
		if (gethostname(buf, sizeof(_res.defdname)) == 0 && (cp = strchr(buf, '.')) != NULL)
			strcpy(_res.defdname, cp + 1);
	}

	/* find components of local domain that might be searched */
	if (havesearch == 0)
	{
		pp = _res.dnsrch;
		*pp++ = _res.defdname;
		for (cp = _res.defdname, n = 0; *cp; cp++)
			if (*cp == '.')
				n++;
		cp = _res.defdname;
		for (; n >= LOCALDOMAINPARTS && pp < _res.dnsrch + MAXDFLSRCH; n--)
		{
			cp = strchr(cp, '.');
			*pp++ = ++cp;
		}
		*pp++ = 0;
	}
	_res.options |= RES_INIT;
	return 0;
}


int res_send(const uint8_t *buf, int buflen, uint8_t *answer, int anslen)
{
	int n = 0;					/* Shut up compiler warning.  */
	int try;
	int v_circuit;
	int resplen;
	int ns;
	int gotsomewhere = 0;
	int connected = 0;
	int connreset = 0;
	uint16_t id;
	uint16_t len;
	uint8_t *cp;
	fd_set dsmask;
	struct timeval timeout;
	const HEADER *hp = (const HEADER *) buf;
	HEADER *anhp = (HEADER *) answer;
	struct iovec iov[2];
	int terrno = ETIMEDOUT;
	char junk[512];

	static int s = -1;	/* socket used for communications */
	static struct sockaddr no_addr;

	if ((_res.options & RES_INIT) == 0)
		res_init();
	v_circuit = (_res.options & RES_USEVC) || buflen > NS_PACKETSZ;
	id = hp->id;
	/*
	 * Send request, RETRY times, or until successful
	 */
	for (try = 0; try < _res.retry; try++)
	{
		for (ns = 0; ns < _res.nscount; ns++)
		{
		  usevc:
			if (v_circuit)
			{
				int truncated = 0;

				/*
				 * Use virtual circuit;
				 * at most one attempt per server.
				 */
				try = _res.retry;
				if (s < 0)
				{
					s = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
					if (s < 0)
					{
						terrno = errno;
						continue;
					}
					if (connect(s, (struct sockaddr *) &(_res.nsaddr_list[ns]), sizeof(struct sockaddr)) < 0)
					{
						terrno = errno;
						Fclose(s);
						s = -1;
						continue;
					}
				}
				/*
				 * Send length & message
				 */
				len = htons((uint16_t) buflen);
				iov[0].iov_base = &len;
				iov[0].iov_len = sizeof(len);
				iov[1].iov_base = (void *)buf;
				iov[1].iov_len = buflen;
				{
					struct msghdr msg;

					msg.msg_name = 0;
					msg.msg_namelen = 0;
					msg.msg_iov = iov;
					msg.msg_iovlen = 2;
					msg.msg_control = 0;
					msg.msg_controllen = 0;

					if ((n = (int)Fsendmsg(s, &msg, 0)) != sizeof(len) + buflen)
					{
						terrno = -n;
						Fclose(s);
						s = -1;
						continue;
					}
				}
				/*
				 * Receive length & response
				 */
				cp = answer;
				len = sizeof(uint16_t);
				while (len != 0 && (n = (int)Fread(s, len, cp)) > 0)
				{
					cp += n;
					len -= n;
				}
				if (n <= 0)
				{
					terrno = -n;
					Fclose(s);
					s = -1;
					/*
					 * A long running process might get its TCP
					 * connection reset if the remote server was
					 * restarted.  Requery the server instead of
					 * trying a new one.  When there is only one
					 * server, this means that a query might work
					 * instead of failing.  We only allow one reset
					 * per query to prevent looping.
					 */
					if (terrno == ECONNRESET && !connreset)
					{
						connreset = 1;
						ns--;
					}
					continue;
				}
				cp = answer;
				if ((resplen = ntohs(*(uint16_t *) cp)) > anslen)
				{
					len = anslen;
					truncated = 1;
				} else
				{
					len = resplen;
				}
				while (len != 0 && (n = (int)Fread(s, len, cp)) > 0)
				{
					cp += n;
					len -= n;
				}
				if (n <= 0)
				{
					terrno = -n;
					Fclose(s);
					s = -1;
					continue;
				}
				if (truncated)
				{
					/*
					 * Flush rest of answer
					 * so connection stays in synch.
					 */
					anhp->tc = 1;
					len = resplen - anslen;
					while (len != 0)
					{
						n = len > sizeof(junk) ? (int)sizeof(junk) : len;
						if ((n = (int)Fread(s, n, junk)) > 0)
							len -= n;
						else
							break;
					}
				}
			} else
			{
				/*
				 * Use datagrams.
				 */
				if (s < 0)
				{
					s = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);
					if (s < 0)
					{
						terrno = errno;
						continue;
					}
				}
				/*
				 * I'm tired of answering this question, so:
				 * On a 4.3BSD+ machine (client and server,
				 * actually), sending to a nameserver datagram
				 * port with no nameserver will cause an
				 * ICMP port unreachable message to be returned.
				 * If our datagram socket is "connected" to the
				 * server, we get an ECONNREFUSED error on the next
				 * socket operation, and select returns if the
				 * error message is received.  We can thus detect
				 * the absence of a nameserver without timing out.
				 * If we have sent queries to at least two servers,
				 * however, we don't want to remain connected,
				 * as we wish to receive answers from the first
				 * server to respond.
				 */
				if (_res.nscount == 1 || (try == 0 && ns == 0))
				{
					/*
					 * Don't use connect if we might
					 * still receive a response
					 * from another server.
					 */
					if (connected == 0)
					{
						if (connect(s, (struct sockaddr *) &_res.nsaddr_list[ns], sizeof(struct sockaddr)) < 0)
						{
							continue;
						}
						connected = 1;
					}
					if (send(s, buf, buflen, 0) != buflen)
					{
						continue;
					}
				} else
				{
					/*
					 * Disconnect if we want to listen
					 * for responses from more than one server.
					 */
					if (connected)
					{
						connect(s, &no_addr, sizeof(no_addr));
						connected = 0;
					}
					if (sendto(s, buf, buflen, 0,
							   (struct sockaddr *) &_res.nsaddr_list[ns], sizeof(struct sockaddr)) != buflen)
					{
						continue;
					}
				}
				/*
				 * Wait for reply
				 */
				timeout.tv_sec = _res.retrans << try;
				if (try > 0)
					timeout.tv_sec /= _res.nscount;
				if (timeout.tv_sec <= 0)
					timeout.tv_sec = 1;
				timeout.tv_usec = 0;
			  wait:
				FD_ZERO(&dsmask);
				FD_SET(s, &dsmask);
				n = select(s + 1, &dsmask, NULL, NULL, &timeout);
				if (n < 0)
				{
					continue;
				}
				if (n == 0)
				{
					/*
					 * timeout
					 */
					gotsomewhere = 1;
					continue;
				}
				if ((resplen = recv(s, answer, anslen, 0)) <= 0)
				{
					continue;
				}
				gotsomewhere = 1;
				if (id != anhp->id)
				{
					/*
					 * response from old query, ignore it
					 */
					goto wait;
				}
				if (!(_res.options & RES_IGNTC) && anhp->tc)
				{
					/*
					 * get rest of answer;
					 * use TCP with same server.
					 */
					Fclose(s);
					s = -1;
					v_circuit = 1;
					goto usevc;
				}
			}
			/*
			 * If using virtual circuits, we assume that the first server
			 * is preferred * over the rest (i.e. it is on the local
			 * machine) and only keep that one open.
			 * If we have temporarily opened a virtual circuit,
			 * or if we haven't been asked to keep a socket open,
			 * close the socket.
			 */
			if ((v_circuit && ((_res.options & RES_USEVC) == 0 || ns != 0)) || (_res.options & RES_STAYOPEN) == 0)
			{
				Fclose(s);
				s = -1;
			}
			return resplen;
		}
	}
	if (s >= 0)
	{
		Fclose(s);
		s = -1;
	}
	if (v_circuit == 0)
	{
		if (gotsomewhere == 0)
		{
			__set_errno(ECONNREFUSED);	/* no nameservers found */
		} else
		{
			__set_errno(ETIMEDOUT);		/* no answer obtained */
		}
	} else
	{
		__set_errno(terrno);
	}
	return -1;
}


/*
 * Formulate a normal query, send, and await answer.
 * Returned answer is placed in supplied buffer "answer".
 * Perform preliminary check of answer, returning success only
 * if no error is indicated and the answer count is nonzero.
 * Return the size of the response on success, -1 on error.
 * Error number is left in h_errno.
 * Caller must parse answer and determine whether it answers the question.
 */
int res_query(const char *name,			/* domain name */
			  int class, int type,		/* class and type of query */
			  uint8_t *answer,			/* buffer to put answer */
			  int anslen)				/* size of answer buffer */
{
	uint8_t buf[MAXPACKET];
	HEADER *hp;
	int n;

	if ((_res.options & RES_INIT) == 0)
		res_init();
	n = res_mkquery(QUERY, name, class, type, NULL, 0, NULL, buf, (int)sizeof(buf));

	if (n <= 0)
	{
		h_errno = NO_RECOVERY;
		return n;
	}
	n = res_send(buf, n, answer, anslen);
	if (n < 0)
	{
		h_errno = TRY_AGAIN;
		return n;
	}

	hp = (HEADER *) answer;
	if (hp->rcode != NOERROR || ntohs(hp->ancount) == 0)
	{
		switch (hp->rcode)
		{
		case NXDOMAIN:
			h_errno = HOST_NOT_FOUND;
			break;
		case SERVFAIL:
			h_errno = TRY_AGAIN;
			break;
		case NOERROR:
			h_errno = NO_DATA;
			break;
		case FORMERR:
		case NOTIMP:
		case REFUSED:
		default:
			h_errno = NO_RECOVERY;
			break;
		}
		return -1;
	}
	return n;
}


/*
 * Form all types of queries.
 * Returns the size of the result or -1.
 */
int res_mkquery(int op,					/* opcode of query */
				const char *dname,		/* domain name */
				int class, int type,	/* class and type of query */
				char *data,				/* resource record data */
				int datalen,			/* length of data */
				struct rrec *newrr,		/* new rr for modify or append */
				uint8_t *buf,				/* buffer to put query */
				int buflen)				/* size of buffer */
{
	HEADER *hp;
	uint8_t *cp;
	int n;
	uint8_t *dnptrs[10];
	uint8_t **dpp;
	uint8_t **lastdnptr;

	(void) newrr;

	/*
	 * Initialize header fields.
	 */
	if (buf == NULL || buflen < (int)sizeof(HEADER))
		return -1;
	memset(buf, 0, sizeof(HEADER));
	hp = (HEADER *) buf;
	hp->id = htons(++_res.id);
	hp->opcode = op;
	hp->pr = (_res.options & RES_PRIMARY) != 0;
	hp->rd = (_res.options & RES_RECURSE) != 0;
	hp->rcode = NOERROR;
	cp = buf + sizeof(HEADER);
	buflen -= (int)sizeof(HEADER);
	dpp = dnptrs;
	*dpp++ = buf;
	*dpp++ = NULL;
	lastdnptr = dnptrs + sizeof(dnptrs) / sizeof(dnptrs[0]);
	/*
	 * perform opcode specific processing
	 */
	switch (op)
	{
	case QUERY:
		if ((buflen -= NS_QFIXEDSZ) < 0)
			return -1;
		if ((n = dn_comp(dname, cp, buflen, dnptrs, lastdnptr)) < 0)
			return -1;
		cp += n;
		buflen -= n;
		__putshort(type, cp);
		cp += 2;
		__putshort(class, cp);
		cp += 2;
		hp->qdcount = htons(1);
		if (op == QUERY || data == NULL)
			break;
		/*
		 * Make an additional record for completion domain.
		 */
		buflen -= NS_RRFIXEDSZ;
		if ((n = dn_comp(data, cp, buflen, dnptrs, lastdnptr)) < 0)
			return -1;
		cp += n;
		buflen -= n;
		__putshort(T_NULL, cp);
		cp += 2;
		__putshort(class, cp);
		cp += 2;
		__putlong(0, cp);
		cp += 4;
		__putshort(0, cp);
		cp += 2;
		hp->arcount = htons(1);
		break;

	case IQUERY:
		/*
		 * Initialize answer section
		 */
		if (buflen < 1 + NS_RRFIXEDSZ + datalen)
			return -1;
		*cp++ = '\0';					/* no domain name */
		__putshort(type, cp);
		cp += 2;
		__putshort(class, cp);
		cp += 2;
		__putlong(0, cp);
		cp += 4;
		__putshort(datalen, cp);
		cp += 2;
		if (datalen)
		{
			memcpy(cp, data, datalen);
			cp += datalen;
		}
		hp->ancount = htons(1);
		break;

#ifdef ALLOW_UPDATES
		/*
		 * For UPDATEM/UPDATEMA, do UPDATED/UPDATEDA followed by UPDATEA
		 * (Record to be modified is followed by its replacement in msg.)
		 */
	case UPDATEM:
	case UPDATEMA:

	case UPDATED:
		/*
		 * The res code for UPDATED and UPDATEDA is the same; user
		 * calls them differently: specifies data for UPDATED; server
		 * ignores data if specified for UPDATEDA.
		 */
	case UPDATEDA:
		buflen -= RRFIXEDSZ + datalen;
		if ((n = dn_comp(dname, cp, buflen, dnptrs, lastdnptr)) < 0)
			return -1;
		cp += n;
		__putshort(type, cp);
		cp += 2;
		__putshort(class, cp);
		cp += 2;
		__putlong(0, cp);
		cp += 4;
		__putshort(datalen, cp);
		cp += 2;
		if (datalen)
		{
			memcpy(cp, data, datalen);
			cp += datalen;
		}
		if ((op == UPDATED) || (op == UPDATEDA))
		{
			hp->ancount = htons(0);
			break;
		}
		/* Else UPDATEM/UPDATEMA, so drop into code for UPDATEA */

	case UPDATEA:						/* Add new resource record */
		buflen -= RRFIXEDSZ + datalen;
		if ((n = dn_comp(dname, cp, buflen, dnptrs, lastdnptr)) < 0)
			return -1;
		cp += n;
		__putshort(newrr->r_type, cp);
		cp += 2;
		__putshort(newrr->r_class, cp);
		cp += 2;
		__putlong(0, cp);
		cp += 4;
		__putshort(newrr->r_size, cp);
		cp += 2;
		if (newrr->r_size)
		{
			memcpy(cp, newrr->r_data, newrr->r_size);
			cp += newrr->r_size;
		}
		hp->ancount = htons(0);
		break;
#endif /* ALLOW_UPDATES */
	}
	return (int)(cp - buf);
}


/*
 * Perform a call on res_query on the concatenation of name and domain,
 * removing a trailing dot from name if domain is NULL.
 */
int res_querydomain(
	const char *name,
	const char *domain,
	int class, int type,	/* class and type of query */
	uint8_t *answer,	/* buffer to put answer */
	int anslen)			/* size of answer */
{
	char nbuf[2 * NS_MAXDNAME + 2];
	const char *longname = nbuf;
	int n;

	if (domain == NULL)
	{
		/*
		 * Check for trailing '.';
		 * copy without '.' if present.
		 */
		n = (int)strlen(name) - 1;
		if (name[n] == '.' && n < (int) sizeof(nbuf) - 1)
		{
			memcpy(nbuf, name, n);
			nbuf[n] = '\0';
		} else
		{
			longname = name;
		}
	} else
	{
		sprintf(nbuf, "%.*s.%.*s", NS_MAXDNAME, name, NS_MAXDNAME, domain);
	}

	return res_query(longname, class, type, answer, anslen);
}


char *__hostalias(const char *name)
{
	char *C1;
	char *C2;
	FILE *fp;
	char *file;
	char buf[BUFSIZ];
	static char abuf[NS_MAXDNAME];

	file = getenv("HOSTALIASES");
	if (file == NULL || (fp = fopen(file, "r")) == NULL)
		return NULL;
	buf[sizeof(buf) - 1] = '\0';
	while (fgets(buf, (int)sizeof(buf), fp))
	{
		for (C1 = buf; *C1 && !isspace(*C1); ++C1) ;
		if (!*C1)
			break;
		*C1 = '\0';
		if (strcasecmp(buf, name) == 0)
		{
			while (isspace(*++C1))
				;
			if (!*C1)
				break;
			for (C2 = C1 + 1; *C2 && !isspace(*C2); ++C2)
				;
			abuf[sizeof(abuf) - 1] = *C2 = '\0';
			strncpy(abuf, C1, sizeof(abuf) - 1);
			fclose(fp);
			return abuf;
		}
	}
	fclose(fp);
	return NULL;
}


/*
 * Formulate a normal query, send, and retrieve answer in supplied buffer.
 * Return the size of the response on success, -1 on error.
 * If enabled, implement search rules until answer or unrecoverable failure
 * is detected.  Error number is left in h_errno.
 * Only useful for queries in the same name hierarchy as the local host
 * (not, for example, for host address-to-name lookups in domain in-addr.arpa).
 */
int res_search(const char *name,				/* domain name */
			   int class, int type,		/* class and type of query */
			   uint8_t *answer,		/* buffer to put answer */
			   int anslen)				/* size of answer */
{
	const char *cp;
	char **domain;
	int n;
	int ret;
	int got_nodata = 0;

	if ((_res.options & RES_INIT) == 0)
		res_init();

	__set_errno(0);
	h_errno = HOST_NOT_FOUND;			/* default, if we never query */
	for (cp = name, n = 0; *cp; cp++)
		if (*cp == '.')
			n++;
	if (n == 0 && (cp = __hostalias(name)) != NULL)
		return res_query(cp, class, type, answer, anslen);

	/*
	 * We do at least one level of search if
	 *  - there is no dot and RES_DEFNAME is set, or
	 *  - there is at least one dot, there is no trailing dot,
	 *    and RES_DNSRCH is set.
	 */
	if ((n == 0 && (_res.options & RES_DEFNAMES)) ||
		(n != 0 && *--cp != '.' && (_res.options & RES_DNSRCH)))
	{
		for (domain = _res.dnsrch; *domain; domain++)
		{
			ret = res_querydomain(name, *domain, class, type, answer, anslen);
			if (ret > 0)
				return ret;
			/*
			 * If no server present, give up.
			 * If name isn't found in this domain,
			 * keep trying higher domains in the search list
			 * (if that's enabled).
			 * On a NO_DATA error, keep trying, otherwise
			 * a wildcard entry of another type could keep us
			 * from finding this entry higher in the domain.
			 * If we get some other error (negative answer or
			 * server failure), then stop searching up,
			 * but try the input name below in case it's fully-qualified.
			 */
			if (errno == ECONNREFUSED)
			{
				h_errno = TRY_AGAIN;
				return -1;
			}
			if (h_errno == NO_DATA)
				got_nodata++;
			if ((h_errno != HOST_NOT_FOUND && h_errno != NO_DATA) || (_res.options & RES_DNSRCH) == 0)
				break;
		}
	}
	/*
	 * If the search/default failed, try the name as fully-qualified,
	 * but only if it contained at least one dot (even trailing).
	 * This is purely a heuristic; we assume that any reasonable query
	 * about a top-level domain (for servers, SOA, etc) will not use
	 * res_search.
	 */
	if (n && (ret = res_querydomain(name, NULL, class, type, answer, anslen)) > 0)
		return ret;
	if (got_nodata)
		h_errno = NO_DATA;
	return -1;
}


struct hostent *gethostbyname(const char *name)
{
	querybuf buf;
	const char *cp;
	int cc;
	int n;
	struct hostent *hp;

	/*
	 * disallow names consisting only of digits/dots, unless
	 * they end in a dot.
	 */
	if (isdigit(name[0]))
	{
		for (cp = name;; ++cp)
		{
			if (!*cp)
			{
				if (*--cp == '.')
					break;
				/*
				 * All-numeric, no dot at the end.
				 * Fake up a hostent as if we'd actually
				 * done a lookup.
				 */
				if (!inet_aton(name, &host_addr))
				{
					h_errno = HOST_NOT_FOUND;
					return NULL;
				}
				host.h_name = (char *) name;
				host.h_aliases = host_aliases;
				host_aliases[0] = NULL;
				host.h_addrtype = AF_INET;
				host.h_length = sizeof(in_addr_t);
				h_addr_ptrs[0] = (char *) &host_addr;
				h_addr_ptrs[1] = NULL;
				host.h_addr_list = h_addr_ptrs;
				return &host;
			}
			if (!isdigit(*cp) && *cp != '.')
				break;
		}
	}

	if (!service_done)
		init_services();

	for (cc = 0; service_order[cc] != SERVICE_NONE && cc <= SERVICE_MAX; cc++)
	{
		switch (service_order[cc])
		{
		case SERVICE_BIND:
			if ((n = res_search(name, C_IN, T_A, buf.buf, (int)sizeof(buf))) < 0)
			{
				break;
			}
			hp = getanswer(&buf, n, 0);
			if (h_addr_ptrs[1] && reorder)
				reorder_addrs(hp);
			if (hp)
				return trim_domains(hp);
			break;

		case SERVICE_HOSTS:
			hp = _gethtbyname(name);
			if (h_addr_ptrs[1] && reorder)
				reorder_addrs(hp);
			if (hp)
				return hp;
			h_errno = HOST_NOT_FOUND;
			break;
		}
	}
	return NULL;
}


static struct hostent *_gethtbyaddr(const unsigned char *addr, __socklen_t len, int type)
{
	struct hostent *p;

	_sethtent();
	while ((p = gethostent()) != NULL)
		if (p->h_addrtype == type && memcmp(p->h_addr, addr, len) == 0)
			break;
	_endhtent();
	return p;
}


struct hostent *gethostbyaddr(const void *__addr, __socklen_t len, int type)
{
	const unsigned char *addr = (const unsigned char *) __addr;
	int n;
	querybuf buf;
	int cc;
	struct hostent *hp;
	char qbuf[NS_MAXDNAME];

	if (type != AF_INET)
		return NULL;
	if (!service_done)
		init_services();

	cc = 0;
	while (service_order[cc] != SERVICE_NONE)
	{
		switch (service_order[cc])
		{
		case SERVICE_BIND:
			sprintf(qbuf, "%u.%u.%u.%u.in-addr.arpa", addr[3], addr[2], addr[1], addr[0]);
			n = res_query(qbuf, C_IN, T_PTR, (uint8_t *) &buf, (int)sizeof(buf));
			if (n < 0)
			{
				break;
			}
			hp = getanswer(&buf, n, 1);
			if (hp)
			{
				if (spoof)
				{
					/* Spoofing check code by
					 * Caspar Dik <casper@fwi.uva.nl> 
					 */
					char nambuf[NS_MAXDNAME + 1];
					int ntd;
					int namelen = (int)strlen(hp->h_name);
					char **addrs;

					if (namelen >= NS_MAXDNAME)
						return NULL;
					strcpy(nambuf, hp->h_name);
					nambuf[namelen] = '.';
					nambuf[namelen + 1] = '\0';

					/* 
					 * turn off domain trimming,
					 * call gethostbyname(), then turn  
					 * it back on, if applicable. This
					 * prevents domain trimming from
					 * making the name comparison fail.
					 */
					ntd = numtrimdomains;
					numtrimdomains = 0;
					hp = gethostbyname(nambuf);
					numtrimdomains = ntd;
					nambuf[namelen] = 0;
					/*
					 * the name must exist and the name 
					 * returned by gethostbyaddr must be 
					 * the canonical name and therefore 
					 * identical to the name returned by 
					 * gethostbyname()
					 */
					if (!hp || strcmp(nambuf, hp->h_name))
					{
						h_errno = HOST_NOT_FOUND;
						return NULL;
					}
					/*
					 * now check the addresses
					 */
					for (addrs = hp->h_addr_list; *addrs; addrs++)
					{
						if (memcmp(addrs[0], addr, len) == 0)
							return trim_domains(hp);
					}
					/* We've been spoofed */
					h_errno = HOST_NOT_FOUND;
					return NULL;
				}
				hp->h_addrtype = type;
				hp->h_length = len;
				h_addr_ptrs[0] = (char *) &host_addr;
				h_addr_ptrs[1] = (char *) 0;
				host_addr = *(struct in_addr *) addr;
				return trim_domains(hp);
			}
			h_errno = HOST_NOT_FOUND;
			break;

		case SERVICE_HOSTS:
			hp = _gethtbyaddr(addr, len, type);
			if (hp)
				return hp;
			h_errno = HOST_NOT_FOUND;
			break;
		}
		cc++;
	}
	return NULL;
}


struct hostent *gethostent(void)
{
	char *p;
	char *cp;
	char **q;

	if (hostf == NULL && (hostf = fopen(_PATH_HOSTS, "r")) == NULL)
		return NULL;

	for (;;)
	{
		if ((p = fgets(hostbuf, BUFSIZ, hostf)) == NULL)
			return NULL;
		if (*p == '#')
			continue;
		cp = strpbrk(p, "#\n");
		if (cp == NULL)
			continue;
		*cp = '\0';
		cp = strpbrk(p, " \t");
		if (cp != NULL)
			break;
	}
	*cp++ = '\0';
	/* THIS STUFF IS INTERNET SPECIFIC */
	host.h_addr_list = host_addrs;
	host.h_addr = hostaddr;
	*((in_addr_t *) host.h_addr) = inet_addr(p);
	host.h_length = sizeof(in_addr_t);
	host.h_addrtype = AF_INET;
	while (*cp == ' ' || *cp == '\t')
		cp++;
	host.h_name = cp;
	q = host.h_aliases = host_aliases;
	cp = strpbrk(cp, " \t");
	if (cp != NULL)
		*cp++ = '\0';
	while (cp && *cp)
	{
		if (*cp == ' ' || *cp == '\t')
		{
			cp++;
			continue;
		}
		if (q < &host_aliases[MAXALIASES - 1])
			*q++ = cp;
		cp = strpbrk(cp, " \t");
		if (cp != NULL)
			*cp++ = '\0';
	}
	*q = NULL;
	return &host;
}
