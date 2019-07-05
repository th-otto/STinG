;----------------------------------------------------------------------------
; File name:	PORT.SH				Revision date:	1998.09.08
; Authors:	P.Rottengatter & R.Andersson	Creation date:	1997.08.10
;----------------------------------------------------------------------------
; Purpose:	Defines port interfacing for STinG
;----------------------------------------------------------------------------
;Required header declarations:
;
;	.include	"uran\struct.sh"
;	.include	"sting\port.sh"
;
;copy the above to the header of your program and 'uncomment' the includes
;----------------------------------------------------------------------------
;	Internal port descriptor.
;
struct	prt_des
	char_p		prt_des_name		;Name of port
	int16		prt_des_type		;Type of port
	int16		prt_des_active		;Flag for port active or not
	uint32		prt_des_flags		;Type dependent operational flags
	uint32		prt_des_ip_addr		;IP address of this network adapter
	uint32		prt_des_sub_mask	;Subnet mask of attached network
	int16		prt_des_mtu		;Maximum packet size to go through
	int16		prt_des_max_mtu		;Maximum allowed value for mtu
	int32		prt_des_stat_sd_data	;Statistics of sent data
	struc_p		prt_des_send		;Link to first entry in send queue
	int32		prt_des_stat_rcv_data	;Statistics of received data
	struc_p		prt_des_receive		;Link to first entry in receive queue
	int16		prt_des_stat_dropped	;Statistics of dropped datagrams
	struc_p		prt_des_driver		;Driver descriptor to handle this port
	struc_p		prt_des_next		;Next port in port chain
d_end	prt_des
;----------------------------------------------------------------------------
;	Port driver descriptor.
;
struct	drv_des
	func_p		drv_des_set_state	;Setup and shutdown port
	func_p		drv_des_cntrl		;Port control function dispatcher
	func_p		drv_des_send		;Send pending packets
	func_p		drv_des_receive		;Receive pending packets
	char_p		drv_des_name		;Name of driver
	char_p		drv_des_version		;Version of driver in "xx.yy" format
	uint16		drv_des_date		;Compile date in GEMDOS format
	char_p		drv_des_author		;Name of programmer
	struc_p		drv_des_next		;Next driver in driver chain
	struc_p		drv_des_basepage	;Basepage of defining module (or kernel)
d_end	drv_des
;----------------------------------------------------------------------------
;	Link Type Definitions
;
L_INTERNAL	equ	0	;Internal pseudo port
L_SER_PTP	equ	1	;  Serial point to point type link
L_PAR_PTP	equ	2	;Parallel point to point type link
L_SER_BUS	equ	3	;  Serial            bus type link
L_PAR_BUS	equ	4	;Parallel            bus type link
L_SER_RING	equ	5	;  Serial           ring type link
L_PAR_RING	equ	6	;Parallel           ring type link
L_MASQUE	equ	7	;Masquerade pseudo port
;----------------------------------------------------------------------------
; End of file:	PORT.SH
;----------------------------------------------------------------------------
