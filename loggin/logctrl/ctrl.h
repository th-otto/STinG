/*
 * resource set indices for ctrl
 *
 * created by ORCS 2.16
 */

/*
 * Number of Strings:        162
 * Number of Bitblks:        0
 * Number of Iconblks:       0
 * Number of Color Iconblks: 0
 * Number of Color Icons:    0
 * Number of Tedinfos:       3
 * Number of Free Strings:   0
 * Number of Free Images:    0
 * Number of Objects:        245
 * Number of Trees:          3
 * Number of Userblks:       0
 * Number of Images:         0
 * Total file size:          7396
 */

#undef RSC_NAME
#ifndef __ALCYON__
#define RSC_NAME "ctrl"
#endif
#undef RSC_ID
#ifdef ctrl
#define RSC_ID ctrl
#else
#define RSC_ID 0
#endif

#ifndef RSC_STATIC_FILE
# define RSC_STATIC_FILE 0
#endif
#if !RSC_STATIC_FILE
#define NUM_STRINGS 162
#define NUM_FRSTR 0
#define NUM_UD 0
#define NUM_IMAGES 0
#define NUM_BB 0
#define NUM_FRIMG 0
#define NUM_IB 0
#define NUM_CIB 0
#define NUM_TI 3
#define NUM_OBS 245
#define NUM_TREE 3
#endif



#define LOGCTRL    0 /* form/dialog */
#define VERSION    4 /* STRING in tree LOGCTRL */
#define ACTIVE     6 /* BUTTON in tree LOGCTRL */
#define PARAS      9 /* BUTTON in tree LOGCTRL */
#define POINTER   12 /* BUTTON in tree LOGCTRL */
#define BUFFER    15 /* BUTTON in tree LOGCTRL */
#define INTERNAL  18 /* BUTTON in tree LOGCTRL */
#define CACHE     21 /* BUTTON in tree LOGCTRL */
#define FILE      25 /* BUTTON in tree LOGCTRL */
#define FILENAM   27 /* FBOXTEXT in tree LOGCTRL */ /* max len 30 */
#define PIPE      28 /* BUTTON in tree LOGCTRL */
#define PIPENAM   29 /* STRING in tree LOGCTRL */
#define CLIENT    30 /* BUTTON in tree LOGCTRL */
#define TERM      31 /* BUTTON in tree LOGCTRL */
#define MODULE    32 /* BUTTON in tree LOGCTRL */
#define FIX       33 /* BUTTON in tree LOGCTRL */
#define CANCEL    34 /* BUTTON in tree LOGCTRL */

