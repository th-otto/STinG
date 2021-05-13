/*
 * GEM resource C output of sting
 *
 * created by ORCS 2.18
 */

#if !defined(__GNUC__) || !defined(__mc68000__)
#include <portab.h>
#endif

#ifndef __STDC__
# ifdef __PUREC__
#  define __STDC__ 1
# endif
#endif

#ifdef OS_WINDOWS
#  include <portaes.h>
#  define SHORT _WORD
#  ifdef __WIN32__
#    define _WORD signed short
#  else
#    define _WORD signed int
 #   pragma option -zE_FARDATA
#  endif
#else
#  ifdef __TURBOC__
#    include <aes.h>
#    define CP (_WORD *)
#  endif
#endif

#ifdef OS_UNIX
#  include <portaes.h>
#  define SHORT _WORD
#else
#  ifdef __GNUC__
#    ifndef __PORTAES_H__
#      if __GNUC__ < 4
#        include <aesbind.h>
#        ifndef _WORD
#          define _WORD int
#        endif
#        define CP (char *)
#      else
#        include <mt_gem.h>
#        ifndef _WORD
#          define _WORD short
#        endif
#        define CP (short *)
#      endif
#      define CW (short *)
#    endif
#  endif
#endif


#ifdef __SOZOBONX__
#  include <xgemfast.h>
#else
#  ifdef SOZOBON
#    include <aes.h>
#  endif
#endif

#ifdef MEGAMAX
#  include <gembind.h>
#  include <gemdefs.h>
#  include <obdefs.h>
#  define _WORD int
#  define SHORT int
#endif

#ifndef _VOID
#  define _VOID void
#endif

#ifndef OS_NORMAL
#  define OS_NORMAL 0x0000
#endif
#ifndef OS_SELECTED
#  define OS_SELECTED 0x0001
#endif
#ifndef OS_CROSSED
#  define OS_CROSSED 0x0002
#endif
#ifndef OS_CHECKED
#  define OS_CHECKED 0x0004
#endif
#ifndef OS_DISABLED
#  define OS_DISABLED 0x0008
#endif
#ifndef OS_OUTLINED
#  define OS_OUTLINED 0x0010
#endif
#ifndef OS_SHADOWED
#  define OS_SHADOWED 0x0020
#endif
#ifndef OS_WHITEBAK
#  define OS_WHITEBAK 0x0040
#endif
#ifndef OS_DRAW3D
#  define OS_DRAW3D 0x0080
#endif

#ifndef OF_NONE
#  define OF_NONE 0x0000
#endif
#ifndef OF_SELECTABLE
#  define OF_SELECTABLE 0x0001
#endif
#ifndef OF_DEFAULT
#  define OF_DEFAULT 0x0002
#endif
#ifndef OF_EXIT
#  define OF_EXIT 0x0004
#endif
#ifndef OF_EDITABLE
#  define OF_EDITABLE 0x0008
#endif
#ifndef OF_RBUTTON
#  define OF_RBUTTON 0x0010
#endif
#ifndef OF_LASTOB
#  define OF_LASTOB 0x0020
#endif
#ifndef OF_TOUCHEXIT
#  define OF_TOUCHEXIT 0x0040
#endif
#ifndef OF_HIDETREE
#  define OF_HIDETREE 0x0080
#endif
#ifndef OF_INDIRECT
#  define OF_INDIRECT 0x0100
#endif
#ifndef OF_FL3DIND
#  define OF_FL3DIND 0x0200
#endif
#ifndef OF_FL3DBAK
#  define OF_FL3DBAK 0x0400
#endif
#ifndef OF_FL3DACT
#  define OF_FL3DACT 0x0600
#endif
#ifndef OF_MOVEABLE
#  define OF_MOVEABLE 0x0800
#endif
#ifndef OF_POPUP
#  define OF_POPUP 0x1000
#endif

