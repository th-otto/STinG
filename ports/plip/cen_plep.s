;----------------------------------------------------------------------------
;File name:	CEN_PLIP.S			Revision date:	1999.10.17
;Creator:	Ulf Ronald Andersson		Creation date:	1997.08.18
;(c)1997 by:	Ulf Ronald Andersson		All rights reserved
;Feedback to:	dlanor@oden.se			Released as FREEWARE
;----------------------------------------------------------------------------
;Required header declarations:

	.include	"uran\struct.sh"	;PASM adapted structures
	.include	"uran\uran_sys.sh"	;Main system definitions
	.include	"uran\uran_la.sh"	;Line A variables etc
	.include	"uran\uran_dos.sh"	;GEMDOS, BIOS, XBIOS
	.include	"uran\uran_jar.sh"	;Cookie jar handling
	.include	"uran\uran_xb.sh"	;XBRA protocol
	.include	"uran\uran_ram.sh"	;RAM allocation

RAM_chunk	=	80038

XB_cpu_mode	set	1	;never test mode or call Super in XB-macros

	.include	"sting\port.sh"		;only used by modules
	.include	"sting\layer.sh"	;only used by servers
	.include	"sting\transprt.sh"	;used by servers/clients
	.include	"sting\tcp.sh"
	.include	"sting\udp.sh"
	.include	"sting\net_tcon.sh"	;handles network time conv.
	.include	"sting\domain.sh"	;handles domain name/ip conv.

;----------------------------------------------------------------------------

USE_TIMER	equ	0	;1 enables 'my_timer_func', 0 disables it.
USE_LOOPING	equ	1	;1 enables multipacket sending, 0 disables it

M_YEAR	equ	1999
M_MONTH	equ	10
M_DAY	equ	19

.MACRO	M_TITLE
	dc.b	'Centr. PLEP'
.ENDM	M_TITLE

.MACRO	M_VERSION
	dc.b	'01.00'
.ENDM	M_VERSION

.MACRO	M_AUTHOR
	dc.b	'Ronald Andersson'
.ENDM	M_AUTHOR

;-------------------------------------
;	PLIP port constants
;-------------------------------------

MAX_mtu		equ	4096	;mtu may not be raised above this level
MAX_resend	equ	5	;at most 10 attempts per packet
MAX_buffers	equ	10	;at most 10 buffers used in plip_tx_q

MAX_portwork	equ	10/5	;For high level STinG ports
MAX_shake	equ	10/5	;Handshakes should be faster
MAX_work	equ	25/5	;Continuous work must last less
MAX_delay	equ	100/5	;tween-interrupt delays must be less

/*
 * SLIP/PLIP special character codes
 */
slip_ch_END	equ	$C0	;indicates end of packet
slip_ch_ESC	equ	$DB	;indicates byte stuffing
slip_ch_ESC_END	equ	$DC	;ESC ESC_END means END data byte
slip_ch_ESC_ESC	equ	$DD	;ESC ESC_ESC means ESC data byte
plep_ch_ACK	equ	$FD	;indicates byte stuffing

struct	PLIP		;queue structure for raw datagram storage
	struc_p		PLIP_next	;-> next PLIP datagram
	uint16		PLIP_length	;length of raw datagram
	d_alias		PLIP_data	;data of varying size follows
d_end	PLIP

;----------------------------------------------------------------------------
;Start of:	STX program
;----------------------------------------------------------------------------

	.text

;----------------------------------------------------------------------------
text_start:
;----------------------------------------------------------------------------

start:
	bra	start_1

;----------------------------------------------------------------------------
;Start of:	Resident STX data
;----------------------------------------------------------------------------

my_port:
	dc.l	port_name_s	;prt_des_name
	dc.w	L_PAR_PTP	;prt_des_type
	dc.w	0		;prt_des_active		Activation flag
	dc.l	0		;prt_des_flags
	dc.l	0		;prt_des_ip_addr	IP_number
	dc.l	-1		;prt_des_sub_mask
	dc.w	MAX_mtu		;prt_des_mtu
	dc.w	MAX_mtu		;prt_des_max_mtu
	dc.l	0		;prt_des_stat_sd_data
	dc.l	0		;prt_des_send		->Tx queue
	dc.l	0		;prt_des_stat_rcv_data
	dc.l	0		;prt_des_receive	->Rx queue
	dc.w	0		;prt_des_stat_dropped
	dc.l	my_driver	;prt_des_driver		->driver struct
	dc.l	0		;prt_des_next		->next port


my_driver:
	dc.l	my_set_state	;drv_des_set_state
	dc.l	my_cntrl		;drv_des_cntrl
	dc.l	my_send		;drv_des_send
	dc.l	my_receive	;drv_des_receive
	dc.l	driver_name_s	;drv_des_name
	dc.l	version_s	;drv_des_version
	dc.w	((M_YEAR-1980)<<9)|(M_MONTH<<5)|M_DAY
	dc.l	author_s	;drv_des_author
	dc.l	0		;drv_des_next		->next driver
basepage_p:
	dc.l	start-$100	;drv_des_basepage	->basepage of self

port_name_s:
driver_name_s:
	M_TITLE
	dc.b	NUL
	even

version_s:
	M_VERSION
	dc.b	NUL
	even

author_s:
	M_AUTHOR
	dc.b	NUL
	even

sting_drivers:	ds.l	1	;DRV_LIST	*sting_drivers;
tpl:		ds.l	1	;TPL		*tpl;
stx:		ds.l	1	;STX		*stx;

mode_vect:	dc.l	cv_ExitRout1	;-> Interrupt mode code routines
maintime:	dc.l	0	;long timer for total work per interrupt
shaketime:	dc.l	0	;long timer for handshakes

pl_open_sema:			dc.w 0

plip_rx_q:	dc.l	0	;-> queue of raw plip datagrams received
recv_buf_p:	dc.l	0	;-> current raw plip datagram being received
recv_pos_p:	dc.l	0	;-> current receive position in datagram
recv_max_p:	dc.l	0	;-> receive position of datagram end

plip_tx_q_ct:	dc.w	0	;counts buffers in plip_tx_q
plip_tx_q:	dc.l	0	;-> queue of raw plip datagrams to send
send_buf_p:	dc.l	0	;-> current raw plip datagram to send
send_pos_p:	dc.l	0	;-> current send position in datagram
send_max_p:	dc.l	0	;-> send position of datagram end

plip_resend_ct:	dc.l 0
last_byte:		dc.w 0
busy_flag:		dc.w 0xff00
interrupt_sema:	dc.w 0
allocate_sema:	dc.w 0

reg_save:	ds.l	16

byte_watch:	dc.b	0
orig_imrb:	dc.b	0
orig_ierb:	dc.b	0
orig_aer:	dc.b	0

.IF	USE_LOOPING
port_time:	ds.w	1
.ENDIF	USE_LOOPING

;----------------------------------------------------------------------------
;End of:	Resident STX data
;----------------------------------------------------------------------------
;-------------------------------------
;	PLIP port macros
;-------------------------------------
;macro pl_interrupt is used to enable system interrupts temporarily,
;mainly to allow the system timers to advance, but affects all interrupts
;allowed by the mask in given in the 'intmask' argument.

.MACRO	pl_interrupt	intmask
	move		intmask,sr	;reenable old interrupts
	or		#$0700,sr	;disable all interrupts
.ENDM	pl_interrupt

;-------------------------------------

.MACRO	pl_test_busy
	btst		#0,(hw_gpip).w
.ENDM	pl_test_busy

;-------------------------------------

.MACRO	pl_set_strobe	state,temp
	move.b		#14,(hw_psgsel).w
	move.b		(hw_psgrd).w,temp
	.IF		state
		bset		#5,temp
	.ELSE
		bclr		#5,temp
	.ENDIF		state
	move.b		temp,(hw_psgwr).w
.ENDM	pl_set_strobe

;-------------------------------------

