/*
 * resource set indices for sting
 *
 * created by ORCS 2.16
 */

/*
 * Number of Strings:        49
 * Number of Bitblks:        0
 * Number of Iconblks:       0
 * Number of Color Iconblks: 0
 * Number of Color Icons:    0
 * Number of Tedinfos:       11
 * Number of Free Strings:   0
 * Number of Free Images:    0
 * Number of Objects:        44
 * Number of Trees:          1
 * Number of Userblks:       0
 * Number of Images:         0
 * Total file size:          1980
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
#define NUM_STRINGS 49
#define NUM_FRSTR 0
#define NUM_UD 0
#define NUM_IMAGES 0
#define NUM_BB 0
#define NUM_FRIMG 0
#define NUM_IB 0
#define NUM_CIB 0
#define NUM_TI 11
#define NUM_OBS 44
#define NUM_TREE 1
#endif



#define STING      0 /* free form */
#define WHOLE      1 /* BOX in tree STING */
#define VERSION    3 /* STRING in tree STING */
#define PROTOCOL   4 /* BUTTON in tree STING */
#define ICMP_BOX   5 /* BOX in tree STING */
#define I_AMC      7 /* BUTTON in tree STING */
#define I_RA      10 /* BUTTON in tree STING */
#define I_RA_ED   11 /* FTEXT in tree STING */ /* max len 3 */
#define I_LAG     12 /* FTEXT in tree STING */ /* max len 4 */
#define TCP_BOX   13 /* BOX in tree STING */
#define MSS       14 /* FTEXT in tree STING */ /* max len 5 */
#define RCV_WIN   15 /* FTEXT in tree STING */ /* max len 5 */
#define T_PORT    16 /* FTEXT in tree STING */ /* max len 5 */
#define DEF_TTL   17 /* FTEXT in tree STING */ /* max len 3 */
#define RTT_INIT  18 /* FTEXT in tree STING */ /* max len 5 */
#define T_ICMP    20 /* BUTTON in tree STING */
#define UDP_BOX   22 /* BOX in tree STING */
#define U_PORT    23 /* FTEXT in tree STING */ /* max len 5 */
#define U_ICMP    25 /* BUTTON in tree STING */
#define DNS_BOX   27 /* BOX in tree STING */
#define DNS_PREV  30 /* BOXCHAR in tree STING */
#define DNS_IP    31 /* FTEXT in tree STING */ /* max len 12 */
#define DNS_NEXT  32 /* BOXCHAR in tree STING */
#define DNS_DOM   34 /* FTEXT in tree STING */ /* max len 28 */
#define DNS_CSZ   36 /* FTEXT in tree STING */ /* max len 4 */
#define DNS_CSV   37 /* BUTTON in tree STING */
#define SAVE      40 /* BUTTON in tree STING */
#define SET       42 /* BUTTON in tree STING */
#define CANCEL    43 /* BUTTON in tree STING */




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
