;----------------------------------------------------------------------------
;File name:	URAn_XB.SH			Revision date:	1997.08.19
;Creator:	Ulf Ronald Andersson		Creation date:	1991.05.07
;(c)1991 by:	Ulf Ronald Andersson		All rights reserved
;Released as:	FREEWARE			(commercial sale forbidden)
;----------------------------------------------------------------------------
;Purpose:	Macro library for XBRA operations
;----------------------------------------------------------------------------
;Required header declarations:
;
;	include	"uran\struct.sh"
;	include	"uran\uran_sys.sh"
;	include	"uran\uran_dos.sh"
;	include	"uran\uran_xb.sh"
;
;----------------------------------------------------------------------------
;	Library macros:
;
;   The first 4 alter no registers
; XB_define	xbname,xbra_id	Defines header for routine code
; XB_gonext	xbname		Smart Link to next routine in chain
; XB_gonext_d	xbname		FAST Link to next routine in chain
; XB_donext_d	xbname		FAST Call to next routine in chain
;
;   The next 2 alter only register a0
; XB_gonext_a0	xbname		BEST Link to next routine in chain
; XB_donext_a0	xbname		BEST Call to next routine in chain
;
;   The next 2 alter only the choosen "areg"
; Pass_Frame	areg			areg->exception argument
; XB_donext	xbname,areg		Smart Call to next routine in chain
;
;   The next 3 affect d0-d2/a0-a2, since they use GEMDOS super
; XB_check	xbname,root		d0= found/last vector  flagged PL/MI
; XB_install	xbname,root		" + Installs xbname in chain(root)
; XB_remove	xbname,root		" + Removes xbname from chain(root)
;
; Legal forms for xbname & root here are mostly the same as for LEA & PEA,
;   except that for XB_define "xbname" is a free name for the new structure.
; "xbname" always refers to the first byte of the entire structure.
; "xbra_id" is a 4-character (longword) string
; "areg" is a free address register of your choice.
; XB_gonext_d & XB_donext_d are faster versions of XB_gonext & XB_donext,
;   but can only handle address modes where "xbname" begins with identifier.
;   eg: "XB_donext_d  my_ikbd_sub(pc)"  but  "XB_donext  (a5)+")
; XB_gonext_a0 & XB_donext_a0 are even faster, but destroy value in a0.
; Pass_Frame makes exception routines (eg: gemdos etc.) 68030-compatible,
;   by setting "areg" = stack pointer before exception (SSP or USP)
;
;XB_define may (optionally) have a third argument which, if present, will be
;placed in the link pointer as its initial value.
;
;XB_cpu_mode is used to control Supervisor mode testing/setting of the three
;macros  XB_check,  XB_install,  XB_remove,  and is normally 2.  This can be
;changed _before_ any of those macros is invoked as follows:
;
;XB_cpu_mode	set	0	=> never test, always call Super(0)
;XB_cpu_mode	set	1	=> never test, never call Super
;XB_cpu_mode	set	2	=> test with Super(1), maybe use Super(0)
;
;In all these cases the routines exit in same mode as entered provided that
;mode 1 is only used with cpu in Super mode, and mode 0 is only used with
;cpu in User mode.  Mode 2 is for code needing both capabilities.
;----------------------------------------------------------------------------
XB_cpu_mode	set	2
;----------------------------------------------------------------------------
;
	struct	xb_struct
	d_l	xb_magic
	d_l	xb_id
	d_l	xb_next
	d_alias	xb_code
	d_alias	xb_size
	d_end	xb_struct
;
;----------------------------------------------------------------------------
;	Macro definitions (with support variables)
;
.MACRO	Pass_Frame	areg
	local	.user,.new_cpu,.have_arg
	btst	#5,(sp)
	beq.s	.user
	tst	(_longframe).w
	bne.s	.new_cpu
	lea	6(sp),areg
	bra.s	.have_arg
.new_cpu:
	lea	8(sp),areg
	bra.s	.have_arg
.user:
	move.l	USP,areg
.have_arg:
.ENDM
;
;----------------------------------------------------------------------------
;
.MACRO	XB_define xbname,xbra_id,init_link
xbname:	dc.l	'XBRA',xbra_id
	ifb	init_link
	dc.l	0
	else
	dc.l	init_link
	endif
.ENDM
;
;----------------------------------------------------------------------------
;
.MACRO	XB_donext_a0	xbname
	move.l	8+xbname,a0
	jsr	(a0)
.ENDM
;
;
.MACRO	XB_donext_d	xbname
	local	.retadr
	pea	.retadr(pc)
	move.l	8+xbname,-(sp)
	rts
.retadr:
.ENDM
;
;
.MACRO	XB_donext	xbname,areg
	lea	xbname,areg
	move.l	8(areg),areg
	jsr	(areg)