.MACRO	pl_set_direct	state,temp	;0=input
	move.b		#7,(hw_psgsel).w
	move.b		(hw_psgrd).w,temp
	.IF		state
		bset		#7,temp
	.ELSE
		bclr		#7,temp
	.ENDIF		state
	move.b		temp,(hw_psgwr).w
.ENDM	pl_set_direct

;-------------------------------------

.MACRO	pl_test_ack
	btst		#0,(hw_iprb).w
.ENDM	pl_test_ack

;-------------------------------------

.MACRO	pl_take_ack
	btst		#0,(hw_iprb).w
	op_isym		beq.s,.done_,uni__v
	move.b		#$FE,(hw_iprb).w
sl_isym	.done_,uni__v
uni__v	=		uni__v+1
.ENDM	pl_take_ack

;-------------------------------------

.MACRO	pl_cli
	bclr		#0,(hw_imrb).w
.ENDM	pl_cli

;-------------------------------------

.MACRO	pl_sti
	bset		#0,(hw_imrb).w
.ENDM	pl_sti


;-------------------------------------

.MACRO	pl_recv_byte	dest
	move.b		#15,(hw_psgsel).w	;select port B
	move.b		(hw_psgrd).w,dest	;dest.b = data from port B
.ENDM	pl_recv_byte

;-------------------------------------

.MACRO	pl_send_byte	data
	move.b		#15,(hw_psgsel).w	;select port B
	move.b		data,(hw_psgwr).w	;write byte data to port B
.ENDM	pl_send_byte

;-------------------------------------

.MACRO	pl_send_ack	temp,temp2
	move.b		#14,(hw_psgsel).w
	move.b		(hw_psgrd).w,temp
	move.b		temp,temp2
	bclr		#5,temp
	bset		#5,temp2
	move.b		temp,(hw_psgwr).w
	move.b		temp,(hw_psgwr).w
	move.b		temp2,(hw_psgwr).w
	move.b		temp2,(hw_psgwr).w
.ENDM	pl_send_ack

;----------------------------------------------------------------------------
;Start of:	Port driver functions
;----------------------------------------------------------------------------

;----------------------------------------------------------------------------
;void	my_cntrl(PORT *port, uint32, int16);
;
my_cntrl:
	link	   a6,#0
	movem.l	   d3-d5/a2-a5,-(sp)
	moveq.l    #E_PARAMETER,d0
	lea	my_port(pc),a5			;a5 -> my_port
	cmpa.l	8(a6),a5			;argument correct
	bne.s      exit_ctrl				;exit if argument incorrect
	moveq.l    #E_FNAVAIL,d0
	move.w     16(a6),d2        ; get type
	lea        ctrl_tab(pc),a0
ctrl_loop:
	move.w     (a0)+,d1
	ble.s      exit_ctrl
	movea.l    (a0)+,a1
	cmp.w      d2,d1
	bne.s      ctrl_loop
	move.l     12(a6),d0        ; get argument
	movea.l    d0,a0
	jsr        (a1)
exit_ctrl:
	movem.l	   (sp)+,d3-d5/a2-a5
	unlk	   a6
	rts

ctrl_tab:
	dc.w    CTL_GENERIC_SET_IP
	dc.l	ctrl_set_ip
	dc.w    CTL_GENERIC_GET_IP
	dc.l	ctrl_get_ip
	dc.w    CTL_GENERIC_SET_MTU
	dc.l	ctrl_set_mtu
	dc.w    CTL_GENERIC_GET_MTU
	dc.l	ctrl_get_mtu
	dc.w    CTL_GENERIC_GET_MMTU
	dc.l	ctrl_get_mmtu
	dc.w    CTL_GENERIC_GET_TYPE
	dc.l	ctrl_get_type
	dc.w    CTL_GENERIC_GET_STAT
	dc.l	ctrl_get_stat
	dc.w    CTL_GENERIC_CLR_STAT
	dc.l	ctrl_clr_stat
	dc.w    0

ctrl_set_ip:
    move.l     d0,prt_des_ip_addr(a5)
    moveq.l    #0,d0
    rts

ctrl_get_ip:
    move.l     prt_des_ip_addr(a5),(a0)
    moveq.l    #E_NORMAL,d0
    rts

ctrl_set_mtu:
    clr.l      d1
    move.w     prt_des_max_mtu(a5),d1
    cmp.l      d1,d0
    bhi.s      ctrl_set_mtu1
    cmp.w      #48,d0
    bcc.s      ctrl_set_mtu2
ctrl_set_mtu1:
    moveq.l    #E_PARAMETER,d0
    rts
ctrl_set_mtu2:
    move.w     d0,prt_des_mtu(a5)
    moveq.l    #E_NORMAL,d0
    rts

ctrl_get_mtu:
    move.w     prt_des_mtu(a5),(a0)
    moveq.l    #E_NORMAL,d0
    rts

ctrl_get_mmtu:
    move.w     prt_des_max_mtu(a5),(a0)
    moveq.l    #E_NORMAL,d0
    rts

ctrl_get_type:
    move.w     prt_des_type(a5),(a0)
    moveq.l    #E_NORMAL,d0
    rts

ctrl_get_stat:
    clr.l      d0
    move.w     prt_des_stat_dropped(a5),d0
    move.l     d0,(a0)+
    move.l     prt_des_stat_sd_data(a5),(a0)+
    move.l     prt_des_stat_rcv_data(a5),(a0)+
    moveq.l    #E_NORMAL,d0
    rts

ctrl_clr_stat:
    clr.w      prt_des_stat_dropped(a5)
    clr.l      prt_des_stat_sd_data(a5)
    clr.l      prt_des_stat_rcv_data(a5)
    moveq.l    #E_NORMAL,d0
    rts


;----------------------------------------------------------------------------
;void	my_send(PORT *port);

my_send:
	move.l		my_port+prt_des_send(pc),d0	;anything to send
	ble		.exit_direct			;exit if none to send
	cmpi.l		#my_port,4(sp)			;argument == my_port ?
	bne		.exit_direct			;exit if argument incorrect
	movem.l		d2-d3/a2-a5,-(sp)		;protect entry regs
	lea		my_port+prt_des_send(pc),a5	;a5 ->Tx queue ptr
	move.l		d0,d3				;d3 ->Tx queue

.IF	USE_LOOPING
	move		(_hz_200+2).w,port_time		;find work start time
.ENDIF	USE_LOOPING
.send_loop:
	move.l		d3,a4				;a4 -> Tx dgram
	move.l		IPDG_next(a4),(a5)		;unlink chain
	check_dgram_ttl	(a4)				;check TTL
	tst		d0
	bpl		.send_it			;continue if ok
	addq		#1,prt_des_stat_dropped-prt_des_send(a5)
	bra		.skip_it

.send_it:
	bsr		pass_send_pkt
	bgt		.count_sent_data
	move.l		(a5),IPDG_next(a4)	;relink to chain head
	move.l		a4,(a5)			;relink as chain head again
	bra		.exit

.count_sent_data:
	add.l		d0,prt_des_stat_sd_data-prt_des_send(a5)	;sent data
	bsr		retrigger_plip			;retrigger PLIP
.discard_datagram:
	IP_discard	(a4),#1				;discard datagram
.skip_it:
.IF	USE_LOOPING
	move.l		(a5),d3			;d3 = Tx queue, empty ?
	ble		.exit			;exit when queue empty
	move		(_hz_200+2).w,d0
	sub		port_time(pc),d0	;d0 = elapsed time
	cmp		#MAX_portwork,d0	;timeout ?
	blo		.send_loop		;loop back until 20 ms
.ENDIF	USE_LOOPING
.exit:
	movem.l		(sp)+,d2-d3/a2-a5		;restore entry regs
.exit_direct:
	bra		retrigger_plip			;retrigger PLIP

;end of my_send
;----------------------------------------------------------------------------
;void	my_receive(PORT *port);

