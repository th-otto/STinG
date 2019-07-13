/*
 *      globdefs.h          (c) Peter Rottengatter  1996
 *                              perot@pallas.amp.uni-hannover.de
 *
 *      Included into all STinG source code files
 */

#include "transprt.h"
#include "port.h"
#include "layer.h"

#ifndef UNUSED
#define UNUSED(x) ((void)(x))
#endif


#define  TCP_DRIVER_VERSION    "01.26"
#define  STX_LAYER_VERSION     "01.06"
#define  LOOPBACK              0x7f000001L

#define  MAX_HANDLE    64    /* Number of handles assigned by PRTCL_request */

#ifndef GNU_ASM_NAME
#ifdef __GNUC__
#define GNU_ASM_NAME(x) __asm__(x)
#else
#define GNU_ASM_NAME(x)
#endif
#endif

/*
 *   Defragmentation queue entries.
 */

typedef  struct defrag_rsc {
    IP_DGRAM  *dgram;           /* Datagram to be reassembled               */
    uint16    ttl_data;         /* Total data length for defragmentation    */
    uint16    act_space;        /* Current space of reassembly buffer       */
    void      *blk_bits;        /* Fragment block bits table                */
    struct defrag_rsc  *next;   /* Next defrag resources in defrag queue    */
 } DEFRAG;


/*
 *   Protocol array entry for received data.
 */

typedef  struct protocol_entry {
    int16     active;           /* Protocol is installed                    */
    IP_DGRAM  *queue;           /* Link to first entry in received queue    */
    DEFRAG    *defrag;          /* Link to defragmentation queue            */
    int16  cdecl  (* process) (IP_DGRAM *);   /* Call to process packet     */
 } IP_PRTCL;



/*--------------------------------------------------------------------------*/


/*
 *   Entry definition for function chain.
 */

typedef  struct func_list {
    int16    cdecl    (* handler) (IP_DGRAM *);
    struct func_list  *next;
 } FUNC_LIST;



/*--------------------------------------------------------------------------*/


/*
 *   Entry for routing table.
 */

typedef  struct route_entry {
    uint32  template;           /* Net to be reached this way               */
    uint32  netmask;            /* Corresponding subnet mask                */
    uint32  ip_gateway;         /* Next gateway on the way to dest. host    */
    PORT    *port;              /* Port to route the datagram to            */
 } ROUTE_ENTRY;


/*
 *   Router return values.
 */

#define  NET_UNREACH     ((void *)  0L)    /* No entry for IP found         */
#define  HOST_UNREACH    ((void *) -1L)    /* Entry found but port inactive */
#define  NO_NETWORK      ((void *) -6L)    /* Routing table empty           */
#define  NO_HOST         ((void *) -7L)    /* Currently unused              */



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



/*--------------------------------------------------------------------------*/


/*
 *   Miscellaneous Definitions.
 */

#define  MAX_CLOCK    86400000L      /* Maximum value for sting_clock       */



/*--------------------------------------------------------------------------*/

/*
 * undef accessor macros from public headers
 */
#undef KRmalloc
#undef KRfree
#undef KRgetfree
#undef KRrealloc
#undef get_err_text
#undef getvstr
#undef carrier_detect
#undef TCP_open
#undef TCP_close
#undef TCP_send
#undef TCP_wait_state
#undef TCP_ack_wait
#undef UDP_open
#undef UDP_close
#undef UDP_send
#undef CNkick
#undef CNbyte_count
#undef CNget_char
#undef CNget_NDB
#undef CNget_block
#undef CNgetinfo
#undef CNgets
#undef housekeep
#undef resolve
#undef ser_disable
#undef ser_enable
#undef set_flag
#undef clear_flag
#undef on_port
#undef off_port
#undef setvstr
#undef query_port
#undef ICMP_send
#undef ICMP_handler
#undef ICMP_discard
#undef TCP_info
#undef cntrl_port
#undef UDP_info
#undef RAW_open
#undef RAW_close
#undef RAW_out
#undef CN_setopt
#undef CN_getopt
#undef CNfree_NDB

#undef set_dgram_ttl
#undef check_dgram_ttl
#undef load_routing_table
#undef set_sysvars
#undef query_chains
#undef IP_send
#undef IP_fetch
#undef IP_handler
#undef IP_discard
#undef PRTCL_announce
#undef PRTCL_get_parameters
#undef PRTCL_request
#undef PRTCL_release
#undef PRTCL_lookup
#undef TIMER_call
#undef TIMER_now
#undef TIMER_elapsed
#undef protect_exec
#undef get_route_entry
#undef set_route_entry


extern STING_CONFIG conf;
extern PORT my_port;
extern IP_PRTCL ip[];
extern uint32 sting_clock GNU_ASM_NAME("sting_clock");
extern void *icmp;
extern char sting_path[];

typedef struct mem_header
{
	struct mem_header *mem_ptr;
	uint32 size;
} MEM_HDR;

