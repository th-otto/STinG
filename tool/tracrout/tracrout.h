/*
 * resource set indices for tracrout
 *
 * created by ORCS 2.18
 */

/*
 * Number of Strings:        33
 * Number of Bitblks:        0
 * Number of Iconblks:       0
 * Number of Color Iconblks: 0
 * Number of Color Icons:    0
 * Number of Tedinfos:       5
 * Number of Free Strings:   12
 * Number of Free Images:    0
 * Number of Objects:        12
 * Number of Trees:          1
 * Number of Userblks:       0
 * Number of Images:         0
 * Total file size:          1368
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
#define NUM_STRINGS 33
#define NUM_FRSTR 12
#define NUM_UD 0
#define NUM_IMAGES 0
#define NUM_BB 0
#define NUM_FRIMG 0
#define NUM_IB 0
#define NUM_CIB 0
#define NUM_TI 5
#define NUM_OBS 12
#define NUM_TREE 1
#endif



#define TRACROUT                           0 /* form/dialog */
#define MODULE                             2 /* TEXT in tree TRACROUT */
#define AUTHOR                             4 /* TEXT in tree TRACROUT */
#define VERSION                            6 /* TEXT in tree TRACROUT */
#define HOST                               8 /* FTEXT in tree TRACROUT */
#define MAX_TTL                            9 /* FTEXT in tree TRACROUT */
#define OK                                10 /* BUTTON in tree TRACROUT */
#define CANCEL                            11 /* BUTTON in tree TRACROUT */

#define NOT_THERE                          0 /* Alert string */

#define CORRUPTED                          1 /* Alert string */

#define NO_MODULE                          2 /* Alert string */

#define TIMEOUT                            3 /* Free string */

#define NO_ROUTE                           4 /* Alert string */

#define NO_MEMORY                          5 /* Alert string */

#define NO_HANDLER                         6 /* Alert string */

#define NO_STIK                            7 /* Alert string */

#define PROB_SEND_PACKET                   8 /* Alert string */

#define HOP_RESULT                         9 /* Alert string */

#define DEST_REACHED                      10 /* Alert string */

#define NO_DEST                           11 /* Alert string */




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
