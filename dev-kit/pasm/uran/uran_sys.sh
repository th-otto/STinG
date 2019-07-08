;----------------------------------------------------------------------------
;File name:	URAn_SYS.SH			Revision date:	1999.11.05
;Creator:	Ulf Ronald Andersson		Creation date:	1992.11.15
;(c)1992 by:	Ulf Ronald Andersson		All rights reserved
;Released as:	FREEWARE			(commercial sale forbidden)
;----------------------------------------------------------------------------
;Purpose:	Main macro & symbol library for Atari systems
;----------------------------------------------------------------------------
;Required header declarations:
;
;	include	"uran\struct.sh"
;	include	"uran\uran_sys.sh"
;
;----------------------------------------------------------------------------
;
	.super		;allow supervisor instructions
;
;----------------------------------------------------------------------------
;	The following 10 Macros implement assembly time indirection
;	and allow indexed assemblytime constant and variable arrays,
;	as well as many other creative assemblytime 'tricks'.
;
;	ev_isym	dest,prefix,suffix	=> dest = prefixXXXX
;	sv_isym	prefix,suffix,value	=> prefixXXXX = value
;	sc_isym	prefix,suffix,value	=> prefixXXXX equ value
;	sl_isym	prefix,suffix		=> prefixXXXX: label defined
;	op_isym	opmnem,prefix,suffix	=> opmnem prefixXXXX
;
;	Above "XXXX" represents whatever hex digits the 'suffix' parameter
;	produces as the expression is evaluated, thus modifying the symbol.
;
;	The other four macros are purely internal submacros
;
.MACRO	ev_isym	dest_sym,prefix,suffix
	sub_ev_isym	dest_sym,prefix,$(suffix)
.ENDM
;
.MACRO	sub_ev_isym	dest_sym,prefix,hexnum
dest_sym	=	prefix&hexnum
.ENDM
;
.MACRO	sv_isym	prefix,suffix,value
	sub_sv_isym	value,prefix,$(suffix)
.ENDM
;
.MACRO	sub_sv_isym	value,prefix,hexnum
prefix&hexnum	=	value
.ENDM
;
;NB: Argument reordering of the submacros of sc_isym and sv_isym was needed !
;    This is due to a bug in PASM !  (numeric expansion terminates line)
;
.MACRO	sc_isym	prefix,suffix,value
	sub_sc_isym	value,prefix,$(suffix)
.ENDM
;
.MACRO	sub_sc_isym	value,prefix,hexnum
prefix&hexnum	equ	value
.ENDM
;
.MACRO	sl_isym	prefix,suffix
	sub_sl_isym	prefix,$(suffix)
.ENDM	sl_isym
;
.MACRO	sub_sl_isym	prefix,hexnum
prefix&hexnum:
.ENDM	sub_sl_isym
;
.MACRO	op_isym	opmnem,prefix,suffix
	sub_op_isym	opmnem,prefix,$(suffix)
.ENDM	op_isym
;
.MACRO	sub_op_isym	opmnem,prefix,hexnum
	&opmnem	prefix&hexnum
.ENDM	sub_op_isym
;
.MACRO	op_isym_21	opmnem,prefix,suffix,arg2
	sub_op_isym_21	opmnem,arg2,prefix,$(suffix)
.ENDM	op_isym_21
;
.MACRO	sub_op_isym_21	opmnem,arg2,prefix,hexnum
	&opmnem	prefix&hexnum,arg2
.ENDM	sub_op_isym_21
;
.MACRO	op_isym_22	opmnem,arg1,prefix,suffix
	sub_op_isym_22	opmnem,arg1,prefix,$(suffix)
.ENDM	op_isym_22
;
.MACRO	sub_op_isym_22	opmnem,arg1,prefix,hexnum
	&opmnem	arg1,prefix&hexnum
.ENDM	sub_op_isym_22
;
;NB: The two macros op_isym_2? are for ops with 2 args, with the '_21'
;    variant for 'indexing' the first arg and the '_22' variant to do
;    do it for the second arg.
;
;----------------------------------------------------------------------------
;The 'uni__v' symbol should be used by macros, when generating label
;groups unique to each entry to such a macro.  It should be incremented after
;use, and is intended as suffix argument to the ..._isym macros above.
;NB: using a common symbol for this purpose increases uniqueness integrity,
;    but care must be taken when submacros also use it.
;
uni__v	=	0
;
;----------------------------------------------------------------------------
;	Macros to ease porting of C functions to assembly code.
;	These implement simple declaration of local arguments & variables.
;
lv_grp	=	0
;
.MACRO	lv_init	areg
lv_grp	=	lv_grp+1
.IF	('&areg'-'##')
	op_isym_22	link,areg,#lv_bot_,lv_grp
lv_top	= 8
lv_bot	= 0
.ELSE
lv_top	= 4
lv_bot	= 0
.ENDIF
.ENDM	lv_init
;
.MACRO	lv_exit	areg
	sc_isym	lv_top_,lv_grp,lv_top
	sc_isym	lv_bot_,lv_grp,lv_bot
.IF	('&areg'-'##')
	unlk	areg
.ENDIF
.ENDM	lv_exit
;
.MACRO	lvar.size	name,count
.IFNB	count
	res_lva.size	lv_bot,-count
.ELSE
	res_lva.size	lv_bot,-1
.ENDIF
name	=	lv_bot
.ENDM	lvar
;
.MACRO	larg.size	name,count
name	=	lv_top
.IFNB	count
	res_lva.size	lv_top,count
.ELSE
	res_lva.size	lv_top,1
.ENDIF
.ENDM	larg
;
.MACRO	res_lva.size	name,count
.IF	(('&.size' == '.b') | ('&.size' == '.B'))
name	=	name+(count)
.ELSE
.IF	(('&.size' == '.w') | ('&.size' == '.W'))
name	=	name+((count)<<1)
.ELSE
.IF	(('&.size' == '.l') | ('&.size' == '.L'))
name	=	name+((count)<<2)
.ELSE
	.ERROR	"Bad size for macro res_lva &name,&count"
.ENDIF
.ENDIF
.ENDIF
.ENDM	res_lva
;
;----------------------------------------------------------------------------
;	Three macros to implement CDECL argument handling for submacro
;	subroutine calls.  Two are intended as submacros of others.
;	A maximum of 16 arguments can be handled.
;
.MACRO	CDECL_args.mode	arg_flags,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16
CDECL_arg_size	=	0
CDECL_arg_count	=	0
	.IFNB	v1
	.IFNB	v2
	.IFNB	v3
	.IFNB	v4
	.IFNB	v5
	.IFNB	v6
	.IFNB	v7
	.IFNB	v8
	.IFNB	v9
	.IFNB	v10
	.IFNB	v11
	.IFNB	v12
	.IFNB	v13
	.IFNB	v14
	.IFNB	v15
	.IFNB	v16
	sub_CDECL_arg.mode	arg_flags>>30,v16
	.ENDIF	v16
	sub_CDECL_arg.mode	arg_flags>>28,v15
	.ENDIF	v15
	sub_CDECL_arg.mode	arg_flags>>26,v14
	.ENDIF	v14
	sub_CDECL_arg.mode	arg_flags>>24,v13
	.ENDIF	v13
	sub_CDECL_arg.mode	arg_flags>>22,v12
	.ENDIF	v12
	sub_CDECL_arg.mode	arg_flags>>20,v11
	.ENDIF	v11
	sub_CDECL_arg.mode	arg_flags>>18,v10
	.ENDIF	v10
	sub_CDECL_arg.mode	arg_flags>>16,v9
	.ENDIF	v9
	sub_CDECL_arg.mode	arg_flags>>14,v8
	.ENDIF	v8
	sub_CDECL_arg.mode	arg_flags>>12,v7
	.ENDIF	v7
	sub_CDECL_arg.mode	arg_flags>>10,v6
	.ENDIF	v6
	sub_CDECL_arg.mode	arg_flags>>8,v5
	.ENDIF	v5
	sub_CDECL_arg.mode	arg_flags>>6,v4
	.ENDIF	v4
	sub_CDECL_arg.mode	arg_flags>>4,v3
	.ENDIF	v3
	sub_CDECL_arg.mode	arg_flags>>2,v2
	.ENDIF	v2
	sub_CDECL_arg.mode	arg_flags,v1
	.ENDIF	v1
