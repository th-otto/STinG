/*--------------------------------------------------------------------------*/


/*
 *   ICMP types.
 */

#define  ICMP_ECHO_REPLY      0       /* Response to echo request           */
#define  ICMP_DEST_UNREACH    3       /* IP error : Destination unreachable */
#define  ICMP_SRC_QUENCH      4       /* IP error : Source quench           */
#define  ICMP_REDIRECT        5       /* IP hint : Redirect datagrams       */
#define  ICMP_ECHO            8       /* Echo requested                     */
#define  ICMP_ROUTER_AD       9       /* Router advertisement               */
#define  ICMP_ROUTER_SOL      10      /* Router solicitation                */
#define  ICMP_TIME_EXCEED     11      /* Datagram TTL exceeded, discarded   */
#define  ICMP_PARAMETER       12      /* IP error : Parameter problem       */
#define  ICMP_STAMP_REQU      13      /* Timestamp requested                */
#define  ICMP_STAMP_REPLY     14      /* Response to timestamp request      */
#define  ICMP_INFO_REQU       15      /* Information requested (obsolete)   */
#define  ICMP_INFO_REPLY      16      /* Response to info req. (obsolete)   */
#define  ICMP_MASK_REQU       17      /* Subnet mask requested              */
#define  ICMP_MASK_REPLY      18      /* Response to subnet mask request    */

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

