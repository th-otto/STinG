/*
 * resource set indices for sting
 *
 * created by ORCS 2.16
 */

/*
 * Number of Strings:        20
 * Number of Bitblks:        2
 * Number of Iconblks:       0
 * Number of Color Iconblks: 0
 * Number of Color Icons:    0
 * Number of Tedinfos:       3
 * Number of Free Strings:   0
 * Number of Free Images:    0
 * Number of Objects:        27
 * Number of Trees:          1
 * Number of Userblks:       0
 * Number of Images:         2
 * Total file size:          2360
 */

#undef RSC_NAME
#ifndef __ALCYON__
#define RSC_NAME "sting"
#endif
#undef RSC_ID
#ifdef sting
#define RSC_ID sting
#else
#define RSC_ID 0
#endif

#ifndef RSC_STATIC_FILE
# define RSC_STATIC_FILE 0
#endif
#if !RSC_STATIC_FILE
#define NUM_STRINGS 20
#define NUM_FRSTR 0
#define NUM_UD 0
#define NUM_IMAGES 2
#define NUM_BB 2
#define NUM_FRIMG 0
#define NUM_IB 0
#define NUM_CIB 0
#define NUM_TI 3
#define NUM_OBS 27
#define NUM_TREE 1
#endif



#define STING      0 /* free form */
#define VERSION    3 /* STRING in tree STING */
#define PROTOCOL   9 /* BUTTON in tree STING */
#define DRIVERS   10 /* BUTTON in tree STING */
#define MEMORY    11 /* BUTTON in tree STING */
#define ACTIVE    14 /* BUTTON in tree STING */
#define F_PARENT  16 /* BOX in tree STING */
#define F_SLIDE   18 /* BOX in tree STING */
#define F_REDRW   20 /* BOX in tree STING */
#define FREQ      21 /* TEXT in tree STING */ /* max len 3 */
#define SAVE      23 /* BUTTON in tree STING */
#define SET       25 /* BUTTON in tree STING */
#define CANCEL    26 /* BUTTON in tree STING */




#ifdef __STDC__
#ifndef _WORD
#  ifdef WORD
#    define _WORD WORD
#  else
#    define _WORD short
#  endif
#endif
extern _WORD sting_rsc_load(_WORD wchar, _WORD hchar);
extern _WORD sting_rsc_gaddr(_WORD type, _WORD idx, void *gaddr);
extern _WORD sting_rsc_free(void);
#endif
