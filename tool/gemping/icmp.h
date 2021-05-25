#ifndef __ORDER_LITTLE_ENDIAN__
#define __ORDER_LITTLE_ENDIAN__ 1234
#endif
#ifndef __ORDER_BIG_ENDIAN__
#define __ORDER_BIG_ENDIAN__    4321
#endif
#ifndef __BYTE_ORDER__
# define __BYTE_ORDER__ __ORDER_BIG_ENDIAN__
#endif

#include <sys/socket.h>
#include <netinet/in.h>

#ifdef __PUREC__
#pragma warn -stv
#endif

typedef uint32_t n_time;
typedef uint32_t n_long;


/*
 * Structure of an internet header, naked of options.
 */
struct ip {
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
    unsigned int ip_tos:8;		/* type of service */
    unsigned int ip_hl:4;		/* header length */
    unsigned int ip_v:4;		/* version */
#endif
#if __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
/*
 * Beware: the original definition uses "unsigned char"
 * for the type of bitfields. This is a GNU-C only
 * extension and not portable. Also, declaring
 * ip_tos as unsigned char, but not being part of the bitfield,
 * causes some compilers (like Pure-C) to align it at an even address.
 */
    unsigned int ip_v:4;		/* version */
    unsigned int ip_hl:4;		/* header length */
    unsigned int ip_tos:8;		/* type of service */
#endif
    unsigned short ip_len;		/* total length */
    unsigned short ip_id;		/* identification */
    unsigned short ip_off;		/* fragment offset field */
#define	IP_RF 0x8000			/* reserved fragment flag */
#define	IP_DF 0x4000			/* dont fragment flag */
#define	IP_MF 0x2000			/* more fragments flag */
#define	IP_OFFMASK 0x1fff		/* mask for fragmenting bits */
    uint8_t ip_ttl;			/* time to live */
    uint8_t ip_p;			/* protocol */
    unsigned short ip_sum;		/* checksum */
    struct in_addr ip_src, ip_dst;	/* source and dest address */
};

#define	MAX_IPOPTLEN		40
#define	IPVERSION	4               /* IP version number */
#define	IP_MAXPACKET	65535U		/* maximum packet size */

struct icmphdr
{
  uint8_t type;		/* message type */
  uint8_t code;		/* type sub-code */
  uint16_t checksum;
  union
  {
    struct
    {
      uint16_t	id;
      uint16_t	sequence;
    } echo;			/* echo datagram */
    uint32_t	gateway;	/* gateway address */
    struct
    {
      uint16_t	__glibc_reserved;
      uint16_t	mtu;
    } frag;			/* path mtu discovery */
  } un;
};

/*
 * Internal of an ICMP Router Advertisement
 */
struct icmp_ra_addr
{
  uint32_t ira_addr;
  uint32_t ira_preference;
};

/*
 * Structure of an icmp header.
 */
struct icmp {
	uint8_t	icmp_type;		/* type of message, see below */
	uint8_t	icmp_code;		/* type sub code */
	uint16_t icmp_cksum;		/* ones complement cksum of struct */
	union {
		uint8_t ih_pptr;			/* ICMP_PARAMPROB */
		struct in_addr ih_gwaddr;	/* ICMP_REDIRECT */
		struct ih_idseq {
			uint16_t	icd_id;
			uint16_t	icd_seq;
		} ih_idseq;
		uint32_t ih_void;

	    /* ICMP_UNREACH_NEEDFRAG -- Path MTU Discovery (RFC1191) */
	    struct ih_pmtu
	    {
	      uint16_t ipm_void;
	      uint16_t ipm_nextmtu;
	    } ih_pmtu;

