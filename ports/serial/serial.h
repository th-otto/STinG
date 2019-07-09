/*
 *      serial.h            (c) Peter Rottengatter  1996
 *                              perot@pallas.amp.uni-hannover.de
 *
 *      Included into the SERIAL.STX source code files
 */

#ifndef SERIAL_H
#define SERIAL_H



/*--------------------------------------------------------------------------*/


/*
 *   Non-blocking mode for opening device.
 */

/* do not use Pure-C definitions, we need MiNT/HSMODEM flags */
#undef O_NDELAY
#define  O_NDELAY      0x0100



/*--------------------------------------------------------------------------*/


/*
 *   MAPTAB function pointer types.
 */

#define  B_STAT     int cdecl (*) (int)
#define  B_IN      long cdecl (*) (int)
#define  B_OUT     void cdecl (*) (int, int)



/*--------------------------------------------------------------------------*/


/*
 *   RSVF entry describing a serial port.
 */

typedef struct rsvf_entry
{
	void *miscell;
	uint8 flags;
	uint8 resvd_a;
	uint8 bios;
	uint8 resvd_b;
} RSVF_DEV;


/*
 *   RSVF flags.
 */

#define  RSVF_DEVICE     0x80
#define  RSVF_FILE_IO    0x40
#define  RSVF_BIOS       0x20
#define  RSVF_MXDDEV     0x01



/*--------------------------------------------------------------------------*/


/*
 *   SLIP framing definitions
 */

#define  SLIP_END       0xc0
#define  SLIP_ESC       0xdb
#define  SLIP_DATEND    0xdc
#define  SLIP_DATESC    0xdd



/*--------------------------------------------------------------------------*/


/*
 *   PPP framing definitions
 */

#define  PPP_FLAG       0x7e
#define  PPP_ADDR       0xff
#define  PPP_CNTRL      0x03
#define  PPP_ESC        0x7d

#define  PPP_LCP        0xc021
#define  PPP_PAP        0xc023
#define  PPP_CHAP       0xc223
#define  PPP_IP         0x0021
#define  PPP_VJHC_C     0x002d
#define  PPP_VJHC_UNC   0x002f
#define  PPP_IPCP       0x8021



/*--------------------------------------------------------------------------*/


/*
 *   PPP state definitions
 */

#define  PPP_SNUM          10

#define  PPP_INITIAL        0
#define  PPP_STARTING       1
#define  PPP_CLOSED         2
#define  PPP_STOPPED        3
#define  PPP_CLOSING        4
#define  PPP_STOPPING       5
#define  PPP_REQ_SENT       6
#define  PPP_ACK_RECVD      7
#define  PPP_ACK_SENT       8
#define  PPP_OPENED         9


/*
 *   PPP event definitions
 */

#define  PPP_ENUM          14

#define  PPP_LAYUP          0
#define  PPP_LAYDOWN        1
#define  PPP_OPEN           2
#define  PPP_CLOSE          3
#define  PPP_TONF           4
#define  PPP_TOFATAL        5
#define  PPP_CREQOK         6
#define  PPP_CREQBAD        7
#define  PPP_CACK           8
#define  PPP_CNAK           9
#define  PPP_TREQ          10
#define  PPP_TACK          11
#define  PPP_REJOK         12
#define  PPP_REJFATAL      13


/*
 *   PPP CP code definitions
 */

#define  PPP_CONF_REQU      1
#define  PPP_CONF_ACK       2
#define  PPP_CONF_NAK       3
#define  PPP_CONF_REJCT     4
#define  PPP_TERM_REQU      5
#define  PPP_TERM_ACK       6
#define  PPP_CODE_REJCT     7
#define  PPP_PRTCL_REJCT    8
#define  PPP_ECHO_REQU      9
#define  PPP_ECHO_REPLY    10
#define  PPP_DISC_REQU     11


/*
 *   PPP option code definitions
 */

