/*
 * GEM resource C output of sample
 *
 * created by ORCS 2.16
 */

#if !defined(__GNUC__) || !defined(__mc68000__)
#include <portab.h>
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
#    include <portaes.h>
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
#    define _WORD short
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

#include "sample.h"

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
extern _VOID hrelease_objs(OBJECT *_ob, _WORD _num_objs);
#endif
#ifndef hfix_objs
extern _VOID *hfix_objs(RSHDR *_hdr, OBJECT *_ob, _WORD _num_objs);
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
#define NUM_STRINGS 28
#define NUM_BB		0
#define NUM_IB		1
#define NUM_CIB     0
#define NUM_CIC     0
#define NUM_TI		3
#define NUM_FRSTR	0
#define NUM_FRIMG	0
#define NUM_OBS     29
#define NUM_TREE	3
#define NUM_UD		0
#endif


static char sample_string_0[] = "Sample";
static char sample_string_1[] = " Sample Server ";
static char sample_string_2[] = "";
static char sample_string_3[] = "";
static char sample_string_4[] = " (w) 1996  by  Peter Rottengatter ";
static char sample_string_5[] = "";
static char sample_string_6[] = "";
static char sample_string_7[] = "Beverage :";
static char sample_string_8[] = "";
static char sample_string_9[] = "Coca Cola";
static char sample_string_10[] = "";
static char sample_string_11[] = "Fanta";
static char sample_string_12[] = "";
static char sample_string_13[] = "Sprite";
static char sample_string_14[] = "___";
static char sample_string_15[] = "Use Port No. ___";
static char sample_string_16[] = "999";
static char sample_string_17[] = "Continent :";
static char sample_string_18[] = "  Antarctica  ";
static char sample_string_19[] = "Save";
static char sample_string_20[] = "Accept";
static char sample_string_21[] = "Cancel";
static char sample_string_22[] = "  America";
static char sample_string_23[] = "  Africa";
static char sample_string_24[] = "  Antarctica";
static char sample_string_25[] = "  Asia";
static char sample_string_26[] = "  Australia";
static char sample_string_27[] = "  Europe";


/* mask of MY_ICON */
static _UBYTE sample_IMAGE0[] = {
0x00, 0x00, 0x00, 0x00, 0x00, 0x07, 0xE0, 0x00, 0x00, 0x1F, 0xF8, 0x00, 0x00, 0x7F, 0xFE, 0x00, 
0x01, 0xFF, 0xFF, 0x80, 0x07, 0xFF, 0xFF, 0xE0, 0x1F, 0xFF, 0xFF, 0xF8, 0x7F, 0xFF, 0xFF, 0xFE, 
0xFF, 0xFF, 0xFF, 0xFF, 0x7F, 0xFF, 0xFF, 0xFE, 0x3F, 0xFF, 0xFF, 0xFC, 0x1F, 0xFF, 0xFF, 0xF8, 
0x0F, 0xFF, 0xFF, 0xF0, 0x07, 0xFF, 0xFF, 0xE0, 0x03, 0xFF, 0xFF, 0xC0, 0x01, 0xFF, 0xFF, 0x80, 
0x00, 0xFF, 0xFF, 0x00, 0x00, 0x7F, 0xFE, 0x00, 0x00, 0x3F, 0xFC, 0x00, 0x00, 0x1F, 0xF8, 0x00, 
0x00, 0x0F, 0xF0, 0x00, 0x00, 0x07, 0xE0, 0x00, 0x00, 0x03, 0xC0, 0x00, 0x00, 0x00, 0x00, 0x00};