	    struct ih_rtradv
	    {
	      uint8_t irt_num_addrs;
	      uint8_t irt_wpa;
	      uint16_t irt_lifetime;
	    } ih_rtradv;
	} icmp_hun;
#define	icmp_pptr	icmp_hun.ih_pptr
#define	icmp_gwaddr	icmp_hun.ih_gwaddr
#define	icmp_id		icmp_hun.ih_idseq.icd_id
#define	icmp_seq	icmp_hun.ih_idseq.icd_seq
#define	icmp_void	icmp_hun.ih_void
#define	icmp_pmvoid	icmp_hun.ih_pmtu.ipm_void
#define	icmp_nextmtu	icmp_hun.ih_pmtu.ipm_nextmtu
#define	icmp_num_addrs	icmp_hun.ih_rtradv.irt_num_addrs
#define	icmp_wpa	icmp_hun.ih_rtradv.irt_wpa
#define	icmp_lifetime	icmp_hun.ih_rtradv.irt_lifetime
	union {
		struct id_ts {
			n_time its_otime;
			n_time its_rtime;
			n_time its_ttime;
		} id_ts;
		struct id_ip  {
			struct ip idi_ip;
			/* options and then 64 bits of data */
		} id_ip;
	    struct icmp_ra_addr id_radv;
		in_addr_t id_mask;
		uint8_t	id_data[1];
	} icmp_dun;
#define	icmp_otime	icmp_dun.id_ts.its_otime
#define	icmp_rtime	icmp_dun.id_ts.its_rtime
#define	icmp_ttime	icmp_dun.id_ts.its_ttime
#define	icmp_ip		icmp_dun.id_ip.idi_ip
#define	icmp_radv	icmp_dun.id_radv
#define	icmp_mask	icmp_dun.id_mask
#define	icmp_data	icmp_dun.id_data
};



#define	ICMP_ECHOREPLY		0		/* echo reply */
#define	ICMP_UNREACH		3		/* dest unreachable, codes: */
#define		ICMP_UNREACH_NET	0		/* bad net */
#define		ICMP_UNREACH_HOST	1		/* bad host */
#define		ICMP_UNREACH_PROTOCOL	2		/* bad protocol */
#define		ICMP_UNREACH_PORT	3		/* bad port */
#define		ICMP_UNREACH_NEEDFRAG	4		/* IP_DF caused drop */
#define		ICMP_UNREACH_SRCFAIL	5		/* src route failed */
#define		ICMP_UNREACH_NET_UNKNOWN        6	/* unknown net */
#define		ICMP_UNREACH_HOST_UNKNOWN       7	/* unknown host */
#define		ICMP_UNREACH_ISOLATED	        8	/* src host isolated */
#define		ICMP_UNREACH_NET_PROHIB	        9	/* net denied */
#define		ICMP_UNREACH_HOST_PROHIB        10	/* host denied */
#define		ICMP_UNREACH_TOSNET	        11	/* bad tos for net */
#define		ICMP_UNREACH_TOSHOST	        12	/* bad tos for host */
#define		ICMP_UNREACH_FILTER_PROHIB      13	/* admin prohib */
#define		ICMP_UNREACH_HOST_PRECEDENCE    14	/* host prec vio. */
#define		ICMP_UNREACH_PRECEDENCE_CUTOFF  15	/* prec cutoff */
#define	ICMP_SOURCEQUENCH	4		/* packet lost, slow down */
#define	ICMP_REDIRECT		5		/* shorter route, codes: */
#define		ICMP_REDIRECT_NET	0		/* for network */
#define		ICMP_REDIRECT_HOST	1		/* for host */
#define		ICMP_REDIRECT_TOSNET	2		/* for tos and net */
#define		ICMP_REDIRECT_TOSHOST	3		/* for tos and host */
#define	ICMP_ECHO		8		/* echo service */
#define	ICMP_ROUTERADVERT	9			/* router advertisement */
#define	ICMP_ROUTERSOLICIT	10			/* router solicitation */
#define	ICMP_TIMXCEED		11		/* time exceeded, code: */
#define		ICMP_TIMXCEED_INTRANS	0		/* ttl==0 in transit */
#define		ICMP_TIMXCEED_REASS	1		/* ttl==0 in reass */
#define	ICMP_PARAMPROB		12		/* ip header bad */
#define		ICMP_PARAMPROB_OPTABSENT 1		/* req. opt. absent */
#define	ICMP_TSTAMP		13		/* timestamp request */
#define	ICMP_TSTAMPREPLY	14		/* timestamp reply */
#define	ICMP_IREQ			15		/* information request */
#define	ICMP_IREQREPLY		16		/* information reply */
#define	ICMP_MASKREQ		17		/* address mask request */
#define	ICMP_MASKREPLY		18		/* address mask reply */