#ifndef R_CICONBLK
#  define R_CICONBLK 17
#endif
#ifndef R_CICON
#  define R_CICON 18
#endif

#ifndef G_SWBUTTON
#  define G_SWBUTTON 34
#endif
#ifndef G_POPUP
#  define G_POPUP 35
#endif
#ifndef G_EDIT
#  define G_EDIT 37
#endif
#ifndef G_SHORTCUT
#  define G_SHORTCUT 38
#endif
#ifndef G_SLIST
#  define G_SLIST 39
#endif
#ifndef G_EXTBOX
#  define G_EXTBOX 40
#endif
#ifndef G_OBLINK
#  define G_OBLINK 41
#endif

#ifndef _WORD
#  ifdef WORD
#    define _WORD WORD
#  else
#    ifdef __PUREC__
#      define _WORD int
#    else
#      define _WORD short
#    endif
#  endif
#endif

#ifndef _UBYTE
#  define _UBYTE char
#endif

#ifndef _BOOL
#  define _BOOL int
#endif

#ifndef _LONG
#  ifdef LONG
#    define _LONG LONG
#  else
#    define _LONG long
#  endif
#endif

#ifndef _ULONG
#  ifdef ULONG
#    define _ULONG ULONG
#  else
#    define _ULONG unsigned long
#  endif
#endif

#ifndef _LONG_PTR
#  define _LONG_PTR _LONG
#endif

#ifndef C_UNION
#ifdef __PORTAES_H__
#  define C_UNION(x) { (_LONG_PTR)(x) }
#endif
#ifdef __GEMLIB__
#  define C_UNION(x) { (_LONG_PTR)(x) }
#endif
#ifdef __PUREC__
#  define C_UNION(x) { (_LONG_PTR)(x) }
#endif
#ifdef __ALCYON__
#  define C_UNION(x) x
#endif
#endif
#ifndef C_UNION
#  define C_UNION(x) (_LONG_PTR)(x)
#endif

#ifndef SHORT
#  define SHORT short
#endif

#ifndef CP
#  define CP (SHORT *)
#endif

#ifndef CW
#  define CW (_WORD *)
#endif


#undef RSC_STATIC_FILE
#define RSC_STATIC_FILE 1

#include "sting.h"

#ifndef RSC_NAMED_FUNCTIONS
#  define RSC_NAMED_FUNCTIONS 0
#endif

#ifndef __ALCYON__
#undef defRSHInit
#undef defRSHInitBit
#undef defRSHInitStr
#ifndef RsArraySize
#define RsArraySize(array) (_WORD)(sizeof(array)/sizeof(array[0]))
#define RsPtrArraySize(type, array) (type *)array, RsArraySize(array)
#endif
#define defRSHInit( aa, bb ) RSHInit( aa, bb, RsPtrArraySize(OBJECT *, rs_trindex), RsArraySize(rs_object) )
#define defRSHInitBit( aa, bb ) RSHInitBit( aa, bb, RsPtrArraySize(BITBLK *, rs_frimg) )
#define defRSHInitStr( aa, bb ) RSHInitStr( aa, bb, RsPtrArraySize(_UBYTE *, rs_frstr) )
#endif

#ifdef __STDC__
#ifndef W_Cicon_Setpalette
extern _BOOL W_Cicon_Setpalette(_WORD *_palette);
#endif
#ifndef hrelease_objs
extern void hrelease_objs(OBJECT *_ob, _WORD _num_objs);
#endif
#ifndef hfix_objs
extern void *hfix_objs(RSHDR *_hdr, OBJECT *_ob, _WORD _num_objs);
#endif
#endif

#ifndef RLOCAL
#  if RSC_STATIC_FILE
#    ifdef LOCAL
#      define RLOCAL LOCAL
#    else
#      define RLOCAL static
#    endif
#  else
#    define RLOCAL
#  endif
#endif


#ifndef N_
#  define N_(x)
#endif


