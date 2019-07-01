/*
 * GEM resource C output of sting
 *
 * created by ORCS 2.16
 */

#ifndef _LONG_PTR
#  define _LONG_PTR LONG
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

#ifndef WHITEBAK
#  define WHITEBAK OS_WHITEBAK
#endif
#ifndef DRAW3D
#  define DRAW3D OS_DRAW3D
#endif
#ifndef FL3DIND
#  define FL3DIND OF_FL3DIND
#endif
#ifndef FL3DBAK
#  define FL3DBAK OF_FL3DBAK
#endif
#ifndef FL3DACT
#  define FL3DACT OF_FL3DACT
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

#define T0OBJ 0
#define FREEBB 0
#define FREEIMG 0
#define FREESTR 49

BYTE *rs_strings[] = {
	(BYTE *)"STinG V00.01",
	(BYTE *)"TCP.......",
	(BYTE *)"",
	(BYTE *)"Address Mask Control",
	(BYTE *)"",
	(BYTE *)"___",
	(BYTE *)"Router Advert. :  ___ min",
	(BYTE *)"999",
	(BYTE *)"____",
	(BYTE *)"Lag behind GMT : ____ min",
	(BYTE *)"XXXX",
	(BYTE *)"_____",
	(BYTE *)"Max. Segment Size : _____",
	(BYTE *)"99999",
	(BYTE *)"_____",
	(BYTE *)"Receive Window : _____",
	(BYTE *)"99999",
	(BYTE *)"_____",
	(BYTE *)"First local port : _____",
	(BYTE *)"99999",
	(BYTE *)"___",
	(BYTE *)"Default TTL : ___",
	(BYTE *)"XXX",
	(BYTE *)"_____",
	(BYTE *)"Initial RTT : __.___ sec",
	(BYTE *)"99999",
	(BYTE *)"",
	(BYTE *)"Evaluate ICMP messages",
	(BYTE *)"_____",
	(BYTE *)"First local port : _____",
	(BYTE *)"99999",
	(BYTE *)"",
	(BYTE *)"Evaluate ICMP messages",
	(BYTE *)"Name Server :",
	(BYTE *)"123456789357",
	(BYTE *)"___.___.___.___",
	(BYTE *)"999999999999",
	(BYTE *)"Domain :",
	(BYTE *)"____________________________",
	(BYTE *)"____________________________",
	(BYTE *)"XXXXXXXXXXXXXXXXXXXXXXXXXXXX",
	(BYTE *)"____",
	(BYTE *)"Cache :  ____ RRs",
	(BYTE *)"XXXX",
	(BYTE *)"",
	(BYTE *)"Save",
	(BYTE *)"Save",
	(BYTE *)"Ok",
	(BYTE *)"Cancel"
};

LONG rs_frstr[] = {
	0
};

BITBLK rs_bitblk[] = {
	{ 0, 0, 0, 0, 0, 0 }
};

LONG rs_frimg[] = {
	0
};

ICONBLK rs_iconblk[] = {
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }
};

TEDINFO rs_tedinfo[] = {
	{ (BYTE *)5L, (BYTE *)6L, (BYTE *)7L, 3, 6, 0, 0x1180, 0x0, -1, 4,26 },
	{ (BYTE *)8L, (BYTE *)9L, (BYTE *)10L, 3, 6, 0, 0x1180, 0x0, -1, 5,26 },
	{ (BYTE *)11L, (BYTE *)12L, (BYTE *)13L, 3, 6, 0, 0x1180, 0x0, -1, 6,26 },
	{ (BYTE *)14L, (BYTE *)15L, (BYTE *)16L, 3, 6, 0, 0x1180, 0x0, -1, 6,23 },
	{ (BYTE *)17L, (BYTE *)18L, (BYTE *)19L, 3, 6, 0, 0x1180, 0x0, -1, 6,25 },
	{ (BYTE *)20L, (BYTE *)21L, (BYTE *)22L, 3, 6, 0, 0x1180, 0x0, -1, 4,18 },
	{ (BYTE *)23L, (BYTE *)24L, (BYTE *)25L, 3, 6, 0, 0x1180, 0x0, -1, 6,25 },
	{ (BYTE *)28L, (BYTE *)29L, (BYTE *)30L, 3, 6, 0, 0x1180, 0x0, -1, 6,25 },
	{ (BYTE *)34L, (BYTE *)35L, (BYTE *)36L, 3, 6, 0, 0x1180, 0x0, -1, 13,16 },
	{ (BYTE *)38L, (BYTE *)39L, (BYTE *)40L, 3, 6, 0, 0x1180, 0x0, -1, 29,29 },
	{ (BYTE *)41L, (BYTE *)42L, (BYTE *)43L, 3, 6, 0, 0x1180, 0x0, -1, 5,18 }
};

