;----------------------------------------------------------------------------
;File name:	URAn_LA.SH			Revision date:	1997.08.08
;Creator:	Ulf Ronald Andersson		Creation date:	1992.11.30
;(c)1992 by:	Ulf Ronald Andersson		All rights reserved
;Released as:	FREEWARE			(commercial sale forbidden)
;----------------------------------------------------------------------------
;Purpose:	Symbol & Macro library for line_A operations
;----------------------------------------------------------------------------
;Required header declarations:
;
;	include	"uran\struct.sh"
;	include	"uran\uran_sys.sh"
;	include	"uran\uran_la.sh"
;
;----------------------------------------------------------------------------
;	Line_a opcode macros:
;
;_a_init
;_a_put_pixel
;_a_get_pixel
;_a_line
;_a_hline
;_a_bar		or 	_a_recfl
;_a_pline
;_a_bitblt
;_a_textblt
;_a_show_c	or	_a_showm
;_a_hide_c	or	_a_hidem
;_a_form
;_a_undraw
;_a_draw
;_a_cpyfm
;_a_contour
;
;linea #n	;special concession to DS_DRAIN
;----------------------------------------------------------------------------
;	Line-A opcodes
;
la_op_init	equ	$a000
la_op_put_pixel	equ	$a001
la_op_get_pixel	equ	$a002
la_op_line	equ	$a003
la_op_hline	equ	$a004
la_op_recfl	equ	$a005
la_op_pline	equ	$a006
la_op_bitblt	equ	$a007
la_op_textblt	equ	$a008
la_op_show_c	equ	$a009
la_op_hide_c	equ	$a00a
la_op_form	equ	$a00b
la_op_undraw	equ	$a00c
la_op_draw	equ	$a00d
la_op_cpyfm	equ	$a00e
la_op_contour	equ	$a00f	;(post TOS 1.0)
;
;	Alternative opcode names
;
la_op_putpix	equ	$a001
la_op_getpix	equ	$a002
la_op_abline	equ	$a003
la_op_habline	equ	$a004
la_op_rectfill	equ	$a005
la_op_polyfill	equ	$a006
la_op_showm	equ	$a009
la_op_showcur	equ	$a009
la_op_hidem	equ	$a00a
la_op_hidecur	equ	$a00a
la_op_chgcur	equ	$a00b
la_op_unsprite	equ	$a00c
la_op_drsprite	equ	$a00d
la_op_copyrstr	equ	$a00e
la_op_seedfill	equ	$a00f	;(post TOS 1.0)
;
;----------------------------------------------------------------------------
;	BITBLT data structure (for ptr in a6)
;
	struct	blt_struct
	d_w	blt_b_wd	;00 block width  [Pixels]
	d_w	blt_b_ht	;02 block height [Pixels]
	d_w	blt_plane_ct	;04 bit planes
	d_w	blt_fg_col	;06 foreground colour
	d_w	blt_bg_col	;08 background colour
	char	blt_op_tab,4	;10 4 blitter opcodes
;
	d_w	blt_s_xmin	;14 source X offset within form [Pixels]
	d_w	blt_s_ymin	;16 source Y offset within form [Pixels]
	d_l	blt_s_form	;18 source form base address
	d_w	blt_s_nxwd	;22 offset between words of a plane  [Bytes]
	d_w	blt_s_nxln	;24 offset between source lines      [Bytes]
	d_w	blt_s_nxpl	;26 offset between source bit planes [Bytes]
;
	d_w	blt_d_xmin	;28 dest X offset within form [Pixels]
	d_w	blt_d_ymin	;30 dest Y offset within form [Pixels]
	d_l	blt_d_form	;32 dest form base address
	d_w	blt_d_nxwd	;36 offset between words of a plane [Bytes]
	d_w	blt_d_nxln	;38 offset between dest lines       [Bytes]
	d_w	blt_d_nxpl	;40 offset between dest bit planes  [Bytes]
;
	d_l	blt_p_addr	;42 source mask pattern address (0=unused) *1
	d_w	blt_p_nxln	;46 offset between pattern lines [2^x Bytes]
	d_w	blt_p_nxpl	;48 offset between pattern bit planes [Bytes]
	d_w	blt_p_mask	;50 pattern index mask [(2^x)*(2^y)-1]
;
	char	blt_work,24	;52 work area for BITBLT routines
	d_alias	blt_size	;"blt_" structure size = 76 Bytes
	d_end	blt_struct
