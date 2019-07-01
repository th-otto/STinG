/*
 * resource set indices for sample
 *
 * created by ORCS 2.16
 */

/*
 * Number of Strings:        28
 * Number of Bitblks:        0
 * Number of Iconblks:       1
 * Number of Color Iconblks: 0
 * Number of Color Icons:    0
 * Number of Tedinfos:       3
 * Number of Free Strings:   0
 * Number of Free Images:    0
 * Number of Objects:        29
 * Number of Trees:          3
 * Number of Userblks:       0
 * Number of Images:         2
 * Total file size:          1284
 */

#undef RSC_NAME
#ifndef __ALCYON__
#define RSC_NAME "sample"
#endif
#undef RSC_ID
#ifdef sample
#define RSC_ID sample
#else
#define RSC_ID 0
#endif

#ifndef RSC_STATIC_FILE
# define RSC_STATIC_FILE 0
#endif
#if !RSC_STATIC_FILE
#define NUM_STRINGS 28
#define NUM_FRSTR 0
#define NUM_UD 0
#define NUM_IMAGES 2
#define NUM_BB 0
#define NUM_FRIMG 0
#define NUM_IB 1
#define NUM_CIB 0
#define NUM_TI 3
#define NUM_OBS 29
#define NUM_TREE 3
#endif



#define ICON       0 /* free form */
#define MY_ICON    1 /* ICON in tree ICON */ /* max len 6 */

#define MAIN       1 /* free form */
#define COKE       6 /* BUTTON in tree MAIN */
#define FANTA      8 /* BUTTON in tree MAIN */
#define SPRITE    10 /* BUTTON in tree MAIN */
#define PORT      12 /* FTEXT in tree MAIN */ /* max len 3 */
#define CONTIN    14 /* BUTTON in tree MAIN */
#define SAVE      16 /* BUTTON in tree MAIN */
#define ACCEPT    18 /* BUTTON in tree MAIN */
#define CANCEL    19 /* BUTTON in tree MAIN */

#define PU_TEST    2 /* form/dialog */




#ifdef __STDC__
#ifndef _WORD
#  ifdef WORD
#    define _WORD WORD
#  else
#    define _WORD short
#  endif
#endif
extern _WORD sample_rsc_load(_WORD wchar, _WORD hchar);
extern _WORD sample_rsc_gaddr(_WORD type, _WORD idx, void *gaddr);
extern _WORD sample_rsc_free(void);
#endif