#define  OPT_MRU            1
#define  OPT_ACCM           2
#define  OPT_AUTH           3
#define  OPT_QUALITY        4
#define  OPT_MAGIC          5
#define  OPT_PRTCL_COMP     7
#define  OPT_A_C_COMP       8

#define  OPT_VJHC           2
#define  OPT_IP_ADDR        3
#define  OPT_PDNS_ADDR      129
#define  OPT_SDNS_ADDR      131


/*
 *   PPP miscellaneous definitions
 */

#define  PPP_UP             0
#define  PPP_DOWN           1



/*--------------------------------------------------------------------------*/


#define  MAX_STATES    16
#define  VJHC_TOSS     1


/*
 *   VJHC packet types.
 */

#define  VJHC_TYPE_ERROR          0x00
#define  VJHC_TYPE_IP             0x40
#define  VJHC_TYPE_UNCOMPR_TCP    0x70
#define  VJHC_TYPE_COMPR_TCP      0x80


/*
 *   What changed in  VJHC packet ?
 */

#define  NEW_C        0x40
#define  NEW_I        0x20
#define  TCP_PUSH     0x10
#define  NEW_S        0x08
#define  NEW_A        0x04
#define  NEW_W        0x02
#define  NEW_U        0x01

#define  SPECIAL_I       ( NEW_S | NEW_W | NEW_U )
#define  SPECIAL_D       ( NEW_S | NEW_A | NEW_W | NEW_U )
#define  SPECIAL_MASK    ( NEW_S | NEW_A | NEW_W | NEW_U )


/*
 *   TCP segment header.
 */

typedef struct tcp_header
{
	uint16 src_port;					/* Port number of sender                    */
	uint16 dest_port;					/* Port number of receiver                  */
	uint32 sequence;					/* Sequence number                          */
	uint32 acknowledge;					/* Acknowledgement number                   */
	unsigned offset:4;					/* Data offset (header length)              */
	unsigned resvd:6;					/* Reserved                                 */
	unsigned urgent:1;					/* Flag for urgent data present             */
	unsigned ack:1;						/* Flag for acknowledgement segment         */
	unsigned push:1;					/* Flag for push function                   */
	unsigned reset:1;					/* Flag for resetting connection            */
	unsigned sync:1;					/* Flag for synchronizing sequence numbers  */
	unsigned fin:1;						/* Flag for no more data from sender        */
	uint16 window;						/* Receive window                           */
	uint16 chksum;						/* Checksum of all header, options and data */
	uint16 urg_ptr;						/* First byte following urgent data         */
} TCP_HDR;


/*
 *   CSTATE for Van Jacobson compression
 */

typedef struct cstate
{
	struct cstate *next;
	uint8 connec_id;
	IP_HDR cs_ip;
	uint8 cs_ip_opt[44];
	TCP_HDR cs_tcp;
	uint8 cs_tcp_opt[44];
} CSTATE;


/*
 *   Structure containing all VJHC data for one port
 */
typedef struct vjhc
{
	struct cstate *last_cstate;
	uint8 last_recvd;
	uint8 last_send;
	uint16 flags;
	int16 max_states;
	CSTATE rec_state[MAX_STATES];
	CSTATE send_state[MAX_STATES];
	uint16 begin;
	uint16 length;
	uint8 header[16];
} VJHC;



/*--------------------------------------------------------------------------*/

typedef struct serial_port SERIAL_PORT;


/*
 *   PPP state machine description.
 */
typedef struct machine
{
	char name[6];
	int16 state;
	uint16 protocol;
	uint32 timer_start;
	uint32 timer_elapsed;
	int16 timer_run;
	int16 restart_cnt;
	int16 block_len;
	int16 conf_len;
	int16 flags;
	int16 identi;
	int16 codes;
	int16 essent;
	int16 event;
	uint8 offset;
	uint8 xtra;
	uint8 *block;
	uint8 *conf;
	void (*up_down)(SERIAL_PORT *port, int16 flag);
	void (*do_lower)(SERIAL_PORT *port, int16 flag);
	void (*create)(SERIAL_PORT *port);
	int16 (*negotiate) (SERIAL_PORT *port, uint8 *own, uint8 *src, uint8 *mod);
	int16 (*implement) (SERIAL_PORT *port, uint8 *option);
	int16 (*process) (SERIAL_PORT *port, uint8 *src, uint8 *mod);
	void (*accept) (SERIAL_PORT *port, uint8 *option);
} MACHINE;


