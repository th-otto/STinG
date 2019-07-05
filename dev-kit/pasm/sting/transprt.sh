;----------------------------------------------------------------------------
; File name:	TRANSPRT.SH			Revision date:	2000.06.14
; Authors:	P.Rottengatter & R.Andersson	Creation date:	1997.03.26
;----------------------------------------------------------------------------
; Purpose:	High level StinG client support
;		Header file included in assembly
;----------------------------------------------------------------------------
;Required header declarations:
;
;	.include	"uran\struct.sh"
;	.include	"sting\transprt.sh"
;
;copy the above to the header of your program and 'uncomment' the includes
;----------------------------------------------------------------------------
;
;TRANSPORT_DRIVER	"TRANSPORT_TCPIP"
;TCP_DRIVER_VERSION	"01.00"
;
;----------------------------------------------------------------------------
;		Driver access structure / functions
;
;MAGIC	"STiKmagic"			; Magic for DRV_LIST.magic
;CJTAG	"STiK"
;-------------------------------------
	struct	DRV_HDR			; Header part of TPL structure
	char_p	DRV_HDR_module		; Specific string that can be searched for
	char_p	DRV_HDR_author		; Any string
	char_p	DRV_HDR_version		; Format `00.00' Version:Revision
	d_end	DRV_HDR
;-------------------------------------
	struct	DRV_LIST
	char	DRV_LIST_magic,10	; Magic string, def'd as "STiKmagic"
	func_p	DRV_LIST_get_dftab	;DRV_HDR * cdecl (*get_dftab) (char *);
	func_p	DRV_LIST_ETM_exec	;int16	cdecl (*ETM_exec) (char *);	; Execute a STinG module
	void_p	DRV_LIST_cfg		; Config structure
	void_p	DRV_LIST_sting_basepage	; STinG basepage address
	d_end	DRV_LIST
;-------------------------------------
	.import	drivers			;extern DRV_LIST *drivers;
;----------------------------------------------------------------------------
;The access macros below assume that the STiK cookie value has been placed
;in a pointer named 'drivers' before they are called.  Doing so is up to
;the user program, and failure to do so inevitably causes bombs...
;----------------------------------------------------------------------------
.MACRO	get_dftab	ch_p
	move.l	ch_p,-(sp)
	move.l	drivers,a0
	move.l	DRV_LIST_get_dftab(a0),a0
	jsr	(a0)
	addq	#4,sp
.ENDM	get_dftab
;-------------------------------------
.MACRO	ETM_exec
	move.l	ch_p,-(sp)
	move.l	drivers,a0
	move.l	DRV_LIST_ETM_exec(a0),a0
	jsr	(a0)
	addq	#4,sp
.ENDM	ETM_exec
;----------------------------------------------------------------------------
;	TCP and UDP port escape flags
;
TCP_ACTIVE	equ	$0000	; Initiate active connection         */
TCP_PASSIVE	equ	$ffff	; Initiate passive connection        */
UDP_EXTEND	equ	$0000	; Extended addressing scheme         */
;----------------------------------------------------------------------------
;	TCP miscellaneous flags.
;
TCP_URGENT	equ	-1	; ((void *) -1)  Mark urgent position
TCP_HALFDUPLEX	equ	-1	; TCP_close() half duplex
TCP_IMMEDIATE	equ	0	; TCP_close() immediate
;----------------------------------------------------------------------------
;		TCP connection states
;-------------------------------------
TCLOSED		equ	0	; No connection.	Null, void, absent, ...
TLISTEN		equ	1	; Wait for remote request
TSYN_SENT	equ	2	; Connect request sent, await matching request
TSYN_RECV	equ	3	; Wait for connection ack
TESTABLISH	equ	4	; Connection established, handshake completed
TFIN_WAIT1	equ	5	; Await termination request or ack
TFIN_WAIT2	equ	6	; Await termination request
TCLOSE_WAIT	equ	7	; Await termination request from local user
TCLOSING	equ	8	; Await termination ack from remote TCP
TLAST_ACK	equ	9	; Await ack of terminate request sent
TTIME_WAIT	equ	10	; Delay, ensures remote has received term' ack
;----------------------------------------------------------------------------
;		UDP connection pseudo states  (for UDP_info)
;-------------------------------------
UCLOSED		equ	0	; No connection.	Null, void, absent, ...
ULISTEN		equ	1	; Wait for remote request
UESTABLISH	equ	4	; Connection established, handshake completed
;----------------------------------------------------------------------------
;		TCP information block
;-------------------------------------
struct	TCPIB
	uint32	TCPIB_request	;32 bit flags requesting various info
	uint16	TCPIB_state	;current TCP state
	uint32	TCPIB_unacked	;unacked outgoing sequence length (incl SYN/FIN)
	uint32	TCPIB_srtt	;smoothed round trip time of this connection
d_end	TCPIB
;-------------------------------------
TCPI_state	equ	1	;request current TCP state
TCPI_unacked	equ	2	;request length of unacked sequence
TCPI_srtt	equ	4	;request smoothed round trip time
TCPI_defer	equ	8	;request switch to DEFER mode
;-------------------------------------
TCPI_bits	equ	4
TCPI_mask	equ	(1<<TCPI_bits)-1
;----------------------------------------------------------------------------
;NB: A TCP_info request using undefined bits will result in E_PARAMETER.
;    else the return value will be TCPI_bits, so user knows what we have.
;    Future additions will use rising bits in sequence, and additions to
;    the TCPIB struct will always be made at its previous end.
;----------------------------------------------------------------------------
;!!! By TCP_info with TCPI_defer, connection is switched to 'DEFER' mode.
;    This means that all situations where internal looping would occur
;    will instead lead to exit to the caller with return value E_LOCKED.
;    Using this mode constitutes agreement to always check for that error
;    code, which is mainly used for connections using DEFER mode. It may
;    also be used in some other instances, where a function is blocked in
;    such a way that internal looping is not possible.
;----------------------------------------------------------------------------
;		UDP information block
;-------------------------------------
struct	UDPIB
	uint32	UDPIB_request	;32 bit flags requesting various info
	uint16	UDPIB_state	;current UDP state
	uint32	UDPIB_unacked	;unacked outgoing sequence length (incl SYN/FIN)
	uint32	UDPIB_srtt	;smoothed round trip time of this connection
d_end	UDPIB
;-------------------------------------
UDPI_state	equ	1	;request current UDP state
UDPI_unacked	equ	2	;request length of unacked sequence
UDPI_srtt	equ	4	;request smoothed round trip time
UDPI_defer	equ	8	;request switch to DEFER mode
;-------------------------------------
UDPI_bits	equ	4
UDPI_mask	equ	(1<<UDPI_bits)-1
;----------------------------------------------------------------------------
;NB: A UDP_info request using undefined bits will result in E_PARAMETER.
;    else the return value will be UDPI_bits, so user knows what we have.
;    Future additions will use rising bits in sequence, and additions to
;    the UDPIB struct will always be made at its previous end.
;----------------------------------------------------------------------------
;!!! By UDP_info with UDPI_defer, connection is switched to 'DEFER' mode.
;    This means that all situations where internal looping would occur
;    will instead lead to exit to the caller with return value E_LOCKED.
;    Using this mode constitutes agreement to always check for that error
;    code, which is mainly used for connections using DEFER mode. It may
;    also be used in some other instances, where a function is blocked in
;    such a way that internal looping is not possible.
;----------------------------------------------------------------------------
;		Port Name Transfer Area
;
struct	PNTA			;Buffer structure for inquiring port names
	uint32	PNTA_opaque	;Kernel internal data
	int16	PNTA_name_len	;Length of name buffer
	char_p	PNTA_port_name	;Address of name buffer
d_end	PNTA
;----------------------------------------------------------------------------
;		Command opcodes for cntrl_port().
;
CTL_KERN_FIRST_PORT	= ('K'<<8|'F')	;Kernel
CTL_KERN_NEXT_PORT	= ('K'<<8|'N')	;Kernel
CTL_KERN_FIND_PORT	= ('K'<<8|'G')	;Kernel
;------------------------------------
CTL_GENERIC_SET_IP	= ('G'<<8|'H')	;Kernel, all ports
CTL_GENERIC_GET_IP	= ('G'<<8|'I')	;Kernel, all ports
CTL_GENERIC_SET_MASK	= ('G'<<8|'L')	;Kernel, all ports
CTL_GENERIC_GET_MASK	= ('G'<<8|'M')	;Kernel, all ports
CTL_GENERIC_SET_MTU	= ('G'<<8|'N')	;Kernel, all ports
CTL_GENERIC_GET_MTU	= ('G'<<8|'O')	;Kernel, all ports
CTL_GENERIC_GET_MMTU	= ('G'<<8|'P')	;Kernel, all ports
CTL_GENERIC_GET_TYPE	= ('G'<<8|'T')	;Kernel, all ports
CTL_GENERIC_GET_STAT	= ('G'<<8|'S')	;Kernel, all ports
CTL_GENERIC_CLR_STAT	= ('G'<<8|'C')	;Kernel, all ports
;------------------------------------
CTL_SERIAL_SET_PRTCL	= ('S'<<8|'P')	;Serial Driver
CTL_SERIAL_GET_PRTCL	= ('S'<<8|'Q')	;Serial Driver
CTL_SERIAL_SET_LOGBUFF	= ('S'<<8|'L')	;Serial Driver
CTL_SERIAL_SET_LOGGING	= ('S'<<8|'F')	;SerialDriver
CTL_SERIAL_SET_AUTH	= ('S'<<8|'A')	;Serial Driver
CTL_SERIAL_SET_PAP	= ('S'<<8|'B')	;Serial Driver
CTL_SERIAL_INQ_STATE	= ('S'<<8|'S')	;Serial Driver
;------------------------------------
CTL_ETHER_SET_MAC	= ('E'<<8|'M')	;EtherNet
CTL_ETHER_GET_MAC	= ('E'<<8|'N')	;EtherNet
CTL_ETHER_INQ_SUPPTYPE	= ('E'<<8|'Q')	;EtherNet
CTL_ETHER_SET_TYPE	= ('E'<<8|'T')	;EtherNet
CTL_ETHER_GET_TYPE	= ('E'<<8|'U')	;EtherNet
;------------------------------------
CTL_MASQUE_SET_PORT	= ('M'<<8|'P')	;Masquerade
CTL_MASQUE_GET_PORT	= ('M'<<8|'Q')	;Masquerade
CTL_MASQUE_SET_MASKIP	= ('M'<<8|'M')	;Masquerade
CTL_MASQUE_GET_MASKIP	= ('M'<<8|'N')	;Masquerade
CTL_MASQUE_GET_REALIP	= ('M'<<8|'R')	;Masquerade
;----------------------------------------------------------------------------
;		Handler flag values.
;
HNDLR_SET	equ	0		; Set new handler if space		*/
HNDLR_FORCE	equ	1		; Force new handler to be set		*/
HNDLR_REMOVE	equ	2		; Remove handler entry			*/
HNDLR_QUERY	equ	3		; Inquire about handler entry		*/
;----------------------------------------------------------------------------
;		IP packet header.
;
struct	IPHD
	d_field	IPHD_verlen_f,8			;bitfield
	d_bits	IPHD_f_version,	4	; IP Version
	d_bits	IPHD_f_hd_len,	4	; Internet Header Length/4
	uint8	IPHD_tos		; Type of Service
	uint16	IPHD_length		; Total of all header, options and data
	uint16	IPHD_ident		; Identification for fragmentation
	d_field	IPHD_frag_f,16			;bitfield
	d_bits	IPHD_f_reserved,1	; Reserved : Must be zero
	d_bits	IPHD_f_dont_frg,1	; Don't fragment flag
	d_bits	IPHD_f_more_frg,1	; More fragments flag
	d_bits	IPHD_f_frag_ofst,13	; Fragment offset
	uint8	IPHD_ttl		; Time to live
	uint8	IPHD_protocol		; Protocol
	uint16	IPHD_hdr_chksum		; Header checksum
	uint32	IPHD_ip_src		; Source IP address
	uint32	IPHD_ip_dest		; Destination IP address
d_end	IPHD
;----------------------------------------------------------------------------
;		Internal IP packet representation.
;
struct	IPDG
	char		IPDG_hdr,sizeof_IPHD	; Header of IP packet
	void_p		IPDG_options		; Options data block
	int16		IPDG_opt_length		; Length of options data block
	void_p		IPDG_pkt_data		; IP packet data block
	int16		IPDG_pkt_length		; Length of IP packet data block
	uint32		IPDG_timeout		; Timeout of packet life
	uint32		IPDG_ip_gateway		; Gateway for forwarding this packet
	void_p		IPDG_recvd		; Receiving port
	struct_p	IPDG_next		; Next IP packet in IP packet queue
d_end	IPDG
;----------------------------------------------------------------------------
;	Values for protocol field
;
P_ICMP	equ	1	; IP assigned number for ICMP
P_TCP	equ	6	; IP assigned number for TCP
P_UDP	equ	17	; IP assigned number for UDP
;----------------------------------------------------------------------------
;		Input queue structures
;
struct	NDB			; Network Data Block.	For data delivery
	char_p		NDB_ptr		; Pointer to base of block. (For KRfree)
	char_p		NDB_ndata	; Pointer to next data to deliver
	uint16		NDB_len		; Length of remaining data
	struct_p	NDB_next	; Next NDB in chain or NULL
d_end	NDB
;----------------------------------------------------------------------------
;		Connection address block
;
struct	CAB
	uint16	CAB_lport	; TCP local  port     (ie: local machine)
	uint16	CAB_rport	; TCP remote port     (ie: remote machine)
	uint32	CAB_rhost	; TCP remote IP addr  (ie: remote machine)
	uint32	CAB_lhost	; TCP local  IP addr  (ie: local machine)
d_end	CAB
;----------------------------------------------------------------------------
;		Connection information block
;
struct	CIB		; Connection Information Block
	uint16	CIB_protocol	; TCP or UDP or ... 0 means CIB is not in use
	d_alias	CIB_address	; CAB structure (see above)
	uint16	CIB_lport	; TCP local	port	(ie: local machine)
	uint16	CIB_rport	; TCP remote port	(ie: remote machine)
	uint32	CIB_rhost	; TCP remote IP addr	(ie: remote machine)
	uint32	CIB_lhost	; TCP local	IP addr	(ie: local machine)
	uint16	CIB_status	; Net status. 0 means normal
d_end	CIB
;
;NB: the d_alias above maintains compatibility between old and new methods
;----------------------------------------------------------------------------
;		Transport structure / functions
;
struct TPL
;-------
	char_p	TPL_module		; Specific string that can be searched for
	char_p	TPL_author		; Any string
	char_p	TPL_version		; Format `00.00' Version:Revision
