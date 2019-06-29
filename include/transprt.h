/*
 *      transprt.h          (c) Peter Rottengatter  1996
 *                              perot@pallas.amp.uni-hannover.de
 *
 *      Include this file to use functions from STinG.
 *      i.e.: tcp, udp, etc ...
 */

#ifndef STING_TRANSPRT_H
#define STING_TRANSPRT_H



/*--------------------------------------------------------------------------*/


/*
 *   Data types used throughout STinG.
 */

typedef          char    int8;        /*   Signed  8 bit (char)             */
typedef unsigned char   uint8;        /* Unsigned  8 bit (byte, octet)      */
typedef          short  int16;        /*   Signed 16 bit (int)              */
typedef unsigned short uint16;        /* Unsigned 16 bit (word)             */
typedef          long   int32;        /*   Signed 32 bit                    */
typedef unsigned long  uint32;        /* Unsigned 32 bit (longword)         */


#ifndef TRUE
#define TRUE    1
#endif
#ifndef FALSE
#define FALSE   0
#endif


#ifdef __PUREC__
/* #define cdecl cdecl */
#endif

#ifdef LATTICE
#define  cdecl  __stdargs
#endif

#ifdef __GNUC__
#define  cdecl
#endif

/*--------------------------------------------------------------------------*/


/*
 *   Driver access structure / functions.
 */

#define MAGIC   "STiKmagic"                 /* Magic for DRV_LIST.magic     */
#define CJTAG   "STiK"


typedef struct drv_header {                 /* Header part of TPL structure */
    const char *module;           /* Specific string that can be searched for     */
    const char *author;           /* Any string                                   */
    const char *version;          /* Format `00.00' Version:Revision              */
} DRV_HDR;

/*--------------------------------------------------------------------------*/

/*
 *   STinG global configuration structure.
 */

#define  STIK_CFG_NUM    100

typedef struct _sting_config
{
	uint32 client_ip;					/* IP address of local machine (obsolete)   */
	uint16 ttl;							/* Default TTL for normal packets           */
	char *cv[STIK_CFG_NUM + 1];			/* Space for config variables               */
	int16 max_num_ports;				/* Maximum number of ports supported        */
	uint32 received_data;				/* Counter for data being received          */
	uint32 sent_data;					/* Counter for data being sent              */
	int16 active;						/* Flag for polling being active            */
	int16 thread_rate;					/* Time between subsequent thread calls     */
	int16 frag_ttl;						/* Time To Live for reassembly resources    */
	void *ports;						/* Pointer to first entry in PORT chain     */
	void *drivers;						/* Pointer to first entry in DRIVER chain   */
	void *layers;						/* Pointer to first entry in LAYER chain    */
	void *interupt;						/* List of application interupt handlers    */
	void *icmp;							/* List of application ICMP handlers        */
	int32 stat_all;						/* All datagrams that pass are counted here */
	int32 stat_lo_mem;					/* Dropped due to low memory                */
	int32 stat_ttl_excd;				/* Dropped due to Time-To-Live exceeded     */
	int32 stat_chksum;					/* Dropped due to failed checksum test      */
	int32 stat_unreach;					/* Dropped due to no way to deliver it      */
    void *memory;						/* Pointer to main memory for KRcalls       */
    int16 new_cookie;					/* Flag indicating if new jar was created   */
} STIK_CONFIG;

/*--------------------------------------------------------------------------*/

#define STIK_COOKIE_MAGIC 0x5354694bUL  /* 'STiK' */

/* cookie points to: */
typedef struct drv_list {
	char      magic[10];                    /* Magic string, def'd as MAGIC */
	DRV_HDR * cdecl (*get_dftab) (const char *);  /* Get Driver Function Table    */
	int16     cdecl (*ETM_exec) (const char *);   /* Execute a STinG module       */
	STIK_CONFIG *cfg;                         /* Config structure             */
	BASPAG    *sting_basepage;              /* STinG basepage address       */
} DRV_LIST;

extern DRV_LIST *drivers;


#define get_dftab(x)    (*drivers->get_dftab)(x)
#define ETM_exec(x)     (*drivers->ETM_exec)(x)

#define TRANSPORT_DRIVER    "TRANSPORT_TCPIP"



/*--------------------------------------------------------------------------*/


/*
 *   TCP and UDP port escape flags.
 */