#if RSC_STATIC_FILE
#undef NUM_STRINGS
#undef NUM_BB
#undef NUM_IB
#undef NUM_CIB
#undef NUM_CIC
#undef NUM_TI
#undef NUM_FRSTR
#undef NUM_FRIMG
#undef NUM_OBS
#undef NUM_TREE
#undef NUM_UD
#define NUM_STRINGS 49
#define NUM_BB		0
#define NUM_IB		0
#define NUM_CIB     0
#define NUM_CIC     0
#define NUM_TI		11
#define NUM_FRSTR	0
#define NUM_FRIMG	0
#define NUM_OBS     44
#define NUM_TREE	1
#define NUM_UD		0
#endif


static char sting_string_0[] = "STinG V00.01";
static char sting_string_1[] = "TCP.......";
static char sting_string_2[] = "";
static char sting_string_3[] = "Address Mask Control";
static char sting_string_4[] = "";
static char sting_string_5[] = "___";
static char sting_string_6[] = "Router Advert. :  ___ min";
static char sting_string_7[] = "9";
static char sting_string_8[] = "____";
static char sting_string_9[] = "Lag behind GMT : ____ min";
static char sting_string_10[] = "X";
static char sting_string_11[] = "_____";
static char sting_string_12[] = "Max. Segment Size : _____";
static char sting_string_13[] = "9";
static char sting_string_14[] = "_____";
static char sting_string_15[] = "Receive Window : _____";
static char sting_string_16[] = "9";
static char sting_string_17[] = "_____";
static char sting_string_18[] = "First local port : _____";
static char sting_string_19[] = "9";
static char sting_string_20[] = "___";
static char sting_string_21[] = "Default TTL : ___";
static char sting_string_22[] = "X";
static char sting_string_23[] = "_____";
static char sting_string_24[] = "Initial RTT : __.___ sec";
static char sting_string_25[] = "9";
static char sting_string_26[] = "";
static char sting_string_27[] = "Evaluate ICMP messages";
static char sting_string_28[] = "_____";
static char sting_string_29[] = "First local port : _____";
static char sting_string_30[] = "9";
static char sting_string_31[] = "";
static char sting_string_32[] = "Evaluate ICMP messages";
static char sting_string_33[] = "Name Server :";
static char sting_string_34[] = "123456789357";
static char sting_string_35[] = "___.___.___.___";
static char sting_string_36[] = "9";
static char sting_string_37[] = "Domain :";
static char sting_string_38[] = "____________________________";
static char sting_string_39[] = "____________________________";
static char sting_string_40[] = "X";
static char sting_string_41[] = "____";
static char sting_string_42[] = "Cache :  ____ RRs";
static char sting_string_43[] = "X";
static char sting_string_44[] = "";
static char sting_string_45[] = "Save";
static char sting_string_46[] = "Save";
static char sting_string_47[] = "Ok";
static char sting_string_48[] = "Cancel";


TEDINFO rs_tedinfo[NUM_TI] = {
	{ sting_string_5, sting_string_6, sting_string_7, IBM, 6, TE_LEFT, 0x1180, 0x0, -1, 4,26 }, /* I_RA_ED */
	{ sting_string_8, sting_string_9, sting_string_10, IBM, 6, TE_LEFT, 0x1180, 0x0, -1, 5,26 }, /* I_LAG */
	{ sting_string_11, sting_string_12, sting_string_13, IBM, 6, TE_LEFT, 0x1180, 0x0, -1, 6,26 }, /* MSS */
	{ sting_string_14, sting_string_15, sting_string_16, IBM, 6, TE_LEFT, 0x1180, 0x0, -1, 6,23 }, /* RCV_WIN */
	{ sting_string_17, sting_string_18, sting_string_19, IBM, 6, TE_LEFT, 0x1180, 0x0, -1, 6,25 }, /* T_PORT */
	{ sting_string_20, sting_string_21, sting_string_22, IBM, 6, TE_LEFT, 0x1180, 0x0, -1, 4,18 }, /* DEF_TTL */
	{ sting_string_23, sting_string_24, sting_string_25, IBM, 6, TE_LEFT, 0x1180, 0x0, -1, 6,25 }, /* RTT_INIT */
	{ sting_string_28, sting_string_29, sting_string_30, IBM, 6, TE_LEFT, 0x1180, 0x0, -1, 6,25 }, /* U_PORT */
	{ sting_string_34, sting_string_35, sting_string_36, IBM, 6, TE_LEFT, 0x1180, 0x0, -1, 13,16 }, /* DNS_IP */
	{ sting_string_38, sting_string_39, sting_string_40, IBM, 6, TE_LEFT, 0x1180, 0x0, -1, 29,29 }, /* DNS_DOM */
	{ sting_string_41, sting_string_42, sting_string_43, IBM, 6, TE_LEFT, 0x1180, 0x0, -1, 5,18 } /* DNS_CSZ */
};


