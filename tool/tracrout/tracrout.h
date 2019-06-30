/*
 * resource set indices for tracrout
 *
 * created by ORCS 2.16
 */

/*
 * Number of Strings:        8
 * Number of Bitblks:        0
 * Number of Iconblks:       0
 * Number of Color Iconblks: 0
 * Number of Color Icons:    0
 * Number of Tedinfos:       2
 * Number of Free Strings:   0
 * Number of Free Images:    0
 * Number of Objects:        5
 * Number of Trees:          1
 * Number of Userblks:       0
 * Number of Images:         0
 * Total file size:          320
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
#define NUM_STRINGS 8
#define NUM_FRSTR 0
#define NUM_UD 0
#define NUM_IMAGES 0
#define NUM_BB 0
#define NUM_FRIMG 0
#define NUM_IB 0
#define NUM_CIB 0
#define NUM_TI 2
#define NUM_OBS 5
#define NUM_TREE 1
#endif



#define TRACROUT   0 /* form/dialog */
#define HOST       2 /* FTEXT in tree TRACROUT */ /* max len 12 */
#define MAX_TTL    3 /* FTEXT in tree TRACROUT */ /* max len 3 */




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