my_receive:
	cmpi.l		#my_port,4(sp)		;argument == my_port ?
	bne		.exit_direct		;exit if argument incorrect
	move.l		plip_rx_q(pc),d0	;d0 -> raw Rx Queue, empty ?
	beq		.exit_direct		;exit directly if empty
	move		sr,-(sp)		;push sr
	movem.l		d2-d5/a2-a5,-(sp)	;push entry regs
	lea		my_port(pc),a5		;a5 -> my_port

.IF	USE_LOOPING
	move		(_hz_200+2).w,port_time		;find work start time
.ENDIF	USE_LOOPING
.transfer_loop:			;loop start to transfer raw dgrams to internal
	or		#$0700,sr		;disable interrupts
	move.l		plip_rx_q(pc),d0	;d0 -> raw Rx Queue, empty ?
	beq		.exit			;we're done if empty
	move.l		d0,a4			;a4 -> raw Rx queue
	move.l		(a4),plip_rx_q		;unlink first Rx datagram
	move		8*4(sp),sr		;reenable interrupts

	move.w	PLIP_length(a4),d0	;d0 -> raw datagram length
	lea		PLIP_data(a4),a0	;a0 -> data
	clr.l		-(sp)			;reserve result space on stack
	lea		(sp),a1			;a1 -> result space
	bsr		make_IP_dgram		;create an internal datagram
	move.l		(sp)+,d0		;pop result to d0
	ble		.error			;drop packet on failure

	move.l		d0,a3			;a3 -> internal datagram
	set_dgram_ttl	(a3)			;init time-to-live data

	lea		prt_des_receive(a5),a0	;a0 -> receive queue root ptr
	bra		.store_test

.store_loop:
	lea.l		IPDG_next(a1),a0	;a1 -> next datagram or is NULL
.store_test:
	move.l		(a0),a1			;a1 -> receive queue root
	move.l		a1,d0			;test next datagram
	bne		.store_loop		;loop until queue end found
	move.l		a3,(a0)			;store new dgram at queue end
	clr.l		d0
	move		PLIP_length(a4),d0
	add.l		d0,prt_des_stat_rcv_data(a5)	;count received data
	bra		.release		;go release raw datagram

.error:
	addq		#1,prt_des_stat_dropped(a5)	;increment drop count
.release:
    ori.w      #$0700,sr
    R_free	(a4)		;release used datagram buffer
    move.w     32(a7),sr
.IF	USE_LOOPING
	move.l		plip_rx_q(pc),d3	;d3 -> Tx queue, empty ?
	ble		.exit			;exit when queue empty
	move		(_hz_200+2).w,d0
	sub		port_time(pc),d0	;d0 = elapsed time
	cmp		#MAX_portwork,d0	;timeout ?
	blo		.transfer_loop		;loop back until 20 ms
.ENDIF	USE_LOOPING
.exit:
	movem.l		(sp)+,d2-d5/a2-a5	;pull entry regs
	move		(sp)+,sr		;pull sr
.exit_direct:
	bra		retrigger_plip

;end of my_receive
;----------------------------------------------------------------------------
;uint16	my_set_state(PORT *port, uint16 state);

my_set_state:
	link		a6,#0
	movem.l		d2-d5/a2-a5,-(sp)
	lea		my_port(pc),a5		;a5 -> my_port
	clr.l		d0			;prep error flag
	cmpa.l		8(a6),a5		;argument correct ?
	bne		.exit			;exit if argument incorrect
	move		12(a6),d0		;d0 = new state
	beq		.passivate
.activate:

;Add port dependent activation code here
;This must include all initialization the port needs

	xbios		Supexec,pl_open(pc)

	bra		.done

.passivate:

;Add port dependent passivation code here
;This must include release of any KRmalloc blocks etc

	xbios		Supexec,pl_close(pc)
;	
.done:
	moveq		#1,d0
.exit:
	movem.l		(sp)+,d2-d5/a2-a5
	unlk		a6
	rts

;end of my_set_state
;----------------------------------------------------------------------------
;End of:	Port driver functions
;----------------------------------------------------------------------------
;Start of:	Resident subroutines
;----------------------------------------------------------------------------
;Aregs:	a0-a3 is free,  a4->datagram
;Dregs  d0-d3 is free

pass_send_pkt:
	clr.l		d0				;d0 = NULL  (error)
	cmpi		#MAX_buffers,plip_tx_q_ct	;too many buffers ?
	bhs		.exit_direct			;exit if past limit

	moveq		#sizeof_IPHD,d3
	add		IPDG_opt_length(a4),d3
	add		IPDG_pkt_length(a4),d3	;d3 = PLIP buffer size
	moveq		#sizeof_PLIP,d0		;d0 = PLIP header size
	add		d3,d0			;d0 = total size of PLIP struct
    move.w     sr,-(a7)
    ori.w      #$0700,sr
	R_alloc	d0			;allocate STinG RAM
    move.w     (a7)+,sr
	tst.l		d0			;did we get any ?
	ble		.exit_direct		;exit if no RAM available

	move.l		d0,a2			;a2 -> PLIP buffer
	clr.l		(a2)			;init link to next buffer
	move		d3,PLIP_length(a2)	;set data length of this one

	lea		IPDG_hdr(a4),a0		;a0 -> IP header
	lea		PLIP_data(a2),a1	;a1 -> PLIP buffer
	moveq		#sizeof_IPHD,d0		;d0 =  standard IP header size
	buf_copy_l.w	a0,a1,d0		;copy header as longs
	move.l		IPDG_options(a4),a0
	move		IPDG_opt_length(a4),d0
	buf_copy_l.w	a0,a1,d0		;copy options as longs
	move.l		IPDG_pkt_data(a4),a0
	move		IPDG_pkt_length(a4),d0
	buf_copy_b.w	a0,a1,d0		;copy packet data as bytes

	move		sr,d2			;save interrupt mask in d2
	ori		#$0700,sr		;disable interrupts

	lea		plip_tx_q(pc),a0	;a0 -> send queue root ptr
	move.l		a0,d0			;d0 -> send queue root ptr
.store_loop:
	move.l		d0,a0			;a0 -> current chain ptr
	move.l		(a0),d0			;d0 -> next datagram or is NULL
	bne		.store_loop		;loop until queue end reached
	move.l		a2,(a0)
	addq		#1,plip_tx_q_ct
	move		d2,sr			;restore interrupt mask
	move.l		d3,d0			;return PLIP packet length
.exit_direct:
	rts		;error <= zero (flagged) else => PLIP length

;ends	pass_send_pkt
;----------------------------------------------------------------------------

retrigger_plip:
	move		sr,-(sp)
	or		#$0700,sr		;disable interrupts
	move.l		mode_vect(pc),d0	;d0 -> current mode routine
	cmp.l		#cv_InitRecv1,d0		;are we in an initial mode ?
	bls		try_more_sending	;try sending when resting
	move.l		(_hz_200).w,d0		;d0 = current time
	sub.l		maintime(pc),d0		;d0 = current time - maintime
	cmp.l		#MAX_delay,d0		;timeout yet ?
	bcc		abort_old_stuff	;abort old traffic at timeout
	bra	retrigger_exit

abort_old_stuff:
	move.l		(_hz_200).w,d0		;d0 = current time
	sub.l		shaketime(pc),d0		;d0 = current time - shaketime
	cmp.l		#MAX_delay,d0		;timeout yet ?
	bcc		    abort_recv
	bra	retrigger_exit
abort_recv:
	bsr allocate_buffers
try_more_sending:
	move.w		plip_tx_q_ct(pc),d0	;Is there anything to send ?
	ble		retrigger_exit
send_stuff:
	pl_set_strobe	1,d0			;normalize strobe
	move.l		(_hz_200).w,maintime	;memorize time
	pl_set_direct 1,d1
	pl_send_byte	#plep_ch_ACK			;send ack as initial RTS
	pl_set_strobe	0,d1
	move.l		#cv_Recv_Pkt,mode_vect	;set mode vector
retrigger_exit:
	move		(sp)+,sr		;restore entry interrupt mask
	rts					;return to caller

