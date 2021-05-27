/*
 * resource set indices for tracrout
 *
 * created by ORCS 2.18
 */

/*
 * Number of Strings:        40
 * Number of Bitblks:        0
 * Number of Iconblks:       0
 * Number of Color Iconblks: 0
 * Number of Color Icons:    0
 * Number of Tedinfos:       6
 * Number of Free Strings:   10
 * Number of Free Images:    0
 * Number of Objects:        20
 * Number of Trees:          1
 * Number of Userblks:       0
 * Number of Images:         0
 * Total file size:          1692
 */

#undef RSC_NAME
#ifndef __ALCYON__
#define RSC_NAME "tracrout"
#endif
#undef RSC_ID
#ifdef tracrout
#define RSC_ID tracrout
#else
#define RSC_ID 0
#endif

#ifndef RSC_STATIC_FILE
# define RSC_STATIC_FILE 0
#endif
#if !RSC_STATIC_FILE
#define NUM_STRINGS 40
#define NUM_FRSTR 10
#define NUM_UD 0
#define NUM_IMAGES 0
#define NUM_BB 0
#define NUM_FRIMG 0
#define NUM_IB 0
#define NUM_CIB 0
#define NUM_TI 6
#define NUM_OBS 20
#define NUM_TREE 1
#endif



#define TRACROUT                           0 /* form/dialog */
#define MODULE                             3 /* TEXT in tree TRACROUT */
#define AUTHOR                             5 /* TEXT in tree TRACROUT */
#define VERSION                            7 /* TEXT in tree TRACROUT */
#define HOST                               8 /* FTEXT in tree TRACROUT */
#define MAX_TTL                            9 /* FTEXT in tree TRACROUT */
#define WAITTIME                          10 /* FTEXT in tree TRACROUT */
#define INFO_BOX                          12 /* BOX in tree TRACROUT */
#define INFO_1                            13 /* STRING in tree TRACROUT */
#define INFO_2                            14 /* STRING in tree TRACROUT */
#define INFO_3                            15 /* STRING in tree TRACROUT */
#define INFO_4                            16 /* STRING in tree TRACROUT */
#define CANCEL                            17 /* BUTTON in tree TRACROUT */
#define START                             18 /* BUTTON in tree TRACROUT */
#define DOTS                              19 /* STRING in tree TRACROUT */

#define NOT_THERE                          0 /* Alert string */

#define CORRUPTED                          1 /* Alert string */

#define NO_MODULE                          2 /* Alert string */

#define NO_ROUTE                           3 /* Alert string */

#define NO_MEMORY                          4 /* Alert string */

#define NO_HANDLER                         5 /* Alert string */

#define NO_STIK                            6 /* Alert string */

#define PROB_SEND_PACKET                   7 /* Alert string */

#define UNKNOWN_HOST                       8 /* Alert string */

#define STIK_TOO_OLD                       9 /* Alert string */




#ifdef __STDC__
#ifndef _WORD
#  ifdef WORD
#    define _WORD WORD
#  else
#    define _WORD short
#  endif
#endif
extern _WORD tracrout_rsc_load(_WORD wchar, _WORD hchar);
extern _WORD tracrout_rsc_gaddr(_WORD type, _WORD idx, void *gaddr);
extern _WORD tracrout_rsc_free(void);
#endif