;-------
	func_p	TPL_KRmalloc		;void *	cdecl	(* KRmalloc) (int32);
	func_p	TPL_KRfree		;void	cdecl	(* KRfree) (void *);
	func_p	TPL_KRgetfree		;int32	cdecl	(* KRgetfree) (int16);
	func_p	TPL_KRrealloc		;void *	cdecl	(* KRrealloc) (void *, int32);
;-------
	func_p	TPL_get_err_text	;char *	cdecl	(* get_err_text) (int16);
	func_p	TPL_getvstr		;char *	cdecl	(* getvstr) (char *);
;-------
	func_p	TPL_carrier_detect	;int16	cdecl	(* carrier_detect) (void);
;-------
	func_p	TPL_TCP_open		;int16	cdecl	(* TCP_open) (uint32, int16, int16, uint16);
	func_p	TPL_TCP_close		;int16	cdecl	(* TCP_close) (int16, int16, int16 *);
	func_p	TPL_TCP_send		;int16	cdecl	(* TCP_send) (int16, void *, int16);
	func_p	TPL_TCP_wait_state	;int16	cdecl	(* TCP_wait_state) (int16, int16, int16);
	func_p	TPL_TCP_ack_wait	;int16	cdecl	(* TCP_ack_wait) (int16, int16);