;ends	retrigger_plip
;----------------------------------------------------------------------------
;void  make_IP_dgram (uint8 *buffer, int16 buff_len, IP_DGRAM **dgram);

make_IP_dgram:
	movem.l		d3-d5/a3-a5,-(sp)
	move.l		a0,a4			;a4 is buffer
	move		d0,d5			;d5 is buff_len
	move.l		a1,a5			;a5 is dgram
	clr.l		(a5)			;*dgram = NULL
	cmp		#sizeof_IPHD,d5
	ble		.exit			;if (buff_len < sizeof (IP_HDR)) return;

	KRmalloc	#sizeof_IPDG
	tst.l		d0
	ble		.exit			;if ((temp = KRmalloc (sizeof (IP_DGRAM))) == NULL) return;
	move.l		d0,a3			;a3 is temp
	lea		IPDG_hdr(a3),a1		;dst_arg = temp
	move		#sizeof_IPHD,d0		;len_arg = sizeof_IPHD
	buf_copy_l.w	a4,a1,d0		;memcpy (& temp->hdr, buffer, sizeof (IP_HDR)); buffer += sizeof (IP_HDR);

	cmp		IPDG_hdr+IPHD_length(a3),d5
	blo		.free_a3		;if (temp->hdr.length > buff_len
	move.b		IPDG_hdr+IPHD_verlen_f(a3),d0
	andi.l		#amask_IPHD_f_hd_len,d0
	asl		#2,d0
	cmp		d5,d0
	bhi		.free_a3		;|| (temp->hdr.hd_len << 2) > buff_len
	move		d0,d3			;d3 = (temp->hdr.hd_len << 2)
	subi		#sizeof_IPHD,d0
	blt		.free_a3		;|| temp->hdr.hd_len < 5) {KRfree (temp); return;}
	move		d0,IPDG_opt_length(a3)
	KRmalloc	d0
	move.l		d0,IPDG_options(a3)	;temp->options  = KRmalloc (temp->opt_length = (temp->hdr.hd_len << 2) - sizeof (IP_HDR));
	clr.l		d0
	move		IPDG_hdr+IPHD_length(a3),d0
	sub		d3,d0
	move		d0,IPDG_pkt_length(a3)
	KRmalloc	d0
	move.l		d0,IPDG_pkt_data(a3)	;temp->pkt_data = KRmalloc (temp->pkt_length = temp->hdr.length - (temp->hdr.hd_len << 2));
	ble		.discard_a3
	move.l		d0,d3			;d3 = temp->pkt_data
	move.l		IPDG_options(a3),d0	;d0 = temp->options
	ble		.discard_a3		;if (temp->options == NULL || temp->pkt_data == NULL ) {IP_discard (temp, TRUE); return;}
	move.l		d0,a1			;a1 = temp->options
	move		IPDG_opt_length(a3),d0
	beq		.have_option
	buf_copy_l.w	a4,a1,d0		;memcpy (temp->options, buffer, temp->opt_length);
.have_option:
	move.l		d3,a1			;a1 = temp->pkt_data
	move		IPDG_pkt_length(a3),d0
	beq		.have_data
	buf_copy_b.w	a4,a1,d0		;memcpy (temp->pkt_data, buffer + temp->opt_length, temp->pkt_length);
.have_data:
	clr.l		IPDG_next(a3)
	move.l		a3,(a5)			;*dgram = temp;
.exit:
	movem.l		(sp)+,d3-d5/a3-a5
	rts

.free_a3:
	KRfree		(a3)
	bra		.exit

.discard_a3:
	IP_discard	(a3),#1
	bra		.exit

;ends	make_IP_dgram
;----------------------------------------------------------------------------
;void  pl_open(void);

pl_open:
    tst.b      pl_open_sema
    beq.s      pl_open1
    bsr        pl_close
pl_open1:
    bsr        release_buffers

	move.b		(hw_imrb).w,d3		;d3 = entry hw_imrb
	pl_cli					;mask reception interrupt
	move		sr,-(sp)		;push interrupt mask
	or		#$0700,sr		;disable interrupts

	pl_set_direct	0,d0			;set port for input
	pl_set_strobe	1,d0			;normalize strobe
	pl_take_ack				;clear pending ACK if present

	XB_remove	cent_busy_XB(pc),(iv_cenbusy).w	;unlink interrupt code
	bpl		.done_primary_open
	move.b		d3,orig_imrb		;save initial hw_imrb state
	move.b		(hw_ierb).w,orig_ierb	;save initial hw_ierb state
	move.b		(hw_aer).w,orig_aer	;save initial hw_aer state
.done_primary_open:

	XB_install	cent_busy_XB(pc),(iv_cenbusy).w	;link interrupt code
	bset		#0,(hw_ierb).w		;enable BUSY interrupt
	bclr		#0,(hw_aer).w		;set BUSY edge sense to falling
	pl_take_ack				;clear pending ACK if present
	move.l		#cv_InitRecv1,mode_vect	;set mode vector
	move		(sp)+,sr		;reenable interrupts
	pl_take_ack				;clear pending ACK if present
	pl_sti					;unmask reception interrupt
	st pl_open_sema
	rts

;ends	pl_open
;----------------------------------------------------------------------------
;void  pl_close(void);

pl_close:
	move		sr,-(sp)		;push interrupt mask
	or		#$0700,sr		;disable interrupts
	move.l		#cv_ExitRout1,mode_vect	;set mode vector

	move.b		(hw_imrb).w,d3		;d3 = entry hw_imrb
	pl_cli					;mask reception interrupt
	pl_set_direct	0,d0			;set port for input
	pl_set_strobe	1,d0			;normalize strobe
	pl_take_ack				;clear pending ACK if present

	XB_remove	cent_busy_XB(pc),(iv_cenbusy).w
	bmi		.done_primary_close
	move.b		orig_imrb(pc),d3	;d3 = original imrb

	btst		#0,orig_ierb(pc)
	beq		.clr_b0_ierb
.set_b0_ierb:
	bset		#0,(hw_ierb).w
	bra		.done_ierb

.clr_b0_ierb:
	bclr		#0,(hw_ierb).w
.done_ierb:

	btst		#0,orig_aer(pc)
	beq		.clr_b0_aer
.set_b0_aer:
	bset		#0,(hw_aer).w
	bra		.done_aer

.clr_b0_aer:
	bclr		#0,(hw_aer).w
.done_aer:

.done_primary_close:
	pl_take_ack				;clear pending ACK if present

	btst		#0,d3
	beq		.clr_b0_imrb
.set_b0_imrb:
	bset		#0,(hw_imrb).w
	bra		.done_imrb

.clr_b0_imrb:
	bclr		#0,(hw_imrb).w
.done_imrb:

	sf pl_open_sema

	move		(sp)+,sr		;pull interrupt mask
release_buffers:
	move		sr,-(sp)		;push interrupt mask
	or		#$0700,sr		;disable interrupts
	pl_set_strobe	1,d0			;normalize strobe
	pl_set_direct	0,d0			;set port for input
	move.l		recv_buf_p(pc),d0
	ble		.recv_buf_released
	R_free.i	d0
	clr.l		recv_buf_p
.recv_buf_released:
.release_rx_queue:
	pl_interrupt	(sp)			;poll interrupts
	move.l		plip_rx_q(pc),d0	;d0 -> raw datagram or is null
	beq		.rx_queue_released
	move.l		d0,a0			;a0 -> raw datagram
	move.l		(a0),plip_rx_q		;unlink this raw datagram
	R_free		(a0)			;release allocated buffer
	bra		.release_rx_queue

.rx_queue_released:
	move.l		send_buf_p(pc),d0
	ble.s		.send_buf_released
	R_free.i	send_buf_p(pc)
	clr.l		send_buf_p
.send_buf_released:
.release_tx_queue:
	pl_interrupt	(sp)			;poll interrupts
	move.l		plip_tx_q(pc),d0	;d0 -> raw datagram or is null
	beq		.tx_queue_released
	move.l		d0,a0			;a0 -> raw datagram
	move.l		(a0),plip_tx_q		;unlink this raw datagram
	R_free		(a0)			;release allocated buffer
	bra		.release_tx_queue

