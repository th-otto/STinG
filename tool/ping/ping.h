/*
 * resource set indices for ping
 *
 * created by ORCS 2.18
 */

/*
 * Number of Strings:        32
 * Number of Bitblks:        0
 * Number of Iconblks:       0
 * Number of Color Iconblks: 0
 * Number of Color Icons:    0
 * Number of Tedinfos:       5
 * Number of Free Strings:   7
 * Number of Free Images:    0
 * Number of Objects:        17
 * Number of Trees:          1
 * Number of Userblks:       0
 * Number of Images:         0
 * Total file size:          1436
 */

#undef RSC_NAME
#ifndef __ALCYON__
#define RSC_NAME "ping"
#endif
#undef RSC_ID
#ifdef ping
#define RSC_ID ping
#else
#define RSC_ID 0
#endif

#ifndef RSC_STATIC_FILE
# define RSC_STATIC_FILE 0
#endif
#if !RSC_STATIC_FILE
#define NUM_STRINGS 32
#define NUM_FRSTR 7
#define NUM_UD 0
#define NUM_IMAGES 0
#define NUM_BB 0
#define NUM_FRIMG 0
#define NUM_IB 0
#define NUM_CIB 0
#define NUM_TI 5
#define NUM_OBS 17
#define NUM_TREE 1
#endif



#define PING                               0 /* form/dialog */
#define MODULE                             2 /* TEXT in tree PING */
#define AUTHOR                             4 /* TEXT in tree PING */
#define VERSION                            6 /* TEXT in tree PING */
#define HOST                               7 /* FTEXT in tree PING */
#define NUM                                8 /* FTEXT in tree PING */
#define INFO_BOX                          10 /* BOX in tree PING */
#define INFO_1                            11 /* STRING in tree PING */
#define INFO_2                            12 /* STRING in tree PING */
#define INFO_3                            13 /* STRING in tree PING */
#define INFO_4                            14 /* STRING in tree PING */
#define CANCEL                            15 /* BUTTON in tree PING */
#define START                             16 /* BUTTON in tree PING */

#define NOT_THERE                          0 /* Alert string */

#define CORRUPTED                          1 /* Alert string */

#define NO_MODULE                          2 /* Alert string */

#define NO_HANDLER                         3 /* Alert string */

#define TAKES                              4 /* Alert string */

#define FIRST                              5 /* Alert string */

#define SECOND                             6 /* Alert string */




#ifdef __STDC__
#ifndef _WORD
#  ifdef WORD
#    define _WORD WORD
#  else
#    define _WORD short
#  endif
#endif
extern _WORD ping_rsc_load(_WORD wchar, _WORD hchar);
extern _WORD ping_rsc_gaddr(_WORD type, _WORD idx, void *gaddr);
extern _WORD ping_rsc_free(void);
#endif