/*
 * Lower bounds on packet lengths for various types.
 * For the error advice packets must first insure that the
 * packet is large enough to contain the returned ip header.
 * Only then can we do the check to see if 64 bits of packet
 * data have been returned, since we need to check the returned
 * ip header length.
 */
#define	ICMP_MINLEN	8				/* abs minimum */



/*
 * Definitions for options.
 */
#define	IPOPT_EOL		0		/* end of option list */
#define	IPOPT_NOP		1		/* no operation */

#define	IPOPT_RR		7		/* record packet route */
#define	IPOPT_TS		68		/* timestamp */
#define	IPOPT_TIMESTAMP		IPOPT_TS
#define	IPOPT_LSRR		131		/* loose source route */
#define	IPOPT_SATID		136		/* satnet id */
#define	IPOPT_SSRR		137		/* strict source route */
#define	IPOPT_RA		148		/* router alert */


#define	IPOPT_OPTVAL		0		/* option ID */
#define	IPOPT_OLEN		1		/* option length */
#define	IPOPT_OFFSET		2		/* offset within option */
#define	IPOPT_MINOFF		4		/* min value of above */

#define IP_MULTICAST_IF            9	/* in_addr; set/get IP multicast i/f */
#define IP_MULTICAST_TTL          10	/* u_char; set/get IP multicast ttl */
#define IP_MULTICAST_LOOP         11	/* i_char; set/get IP multicast loopback */

#define	MAXTTL		255		/* maximum time to live (seconds) */



#define IP_HDRINCL                 2        /* int; Header is included with data.  */


#if 0
typedef unsigned int __socklen_t;
typedef unsigned short int sa_family_t;
typedef uint16_t in_port_t;
#endif

#define MAXHOSTNAMELEN 64


/* Address families.  */
#define	PF_UNSPEC	0	/* Unspecified.  */
#define PF_INET         2
#define	AF_UNSPEC	PF_UNSPEC
#define AF_INET PF_INET

#define	IFNAMSIZ	16

struct	ifstat {
	uint32_t	in_packets;	/* # input packets */
	uint32_t	in_errors;	/* # input errors */
	uint32_t	out_packets;	/* # output packets */
	uint32_t	out_errors;	/* # output errors */
	uint32_t	collisions;	/* # collisions */	
};

/*
 * Interface request structure used for socket
 * ioctl's.  All interface ioctl's must have parameter
 * definitions which begin with ifr_name.  The
 * remainder may be interface specific.
 */
struct	ifreq {
	char	ifr_name[IFNAMSIZ];		/* if name, e.g. "en0" */
	union {
		struct	sockaddr ifru_addr;
		struct	sockaddr ifru_hwaddr;
		struct	sockaddr ifru_dstaddr;
		struct	sockaddr ifru_broadaddr;
		struct	sockaddr ifru_netmask;
		short	ifru_flags;
		long	ifru_metric;
		long	ifru_mtu;
		struct	ifstat ifru_stats;
		void *ifru_data;
	} ifr_ifru;
#define	ifr_addr	ifr_ifru.ifru_addr	/* address */
#define	ifr_hwaddr	ifr_ifru.ifru_hwaddr	/* hardware address */
#define	ifr_dstaddr	ifr_ifru.ifru_dstaddr	/* other end of p-to-p link */
#define	ifr_broadaddr	ifr_ifru.ifru_broadaddr	/* broadcast address */
#define	ifr_netmask	ifr_ifru.ifru_netmask	/* netmask */
#define	ifr_flags	ifr_ifru.ifru_flags	/* flags */
#define	ifr_metric	ifr_ifru.ifru_metric	/* metric */
#define ifr_mtu		ifr_ifru.ifru_mtu	/* mtu */
#define ifr_stats	ifr_ifru.ifru_stats	/* statistics */
#define	ifr_data	ifr_ifru.ifru_data	/* for use by interface */
};

/*
 * Structure used in SIOCGIFCONF request.
 * Used to retrieve interface configuration
 * for machine (useful for programs which
 * must know all networks accessible).
 */
struct	ifconf {
	short	ifc_len;		/* size of associated buffer */
	union {
		void *ifcu_buf;
		struct	ifreq *ifcu_req;
	} ifc_ifcu;
#define	ifc_buf	ifc_ifcu.ifcu_buf	/* buffer address */
#define	ifc_req	ifc_ifcu.ifcu_req	/* array of structures returned */
};