OBJECT rs_object[NUM_OBS] = {
/* STING */

	{ -1, 1, 41, G_BOX, OF_NONE, OS_NORMAL, C_UNION(0x1101L), 0,0, 32,11 },
	{ 39, 2, 27, G_BOX, OF_NONE, OS_NORMAL, C_UNION(0xFF1100L), 0,0, 32,777 }, /* WHOLE */
	{ 4, 3, 3, G_BOX, OF_NONE, OS_NORMAL, C_UNION(0xFF1100L), 18,0, 14,2561 },
	{ 2, -1, -1, G_STRING, OF_NONE, OS_NORMAL, C_UNION(sting_string_0), 1,1536, 12,1 }, /* VERSION */
	{ 5, -1, -1, G_BUTTON, 0x41, OS_SHADOWED, C_UNION(sting_string_1), 2,1536, 14,1 }, /* PROTOCOL */
	{ 13, 6, 12, G_BOX, OF_NONE, OS_NORMAL, C_UNION(0x1100L), 0,2, 32,775 }, /* ICMP_BOX */
	{ 9, 7, 8, G_BOX, OF_NONE, OS_NORMAL, C_UNION(0x1100L), 2,2, 23,1 },
	{ 8, -1, -1, (1<<8)+G_BUTTON, OF_SELECTABLE, 0x6, C_UNION(sting_string_2), 0,0, 2,1 }, /* I_AMC */
	{ 6, -1, -1, G_STRING, OF_NONE, OS_NORMAL, C_UNION(sting_string_3), 3,0, 20,1 },
	{ 12, 10, 11, G_BOX, OF_NONE, OS_NORMAL, C_UNION(0x1100L), 2,2051, 28,1 },
	{ 11, -1, -1, (1<<8)+G_BUTTON, OF_SELECTABLE, 0x6, C_UNION(sting_string_4), 0,0, 2,1 }, /* I_RA */
	{ 9, -1, -1, G_FTEXT, OF_EDITABLE, OS_NORMAL, C_UNION(&rs_tedinfo[0]), 3,0, 25,1 }, /* I_RA_ED */
	{ 5, -1, -1, G_FTEXT, OF_EDITABLE, OS_NORMAL, C_UNION(&rs_tedinfo[1]), 5,5, 26,1 }, /* I_LAG */
	{ 22, 14, 19, G_BOX, OF_NONE, OS_NORMAL, C_UNION(0x1100L), 0,2, 32,775 }, /* TCP_BOX */
	{ 15, -1, -1, G_FTEXT, OF_EDITABLE, OS_NORMAL, C_UNION(&rs_tedinfo[2]), 3,512, 25,1 }, /* MSS */
	{ 16, -1, -1, G_FTEXT, OF_EDITABLE, OS_NORMAL, C_UNION(&rs_tedinfo[3]), 6,513, 22,1 }, /* RCV_WIN */
	{ 17, -1, -1, G_FTEXT, OF_EDITABLE, OS_NORMAL, C_UNION(&rs_tedinfo[4]), 4,514, 24,1 }, /* T_PORT */
	{ 18, -1, -1, G_FTEXT, OF_EDITABLE, OS_NORMAL, C_UNION(&rs_tedinfo[5]), 9,515, 17,1 }, /* DEF_TTL */
	{ 19, -1, -1, G_FTEXT, OF_EDITABLE, OS_NORMAL, C_UNION(&rs_tedinfo[6]), 5,1796, 24,1 }, /* RTT_INIT */
	{ 13, 20, 21, G_BOX, OF_NONE, OS_NORMAL, C_UNION(0x1100L), 4,3077, 25,1 },
	{ 21, -1, -1, (1<<8)+G_BUTTON, OF_SELECTABLE, 0x6, C_UNION(sting_string_26), 0,0, 2,1 }, /* T_ICMP */
	{ 19, -1, -1, G_STRING, OF_NONE, OS_NORMAL, C_UNION(sting_string_27), 3,0, 22,1 },
	{ 27, 23, 24, G_BOX, OF_NONE, OS_NORMAL, C_UNION(0x1100L), 0,2, 32,775 }, /* UDP_BOX */
	{ 24, -1, -1, G_FTEXT, OF_EDITABLE, OS_NORMAL, C_UNION(&rs_tedinfo[7]), 4,2, 24,1 }, /* U_PORT */
	{ 22, 25, 26, G_BOX, OF_NONE, OS_NORMAL, C_UNION(0x1100L), 4,4, 25,1 },
	{ 26, -1, -1, (1<<8)+G_BUTTON, OF_SELECTABLE, 0x6, C_UNION(sting_string_31), 0,0, 2,1 }, /* U_ICMP */
	{ 24, -1, -1, G_STRING, OF_NONE, OS_NORMAL, C_UNION(sting_string_32), 3,0, 22,1 },
	{ 1, 28, 35, G_BOX, OF_NONE, OS_NORMAL, C_UNION(0x1100L), 0,2, 32,775 }, /* DNS_BOX */
	{ 29, -1, -1, G_STRING, OF_NONE, OS_NORMAL, C_UNION(sting_string_33), 3,1024, 13,1 },
	{ 33, 30, 32, G_BOX, OF_NONE, OS_NORMAL, C_UNION(0x1100L), 2,2049, 21,1 },
	{ 31, -1, -1, G_BOXCHAR, 0x41, OS_NORMAL, C_UNION(0x4FF1100L), 0,0, 2,1 }, /* DNS_PREV */
	{ 32, -1, -1, G_FTEXT, OF_EDITABLE, OS_NORMAL, C_UNION(&rs_tedinfo[8]), 3,0, 15,1 }, /* DNS_IP */
	{ 29, -1, -1, G_BOXCHAR, 0x41, OS_NORMAL, C_UNION(0x3FF1100L), 19,0, 2,1 }, /* DNS_NEXT */
	{ 34, -1, -1, G_STRING, OF_NONE, OS_NORMAL, C_UNION(sting_string_37), 3,3586, 8,1 },
	{ 35, -1, -1, G_FTEXT, OF_EDITABLE, OS_NORMAL, C_UNION(&rs_tedinfo[9]), 2,516, 28,1 }, /* DNS_DOM */
	{ 27, 36, 38, G_BOX, OF_NONE, OS_NORMAL, C_UNION(0x1100L), 2,2053, 28,1 },
	{ 37, -1, -1, G_FTEXT, OF_EDITABLE, OS_NORMAL, C_UNION(&rs_tedinfo[10]), 1,0, 17,1 }, /* DNS_CSZ */
	{ 38, -1, -1, (1<<8)+G_BUTTON, OF_SELECTABLE, 0x6, C_UNION(sting_string_44), 21,0, 2,1 }, /* DNS_CSV */
	{ 35, -1, -1, G_STRING, OF_NONE, OS_NORMAL, C_UNION(sting_string_45), 24,0, 4,1 },
	{ 41, 40, 40, G_BOX, OF_NONE, OS_NORMAL, C_UNION(0xFF1100L), 0,1033, 1803,3073 },
	{ 39, -1, -1, G_BUTTON, 0x5, OS_NORMAL, C_UNION(sting_string_46), 1793,1536, 8,1 }, /* SAVE */
	{ 0, 42, 43, G_BOX, OF_NONE, OS_NORMAL, C_UNION(0xFF1100L), 12,1033, 20,3073 },
	{ 43, -1, -1, G_BUTTON, 0x7, OS_NORMAL, C_UNION(sting_string_47), 514,1536, 5,1 }, /* SET */
	{ 41, -1, -1, G_BUTTON, 0x25, OS_NORMAL, C_UNION(sting_string_48), 9,1536, 9,1 } /* CANCEL */
};