.tx_queue_released:
	clr		plip_tx_q_ct
	move		(sp)+,sr		;pull interrupt mask
	rts

;ends	pl_close
;----------------------------------------------------------------------------

allocate_recv_buf:
	move.l		recv_buf_p(pc),d0	;is there an old buffer ?
	bgt		.have_buffer		;keep old buffer if present
	R_alloc	#sizeof_PLIP+2+MAX_mtu	;allocate raw datagram buffer
	tst.l		d0			;did we get any ?
	ble		.exit			;exit on allocation failure
	move.l		d0,recv_buf_p
.have_buffer:
	move.l		d0,a0			;a0 -> receive buffer
	clr.l		(a0)			;clear its own queue link
	move		#MAX_mtu,d0
	move		d0,PLIP_length(a0)	;setup its max data length
	lea		PLIP_data(a0),a1
	move.l		a1,recv_pos_p		;init buffer position ptr
	lea		PLIP_data(a0,d0.w),a1	;a1 -> buffer end
	move.l		a1,recv_max_p		;init buffer end ptr
.exit:
	rts

allocate_buffers:
	clr.w      -(a7)
	move.w     sr,-(a7)
	ori.w      #$0700,sr
	move.w     allocate_sema(pc),d0
	beq        allocate4
	move.b     allocate_sema+1(pc),d0
	beq.s      allocate2
	movem.l    d2/a2,-(a7)
	move.l     send_buf_p(pc),d0
	ble.s      allocate1
	movea.l    d0,a0
	move.l     plip_tx_q(pc),(a0)
	move.l     a0,plip_tx_q
	addq.w     #1,plip_tx_q_ct
	clr.l      send_buf_p
	addq.w     #1,my_port+prt_des_stat_dropped
allocate1:
	pl_set_strobe	1,d0			;normalize strobe
	pl_set_direct	0,d0			;set port for input
    sf         allocate_sema+1
    move.l     #cv_InitRecv1,mode_vect
    movem.l    (a7)+,d2/a2
allocate2:
	move.b     allocate_sema(pc),d0
	beq.s      allocate4
	movem.l    d2/a2,-(a7)
	tst.b      allocate_sema
	beq.s      allocate3
	addq.w     #1,my_port+prt_des_stat_dropped
allocate3:
	pl_set_strobe	1,d0			;normalize strobe
	pl_set_direct	0,d0			;set port for input
    sf         allocate_sema
	move.l     #cv_InitRecv1,mode_vect
	movem.l    (a7)+,d2/a2
allocate4:
	move.l     #cv_InitRecv1,mode_vect
	move.w     (a7)+,sr
	move.w     (a7)+,d0
	ext.l      d0
	rts

;----------------------------------------------------------------------------
;End of:	Resident subroutines
;----------------------------------------------------------------------------
;Start of:	Resident interrupt routines
;----------------------------------------------------------------------------

	XB_define	cent_busy_XB,'plep'	;XBRA header
	or		#$0700,sr		;disable all interrupts
interrupt_loop:
	movem.l		d0-d2/a0-a2,reg_save	;save entry regs d0-d2/a0-a2
	move.w     (a7),-(a7)
	clr.w      d0
	move.b     (a7),d0
	move.b     int_mask_table(pc,d0.w),(a7)
	move.l		(_hz_200).w,maintime	;memorize initial time
	pl_take_ack				;clear pending ACK if present
	bra cv_GoRout

int_mask_table:
	.dc.b 0x24,0x24,0x24,0x24,0x24,0x25,0x26,0x27,0x24,0x24,0x24,0x24,0x24,0x25,0x26,0x27
	.dc.b 0x24,0x24,0x24,0x24,0x24,0x25,0x26,0x27,0x24,0x24,0x24,0x24,0x24,0x25,0x26,0x27
	.dc.b 0x24,0x24,0x24,0x24,0x24,0x25,0x26,0x27,0x24,0x24,0x24,0x24,0x24,0x25,0x26,0x27
	.dc.b 0x24,0x24,0x24,0x24,0x24,0x25,0x26,0x27,0x24,0x24,0x24,0x24,0x24,0x25,0x26,0x27
	.dc.b 0x24,0x24,0x24,0x24,0x24,0x25,0x26,0x27,0x24,0x24,0x24,0x24,0x24,0x25,0x26,0x27
	.dc.b 0x24,0x24,0x24,0x24,0x24,0x25,0x26,0x27,0x24,0x24,0x24,0x24,0x24,0x25,0x26,0x27
	.dc.b 0x24,0x24,0x24,0x24,0x24,0x25,0x26,0x27,0x24,0x24,0x24,0x24,0x24,0x25,0x26,0x27
	.dc.b 0x24,0x24,0x24,0x24,0x24,0x25,0x26,0x27,0x24,0x24,0x24,0x24,0x24,0x25,0x26,0x27
	.dc.b 0xa4,0xa4,0xa4,0xa4,0xa4,0xa5,0xa6,0xa7,0xa4,0xa4,0xa4,0xa4,0xa4,0xa5,0xa6,0xa7
	.dc.b 0xa4,0xa4,0xa4,0xa4,0xa4,0xa5,0xa6,0xa7,0xa4,0xa4,0xa4,0xa4,0xa4,0xa5,0xa6,0xa7
	.dc.b 0xa4,0xa4,0xa4,0xa4,0xa4,0xa5,0xa6,0xa7,0xa4,0xa4,0xa4,0xa4,0xa4,0xa5,0xa6,0xa7
	.dc.b 0xa4,0xa4,0xa4,0xa4,0xa4,0xa5,0xa6,0xa7,0xa4,0xa4,0xa4,0xa4,0xa4,0xa5,0xa6,0xa7
	.dc.b 0xa4,0xa4,0xa4,0xa4,0xa4,0xa5,0xa6,0xa7,0xa4,0xa4,0xa4,0xa4,0xa4,0xa5,0xa6,0xa7
	.dc.b 0xa4,0xa4,0xa4,0xa4,0xa4,0xa5,0xa6,0xa7,0xa4,0xa4,0xa4,0xa4,0xa4,0xa5,0xa6,0xa7
	.dc.b 0xa4,0xa4,0xa4,0xa4,0xa4,0xa5,0xa6,0xa7,0xa4,0xa4,0xa4,0xa4,0xa4,0xa5,0xa6,0xa7
	.dc.b 0xa4,0xa4,0xa4,0xa4,0xa4,0xa5,0xa6,0xa7,0xa4,0xa4,0xa4,0xa4,0xa4,0xa5,0xa6,0xa7

;----------------------------------------------------------------------------

cv_GoRout:
	pl_set_strobe	1,d0			;raise strobe to complete CTS
	pl_set_direct	0,d0			;set port for input
	pl_test_busy
	sne busy_flag
	bne cv_GoRout1
    move.b     #$0F,(hw_psgsel).w
    move.b     (hw_psgsel).w,busy_flag+1
    pl_send_ack d0,d1
cv_GoRout1:
    move.w     busy_flag(pc),d0
	move.l		mode_vect(pc),a0	;get current mode vector
	jmp		(a0)			;go to current mode code

cv_ExitRecv:
	move.l     (_hz_200).w,shaketime
	addq.b     #4,byte_watch
	bcs        cv_ExitRout
	btst       #0,(hw_iprb).w
	beq.s      cv_ExitRout
	move.b     #$FE,(hw_iprb).w
	btst       #0,(hw_iprb).w
	beq.s      cv_ExitRecv1
	move.b     #$FE,(hw_iprb).w
cv_ExitRecv1:
	move.w     (a7),sr
	ori.w      #$0700,sr
	bra        cv_GoRout

cv_ExitRout:
	addq		#2,sp			 ;pop polling SR
	movem.l		reg_save(pc),d0-d2/a0-a2 ;pull entry regs d0-d2/a0-a2
	move.b		#$FE,(hw_isrb).w		;flag service to hardware
	rte
