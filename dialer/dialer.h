/*
 * resource set indices for dialer
 *
 * created by ORCS 2.16
 */

/*
 * Number of Strings:        614
 * Number of Bitblks:        0
 * Number of Iconblks:       1
 * Number of Color Iconblks: 0
 * Number of Color Icons:    0
 * Number of Tedinfos:       113
 * Number of Free Strings:   0
 * Number of Free Images:    0
 * Number of Objects:        514
 * Number of Trees:          19
 * Number of Userblks:       0
 * Number of Images:         2
 * Total file size:          24552
 */

#undef RSC_NAME
#ifndef __ALCYON__
#define RSC_NAME "dialer"
#endif
#undef RSC_ID
#ifdef dialer
#define RSC_ID dialer
#else
#define RSC_ID 0
#endif

#ifndef RSC_STATIC_FILE
# define RSC_STATIC_FILE 0
#endif
#if !RSC_STATIC_FILE
#define NUM_STRINGS 614
#define NUM_FRSTR 0
#define NUM_UD 0
#define NUM_IMAGES 2
#define NUM_BB 0
#define NUM_FRIMG 0
#define NUM_IB 1
#define NUM_CIB 0
#define NUM_TI 113
#define NUM_OBS 514
#define NUM_TREE 19
#endif



#define START      0 /* form/dialog */
#define ST_PBOX    5 /* BOX in tree START */
#define DL_VERS    8 /* STRING in tree START */
#define ST_VERS   10 /* STRING in tree START */
#define MOD_STAT  12 /* STRING in tree START */
#define ST_BOX    15 /* IBOX in tree START */
#define ST_CNCT   16 /* BUTTON in tree START */
#define ST_ABLE   17 /* BUTTON in tree START */
#define ST_STUFF  18 /* BUTTON in tree START */
#define ST_CONF   19 /* BUTTON in tree START */
#define TL_TTL    20 /* BUTTON in tree START */
#define AUTHORS   21 /* BUTTON in tree START */
#define ST_EXIT   22 /* BUTTON in tree START */

#define PU_S_TL    1 /* free form */

#define ICONIFY    2 /* free form */

#define CREDITS    3 /* free form */

#define DIALER     4 /* form/dialog */
#define DL_STAT    5 /* FBOXTEXT in tree DIALER */ /* max len 30 */
#define DL_STRNG   6 /* FBOXTEXT in tree DIALER */ /* max len 30 */
#define DL_SPEED   7 /* FBOXTEXT in tree DIALER */ /* max len 10 */
#define DL_IP      8 /* FBOXTEXT in tree DIALER */ /* max len 16 */
#define DL_TIMER   9 /* FBOXTEXT in tree DIALER */ /* max len 4 */
#define DL_RDIAL  11 /* BUTTON in tree DIALER */
#define DL_ABORT  12 /* BUTTON in tree DIALER */

#define PASSWORD   5 /* free form */
#define PW_BUTT    4 /* BUTTON in tree PASSWORD */
#define PW_STRNG   9 /* FTEXT in tree PASSWORD */ /* max len 48 */

