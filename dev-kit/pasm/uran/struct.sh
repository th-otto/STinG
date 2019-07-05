;----------------------------------------------------------------------------
; File name:	STRUCT.SH			Revision date:	1998.06.10
; Authors:	Ronald Andersson		Creation date:	1997.03.26
;(c)1997 by:	Ulf Ronald Andersson		All rights reserved
;Released as:	FREEWARE			(commercial sale forbidden)
;----------------------------------------------------------------------------
; Purpose:	Header file included in assembly sources to define
;		macros that declare data structures, similar to C.
;----------------------------------------------------------------------------
;Required header declarations:
;
;	include	"uran\struct.sh"
;
;----------------------------------------------------------------------------
;	Macro definitions for general data structure definitions
;
.MACRO	d__s.size	name,count,dummy
.IFNB	dummy
.ERROR	"Extra arg in: d__s&size &name,&count,&dummy"
.EXITM
.ENDIF
.IFB	name
.ERROR	"Missing args in: d__s&size &name,&count"
.EXITM
.ENDIF
name	equ	st__pos
.IFB	count
.SET	st__cnt,1
.ELSE
.SET	st__cnt,count
.ENDIF
.IF	(('&.size' == '.b') | ('&.size' == '.B'))
.SET	st__exp,0
.ELSE
.IF	(('&.size' == '.w') | ('&.size' == '.W'))
.SET	st__exp,1
.ELSE
.IF	(('&.size' == '.l') | ('&.size' == '.L'))
.SET	st__exp,2
.ELSE
.SET	st__exp,0
.ERROR	"Bad size in: d__s&size &name,&count"
.ENDIF
.ENDIF
.ENDIF
.SET	st__pos,st__pos+st__cnt<<st__exp
.ENDM	d__s
;
.MACRO	d_start	name,base
.SET	st__pos,base
name	equ	st__pos
.ENDM	d_start
;
.MACRO	struct	name
.SET	st__pos,0
name	equ	st__pos
.ENDM	struct
;
.MACRO	d_b	name,count
	d__s.b	name,count
.ENDM	d_b
;
.MACRO	d_even
.IF	(st__pos & 1)
.SET	st__pos,st__pos+1
.ENDIF
.ENDM	d_even
;
.MACRO	d_w	name,count
	d_even
	d__s.w	name,count
.ENDM	d_w
;
.MACRO	d_ow	name,count
	d__s.w	name,count
.ENDM	d_ow
;
.MACRO	d_l	name,count
	d_even
	d__s.l	name,count
.ENDM	d_l
;
.MACRO	d_ol	name,count
	d__s.l	name,count
.ENDM	d_ol
;
.MACRO	d_s	name,count
	d__s.b	name,count
.ENDM	d_s
;
.MACRO	d_field	name,len
name	equ st__pos
st__pos	=	st__pos+(len+7)/8
field_index	=	len
.ENDM
;
.MACRO	d_bits	name,len
field_index	=	field_index-len
name		equ	field_index
mask_&name	equ	(1<<len)-1
amask_&name	equ	mask_&name<<field_index
	.if	(field_index < 0)
	.error	"bad bit field definition: name,len"
	.endif
.ENDM
;
.MACRO	d_alias	name
name	equ	st__pos
.ENDM
;
.MACRO	d_end	name
sizeof_&name	equ	st__pos
.ENDM
;
;----------------------------------------------------------------------------
;	Macros for various data types
;
.MACRO	byte	name,count
	d__s.b	name,count
.ENDM
;
.MACRO	char	name,count
	d__s.b	name,count
.ENDM
;
.MACRO	uint8	name,count
	d__s.b	name,count
.ENDM
;
.MACRO	int8	name,count
	d__s.b	name,count
.ENDM
;
.MACRO	word	name,count
	d_even
	d__s.w	name,count
.ENDM
;
.MACRO	uint16	name,count
	d_even
	d__s.w	name,count
.ENDM
;
.MACRO	int16	name,count
	d_even
	d__s.w	name,count
.ENDM
;
.MACRO	uint32	name,count
	d_even
	d__s.l	name,count
.ENDM
;
.MACRO	int32	name,count
	d_even
	d__s.l	name,count
.ENDM
;
.MACRO	uint8_p	name,count
	d_even
	d__s.l	name,count
.ENDM
;
.MACRO	int8_p	name,count
	d_even
	d__s.l	name,count
.ENDM
;
.MACRO	uint16_p	name,count
	d_even
	d__s.l	name,count
.ENDM
;
.MACRO	int16_p	name,count
	d_even
	d__s.l	name,count
.ENDM
;
.MACRO	long	name,count
	d_even
	d__s.l	name,count
.ENDM
;
.MACRO	uint32_p	name,count
	d_even
	d__s.l	name,count
.ENDM
;
.MACRO	int32_p	name,count
	d_even
	d__s.l	name,count
.ENDM
;
.MACRO	void_p	name,count
	d_even
	d__s.l	name,count
.ENDM
;
.MACRO	char_p	name,count
	d_even
	d__s.l	name,count
.ENDM
;
.MACRO	long_p	name,count
	d_even
	d__s.l	name,count
.ENDM
;
.MACRO	func_p	name,count
	d_even
	d__s.l	name,count
.ENDM
;
.MACRO	struct_p	name,count
	d_even
	d__s.l	name,count
.ENDM
;
.MACRO	struc_p	name,count
	d_even
	d__s.l	name,count
.ENDM
;
;----------------------------------------------------------------------------
; End of file:	STRUCT.SH
;----------------------------------------------------------------------------