cv_ExitRout1:
	bra cv_ExitRecv

;----------------------------------------------------------------------------

cv_InitRecv:
	move.l #cv_InitRecv0,mode_vect
	bra cv_ExitRecv

cv_InitRecv0:
	bpl		cv_InitRecv3		;abort on overlong ACK
	pl_set_direct 1,d1
	pl_send_byte #0
	pl_set_strobe	0,d1			;lower strobe as initial CTS
cv_InitRecv4:
	move.l #cv_InitRecv1,mode_vect
	bra cv_ExitRecv

cv_InitRecv3:
	move.l #cv_InitRecv1,mode_vect
	bra cv_checkack

cv_InitRecv1:
	bpl cv_checkack
	move.w plip_tx_q_ct(pc),d0
	beq cv_InitRecv4
	pl_set_direct 1,d1
	pl_send_byte #plep_ch_ACK
	pl_set_strobe	0,d1
	move.l #cv_21,mode_vect
	bra cv_ExitRecv

cv_InitRecv2:
	move.l #cv_InitRecv5,mode_vect
	bra cv_ExitRecv
	
cv_InitRecv5:
	bpl cv_InitRecv3
	sf interrupt_sema
	pl_set_direct 1,d1
	pl_send_byte #plep_ch_ACK
	pl_set_strobe	0,d1
	move.l #cv_21,mode_vect
	bra cv_ExitRecv
	
cv_checkack:
	;cmp.w #0,d0
	dc.w 0xb07c,0
	beq cv_InitRecv4
	cmp.w #plep_ch_ACK,d0
	bne cv_InitRecv
	move.w plip_tx_q_ct(pc),d0
	beq cv_1
	move.b interrupt_sema(pc),d0
	bne cv_InitRecv2
cv_1:
	bsr allocate_recv_buf
	ble cv_InitRecv
	move.l #cv_checkack2,mode_vect
	bra cv_ExitRecv

cv_checkack2:
	bpl cv_InitRecv3
	pl_set_direct 1,d1
	pl_send_byte #0xfe
	pl_set_strobe 0,d1
	move.l #cv_checkack3,mode_vect
	bra cv_ExitRecv

cv_checkack3:
	bpl cv_InitRecv3
	clr.l plip_resend_ct
	pl_send_ack d0,d1
	move.l #cv_checkack4,mode_vect
	bra cv_ExitRecv
	
cv_checkack4:
	bmi cv_checkack7
	subi.b #0xf1,d0
	cmp.b #0x0f,d0
	bls cv_checkack5
	move.l		recv_pos_p(pc),a0	;a0 -> IP packet position
	cmpa.l		recv_max_p(pc),a0	;is it too high ?
	bcc cv_checkack7
	addi.b #0xa0,d0
	move.b d0,(a0)+
	move.l a0,recv_pos_p
	move.w plip_resend_ct(pc),d1
	rol.w #8,d1
	eor.b d0,d1
	move.b d1,d0
	lsr.b #4,d0
	eor.b d0,d1
	move.b d1,d0
	lsl.w #7,d0
	eor.b d1,d0
	lsl.w #5,d0
	eor.w d0,d1
	move.w d1,plip_resend_ct
	st allocate_sema
	move.l #cv_checkack8,mode_vect
	bra cv_ExitRecv
cv_checkack5:
	ext.w d0
	add.w d0,d0
	move.w checktab(pc,d0.w),d0
	jmp checktab(pc,d0.w)

checktab:
	dc.w cv_checkack7-checktab
	dc.w cv_checkack7-checktab
	dc.w cv_checkack7-checktab
	dc.w cv_checkack7-checktab
	dc.w cv_checkack7-checktab
	dc.w cv_checkack7-checktab
	dc.w cv_checkack7-checktab
	dc.w cv_checkack7-checktab
	dc.w cv_checkack7-checktab
	dc.w cv_checkack7-checktab
	dc.w cv_14-checktab
	dc.w cv_16-checktab
	dc.w cv_checkack7-checktab
	dc.w cv_checkack7-checktab
	dc.w cv_checkack7-checktab
	dc.w cv_checkack6-checktab

cv_checkack6:
	move.l #cv_InitRecv1,mode_vect
	bra cv_ExitRecv
	
cv_checkack7:
	move.l #cv_InitRecv0,mode_vect
	bra cv_ExitRecv

cv_checkack8:
	bmi cv_12
	subi.b #0xf1,d0
	cmp.b #0x0f,d0
	bls cv_checkack9
	move.l		recv_pos_p(pc),a0	;a0 -> IP packet position
	cmpa.l		recv_max_p(pc),a0	;is it too high ?
	bcc cv_12
	addi.b #0xa0,d0
	move.b d0,(a0)+
	move.l a0,recv_pos_p
	move.w plip_resend_ct(pc),d1
	rol.w #8,d1
	eor.b d0,d1
	move.b d1,d0
	lsr.b #4,d0
	eor.b d0,d1
	move.b d1,d0
	lsl.w #7,d0
	eor.b d1,d0
	lsl.w #5,d0
	eor.w d0,d1
	move.w d1,plip_resend_ct
	move.l #cv_checkack8,mode_vect
	bra cv_ExitRecv
cv_checkack9:
	ext.w d0
	add.w d0,d0
	move.w checktab2(pc,d0.w),d0
	jmp checktab2(pc,d0.w)

checktab2:
	dc.w cv_12-checktab2
	dc.w cv_12-checktab2
	dc.w cv_12-checktab2
	dc.w cv_12-checktab2
	dc.w cv_12-checktab2
	dc.w cv_12-checktab2
	dc.w cv_12-checktab2
	dc.w cv_12-checktab2
	dc.w cv_12-checktab2
	dc.w cv_12-checktab2
	dc.w cv_14-checktab2
	dc.w cv_16-checktab2
	dc.w cv_12-checktab2
	dc.w cv_12-checktab2
	dc.w cv_12-checktab2
	dc.w cv_10-checktab2
cv_10:
	tst.b allocate_sema
	beq cv_11
	addq.w #1,my_port+prt_des_stat_dropped
cv_11:
	pl_set_strobe 1,d0
	pl_set_direct 0,d0
	sf allocate_sema
	move.l #cv_InitRecv1,mode_vect
	move.l #cv_InitRecv1,mode_vect
	bra cv_ExitRecv
	
cv_12:
	tst.b allocate_sema
	beq cv_13
	addq.w #1,my_port+prt_des_stat_dropped
cv_13:
	pl_set_strobe 1,d0
	pl_set_direct 0,d0
	sf allocate_sema
	move.l #cv_InitRecv1,mode_vect
	move.l #cv_InitRecv0,mode_vect
	bra cv_ExitRecv

cv_14:
	move.l #cv_15,mode_vect
	bra cv_ExitRecv
	
cv_15:
	bmi cv_12
	addi.b #0x2f,d0
	move.l		recv_pos_p(pc),a0	;a0 -> IP packet position
	move.b d0,(a0)+
	move.l a0,recv_pos_p
	move.w plip_resend_ct(pc),d1
	rol.w #8,d1
	eor.b d0,d1
	move.b d1,d0
	lsr.b #4,d0
	eor.b d0,d1
	move.b d1,d0
	lsl.w #7,d0
	eor.b d1,d0
	lsl.w #5,d0
	eor.w d0,d1
	move.w d1,plip_resend_ct
	move.l #cv_checkack8,mode_vect
	bra cv_ExitRecv

cv_16:
	move.l #cv_17,mode_vect
	bra cv_ExitRecv

cv_17:
	bmi cv_18
	;cmp.w #0,d0
	dc.w 0xb07c,0
	beq cv_10
	bra cv_12
cv_18:
	move.w plip_resend_ct(pc),d0
	bne cv_12
	move.l		recv_buf_p(pc),a0
	move.l		recv_pos_p(pc),d0
	lea PLIP_data+2(a0),a1
	sub.l a1,d0
	ble cv_checkack7
	move.w d0,PLIP_length(a0)
	lea plip_rx_q(pc),a0
	move.l a0,d0
