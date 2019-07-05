;----------------------------------------------------------------------------
;File name:	URAn_JAR.SH			Revision date:	1997.08.08
;Creator:	Ulf Ronald Andersson		Creation date:	1992.11.15
;(c)1992 by:	Ulf Ronald Andersson		All rights reserved
;Released as:	FREEWARE			(commercial sale forbidden)
;----------------------------------------------------------------------------
;Purpose:	Macro library for cookie jar operations
;----------------------------------------------------------------------------
;Required header declarations:
;
;	include	"uran\struct.sh"
;	include	"uran\uran_sys.sh"
;	include	"uran\uran_dos.sh"
;	include	"uran\uran_jar.sh"
;
;----------------------------------------------------------------------------
;	Library macros:
;
;get_jar_size			=> d0  = Total number of entries in jar
;get_jar_used			=> d0  = Number of used entries incl endmark
;seek_cookie	string		=> d0 -> cookie/err_code flagged PL/MI
;eval_cookie	string		=> d0  = cookie_value/err_code flagged PL/MI
;make_cookie	string,value	=> d0 -> new cookie/=err_code flagged PL/MI
;edit_cookie	string,value	=> d0 -> edited cookie/=err flagged PL/MI
;extend_jar	add_entries	=> creates new enlarged cookie jar
;set_new_jar	jar_ptr		=> activates new jar and its reset routine
;
;'make_cookie' trashes d1, all others only affect d0 + flags.
;These macros should only be used in supervisor mode.
;----------------------------------------------------------------------------
;NB: You must include the following line somewhere after the macro calls
;	"make	JAR_links"
;NB: That will cause the needed routines to be included at that point.
;NB: You may do this several times (after further calls), and at each
;NB: point only the 'newly' referenced routines will be added to the code.
;NB: A dummy reference macro exists, so routines can be added to code even
;NB: without calling them.  The macro is '_uniref', used as below:
;	"_uniref reset_old_jar"
;NB: "reset_old_jar" is the only part of a cookie program that _must_ remain
;NB: resident until reset when the jar has been extended.
;NB: To simplify this, its code is location independent.
;----------------------------------------------------------------------------
;	Macro definitions:
;
.MACRO	JAR_links
	_unidef	get_jar_size,get_jar_used,seek_cookie
	_unidef	eval_cookie,edit_cookie,make_cookie
	_unidef	extend_jar,set_new_jar,reset_old_jar
.ENDM	JAR_links
;
	_unidec	get_jar_size
	_unidec	get_jar_used
	_unidec	seek_cookie
	_unidec	eval_cookie
	_unidec	edit_cookie
	_unidec	make_cookie
	_unidec	extend_jar
	_unidec	set_new_jar
	_unidec	reset_old_jar
;
.MACRO	get_jar_size
	_uniref	get_jar_size
	bsr	code_get_jar_size
.ENDM	get_jar_size
;
.MACRO	code_get_jar_size
	move.l	a0,-(sp)
	move.l	(_cookies).w,d0
op_isym	beq.s,.done,uni__v
	move.l	d0,a0
sl_isym	.loop,uni__v
	tst.l	(a0)
op_isym	beq.s,.atend,uni__v
	addq.l	#8,a0
op_isym	bra.s,.loop,uni__v
;
sl_isym	.atend,uni__v
	move.l	4(a0),d0
sl_isym	.done,uni__v
	move.l	(sp)+,a0
	rts
uni__v	=	uni__v+1
.ENDM	code_get_jar_size
;
;
.MACRO	get_jar_used
	_uniref	get_jar_used
	bsr	code_get_jar_used
.ENDM	get_jar_used
;
.MACRO	code_get_jar_used
	move.l	a0,-(sp)
	move.l	(_cookies).w,d0
op_isym	beq.s,.done,uni__v
	move.l	d0,a0
	clr	d0
sl_isym	.loop,uni__v
	addq	#1,d0
	tst.l	(a0)
op_isym	beq.s,.done,uni__v
	addq.l	#8,a0
op_isym	bra.s,.loop,uni__v
;
sl_isym	.done,uni__v
	move.l	(sp)+,a0
	rts
uni__v	=	uni__v+1
.ENDM	code_get_jar_used
;
;
.MACRO	seek_cookie	name
	_uniref	seek_cookie
	move.l	name,d0
	bsr	code_seek_cookie
.ENDM	seek_cookie
;
.MACRO	code_seek_cookie
	movem.l	d6-d7/a0,-(sp)
	move.l	d0,d7
	moveq	#-34,d0		;prep 'path not found' error
	move.l	(_cookies).w,d6
op_isym	beq.s,.exit,uni__v
	move.l	d6,a0
	moveq	#-33,d0		;prep 'file not found' error
sl_isym	.loop,uni__v
	tst.l	(a0)
op_isym	beq.s,.exit,uni__v
	cmp.l	(a0),d7
op_isym	beq.s,.found,uni__v
	addq.l	#8,a0
op_isym	bra.s,.loop,uni__v
;
sl_isym	.found,uni__v
	move.l	a0,d0
sl_isym	.exit,uni__v
	tst.l	d0
	movem.l	(sp)+,d6-d7/a0
	rts		;return pl,d0->cookie or mi,d0=err_code
uni__v	=	uni__v+1
.ENDM	code_seek_cookie
;
;
.MACRO	eval_cookie	name
	_uniref	eval_cookie
	move.l	name,d0
	bsr	code_eval_cookie