;-------
	func_p	TPL_UDP_open		;int16	cdecl	(* UDP_open) (uint32, int16);
	func_p	TPL_UDP_close		;int16	cdecl	(* UDP_close) (int16);
	func_p	TPL_UDP_send		;int16	cdecl	(* UDP_send) (int16, void *, int16);
;-------
	func_p	TPL_CNkick		;int16	cdecl	(* CNkick) (int16);
	func_p	TPL_CNbyte_count	;int16	cdecl	(* CNbyte_count) (int16);
	func_p	TPL_CNget_char		;int16	cdecl	(* CNget_char) (int16);
	func_p	TPL_CNget_NDB		;NDB *	cdecl	(* CNget_NDB) (int16);
	func_p	TPL_CNget_block		;int16	cdecl	(* CNget_block) (int16, void *, int16);
;-------
	func_p	TPL_housekeep		;void	cdecl	(* housekeep) (void);
	func_p	TPL_resolve		;int16	cdecl	(* resolve) (char *, char **, uint32 *, int16);
	func_p	TPL_ser_disable		;void	cdecl	(* ser_disable) (void);
	func_p	TPL_ser_enable		;void	cdecl	(* ser_enable) (void);
	func_p	TPL_set_flag		;int16	cdecl	(* set_flag) (int16);
	func_p	TPL_clear_flag		;void	cdecl	(* clear_flag) (int16);
	func_p	TPL_CNgetinfo		;CIB *	cdecl	(* CNgetinfo) (int16);