/* data of MY_ICON */
static _UBYTE sample_IMAGE1[] = {
0x00, 0x00, 0x00, 0x00, 0x00, 0x07, 0xE0, 0x00, 0x00, 0x19, 0x98, 0x00, 0x00, 0x62, 0x46, 0x00, 
0x01, 0x84, 0x21, 0x80, 0x06, 0x08, 0x10, 0x60, 0x18, 0x10, 0x08, 0x18, 0x60, 0x20, 0x04, 0x06, 
0xFF, 0xFF, 0xFF, 0xFF, 0x40, 0x40, 0x02, 0x02, 0x20, 0x20, 0x04, 0x04, 0x10, 0x20, 0x04, 0x08, 
0x08, 0x10, 0x08, 0x10, 0x04, 0x10, 0x08, 0x20, 0x02, 0x08, 0x10, 0x40, 0x01, 0x08, 0x10, 0x80, 
0x00, 0x84, 0x21, 0x00, 0x00, 0x44, 0x22, 0x00, 0x00, 0x22, 0x44, 0x00, 0x00, 0x12, 0x48, 0x00, 
0x00, 0x09, 0x90, 0x00, 0x00, 0x05, 0xA0, 0x00, 0x00, 0x03, 0xC0, 0x00, 0x00, 0x00, 0x00, 0x00};


ICONBLK rs_iconblk[] = {
	{ CW sample_IMAGE0, CW sample_IMAGE1, sample_string_0, 0x1000,13,9, 2,0,32,24, 0,26,36,8 }
};


TEDINFO rs_tedinfo[NUM_TI] = {
	{ sample_string_1, sample_string_2, sample_string_3, IBM, 6, TE_CNTR, 0x1180, 0x0, -1, 16,1 },
	{ sample_string_4, sample_string_5, sample_string_6, SMALL, 6, TE_LEFT, 0x1180, 0x0, -1, 35,1 },
	{ sample_string_14, sample_string_15, sample_string_16, IBM, 6, TE_LEFT, 0x1180, 0x0, -1, 4,17 } /* PORT */
};


