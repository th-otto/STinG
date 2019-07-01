/*
 * resource set indices for inetd
 *
 * created by ORCS 2.16
 */

/*
 * Number of Strings:        128
 * Number of Bitblks:        0
 * Number of Iconblks:       5
 * Number of Color Iconblks: 0
 * Number of Color Icons:    0
 * Number of Tedinfos:       6
 * Number of Free Strings:   0
 * Number of Free Images:    0
 * Number of Objects:        157
 * Number of Trees:          7
 * Number of Userblks:       0
 * Number of Images:         10
 * Total file size:          7192
 */

#undef RSC_NAME
#ifndef __ALCYON__
#define RSC_NAME "inetd"
#endif
#undef RSC_ID
#ifdef inetd
#define RSC_ID inetd
#else
#define RSC_ID 0
#endif

#ifndef RSC_STATIC_FILE
# define RSC_STATIC_FILE 0
#endif
#if !RSC_STATIC_FILE
#define NUM_STRINGS 128
#define NUM_FRSTR 0
#define NUM_UD 0
#define NUM_IMAGES 10
#define NUM_BB 0
#define NUM_FRIMG 0
#define NUM_IB 5
#define NUM_CIB 0
#define NUM_TI 6
#define NUM_OBS 157
#define NUM_TREE 7
#endif



#define START      0 /* free form */
#define ST_PBOX    4 /* BOX in tree START */
#define SS_VERS    6 /* STRING in tree START */
#define ST_VERS    8 /* STRING in tree START */
#define ST_MBOX    9 /* BOX in tree START */
#define ST_MBX1   10 /* BOX in tree START */
#define ST_MIC1   11 /* ICON in tree START */ /* max len 12 */
#define ST_MNAM1  12 /* STRING in tree START */
#define ST_MCK1   13 /* IBOX in tree START */
#define ST_MBX2   14 /* BOX in tree START */
#define ST_MIC2   15 /* ICON in tree START */ /* max len 12 */
#define ST_MNAM2  16 /* STRING in tree START */
#define ST_MCK2   17 /* IBOX in tree START */
#define ST_MBX3   18 /* BOX in tree START */
#define ST_MIC3   19 /* ICON in tree START */ /* max len 12 */
#define ST_MNAM3  20 /* STRING in tree START */
#define ST_MCK3   21 /* IBOX in tree START */
#define ST_MBX4   22 /* BOX in tree START */
#define ST_MIC4   23 /* ICON in tree START */ /* max len 12 */
#define ST_MNAM4  24 /* STRING in tree START */
#define ST_MCK4   25 /* IBOX in tree START */
#define ST_SLIDE  26 /* BOX in tree START */
#define ST_S_UP   27 /* BUTTON in tree START */
#define ST_S_GND  28 /* BOX in tree START */
#define ST_S_BTN  29 /* BOX in tree START */
#define ST_S_DWN  30 /* BUTTON in tree START */
#define AUTHORS   31 /* BUTTON in tree START */
#define ST_CONF   32 /* BUTTON in tree START */

#define ICONIFY    1 /* free form */

#define CREDITS    2 /* free form */

#define CONF       3 /* free form */
#define OBJ8       0 /* BOX in tree CONF */
#define CON_BOX    4 /* BOX in tree CONF */
#define CI_BOX     5 /* BOX in tree CONF */
#define CIC_ALPH   9 /* BUTTON in tree CONF */
#define CIC_RNDM  11 /* BUTTON in tree CONF */
#define CIC_LEN   14 /* FTEXT in tree CONF */ /* max len 3 */
#define CE_BOX    16 /* BOX in tree CONF */
#define CE_PORT   19 /* FTEXT in tree CONF */ /* max len 4 */
#define CE_TCP    21 /* BUTTON in tree CONF */
#define CE_UDP    23 /* BUTTON in tree CONF */
#define CE_NO     26 /* STRING in tree CONF */
#define CE_UP     27 /* BOXCHAR in tree CONF */
#define CE_DWN    28 /* BOXCHAR in tree CONF */
#define CE_INS    29 /* BUTTON in tree CONF */
#define CE_DEL    30 /* BUTTON in tree CONF */
#define CE_SS_A   33 /* STRING in tree CONF */
#define CE_SS_B   34 /* STRING in tree CONF */
#define CE_SERV   37 /* BUTTON in tree CONF */
#define CE_OPEN   39 /* BUTTON in tree CONF */
#define CE_MULTI  42 /* BUTTON in tree CONF */
#define CM_BOX    45 /* BOX in tree CONF */
#define CM_RATE   48 /* FTEXT in tree CONF */ /* max len 3 */
#define CC_M_NAM  50 /* BUTTON in tree CONF */
#define CC_SAVE   51 /* BUTTON in tree CONF */
#define CC_SET    52 /* BUTTON in tree CONF */

#define PU_C_TYP   4 /* form/dialog */

#define PU_C_TCP   5 /* form/dialog */

#define PU_C_UDP   6 /* form/dialog */




#ifdef __STDC__
#ifndef _WORD
#  ifdef WORD
#    define _WORD WORD
#  else
#    define _WORD short
#  endif
#endif
extern _WORD inetd_rsc_load(_WORD wchar, _WORD hchar);
extern _WORD inetd_rsc_gaddr(_WORD type, _WORD idx, void *gaddr);
extern _WORD inetd_rsc_free(void);
#endif