;
;----------------------------------------------------------------------------
;	Line-A Variables
;
la_cur_font	equ	-$38A	;->
;-$386 .. -$359 are reserved
la_m_pos_hx	equ	-$358	;w
la_m_pos_hy	equ	-$356	;w
la_m_planes	equ	-$354	;w
la_m_cdb_bg	equ	-$352	;w
la_m_cdb_fg	equ	-$350	;w
la_mask_form	equ	-$34e	;w*32
la_inq_tab	equ	-$30e	;w*45
;
la_inq_bgcolours	equ	-$30C	;w
la_inq_bgcolors	equ	-$30C	;w
;
la_inq_planes	equ	-$306	;w
la_inq_lookup_f	equ	-$304	;w
la_inq_op_speed	equ	-$302	;w
;
la_dev_tab	equ	-$2b4	;w*45
la_workout	equ	-$2B4	;w*45
la_wk_xmax	equ	-$2B4	;w
la_wk_ymax	equ	-$2B2	;w
la_wk_coord_f	equ	-$2B0	;w
la_wk_pixwidth	equ	-$2AE	;w
la_wk_pixheight	equ	-$2AC	;w
la_wk_t_sizes	equ	-$2AA	;w
la_wk_l_types	equ	-$2A8	;w
la_wk_l_widths	equ	-$2A6	;w
la_wk_m_types	equ	-$2A4	;w
la_wk_m_sizes	equ	-$2A2	;w
la_wk_t_fonts	equ	-$2A0	;w
la_wk_f_p_pats	equ	-$29E	;w
la_wk_f_l_pats	equ	-$29C	;w
la_wk_maxcolours	equ	-$29A	;w
la_wk_maxcolors	equ	-$29A	;w
la_wk_palregs	equ	-$29A	;w
;
la_wk_colour_f	equ	-$26E
la_wk_color_f	equ	-$26E
;
la_wk_palcolours	equ	-$266	;w
la_wk_palcolors	equ	-$266	;w
la_wk_palvalues	equ	-$266	;w
;
la_gcurx	equ	-$25a	;w
la_gcury	equ	-$258	;w
la_m_hid_ct	equ	-$256	;w
la_mouse_bt	equ	-$254	;w
la_req_col	equ	-$252	;w*3*16
la_siz_tab	equ	-$1f2	;w*15
;-$1d4 .. -$1d1 are reserved
la_cur_work	equ	-$1d0	;->
la_def_font	equ	-$1cc	;->
;
la_font_ring	equ	-$1c8	;->*4
la_font_gdos	equ	-$1c0	;->
;
la_font_count	equ	-$1b8	;w
;-$1b6 .. -$15d are reserved
la_cur_ms_stat	equ	-$15c	;b
;-$15b is reserved
la_v_hid_cnt	equ	-$15a	;w
la_cur_x	equ	-$158	;w
la_cur_y	equ	-$156	;w
la_cur_flag	equ	-$154	;b
la_mouse_flag	equ	-$153	;b
;-$152 .. -$14f	are reserved
la_v_sav_xy	equ	-$14e	;w*2
la_save_len	equ	-$14a	;w
la_save_addr	equ	-$148	;->
la_save_stat	equ	-$144	;w
la_save_area	equ	-$142	;b*256
la_user_tim	equ	-$42	;->
la_next_tim	equ	-$3e	;->
la_user_but	equ	-$3a	;->
la_user_cur	equ	-$36	;->
la_user_mot	equ	-$32	;->
la_v_cel_ht	equ	-$2E	;w
la_v_cel_mx	equ	-$2C	;w
la_v_cel_my	equ	-$2A	;w
la_v_cel_wr	equ	-$28	;w
la_v_col_bg	equ	-$26	;w
la_v_col_fg	equ	-$24	;w
la_v_cur_ad	equ	-$22	;w
la_v_cur_of	equ	-$1E	;w
;
la_v_cur_xy	equ	-$1C	;w*2
la_v_cur_x	equ	-$1C	;w
la_v_cur_y	equ	-$1A	;w
;
la_v_period	equ	-$18	;b
la_v_cur_ct	equ	-$17	;b
la_v_fnt_ad	equ	-$16	;->
la_v_fnt_nd	equ	-$12	;w
la_v_fnt_st	equ	-$10	;w
la_v_fnt_wd	equ	-$0e	;w
la_v_rez_hz	equ	-$0C	;w
la_v_off_ad	equ	-$0a	;->
;-$06 .. -$05 are reserved
la_resv_n_1	equ	-$06
la_v_rez_vt	equ	-$04	;w
la_bytes_lin	equ	-$02	;w
la_planes	equ	$00	;w
la_width	equ	$02	;w
;
la_vdipb	equ	$04	;->*5
la_contrl_p	equ	$04	;->
la_intin_p	equ	$08	;->
la_ptsin_p	equ	$0C	;->
la_intout_p	equ	$10	;->
la_ptsout_p	equ	$14	;->
;
la_colbit0	equ	$18	;w
la_colbit1	equ	$1A	;w
la_colbit2	equ	$1C	;w
la_colbit3	equ	$1E	;w
la_lstlin	equ	$20	;w
la_lnmask	equ	$22	;w
la_wmode	equ	$24	;w
la_x1		equ	$26	;w
la_y1		equ	$28	;w
la_x2		equ	$2A	;w
la_y2		equ	$2C	;w
la_patptr	equ	$2E	;l
la_patmsk	equ	$32	;w
la_mfill	equ	$34	;w
la_clip		equ	$36	;w
;
la_cliprect	equ	$38	;w*4
la_xmincl	equ	$38	;w
la_ymincl	equ	$3a	;w
la_xmaxcl	equ	$3c	;w
la_ymaxcl	equ	$3e	;w
;
la_xdda		equ	$40	;w
la_ddainc	equ	$42	;w
la_scaldir	equ	$44	;w
la_mono		equ	$46	;w
la_sourcex	equ	$48	;w
la_sourcey	equ	$4a	;w
la_destx	equ	$4c	;w
la_desty	equ	$4e	;w
la_delx		equ	$50	;w
la_dely		equ	$52	;w
la_fbase	equ	$54	;->
la_fwidth	equ	$58	;w
la_style	equ	$5a	;w
la_litemask	equ	$5c	;w
la_skewmask	equ	$5e	;w
la_weight	equ	$60	;w
la_roff		equ	$62	;w
la_loff		equ	$64	;w
la_scale	equ	$66	;w
la_chup		equ	$68	;w
la_textfg	equ	$6a	;w
la_scrtchp	equ	$6c	;->
la_scrpt2	equ	$70	;w
la_textbg	equ	$72	;w
la_copytran	equ	$74	;w
la_seedabort	equ	$76	;-> (post TOS 1.0)
;
la_curblit_tb	equ	$7A
la_lk_scroller	equ	$7E	;-> scroll routine
la_lk_scrollend	equ	$82	;-> jump destination when scroll done
la_actblit_p	equ	$A2	;-> hard blitter link table
la_pasblit_p	equ	$A6	;-> soft blitter link table
la_blitstate	equ	$AA	;.w blitter active=3, passive=2, absent=0
;
;----------------------------------------------------------------------------
;	Line-A macros
;
	macro	_a_init
	dc.w	la_op_init
	endm
