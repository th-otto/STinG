;----------------------------------------------------------------------------
; File name:	DOMAIN.SH			Revision date:	1997.08.09
; Authors:	Ronald Andersson		Creation date:	1997.08.01
;						Version:	1.00
;----------------------------------------------------------------------------
; Purpose:	Defines constants and macros which are useful in handling
;		domain names and IP addresses, including some conversions.
;----------------------------------------------------------------------------
;Required header declarations:
;
;	.include	"uran\struct.hs"
;	.include	"sting\domain.sh"
;
;copy the above to the header of your program and 'uncomment' the includes
;
;----------------------------------------------------------------------------
;NB: Having used any of these macros, you have to include a call like this:
;
;		make	DOMAIN_links
;
;    This must be somewhere in the executable code, after all 'tcon_' macros
;    that are to be used in the program have been called at least once.
;----------------------------------------------------------------------------
;Available macros and their usage:
;
;-------------------------------------------------------------------------------------)
;
.MACRO	DOMAIN_links
	_unidef	is_domname
	_unidef	is_dip
	_unidef	is_unblank
	_unidef	next_dip
	_unidef	skip_space
	_unidef	diptobip
	_unidef	biptodip
	_unidef	biptodrip
	_unidef	pass_RRname
.ENDM	DOMAIN_links
;
	_unidec	is_domname
	_unidec	is_dip
	_unidec	is_unblank
	_unidec	next_dip
	_unidec	skip_space
	_unidec	diptobip
	_unidec	biptodip
	_unidec	biptodrip
	_unidec	pass_RRname
;
;-------------------------------------------------------------------------------------)
;char	*is_domname(char *string, len)	/* flags -> domain name */
;
.MACRO	is_domname	string_p,len
	PUREC_func	is_domname,2,7,string_p,len
.ENDM	is_domname
;
.MACRO	code_is_domname
	movem.l	d1-d2/a1,-(sp)
	move.l	a0,d1		;NULL string ?
	beq.s	.exit
	move.l	a0,a1		;a1 = a0 = string
	move	d0,d2		;d2 = len
	ble.s	.label_bad
	skip_space	(a0)	;string passed to non-blank
	move.l	a0,d0
	sub.l	a1,d0		;d0 = passed length
	sub	d0,d2		;reduce total length by passed blanks
	ble.s	.label_bad
	move.l	a0,a1		;a1 = a0 = non_blank string
	move	d2,d0		;d0 = remaining len
.label_start:
	clr	d2		;d2 = 0  indicates label starts
.label_char:
	dbra	d0,.test_char	;test len characters
	tst	d2		;last label empty ?
	beq.s	.label_bad
.label_ok:
	movem.l	(sp)+,d1-d2/a1
	rts			;return a0 -> domname
;
.test_char:
	move.b	(a1)+,d1	;d1 = current char
	beq.s	.label_bad
	cmp.b	#'.',d1
	beq.s	.label_start
	tst	d2
	beq.s	.test_alfa
	cmp.b	#'-',d1
	beq.s	.label_char
	cmp.b	#'0',d1
	blo.s	.label_bad
	cmp.b	#'9',d1
	bls.s	.label_char
.test_alfa:
	st	d2
	cmp.b	#'A',d1
	blo.s	.label_bad
	cmp.b	#'Z',d1
	bls.s	.label_char
	cmp.b	#'a',d1
	blo.s	.label_bad
	cmp.b	#'z',d1
	bls.s	.label_char
.label_bad:
	suba.l	a0,a0		;return NULL on error
.exit:
	movem.l	(sp)+,d1-d2/a1
	rts
.ENDM	code_is_domname
;
;-------------------------------------------------------------------------------------)
;char	*is_dip(char *string)		/* flags -> dotted ip */
;
.MACRO	is_dip.mode	string_p
	PUREC_func.mode	is_dip,1,3,string_p
.ENDM	is_dip
;
.MACRO	code_is_dip
	movem.l	d1/a1,-(sp)
	move.l	a0,d0
	beq.s	.exit
	skip_space	(a0)
	move.l	a0,a1
	moveq	#3,d1
.loop:
	move.b	(a0)+,d0
	cmp.b	#'.',d0
	bne.s	.try_digits
	dbra	d1,.loop
	bra.s	.make_null