.ENDM	CDECL_arg
;
;-------------------------------------
;
.MACRO	CDECL_cleanargs	function,arg_count
	.IF	CDECL_arg_size
	.IF	CDECL_arg_size>8
	lea	CDECL_arg_size(sp),sp
	.ELSE
	addq	#CDECL_arg_size,sp
	.ENDIF
	.ENDIF
	.IF	arg_count!=CDECL_arg_count
	.ERROR	"&function needs another number of arguments"
	.ENDIF
.ENDM	CDECL_cleanargs
;
;-------------------------------------
;
.MACRO	sub_CDECL_arg.mode	flags,arg
CDECL_arg_count	=	CDECL_arg_count+1
	.IF	(flags & 3)==3	;pointer arg ?
CDECL_arg_size	=	CDECL_arg_size+4
	.IF	'&.mode'=='.i'	;indirection mode ?
	move.l	arg,-(sp)
	.ELSE	not indirection
	pea	arg
	.ENDIF	indirection
	.ELSE	not pointer arg
	.IF	(flags & 3)==2	;long arg ?
CDECL_arg_size	=	CDECL_arg_size+4
	move.l	arg,-(sp)
	.ELSE	not long arg
	.IF	(flags & 3)==1	;word arg ?
CDECL_arg_size	=	CDECL_arg_size+2
	move.w	arg,-(sp)
	.ELSE	not word arg
CDECL_arg_size	=	CDECL_arg_size+2
	move.b	arg,-(sp)
	.ENDIF
	.ENDIF
	.ENDIF
.ENDM	sub_CDECL_arg
;
;----------------------------------------------------------------------------
;	Six macros to implement PUREC argument handling for submacro
;	subroutine calls.  Two are intended as submacros of others.
;	A maximum of 16 arguments can be handled.
;
.MACRO	PUREC_args.mode	arg_flags,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16
PUREC_arg_size	=	0
PUREC_arg_count	=	0
PUREC_areg_used	=	0
PUREC_dreg_used	=	0
	.IFNB	v1
	sub_PUREC_type	1,arg_flags
	.IFNB	v2
	sub_PUREC_type	2,arg_flags>>2
	.IFNB	v3
	sub_PUREC_type	3,arg_flags>>4
	.IFNB	v4
	sub_PUREC_type	4,arg_flags>>6
	.IFNB	v5
	sub_PUREC_type	5,arg_flags>>8
	.IFNB	v6
	sub_PUREC_type	6,arg_flags>>10
	.IFNB	v7
	sub_PUREC_type	7,arg_flags>>12
	.IFNB	v8
	sub_PUREC_type	8,arg_flags>>14
	.IFNB	v9
	sub_PUREC_type	9,arg_flags>>16
	.IFNB	v10
	sub_PUREC_type	10,arg_flags>>18
	.IFNB	v11
	sub_PUREC_type	11,arg_flags>>20
	.IFNB	v12
	sub_PUREC_type	12,arg_flags>>22
	.IFNB	v13
	sub_PUREC_type	13,arg_flags>>24
	.IFNB	v14
	sub_PUREC_type	14,arg_flags>>26
	.IFNB	v15
	sub_PUREC_type	15,arg_flags>>28
	.IFNB	v16
	sub_PUREC_type	16,arg_flags>>30
	sub_PUREC_arg.mode	16,v16
	.ENDIF	v16
	sub_PUREC_arg.mode	15,v15
	.ENDIF	v15
	sub_PUREC_arg.mode	14,v14
	.ENDIF	v14
	sub_PUREC_arg.mode	13,v13
	.ENDIF	v13
	sub_PUREC_arg.mode	12,v12
	.ENDIF	v12
	sub_PUREC_arg.mode	11,v11
	.ENDIF	v11
	sub_PUREC_arg.mode	10,v10
	.ENDIF	v10
	sub_PUREC_arg.mode	9,v9
	.ENDIF	v9
	sub_PUREC_arg.mode	8,v8
	.ENDIF	v8
	sub_PUREC_arg.mode	7,v7
	.ENDIF	v7
	sub_PUREC_arg.mode	6,v6
	.ENDIF	v6
	sub_PUREC_arg.mode	5,v5
	.ENDIF	v5
	sub_PUREC_arg.mode	4,v4
	.ENDIF	v4
	sub_PUREC_arg.mode	3,v3
	.ENDIF	v3
	sub_PUREC_arg.mode	2,v2
	.ENDIF	v2
	sub_PUREC_arg.mode	1,v1
	.ENDIF	v1
.ENDM	PUREC_arg
;
;-------------------------------------
;
.MACRO	PUREC_cleanargs	function,arg_count
	.IF	PUREC_arg_size
	.IF	PUREC_arg_size>8
	add	#PUREC_arg_size,sp
	.ELSE	not size>8
	addq	#PUREC_arg_size,sp
	.ENDIF	size>8
	.ENDIF	size
	.IF	arg_count!=PUREC_arg_count
	.ERROR	"&function needs another number of arguments"
	.ENDIF	count
.ENDM	PUREC_cleanargs
;
;-------------------------------------
;
.MACRO	sub_PUREC_type	index,flags
	.IF	(flags & 3)==3	;pointer arg ?
	.IF	PUREC_areg_used<2
	.IF	PUREC_areg_used<1
PUREC_type_&index	=	$30
	.ELSE
PUREC_type_&index	=	$31
	.ENDIF
PUREC_areg_used	=	PUREC_areg_used+1
	.ELSE
PUREC_type_&index	=	$3F
	.ENDIF
	.ELSE	not pointer
PUREC_type_&index	=	((flags) & 3)<<4
	.IF	PUREC_dreg_used<3
	.IF	PUREC_dreg_used<2
	.IF	PUREC_dreg_used<1
PUREC_type_&index	=	0+PUREC_type_&index
	.ELSE
PUREC_type_&index	=	1+PUREC_type_&index
	.ENDIF
	.ELSE
PUREC_type_&index	=	2+PUREC_type_&index
	.ENDIF
PUREC_dreg_used	=	PUREC_dreg_used+1
	.ELSE
PUREC_type_&index	=	$F+PUREC_type_&index
	.ENDIF
	.ENDIF