#define CONF       6 /* form/dialog */
#define CON_BOX    4 /* BOX in tree CONF */
#define CC_SCRPT   5 /* FTEXT in tree CONF */ /* max len 40 */
#define CM_BOX     6 /* BOX in tree CONF */
#define CM_INIT    9 /* FTEXT in tree CONF */ /* max len 16 */
#define CM_DIAL   10 /* FTEXT in tree CONF */ /* max len 8 */
#define CM_HANG   11 /* FTEXT in tree CONF */ /* max len 8 */
#define CM_CTOUT  12 /* FTEXT in tree CONF */ /* max len 3 */
#define CM_RDLAY  13 /* FTEXT in tree CONF */ /* max len 3 */
#define CM_CON1   16 /* FTEXT in tree CONF */ /* max len 12 */
#define CM_CON2   17 /* FTEXT in tree CONF */ /* max len 12 */
#define CM_CON3   18 /* FTEXT in tree CONF */ /* max len 12 */
#define CM_RED1   20 /* FTEXT in tree CONF */ /* max len 12 */
#define CM_RED2   21 /* FTEXT in tree CONF */ /* max len 12 */
#define CM_RED3   22 /* FTEXT in tree CONF */ /* max len 12 */
#define CM_ABO1   24 /* FTEXT in tree CONF */ /* max len 12 */
#define CM_ABO2   25 /* FTEXT in tree CONF */ /* max len 12 */
#define CM_ABO3   26 /* FTEXT in tree CONF */ /* max len 12 */
#define CP_BOX    28 /* BOX in tree CONF */
#define CP_IP     30 /* FTEXT in tree CONF */ /* max len 12 */
#define CP_MTU    31 /* FTEXT in tree CONF */ /* max len 5 */
#define CP_PID    32 /* FTEXT in tree CONF */ /* max len 30 */
#define CP_PPTXT  33 /* STRING in tree CONF */
#define CP_PPASS  34 /* FTEXT in tree CONF */ /* max len 30 */
#define CP_FPPW   36 /* BUTTON in tree CONF */
#define CP_PPPW   37 /* BUTTON in tree CONF */
#define CP_TPPW   38 /* STRING in tree CONF */
#define CP_SLIP   41 /* BUTTON in tree CONF */
#define CP_PPP    43 /* BUTTON in tree CONF */
#define CP_VJHC   46 /* BUTTON in tree CONF */
#define CP_LAN    49 /* BUTTON in tree CONF */
#define CP_LTXT   50 /* STRING in tree CONF */
#define CA_BOX    52 /* BOX in tree CONF */
#define CA_USER   54 /* FTEXT in tree CONF */ /* max len 16 */
#define CA_FULL   55 /* FTEXT in tree CONF */ /* max len 32 */
#define CA_FQDN   57 /* FTEXT in tree CONF */ /* max len 42 */
#define CA_PREV   60 /* BUTTON in tree CONF */
#define CA_DNS    61 /* FTEXT in tree CONF */ /* max len 12 */
#define CA_NEXT   62 /* BUTTON in tree CONF */
#define CA_ADD    63 /* BUTTON in tree CONF */
#define CA_GETNS  65 /* BUTTON in tree CONF */
#define CA_NSTXT  66 /* STRING in tree CONF */
#define CG_BOX    68 /* BOX in tree CONF */
#define CG_PORT   72 /* BUTTON in tree CONF */
#define CG_DEFRT  75 /* BUTTON in tree CONF */
#define CG_MASQU  78 /* BUTTON in tree CONF */
#define CG_MTXT   79 /* STRING in tree CONF */
#define CG_LOGIN  81 /* BUTTON in tree CONF */
#define CG_CIX    84 /* BUTTON in tree CONF */
#define CG_RESI   87 /* BUTTON in tree CONF */
#define CG_RTXT   88 /* STRING in tree CONF */
#define CG_DEBUG  90 /* BUTTON in tree CONF */
#define CT_BOX    93 /* BOX in tree CONF */
#define CT_PREV   97 /* BUTTON in tree CONF */
#define CT_NUM    98 /* FTEXT in tree CONF */ /* max len 16 */
#define CT_NEXT   99 /* BUTTON in tree CONF */
#define CT_ADD   100 /* BUTTON in tree CONF */
#define CT_FEE   101 /* FTEXT in tree CONF */ /* max len 12 */
#define CI_TU    104 /* FTEXT in tree CONF */ /* max len 8 */
#define CI_TC    105 /* FTEXT in tree CONF */ /* max len 6 */
#define CI_SU    106 /* FTEXT in tree CONF */ /* max len 8 */
#define CI_SC    107 /* FTEXT in tree CONF */ /* max len 6 */
#define CI_RU    108 /* FTEXT in tree CONF */ /* max len 8 */
#define CI_RC    109 /* FTEXT in tree CONF */ /* max len 6 */
#define CD_BOX   112 /* BOX in tree CONF */
#define CD_TOUT  114 /* FTEXT in tree CONF */ /* max len 3 */
#define CD_IBOX  115 /* IBOX in tree CONF */
#define CD_PREV  118 /* BUTTON in tree CONF */
#define CD_STEP  119 /* FBOXTEXT in tree CONF */ /* max len 4 */
#define CD_NEXT  120 /* BUTTON in tree CONF */
#define CD_INS   121 /* BUTTON in tree CONF */
#define CD_DEL   122 /* BUTTON in tree CONF */
#define CD_TIME  123 /* FTEXT in tree CONF */ /* max len 6 */
#define CD_REPT  124 /* FTEXT in tree CONF */ /* max len 30 */
#define CD_FIND  125 /* FTEXT in tree CONF */ /* max len 30 */
#define CD_RESP  126 /* FTEXT in tree CONF */ /* max len 30 */
#define CN_BOX   128 /* BOX in tree CONF */
#define CN_PREV  132 /* BUTTON in tree CONF */
#define CN_OFF   133 /* FBOXTEXT in tree CONF */ /* max len 4 */
#define CN_NEXT  134 /* BUTTON in tree CONF */
#define CN_ADD   135 /* BUTTON in tree CONF */
#define CN_BIGBX 136 /* BOX in tree CONF */
#define CN_ED1   137 /* FTEXT in tree CONF */ /* max len 42 */
#define CN_ED2   138 /* FTEXT in tree CONF */ /* max len 42 */
#define CN_ED3   139 /* FTEXT in tree CONF */ /* max len 42 */
#define CN_ED4   140 /* FTEXT in tree CONF */ /* max len 42 */
#define CN_ED5   141 /* FTEXT in tree CONF */ /* max len 42 */
#define CN_ED6   142 /* FTEXT in tree CONF */ /* max len 42 */
#define CN_ED7   143 /* FTEXT in tree CONF */ /* max len 42 */
#define CN_SLDBX 144 /* BOX in tree CONF */
#define CN_UP    145 /* BOXCHAR in tree CONF */
#define CN_BACK  146 /* BOX in tree CONF */
#define CN_SLDR  147 /* BOX in tree CONF */
#define CN_DOWN  148 /* BOXCHAR in tree CONF */
#define CC_M_NAM 150 /* BUTTON in tree CONF */
#define CC_SAVE  151 /* BUTTON in tree CONF */
#define CC_SET   152 /* BUTTON in tree CONF */