.ENDM
;
;----------------------------------------------------------------------------
;
.MACRO	XB_gonext_a0	xbname
	move.l	8+xbname,a0
	jmp	(a0)
	endm
;
;
	macro	XB_gonext_d	xbname
	move.l	8+xbname,-(sp)
	rts
	endm
;
;
	macro	XB_gonext	xbname
	movem.l	a0-a1,-(sp)
	lea	xbname,a0
	move.l	8(a0),4(sp)
	move.l	(sp)+,a0
	rts
.ENDM
;
;----------------------------------------------------------------------------
;
XB_check_defined	set	0
;
.MACRO	XB_check	xbname,root
;
	ifne	XB_check_defined==0
	bra	XB_check_code_end
;
XB_check_code:
	movem.l	a3/a4,-(sp)
	movem.l	12(sp),a3/a4	;a3 -> struct  a4 -> chain root
;
	ifne	XB_cpu_mode==2
	subq	#4,sp		;reserve result storage on stack
	gemdos	Super,1.w	;User or Super mode call ?
	tst.l	d0
	bmi.s	.done_super_1
	gemdos	Super,!
.done_super_1:			;here CPU is in supervisor state
	move.l	d0,-(sp)	;push -1 or previous SSP
	endc
;
	ifne	XB_cpu_mode==0
	subq	#4,sp		;reserve result storage on stack
	gemdos	Super,!
	move.l	d0,-(sp)	;push previous SSP
	endc
;
	move	SR,d2		;d2 = entry interrupt mask
	or	#$0700,SR	;disable interrupts
.search_lp:
	move.l	a4,d0
	bset	#31,d0			;flag vector missing as MI
	move.l	(a4),d1
	beq.s	.done_search		;exit if vector missing
	move.l	d1,a4
	subq	#xb_code-xb_next,a4
	cmpi.l	#'XBRA',xb_magic-xb_next(a4)
	bne.s	.done_search		;exit if chain broken
	move.l	xb_id-xb_next(a4),d1	;d1 = current id
	cmp.l	xb_id(a3),d1		;d1 == sought id ?
	bne.s	.search_lp		;loop back to check remaining chain
	bclr	#31,d0		;flag vector found as PL
.done_search:
	move	d2,SR		;restore entry interrupt mask
;
	ifne	XB_cpu_mode==2
	move.l	d0,4(sp)	;store search result in stack
	move.l	(sp)+,d0
	bmi.s	.done_super_2
	gemdos	Super|_ind,d0
.done_super_2:			;here CPU is back in entry state
	move.l		(sp)+,d0	;d0 = search result from stack
	endc
;
	ifne	XB_cpu_mode==1
	tst.l	d0
	endc
;
	ifne	XB_cpu_mode==0
	move.l		d0,4(sp)	;store search result in stack
	gemdos.1	Super,__ARG__on_stack
	move.l		(sp)+,d0	;d0 = search result from stack
	endc
;
	movem.l	(sp)+,a3/a4
	rts	;d0=found_vector  or  (last_chain_vector+1<<31)
XB_check_code_end:
	endc
;
	pea	root
	pea	xbname
	bsr	XB_check_code
	addq	#8,sp
;
XB_check_defined	set	XB_check_defined+1
.ENDM	;d0=found_vector/(last_chain_vector+1<<31) flagged PL/MI
;
;----------------------------------------------------------------------------
;
XB_install_defined	set	0
;
.MACRO	XB_install	xbname,root

	pea	root
	pea	xbname
	ifne	XB_install_defined
	bsr	XB_install_code
	addq	#8,sp
	.else
	bsr.s	XB_install_code
	addq	#8,sp
	bra	XB_install_code_end
;
XB_install_code:
	movem.l	a3/a4,-(sp)
	movem.l	12(sp),a3/a4	;a3 -> struct  a4 -> chain root
;
	ifne	XB_cpu_mode==2
	subq	#4,sp		;reserve result storage on stack
	gemdos	Super,1.w	;User or Super mode call ?
	tst.l	d0
	bmi.s	.done_super_1
	gemdos	Super,!
.done_super_1:			;here CPU is in supervisor state
	move.l	d0,-(sp)	;push -1 or previous SSP
	endc
;
	ifne	XB_cpu_mode==0
	subq	#4,sp		;reserve result storage on stack
	gemdos	Super,!
	move.l	d0,-(sp)	;push previous SSP
	endc
;
	move.l	a3,a1
	move.l	a4,a2
	move	SR,d2		;d2 = entry interrupt mask
	or	#$0700,SR	;disable interrupts