OBJECT *rs_trindex[NUM_TREE] = {
	&rs_object[0] /* STING */
};





#if RSC_STATIC_FILE

#if RSC_NAMED_FUNCTIONS
#ifdef __STDC__
_WORD sting_rsc_load(_WORD wchar, _WORD hchar)
#else
_WORD sting_rsc_load(wchar, hchar)
_WORD wchar;
_WORD wchar;
#endif
{
#ifndef RSC_HAS_PALETTE
#  define RSC_HAS_PALETTE 0
#endif
#ifndef RSC_USE_PALETTE
#  define RSC_USE_PALETTE 0
#endif
#if RSC_HAS_PALETTE || RSC_USE_PALETTE
	W_Cicon_Setpalette(&rgb_palette[0][0]);
#endif
#if NUM_OBS != 0
	{
		_WORD Obj;
		OBJECT *tree;
		for (Obj = 0, tree = rs_object; Obj < NUM_OBS; Obj++, tree++)
		{
			tree->ob_x = wchar * (tree->ob_x & 0xff) + (tree->ob_x >> 8);
			tree->ob_y = hchar * (tree->ob_y & 0xff) + (tree->ob_y >> 8);
			tree->ob_width = wchar * (tree->ob_width & 0xff) + (tree->ob_width >> 8);
			tree->ob_height = hchar * (tree->ob_height & 0xff) + (tree->ob_height >> 8);
		}
		hfix_objs(NULL, rs_object, NUM_OBS);
	}
#endif
	return 1;
}


