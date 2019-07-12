/*
 *      tcp.h               (c) Peter Rottengatter  1997
 *                              perot@pallas.amp.uni-hannover.de
 *
 *      Included into the TCP.STX source code files
 */

#ifndef TCP_H
#define TCP_H



/*--------------------------------------------------------------------------*/


/*
 *   Evaluate Precedence
 */

#define	PREC(x)		(((x)>>5) & 7)       /* Calculate Precedence from IP tos  */
#define	PRECMASK(x)	((x) & 0xe0)



/*--------------------------------------------------------------------------*/


/*
 *   State Categories
 */

#define	C_DEFAULT     0        /* Default for invalid states                */
#define	C_LISTEN      1        /* We're listening for connections           */
#define	C_READY       2        /* Ready to receive data                     */
#define	C_FIN         3        /* Nothing to send anyore (FIN sent)         */
#define	C_END         4        /* Nothing to receive anyore (FIN rec'vd)    */
#define	C_CLSD        5        /* We've closed. Nothing can be done.        */



/*--------------------------------------------------------------------------*/


/*
 *   TCP header and chain link structure.
 */

typedef  struct header  {
     uint16    src_port;        /* Port number of sender                    */
     uint16    dest_port;       /* Port number of receiver                  */
     uint32    sequence;        /* Sequence number                          */
     uint32    acknowledge;     /* Acknowledgement number                   */
     unsigned  offset : 4;      /* Data offset (header length)              */
     unsigned  resvd  : 6;      /* Reserved                                 */
     unsigned  urgent : 1;      /* Flag for urgent data present             */
     unsigned  ack    : 1;      /* Flag for acknowledgement segment         */
     unsigned  push   : 1;      /* Flag for push function                   */
     unsigned  reset  : 1;      /* Flag for resetting connection            */
     unsigned  sync   : 1;      /* Flag for synchronizing sequence numbers  */
     unsigned  fin    : 1;      /* Flag for no more data from sender        */
     uint16    window;          /* Receive window                           */
     uint16    chksum;          /* Checksum of all header, options and data */
     uint16    urg_ptr;         /* First byte following urgent data         */
} TCP_HDR;



/*--------------------------------------------------------------------------*/


/*
 *   Values for flags in CONNEC.
 */

#define  FORCE      0x01        /* Flag forcing emission of a TCP segment   */
#define  RETRAN     0x02        /* Flag indicating this is a retransmission */
#define  CLOSING    0x04        /* Flag indicating background close action  */
#define  DISCARD    0x08        /* Flag forcing discarding of CONNEC block  */
#define  BLOCK      0x10        /* Flag indicating blocking TCP_close()     */
#define  FLAG20     0x20
#define  FLAG40     0x40
#define  DEFERRED   0x80



/*--------------------------------------------------------------------------*/


/*
 *   TCP re-sequencing structure.
 */

typedef  struct resequ  {
     struct resequ    *next;         /* Link to next segment in chain       */
     uint8            tos;           /* TOS used by this segment to travel  */
     TCP_HDR          *hdr;          /* Pointer to TCP header data          */
     uint8            *data;         /* Pointer to data in this segment     */
     uint16           data_len;      /* Amount of data in this segment      */
} RESEQU;



/*--------------------------------------------------------------------------*/


/*
 *   TCP connection structure.
 */

typedef  struct connec  {
     uint32    remote_IP_address;    /* Foreign socket IP address           */
     uint16    remote_port;          /* Foreign socket port number          */
     uint32    local_IP_address;     /* Local socket IP address             */
     uint16    local_port;           /* Local socket port number            */
     uint8     state;                /* TCP state of the connection         */
     uint8     flags;                /* Various flags (FORCE, RETRAN, etc.) */
     uint16    mss;                  /* Maximum Segment Size                */
     uint16    mtu;                  /* Maximum Transmission Unit  (for IP) */
     int16     tos;                  /* Type Of Service            (for IP) */
     int16     ttl;                  /* Time To Live               (for IP) */
     CIB       *info;                /* Connection information link         */
     int16     reason;               /* Reason for closing  (in TCLOSED)    */
     int16     net_error;            /* Error to be reported with next call */
     struct {                        /* Structure containing SEND info :    */
         uint32   next;              /*   Next new sequence number to send  */
         uint32   ptr;               /*   Sequence number in transmission   */
         uint32   unack;             /*   First unacknowledged sequence     */
         uint16   window;            /*   Actual size of send window        */
         uint32   lwup_seq;          /*   Sequence of last window update    */
         uint32   lwup_ack;          /*   Acknowledge of last window update */
         int16    total;             /*   Total real data in queue          */
         int16    bufflen;           /*   Maximum amount of data in queue   */
         int16    count;             /*   Data (with flags) in queue        */
         uint32   ini_sequ;          /*   Initial sequence number           */
         uint32   start;
         NDB      *queue;            /*   Send queue                        */
     } send;                         /* End of SEND info                    */
     struct {                        /* Structure containing RECEIVE info : */
         uint32   next;              /*   Next acceptable sequence number   */
         uint16   window;            /*   Actual size of receive window     */
         uint32   lst_next;          /*   Next acceptable sequence number   */
         uint16   lst_win;           /*   Last window size reported         */
         RESEQU   *reseq;            /*   Segment queue for resequencing    */
         int16    count;             /*   Real data in queue                */
         NDB      *queue;            /*   Receive queue                     */
     } recve;                        /* End of RECEIVE info                 */
     struct {                        /* Struct. for retransmission timer :  */
         uint32   start;             /*   Starting time                     */
         uint32   timeout;           /*   Timeout (ms)                      */
         uint8    mode;              /*   Mode flag : Running / Stopped     */
         uint8    backoff;           /*   Retransmission backoff counter    */
     } rtrn;                         /* End of retransmission timer info    */
     struct {                        /* Structure for round trip timer :    */
         uint32   start;             /*   Starting time                     */
         uint8    mode;              /*   Mode flag : Running / Stopped     */
         uint32   sequ;              /*   Sequence number being timed       */
         uint32   smooth;            /*   Smoothed round trip time          */
         uint32   timeout;
     } rtrp;                         /* End of round trip timer info        */
     struct {                        /* Structure for closing timer :       */
         uint32   start;             /*   Starting time                     */
         uint32   timeout;           /*   Timeout time                      */
     } close;                        /* End of closing timer info           */

     uint16 act_pass;
     uint16 lport_orig;
     uint16 rport_orig;
     uint32 remote_IP_address_orig;
     uint32 local_IP_address_orig;
     uint16 handle;
     uint16 o140;
     uint16 o142;
     uint32 smooth_start;
     uint16 o148;

     volatile signed char sema;      /* Semaphore for locking structures    */
     int16     *result;              /* Return deferred results here        */
     uint32    last_work;            /* Last time work has been done        */
     IP_DGRAM  *pending;             /* Pending IP datagrams                */
     struct connec  *next;           /* Link to next connection in chain    */
} CONNEC;