#define PU_C_TYP   7 /* free form */

#define PU_C_PRT   8 /* free form */

#define O_MEM      9 /* free form */
#define OM_BOX     4 /* BOX in tree O_MEM */
#define OM_STING   5 /* BOX in tree O_MEM */
#define SKR_TTL    7 /* FTEXT in tree O_MEM */ /* max len 8 */
#define SKR_LBLK   8 /* FTEXT in tree O_MEM */ /* max len 8 */
#define OM_STRAM  10 /* BOX in tree O_MEM */
#define STR_TTL   12 /* FTEXT in tree O_MEM */ /* max len 8 */
#define STR_LBLK  13 /* FTEXT in tree O_MEM */ /* max len 8 */
#define OM_ARAM   15 /* BOX in tree O_MEM */
#define ALR_TTL   17 /* FTEXT in tree O_MEM */ /* max len 8 */
#define ALR_LBLK  18 /* FTEXT in tree O_MEM */ /* max len 8 */

#define O_STAT    10 /* free form */
#define OS_LOMEM   8 /* FTEXT in tree O_STAT */ /* max len 8 */
#define OS_TTLEX   9 /* FTEXT in tree O_STAT */ /* max len 8 */
#define OS_WCHKS  10 /* FTEXT in tree O_STAT */ /* max len 8 */
#define OS_UNDEL  11 /* FTEXT in tree O_STAT */ /* max len 8 */
#define OS_TOTAL  12 /* FTEXT in tree O_STAT */ /* max len 8 */
#define OS_PORT   16 /* BUTTON in tree O_STAT */
#define OSP_DROP  17 /* FTEXT in tree O_STAT */ /* max len 8 */
#define OSP_SENT  18 /* FTEXT in tree O_STAT */ /* max len 8 */
#define OSP_RECV  19 /* FTEXT in tree O_STAT */ /* max len 8 */
#define OS_LAYER  23 /* BUTTON in tree O_STAT */
#define OSL_DROP  24 /* FTEXT in tree O_STAT */ /* max len 8 */

#define PU_S_PRT  11 /* free form */

#define PU_S_LAY  12 /* free form */