int sendto(int fd, const void *buf, size_t buflen, int flags, const struct sockaddr *addr, socklen_t addrlen);
int setsockopt(int fd, int level, int optname, const void *optval, socklen_t optlen);
int recvfrom(int fd, void *buf, size_t buflen, int flags, struct sockaddr *addr, socklen_t *addrlen);
int shutdown(int fd, int how);
int socket(int domain, int type, int proto);

/* Possible values left in `h_errno'.  */
#define	NETDB_INTERNAL	-1	/* See errno.  */
#define	NETDB_SUCCESS	0	/* No problem.  */
#define	HOST_NOT_FOUND	1	/* Authoritative Answer Host not found.  */
#define	TRY_AGAIN	2	/* Non-Authoritative Host not found,
				   or SERVERFAIL.  */
#define	NO_RECOVERY	3	/* Non recoverable errors, FORMERR, REFUSED,
				   NOTIMP.  */
#define	NO_DATA		4	/* Valid name, no data record of requested
				   type.  */
#define	NO_ADDRESS	NO_DATA	/* No address, look for MX record.  */


extern int h_errno;
const char *hstrerror(int err_num);

char *inet_ntoa (struct in_addr __in);
in_addr_t inet_aton (const char *cp, struct in_addr *inp);
struct hostent *gethostbyaddr (const void *addr, __socklen_t __len, int __type);
struct hostent *gethostbyname (const char *name);
in_addr_t inet_addr(const char *cp);
struct hostent *gethostent(void);

/*
 * Resolver options
 */
#define RES_INIT	0x00000001UL	/* address initialized */
#define RES_DEBUG	0x00000002UL	/* print debug messages */
#define RES_AAONLY	0x00000004UL	/* authoritative answers only (!IMPL)*/
#define RES_USEVC	0x00000008UL	/* use virtual circuit */
#define RES_PRIMARY	0x00000010UL	/* query primary server only (!IMPL) */
#define RES_IGNTC	0x00000020UL	/* ignore trucation errors */
#define RES_RECURSE	0x00000040UL	/* recursion desired */
#define RES_DEFNAMES	0x00000080UL	/* use default domain name */
#define RES_STAYOPEN	0x00000100UL	/* Keep TCP socket open */
#define RES_DNSRCH		0x00000200UL	/* search up local domain tree */
#define	RES_INSECURE1	0x00000400UL	/* type 1 security disabled */
#define	RES_INSECURE2	0x00000800UL	/* type 2 security disabled */
#define	RES_NOALIASES	0x00001000UL	/* shuts off HOSTALIASES feature */
#define	RES_USE_INET6	0x00002000UL	/* use/map IPv6 in gethostbyname() */
#define RES_ROTATE		0x00004000UL	/* rotate ns list after each query */
#define	RES_NOCHECKNAME	0x00008000UL	/* do not check names for sanity (!IMPL) */
#define	RES_KEEPTSIG	0x00010000UL	/* do not strip TSIG records */
#define	RES_BLAST		0x00020000UL	/* blast all recursive servers */
#define RES_USEBSTRING	0x00040000UL	/* IPv6 reverse lookup with byte
					   strings */
#define RES_NOIP6DOTINT	0x00080000UL	/* Do not use .ip6.int in IPv6
					   reverse lookup */
#define RES_USE_EDNS0	0x00100000UL	/* Use EDNS0.  */
#define RES_SNGLKUP	0x00200000	/* one outstanding request at a time */
#define RES_SNGLKUPREOP	0x00400000UL	/* -"-, but open new socket for each
					   request */
#define RES_USE_DNSSEC	0x00800000UL	/* use DNSSEC using OK bit in OPT */
#define RES_NOTLDQUERY	0x01000000UL	/* Do not look up unqualified name
					   as a TLD.  */

#define RES_DEFAULT	(RES_RECURSE|RES_DEFNAMES|RES_DNSRCH|RES_NOIP6DOTINT)

#ifdef __PUREC__
#define strcasecmp(a,b)		stricmp(a,b)
#define strncasecmp(a,b,c)	strnicmp(a,b,c)
#endif