;-------
	func_p	TPL_on_port		;int16	cdecl	(* on_port) (char *);
	func_p	TPL_off_port		;void	cdecl	(* off_port) (char *);
	func_p	TPL_setvstr		;int16	cdecl	(* setvstr) (char *, char *);
	func_p	TPL_query_port		;int16	cdecl	(* query_port) (char *);
	func_p	TPL_CNgets		;int16	cdecl	(* CNgets) (int16, char *, int16, char);
;------- STinG ICMP functions ----------
	func_p	TPL_ICMP_send		;int16	cdecl	(* ICMP_send) (uint32, uint8, uint8, void *, uint16);
	func_p	TPL_ICMP_handler	;int16	cdecl	(* ICMP_handler) (int16 cdecl (*) (IP_DGRAM *), int16);
	func_p	TPL_ICMP_discard	;void	cdecl	(* ICMP_discard) (IP_DGRAM *);
;------- STinG extensions mid-1998 -----
	func_p	TPL_TCP_info		;int16	cdecl	(* TCP_info) (int16, TCPIB *);
	func_p	TPL_cntrl_port		;int16	cdecl	(* cntrl_port) (char *, uint32, int16);
;------- STinG extension 1999.10.01 ----
	func_p	TPL_UDP_info		;int16	cdecl	(* UDP_info) (int16, UDPIB *);