.ENDM	sub_PUREC_type
;
;-------------------------------------
;
.MACRO	sub_PUREC_arg.mode	index,arg
PUREC_arg_count	=	PUREC_arg_count+1
	.IF	(PUREC_type_&index)>$2F	;pointer arg ?
	.IF	(PUREC_type_&index & 15)<2
	.IF	(PUREC_type_&index & 15)<1
	sub_PUREC_ptr.mode	arg,a0,PURE
	.ELSE
	sub_PUREC_ptr.mode	arg,a1,PURE
	.ENDIF
	.ELSE
PUREC_arg_size	=	PUREC_arg_size+4
	sub_PUREC_ptr.mode	arg,-(sp),CDEC
	.ENDIF
	.ELSE
	.IF	(PUREC_type_&index & 15)<3
	.IF	(PUREC_type_&index & 15)<2
	.IF	(PUREC_type_&index & 15)<1
	sub_PUREC_data.mode	(PUREC_type_&index >>4),arg,d0,PURE
	.ELSE
	sub_PUREC_data.mode	(PUREC_type_&index >>4),arg,d1,PURE
	.ENDIF
	.ELSE
	sub_PUREC_data.mode	(PUREC_type_&index >>4),arg,d2,PURE
	.ENDIF
	.ELSE
	sub_PUREC_data.mode	(PUREC_type_&index >>4),arg,-(sp),CDEC
PUREC_arg_size	=	PUREC_arg_size+PUREC_inc_size
	.ENDIF
	.ENDIF
.ENDM	sub_PUREC_arg
;
;-------------------------------------
;
.MACRO	sub_PUREC_data.mode	flags,arg,dest,method
	.IF	(flags & 3)==2	;long arg ?
PUREC_inc_size	=	4
	move.l	arg,dest
	.ELSE	not long arg
PUREC_inc_size	=	2
	.IF	(flags & 3)==1	;word arg ?
	move.w	arg,dest
	.ELSE	not word arg
	move.b	arg,dest
	.ENDIF	word
	.ENDIF	long
.ENDM	sub_PUREC_data
;
;-------------------------------------
;
.MACRO	sub_PUREC_ptr.mode	arg,dest,method
	.IF	'&.mode'=='.i'	;indirection mode ?
	move.l	arg,dest
	.ELSE	not indirection
	.IF	'&method'=='CDEC'	;stacking ?
	pea	arg
	.ELSE	not stacking
	lea	arg,dest
	.ENDIF	stacking
	.ENDIF	indirection
.ENDM	sub_PUREC_ptr
;
;----------------------------------------------------------------------------
;	Four macros to call subroutines and library functions with arguments
;
.MACRO	CDECL_sub.mode	subroutine,arg_count,arg_flags,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16
	CDECL_args.mode	arg_flags,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16
	jsr	subroutine
	CDECL_cleanargs	subroutine,arg_count
.ENDM	CDECL_sub
;
;-------------------------------------
;
.MACRO	PUREC_sub.mode	subroutine,arg_count,arg_flags,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16
	PUREC_args.mode	arg_flags,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16
	jsr	subroutine
	PUREC_cleanargs	subroutine,arg_count
.ENDM	PUREC_sub
;
;-------------------------------------
;
.MACRO	CDECL_func.mode	function,arg_count,arg_flags,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16
	CDECL_args.mode	arg_flags,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16
	_uniref	function
	jsr	code_&function
	CDECL_cleanargs	function,arg_count
.ENDM	CDECL_func
;
;-------------------------------------
;
.MACRO	PUREC_func.mode	function,arg_count,arg_flags,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16
	PUREC_args.mode	arg_flags,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16
	_uniref	function
	jsr	code_&function
	PUREC_cleanargs	function,arg_count
.ENDM	PUREC_func
;
;----------------------------------------------------------------------------
;	Macro to XDEF labels only at requested debugging levels
;needs:	'_deblev' set to the debugging level before macro use
;
_deblev	=	0
;
.MACRO	_debdef	symbol,levlim
	.IF	_deblev >= levlim
	XDEF	symbol
	.ENDIF
.ENDM	_debdef
;
;----------------------------------------------------------------------------
;	The following macros implement module linking
;	on assembly source level of the code
;
;
.MACRO	_unidec	sym
_uni_ct_&sym	=	0
_uni_df_&sym	=	0
.ENDM	_unidec
;
;
.MACRO	_uniref	sym
_uni_ct_&sym	=	1+_uni_ct_&sym
.ENDM	_uniref
;
.MACRO	_unidef	v1,v2,v3,v4,v5,v6,v7,v8,v9,va
	ifeq	_uni_df_&v1
	ifne	_uni_ct_&v1
_uni_df_&v1	=	1
_uni_flag	=	1
code_&v1:
	code_&v1
	endif
	endif
	ifnb	v2
	_unidef	v2,v3,v4,v5,v6,v7,v8,v9,va
	endif
.ENDM	_unidef
;
;
.MACRO	make	v1,v2,v3,v4,v5,v6,v7,v8,v9,va
_uni_flag	=	0
		&v1
		ifne	_uni_flag
		make	v1
		endif
		ifnb	v2
		make	v2,v3,v4,v5,v6,v7,v8,v9,va
		endif
.ENDM	make
;
;----------------------------------------------------------------------------
;	Macro to create an exception frame on supervisor stack
;
.MACRO	push_ex.md	v1,v2
	tst	(_longframe).w
	op_isym	beq.s,.cpu_adapted,uni__v
	clr	-(sp)
sl_isym	.cpu_adapted,uni__v
	.IF	'&.md'=='.i'	;new style indirection
	move.l	v1,-(sp)
	.ELSE
	pea	v1
	.ENDIF	mode
	.IFNB	v2
	.ERROR	"Too many 'push_ex' arguments: &v1,&v2"
	.ENDIF	v2
	move	sr,-(sp)
uni__v	=	uni__v+1
.ENDM	push_ex
;
;----------------------------------------------------------------------------
;	Macro for good 32-bit random function (31-bit pseudorandomicity)
;NB:	d0 = result  d1 = smashed
;
rand_32_defined	=	0
;
.MACRO	rand_32		;generates 32 new bits per go
	.IF	rand_32_defined == 0
	bsr.s	code_rand_32
	bra.s	code_rand_32_end
;
code_rand_32:
	move.l	rand_32_seed(pc),d0	;seed = last value (bit 31 irrelevant)
	move.l	d0,d1	;copy to d1  (bit 31 will be lost later)
	lsr.l	#3,d1	;shift d1 by 3 bits for 31 bit pseudorandomicity
	eor.w	d1,d0	;generate 16 new pseudorandom top bits 
	swap	d0	;align 16 new bits correctly
	roxl.w	#1,d0	;align 15 old bits, orig bit 2 as bit 0, lose orig bit 31
	ror.l	#1,d0	;Adjust 31-bit pseudorandom shifter (bit 31=orig bit 2)
	move.l	d0,d1	;copy to d1  (dummy bit 31 will be lost later)
	lsr.l	#3,d1	;shift d1 by 3 bits for 31 bit pseudorandomicity
	eor.w	d1,d0	;generate 16 new pseudorandom top bits 
	swap	d0	;align 16 new bits correctly
	roxl.w	#1,d0	;align 15 old bits, get dummy bit 0, lose dummy bit 31
	ror.l	#1,d0	;Adjust 31-bit pseudorandom shifter (bit 31 = new dummy)
	move.l	d0,rand_32_seed		;store seed (bit 31 irrelevant)
	rts