cv_19:
	move.l d0,a0
	move.l PLIP_next(a0),d0
	bne cv_19
	move.l recv_buf_p(pc),(a0)
	clr.l recv_buf_p
	sf allocate_sema
	st interrupt_sema
	pl_set_direct 1,d1
	pl_send_byte #0xfc
	pl_set_strobe 0,d1
	move.l #cv_20,mode_vect
	bra cv_ExitRecv
	
cv_20:
	pl_send_ack d0,d1
	move.l #cv_InitRecv1,mode_vect
	bra cv_ExitRecv

cv_Recv_Pkt:
cv_21:
	bpl cv_InitRecv3
	pl_send_ack d0,d1
	move.l #cv_22,mode_vect
	bra cv_ExitRecv

cv_22:
	bmi cv_InitRecv
	;cmp.w #0,d0
	dc.w 0xb07c,0
	beq cv_InitRecv4
	cmp.w #0xfe,d0
	bne cv_InitRecv
	move.l plip_tx_q(pc),d0
	ble cv_InitRecv
	move.l d0,send_buf_p
	move.l d0,a0
	move.l PLIP_next(a0),plip_tx_q
	subq.w #1,plip_tx_q_ct
	st allocate_sema+1
	move.w PLIP_length(a0),d0
	addq.w #PLIP_data,a0
	move.l a0,send_pos_p
	lea 0(a0,d0.w),a0
	move.l a0,send_max_p
	clr.l plip_resend_ct
	move.l #cv_23,mode_vect
	bra cv_ExitRecv

cv_23:
	bmi cv_26
cv_24:
	move.l     send_buf_p(pc),d0
	ble.s      cv_25
	movea.l    d0,a0
	move.l     plip_tx_q(pc),(a0)
	move.l     a0,plip_tx_q
	addq.w     #1,plip_tx_q_ct
	clr.l      send_buf_p
	addq.w     #1,my_port+prt_des_stat_dropped
cv_25:
	pl_set_strobe 1,d0
	pl_set_direct 0,d0
	sf         allocate_sema+1
	move.l     #cv_InitRecv1,mode_vect
	;cmpi.w     #0,busy_flag
	dc.w 0x0c79,0
	dc.l busy_flag
	beq        cv_InitRecv4
	move.l     #cv_InitRecv0,mode_vect
	bra        cv_ExitRecv
cv_26:
	movea.l    send_pos_p(pc),a0
	cmpa.l     send_max_p(pc),a0
	bcc        cv_30
	move.b     (a0),d0
	move.w     plip_resend_ct(pc),d1
	rol.w      #8,d1
	eor.b      d0,d1
	move.b     d1,d0
	lsr.b      #4,d0
	eor.b      d0,d1
	move.b     d1,d0
	lsl.w      #7,d0
	eor.b      d1,d0
	lsl.w      #5,d0
	eor.w      d0,d1
	move.w     d1,plip_resend_ct
	move.b     (a0)+,d0
	move.l     a0,send_pos_p
cv_27:
	subi.b     #$A0,d0
	cmp.b      #$0F,d0
	bls.s      cv_28
	addi.b     #$F1,d0
	pl_set_direct 1,d1
	pl_send_byte d0
	pl_set_strobe 0,d1
	bra        cv_ExitRecv

cv_28:
	addi.b     #$71,d0
	move.b     d0,last_byte
	pl_set_direct 1,d1
	pl_send_byte #0xfb
	pl_set_strobe 0,d1
	move.l #cv_29,mode_vect
	bra        cv_ExitRecv

cv_29:
	bpl        cv_24
	pl_set_direct 1,d1
	pl_send_byte last_byte(pc)
	pl_set_strobe 0,d1
	move.l     #cv_23,mode_vect
	bra        cv_ExitRecv

cv_30:
	move.w plip_resend_ct+2(pc),d0
	addq.w #1,plip_resend_ct+2
	cmp.w      #2,d0
	bcc.s      cv_31
	lea.l      plip_resend_ct(pc),a0
	move.b     0(a0,d0.w),d0
	bra        cv_27

cv_31:
	pl_set_direct 1,d1
	pl_send_byte #0xfc
	pl_set_strobe 0,d1
	move.l     #cv_32,mode_vect
	bra        cv_ExitRecv

cv_32:
	bpl        cv_24
	pl_send_ack d0,d1
	move.l     #cv_33,mode_vect
	bra        cv_ExitRecv

cv_33:
	bmi        cv_24
	cmp.w      #0xfc,d0
	bne        cv_24
	move.l     #cv_34,mode_vect
	bra        cv_ExitRecv

cv_34:
	sf         interrupt_sema
	R_free.i   send_buf_p(pc)
	clr.l      send_buf_p
	sf         allocate_sema+1
	move.w     plip_tx_q_ct(pc),d0
	beq.s      cv_35
	pl_set_direct 1,d1
	pl_send_byte #0xfd
	pl_set_strobe 0,d1
	move.l     #cv_21,mode_vect
	bra        cv_ExitRecv

cv_35:
	pl_send_ack d0,d1
	move.l     #cv_InitRecv1,mode_vect
	bra        cv_ExitRecv
	
	
;ends	cent_busy_XB
;----------------------------------------------------------------------------
;Since TOS can leave garbage in iv_cenbusy at boot, here is a dumb routine
;to reside there when CEN_PLIP is turned 'off'.  It will only 'eat' all the
;incoming 'busy' pulses without responding in any way.

dumb_cenbusy:
	bclr		#0,(hw_imrb)
	move.b		#$FE,(hw_isrb).w		;flag service to hardware
	rte

;----------------------------------------------------------------------------
;End of:	Resident interrupt routines
;----------------------------------------------------------------------------
;	Resident library function code will be expanded here

	make		JAR_links
	make		TCON_links
	make		DOMAIN_links
	_uniref RAM_own
	make		RAM_links

;----------------------------------------------------------------------------
resident_end:
;all beyond that point will be released in going resident
resident_size	equ	resident_end-text_start+$100
;----------------------------------------------------------------------------
;Start of:	STX Non-resident initialization code with tests
;----------------------------------------------------------------------------

start_1:
	move.l		a0,d0
	sne		d7
	bne		.have_basepage
	move.l		4(sp),d0
.have_basepage:
	move.l		d0,a5
	lea		mystack(pc),sp
	move.l		a5,basepage_p
	tst.b		d7
	bne		.ACC_launch

	; gemdos		Mshrink,#0,(a5),#initial_size
	; using the equ (text_size+data_size+bss_size) here is not reliable;
	; seems PASM evaluates them only once during the 1st pass
	move.l #((text_limit-text_start)+(data_limit-data_start)+(bss_limit-bss_start)+$100),-(a7)
	pea.l      (a5)
	move.w     #0,-(a7)
	move.w     #$004A,-(a7)
	trap       #1
	lea.l      12(a7),a7

	lea		bp_arglen(a5),a0
	lea		STinG_Load_s(pc),a1
	str_comp	a0,a1
	bne		.bad_launch

	gemdos		Super,0.w
	move.l		d0,d4

	bsr		ensure_iv_cenbusy

	eval_cookie	#"STiK"
	move.l		d0,d3				;d3 = d0 -> DRV_LIST structure
	gemdos		Super|_ind,d4

	move.l		d3,sting_drivers		;sting_drivers -> DRV_LIST structure
	ble		.STiK_not_found
	move.l		d3,a3				;a3 -> DRV_LIST structure
	lea		DRV_LIST_magic(a3),a0
	lea		STiKmagic_s(pc),a1
	moveq		#10-1,d0
