/*
 * resource set indices for sntp
 *
 * created by ORCS 2.18
 */

/*
 * Number of Strings:        43
 * Number of Bitblks:        0
 * Number of Iconblks:       0
 * Number of Color Iconblks: 0
 * Number of Color Icons:    0
 * Number of Tedinfos:       6
 * Number of Free Strings:   14
 * Number of Free Images:    0
 * Number of Objects:        19
 * Number of Trees:          1
 * Number of Userblks:       0
 * Number of Images:         0
 * Total file size:          1756
 */

#undef RSC_NAME
#ifndef __ALCYON__
#define RSC_NAME "sntp"
#endif
#undef RSC_ID
#ifdef sntp
#define RSC_ID sntp
#else
#define RSC_ID 0
#endif

#ifndef RSC_STATIC_FILE
# define RSC_STATIC_FILE 0
#endif
#if !RSC_STATIC_FILE
#define NUM_STRINGS 43
#define NUM_FRSTR 14
#define NUM_UD 0
#define NUM_IMAGES 0
#define NUM_BB 0
#define NUM_FRIMG 0
#define NUM_IB 0
#define NUM_CIB 0
#define NUM_TI 6
#define NUM_OBS 19
#define NUM_TREE 1
#endif



#define SNTP_DIALOG                        0 /* form/dialog */
#define MODULE                             3 /* TEXT in tree SNTP_DIALOG */
#define KERNEL_VERSION                     5 /* TEXT in tree SNTP_DIALOG */
#define HOST                               6 /* FTEXT in tree SNTP_DIALOG */
#define GMT_OFFSET                         7 /* FTEXT in tree SNTP_DIALOG */
#define TIMEOUT                            8 /* FTEXT in tree SNTP_DIALOG */
#define CURRTIME                           9 /* FTEXT in tree SNTP_DIALOG */
#define UPDATE_TIME                       10 /* BUTTON in tree SNTP_DIALOG */
#define INFO_BOX                          11 /* BOX in tree SNTP_DIALOG */
#define INFO_1                            12 /* STRING in tree SNTP_DIALOG */
#define INFO_2                            13 /* STRING in tree SNTP_DIALOG */
#define INFO_3                            14 /* STRING in tree SNTP_DIALOG */
#define INFO_4                            15 /* STRING in tree SNTP_DIALOG */
#define DOTS                              16 /* STRING in tree SNTP_DIALOG */
#define CANCEL                            17 /* BUTTON in tree SNTP_DIALOG */
#define START                             18 /* BUTTON in tree SNTP_DIALOG */

#define NO_NETDRIVER                       0 /* Free string */

#define AL_NO_MEMORY                       1 /* Alert string */

#define AL_SOURCE_ADDR                     2 /* Alert string */

#define AL_POLL_ERROR                      3 /* Alert string */

#define AL_RECV_ERROR                      4 /* Alert string */

#define INVALID_ADDR                       5 /* Alert string */

#define HANGUP                             6 /* Alert string */

#define AL_SEND_ERROR                      7 /* Alert string */

#define KOD                                8 /* Alert string */

#define CORRUPTED                          9 /* Alert string */

#define NO_MODULE                         10 /* Alert string */

#define AL_UDP_OPEN                       11 /* Alert string */

#define AL_SETTIME                        12 /* Alert string */

#define NO_STIK                           13 /* Alert string */




#ifdef __STDC__
#ifndef _WORD
#  ifdef WORD
#    define _WORD WORD
#  else
#    define _WORD short
#  endif
#endif
extern _WORD sntp_rsc_load(_WORD wchar, _WORD hchar);
extern _WORD sntp_rsc_gaddr(_WORD type, _WORD idx, void *gaddr);
extern _WORD sntp_rsc_free(void);
#endif