#define  TCP_ACTIVE        0x0000     /* Initiate active connection         */
#define  TCP_PASSIVE       0xffff     /* Initiate passive connection        */
#define  UDP_EXTEND        0x0000     /* Extended addressing scheme         */


/*
 *   TCP miscellaneous flags.
 */

#define  TCP_URGENT        ((void *) -1)     /* Mark urgent position        */
#define  TCP_HALFDUPLEX    (-1)              /* TCP_close() half duplex     */
#define  TCP_IMMEDIATE     (0)               /* TCP_close() immediate       */


/*
 *   TCP connection states.
 */

#define  TCLOSED       0    /* No connection.  Null, void, absent, ...      */
#define  TLISTEN       1    /* Wait for remote request                      */
#define  TSYN_SENT     2    /* Connect request sent, await matching request */
#define  TSYN_RECV     3    /* Wait for connection ack                      */
#define  TESTABLISH    4    /* Connection established, handshake completed  */
#define  TFIN_WAIT1    5    /* Await termination request or ack             */
#define  TFIN_WAIT2    6    /* Await termination request                    */
#define  TCLOSE_WAIT   7    /* Await termination request from local user    */
#define  TCLOSING      8    /* Await termination ack from remote TCP        */
#define  TLAST_ACK     9    /* Await ack of terminate request sent          */
#define  TTIME_WAIT   10    /* Delay, ensures remote has received term' ack */


/*
 *   TCP information block.
 */

typedef struct tcpib {      /* TCP Information Block                        */
    int16  state;           /* Connection state                             */
 } TCPIB;



/*--------------------------------------------------------------------------*/


/*
 *   Buffer for inquiring port names.
 */

typedef  struct pnta {
    void    *opaque;            /* Kernel internal data                     */
    int16   name_len;           /* Length of port name buffer               */
    char    *port_name;         /* Buffer address                           */
 } PNTA;


/*
 *   Command opcodes for cntrl_port().
 */

#define  CTL_KERN_FIRST_PORT      (('K' << 8) | 'F')   /* Kernel            */
#define  CTL_KERN_NEXT_PORT       (('K' << 8) | 'N')   /* Kernel            */
#define  CTL_KERN_FIND_PORT       (('K' << 8) | 'G')   /* Kernel            */

#define  CTL_GENERIC_SET_IP       (('G' << 8) | 'H')   /* Kernel, all ports */
#define  CTL_GENERIC_GET_IP       (('G' << 8) | 'I')   /* Kernel, all ports */
#define  CTL_GENERIC_SET_MASK     (('G' << 8) | 'L')   /* Kernel, all ports */
#define  CTL_GENERIC_GET_MASK     (('G' << 8) | 'M')   /* Kernel, all ports */
#define  CTL_GENERIC_SET_MTU      (('G' << 8) | 'N')   /* Kernel, all ports */
#define  CTL_GENERIC_GET_MTU      (('G' << 8) | 'O')   /* Kernel, all ports */
#define  CTL_GENERIC_GET_MMTU     (('G' << 8) | 'P')   /* Kernel, all ports */
#define  CTL_GENERIC_GET_TYPE     (('G' << 8) | 'T')   /* Kernel, all ports */
#define  CTL_GENERIC_GET_STAT     (('G' << 8) | 'S')   /* Kernel, all ports */
#define  CTL_GENERIC_CLR_STAT     (('G' << 8) | 'C')   /* Kernel, all ports */

#define  CTL_SERIAL_SET_PRTCL     (('S' << 8) | 'P')   /* Serial Driver     */
#define  CTL_SERIAL_GET_PRTCL     (('S' << 8) | 'Q')   /* Serial Driver     */
#define  CTL_SERIAL_SET_LOGBUFF   (('S' << 8) | 'L')   /* Serial Driver     */
#define  CTL_SERIAL_SET_LOGGING   (('S' << 8) | 'F')   /* Serial Driver     */
#define  CTL_SERIAL_SET_AUTH      (('S' << 8) | 'A')   /* Serial Driver     */
#define  CTL_SERIAL_SET_PAP       (('S' << 8) | 'B')   /* Serial Driver     */
#define  CTL_SERIAL_INQ_STATE     (('S' << 8) | 'S')   /* Serial Driver     */

