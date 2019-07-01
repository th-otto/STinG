/*
 * resource set indices for sting
 *
 * created by ORCS 2.16
 */

/*
 * Number of Strings:        42
 * Number of Bitblks:        0
 * Number of Iconblks:       0
 * Number of Color Iconblks: 0
 * Number of Color Icons:    0
 * Number of Tedinfos:       6
 * Number of Free Strings:   0
 * Number of Free Images:    0
 * Number of Objects:        50
 * Number of Trees:          1
 * Number of Userblks:       0
 * Number of Images:         0
 * Total file size:          1912
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
#define NUM_STRINGS 42
#define NUM_FRSTR 0
#define NUM_UD 0
#define NUM_IMAGES 0
#define NUM_BB 0
#define NUM_FRIMG 0
#define NUM_IB 0
#define NUM_CIB 0
#define NUM_TI 6
#define NUM_OBS 50
#define NUM_TREE 1
#endif



#define STING      0 /* free form */
#define VERSION    3 /* STRING in tree STING */
#define MODE       4 /* BUTTON in tree STING */
#define PNAME      5 /* BUTTON in tree STING */
#define ACTIVE     7 /* BUTTON in tree STING */
#define TYPE       9 /* TEXT in tree STING */ /* max len 28 */
#define BOX_ADDR  10 /* BOX in tree STING */
#define IP_ADDR   12 /* FTEXT in tree STING */ /* max len 12 */
#define SUBNET    13 /* FTEXT in tree STING */ /* max len 12 */
#define MTU       14 /* FTEXT in tree STING */ /* max len 5 */
#define ROUTE     16 /* BUTTON in tree STING */
#define BOX_PAR1  18 /* BOX in tree STING */
#define PP_SLIP   21 /* BUTTON in tree STING */
#define PP_PPP    23 /* BUTTON in tree STING */
#define PP_VJHC   26 /* BUTTON in tree STING */
#define PP_LAN    29 /* BUTTON in tree STING */
#define PP_N_LAN  30 /* STRING in tree STING */
#define BOX_PAR2  31 /* BOX in tree STING */
#define BOX_PAR3  34 /* BOX in tree STING */
#define SBL_HARD  36 /* BUTTON in tree STING */
#define SBL_MAC   37 /* FTEXT in tree STING */ /* max len 12 */
#define BOX_PAR4  38 /* BOX in tree STING */
#define BOX_PAR5  39 /* BOX in tree STING */
#define BOX_PAR6  40 /* BOX in tree STING */
#define BOX_PAR7  41 /* BOX in tree STING */
#define M_PORT    43 /* BUTTON in tree STING */
#define M_IP      44 /* FTEXT in tree STING */ /* max len 12 */
#define SAVE      46 /* BUTTON in tree STING */
#define SET       48 /* BUTTON in tree STING */
#define CANCEL    49 /* BUTTON in tree STING */




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