#ifdef __STDC__
_WORD sting_rsc_gaddr(_WORD type, _WORD idx, void *gaddr)
#else
_WORD sting_rsc_gaddr(type, idx, gaddr)
_WORD type;
_WORD idx;
void *gaddr;
#endif
{
	switch (type)
	{
#if NUM_TREE != 0
	case R_TREE:
		if (idx < 0 || idx >= NUM_TREE)
			return 0;
		*((OBJECT **)gaddr) = rs_trindex[idx];
		break;
#endif
#if NUM_OBS != 0
	case R_OBJECT:
		if (idx < 0 || idx >= NUM_OBS)
			return 0;
		*((OBJECT **)gaddr) = &rs_object[idx];
		break;
#endif
#if NUM_TI != 0
	case R_TEDINFO:
		if (idx < 0 || idx >= NUM_TI)
			return 0;
		*((TEDINFO **)gaddr) = &rs_tedinfo[idx];
		break;
#endif
#if NUM_IB != 0
	case R_ICONBLK:
		if (idx < 0 || idx >= NUM_IB)
			return 0;
		*((ICONBLK **)gaddr) = &rs_iconblk[idx];
		break;
#endif
#if NUM_BB != 0
	case R_BITBLK:
		if (idx < 0 || idx >= NUM_BB)
			return 0;
		*((BITBLK **)gaddr) = &rs_bitblk[idx];
		break;
#endif
#if NUM_FRSTR != 0
	case R_STRING:
		if (idx < 0 || idx >= NUM_FRSTR)
			return 0;
		*((char **)gaddr) = (char *)(rs_frstr[idx]);
		break;
#endif
#if NUM_FRIMG != 0
	case R_IMAGEDATA:
		if (idx < 0 || idx >= NUM_FRIMG)
			return 0;
		*((BITBLK **)gaddr) = rs_frimg[idx];
		break;
#endif
#if NUM_OBS != 0
	case R_OBSPEC:
		if (idx < 0 || idx >= NUM_OBS)
			return 0;
		*((_LONG **)gaddr) = &rs_object[idx].ob_spec.index;
		break;
#endif
#if NUM_TI != 0
	case R_TEPTEXT:
		if (idx < 0 || idx >= NUM_TI)
			return 0;
		*((char ***)gaddr) = (char **)(&rs_tedinfo[idx].te_ptext);
		break;
#endif
#if NUM_TI != 0
	case R_TEPTMPLT:
		if (idx < 0 || idx >= NUM_TI)
			return 0;
		*((char ***)gaddr) = (char **)(&rs_tedinfo[idx].te_ptmplt);
		break;
#endif
#if NUM_TI != 0
	case R_TEPVALID:
		if (idx < 0 || idx >= NUM_TI)
			return 0;
		*((char ***)gaddr) = (char **)(&rs_tedinfo[idx].te_pvalid);
		break;
#endif
#if NUM_IB != 0
	case R_IBPMASK:
		if (idx < 0 || idx >= NUM_IB)
			return 0;
		*((char ***)gaddr) = (char **)(&rs_iconblk[idx].ib_pmask);
		break;
#endif
#if NUM_IB != 0
	case R_IBPDATA:
		if (idx < 0 || idx >= NUM_IB)
			return 0;
		*((char ***)gaddr) = (char **)(&rs_iconblk[idx].ib_pdata);
		break;
#endif
#if NUM_IB != 0
	case R_IBPTEXT:
		if (idx < 0 || idx >= NUM_IB)
			return 0;
		*((char ***)gaddr) = (char **)(&rs_iconblk[idx].ib_ptext);
		break;
#endif
#if NUM_BB != 0
	case R_BIPDATA:
		if (idx < 0 || idx >= NUM_BB)
			return 0;
		*((char ***)gaddr) = (char **)(&rs_bitblk[idx].bi_pdata);
		break;
#endif
#if NUM_FRSTR != 0
	case R_FRSTR:
		if (idx < 0 || idx >= NUM_FRSTR)
			return 0;
		*((char ***)gaddr) = (char **)(&rs_frstr[idx]);
		break;
#endif
#if NUM_FRIMG != 0
	case R_FRIMG:
		if (idx < 0 || idx >= NUM_FRIMG)
			return 0;
		*((BITBLK ***)gaddr) = &rs_frimg[idx];
		break;
#endif
	default:
		return 0;
	}
	return 1;
}


#ifdef __STDC__
_WORD sting_rsc_free(void)
#else
_WORD sting_rsc_free()
#endif
{
#if NUM_OBS != 0
	hrelease_objs(rs_object, NUM_OBS);
#endif
	return 1;
}

#endif /* RSC_NAMED_FUNCTIONS */

#else /* !RSC_STATIC_FILE */
#if 0
_WORD rs_numstrings = 49;
_WORD rs_numfrstr = 0;

_WORD rs_nuser = 0;
_WORD rs_numimages = 0;
_WORD rs_numbb = 0;
_WORD rs_numfrimg = 0;
_WORD rs_numib = 0;
_WORD rs_numcib = 0;
_WORD rs_numti = 11;
_WORD rs_numobs = 44;
_WORD rs_numtree = 1;

char rs_name[] = "sting.rsc";

_WORD _rsc_format = 2; /* RSC_FORM_SOURCE2 */
#endif
#endif /* RSC_STATIC_FILE */
