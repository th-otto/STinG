;----------------------------------------------------------------------------
; File name:	UDP.SH				Revision date:	1999.10.26
; Authors:	Peter Rottengatter & RA		Creation date:	1997.05.26
;----------------------------------------------------------------------------
; Purpose:	High level StinG UDP protocol
;		Header file included in assembly
;----------------------------------------------------------------------------
;needs:
;	"uran\struct.sh"
;	"sting\layer.sh"
;----------------------------------------------------------------------------
;	UDP header and chain link structure.
;----------------------------------------------------------------------------
struct	UDP_hdr
	uint16	UDP_hdr_source_port	;Source UDP port
	uint16	UDP_hdr_dest_port	;Destination UDP port
	uint16	UDP_hdr_length		;UDP length of data
	uint16	UDP_hdr_checksum	;UDP checksum
d_end	UDP_hdr
;
struct	UDP_blk
	struc_p	UDP_blk_next		;Link to next data block in chain
	uint16	UDP_blk_length		;Amount of data in this block
	uint16	UDP_blk_index		;Index to data start in this block
d_end	UDP_blk
;----------------------------------------------------------------------------
;	UDP connection structure.
;----------------------------------------------------------------------------
struct	CONN
	uint32	CONN_remote_IP_address;	;Foreign socket IP address
	uint16	CONN_remote_port;	;Foreign socket port number
	uint32	CONN_local_IP_address;	;Local socket IP address
	uint16	CONN_local_port;	;Local socket port number
	uint8	CONN_state;		;connection state (1==ULISTEN)
	uint8	CONN_flags;		;special flags for connection
	int16	CONN_ttl;		;Time To Live		(for IP)
	uint32	CONN_total_data;	;Total real data in queue
	struc_p	CONN_info;		;Connection information CIB link
	int16	net_error;		;Error to be reported with next call
	struc_p	CONN_receive_queue;	;UDP_blk Receive queue
	struc_p	CONN_pending;		;UDP_hdr Pending IP datagrams
	int16	CONN_semaphore;		;Semaphore for locking structures
	uint32	CONN_last_work;		;Last time work has been done
	struc_p	CONN_next;		;Link to next connection in chain
d_end	CONN
;----------------------------------------------------------------------------
;	UDP connection states
;----------------------------------------------------------------------------
;ULISTEN		equ	1
;UESTABLISH	equ	4
;----------------------------------------------------------------------------
;	Values for flags in CONNEC
;----------------------------------------------------------------------------
;DEFER		equ	1
;----------------------------------------------------------------------------
; End of file:	UDP.SH
;----------------------------------------------------------------------------