#define O_ROUTE   13 /* free form */
#define OU_BIGBX  10 /* BOX in tree O_ROUTE */
#define OU_NET1   11 /* STRING in tree O_ROUTE */
#define OU_MSK1   12 /* STRING in tree O_ROUTE */
#define OU_PRT1   13 /* STRING in tree O_ROUTE */
#define OU_GTW1   14 /* STRING in tree O_ROUTE */
#define OU_NET2   15 /* STRING in tree O_ROUTE */
#define OU_MSK2   16 /* STRING in tree O_ROUTE */
#define OU_PRT2   17 /* STRING in tree O_ROUTE */
#define OU_GTW2   18 /* STRING in tree O_ROUTE */
#define OU_NET3   19 /* STRING in tree O_ROUTE */
#define OU_MSK3   20 /* STRING in tree O_ROUTE */
#define OU_PRT3   21 /* STRING in tree O_ROUTE */
#define OU_GTW3   22 /* STRING in tree O_ROUTE */
#define OU_NET4   23 /* STRING in tree O_ROUTE */
#define OU_MSK4   24 /* STRING in tree O_ROUTE */
#define OU_PRT4   25 /* STRING in tree O_ROUTE */
#define OU_GTW4   26 /* STRING in tree O_ROUTE */
#define OU_NET5   27 /* STRING in tree O_ROUTE */
#define OU_MSK5   28 /* STRING in tree O_ROUTE */
#define OU_PRT5   29 /* STRING in tree O_ROUTE */
#define OU_GTW5   30 /* STRING in tree O_ROUTE */
#define OU_NET6   31 /* STRING in tree O_ROUTE */
#define OU_MSK6   32 /* STRING in tree O_ROUTE */
#define OU_PRT6   33 /* STRING in tree O_ROUTE */
#define OU_GTW6   34 /* STRING in tree O_ROUTE */
#define OU_NET7   35 /* STRING in tree O_ROUTE */
#define OU_MSK7   36 /* STRING in tree O_ROUTE */
#define OU_PRT7   37 /* STRING in tree O_ROUTE */
#define OU_GTW7   38 /* STRING in tree O_ROUTE */
#define OU_NET8   39 /* STRING in tree O_ROUTE */
#define OU_MSK8   40 /* STRING in tree O_ROUTE */
#define OU_PRT8   41 /* STRING in tree O_ROUTE */
#define OU_GTW8   42 /* STRING in tree O_ROUTE */
#define OU_NET9   43 /* STRING in tree O_ROUTE */
#define OU_MSK9   44 /* STRING in tree O_ROUTE */
#define OU_PRT9   45 /* STRING in tree O_ROUTE */
#define OU_GTW9   46 /* STRING in tree O_ROUTE */
#define OU_NET10  47 /* STRING in tree O_ROUTE */
#define OU_MSK10  48 /* STRING in tree O_ROUTE */
#define OU_PRT10  49 /* STRING in tree O_ROUTE */
#define OU_GTW10  50 /* STRING in tree O_ROUTE */
#define OU_SLDBX  51 /* BOX in tree O_ROUTE */
#define OU_UP     52 /* BOXCHAR in tree O_ROUTE */
#define OU_PRNT   53 /* BOX in tree O_ROUTE */
#define OU_SLDR   54 /* BOX in tree O_ROUTE */
#define OU_DOWN   55 /* BOXCHAR in tree O_ROUTE */

#define O_RSLV    14 /* free form */
#define OR_BOX     4 /* BOX in tree O_RSLV */
#define R_HOST     7 /* FTEXT in tree O_RSLV */ /* max len 48 */
#define RA_BOX     9 /* BOX in tree O_RSLV */
#define R_ALIAS   10 /* FBOXTEXT in tree O_RSLV */ /* max len 48 */
#define RM_BOX    12 /* BOX in tree O_RSLV */
#define RIP_BOX   13 /* BOX in tree O_RSLV */
#define RIP_FRM   14 /* BOX in tree O_RSLV */
#define R_IP1     15 /* FTEXT in tree O_RSLV */ /* max len 15 */
#define R_IP2     16 /* FTEXT in tree O_RSLV */ /* max len 15 */
#define R_IP3     17 /* FTEXT in tree O_RSLV */ /* max len 15 */
#define R_IP4     18 /* FTEXT in tree O_RSLV */ /* max len 15 */
#define R_IP5     19 /* FTEXT in tree O_RSLV */ /* max len 15 */
#define R_IP6     20 /* FTEXT in tree O_RSLV */ /* max len 15 */
#define R_IP7     21 /* FTEXT in tree O_RSLV */ /* max len 15 */
#define R_IP8     22 /* FTEXT in tree O_RSLV */ /* max len 15 */
#define DO_RSLV   24 /* BUTTON in tree O_RSLV */

#define O_PING    15 /* form/dialog */
#define OP_HOST    7 /* FTEXT in tree O_PING */ /* max len 48 */
#define OP_OK      9 /* BUTTON in tree O_PING */