#define  CTL_ETHER_SET_MAC        (('E' << 8) | 'M')   /* EtherNet          */
#define  CTL_ETHER_GET_MAC        (('E' << 8) | 'N')   /* EtherNet          */
#define  CTL_ETHER_INQ_SUPPTYPE   (('E' << 8) | 'Q')   /* EtherNet          */
#define  CTL_ETHER_SET_TYPE       (('E' << 8) | 'T')   /* EtherNet          */
#define  CTL_ETHER_GET_TYPE       (('E' << 8) | 'U')   /* EtherNet          */

#define  CTL_MASQUE_SET_PORT      (('M' << 8) | 'P')   /* Masquerade        */
#define  CTL_MASQUE_GET_PORT      (('M' << 8) | 'Q')   /* Masquerade        */
#define  CTL_MASQUE_SET_MASKIP    (('M' << 8) | 'M')   /* Masquerade        */
#define  CTL_MASQUE_GET_MASKIP    (('M' << 8) | 'N')   /* Masquerade        */
#define  CTL_MASQUE_GET_REALIP    (('M' << 8) | 'R')   /* Masquerade        */



/*--------------------------------------------------------------------------*/


/*
 *   Handler flag values.
 */

#define  HNDLR_SET        0         /* Set new handler if space             */
#define  HNDLR_FORCE      1         /* Force new handler to be set          */
#define  HNDLR_REMOVE     2         /* Remove handler entry                 */
#define  HNDLR_QUERY      3         /* Inquire about handler entry          */



/*--------------------------------------------------------------------------*/


/*
 *   IP packet header.
 */

typedef  struct ip_header {
    unsigned  version   : 4;    /* IP Version                               */
    unsigned  hd_len    : 4;    /* Internet Header Length                   */
    unsigned  tos       : 8;    /* Type of Service                          */
    uint16    length;           /* Total of all header, options and data    */
    uint16    ident;            /* Identification for fragmentation         */
    unsigned  reserved  : 1;    /* Reserved : Must be zero                  */
    unsigned  dont_frg  : 1;    /* Don't fragment flag                      */
    unsigned  more_frg  : 1;    /* More fragments flag                      */
    unsigned  frag_ofst : 13;   /* Fragment offset                          */
    uint8     ttl;              /* Time to live                             */
    uint8     protocol;         /* Protocol                                 */
    uint16    hdr_chksum;       /* Header checksum                          */
    uint32    ip_src;           /* Source IP address                        */
    uint32    ip_dest;          /* Destination IP address                   */
 } IP_HDR;


/*
 *   Internal IP packet representation.
 */

typedef  struct ip_packet {
    IP_HDR    hdr;              /* Header of IP packet                      */
    void      *options;         /* Options data block                       */
    int16     opt_length;       /* Length of options data block             */
    void      *pkt_data;        /* IP packet data block                     */
    int16     pkt_length;       /* Length of IP packet data block           */
    uint32    timeout;          /* Timeout of packet life                   */
    uint32    ip_gateway;       /* Gateway for forwarding this packet       */
    void      *recvd;           /* Receiving port                           */
    struct ip_packet  *next;    /* Next IP packet in IP packet queue        */
 } IP_DGRAM;


/*
 *   Values for protocol field.
 */

#define P_ICMP     1        /* IP assigned number for ICMP                  */
#define P_TCP      6        /* IP assigned number for TCP                   */
#define P_UDP     17        /* IP assigned number for UDP                   */



/*--------------------------------------------------------------------------*/


/*
 *   Network Data Block.  For data delivery.
 */

typedef struct ndb {        /* Network Data Block.  For data delivery       */
    char        *ptr;       /* Pointer to base of block. (For KRfree();)    */
    char        *ndata;     /* Pointer to next data to deliver              */
    uint16      len;        /* Length of remaining data                     */
    struct ndb  *next;      /* Next NDB in chain or NULL                    */
 } NDB;


/*
 *   Addressing information block.
 */

typedef  struct cab {
    uint16      lport;      /* TCP local  port     (ie: local machine)      */
    uint16      rport;      /* TCP remote port     (ie: remote machine)     */
    uint32      rhost;      /* TCP remote IP addr  (ie: remote machine)     */
    uint32      lhost;      /* TCP local  IP addr  (ie: local machine)      */
 } CAB;


/*
 *   Connection information block.
 */