;
.try_digits:
	cmp.b	#'0',d0
	blo.s	.test_dip
	cmp.b	#'9',d0
	bls.s	.loop
.test_dip:
	tst	d1
	bne.s	.make_null
	move.l	a1,a0
.exit:
	movem.l	(sp)+,d1/a1
	rts
;
.make_null:
	suba.l	a0,a0
	movem.l	(sp)+,d1/a1
	rts
.ENDM	code_is_dip
;
;-------------------------------------------------------------------------------------)
;char	*is_unblank(char *string)	/* flags -> unblank string */
;
.MACRO	is_unblank.mode	string_p
	PUREC_func.mode	is_unblank,1,3,string_p
.ENDM	is_unblank
;
.MACRO	code_is_unblank
	move.l	a0,d0
	beq.s	.exit
	skip_space	(a0)
	tst.b	(a0)
	bne.s	.exit
	suba.l	a0,a0
.exit:
	rts
.ENDM	code_is_unblank
;
;-------------------------------------------------------------------------------------)
;char	*next_dip(char *string)		/* passes comma-separated arguments */
;
.MACRO	next_dip.mode	string_p
	PUREC_func.mode	next_dip,1,3,string_p
.ENDM	next_dip
;
.MACRO	code_next_dip
.loop:
	move.l	a0,d0
	beq.s	.exit
	move.b	(a0)+,d0
	beq.s	.back_a0		;refuse to pass terminator
	cmp.b	#';',d0
	beq.s	.pass_tail	;pass entire comment
	cmp.b	#',',d0
	bne.s	.loop	;accept comma as separator
	skip_space	(a0)
	rts
;
.pass_tail:
	tst.b	(a0)+
	bne.s	.pass_tail	;pass all non-terminators
.back_a0:
	subq	#1,a0		;back a0 to terminator
.exit:
	rts
.ENDM	code_next_dip
;
;-------------------------------------------------------------------------------------)
;char	*skip_space(char *string)	/* skips leading spaces & tabs */
;
.MACRO	skip_space.mode	string_p
	PUREC_func.mode	skip_space,1,3,string_p
.ENDM	skip_space
;
.MACRO	code_skip_space
.loop:
	move.l	a0,d0
	beq.s	.exit
	move.b	(a0)+,d0
	cmp.b	#' ',d0		;space ?
	beq.s	.loop		;pass leading spaces
	cmp.b	#$09,d0		;HT ?
	beq.s	.loop		;pass leading tabs
	subq	#1,a0
.exit:
	rts
.ENDM	code_skip_space
;
;-------------------------------------------------------------------------------------)
;uint32	diptobip(char *s_p)
;
.MACRO	diptobip.mode	string_p
	PUREC_func.mode	diptobip,1,3,string_p
.ENDM	diptobip
;
.MACRO	code_diptobip
	movem.l	d1-d2,-(sp)
	skip_space	(a0)
	moveq	#-1,d0
	cmp.b	#'0',(a0)
	blo.s	.exit
	cmp.b	#'9',(a0)
	bhi.s	.exit
	clr.l	d0
	moveq	#4-1,d2
.loop_1:
	clr	d1
.loop_2:
	cmp.b	#'0',(a0)
	blo.s	.exit_loop_2
	cmp.b	#'9',(a0)
	bhi.s	.exit_loop_2
	mulu	#10,d1
	add.b	(a0)+,d1
	sub	#'0',d1
	and	#$FF,d1
	bra.s	.loop_2
;
.exit_loop_2:
	lsl.l	#8,d0
	or.l	d1,d0
	cmp.b	#'.',(a0)+
	beq.s	.next_loop_1
	subq	#1,a0
.next_loop_1:
	dbra	d2,.loop_1
.exit:
	movem.l	(sp)+,d1-d2
	rts
.ENDM	code_diptobip
;
;-------------------------------------------------------------------------------------)
;char	*biptodip(uint32 ip_n, char *s_p)
;
.MACRO	biptodip.mode	ip_n,string_p
	PUREC_func.mode	biptodip,2,$E,ip_n,string_p
.ENDM	biptodip
;
.MACRO	code_biptodip
	movem.l	d1-d2,-(sp)
	moveq	#4-1,d2