#define OP_DOIT   16 /* free form */
#define OP_IP      5 /* FTEXT in tree OP_DOIT */ /* max len 15 */
#define OP_SENT    6 /* FTEXT in tree OP_DOIT */ /* max len 5 */
#define OP_STOP    7 /* BUTTON in tree OP_DOIT */
#define OP_RECVD   8 /* FTEXT in tree OP_DOIT */ /* max len 5 */
#define OP_RTTBX  11 /* BOX in tree OP_DOIT */
#define OP_RTT1   12 /* FTEXT in tree OP_DOIT */ /* max len 5 */
#define OP_RTT2   13 /* FTEXT in tree OP_DOIT */ /* max len 5 */
#define OP_RTT3   14 /* FTEXT in tree OP_DOIT */ /* max len 5 */
#define OP_STBOX  18 /* BOX in tree OP_DOIT */
#define OP_MINI   19 /* FTEXT in tree OP_DOIT */ /* max len 5 */
#define OP_AVE    20 /* FTEXT in tree OP_DOIT */ /* max len 5 */
#define OP_MAXI   21 /* FTEXT in tree OP_DOIT */ /* max len 5 */

#define O_TRACE   17 /* form/dialog */
#define OT_HOST    7 /* FTEXT in tree O_TRACE */ /* max len 48 */
#define OT_OK      9 /* BUTTON in tree O_TRACE */

#define OT_DOIT   18 /* free form */
#define OT_DEST    5 /* FTEXT in tree OT_DOIT */ /* max len 48 */
#define OT_STATE   6 /* FTEXT in tree OT_DOIT */ /* max len 9 */
#define OT_BIGBX  10 /* BOX in tree OT_DOIT */
#define OT_HOP1   11 /* STRING in tree OT_DOIT */
#define OT_IP1    12 /* STRING in tree OT_DOIT */
#define OT_DN1    13 /* STRING in tree OT_DOIT */
#define OT_HOP2   14 /* STRING in tree OT_DOIT */
#define OT_IP2    15 /* STRING in tree OT_DOIT */
#define OT_DN2    16 /* STRING in tree OT_DOIT */
#define OT_HOP3   17 /* STRING in tree OT_DOIT */
#define OT_IP3    18 /* STRING in tree OT_DOIT */
#define OT_DN3    19 /* STRING in tree OT_DOIT */
#define OT_HOP4   20 /* STRING in tree OT_DOIT */
#define OT_IP4    21 /* STRING in tree OT_DOIT */
#define OT_DN4    22 /* STRING in tree OT_DOIT */
#define OT_HOP5   23 /* STRING in tree OT_DOIT */
#define OT_IP5    24 /* STRING in tree OT_DOIT */
#define OT_DN5    25 /* STRING in tree OT_DOIT */
#define OT_HOP6   26 /* STRING in tree OT_DOIT */
#define OT_IP6    27 /* STRING in tree OT_DOIT */
#define OT_DN6    28 /* STRING in tree OT_DOIT */
#define OT_HOP7   29 /* STRING in tree OT_DOIT */
#define OT_IP7    30 /* STRING in tree OT_DOIT */
#define OT_DN7    31 /* STRING in tree OT_DOIT */
#define OT_HOP8   32 /* STRING in tree OT_DOIT */
#define OT_IP8    33 /* STRING in tree OT_DOIT */
#define OT_DN8    34 /* STRING in tree OT_DOIT */
#define OT_HOP9   35 /* STRING in tree OT_DOIT */
#define OT_IP9    36 /* STRING in tree OT_DOIT */
#define OT_DN9    37 /* STRING in tree OT_DOIT */
#define OT_HOP10  38 /* STRING in tree OT_DOIT */
#define OT_IP10   39 /* STRING in tree OT_DOIT */
#define OT_DN10   40 /* STRING in tree OT_DOIT */
#define OT_SLDBX  41 /* BOX in tree OT_DOIT */
#define OT_UP     42 /* BOXCHAR in tree OT_DOIT */
#define OT_PRNT   43 /* BOX in tree OT_DOIT */
#define OT_SLDR   44 /* BOX in tree OT_DOIT */
#define OT_DOWN   45 /* BOXCHAR in tree OT_DOIT */




#ifdef __STDC__
#ifndef _WORD
#  ifdef WORD
#    define _WORD WORD
#  else
#    define _WORD short
#  endif
#endif
extern _WORD dialer_rsc_load(_WORD wchar, _WORD hchar);
extern _WORD dialer_rsc_gaddr(_WORD type, _WORD idx, void *gaddr);
extern _WORD dialer_rsc_free(void);
#endif