/*--------------------------------------------------------------------------*/


/*
 *   STX internal structure for all protocol relevant information.
 */

typedef  struct tcp_desc  {
     LAYER   generic;              /* Standard layer structure              */
     uint16  mss;                  /* Maximum segment size                  */
     uint16  rcv_window;           /* TCP receive window                    */
     uint16  def_rtt;              /* Initial Round Trip Time in ms         */
     int16   def_ttl;              /* Default value for IP TTL              */
     int16   max_slt;              /* Estimated maximum segment lifetime    */
     uint16  con_out;              /* Outgoing connection attempts          */
     uint16  con_in;               /* Incoming connection attempts          */
     uint16  resets;               /* Counting sent resets                  */
} TCP_CONF;


/*--------------------------------------------------------------------------*/

#define sequ_within_range(actual, low, high) \
	(0 <= (int32)(actual) - (int32)(low) && 0 <= (int32)(high) - (int32)(actual))

#define sequ_outside_range(actual, low, high) \
	(0 >= (int32)(actual) - (int32)(low) && 0 >= (int32)(high) - (int32)(actual))

#define sequ_within(actual, low, high) \
	((0 <= (int32) high - (int32) low) ? \
		sequ_within_range(actual, low, high) : \
        sequ_outside_range(actual, low, high))


/*--------------------------------------------------------------------------*/

void _appl_yield(void);


#ifndef GNU_ASM_NAME
#ifdef __GNUC__
#define GNU_ASM_NAME(x) __asm__(x)
#else
#define GNU_ASM_NAME(x)
#endif
#endif

#ifndef NO_CONST
#  ifdef __GNUC__
#    define NO_CONST(p) __extension__({ union { const void *cs; void *s; } x; x.cs = p; x.s; })
#  else
#    define NO_CONST(p) ((void *)(p))
#  endif
#endif


int16 cdecl TCP_handler(IP_DGRAM *dgram);
void do_output(CONNEC *connec);

extern CONNEC *root_list;
extern TCP_CONF my_conf;
extern uint16 tcp_id;

int16 cdecl timer_function(IP_DGRAM *);
int16 poll_receive(CONNEC *connec);
int16 cdecl do_ICMP(IP_DGRAM *dgram);
void send_sync(CONNEC *connec);

void do_arrive(CONNEC *conn, IP_DGRAM *dgram);

uint16 pull_up(NDB **queue, char *buffer, uint16 length);
uint16 pull_char(NDB *queue, char *buffer, uint16 length);
void do_output(CONNEC *connec);
void update_wind(CONNEC *connec, TCP_HDR *tcph);
int16 trim_segm(CONNEC *connec, IP_DGRAM *dgram, RESEQU **block, int16 make_resequ);
void add_resequ(CONNEC *connec, RESEQU *block);

void process_sync(CONNEC *connec, IP_DGRAM *dgram);
void process_options(CONNEC *connec, IP_DGRAM *dgram);
void send_reset(IP_DGRAM *dgram);
void close_self(CONNEC * connec, int16 reason);
int16 receive(CONNEC *connec, uint8 *buffer, int16 *length, int16 flag);
int16 categorize(CONNEC *connec);
void abort_conn(CONNEC *connec);
void destroy_conn(CONNEC *connec);
int32 cdecl unlink_connect(void *connec);

int16 req_flag(volatile signed char *semaphore) GNU_ASM_NAME("req_flag");
void rel_flag(volatile signed char *semaphore) GNU_ASM_NAME("rel_flag");
IP_DGRAM *get_pending(IP_DGRAM **pointer) GNU_ASM_NAME("get_pending");
uint16 check_sum(uint32 src_ip, uint32 dest_ip, TCP_HDR *packet, uint16 length) GNU_ASM_NAME("check_sum");
int32 cdecl get_sr(void *) GNU_ASM_NAME("get_sr");
int32 cdecl set_sr(void *) GNU_ASM_NAME("set_sr");


#endif /* TCP_H */
