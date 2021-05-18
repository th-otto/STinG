/*--------------------------------------------------------------------------*/


/*
 *   ICMP types.
 */

#define  ICMP_ECHOREPLY       0       /* Response to echo request           */
#define  ICMP_DEST_UNREACH    3       /* IP error : Destination unreachable */
#define		ICMP_NET_UNREACH		0	/* bad net */
#define		ICMP_HOST_UNREACH		1	/* bad host */
#define		ICMP_PROT_UNREACH		2	/* bad protocol */
#define		ICMP_PORT_UNREACH		3	/* bad port */
#define		ICMP_FRAG_NEEDED		4	/* IP_DF caused drop */
#define		ICMP_SR_FAILED			5	/* src route failed */
#define		ICMP_NET_UNKNOWN		6	/* unknown net */	
#define		ICMP_HOST_UNKNOWN		7	/* unknown host */
#define		ICMP_HOST_ISOLATED		8	/* src host isolated */
#define		ICMP_NET_ANO			9	/* net denied */
#define		ICMP_HOST_ANO			10	/* host denied */
#define		ICMP_NET_UNR_TOS		11	/* bad tos for net */
#define		ICMP_HOST_UNR_TOS		12	/* bad tos for host */
#define		ICMP_PKT_FILTERED		13	/* Packet filtered */
#define		ICMP_PREC_VIOLATION		14	/* Precedence violation */
#define		ICMP_PREC_CUTOFF		15	/* Precedence cut off */
#define		NR_ICMP_UNREACH			15	/* instead of hardcoding immediate value */
#define  ICMP_SOURCE_QUENCH   4       /* IP error : Source quench           */
#define		ICMP_REDIR_NET			0	/* for network */
#define		ICMP_REDIR_HOST			1	/* for host */
#define		ICMP_REDIR_NETTOS		2	/* for net and tos */
#define		ICMP_REDIR_HOSTTOS		3	/* for host and tos */
#define  ICMP_REDIRECT        5       /* IP hint : Redirect datagrams       */
#define  ICMP_ECHO            8       /* Echo requested                     */
#define  ICMP_ROUTERADVERT    9       /* Router advertisement               */
#define  ICMP_ROUTERSOLICIT   10      /* Router solicitation                */
#define  ICMP_TIME_EXCEEDED   11      /* Datagram TTL exceeded, discarded   */
#define		ICMP_EXC_TTL			0	/* TTL count exceeded */
#define		ICMP_EXC_FRAGTIME		1	/* Fragment Reass time exceeded	*/
#define  ICMP_PARAMETERPROB   12      /* IP error : Parameter problem       */
#define  ICMP_TIMESTAMP       13      /* Timestamp requested                */
#define  ICMP_TIMESTAMPREPLY  14      /* Response to timestamp request      */
#define  ICMP_INFO_REQUEST    15      /* Information requested (obsolete)   */
#define  ICMP_INFO_REPLY      16      /* Response to info req. (obsolete)   */
#define  ICMP_ADDRESS         17      /* Subnet mask requested              */
#define  ICMP_ADDRESSREPLY    18      /* Response to subnet mask request    */

/*
 * ICMP subcodes
 */
#define  ICMP_DU_PRTCL        2
#define  ICMP_DU_PORT         3

struct icmp_header {
	uint8 type;
	uint8 code;
	uint16 checksum;
};

