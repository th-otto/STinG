/*
 *      udp.h               (c) Peter Rottengatter  1996
 *                              perot@pallas.amp.uni-hannover.de
 *
 *      Included into the UDP.STX source code files
 */

#ifndef UDP_H
#define UDP_H



/*--------------------------------------------------------------------------*/


/*
 *   UDP header and chain link structure.
 */

typedef  union header  {
     struct {
         uint16  source_port;        /* Source UDP port                     */
         uint16  dest_port;          /* Destination UDP port                */
         uint16  length;             /* UDP length of data                  */
         uint16  checksum;           /* UDP checksum                        */
     } udp;                          /* Structure when used as net packet   */
     struct {
         union header  *next;        /* Link to next data block in chain    */
         uint16        length;       /* Amount of data in this block        */
         uint16        index;        /* Index to data start in this block   */
     } chain;                        /* Structure when used as queued data  */
} UDP_HDR;


/*
 *   UDP connection structure.
 */

typedef  struct connec  {
     uint32   remote_IP_address;     /* Foreign socket IP address           */
     uint16   remote_port;           /* Foreign socket port number          */
     uint32   local_IP_address;      /* Local socket IP address             */
     uint16   local_port;            /* Local socket port number            */
     int16    ttl;                   /* Time To Live (for IP)               */
     uint32   total_data;            /* Total real data in queue            */
     CIB      *info;                 /* Connection information link         */
     int16     net_error;            /* Error to be reported with next call */
     UDP_HDR  *receive_queue;        /* Receive queue                       */
     UDP_HDR  *pending;              /* Pending IP datagrams                */
     volatile signed char semaphore; /* Semaphore for locking structures    */
     uint32   last_work;             /* Last time work has been done        */
     struct connec  *next;           /* Link to next connection in chain    */
} CONNEC;



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


void wait_flag(volatile signed char *semaphore) GNU_ASM_NAME("wait_flag");
int16 req_flag(volatile signed char *semaphore) GNU_ASM_NAME("req_flag");
void rel_flag(volatile signed char *semaphore) GNU_ASM_NAME("rel_flag");
long dis_intrpt(void) GNU_ASM_NAME("dis_intrpt");
long en_intrpt(void) GNU_ASM_NAME("en_intrpt");
UDP_HDR *get_pending(UDP_HDR **pointer) GNU_ASM_NAME("get_pending");
uint16 check_sum(uint32 src_ip, uint32 dest_ip, UDP_HDR *packet, uint16 length) GNU_ASM_NAME("check_sum");

#endif /* UDP_H */