;------- STinG extension 2000.06.14 ----    /* STiK2 compatibility funcs */
	func_p	RAW_open		;int16	cdecl	(* RAW_open)(uint32);
	func_p	RAW_close		;int16	cdecl	(* RAW_close)(int16);
	func_p	RAW_out			;int16	cdecl	(* RAW_out)(int16, void *, int16, uint32);
	func_p	CN_setopt		;int16 	cdecl	(* CN_setopt)(int16, int16, const void *, int16);
	func_p	CN_getopt		;int16 	cdecl	(* CN_getopt)(int16, int16, void *, int16 *);
	func_p	CNfree_NDB		;void	cdecl	(* CNfree_NDB)(int16, NDB *);
d_end	TPL
;----------------------------------------------------------------------------
	.import	tpl	;extern TPL *tpl;
;----------------------------------------------------------------------------
;	Definitions of transport function macros for direct use
;
;	The user program must define the pointer 'tpl' and initialize
;	its value via the STiK cookie before using any TPL functions.
;----------------------------------------------------------------------------
;	Submacros are used to make the interface flexible and powerful
;
;sub_tpl.mode	is used to implement all TPL functions, and uses submacro
;		sub_sub_tpl as well as some of URAn_SYS.SH.  It gives argument
;		count check for all macros, and allows indirection of pointers.
;----------------------------------------------------------------------------
.MACRO	sub_tpl.mode	function,arg_count,arg_flags,arg1,arg2,arg3,arg4,arg5
	CDECL_args.mode	arg_flags,arg1,arg2,arg3,arg4,arg5
	sub_sub_tpl	TPL_&function
	CDECL_cleanargs	function,arg_count
