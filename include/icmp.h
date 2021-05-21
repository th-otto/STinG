/*--------------------------------------------------------------------------*/


/*
 *   ICMP types.
 */

#define ICMP_ECHOREPLY		0		/* echo reply */
#define ICMP_UNREACH		3		/* dest unreachable, codes: */
#define		ICMP_UNREACH_NET		0	/* bad net */
#define		ICMP_UNREACH_HOST		1	/* bad host */
#define		ICMP_UNREACH_PROTOCOL	2	/* bad protocol */
#define		ICMP_UNREACH_PORT		3	/* bad port */
#define		ICMP_UNREACH_NEEDFRAG	4	/* IP_DF caused drop */
#define		ICMP_UNREACH_SRCFAIL	5	/* src route failed */
#define		ICMP_UNREACH_NET_UNKNOWN 6	/* unknown net */	
#define		ICMP_UNREACH_HOST_UNKNOWN 7	/* unknown host */
#define		ICMP_UNREACH_ISOLATED	8	/* src host isolated */
#define		ICMP_UNREACH_NET_PROHIB	9	/* net denied */
#define		ICMP_UNREACH_HOST_PROHIB 10	/* host denied */
#define		ICMP_UNREACH_TOSNET		11	/* bad tos for net */
#define		ICMP_UNREACH_TOSHOST	12	/* bad tos for host */
#define		ICMP_UNREACH_FILTER_PROHIB	13	/* Packet filtered */
#define		ICMP_UNREACH_HOST_PRECEDENCE 14	/* Precedence violation */
#define		ICMP_UNREACH_PRECEDENCE_CUTOFF 15	/* Precedence cut off */
#define ICMP_SOURCEQUENCH   4       /* IP error : Source quench           */
#define ICMP_REDIRECT       5       /* IP hint : Redirect datagrams       */
#define		ICMP_REDIRECT_NET		0	/* for network */
#define		ICMP_REDIRECT_HOST		1	/* for host */
#define		ICMP_REDIRECT_TOSNET	2	/* for net and tos */
#define		ICMP_REDIRECT_TOSHOST	3	/* for host and tos */
#define ICMP_ALTHOSTADDR	6		/* alternative host address */
#define ICMP_ECHO            8       /* Echo requested                     */
#define ICMP_ROUTERADVERT    9       /* Router advertisement               */
#define		ICMP_ROUTERADVERT_NORMAL 0
#define		ICMP_ROUTERADVERT_NOROUTE 16
#define ICMP_ROUTERSOLICIT   10      /* Router solicitation                */
#define ICMP_TIMXCEED        11      /* Datagram TTL exceeded, discarded   */
#define		ICMP_TIMXCEED_INTRANS	0	/* TTL count exceeded */
#define		ICMP_TIMXCEED_REASS		1	/* Fragment Reass time exceeded	*/
#define ICMP_PARAMPROB       12      /* IP error : Parameter problem       */
#define		ICMP_PARAMPROB_ERRATPTR 0
#define		ICMP_PARAMPROB_OPTABSENT 1 /* req. opt. absent */
#define		ICMP_PARAMPROB_LENGTH	2
#define ICMP_TIMESTAMP       13      /* Timestamp requested                */
#define ICMP_TIMESTAMPREPLY  14      /* Response to timestamp request      */
#define ICMP_INFO_REQUEST    15      /* Information requested (obsolete)   */
#define ICMP_INFO_REPLY      16      /* Response to info req. (obsolete)   */
#define ICMP_MASKREQ         17      /* Subnet mask requested              */
#define ICMP_MASKREPLY       18      /* Response to subnet mask request    */

/* aliases */
#define ICMP_DEST_UNREACH        ICMP_UNREACH
#define ICMP_NET_UNREACH         ICMP_UNREACH_NET
#define ICMP_HOST_UNREACH        ICMP_UNREACH_HOST
#define ICMP_PROT_UNREACH        ICMP_UNREACH_PROTOCOL
#define ICMP_PORT_UNREACH        ICMP_UNREACH_PORT
#define ICMP_FRAG_NEEDED         ICMP_UNREACH_NEEDFRAG
#define ICMP_SR_FAILED           ICMP_UNREACH_SRCFAIL
#define ICMP_NET_UNKNOWN         ICMP_UNREACH_NET_UNKNOWN
#define ICMP_HOST_UNKNOWN        ICMP_UNREACH_HOST_UNKNOWN
#define ICMP_HOST_ISOLATED       ICMP_UNREACH_ISOLATED
#define ICMP_NET_ANO             ICMP_UNREACH_NET_PROHIB
#define ICMP_HOST_ANO            ICMP_UNREACH_HOST_PROHIB
#define ICMP_NET_UNR_TOS         ICMP_UNREACH_TOSNET
#define ICMP_HOST_UNR_TOS        ICMP_UNREACH_TOSHOST
#define ICMP_PKT_FILTERED        ICMP_UNREACH_FILTER_PROHIB
#define ICMP_PREC_VIOLATION      ICMP_UNREACH_HOST_PRECEDENCE
#define ICMP_PREC_CUTOFF         ICMP_UNREACH_PRECEDENCE_CUTOFF

#define ICMP_TIME_EXCEEDED       ICMP_TIMXCEED


struct icmp_header {
	uint8 type;
	uint8 code;
	uint16 checksum;
};

#define	ICMP_MINLEN	8				/* abs minimum */