.loop:
	move.l	a0,a1			;a1 = a0 = start of next number
	rol.l	#8,d0
	clr.l	d1
	move.b	d0,d1
	divu	#100,d1
	beq.s	.hundreds_done
	add.b	#'0',d1
	move.b	d1,(a0)+
	clr.w	d1
.hundreds_done:
	swap	d1
	divu	#10,d1
	bne.s	.use_tens_digit
	cmpa.l	a0,a1			;no hundreds digit ?
	beq.s	.tens_done
.use_tens_digit:
	add.b	#'0',d1
	move.b	d1,(a0)+
	clr.w	d1
.tens_done:
	swap	d1
	add.b	#'0',d1
	move.b	d1,(a0)+
	move.b	#'.',(a0)+
	dbra	d2,.loop
	clr.b	-(a0)
	move.l	a0,d0
	movem.l	(sp)+,d1-d2
	rts
.ENDM	code_biptodip
;
;-------------------------------------------------------------------------------------)
;char	*biptodrip(uint32 ip_n, char *s_p)
;
.MACRO	biptodrip.mode	ip_n,string_p
	PUREC_func.mode	biptodrip,2,$E,ip_n,string_p
.ENDM	biptodrip
;
.MACRO	code_biptodrip
	movem.l	d1-d2,-(sp)
	moveq	#4-1,d2
.loop:
	move.l	a0,a1			;a1 = a0 = start of next number
	clr.l	d1
	move.b	d0,d1
	divu	#100,d1
	beq.s	.hundreds_done
	add.b	#'0',d1
	move.b	d1,(a0)+
	clr.w	d1
.hundreds_done:
	swap	d1
	divu	#10,d1
	bne.s	.use_tens_digit
	cmpa.l	a0,a1			;no hundreds digit ?
	beq.s	.tens_done
.use_tens_digit:
	add.b	#'0',d1
	move.b	d1,(a0)+
	clr.w	d1
.tens_done:
	swap	d1
	add.b	#'0',d1
	move.b	d1,(a0)+
	move.b	#'.',(a0)+
	ror.l	#8,d0
	dbra	d2,.loop
	clr.b	-(a0)
	move.l	a0,d0
	movem.l	(sp)+,d1-d2
	rts
.ENDM	code_biptodrip
;
;-------------------------------------------------------------------------------------)
;char *pass_RRname(char *data_p, char *pos_p, char *dest_p)
;
.MACRO	pass_RRname.mode	data_p,pos_p,dest_p
	PUREC_func.mode	pass_RRname,3,$3F,data_p,pos_p,dest_p
.ENDM	pass_RRname
;
.MACRO	code_pass_RRname
	movem.l	d1-d2/a1-a2,-(sp)
	move.l	a2,-(sp)
	move.l	8(sp),a2
	clr.l	d2
.loop_2:
	clr	d0
	move.b	(a1)+,d0
	beq.s	.exit
	bpl.s	.no_compr
	and	#$3F,d0		;d0 == compression mark ?
	lsl	#8,d0
	move.b	(a1)+,d0	;d0 = compressed label
	tst.l	d2		;done some expansion ?
	bne.s	.keep_d2
	move.l	a1,d2		;d2 = a1 -> type beyond current name name
.keep_d2:
	lea	-12(a0,d0),a1	;a1 -> expansion label
	bra.s	.loop_2		;go expand label
;
.no_compr:
	move.l	a2,d1		;dest_p == NULL ?
	beq.s	.skip_1
	subq	#1,d0
.loop_1:
	move.b	(a1)+,(a2)+	;store string
	dbra	d0,.loop_1
	move.b	#'.',(a2)+	;store '.'
	bra.s	.loop_2
;
.skip_1:
	add	d0,a1		;pass string
	bra.s	.loop_2
;
.exit:
	move.l	a2,d1		;dest_p == NULL ?
	beq.s	.skip_2
	cmp.b	#'.',-(a2)
	bne.s	.skip_2
	clr.b	(a2)
.skip_2:
	tst.l	d2		;done some expansion ?
	bne.s	.return_d2
	move.l	a1,d2		;d2 = a1 -> type beyond current name
.return_d2:
	movem.l	(sp)+,a2
	move.l	d2,a0
	movem.l	(sp)+,d1-d2/a1-a2
	rts
.ENDM	code_pass_RRname
;
;----------------------------------------------------------------------------
; End of file:	DOMAIN.SH
;----------------------------------------------------------------------------