;
;NB: In the above "irrelevant" means that bit 31 has no effect on future
;    results from the generator, although this bit too is pseudorandom.
;
rand_32_seed:	dc.l	'Rand'	;Arbitrary seed.  Avoid 0 and $80000000 (Don't work)
code_rand_32_end:
	.ELSE
	jsr	code_rand_32
	.ENDIF
.ENDM	rand_32
;
;----------------------------------------------------------------------------
;	Macro for good 32-bit hashing function (31-bit pseudorandomicity)
;NB:	d0 = result  d1 = smashed
;
hash_32_defined	=	0
;
.MACRO	hash_32		;generates 32 new bits per go
	.IF	hash_32_defined == 0
	bsr.s	code_hash_32
	bra.s	code_hash_32_end
;
code_hash_32:
	move.l	d0,d1	;copy to d1  (bit 31 will be lost later)
	lsr.l	#3,d1	;shift d1 by 3 bits for 31 bit pseudorandomicity
	eor.w	d1,d0	;generate 16 new pseudorandom top bits 
	swap	d0	;align 16 new bits correctly
	roxl.w	#1,d0	;align 15 old bits, orig bit 2 as bit 0, lose orig bit 31
	ror.l	#1,d0	;Adjust 31-bit pseudorandom shifter (bit 31=orig bit 2)
	move.l	d0,d1	;copy to d1  (dummy bit 31 will be lost later)
	lsr.l	#3,d1	;shift d1 by 3 bits for 31 bit pseudorandomicity
	eor.w	d1,d0	;generate 16 new pseudorandom top bits 
	swap	d0	;align 16 new bits correctly
	roxl.w	#1,d0	;align 15 old bits, get dummy bit 0, lose dummy bit 31
	ror.l	#1,d0	;Adjust 31-bit pseudorandom shifter (bit 31 = new dummy)
	rts
code_hash_32_end:
	.ELSE
	jsr	code_hash_32
	.ENDIF
.ENDM	hash_32
;
;----------------------------------------------------------------------------
;	Macros to copy memory    NB: 2 aregs & 1 dreg changed
;
.MACRO	mem_copy_b.sz	src_areg,dest_areg,len_dreg
	cmp.l	src_areg,dest_areg
	op_isym	bhi.s,.mem_copy_up_,uni__v
	op_isym	blo.s,.mem_copy_down_ct_,uni__v
	op_isym	bra.s,.mem_copy_done_,uni__v
sl_isym	.mem_copy_down_lp_,uni__v
	move.b	(src_areg)+,(dest_areg)+
sl_isym	.mem_copy_down_ct_,uni__v
	subq.sz	#1,len_dreg
	op_isym	bge.s,.mem_copy_down_lp_,uni__v
	op_isym	bra.s,.mem_copy_done_,uni__v
sl_isym	.mem_copy_up_,uni__v
	add.l	len_dreg,src_areg
	add.l	len_dreg,dest_areg
	op_isym	bra.s,.mem_copy_up_ct_,uni__v
sl_isym	.mem_copy_up_lp_,uni__v
	move.b	-(src_areg),-(dest_areg)
sl_isym	.mem_copy_up_ct_,uni__v
	subq.sz	#1,len_dreg
	op_isym	bge.s,.mem_copy_up_lp_,uni__v
sl_isym	.mem_copy_done_,uni__v
uni__v	=	uni__v+1
.ENDM	mem_copy_b
;
.MACRO	mem_copy_w.sz	src_areg,dest_areg,len_dreg
	cmp.l	src_areg,dest_areg
	op_isym	bhi.s,.mem_copy_up_,uni__v
	op_isym	blo.s,.mem_copy_down_ct_,uni__v
	op_isym	bra.s,.mem_copy_done_,uni__v
sl_isym	.mem_copy_down_lp_,uni__v
	move.w	(src_areg)+,(dest_areg)+
sl_isym	.mem_copy_down_ct_,uni__v
	subq.sz	#2,len_dreg
	op_isym	bge.s,.mem_copy_down_lp_,uni__v
	op_isym	bra.s,.mem_copy_done_,uni__v
sl_isym	.mem_copy_up_,uni__v
	add.l	len_dreg,src_areg
	add.l	len_dreg,dest_areg
	op_isym	bra.s,.mem_copy_up_ct_,uni__v
sl_isym	.mem_copy_up_lp_,uni__v
	move.w	-(src_areg),-(dest_areg)
sl_isym	.mem_copy_up_ct_,uni__v
	subq.sz	#2,len_dreg
	op_isym	bge.s,.mem_copy_up_lp_,uni__v
sl_isym	.mem_copy_done_,uni__v
uni__v	=	uni__v+1
.ENDM	mem_copy_w
;
.MACRO	mem_copy_l.sz	src_areg,dest_areg,len_dreg
	cmp.l	src_areg,dest_areg
	op_isym	bhi.s,.mem_copy_up_,uni__v
	op_isym	blo.s,.mem_copy_down_ct_,uni__v
	op_isym	bra.s,.mem_copy_done_,uni__v
sl_isym	.mem_copy_down_lp_,uni__v
	move.l	(src_areg)+,(dest_areg)+
sl_isym	.mem_copy_down_ct_,uni__v
	subq.sz	#4,len_dreg
	op_isym	bge.s,.mem_copy_down_lp_,uni__v
	op_isym	bra.s,.mem_copy_done_,uni__v
sl_isym	.mem_copy_up_,uni__v
	add.l	len_dreg,src_areg
	add.l	len_dreg,dest_areg
	op_isym	bra.s,.mem_copy_up_ct_,uni__v
sl_isym	.mem_copy_up_lp_,uni__v
	move.l	-(src_areg),-(dest_areg)
sl_isym	.mem_copy_up_ct_,uni__v
	subq.sz	#4,len_dreg
	op_isym	bge.s,.mem_copy_up_lp_,uni__v
sl_isym	.mem_copy_done_,uni__v
uni__v	=	uni__v+1
.ENDM	mem_copy_l
;
.MACRO	buf_copy_b.sz	src_areg,dest_areg,len_dreg
	op_isym	bra.s,.buf_copy_ct_,uni__v
sl_isym	.buf_copy_lp_,uni__v
	move.b	(src_areg)+,(dest_areg)+
sl_isym	.buf_copy_ct_,uni__v
	subq.sz	#1,len_dreg
	op_isym	bcc.s,.buf_copy_lp_,uni__v
uni__v	=	uni__v+1
.ENDM	buf_copy_b
;
.MACRO	buf_copy_w.sz	src_areg,dest_areg,len_dreg
	op_isym	bra.s,.buf_copy_ct_,uni__v
sl_isym	.buf_copy_lp_,uni__v
	move.w	(src_areg)+,(dest_areg)+
sl_isym	.buf_copy_ct_,uni__v
	subq.sz	#2,len_dreg
	op_isym	bcc.s,.buf_copy_lp_,uni__v
uni__v	=	uni__v+1
.ENDM	buf_copy_w
;
.MACRO	buf_copy_l.sz	src_areg,dest_areg,len_dreg
	op_isym	bra.s,.buf_copy_ct_,uni__v
sl_isym	.buf_copy_lp_,uni__v
	move.l	(src_areg)+,(dest_areg)+
sl_isym	.buf_copy_ct_,uni__v
	subq.sz	#4,len_dreg
	op_isym	bcc.s,.buf_copy_lp_,uni__v
uni__v	=	uni__v+1
.ENDM	buf_copy_l
;
;----------------------------------------------------------------------------
;	Macro to pass a string	NB: 1 address reg changed
;
.MACRO	str_pass	p1
sl_isym	.str_pass,uni__v
	tst.b	(&p1)+
	op_isym	bne.s,.str_pass,uni__v
uni__v	=	uni__v+1
.ENDM	str_pass
;
;----------------------------------------------------------------------------
;	Macro to copy a string    NB: 2 address regs changed
;
.MACRO	str_copy	src_areg,dest_areg
sl_isym	.str_copy,uni__v
	move.b	(src_areg)+,(dest_areg)+
	op_isym	bne.s,.str_copy,uni__v
uni__v	=	uni__v+1
.ENDM	str_copy
;
;----------------------------------------------------------------------------
;	Macro to concatenate strings    NB: 2 address regs changed
;
.MACRO	str_conc	src_areg,dest_areg
	str_pass	dest_areg
	subq		#1,dest_areg
sl_isym	.str_conc,uni__v
	move.b	(src_areg)+,(dest_areg)+
	op_isym	bne.s,.str_conc,uni__v
uni__v	=	uni__v+1
.ENDM	str_conc
;
;----------------------------------------------------------------------------
;	Macro to compare two strings	NB: 2 address regs changed
;
.MACRO	str_comp	p1,p2
sl_isym	.str_comp_loop,uni__v
	tst.b	(p1)
	op_isym	beq.s,.str_comp_term_1,uni__v
	cmpm.b	(p1)+,(p2)+
	op_isym	beq.s,.str_comp_loop,uni__v
	op_isym	bra.s,.str_comp_done,uni__v
sl_isym	.str_comp_term_1,uni__v
	tst.b	(p2)
sl_isym	.str_comp_done,uni__v
uni__v	=	uni__v+1
.ENDM	str_comp
;
;----------------------------------------------------------------------------
;	Ascii constants
;
NUL	equ	$00
BEL	equ	$07
BS	equ	$08
HT	equ	$09
LF	equ	$0A
VT	equ	$0B
FF	equ	$0C
CR	equ	$0D
ESC	equ	$1B
;
;
;	Diverse constants
;
NULL		equ	0
maxsbyte	equ	$7F
maxubyte	equ	$FF
maxsword	equ	$7FFF
maxuword	equ	$FFFF
maxslong	equ	$7FFFFFFF
maxulong	equ	$FFFFFFFF
;
Kd		equ	1000
Md		equ	Kd*Kd
Kb		equ	1024
Mb		equ	Kb*Kb
;
_ind	equ	1<<30	;bit 30 is indirection flag for some macro libraries
;
;
;----------------------------------------------------------------------------
;	System init vectors
;
ssp_init	equ	$000
ev_reset	equ	$004
;
;
;	System exception vectors
;
ev_buserr	equ	$008
ev_adrerr	equ	$00C
ev_illegal	equ	$010
ev_divby0	equ	$014
ev_chk_ofl	equ	$018
ev_trapv	equ	$01C
ev_priverr	equ	$020
ev_trace	equ	$024
ev_a_line	equ	$028
ev_f_line	equ	$02C
;
; vectors at $030..$03B are unassigned
; vector at $3C is dubiously specified, but can not be considered free
; vectors at $040..$05F are unassigned
;
ev_spurerr	equ	$060
ev_level1	equ	$064
ev_level2	equ	$068
ev_level3	equ	$06C
ev_level4	equ	$070
ev_level5	equ	$074
ev_level6	equ	$078
ev_level7	equ	$07C
; the odd vectors above are unavailable since IPL0 is tied high
ev_HBI		equ	ev_level2
ev_VBI		equ	ev_level4
ev_MFP		equ	ev_level6
;
ev_trap0	equ	$080
ev_trap1	equ	$084
ev_trap2	equ	$088
ev_trap3	equ	$08C
ev_trap4	equ	$090
ev_trap5	equ	$094
ev_trap6	equ	$098
ev_trap7	equ	$09C
ev_trap8	equ	$0A0
ev_trap9	equ	$0A4
ev_trap10	equ	$0A8
ev_trap11	equ	$0AC
ev_trap12	equ	$0B0
ev_trap13	equ	$0B4
ev_trap14	equ	$0B8
ev_trap15	equ	$0BC
;
ev_gemdos	equ	ev_trap1
ev_xgemdos	equ	ev_trap2
ev_bios		equ	ev_trap13
ev_xbios	equ	ev_trap14
;
; vectors at $0C0..$0FF are unassigned
;
;----------------------------------------------------------------------------
; 	interrupt vectors for MFP interrupts
;
iv_cenbusy	equ	$100
iv_v24dcd	equ	$104
iv_v24cts	equ	$108
iv_blitter	equ	$10C
iv_time_d	equ	$110
iv_time_c	equ	$114
iv_kb_midi	equ	$118
iv_disk		equ	$11C
iv_time_b	equ	$120
iv_v24terr	equ	$124
iv_v24treq	equ	$128
iv_v24rerr	equ	$12C
iv_v24rreq	equ	$130
iv_time_a	equ	$134
iv_v24ring	equ	$138
iv_monodet	equ	$13C
;
; vectors at $140..$1FF are unassigned
; vectors at $200..$37F are reserved for OEM products
;
;----------------------------------------------------------------------------
; 	System bomb info for debug analysis
;
bombflag	equ	$380
bomb_d0		equ	$384
bomb_d1		equ	$388
bomb_d2		equ	$38C
bomb_d3		equ	$390
bomb_d4		equ	$394
bomb_d5		equ	$398
bomb_d6		equ	$39C
bomb_d7		equ	$3A0
bomb_a0		equ	$3A4
bomb_a1		equ	$3A8
bomb_a2		equ	$3AC
bomb_a3		equ	$3B0
bomb_a4		equ	$3B4
bomb_a5		equ	$3B8
bomb_a6		equ	$3BC
bomb_a7		equ	$3C0
bombvector	equ	$3C4
bomb_usp	equ	$3C8
bomb_ssp	equ	bomb_a7
bombstack	equ	$3CC
;
; vector area at $3EC..$3FF is unassigned, but known to be used by
; some system support programs for non_vector purposes (eg. timesave)
;
;
;----------------------------------------------------------------------------
; 	System variables
;
etv_timer	equ	$400
etv_critic	equ	$404
etv_term	equ	$408
etv_extra	equ	$40C	;5.L
memvalid	equ	$420	;.L == $752019F3 when 'memctrl' valid
memctrl		equ	$424	;.W == ST/STE MMU code (hw_mapper)
resvalid	equ	$426	;.L == $31415926 when 'resvector' valid
resvector	equ	$42A	;.L -> extra reset routine(s)
phystop		equ	$42E	;.L -> end of RAM accessible to system
_membot		equ	$432	;.L -> current start of free ram
_memtop		equ	$436	;.L -> current end of free ram
memval2		equ	$43A	;.L == $237698AA when the 3 above are valid
flock		equ	$43E
seekrate	equ	$440
_timr_ms	equ	$442
_fverify	equ	$444
_bootdev	equ	$446
_palmode	equ	$448
defshiftmd	equ	$44A
sshiftmd	equ	$44C
_v_bas_ad	equ	$44E
vblsem		equ	$452
nvbls		equ	$454
_vblqueue	equ	$456
colorptr	equ	$45A
screenpt	equ	$45E
_vbclock	equ	$462
_frclock	equ	$466
hdv_init	equ	$46A
swv_vec		equ	$46E
hdv_bpb		equ	$472
hdv_rw		equ	$476
hdv_boot	equ	$47A
hdv_mediach	equ	$47E
_cmdload	equ	$482
conterm		equ	$484
trp14ret	equ	$486
criticret	equ	$48A
themd		equ	$48E
_md		equ	$49E
savptr		equ	$4A2
_nflops		equ	$4A6
constate	equ	$4A8
save_row	equ	$4AC
sav_contxt	equ	$4AE
_bufl		equ	$4B2
_hz_200		equ	$4BA
the_env		equ	$4BE
_drvbits	equ	$4C2
_dskbufp	equ	$4C6
_autopath	equ	$4CA
_vbl_list	equ	$4CE	;8.L
_prt_cnt	equ	$4EE
_prtabt		equ	$4F0
_sysbase	equ	$4F2
_shell_p	equ	$4F6
end_os		equ	$4FA
exec_os		equ	$4FE
scr_dump	equ	$502
prv_lsto	equ	$506
prv_lst		equ	$50A
prv_auxo	equ	$50E
prv_aux		equ	$512
pun_ptr		equ	$516
memval3		equ	$51A	;.L == $5555AAAA when ST RAM settings valid
dev_vecs	equ	$51E	;8.L (1.L per device) per I/O function
xconstat	equ	$51E
xconin		equ	$53E
xcostat		equ	$55E
xconout		equ	$57E
_longframe	equ	$59E
_cookies	equ	$5A0
ramtop		equ	$5A4	;.L -> Alternate RAM (aka: Fast-RAM) top
ramvalid	equ	$5A8	;.L == $1357BD13 when 'ramtop' is valid
;
;
;----------------------------------------------------------------------------
;	System vector numbers
;-------------------------------------
;		;vector $00 does not exist (SSP value for reset)
;-------------------------------------
evn_reset	equ	$01
evn_buserr	equ	$02
evn_adrerr	equ	$03
evn_illegal	equ	$04
evn_divby0	equ	$05
evn_chk_ofl	equ	$06
evn_trapv	equ	$07
evn_priverr	equ	$08
evn_trace	equ	$09
evn_a_line	equ	$0a
evn_f_line	equ	$0b
;-------------------------------------
;		;vectors $0C..$0E are unassigned
;		;vector $0F is dubiously specified, but is not free
;		;vectors $10..$17 are unassigned
;-------------------------------------
evn_spurerr	equ	$18
evn_level1	equ	$19
evn_level2	equ	$1a
evn_level3	equ	$1b
evn_level4	equ	$1c
evn_level5	equ	$1d
evn_level6	equ	$1e
evn_level7	equ	$1f
; 		;odd vectors above are unavailable with IPL0 tied high
;-------------------------------------
evn_HBI		equ	evn_level2
evn_VBI		equ	evn_level4
evn_MFP		equ	evn_level6
;-------------------------------------
evn_trap0	equ	$20
evn_trap1	equ	$21
evn_trap2	equ	$22
evn_trap3	equ	$23
evn_trap4	equ	$24
evn_trap5	equ	$25
evn_trap6	equ	$26
evn_trap7	equ	$27
evn_trap8	equ	$28
evn_trap9	equ	$29
evn_trap10	equ	$2a
evn_trap11	equ	$2b
evn_trap12	equ	$2c
evn_trap13	equ	$2d
evn_trap14	equ	$2e
evn_trap15	equ	$2f
;-------------------------------------
evn_gemdos	equ	evn_trap1
evn_xgemdos	equ	evn_trap2
evn_bios	equ	evn_trap13
evn_xbios	equ	evn_trap14
;-------------------------------------
;		;vectors $30..$3F are unassigned
;-------------------------------------
; 	MFP interrupt vector numbers
;-------------------------------------
ivn_cenbusy	equ	$40
ivn_v24dcd	equ	$41
ivn_v24cts	equ	$42
ivn_blitter	equ	$43
ivn_time_d	equ	$44
ivn_time_c	equ	$45
ivn_kb_midi	equ	$46
ivn_disk	equ	$47
ivn_time_b	equ	$48
ivn_v24terr	equ	$49
ivn_v24treq	equ	$4a
ivn_v24rerr	equ	$4b
ivn_v24rreq	equ	$4c
ivn_time_a	equ	$4d
ivn_v24ring	equ	$4e
ivn_monodet	equ	$4f
;-------------------------------------
;		;vectors $50..$7F are unassigned
;		;vectors $80..$DF are reserved for OEM products
;		;vectors $E0..$FA do not exist (bomb info area)
;		;vectors $FB..$FF may not exist (unassigned but often used!)
;-------------------------------------
; 	System variable vector numbers
;-------------------------------------
etvn_timer	equ	$100
etvn_critic	equ	$101
etvn_term	equ	$102
;----------------------------------------------------------------------------
;	OS header offsets
;
os_codebra	equ $00	;w $60xx
os_version	equ $02	;w $0v0r
os_reset_p	equ $04	;L
os_selfbeg_p	equ $08	;L
os_varend_p	equ $0c	;L
os_unknown_p	equ $10	;L
os_gem_mpb_p	equ $14	;L
os_date_bcd	equ $18	;L
os_config	equ $1c	;w
os_date_gem	equ $1e	;w
;	next follows the part valid only for TOS 1.02 & later
os_pool_p	equ $20
os_kbshift_p	equ $24
os_currbp_p_p	equ $28
os_reserved	equ $2c
;
TOS_100_kbshift	equ $E1B
TOS_100_currbp_p	equ $602C
;
;-------------------------------------
;	Cartridge ROM definitions
;
rom_diag_id	equ $fa52255f
rom_appl_id	equ $abcdef42
rom_base	equ $fa0000
rom_head	equ $fa0004
;
rh_diagcode	equ 0
rh_applnext_p	equ 0
rh_applinit_p	equ 4
rh_applcode_p	equ 8
rh_appltime	equ 12
rh_appldate	equ 14
rh_applsize	equ 16
rh_applname	equ 20	;<equ13b incl terminal NUL
;
;-------------------------------------
;	IOREC data structure
;
io_sel_serial	equ 0
io_sel_keyboard	equ 1
io_sel_midi	equ 2
;
io_buffer_p	equ  0	;L-> data buffer (NB: kbd uses 4 bytes/char)
io_size_ix	equ  4	;w equ total size of buffer, in bytes
io_head_ix	equ  6	;w equ indexes position of next write
io_tail_ix	equ  8	;w equ indexes position of next read
io_lo_fill	equ 10	;w equ flow reactivation limit
io_hi_fill	equ 12	;w equ flow deactivation limit
;
Rx_buffer_p	equ $00	;L-> data buffer (NB: kbd uses 4 bytes/char)
Rx_size_ix	equ $04	;w equ total size of buffer, in bytes
Rx_head_ix	equ $06	;w equ indexes position of next write
Rx_tail_ix	equ $08	;w equ indexes position of next read
Rx_lo_fill	equ $0A	;w equ flow reactivation limit
Rx_hi_fill	equ $0C	;w equ flow deactivation limit
Tx_buffer_p	equ $0E	;L-> data buffer (NB: kbd uses 4 bytes/char)
Tx_size_ix	equ $12	;w equ total size of buffer, in bytes
Tx_head_ix	equ $14	;w equ indexes position of next write
Tx_tail_ix	equ $16	;w equ indexes position of next read
Tx_lo_fill	equ $18	;w equ flow reactivation limit
Tx_hi_fill	equ $1A	;w equ flow deactivation limit
;-------------------------------------
;	KBDVBASE data structure
;
kv_midi_input	equ $00
kv_keybrd_err	equ $04
kv_midi_err	equ $08
kv_ikbd_stat	equ $0C
kv_mouse_pack	equ $10
kv_clock_pack	equ $14
kv_joyst_pack	equ $18
kv_midi_vec	equ $1C
kv_ikbd_vec	equ $20
;
;-------------------------------------
;	Virtual Screen cookie structure
;
vs_vs_magic	equ $00	;l equ 'VSCR' when screen driver is _active_ !!!
vs_driver_magic equ $04	;l equ XBRA/Cookie id of screen driver
vs_version	equ $08	;w equ version of VSCR protocol (for future exp)
vs_x		equ $0A	;w equ left edge \
vs_y		equ $0C	;w equ top edge   \/ of visible
vs_w		equ $0E	;w equ width      /\ rectangle
vs_h		equ $10	;w equ height    /
vs_size		equ $12	;size of virtual screen structure
vs_rect		equ vs_x
;
;-------------------------------------
;	Font header offsets
;
;NB: Intel format is used in all words and longs, when fnt_flag bit 2 is zero.
;NB: Test this as 68000 word, however, for the first byte contains that flag
;NB: in Intel mode only, with the second byte zeroed so 68000 test will work.
;NB: In 68000 mode the first byte is always zero, so Intel test would .error.
;
fnt_id		equ $00	;w 
fnt_pts		equ $02	;w
fnt_name	equ $04	;32b
fnt_lasc	equ $24	;w
fnt_hasc	equ $26	;w
fnt_dtop	equ $28	;w
fnt_dasce	equ $2A	;w
fnt_dhalf	equ $2C	;w
fnt_ddesc	equ $2E	;w
fnt_dbott	equ $30	;w
fnt_charw	equ $32	;w
fnt_cellw	equ $34	;w
fnt_loff	equ $36	;w
fnt_roff	equ $38	;w
fnt_weight	equ $3A	;w
fnt_ulheight	equ $3C	;w
fnt_litemask	equ $3E	;w
fnt_skewmask	equ $40	;w
fnt_flag	equ $42	;w
fnt_hor_tp	equ $44	;l->horiz. offset table, or equ fnt_chr_tp
fnt_chr_tp	equ $48	;l->char. offset table, in files use $58 (fnt_next+4)
fnt_fbase	equ $4C	;l->
fnt_fwidth	equ $50	;w
fnt_fheight	equ $52	;w
fnt_next	equ $54	;l->next font in chain
;
;
;----------------------------------------------------------------------------
; 	fpu offsets (68881 or 68882)
;
fpu_base	equ	$00
fpu_stat	equ	$00	;W Rd
fpu_cont	equ	$02	;W Wr
fpu_save	equ	$04	;W Rd
fpu_rest	equ	$06	;W R/W
fpu_comm	equ	$0A	;W Wr
fpu_cond	equ	$0E	;W Wr
fpu_oper	equ	$10	;L R/W
fpu_rsel	equ	$14	;W Rd
fpu_iadr	equ	$18	;L Wr
fpu_opad	equ	$1C	;L R/W (dummy in 68881 and 68882)
;
;
;----------------------------------------------------------------------------
; 	System hardware
;
hw_mapper	equ	$ffff8001
;
hw_f30_mon_mem	equ	$ffff8006	;b7.6 3-0==TV,VGA,PAL,SM  b5.4 3-0==na,16M,4M,1M
hw_f30_comp_div	equ	$ffff8007
;
f30_comp_cpu_b	equ 0			;1 equ> 16 MHz   0 equ> 8 MHz   CPU
f30_comp_cpu_v	equ 1<<0
f30_comp_blit_b	equ 2			;1 equ> 16 MHz   0 equ> 8 MHz   Blitter
f30_comp_blit_v	equ 1<<2
f30_comp_bus_b	equ 5			;1 equ> Falcon   0 equ> STE     Bus
f30_comp_bus_v  equ 1<<5
f30_div_mon_b	equ 6			;1 equ> On       0 equ> Off     Screen
f30_div_mon_v	equ 1<<6
;
hw_vbase2	equ	$ffff8201	;base MSB
hw_vbase1	equ	$ffff8203
hw_vpos2	equ	$ffff8205	;pos MSB
hw_vpos1	equ	$ffff8207
hw_vpos0	equ	$ffff8209	;pos LSB
hw_syn		equ	$ffff820A
;
hw_vbase0H	equ	$ffff820C	;STE (unused)
hw_vbase0	equ	$ffff820D	;base LSB, STE
hw_horextH	equ	$ffff820E	;STE (unused)
hw_horext	equ	$ffff820F	;STE
;
hw_pal		equ	$ffff8240
hw_pal_00	equ	hw_pal+00
hw_pal_01	equ	hw_pal+02
hw_pal_02	equ	hw_pal+04
hw_pal_03	equ	hw_pal+06
hw_pal_04	equ	hw_pal+08
hw_pal_05	equ	hw_pal+10
hw_pal_06	equ	hw_pal+12
hw_pal_07	equ	hw_pal+14
hw_pal_08	equ	hw_pal+16
hw_pal_09	equ	hw_pal+18
hw_pal_10	equ	hw_pal+20
hw_pal_11	equ	hw_pal+22
hw_pal_12	equ	hw_pal+24
hw_pal_13	equ	hw_pal+26
hw_pal_14	equ	hw_pal+28
hw_pal_15	equ	hw_pal+30
;
hw_rez		equ	$ffff8260
;
hw_rez_tt	equ	$ffff8262
;
hw_pixoffH	equ	$ffff8264	;STE (unused)
hw_pixoff	equ	$ffff8265	;STE
;
hw_pal_tt	equ	$ffff8400
;
hw_dmadata	equ	$ffff8604
hw_dmascnt	equ	$ffff8604
hw_dmastat	equ	$ffff8606
hw_dmacont	equ	$ffff8606
hw_dmamode	equ	$ffff8606
dma_srcmd	equ $080
dma_srtrk	equ $082
dma_srsec	equ $084
dma_srdat	equ $086
dma_srcnt	equ $090
dma_wr_bit	equ $100
dma_swcmd	equ $180
dma_swtrk	equ $182
dma_swsec	equ $184
dma_swdat	equ $186
dma_swcnt	equ $190
hw_dmabase2	equ	$ffff8609	;MSB
hw_dmabase1	equ	$ffff860B
hw_dmabase0	equ	$ffff860D	;LSB
;
hw_scsi_tt_dma3	equ	$ffff8701	;MSB
hw_scsi_tt_dma2	equ	$ffff8703
hw_scsi_tt_dma1	equ	$ffff8705
hw_scsi_tt_dma0	equ	$ffff8707	;LSB
hw_scsi_tt_cnt3	equ	$ffff8709	;MSB
hw_scsi_tt_cnt2	equ	$ffff870B
hw_scsi_tt_cnt1	equ	$ffff870D
hw_scsi_tt_cnt0	equ	$ffff870F	;LSB
hw_scsi_tt_drr	equ	$ffff8710	;Long
hw_scsi_tt_cr	equ	$ffff8715	;Byte
;
hw_5380_tt_dr	equ	$ffff8781
hw_5380_tt_icr	equ	$ffff8783
hw_5380_tt_mr	equ	$ffff8785
hw_5380_tt_tcr	equ	$ffff8787
hw_5380_tt_idcr	equ	$ffff8789
hw_5380_tt_stsr	equ	$ffff878B
hw_5380_tt_trid	equ	$ffff878D
hw_5380_tt_irrs	equ	$ffff878F
;
hw_giselect	equ	$ffff8800
hw_psgsel	equ	$ffff8800
hw_psgrd	equ	$ffff8800
hw_giwrite	equ	$ffff8802
hw_psgwr	equ	$ffff8802
;
hw_sdmacontH	equ	$ffff8900	;STE (unused)
hw_sdmacont	equ	$ffff8901	;STE
hw_sdmabeg2	equ	$ffff8903	;STE
hw_sdmabeg1	equ	$ffff8905	;STE
hw_sdmabeg0	equ	$ffff8907	;STE
hw_sdmaloop2	equ	$ffff8909	;STE
hw_sdmaloop1	equ	$ffff890B	;STE
hw_sdmaloop0	equ	$ffff890D	;STE
hw_sdmaend2	equ	$ffff890F	;STE
hw_sdmaend1	equ	$ffff8911	;STE
hw_sdmaend0	equ	$ffff8913	;STE
;
hw_f30_dac_trk	equ	$ffff8920	;F30
hw_sdmamode	equ	$ffff8921	;STE/TT/F30
;
hw_mywiredata	equ	$ffff8922	;STE/TT 16 bits
hw_mywiremask	equ	$ffff8924	;STE/TT 16 bits
;
hw_f30_xbar_sc	equ	$ffff8930	;word
hw_f30_xbar_dc	equ	$ffff8932	;word
;
hw_rtc_tt_ar	equ	$ffff8960	;word
hw_rtc_tt_dr	equ	$ffff8962	;word
;
hw_scc_tt_dma3	equ	$ffff8C01
hw_scc_tt_dma2	equ	$ffff8C03
hw_scc_tt_dma1	equ	$ffff8C05
hw_scc_tt_dma0	equ	$ffff8C07
hw_scc_tt_cnt3	equ	$ffff8C09
hw_scc_tt_cnt2	equ	$ffff8C0B
hw_scc_tt_cnt1	equ	$ffff8C0D
hw_scc_tt_cnt0	equ	$ffff8C0F
hw_scc_tt_drr	equ	$ffff8C10	;Long
hw_scc_tt_cr	equ	$ffff8C15
;
hw_8530_tt_a_cr	equ	$ffff8C81
hw_8530_tt_a_dr	equ	$ffff8C83
hw_8530_tt_b_cr	equ	$ffff8C85
hw_8530_tt_b_dr	equ	$ffff8C87
;
hw_tt_sys_im	equ	$ffff8E01
hw_tt_sys_is	equ	$ffff8E03
hw_tt_sys_ig	equ	$ffff8E05
hw_tt_vme_ig	equ	$ffff8E07
hw_tt_scu_gpr1	equ	$ffff8E09
hw_tt_scu_gpr2	equ	$ffff8E0B
hw_tt_vme_im	equ	$ffff8E0D
hw_tt_vme_is	equ	$ffff8E0F
;
hw_switches	equ	$ffff9200	;FALCON
hw_joyfire	equ	$ffff9201	;STE 4 bits
hw_joydirect	equ	$ffff9202	;STE 4*4bits
;
hw_pad0_y	equ	$ffff9211	;STE
hw_pad0_x	equ	$ffff9213	;STE
hw_pad1_y	equ	$ffff9215	;STE
hw_pad1_x	equ	$ffff9217	;STE
;
hw_light_x	equ	$ffff9220	;STE
hw_light_y	equ	$ffff9222	;STE
;
hw_f30_pal	equ	$ffff9800	;F30
;
hw_dsp_ic	equ	$ffffa200	;F30
hw_dsp_cv	equ	$ffffa201	;F30
hw_dsp_is	equ	$ffffa202	;F30
hw_dsp_iv	equ	$ffffa203	;F30
;
hw_dsp_long	equ	$ffffa204	;F30	top byte is ignored
hw_dsp_d2	equ	$ffffa205	;F30
hw_dsp_d1	equ	$ffffa206	;F30
hw_dsp_d0	equ	$ffffa207	;F30
;
.MACRO	Tx_DSP_wait
sl_isym	.Tx_delay,uni__v		;loop start awaiting Tx permission
	btst.b	#1,(hw_dsp_is).w
	op_isym	beq.s,.Tx_delay,uni__v	;loop back until data reg empty
.ENDM	Tx_DSP_wait
;
.MACRO	Rx_DSP_wait
sl_isym	.Rx_delay,uni__v		;loop start awaiting data
	btst.b	#0,(hw_dsp_is).w
	op_isym	beq.s,.Rx_delay,uni__v	;loop back until data here
.ENDM
;
hw_mfp		equ	$ffffFA01
hw_gpip		equ	$ffffFA01
hw_aer		equ	$ffffFA03
hw_ddr		equ	$ffffFA05
hw_iera		equ	$ffffFA07
hw_ierb		equ	$ffffFA09
hw_ipra		equ	$ffffFA0B
hw_iprb		equ	$ffffFA0D
hw_isra		equ	$ffffFA0F
hw_isrb		equ	$ffffFA11
hw_imra		equ	$ffffFA13
hw_imrb		equ	$ffffFA15
hw_vr		equ	$ffffFA17
hw_tacr		equ	$ffffFA19
hw_tbcr		equ	$ffffFA1B
hw_tcdcr	equ	$ffffFA1D
hw_tadr		equ	$ffffFA1F
hw_tbdr		equ	$ffffFA21
hw_tcdr		equ	$ffffFA23
hw_tddr		equ	$ffffFA25
hw_scr		equ	$ffffFA27
hw_ucr		equ	$ffffFA29
hw_rsr		equ	$ffffFA2B
hw_tsr		equ	$ffffFA2D
hw_udr		equ	$ffffFA2F
;
hw_fpu_base	equ	$ffffFA40	;SFP-004 with 68881 or 68882
hw_fpu_stat	equ	$ffffFA40	;W Rd
hw_fpu_cont	equ	$ffffFA42	;W Wr
hw_fpu_save	equ	$ffffFA44	;W Rd
hw_fpu_rest	equ	$ffffFA46	;W R/W
hw_fpu_comm	equ	$ffffFA4A	;W Wr
hw_fpu_cond	equ	$ffffFA4E	;W Wr
hw_fpu_oper	equ	$ffffFA50	;L R/W
hw_fpu_rsel	equ	$ffffFA54	;W Rd
hw_fpu_iadr	equ	$ffffFA58	;L Wr
hw_fpu_opad	equ	$ffffFA5C	;L R/W (dummy in 68881 and 68882)
;
hw_tt_mfp2	equ	$fffffa81
;
hw_kbstat	equ	$ffffFC00
hw_kbcont	equ	$ffffFC00
hw_kbdata	equ	$ffffFC02
;
hw_midistat	equ	$ffffFC04
hw_midicont	equ	$ffffFC04
hw_mididata	equ	$ffffFC06
;
;----------------------------------------------------------------------------
; End of file:	URAn_SYS.SH
;----------------------------------------------------------------------------