OBJECT rs_object[] = {
	{ -1, 1, 41, G_BOX, OF_NONE, OS_NORMAL, C_UNION(0x1101L), 0x0000,0x0000, 0x0020,0x000b },
	{ 39, 2, 27, G_BOX, OF_NONE, OS_NORMAL, C_UNION(0xFF1100L), 0x0000,0x0000, 0x0020,0x0309 },
	{ 4, 3, 3, G_BOX, OF_NONE, OS_NORMAL, C_UNION(0xFF1100L), 0x0012,0x0000, 0x000e,0x0a01 },
	{ 2, -1, -1, G_STRING, OF_NONE, OS_NORMAL, C_UNION(0x0L), 0x0001,0x0600, 0x000c,0x0001 },
	{ 5, -1, -1, G_BUTTON, 0x41, OS_SHADOWED, C_UNION(0x1L), 0x0002,0x0600, 0x000e,0x0001 },
	{ 13, 6, 12, G_BOX, OF_NONE, OS_NORMAL, C_UNION(0x1100L), 0x0000,0x0002, 0x0020,0x0307 },
	{ 9, 7, 8, G_BOX, OF_NONE, OS_NORMAL, C_UNION(0x1100L), 0x0002,0x0002, 0x0017,0x0001 },
	{ 8, -1, -1, (0x1<<8)+G_BUTTON, OF_SELECTABLE, 0x6, C_UNION(0x2L), 0x0000,0x0000, 0x0002,0x0001 },
	{ 6, -1, -1, G_STRING, OF_NONE, OS_NORMAL, C_UNION(0x3L), 0x0003,0x0000, 0x0014,0x0001 },
	{ 12, 10, 11, G_BOX, OF_NONE, OS_NORMAL, C_UNION(0x1100L), 0x0002,0x0803, 0x001c,0x0001 },
	{ 11, -1, -1, (0x1<<8)+G_BUTTON, OF_SELECTABLE, 0x6, C_UNION(0x4L), 0x0000,0x0000, 0x0002,0x0001 },
	{ 9, -1, -1, G_FTEXT, OF_EDITABLE, OS_NORMAL, C_UNION(0x0L), 0x0003,0x0000, 0x0019,0x0001 },
	{ 5, -1, -1, G_FTEXT, OF_EDITABLE, OS_NORMAL, C_UNION(0x1L), 0x0005,0x0005, 0x001a,0x0001 },
	{ 22, 14, 19, G_BOX, OF_NONE, OS_NORMAL, C_UNION(0x1100L), 0x0000,0x0002, 0x0020,0x0307 },
	{ 15, -1, -1, G_FTEXT, OF_EDITABLE, OS_NORMAL, C_UNION(0x2L), 0x0003,0x0200, 0x0019,0x0001 },
	{ 16, -1, -1, G_FTEXT, OF_EDITABLE, OS_NORMAL, C_UNION(0x3L), 0x0006,0x0201, 0x0016,0x0001 },
	{ 17, -1, -1, G_FTEXT, OF_EDITABLE, OS_NORMAL, C_UNION(0x4L), 0x0004,0x0202, 0x0018,0x0001 },
	{ 18, -1, -1, G_FTEXT, OF_EDITABLE, OS_NORMAL, C_UNION(0x5L), 0x0009,0x0203, 0x0011,0x0001 },
	{ 19, -1, -1, G_FTEXT, OF_EDITABLE, OS_NORMAL, C_UNION(0x6L), 0x0005,0x0704, 0x0018,0x0001 },
	{ 13, 20, 21, G_BOX, OF_NONE, OS_NORMAL, C_UNION(0x1100L), 0x0004,0x0c05, 0x0019,0x0001 },
	{ 21, -1, -1, (0x1<<8)+G_BUTTON, OF_SELECTABLE, 0x6, C_UNION(0x1AL), 0x0000,0x0000, 0x0002,0x0001 },
	{ 19, -1, -1, G_STRING, OF_NONE, OS_NORMAL, C_UNION(0x1BL), 0x0003,0x0000, 0x0016,0x0001 },
	{ 27, 23, 24, G_BOX, OF_NONE, OS_NORMAL, C_UNION(0x1100L), 0x0000,0x0002, 0x0020,0x0307 },
	{ 24, -1, -1, G_FTEXT, OF_EDITABLE, OS_NORMAL, C_UNION(0x7L), 0x0004,0x0002, 0x0018,0x0001 },
	{ 22, 25, 26, G_BOX, OF_NONE, OS_NORMAL, C_UNION(0x1100L), 0x0004,0x0004, 0x0019,0x0001 },
	{ 26, -1, -1, (0x1<<8)+G_BUTTON, OF_SELECTABLE, 0x6, C_UNION(0x1FL), 0x0000,0x0000, 0x0002,0x0001 },
	{ 24, -1, -1, G_STRING, OF_NONE, OS_NORMAL, C_UNION(0x20L), 0x0003,0x0000, 0x0016,0x0001 },
	{ 1, 28, 35, G_BOX, OF_NONE, OS_NORMAL, C_UNION(0x1100L), 0x0000,0x0002, 0x0020,0x0307 },
	{ 29, -1, -1, G_STRING, OF_NONE, OS_NORMAL, C_UNION(0x21L), 0x0003,0x0400, 0x000d,0x0001 },
	{ 33, 30, 32, G_BOX, OF_NONE, OS_NORMAL, C_UNION(0x1100L), 0x0002,0x0801, 0x0015,0x0001 },
	{ 31, -1, -1, G_BOXCHAR, 0x41, OS_NORMAL, C_UNION(0x4FF1100L), 0x0000,0x0000, 0x0002,0x0001 },
	{ 32, -1, -1, G_FTEXT, OF_EDITABLE, OS_NORMAL, C_UNION(0x8L), 0x0003,0x0000, 0x000f,0x0001 },
	{ 29, -1, -1, G_BOXCHAR, 0x41, OS_NORMAL, C_UNION(0x3FF1100L), 0x0013,0x0000, 0x0002,0x0001 },
	{ 34, -1, -1, G_STRING, OF_NONE, OS_NORMAL, C_UNION(0x25L), 0x0003,0x0e02, 0x0008,0x0001 },
	{ 35, -1, -1, G_FTEXT, OF_EDITABLE, OS_NORMAL, C_UNION(0x9L), 0x0002,0x0204, 0x001c,0x0001 },
	{ 27, 36, 38, G_BOX, OF_NONE, OS_NORMAL, C_UNION(0x1100L), 0x0002,0x0805, 0x001c,0x0001 },
	{ 37, -1, -1, G_FTEXT, OF_EDITABLE, OS_NORMAL, C_UNION(0xAL), 0x0001,0x0000, 0x0011,0x0001 },
	{ 38, -1, -1, (0x1<<8)+G_BUTTON, OF_SELECTABLE, 0x6, C_UNION(0x2CL), 0x0015,0x0000, 0x0002,0x0001 },
	{ 35, -1, -1, G_STRING, OF_NONE, OS_NORMAL, C_UNION(0x2DL), 0x0018,0x0000, 0x0004,0x0001 },
	{ 41, 40, 40, G_BOX, OF_NONE, OS_NORMAL, C_UNION(0xFF1100L), 0x0000,0x0409, 0x070b,0x0c01 },
	{ 39, -1, -1, G_BUTTON, 0x5, OS_NORMAL, C_UNION(0x2EL), 0x0701,0x0600, 0x0008,0x0001 },
	{ 0, 42, 43, G_BOX, OF_NONE, OS_NORMAL, C_UNION(0xFF1100L), 0x000c,0x0409, 0x0014,0x0c01 },
	{ 43, -1, -1, G_BUTTON, 0x7, OS_NORMAL, C_UNION(0x2FL), 0x0202,0x0600, 0x0005,0x0001 },
	{ 41, -1, -1, G_BUTTON, 0x25, OS_NORMAL, C_UNION(0x30L), 0x0009,0x0600, 0x0009,0x0001 }
};

_LONG_PTR rs_trindex[] = {
	0L
};

#ifndef __foobar_defined
#define __foobar_defined 1
struct foobar {
	WORD 	dummy;
	WORD 	*image;
};
#endif
struct foobar rs_imdope[] = {
	{ 0, 0 }
};



#define NUM_STRINGS 49
#define NUM_FRSTR 0
#define NUM_UD 0
#define NUM_IMAGES 0
#define NUM_BB 0
#define NUM_FRIMG 0
#define NUM_IB 0
#define NUM_CIB 0
#define NUM_TI 11
#define NUM_OBS 44
#define NUM_TREE 1

BYTE pname[] = "STING.RSC";