.ENDM	eval_cookie
;
.MACRO	code_eval_cookie
	move.l	a0,-(sp)
	seek_cookie	d0
op_isym	bmi.s,.exit,uni__v
	move.l	d0,a0		;a0 -> cookie   (but still flagging PL)
	movem.l	4(a0),d0	;d0  = cookie value (still flagging PL)
sl_isym	.exit,uni__v
	move.l	(sp)+,a0
	rts		;return  pl,d0==cookie_value  or  mi,d0==err_code
uni__v	=	uni__v+1
.ENDM	code_eval_cookie
;
;
.MACRO	edit_cookie	name,value
	_uniref	edit_cookie
	move.l	name,d0
	move.l	value,d1
	bsr	code_edit_cookie
.ENDM	edit_cookie
;
.MACRO	code_edit_cookie
	move.l	a0,-(sp)
	seek_cookie	d0
op_isym	bmi.s,.exit,uni__v
	move.l	d0,a0		;a0 -> cookie   (still flagging PL)
	movem.l	d1,4(a0)	;cookie value = d1 (still flagging PL)
sl_isym	.exit,uni__v
	move.l	(sp)+,a0
	rts		;return  pl,d0->cookie  or  mi,d0==err_code
uni__v	=	uni__v+1
.ENDM	code_edit_cookie
;
;
.MACRO	make_cookie	name,value
	_uniref	make_cookie
	move.l	name,d0
	move.l	value,d1
	bsr	code_make_cookie
.ENDM	make_cookie
;
.MACRO	code_make_cookie
	movem.l	d2-d3/a0,-(sp)
	move.l	d0,d2
	seek_cookie	d0
	bmi.s	.make_cookie_new
	moveq	#E~ACCDN,d0		;old cookie => 'access illegal/denied'
	bra.s	.make_cookie_exit
;
.make_cookie_new:
	get_jar_size
	move	d0,d3
	get_jar_used
	sub	d0,d3
	bgt.s	.have_room
	extend_jar	#32
	bmi.s	.make_cookie_exit
	get_jar_used
.have_room:
	subq	#1,d0
	lsl.w	#3,d0
	move.l	(_cookies).w,a0
	adda	d0,a0
	clr.l	8(a0)
	move.l	4(a0),12(a0)
	move.l	d2,(a0)
	move.l	d1,4(a0)
	move.l	a0,d0
.make_cookie_exit:
	tst.l	d0
	movem.l	(sp)+,d2-d3/a0
	rts	;return  pl,d0->cookie  or  mi,d0==err_code
.ENDM	code_make_cookie
;
;
.MACRO	extend_jar	add_entries
	_uniref	extend_jar
	move.l	add_entries,d0
	bsr	code_extend_jar
.ENDM	extend_jar
;
.MACRO	code_extend_jar
	movem.l	d1-d7/a0-a6,-(sp)
	andi.l	#$FFFF,d0
	move.l	d0,d7			;d7 = size of jar extension
	get_jar_size			;d0 = size of old jar (in entries)
	add.l	d7,d0			;d0 = size of new jar (in entries)
	move.l	d0,d7			;d7 = size of new jar (in entries)
	lsl.w	#3,d0
	gemdos	Malloc,d0
	tst.l	d0
	bmi.s	.extend_cookie_exit
	move.l	d0,a2			;a2 -> new_jar
	move.l	d0,a0			;a0 -> new_jar
	move.l	(_cookies).w,a1		;a1 -> old_jar
	get_jar_used			;d0 = used size of jar
	subq	#1,d0			;prep for dbra
	bmi.s	.set_limit		;skip move if no jar
	bra.s	.move_next		;do move excluding limit_mark
;
.move_loop:
	move.l	(a1)+,(a0)+
	move.l	(a1)+,(a0)+
.move_next:
	dbra	d0,.move_loop
.set_limit:
	clr.l	(a0)+
	move.l	d7,(a0)
	move.l	a2,d0
	set_new_jar	d0
.extend_cookie_exit:
	tst.l	d0
	movem.l	(sp)+,d1-d7/a0-a6
	rts	;return  pl,d0->cookie_jar  or  mi,d0==err_code
.ENDM	code_extend_jar
;
;----------------------------------------------------------------------------
;	Here is the XBRA_linked reset routine to remove altered cookie jar,
;	with the necessary variables, and the installation routine
;
.MACRO	set_new_jar	jar_ptr
	_uniref	set_new_jar
	move.l	jar_ptr,d0
	bsr	code_set_new_jar
.ENDM	set_new_jar
;
.MACRO	code_set_new_jar
	_uniref	reset_old_jar
	move.l	(_cookies).w,_cookies_save
	move.l	(resvalid).w,resvalid_save
	move.l	(resvector).w,resvector_save
	move.l	#uncookie_reset,(resvector).w
	move.l	#$31415926,(resvalid).w
	move.l	d0,(_cookies).w		;activate new_jar
	rts
.ENDM	code_set_new_jar
;
.MACRO	code_reset_old_jar
_cookies_save:		dc.l	0
resvalid_save:		dc.l	0
			dc.l	'XBRA','UnCk'
resvector_save:		dc.l	0
uncookie_reset:
	move.l	_cookies_save(pc),(_cookies).w
	move.l	resvector_save(pc),(resvector).w
	move.l	resvalid_save(pc),(resvalid).w
	jmp	(a6)	;NB:reset routine can not use standard linking
.ENDM	code_reset_old_jar
;
;----------------------------------------------------------------------------
;end of file:	URAn_JAR.SH
;----------------------------------------------------------------------------
