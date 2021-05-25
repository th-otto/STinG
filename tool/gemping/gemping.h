/*
 * resource set indices for gemping
 *
 * created by ORCS 2.18
 */

/*
 * Number of Strings:        52
 * Number of Bitblks:        0
 * Number of Iconblks:       0
 * Number of Color Iconblks: 0
 * Number of Color Icons:    0
 * Number of Tedinfos:       7
 * Number of Free Strings:   21
 * Number of Free Images:    0
 * Number of Objects:        19
 * Number of Trees:          1
 * Number of Userblks:       0
 * Number of Images:         0
 * Total file size:          2174
 */

#undef RSC_NAME
#ifndef __ALCYON__
#define RSC_NAME "gemping"
#endif
#undef RSC_ID
#ifdef gemping
#define RSC_ID gemping
#else
#define RSC_ID 0
#endif

#ifndef RSC_STATIC_FILE
# define RSC_STATIC_FILE 0
#endif
#if !RSC_STATIC_FILE
#define NUM_STRINGS 52
#define NUM_FRSTR 21
#define NUM_UD 0
#define NUM_IMAGES 0
#define NUM_BB 0
#define NUM_FRIMG 0
#define NUM_IB 0
#define NUM_CIB 0
#define NUM_TI 7
#define NUM_OBS 19
#define NUM_TREE 1
#endif



#define PING                               0 /* form/dialog */
#define HOSTNAME                           2 /* TEXT in tree PING */
#define KERNEL_VERSION                     4 /* TEXT in tree PING */
#define HOST                               5 /* FTEXT in tree PING */
#define NUM                                6 /* FTEXT in tree PING */
#define INTERVAL                           7 /* FTEXT in tree PING */
#define INFO_BOX                           9 /* BOX in tree PING */
#define INFO_1                            10 /* STRING in tree PING */
#define INFO_2                            11 /* STRING in tree PING */
#define INFO_3                            12 /* STRING in tree PING */
#define INFO_4                            13 /* STRING in tree PING */
#define INFOLINE1                         14 /* TEXT in tree PING */
#define INFOLINE2                         15 /* TEXT in tree PING */
#define CANCEL                            16 /* BUTTON in tree PING */
#define START                             17 /* BUTTON in tree PING */

#define NO_SOCKET                          0 /* Free string */

#define AL_SUPERUSER                       1 /* Alert string */

#define AL_INTERVAL                        2 /* Alert string */

#define AL_PRELOAD                         3 /* Alert string */

#define AL_INTERVAL_TOO_SHORT              4 /* Alert string */

#define NO_AUDIBLE                         5 /* Alert string */

#define AL_NO_MEMORY                       6 /* Alert string */

#define AL_FILLPAT                         7 /* Alert string */

#define AL_NO_DEBUG                        8 /* Alert string */

#define AL_DONTROUTE                       9 /* Alert string */

#define AL_IP_HEADER                      10 /* Alert string */

#define AL_DISABLE_MCAST                  11 /* Alert string */

#define AL_MCAST_TTL                      12 /* Alert string */

#define AL_MCAST_SOURCE                   13 /* Alert string */

#define AL_SOURCE_ADDR                    14 /* Alert string */

#define AL_RECEIVE_BUFFER                 15 /* Alert string */

#define FIRST                             16 /* Free string */

#define SECOND                            17 /* Free string */

#define AL_POLL_ERROR                     18 /* Alert string */

#define AL_RECV_ERROR                     19 /* Alert string */

#define INVALID_ADDR                      20 /* Alert string */




#ifdef __STDC__
#ifndef _WORD
#  ifdef WORD
#    define _WORD WORD
#  else
#    define _WORD short
#  endif
#endif
extern _WORD gemping_rsc_load(_WORD wchar, _WORD hchar);
extern _WORD gemping_rsc_gaddr(_WORD type, _WORD idx, void *gaddr);
extern _WORD gemping_rsc_free(void);
#endif