extern MEM_HDR *memory;

typedef struct cn_desc
{
	int16 handle;
	void *anonymous;
	CN_FUNCS *funcs;
	struct cn_desc *next_desc;
} CN_DESC;


extern CN_DESC *cn_array[MAX_HANDLE];



int16 cdecl setvstr(const char *name, const char *value);
const char *cdecl getvstr(const char *name);

int32 cdecl set_sysvars(int16 new_act, int16 new_frac);
void cdecl query_chains(PORT **port, DRIVER **drv, LAYER **layer);
const char *cdecl get_error_text(int16 error_code);

int16 cdecl set_flag(int16 flag) GNU_ASM_NAME("set_flag");
void cdecl clear_flag(int16 flag) GNU_ASM_NAME("clear_flag");
int32 cdecl protect_exec(void *parameter, int32 cdecl(*code) (void *)) GNU_ASM_NAME("protect_exec");

int16 cdecl on_port(const char *port);
void cdecl off_port(const char *port);
int16 cdecl query_port(const char *port);
int16 cdecl cntrl_port(const char *port, uint32 argument, int16 code);

void cdecl set_dgram_ttl(IP_DGRAM *datagram);
int16 cdecl check_dgram_ttl(IP_DGRAM *datagram);

int16 cdecl get_route_entry(int16 no, uint32 *tmplt, uint32 *mask, PORT **port, uint32 *gway);
int16 cdecl set_route_entry(int16 no, uint32 tmplt, uint32 mask, PORT *port, uint32 gway);
int16 cdecl routing_table(void);
int16 cdecl IP_send(uint32 source, uint32 dest, uint8 tos, uint16 fragm_flg, uint8 ttl, uint8 protocol, uint16 ident, void *data, uint16 data_len, void *options, uint16 opt_len);
IP_DGRAM *cdecl IP_fetch(int16 prtcl);
int16 cdecl IP_handler(int16 prtcl, int16 cdecl(*hndlr) (IP_DGRAM *), int16 flag);
void cdecl IP_discard(IP_DGRAM *datagram, int16 all_flag);

extern LAYER icmp_desc;

int16 cdecl ICMP_send(uint32 dest, uint8 type, uint8 code, const void *data, uint16 len);
int16 cdecl ICMP_handler(int16 cdecl(*hndlr) (IP_DGRAM *), int16 flag);
void cdecl ICMP_discard(IP_DGRAM *datagram);

long init_cookie(void);
void install(void);

uint16 check_sum(IP_HDR *header, void *options, int16 length) GNU_ASM_NAME("check_sum");
uint16 lock_exec(uint16 status) GNU_ASM_NAME("lock_exec");
int16 check_sequence(uint32 first, uint32 second, int32 *diff);

int16 KRinitialize(int32 size);
void *cdecl KRmalloc(int32 size);
void cdecl KRfree(void *mem_block);
int32 cdecl KRgetfree(int16 block_flag);
void *cdecl KRrealloc(void *mem_block, int32 new_size);


int16 handle_lookup(int16 connec, void **anonymous, CN_FUNCS **entry);
int16 cdecl PRTCL_announce(int16 protocol);
int16 cdecl PRTCL_get_parameters(uint32 rem_host, uint32 *src_ip, int16 *ttl, uint16 *mtu);
int16 cdecl PRTCL_request(void *anonymous, CN_FUNCS *cn_functions);
void cdecl PRTCL_release(int16 handle);
void *cdecl PRTCL_lookup(int16 handle, CN_FUNCS *cn_functions);

int16 cdecl TIMER_call(int16 cdecl (*handler)(IP_DGRAM *), int16 flag);
int32 cdecl TIMER_now(void);
int32 cdecl TIMER_elapsed(int32 then);
int16 reassembly(IP_DGRAM **datagram, int16 protocol);
int16 fragment(IP_DGRAM **datagram, uint16 mtu);

int16 cdecl ICMP_process(IP_DGRAM *dgram);
int16 ICMP_reply(uint8 type, uint8 code, IP_DGRAM *dgram, uint32 supple);


PORT *route_it(uint32 ip_destination, uint32 *gateway);
void init_ip(void);
void cdecl my_send(PORT *port);
void cdecl my_receive(PORT *port);

void init_ports(void);

#ifdef __GNUC__
#define _BasPag _base
#endif


/*
 * from thread.s
 */
extern short active GNU_ASM_NAME("active");
extern short fraction GNU_ASM_NAME("fraction");

void install_PrivVio(void) GNU_ASM_NAME("install_PrivVio");
void uninst_PrivVio(void) GNU_ASM_NAME("uninst_PrivVio");
void clean_up(void) GNU_ASM_NAME("clean_up");
void poll_ports(void) GNU_ASM_NAME("poll_ports");
void install_timer(void) GNU_ASM_NAME("install_timer");