.search_lp:
	move.l	a4,d0
	bset	#31,d0			;flag vector missing as MI
	move.l	(a4),d1
	beq.s	.done_search		;exit if vector missing
	move.l	d1,a4
	subq	#xb_code-xb_next,a4
	cmpi.l	#'XBRA',xb_magic-xb_next(a4)
	bne.s	.done_search		;exit if chain broken
	move.l	xb_id-xb_next(a4),d1	;d1 = current id
	cmp.l	xb_id(a3),d1		;d1 == sought id ?
	bne.s	.search_lp		;loop back to check remaining chain
	bclr	#31,d0		;flag vector found as PL
.done_search:
	tst.l	d0
	bpl.s	.done_install	;branch if already installed
	addq.w  #xb_next,a1
	move.l	(a2),(a1)+	;link new XBRA to old chain
	move.l	a1,(a2)		;store -> new XBRA as chain root
.done_install:
	move	d2,SR		;restore entry interrupt mask
;
	ifne	XB_cpu_mode==2
	move.l	d0,4(sp)	;store search result in stack
	move.l	(sp)+,d0
	bmi.s	.done_super_2
	gemdos	Super|_ind,d0
.done_super_2:			;here CPU is back in entry state
	move.l		(sp)+,d0	;d0 = search result from stack
	endc
;
	ifne	XB_cpu_mode==1
	tst.l	d0		;flag result in CCR
	endc
;
	ifne	XB_cpu_mode==0
	move.l		d0,4(sp)	;store search result in stack
	gemdos.1	Super,__ARG__on_stack
	move.l		(sp)+,d0	;d0 = search result from stack
	endc
;
	movem.l	(sp)+,a3/a4
	rts	;d0=found_vector  or  (last_chain_vector+1<<31)

XB_install_defined	set	1

XB_install_code_end:
	endc
;
.ENDM
;
;----------------------------------------------------------------------------
;
XB_remove_defined	set	0
;
.MACRO	XB_remove	xbname,root
	pea	root
	pea	xbname

	ifne	XB_remove_defined
	bsr	XB_remove_code
	addq	#8,sp
	.else
	bsr.s	XB_remove_code
	addq	#8,sp
	bra	XB_remove_code_end

;
XB_remove_code:
	movem.l	a3/a4,-(sp)
	movem.l	12(sp),a3/a4	;a3 -> struct  a4 -> chain root
;
	ifne	XB_cpu_mode==2
	subq	#4,sp		;reserve result storage on stack
	gemdos	Super,1.w	;User or Super mode call ?
	tst.l	d0
	bmi.s	.done_super_1
	gemdos	Super,!
.done_super_1:			;here CPU is in supervisor state
	move.l	d0,-(sp)	;push -1 or previous SSP
	endc
;
	ifne	XB_cpu_mode==0
	subq	#4,sp		;reserve result storage on stack
	gemdos	Super,!
	move.l	d0,-(sp)	;push previous SSP
	endc
;
	move.l	a3,a1
	move.l	a4,a2
	move	SR,d2		;d2 = entry interrupt mask
	or	#$0700,SR	;disable interrupts
.search_lp:
	move.l	a4,d0
	bset	#31,d0			;flag vector missing as MI
	move.l	(a4),d1
	beq.s	.done_search		;exit if vector missing
	move.l	d1,a4
	subq	#xb_code-xb_next,a4
	cmpi.l	#'XBRA',xb_magic-xb_next(a4)
	bne.s	.done_search		;exit if chain broken
	move.l	xb_id-xb_next(a4),d1	;d1 = current id
	cmp.l	xb_id(a3),d1		;d1 == sought id ?
	bne.s	.search_lp		;loop back to check remaining chain
	bclr	#31,d0		;flag vector found as PL
.done_search:
	tst.l	d0
	bmi.s	.done_remove
	move.l	d0,a0				;a0 -> link -> found XBRA
	move.l	(a0),a1				;a1 -> xb_code of found XBRA
	move.l	xb_next-xb_code(a1),(a0)	;unlink found XBRA from chain
.done_remove:
	move	d2,SR		;restore entry interrupt mask
;
	ifne	XB_cpu_mode==2
	move.l	d0,4(sp)	;store search result in stack
	move.l	(sp)+,d0
	bmi.s	.done_super_2
	gemdos	Super|_ind,d0
.done_super_2:			;here CPU is back in entry state
	move.l		(sp)+,d0	;d0 = search result from stack
	endc
;
	ifne	XB_cpu_mode==1
	tst.l	d0		;flag result in CCR
	endc
;
	ifne	XB_cpu_mode==0
	move.l		d0,4(sp)	;store search result in stack
	gemdos.1	Super,__ARG__on_stack
	move.l		(sp)+,d0	;d0 = search result from stack
	endc
;
	movem.l	(sp)+,a3/a4
	rts	;d0=found_vector  or  (last_chain_vector+1<<31)

XB_remove_defined	set	1
;
XB_remove_code_end:
	endc
;
.ENDM
;
;----------------------------------------------------------------------------
; End of file:	URAn_XB.SH
;----------------------------------------------------------------------------