.strcmp_loop:					;loop to test STiKmagic of DRV_LIST
	cmpm.b		(a0)+,(a1)+
	dbne		d0,.strcmp_loop
	bne		.STiK_not_valid

	move.l		DRV_LIST_get_dftab(a3),a0	;a0 -> get_dftab function
	pea		TRANSPORT_DRIVER_s		;-(sp) = "TRANSPORT_TCPIP"
	jsr		(a0)				;call get_dftab
	addq		#4,sp
	move.l		d0,tpl				;store pointer in 'tpl'
	ble		.driver_not_valid

	move.l		DRV_LIST_get_dftab(a3),a0	;a0 -> get_dftab function
	pea		MODULE_DRIVER_s			;-(sp) = "MODULE_LAYER"
	jsr		(a0)				;call get_dftab
	addq		#4,sp
	move.l		d0,stx				;store pointer in 'tpl'
	ble		.layer_not_valid
.install:
	link		a6,#-8
	query_chains	-8(a6),-4(a6),0.w
	movem.l		-8(a6),a3/a4			;a3->ports  a4->drivers
	unlk		a6

	move.l		a3,d0
	beq		.bad_port_list
.port_loop:
	move.l		d0,a3
	move.l		prt_des_next(a3),d0
	bne		.port_loop

	move.l		a4,d0
	beq		.bad_driver_list
.driver_loop:
	move.l		d0,a4
	move.l		drv_des_next(a4),d0
	bne		.driver_loop

.IF	USE_TIMER
	TIMER_call	my_timer_func(pc),#HNDLR_SET
	tst		d0
	beq		.timer_failure
.ENDIF	USE_TIMER

.final_install:
	move.l		#my_port,prt_des_next(a3)
	move.l		#my_driver,drv_des_next(a4)

	gemdos		Super,0.w
	move.l		d0,d4
	RAM_own #1
	RAM_set	resident_end(pc),#RAM_chunk+2042
	gemdos		Super|_ind,d4

	;gemdos		Ptermres,#resident_size,#0
	move.w #0,-(a7)
	move.l #resident_size+RAM_chunk+1556,-(a7)
	move.w #0x31,-(a7)
	trap #1

;-------------------------------------

.ACC_launch:
	lea		ACC_launch_s(pc),a0
	bsr		report_error
.loop:
	bra		.loop

;-------------------------------------

.bad_launch:
	lea		bad_launch_s(pc),a0
	bra		.error_exit

;-------------------------------------

.bad_port_list:
	lea		bad_port_list_s(pc),a0
	bra		.error_exit

;-------------------------------------

.bad_driver_list:
	lea		bad_driver_list_s(pc),a0
	bra		.error_exit

;-------------------------------------

.IF	USE_TIMER
.timer_failure:
	lea		timer_failure_s(pc),a0
	bra		.error_exit
.ENDIF	USE_TIMER

;-------------------------------------

.STiK_not_found:
	lea		STiK_not_found_s,a0
	bra		.error_exit

;-------------------------------------

.STiK_not_valid:
	lea		STiK_not_valid_s,a0
	bra		.error_exit

;-------------------------------------

.driver_not_valid:
	lea		driver_not_valid_s,a0
	bra		.error_exit

;-------------------------------------
.layer_not_valid:
	lea		layer_not_valid_s(pc),a0
.error_exit:
	bsr.s		report_error
	;gemdos		Pterm,#-1
	move.w #-1,-(a7)
	move.w #0x4c,-(a7)
	trap #1

;-------------------------------------

ensure_iv_cenbusy:
	move		sr,d2				;d2 = entry SR
	or		#$0700,sr			;disable interrupts
	move.l		(ev_buserr).w,a1		;save ev_buserr
	move.l		(ev_adrerr).w,a2		;save ev_adrerr
	move.l		#bad_iv_cenbusy,(ev_buserr).w	;patch ev_adrerr
	move.l		#bad_iv_cenbusy,(ev_adrerr).w	;patch ev_adrerr
	move.l		(iv_cenbusy).w,a0		;a0 = suspect pointer
	move.l		sp,d1				;d1 = entry SP
	move.l		(a0),d0				;test pointer ?
good_iv_cenbusy:
	moveq		#E~OK,d0		;flag good pointer
	bra		know_iv_cenbusy		;go to common restore code

bad_iv_cenbusy:		;bus error or address error leads here !!!
	move.l		d1,SP			;SP = entry SP  (pop big frame)
	moveq		#E~ERROR,d0		;flag bad pointer
know_iv_cenbusy:
	move.l		a1,(ev_buserr).w	;restore ev_buserr
	move.l		a2,(ev_adrerr).w	;restore ev_adrerr
	move		d2,sr			;SR = entry SR
	tst.l		d0			;remember test result
	beq		.exit			;just exit if pointer is good
.patch_vector:
	move.l		#dumb_cenbusy,(iv_cenbusy).w	;patch iv_cenbusy
.exit:
	rts

	

;-------------------------------------

report_error:
	move.l		a0,-(sp)
	lea		error_title_s(pc),a0
	bsr.s	Cconws_sub
	move.l		(sp)+,a0
	bsr.s	Cconws_sub
	lea		error_tail_s(pc),a0
Cconws_sub:
	gemdos		Cconws,(a0)
	rts

;----------------------------------------------------------------------------
;End of:	STX Non-resident initialization code with tests
;----------------------------------------------------------------------------
;	Non-resident library function code will be expanded here

	make	JAR_links
	make	TCON_links
	make	DOMAIN_links
	make		RAM_links

;----------------------------------------------------------------------------

text_limit:
text_size	= text_limit-text_start
	.data
data_start:

;----------------------------------------------------------------------------

STinG_Load_s:
	dc.b	10,'STinG_Load',NUL
STiKmagic_s:
	dc.b	'STiKmagic',NUL
TRANSPORT_DRIVER_s:
	dc.b	'TRANSPORT_TCPIP',NUL
MODULE_DRIVER_s:
	dc.b	'MODULE_LAYER',NUL

ACC_launch_s:
	dc.b	'This non-ACC, was launched as an ACC,',CR,LF
	dc.b	'so now you must reset the computer !',CR,LF
	dc.b	'I am looping forever to avoid damage',CR,LF
	dc.b	'that could occur if I try to exit !',CR,LF
	dc.b	NUL

bad_launch_s:	
	dc.b	'This STX should only be launched by',CR,LF
	dc.b	'STinG, or another TCP/IP stack with',CR,LF
	dc.b	'a compatible module interface !',CR,LF
	dc.b	NUL

bad_port_list_s:
	dc.b	'The list chain of STinG ports was',CR,LF
	dc.b	'not found...!',CR,LF
	dc.b	NUL

bad_driver_list_s:
	dc.b	'The list chain of STinG drivers was',CR,LF
	dc.b	'not found...!',CR,LF
	dc.b	NUL

.IF	USE_TIMER
timer_failure_s:
	dc.b	'STinG TIMER_call function failed !',CR,LF
	dc.b	NUL
.ENDIF	USE_TIMER

STiK_not_found_s:
	dc.b	'There is no STiK cookie in the jar !',CR,LF
	dc.b	NUL

STiK_not_valid_s:
	dc.b	'The STiK cookie data is corrupt !',CR,LF
	dc.b	NUL

driver_not_valid_s:
	dc.b	'The main STinG driver is not valid !',CR,LF
	dc.b	NUL

layer_not_valid_s:
	dc.b	'The STinG module layer is not valid !',CR,LF
	dc.b	NUL

error_title_s:
	dc.b	BEL,CR,LF
	dc.b	'------------'
	M_TITLE
	dc.b	' '
	M_VERSION
	dc.b	'------------',CR,LF
	dc.b	NUL

error_tail_s:
	dc.b	BEL,CR,LF,NUL

;----------------------------------------------------------------------------
	.even
data_limit:
data_size	equ	data_limit-data_start
	.bss
bss_start:
;----------------------------------------------------------------------------

		ds.l	200		;subroutine stack >= 100 longs
mystack:	ds.l	1		;top of subroutine stack

;----------------------------------------------------------------------------
	ds.b RAM_chunk

bss_limit:
bss_size	equ	bss_limit-bss_start
;----------------------------------------------------------------------------
initial_size	equ	text_size+data_size+bss_size+$100
;----------------------------------------------------------------------------
