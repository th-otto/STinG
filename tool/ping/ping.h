/*
 * resource set indices for ping
 *
 * created by ORCS 2.18
 */

/*
 * Number of Strings:        45
 * Number of Bitblks:        0
 * Number of Iconblks:       0
 * Number of Color Iconblks: 0
 * Number of Color Icons:    0
 * Number of Tedinfos:       8
 * Number of Free Strings:   10
 * Number of Free Images:    0
 * Number of Objects:        21
 * Number of Trees:          1
 * Number of Userblks:       0
 * Number of Images:         0
 * Total file size:          1770
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
#define NUM_STRINGS 45
#define NUM_FRSTR 10
#define NUM_UD 0
#define NUM_IMAGES 0
#define NUM_BB 0
#define NUM_FRIMG 0
#define NUM_IB 0
#define NUM_CIB 0
#define NUM_TI 8
#define NUM_OBS 21
#define NUM_TREE 1
#endif



#define PING                               0 /* form/dialog */
#define MODULE                             3 /* TEXT in tree PING */
#define AUTHOR                             5 /* TEXT in tree PING */
#define VERSION                            7 /* TEXT in tree PING */
#define HOST                               8 /* FTEXT in tree PING */
#define NUM                                9 /* FTEXT in tree PING */
#define INTERVAL                          10 /* FTEXT in tree PING */
#define INFO_BOX                          12 /* BOX in tree PING */
#define INFO_1                            13 /* STRING in tree PING */
#define INFO_2                            14 /* STRING in tree PING */
#define INFO_3                            15 /* STRING in tree PING */
#define INFO_4                            16 /* STRING in tree PING */
#define INFOLINE1                         17 /* TEXT in tree PING */
#define INFOLINE2                         18 /* TEXT in tree PING */
#define CANCEL                            19 /* BUTTON in tree PING */
#define START                             20 /* BUTTON in tree PING */

#define NOT_THERE                          0 /* Alert string */

#define CORRUPTED                          1 /* Alert string */

#define NO_MODULE                          2 /* Alert string */

#define NO_HANDLER                         3 /* Alert string */

#define FIRST                              4 /* Free string */

#define SECOND                             5 /* Free string */

#define NO_MULTICAST                       6 /* Alert string */

#define STING_STRERROR                     7 /* Alert string */

#define INVALID_ADDR                       8 /* Alert string */

#define STIK_TOO_OLD                       9 /* Alert string */




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