.ENDM	sub_tpl
;-------------------------------------
.MACRO	sub_sub_tpl	tpl_function
	move.l	tpl,a0
	move.l	tpl_function(a0),a0
	jsr	(a0)
.ENDM	sub_sub_tpl
;----------------------------------------------------------------------------
.MACRO	KRmalloc	size
	sub_tpl	KRmalloc,1,2,size
.ENDM	KRmalloc
;------------------------------------
.MACRO	KRfree.mode	block_p
	sub_tpl.mode	KRfree,1,3,block_p
.ENDM	KRfree
;------------------------------------
.MACRO	KRgetfree	mode_f
	sub_tpl	KRgetfree,1,1,mode_f
.ENDM	KRgetfree
;------------------------------------
.MACRO	KRrealloc.mode	block_p,size
	sub_tpl.mode	KRrealloc,2,$B,block_p,size
.ENDM	KRrealloc
;------------------------------------
.MACRO	get_err_text	err_code
	sub_tpl	get_err_text,1,1,err_code
.ENDM	get_err_text
;------------------------------------
.MACRO	getvstr.mode	varname_p
	sub_tpl.mode	getvstr,1,3,varname_p
.ENDM	getvstr
;------------------------------------
.MACRO	carrier_detect
	sub_tpl	carrier_detect,0,0
.ENDM	carrier_detect
;------------------------------------
.MACRO	TCP_open	remote_IP,protoport,tos,obuff_size
	sub_tpl	TCP_open,4,$56,remote_IP,protoport,tos,obuff_size
.ENDM	TCP_open
;------------------------------------
.MACRO	TCP_close	cn_handle,timeout_sec_mode,result_p
	sub_tpl	TCP_close,3,$35,cn_handle,timeout_sec_mode,result_p
.ENDM	TCP_close
;------------------------------------
.MACRO	TCP_send.mode	cn_handle,data_p,datalen
	sub_tpl.mode	TCP_send,3,$1D,cn_handle,data_p,datalen
.ENDM	TCP_send
;------------------------------------
.MACRO	TCP_wait_state	cn_handle,state,timeout_sec
	sub_tpl	TCP_wait_state,3,$15,cn_handle,state,timeout_sec
.ENDM	TCP_wait_state
;------------------------------------
.MACRO	TCP_ack_wait	cn_handle,timeout_ms
	sub_tpl	TCP_ack_wait,2,$5,cn_handle,timeout_ms
.ENDM	TCP_ack_wait
;------------------------------------
.MACRO	UDP_open	remote_IP,remote_port
	sub_tpl	UDP_open,2,$6,remote_IP,remote_port
.ENDM	UDP_open
;------------------------------------
.MACRO	UDP_close	cn_handle
	sub_tpl	UDP_close,1,1,cn_handle
.ENDM	UDP_close
;------------------------------------
.MACRO	UDP_send.mode	cn_handle,data_p,datalen
	sub_tpl.mode	UDP_send,3,$1D,cn_handle,data_p,datalen