/*
 *   Structure for all PPP relevant information.
 */

typedef struct ppp_data
{
	int16 peer_mru;
	uint32 recve_accm;
	uint32 send_accm;
	uint32 local_magic;
	uint32 remote_magic;
	uint32 offered;
	char pap_id[128];
	char pap_passwd[128];
	char **pap_auth;
#if 0
	char *pap_ack;
	char *pap_nak;
#endif
	uint32 p_dns;
	uint32 s_dns;
	uint8 *cp_send_data;
	uint8 *data;
	int16 cp_send_len;
	int16 length;
	int16 mtu2;
	int16 vjhc_max1;
	int16 vjhc_max2;
	char *message;
	MACHINE lcp;
	MACHINE ipcp;
	MACHINE pap;
} PPP_DATA;



/*--------------------------------------------------------------------------*/


/*
 *   STX internal structure for all port relevant information.
 */

struct serial_port
{
	PORT generic;
	int16 bios_addr;
	const char *gemdos;
	int16 handle;
	MAPTAB *handler;
	void *iocntl;
	VJHC *vjhc;
	PPP_DATA ppp;
	char *log_buffer;
	uint16 log_len;
	uint16 log_ptr;
	uint8 *send_buffer;
	int16 send_length;
	int16 send_index;
	uint8 *recve_buffer;
	int16 recve_length;
	int16 recve_index;
	int16 is_magx;
};



/*--------------------------------------------------------------------------*/


/*
 *   PPP state machine transition.
 */

typedef struct transition
{
	void (*first)(SERIAL_PORT *port, MACHINE *which, int16 event);
	void (*second)(SERIAL_PORT *port, MACHINE *which, int16 event);
	void (*third)(SERIAL_PORT *port, MACHINE *which, int16 event);
	int16 new_state;
} TRANSITION;



/*--------------------------------------------------------------------------*/


/*
 *   Mask bits for generic.flags
 */

#define  FLG_PRTCL           0x00000001L
#define  FLG_VJHC            0x00000002L
#define  FLG_LANBIT          0x00000004L
#define  FLG_DNS             0x00000008L

#define  FLG_ALLOW_PAP       0x00001000L
#define  FLG_REQU_AUTH       0x00002000L
#define  FLG_LOGGING         0x00010000L
#define  FLG_DONE            0x00020000L
#define  FLG_SUCCESS         0x00040000L
#define  FLG_FINISH          0x00100000L
#define  FLG_UP_DOWN         0x00200000L
#define  FLG_PRTCL_COMP      0x01000000L
#define  FLG_A_C_COMP        0x02000000L
#define  FLG_DCD_UP          0x10000000L
#define  FLG_DCD_DOWN        0x20000000L
#define  FLG_DTR_UP          0x40000000L
#define  FLG_DTR_DOWN        0x80000000L



/*--------------------------------------------------------------------------*/

#ifndef GNU_ASM_NAME
#ifdef __GNUC__
#define GNU_ASM_NAME(x) __asm__(x)
#else
#define GNU_ASM_NAME(x)
#endif
#endif

#ifndef UNUSED
# define UNUSED(x) ((void)(x))
#endif

#ifdef __GNUC__
#define _BasPag _base
#endif


extern uint8 identifier;
extern SERIAL_PORT *my_ports;
extern int space;
extern DRIVER my_driver;


void make_IP_dgram(uint8 *buffer, int16 buff_len, IP_DGRAM ** dgram);

void init_vjhc(VJHC *vjhc, int16 num_states);
int vjhc_compress(IP_DGRAM *datagram, VJHC *vjhc);
int vjhc_uncompress(uint8 *buff, int16 len, int16 type, VJHC *vjhc, IP_DGRAM ** dgram);