typedef struct cib {        /* Connection Information Block                 */
    uint16      protocol;   /* TCP or UDP or ... 0 means CIB is not in use  */
    CAB         address;    /* Adress information                           */
    uint16      status;     /* Net status. 0 means normal                   */
 } CIB;



/*--------------------------------------------------------------------------*/


/*
 *   Transport structure / functions.
 */

typedef  struct tpl  {
    const char *     module;      /* Specific string that can be searched for     */
    const char *     author;      /* Any string                                   */
    const char *     version;     /* Format `00.00' Version:Revision              */
    void *     cdecl  (* KRmalloc) (int32);
    void       cdecl  (* KRfree) (void *);
    int32      cdecl  (* KRgetfree) (int16);
    void *     cdecl  (* KRrealloc) (void *, int32);
    const char *     cdecl  (* get_err_text) (int16);
    const char *     cdecl  (* getvstr) (const char *);
    int16      cdecl  (* carrier_detect) (void);
    int16      cdecl  (* TCP_open) (uint32, uint16, uint16, uint16);
    int16      cdecl  (* TCP_close) (int16, int16, int16 *);
    int16      cdecl  (* TCP_send) (int16, const void *, int16);
    int16      cdecl  (* TCP_wait_state) (int16, int16, int16);
    int16      cdecl  (* TCP_ack_wait) (int16, int16);
    int16      cdecl  (* UDP_open) (uint32, uint16);
    int16      cdecl  (* UDP_close) (int16);
    int16      cdecl  (* UDP_send) (int16, const void *, int16);
    int16      cdecl  (* CNkick) (int16);
    int16      cdecl  (* CNbyte_count) (int16);
    int16      cdecl  (* CNget_char) (int16);
    NDB *      cdecl  (* CNget_NDB) (int16);
    int16      cdecl  (* CNget_block) (int16, void *, int16);
    void       cdecl  (* housekeep) (void);
    int16      cdecl  (* resolve) (const char *, char **, uint32 *, int16);
    void       cdecl  (* ser_disable) (void);
    void       cdecl  (* ser_enable) (void);
    int16      cdecl  (* set_flag) (int16);
    void       cdecl  (* clear_flag) (int16);
    CIB *      cdecl  (* CNgetinfo) (int16);
    int16      cdecl  (* on_port) (const char *);
    void       cdecl  (* off_port) (const char *);
    int16      cdecl  (* setvstr) (const char *, const char *);
    int16      cdecl  (* query_port) (const char *);
    int16      cdecl  (* CNgets) (int16, char *, int16, char);
    int16      cdecl  (* ICMP_send) (uint32, uint8, uint8, void *, uint16);
    int16      cdecl  (* ICMP_handler) (int16 cdecl (*) (IP_DGRAM *), int16);
    void       cdecl  (* ICMP_discard) (IP_DGRAM *);
    int16      cdecl  (* TCP_info) (int16, TCPIB *);
    int16      cdecl  (* cntrl_port) (const char *, uint32, int16);
 } TPL;

extern TPL *tpl;


/*
 *   Definitions of transport functions for direct use.
 */