OBJECT rs_object[NUM_OBS] = {
/* ICON */

	{ -1, 1, 1, G_BOX, OF_NONE, OS_NORMAL, C_UNION(0x11100L), 0,0, 529,5 },
	{ 0, -1, -1, G_ICON, 0x21, OS_NORMAL, C_UNION(&rs_iconblk[0]), 262,769, 9216,8704 }, /* MY_ICON */

/* MAIN */

	{ -1, 1, 17, G_BOX, OF_NONE, OS_NORMAL, C_UNION(0x1100L), 0,0, 37,3597 },
	{ 4, 2, 3, G_BOX, OF_NONE, OS_NORMAL, C_UNION(0xFF1100L), 0,0, 37,2307 },
	{ 3, -1, -1, G_TEXT, OF_NONE, OS_NORMAL, C_UNION(&rs_tedinfo[0]), 11,2560, 15,1 },
	{ 1, -1, -1, G_TEXT, OF_NONE, OS_NORMAL, C_UNION(&rs_tedinfo[1]), 6,2, 26,1 },
	{ 12, 5, 11, G_IBOX, OF_NONE, OS_NORMAL, C_UNION(0x1100L), 22,1028, 13,5 },
	{ 6, -1, -1, G_STRING, OF_NONE, OS_NORMAL, C_UNION(sample_string_7), 0,3072, 10,1 },
	{ 7, -1, -1, (1<<8)+G_BUTTON, 0x11, 0x6, C_UNION(sample_string_8), 1,2, 2,1 }, /* COKE */
	{ 8, -1, -1, G_STRING, OF_NONE, OS_NORMAL, C_UNION(sample_string_9), 4,2, 9,1 },
	{ 9, -1, -1, (1<<8)+G_BUTTON, 0x11, 0x6, C_UNION(sample_string_10), 1,3, 2,1 }, /* FANTA */
	{ 10, -1, -1, G_STRING, OF_NONE, OS_NORMAL, C_UNION(sample_string_11), 4,3, 5,1 },
	{ 11, -1, -1, (1<<8)+G_BUTTON, 0x11, 0x6, C_UNION(sample_string_12), 1,4, 2,1 }, /* SPRITE */
	{ 4, -1, -1, G_STRING, OF_NONE, OS_NORMAL, C_UNION(sample_string_13), 4,4, 6,1 },
	{ 13, -1, -1, G_FTEXT, OF_EDITABLE, OS_NORMAL, C_UNION(&rs_tedinfo[2]), 2,5, 16,1 }, /* PORT */
	{ 14, -1, -1, G_STRING, OF_NONE, OS_NORMAL, C_UNION(sample_string_17), 2,3590, 11,1 },
	{ 15, -1, -1, G_BUTTON, OF_TOUCHEXIT, OS_SHADOWED, C_UNION(sample_string_18), 5,1544, 14,1 }, /* CONTIN */
	{ 17, 16, 16, G_BOX, OF_NONE, OS_NORMAL, C_UNION(0xFF1100L), 0,11, 12,3586 },
	{ 15, -1, -1, G_BUTTON, 0x5, OS_NORMAL, C_UNION(sample_string_19), 3,3840, 6,1 }, /* SAVE */
	{ 0, 18, 19, G_BOX, OF_NONE, OS_NORMAL, C_UNION(0xFF1100L), 12,11, 25,3586 },
	{ 19, -1, -1, G_BUTTON, 0x5, OS_NORMAL, C_UNION(sample_string_20), 3,3840, 8,1 }, /* ACCEPT */
	{ 17, -1, -1, G_BUTTON, 0x25, OS_NORMAL, C_UNION(sample_string_21), 14,3840, 8,1 }, /* CANCEL */

/* PU_TEST */

	{ -1, 1, 6, G_BOX, OF_NONE, OS_SHADOWED, C_UNION(0xFF1100L), 0,0, 14,6 },
	{ 2, -1, -1, G_STRING, OF_NONE, OS_NORMAL, C_UNION(sample_string_22), 0,0, 14,1 },
	{ 3, -1, -1, G_STRING, OF_NONE, OS_CHECKED, C_UNION(sample_string_23), 0,1, 14,1 },
	{ 4, -1, -1, G_STRING, OF_NONE, OS_NORMAL, C_UNION(sample_string_24), 0,2, 14,1 },
	{ 5, -1, -1, G_STRING, OF_NONE, OS_NORMAL, C_UNION(sample_string_25), 0,3, 14,1 },
	{ 6, -1, -1, G_STRING, OF_NONE, OS_NORMAL, C_UNION(sample_string_26), 0,4, 14,1 },
	{ 0, -1, -1, G_STRING, OF_LASTOB, OS_NORMAL, C_UNION(sample_string_27), 0,5, 14,1 }
};


OBJECT *rs_trindex[NUM_TREE] = {
	&rs_object[0], /* ICON */
	&rs_object[2], /* MAIN */
	&rs_object[22] /* PU_TEST */
};





#if RSC_STATIC_FILE

#if RSC_NAMED_FUNCTIONS
#ifdef __STDC__
_WORD sample_rsc_load(_WORD wchar, _WORD hchar)
#else
_WORD sample_rsc_load(wchar, hchar)
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
_WORD sample_rsc_gaddr(_WORD type, _WORD idx, void *gaddr)
#else
_WORD sample_rsc_gaddr(type, idx, gaddr)
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
_WORD sample_rsc_free(void)
#else
_WORD sample_rsc_free()
#endif
{
#if NUM_OBS != 0
	hrelease_objs(rs_object, NUM_OBS);
#endif
	return 1;
}

#endif /* RSC_NAMED_FUNCTIONS */

#else /* !RSC_STATIC_FILE */
_WORD rs_numstrings = 28;
_WORD rs_numfrstr = 0;

_WORD rs_nuser = 0;
_WORD rs_numimages = 2;
_WORD rs_numbb = 0;
_WORD rs_numfrimg = 0;
_WORD rs_numib = 1;
_WORD rs_numcib = 0;
_WORD rs_numti = 3;
_WORD rs_numobs = 29;
_WORD rs_numtree = 3;

char rs_name[] = "sample.rsc";

_WORD _rsc_format = 2; /* RSC_FORM_SOURCE2 */
#endif /* RSC_STATIC_FILE */
