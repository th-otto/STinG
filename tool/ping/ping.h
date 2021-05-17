/*
 * resource set indices for ping
 *
 * created by ORCS 2.18
 */

/*
 * Number of Strings:        43
 * Number of Bitblks:        0
 * Number of Iconblks:       0
 * Number of Color Iconblks: 0
 * Number of Color Icons:    0
 * Number of Tedinfos:       8
 * Number of Free Strings:   9
 * Number of Free Images:    0
 * Number of Objects:        20
 * Number of Trees:          1
 * Number of Userblks:       0
 * Number of Images:         0
 * Total file size:          1658
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
#define NUM_STRINGS 43
#define NUM_FRSTR 9
#define NUM_UD 0
#define NUM_IMAGES 0
#define NUM_BB 0
#define NUM_FRIMG 0
#define NUM_IB 0
#define NUM_CIB 0
#define NUM_TI 8
#define NUM_OBS 20
#define NUM_TREE 1
#endif



#define PING                               0 /* form/dialog */
#define MODULE                             2 /* TEXT in tree PING */
#define AUTHOR                             4 /* TEXT in tree PING */
#define VERSION                            6 /* TEXT in tree PING */
#define HOST                               7 /* FTEXT in tree PING */
#define NUM                                8 /* FTEXT in tree PING */
#define INTERVAL                           9 /* FTEXT in tree PING */
#define INFO_BOX                          11 /* BOX in tree PING */
#define INFO_1                            12 /* STRING in tree PING */
#define INFO_2                            13 /* STRING in tree PING */
#define INFO_3                            14 /* STRING in tree PING */
#define INFO_4                            15 /* STRING in tree PING */
#define INFOLINE1                         16 /* TEXT in tree PING */
#define INFOLINE2                         17 /* TEXT in tree PING */
#define CANCEL                            18 /* BUTTON in tree PING */
#define START                             19 /* BUTTON in tree PING */

#define NOT_THERE                          0 /* Alert string */

#define CORRUPTED                          1 /* Alert string */

#define NO_MODULE                          2 /* Alert string */

#define NO_HANDLER                         3 /* Alert string */

#define FIRST                              4 /* Free string */

#define SECOND                             5 /* Free string */

#define NO_MULTICAST                       6 /* Alert string */

#define STING_STRERROR                     7 /* Alert string */

#define INVALID_ADDR                       8 /* Alert string */




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