#define CLI_API    1 /* form/dialog */
#define C_TOPEN    4 /* BUTTON in tree CLI_API */
#define C_TCLOSE   7 /* BUTTON in tree CLI_API */
#define C_TSEND   10 /* BUTTON in tree CLI_API */
#define C_TSTATE  13 /* BUTTON in tree CLI_API */
#define C_TACK    16 /* BUTTON in tree CLI_API */
#define C_TINFO   19 /* BUTTON in tree CLI_API */
#define C_UOPEN   23 /* BUTTON in tree CLI_API */
#define C_UCLOSE  26 /* BUTTON in tree CLI_API */
#define C_USEND   29 /* BUTTON in tree CLI_API */
#define C_UINFO   32 /* BUTTON in tree CLI_API */
#define C_ISEND   36 /* BUTTON in tree CLI_API */
#define C_IHNDLR  39 /* BUTTON in tree CLI_API */
#define C_IDSCRD  42 /* BUTTON in tree CLI_API */
#define C_CGNDB   46 /* BUTTON in tree CLI_API */
#define C_CGBLK   49 /* BUTTON in tree CLI_API */
#define C_CGCHAR  52 /* BUTTON in tree CLI_API */
#define C_CGS     55 /* BUTTON in tree CLI_API */
#define C_CBCNT   58 /* BUTTON in tree CLI_API */
#define C_CGINFO  61 /* BUTTON in tree CLI_API */
#define C_CKICK   64 /* BUTTON in tree CLI_API */
#define C_KMALL   68 /* BUTTON in tree CLI_API */
#define C_KREALL  71 /* BUTTON in tree CLI_API */
#define C_KFREE   74 /* BUTTON in tree CLI_API */
#define C_KGETFR  77 /* BUTTON in tree CLI_API */
#define C_GTVSTR  81 /* BUTTON in tree CLI_API */
#define C_STVSTR  84 /* BUTTON in tree CLI_API */
#define C_ONPRT   88 /* BUTTON in tree CLI_API */
#define C_OFFPRT  91 /* BUTTON in tree CLI_API */
#define C_QUPRT   94 /* BUTTON in tree CLI_API */
#define C_CTLPRT  97 /* BUTTON in tree CLI_API */
#define C_RSLV   100 /* BUTTON in tree CLI_API */
#define C_SETFLG 104 /* BUTTON in tree CLI_API */
#define C_CLRFLG 107 /* BUTTON in tree CLI_API */
#define C_ERRTXT 110 /* BUTTON in tree CLI_API */
#define C_HSKEEP 114 /* BUTTON in tree CLI_API */
#define C_CARDET 117 /* BUTTON in tree CLI_API */
#define C_SEREN  120 /* BUTTON in tree CLI_API */
#define C_SERDIS 123 /* BUTTON in tree CLI_API */
#define C_ALL    130 /* BUTTON in tree CLI_API */
#define C_NONE   131 /* BUTTON in tree CLI_API */
#define C_OK     132 /* BUTTON in tree CLI_API */
#define C_CANCEL 133 /* BUTTON in tree CLI_API */

#define MOD_API    2 /* form/dialog */
#define M_IPSEND   4 /* BUTTON in tree MOD_API */
#define M_IPFTCH   7 /* BUTTON in tree MOD_API */
#define M_IPHAND  10 /* BUTTON in tree MOD_API */
#define M_IPDISC  13 /* BUTTON in tree MOD_API */
#define M_PANNOU  17 /* BUTTON in tree MOD_API */
#define M_PGTPAR  20 /* BUTTON in tree MOD_API */
#define M_PREQU   23 /* BUTTON in tree MOD_API */
#define M_PRELEA  26 /* BUTTON in tree MOD_API */
#define M_PLOOK   29 /* BUTTON in tree MOD_API */
#define M_EXEC    32 /* BUTTON in tree MOD_API */
#define M_TCALL   36 /* BUTTON in tree MOD_API */
#define M_TNOW    39 /* BUTTON in tree MOD_API */
#define M_TELAPS  42 /* BUTTON in tree MOD_API */
#define M_SETTTL  46 /* BUTTON in tree MOD_API */
#define M_CHKTTL  49 /* BUTTON in tree MOD_API */
#define M_SETSYS  52 /* BUTTON in tree MOD_API */
#define M_QUCHNS  55 /* BUTTON in tree MOD_API */
#define M_GTROUT  59 /* BUTTON in tree MOD_API */
#define M_STROUT  62 /* BUTTON in tree MOD_API */
#define M_LDROUT  65 /* BUTTON in tree MOD_API */
#define M_ALL     72 /* BUTTON in tree MOD_API */
#define M_NONE    73 /* BUTTON in tree MOD_API */
#define M_OK      74 /* BUTTON in tree MOD_API */
#define M_CANCEL  75 /* BUTTON in tree MOD_API */




#ifdef __STDC__
#ifndef _WORD
#  ifdef WORD
#    define _WORD WORD
#  else
#    define _WORD short
#  endif
#endif
extern _WORD ctrl_rsc_load(_WORD wchar, _WORD hchar);
extern _WORD ctrl_rsc_gaddr(_WORD type, _WORD idx, void *gaddr);
extern _WORD ctrl_rsc_free(void);
#endif