.ENDM	UDP_send
;------------------------------------
.MACRO	CNkick	cn_handle
	sub_tpl	CNkick,1,1,cn_handle
.ENDM	CNkick
;------------------------------------
.MACRO	CNbyte_count	cn_handle
	sub_tpl	CNbyte_count,1,1,cn_handle
.ENDM	CNbyte_count
;------------------------------------
.MACRO	CNget_char	cn_handle
	sub_tpl	CNget_char,1,1,cn_handle
.ENDM	CNget_char
;------------------------------------
.MACRO	CNget_NDB	cn_handle
	sub_tpl	CNget_NDB,1,1,cn_handle
.ENDM	CNget_NDB
;------------------------------------
.MACRO	CNget_block.mode	cn_handle,buff_p,bufflen
	sub_tpl.mode	CNget_block,3,$1D,cn_handle,buff_p,bufflen
.ENDM	CNget_block
;------------------------------------
.MACRO	CNgetinfo	cn_handle
	sub_tpl	CNgetinfo,1,1,cn_handle
.ENDM	CNgetinfo
;------------------------------------
.MACRO	CNgets.mode	cn_handle,buff_p,bufflen,delim_ch
	sub_tpl.mode	CNgets,4,$1D,cn_handle,buff_p,bufflen,delim_ch
.ENDM	CNgets
;------------------------------------
.MACRO	housekeep
	sub_tpl	housekeep,0,0
.ENDM	housekeep
;------------------------------------
.MACRO	resolve.mode	search_p,real_p_p,IP_list_p,list_len
	sub_tpl.mode	resolve,4,$7F,search_p,real_p_p,IP_list_p,list_len
.ENDM	resolve
;------------------------------------
.MACRO	ser_disable
	sub_tpl	ser_disable,0,0
.ENDM	ser_disable
;------------------------------------
.MACRO	ser_enable
	sub_tpl	ser_enable,0,0
.ENDM	ser_enable
;------------------------------------
.MACRO	set_flag	flag_index
	sub_tpl	set_flag,1,1,flag_index
.ENDM	set_flag
;------------------------------------
.MACRO	clear_flag	flag_index
	sub_tpl	clear_flag,1,1,flag_index
.ENDM	clear_flag
;------------------------------------
.MACRO	on_port.mode	char_p
	sub_tpl.mode	on_port,1,3,char_p
.ENDM	on_port
;------------------------------------
.MACRO	off_port.mode	char_p
	sub_tpl.mode	off_port,1,3,char_p
.ENDM	off_port
;------------------------------------
.MACRO	setvstr.mode	varname_p,valuestring_p
	sub_tpl.mode	setvstr,2,$F,varname_p,valuestring_p
.ENDM	setvstr
;------------------------------------
.MACRO	query_port.mode	char_p
	sub_tpl.mode	query_port,1,3,char_p
.ENDM	query_port
;------------------------------------
.MACRO	ICMP_send.mode	remote_IP,type,code,data_p,datalen
	sub_tpl.mode	ICMP_send,5,$182,remote_IP,type,code,data_p,datalen
.ENDM	ICMP_send
;------------------------------------
.MACRO	ICMP_handler.mode	handler_p,install_code
	sub_tpl.mode	ICMP_handler,2,7,handler_p,install_code
.ENDM	ICMP_handler
;------------------------------------
.MACRO	ICMP_discard.mode	dgram_p
	sub_tpl.mode	ICMP_discard,1,3,dgram_p
.ENDM	ICMP_discard
;------------------------------------
.MACRO	TCP_info.mode	cn_handle,TCPIB_p
	sub_tpl.mode	TCP_info,2,$D,cn_handle,TCPIB_p
.ENDM	TCP_info
;------------------------------------
.MACRO	cntrl_port.mode	port_p,long_arg,opcode
	sub_tpl.mode	cntrl_port,3,$1B,port_p,long_arg,opcode