#define KRmalloc(x)                      (*tpl->KRmalloc)(x)
#define KRfree(x)                        (*tpl->KRfree)(x)
#define KRgetfree(x)                     (*tpl->KRgetfree)(x)
#define KRrealloc(x,y)                   (*tpl->KRrealloc)(x,y)
#define get_err_text(x)                  (*tpl->get_err_text)(x)
#define getvstr(x)                       (*tpl->getvstr)(x)
#define carrier_detect()                 (*tpl->carrier_detect)()
#define TCP_open(w,x,y,z)                (*tpl->TCP_open)(w,x,y,z)
#define TCP_close(x,y,z)                 (*tpl->TCP_close)(x,y,z)
#define TCP_send(x,y,z)                  (*tpl->TCP_send)(x,y,z)
#define TCP_wait_state(x,y,z)            (*tpl->TCP_wait_state)(x,y,z)
#define TCP_ack_wait(x,y)                (*tpl->TCP_ack_wait)(x,y)
#define UDP_open(x,y)                    (*tpl->UDP_open)(x,y)
#define UDP_close(x)                     (*tpl->UDP_close)(x)
#define UDP_send(x,y,z)                  (*tpl->UDP_send)(x,y,z)
#define CNkick(x)                        (*tpl->CNkick)(x)
#define CNbyte_count(x)                  (*tpl->CNbyte_count)(x)
#define CNget_char(x)                    (*tpl->CNget_char)(x)
#define CNget_NDB(x)                     (*tpl->CNget_NDB)(x)
#define CNget_block(x,y,z)               (*tpl->CNget_block)(x,y,z)
#define CNgetinfo(x)                     (*tpl->CNgetinfo)(x)
#define CNgets(w,x,y,z)                  (*tpl->CNgets)(w,x,y,z)
#define housekeep()                      (*tpl->housekeep)()
#define resolve(w,x,y,z)                 (*tpl->resolve)(w,x,y,z)
#define ser_disable()                    (*tpl->ser_disable)()
#define ser_enable()                     (*tpl->ser_enable)()
#define set_flag(x)                      (*tpl->set_flag)(x)
#define clear_flag(x)                    (*tpl->clear_flag)(x)
#define on_port(x)                       (*tpl->on_port)(x)
#define off_port(x)                      (*tpl->off_port)(x)
#define setvstr(x,y)                     (*tpl->setvstr)(x,y)
#define query_port(x)                    (*tpl->query_port)(x)
#define ICMP_send(v,w,x,y,z)             (*tpl->ICMP_send)(v,w,x,y,z)
#define ICMP_handler(x,y)                (*tpl->ICMP_handler)(x,y)
#define ICMP_discard(x)                  (*tpl->ICMP_discard)(x)
#define TCP_info(x,y)                    (*tpl->TCP_info)(x,y)
#define cntrl_port(x,y,z)                (*tpl->cntrl_port)(x,y,z)



/*--------------------------------------------------------------------------*/


/*
 *   Error return values.
 */

#define  E_NORMAL         0     /* No error occured ...                     */
#define  E_OBUFFULL      -1     /* Output buffer is full                    */
#define  E_NODATA        -2     /* No data available                        */
#define  E_EOF           -3     /* EOF from remote                          */
#define  E_RRESET        -4     /* Reset received from remote               */
#define  E_UA            -5     /* Unacceptable packet received, reset      */
#define  E_NOMEM         -6     /* Something failed due to lack of memory   */
#define  E_REFUSE        -7     /* Connection refused by remote             */
#define  E_BADSYN        -8     /* A SYN was received in the window         */
#define  E_BADHANDLE     -9     /* Bad connection handle used.              */
#define  E_LISTEN        -10    /* The connection is in LISTEN state        */
#define  E_NOCCB         -11    /* No free CCB's available                  */
#define  E_NOCONNECTION  -12    /* No connection matches this packet (TCP)  */
#define  E_CONNECTFAIL   -13    /* Failure to connect to remote port (TCP)  */
#define  E_BADCLOSE      -14    /* Invalid TCP_close() requested            */
#define  E_USERTIMEOUT   -15    /* A user function timed out                */
#define  E_CNTIMEOUT     -16    /* A connection timed out                   */
#define  E_CANTRESOLVE   -17    /* Can't resolve the hostname               */
#define  E_BADDNAME      -18    /* Domain name or dotted dec. bad format    */
#define  E_LOSTCARRIER   -19    /* The modem disconnected                   */
#define  E_NOHOSTNAME    -20    /* Hostname does not exist                  */
#define  E_DNSWORKLIMIT  -21    /* Resolver Work limit reached              */
#define  E_NONAMESERVER  -22    /* No nameservers could be found for query  */
#define  E_DNSBADFORMAT  -23    /* Bad format of DS query                   */
#define  E_UNREACHABLE   -24    /* Destination unreachable                  */
#define  E_DNSNOADDR     -25    /* No address records exist for host        */
#define  E_NOROUTINE     -26    /* Routine unavailable                      */
#define  E_LOCKED        -27    /* Locked by another application            */
#define  E_FRAGMENT      -28    /* Error during fragmentation               */
#define  E_TTLEXCEED     -29    /* Time To Live of an IP packet exceeded    */
#define  E_PARAMETER     -30    /* Problem with a parameter                 */
#define  E_BIGBUF        -31    /* Input buffer is too small for data       */
#define  E_FNAVAIL       -32    /* Function not available                   */
#define  E_LASTERROR      32    /* ABS of last error code in this list      */



/*--------------------------------------------------------------------------*/


#endif /* STING_TRANSPRT_H */