int16 send_cp(SERIAL_PORT *port, uint16 protocol, uint8 code, uint8 ident, uint16 length, uint8 *options);
void ppp_log_text(SERIAL_PORT *port, char *text);
void ppp_log_it(SERIAL_PORT *port, MACHINE *machine, const char *action, const char *which, int ident);
void ppp_log_options(SERIAL_PORT *port, uint8 *data, int16 len, uint8 offs, uint8 xtra);
void number_to_string(uint32 number, char *string, int16 digits);

void user_event(SERIAL_PORT *port, int event, MACHINE *machine);
void timer_event(SERIAL_PORT *port, MACHINE *machine);
void network_event(SERIAL_PORT *port, MACHINE *machine);

void ppp_dummy(SERIAL_PORT *port, int16 flag);

void lcp_up_down(SERIAL_PORT *port, int16 flag);
void lcp_do_lower(SERIAL_PORT *port, int16 flag);
void lcp_create(SERIAL_PORT *port);
int16 lcp_nego(SERIAL_PORT *port, uint8 *own, uint8 *source, uint8 *modified);
int16 lcp_imple(SERIAL_PORT *port, uint8 *option);
int16 lcp_options(SERIAL_PORT *port, uint8 *source, uint8 *modified);
void lcp_accept(SERIAL_PORT *port, uint8 *option);

void ipcp_up_down(SERIAL_PORT *port, int16 flag);
void ipcp_create(SERIAL_PORT *port);
int16 ipcp_nego(SERIAL_PORT *port, uint8 *own, uint8 *source, uint8 *modified);
int16 ipcp_imple(SERIAL_PORT *port, uint8 *option);
int16 ipcp_options(SERIAL_PORT *port, uint8 *source, uint8 *modified);
void ipcp_accept(SERIAL_PORT *port, uint8 *option);

void pap_up_down(SERIAL_PORT *port, int16 flag);
void pap_create(SERIAL_PORT *port);
int16 pap_nego(SERIAL_PORT *port, uint8 *own, uint8 *source, uint8 *modified);
int16 pap_imple(SERIAL_PORT *port, uint8 *option);
int16 pap_options(SERIAL_PORT *port, uint8 *source, uint8 *modified);
void pap_accept(SERIAL_PORT *port, uint8 *option);

void init_ppp(SERIAL_PORT *port);
int16 cdecl ppp_timer(IP_DGRAM *);
int16 open_ppp(SERIAL_PORT *port);
void close_ppp(SERIAL_PORT *port);
void open_ipcp(SERIAL_PORT *port);
void close_ipcp(SERIAL_PORT *port);
void ppp_control(SERIAL_PORT *port, uint16 protocol, uint8 *data, int16 len);

uint16 fetch_16bit(uint8 *dest, const uint8 *source);
uint32 fetch_32bit(uint8 *dest, const uint8 *source);
uint32 choose_a_magic(uint32 avoid);
int16 add_ps_dns(char *buffer, uint32 dns_ip);
int16 pap_check_auth(SERIAL_PORT *port, const char *secret);

void cdecl my_send(PORT *port);
void cdecl my_receive(PORT *port);


void set_dtr(void *hsm_code, int new_dtr) GNU_ASM_NAME("set_dtr");
int inq_cd(void *hsm_code) GNU_ASM_NAME("inq_cd");
long choose_magic(void) GNU_ASM_NAME("choose_magic");
int execute(short cdecl (*code)(short dev)) GNU_ASM_NAME("execute");

long bconmap_bcostat(MAPTAB *handler) GNU_ASM_NAME("bconmap_bcostat");
long bconmap_bconstat(MAPTAB *handler) GNU_ASM_NAME("bconmap_bconstat");
long bconmap_read(MAPTAB *handler, long count, uint8 *buffer) GNU_ASM_NAME("bconmap_read");
long bconmap_write(MAPTAB *handler, long count, const uint8 *buffer) GNU_ASM_NAME("bconmap_write");

#endif /* SERIAL_H */