.ENDM	cntrl_port
;------------------------------------
.MACRO	UDP_info.mode	cn_handle,UDPIB_p
	sub_tpl.mode	UDP_info,2,$D,cn_handle,UDPIB_p
.ENDM	UDP_info
;------------------------------------
.MACRO	RAW_open	rhost
	sub_tpl	RAW_open,1,$2,rhost
.ENDM	RAW_open
;------------------------------------
.MACRO	RAW_close	cn_handle
	sub_tpl	RAW_close,1,$1,cn_handle
.ENDM	RAW_close
;------------------------------------
.MACRO	RAW_out.mode	cn_handle,dptr,dlen,dest
	sub_tpl.mode	RAW_out,4,$9D,cn_handle,dptr,dlen,dest
.ENDM	RAW_out
;------------------------------------
.MACRO	CN_setopt.mode	cn_handle,opt_id,optval_p,optlen
	sub_tpl.mode	CN_setopt,4,$75,cn_handle,opt_id,optval_p,optlen
.ENDM	CN_setopt
;------------------------------------
.MACRO	CN_getopt.mode	cn_handle,opt_id,optval_p,optlen_p
	sub_tpl.mode	CN_getopt,4,$F5,cn_handle,opt_id,optval_p,optlen_p
.ENDM	CN_getopt
;------------------------------------
.MACRO	CNfree_NDB.mode	cn_handle,NDB_p
	sub_tpl.mode	CNfree_NDB,2,$D,cn_handle,NDB_p
.ENDM	CNfree_NDB
;----------------------------------------------------------------------------
;		Error return values
;
E_NORMAL	equ	0	; No error occured ...
E_OBUFFULL	equ	-1	; Output buffer is full
E_NODATA	equ	-2	; No data available
E_EOF		equ	-3	; EOF from remote
E_RRESET	equ	-4	; Reset received from remote
E_UA		equ	-5	; Unacceptable packet received, reset
E_NOMEM		equ	-6	; Something failed due to lack of memory
E_REFUSE	equ	-7	; Connection refused by remote
E_BADSYN	equ	-8	; A SYN was received in the window
E_BADHANDLE	equ	-9	; Bad connection handle used.
E_LISTEN	equ	-10	; The connection is in LISTEN state
E_NOCCB		equ	-11	; No free CCB's available
E_NOCONNECTION	equ	-12	; No connection matches this packet (TCP)
E_CONNECTFAIL	equ	-13	; Failure to connect to remote port (TCP)
E_BADCLOSE	equ	-14	; Invalid TCP_close() requested
E_USERTIMEOUT	equ	-15	; A user function timed out
E_CNTIMEOUT	equ	-16	; A connection timed out
E_CANTRESOLVE	equ	-17	; Can't resolve the hostname
E_BADDNAME	equ	-18	; Domain name or dotted dec. bad format
E_LOSTCARRIER	equ	-19	; The modem disconnected
E_NOHOSTNAME	equ	-20	; Hostname does not exist
E_DNSWORKLIMIT	equ	-21	; Resolver Work limit reached
E_NONAMESERVER	equ	-22	; No nameservers could be found for query
E_DNSBADFORMAT	equ	-23	; Bad format of DS query
E_UNREACHABLE	equ	-24	; Destination unreachable
E_DNSNOADDR	equ	-25	; No address records exist for host
E_NOROUTINE	equ	-26	; Routine unavailable
E_LOCKED	equ	-27	; Locked by another application
E_FRAGMENT	equ	-28	; Error during fragmentation
E_TTLEXCEED	equ	-29	; Time To Live of an IP packet exceeded
E_PARAMETER	equ	-30	; Problem with a parameter
E_BIGBUF	equ	-31	; Input buffer is too small for data
E_FNAVAIL	equ	-32	; Function not available
E_LASTERROR	equ	32	; ABS of last error code in this list
;----------------------------------------------------------------------------
; End of file:	TRANSPRT.SH
;----------------------------------------------------------------------------