;
	macro	_a_put_pixel
	dc.w	la_op_put_pixel
	endm
;
	macro	_a_get_pixel
	dc.w	la_op_get_pixel
	endm
;
	macro	_a_line
	dc.w	la_op_line
	endm
;
	macro	_a_hline
	dc.w	la_op_hline
	endm
;
	macro	_a_recfl
	dc.w	la_op_recfl
	endm
;
	macro	_a_bar
	dc.w	la_op_recfl
	endm
;
	macro	_a_pline
	dc.w	la_op_pline
	endm
;
	macro	_a_bitblt
	dc.w	la_op_bitblt
	endm
;
	macro	_a_textblt
	dc.w	la_op_textblt
	endm
;
	macro	_a_show_c
	dc.w	la_op_show_c
	endm
;
	macro	_a_showm
	dc.w	la_op_showm
	endm
;
	macro	_a_hide_c
	dc.w	la_op_hide_c
	endm
;
	macro	_a_hidem
	dc.w	la_op_hidem
	endm
;
	macro	_a_form
	dc.w	la_op_form
	endm
;
	macro	_a_undraw
	dc.w	la_op_undraw
	endm
;
	macro	_a_draw
	dc.w	la_op_draw
	endm
;
	macro	_a_cpyfm
	dc.w	la_op_cpyfm
	endm
;
	macro	_a_contour
	dc.w	la_op_contour	;(post TOS 1.0)
	endm
;
	macro	linea	funcnum
	aline	funcnum
	endm
;
;----------------------------------------------------------------------------
;	End of file:	URAn_LA.SH
;----------------------------------------------------------------------------
