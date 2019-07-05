;----------------------------------------------------------------------------
;File name:	URAn_DOS.SH			Revision date:	1998.08.12
;Creator:	Ulf Ronald Andersson		Creation date:	1991.01.02
;(c)1991 by:	Ulf Ronald Andersson		All rights reserved
;Released as:	FREEWARE			(commercial sale forbidden)
;----------------------------------------------------------------------------
;Purpose:	Symbol & Macro library for GEMDOS, XBIOS, and BIOS functions
;----------------------------------------------------------------------------
;Required header declarations:
;
;	include	"uran\struct.sh"
;	include	"uran\uran_sys.sh"
;	include	"uran\uran_dos.sh"
;
;----------------------------------------------------------------------------
;NB:	System error messages were badly defined by Atari to have a prefix
;	simply consisting of the letter 'E', which makes for hard reading.
;	Using underscore at prefix end would make this 'E_', but this is used
;       by many packages for proprietary error messages, since Atari had left
;	them 'free'.  I have therefore redefined the system error messages to
;	use a prefix of 'E~' to make them clearly distinguishable.
;
;	eg: Use E~OK, E~NOMEM, E~ACCESS, instead of EOK, ENOMEM, EACCESS.
;----------------------------------------------------------------------------
;	Special argument definitions:
;
__ARG__minus_1	=	$ffdecaff
__ARG__fast_0	=	$ffdeca00
__ARG__plus_1	=	$ffdeca01
__ARG__on_stack	=	$ffdeca02
;
;These are used in 'extended' modes
;eg:	'gemdos.3	Mshrink,__ARG__fast_0,__ARG__on_stack,#your_size'
;
;Each bit in the 'extend' mode hex digit matches an argument position, and
;the matching args will be tested for equality with the __ARG__* values,
;for special treatment.  This must be done this way, because generally
;arguments can't be evaluated at assembly time, and such comparisons would
;generate error messages preventing proper assembly.
;
;What I really wanted to do was to compare the argument strings as strings,
;but that is a feature completely missing from PASM.
;----------------------------------------------------------------------------
;	The TRAP_def macro is useful to define trap function dispatch codes
;
.MACRO	TRAP_def	trapnum,mnem,op,argc,pmask,lmask
trap__&mnem	EQU	trapnum
op__&mnem	EQU	op
argc__&mnem	EQU	argc
pm__&mnem	EQU	pmask
lm__&mnem	EQU	lmask
.IF	(op<4096)
mnem	EQU	((trapnum<<24)|(argc<<20)|(pmask<<16)|(lmask<<12)|op)
.ELSE
mnem	EQU	op
.ENDIF
.ENDM
;----------------------------------------------------------------------------
;
	TRAP_def  $1,	Pterm0,		$00,0,$0,$0
	TRAP_def  $1,	Cconin,		$01,0,$0,$0
	TRAP_def  $1,	Cconout,	$02,1,$0,$0
	TRAP_def  $1,	Cauxin,		$03,0,$0,$0
	TRAP_def  $1,	Cauxout,	$04,1,$0,$0
	TRAP_def  $1,	Cprnout,	$05,1,$0,$0
	TRAP_def  $1,	Crawio,		$06,1,$0,$0
	TRAP_def  $1,	Crawcin,	$07,0,$0,$0
	TRAP_def  $1,	Cnecin,		$08,0,$0,$0
	TRAP_def  $1,	Cconws,		$09,1,$1,$0
	TRAP_def  $1,	Cconrs,		$0A,1,$1,$0
	TRAP_def  $1,	Cconis,		$0B,0,$0,$0
	TRAP_def  $1,	Dsetdrv,	$0E,1,$0,$0
	TRAP_def  $1,	Cconos,		$10,0,$0,$0
	TRAP_def  $1,	Cprnos,		$11,0,$0,$0
	TRAP_def  $1,	Cauxis,		$12,0,$0,$0
	TRAP_def  $1,	Cauxos,		$13,0,$0,$0
	TRAP_def  $1,	Maddalt,	$14,2,$1,$2
	TRAP_def  $1,	Slbopen,	$16,5,$1B,$4	;\/ MagiC Shared libraries
	TRAP_def  $1,	Slbclose,	$17,1,$1,$0	;/\ also with BetaDOS & MetaDOS>2.74
	TRAP_def  $1,	Dgetdrv,	$19,0,$0,$0
	TRAP_def  $1,	Fsetdta,	$1A,1,$1,$0
	TRAP_def  $1,	Super,		$20,1,$1,$0
	TRAP_def  $1,	Tgetdate,	$2A,0,$0,$0
	TRAP_def  $1,	Tsetdate,	$2B,1,$0,$0
	TRAP_def  $1,	Tgettime,	$2C,0,$0,$0
	TRAP_def  $1,	Tsettime,	$2D,1,$0,$0
	TRAP_def  $1,	Fgetdta,	$2F,0,$0,$0
	TRAP_def  $1,	Sversion,	$30,0,$0,$0
	TRAP_def  $1,	Ptermres,	$31,2,$0,$1
	TRAP_def  $1,	Dfree,		$36,2,$1,$0
	TRAP_def  $1,	Dcreate,	$39,1,$1,$0
	TRAP_def  $1,	Ddelete,	$3A,1,$1,$0
	TRAP_def  $1,	Dsetpath,	$3B,1,$1,$0
	TRAP_def  $1,	Fcreate,	$3C,2,$1,$0
	TRAP_def  $1,	Fopen,		$3D,2,$1,$0
	TRAP_def  $1,	Fclose,		$3E,1,$0,$0
	TRAP_def  $1,	Fread,		$3F,3,$4,$2
	TRAP_def  $1,	Fwrite,		$40,3,$4,$2
	TRAP_def  $1,	Fdelete,	$41,1,$1,$0
	TRAP_def  $1,	Fseek,		$42,3,$0,$1
	TRAP_def  $1,	Fattrib,	$43,3,$1,$0
	TRAP_def  $1,	Mxalloc,	$44,2,$0,$1	;very modern TOS only
	TRAP_def  $1,	Fdup,		$45,1,$0,$0
	TRAP_def  $1,	Fforce,		$46,2,$0,$0
	TRAP_def  $1,	Dgetpath,	$47,2,$1,$0
	TRAP_def  $1,	Malloc,		$48,1,$0,$1
	TRAP_def  $1,	Mfree,		$49,1,$1,$0
	TRAP_def  $1,	Mshrink,	$4A,3,$2,$4
	TRAP_def  $1,	Pexec,		$4B,4,$E,$0
	TRAP_def  $1,	Pterm,		$4C,1,$0,$0
	TRAP_def  $1,	Fsfirst,	$4E,2,$1,$0
	TRAP_def  $1,	Fsnext,		$4F,0,$0,$0
	TRAP_def  $1,	Frename,	$56,3,$6,$0
	TRAP_def  $1,	Fdatime,	$57,3,$1,$0
;
	TRAP_def  $1,	Fcntl,		260,3,$0,$2
	TRAP_def  $1,	Finstat,	261,1,$0,$0
	TRAP_def  $1,	Foutstat,	262,1,$0,$0
	TRAP_def  $1,	Fgetchar,	263,2,$0,$0
	TRAP_def  $1,	Fputchar,	264,3,$0,$0
;
	TRAP_def  $1,	Pdomain,	281,1,$0,$0
;
	TRAP_def  $1,	Fselect,	285,4,$6,$8
;
	TRAP_def  $1,	Dpathconf,	292,2,$1,$0
;
	TRAP_def  $1,	Fmidipipe,	294,3,$0,$0
;
	TRAP_def  $1,	Dopendir,	296,2,$1,$0
	TRAP_def  $1,	Dreaddir,	297,3,$4,$2
	TRAP_def  $1,	Drewinddir,	298,1,$0,$1
	TRAP_def  $1,	Dclosedir,	299,1,$0,$1
	TRAP_def  $1,	Fxattr,		300,3,$6,$0
	TRAP_def  $1,	Flink,		301,2,$3,$0
	TRAP_def  $1,	Fsymlink,	302,2,$3,$0
	TRAP_def  $1,	Freadlink,	303,3,$6,$0
	TRAP_def  $1,	Dcntl,		304,3,$2,$4
	TRAP_def  $1,	Fchown,		305,3,$1,$0
	TRAP_def  $1,	Fchmod,		306,2,$1,$0
;
	TRAP_def  $1,	Dlock,		309,2,$0,$0
;
	TRAP_def  $1,	Dgetcwd,	316,3,$1,$0
;
	TRAP_def  $1,	Dxreaddir,	322,5,$1c,$2
;
	TRAP_def  $1,	Dreadlabel,	338,3,$3,$0
	TRAP_def  $1,	Dwritelabel,	339,2,$3,$0
;
;----------------------------------------------------------------------------
;	BIOS DEFINITIONS
;
	TRAP_def  $D,	Getmpb,		$000,1,$1,$0
	TRAP_def  $D,	Bconstat,	$001,1,$0,$0
	TRAP_def  $D,	Bconin,		$002,1,$0,$0
	TRAP_def  $D,	Bconout,	$003,2,$0,$0
	TRAP_def  $D,	Rwabs,		$004,5,$2,$0
	TRAP_def  $D,	Setexc,		$005,2,$2,$0
	TRAP_def  $D,	Tickal,		$006,0,$0,$0
	TRAP_def  $D,	Getbpb,		$007,1,$0,$0
	TRAP_def  $D,	Bcostat,	$008,1,$0,$0
	TRAP_def  $D,	Mediach,	$009,1,$0,$0
	TRAP_def  $D,	Drvmap,		$00A,0,$0,$0
	TRAP_def  $D,	Kbshift,	$00B,1,$0,$0
;
;----------------------------------------------------------------------------
;	XBIOS DEFINITIONS
;
	TRAP_def  $E,	Initmous,	$000,3,$6,$0
	TRAP_def  $E,	Ssbrk,		$001,2,$0,$0
	TRAP_def  $E,	Physbase,	$002,0,$0,$0
	TRAP_def  $E,	Logbase,	$003,0,$0,$0
	TRAP_def  $E,	Getrez,		$004,0,$0,$0
	TRAP_def  $E,	Setscreen,	$005,3,$3,$0
	TRAP_def  $E,	Setpallete,	$006,1,$1,$0
	TRAP_def  $E,	Setpalette,	$006,1,$1,$0
	TRAP_def  $E,	Setcolor,	$007,2,$0,$0
	TRAP_def  $E,	Floprd,		$008,7,$1,$2
	TRAP_def  $E,	Flopwr,		$009,7,$1,$2
	TRAP_def  $E,	Flopfmt,	$00A,7,$1,$F
;; CLR_D0		equ	$E0000B
	TRAP_def  $E,	Midiws,		$00C,2,$2,$0
	TRAP_def  $E,	Mfpint,		$00D,2,$2,$0
	TRAP_def  $E,	Iorec,		$00E,1,$0,$0
	TRAP_def  $E,	Rsconf,		$00F,6,$0,$0
	TRAP_def  $E,	Keytbl,		$010,3,$7,$0
	TRAP_def  $E,	Random,		$011,0,$0,$0
	TRAP_def  $E,	Protobt,	$012,4,$1,$2
	TRAP_def  $E,	Flopver,	$013,7,$1,$2
	TRAP_def  $E,	Scrdmp,		$014,0,$0,$0
	TRAP_def  $E,	Cursconf,	$015,2,$0,$0
	TRAP_def  $E,	Settime,	$016,1,$0,$1
	TRAP_def  $E,	Gettime,	$017,0,$0,$0
	TRAP_def  $E,	Bioskeys,	$018,0,$0,$0
	TRAP_def  $E,	Ikbdws,		$019,2,$2,$0
	TRAP_def  $E,	Jdisint,	$01A,1,$0,$0
	TRAP_def  $E,	Jenabint,	$01B,1,$0,$0
	TRAP_def  $E,	Giaccess,	$01C,2,$0,$0
	TRAP_def  $E,	Offgibit,	$01D,1,$0,$0
	TRAP_def  $E,	Ongibit,	$01E,1,$0,$0
	TRAP_def  $E,	Xbtimer,	$01F,4,$8,$0
	TRAP_def  $E,	Dosound,	$020,1,$1,$0
	TRAP_def  $E,	Setprt,		$021,1,$0,$0
	TRAP_def  $E,	Kbdvbase,	$022,0,$0,$0
	TRAP_def  $E,	Kbrate,		$023,2,$0,$0
	TRAP_def  $E,	Prtblk,		$024,1,$1,$0
	TRAP_def  $E,	Vsync,		$025,0,$0,$0
	TRAP_def  $E,	Supexec,	$026,1,$1,$0
	TRAP_def  $E,	Puntaes,	$027,0,$0,$0
	TRAP_def  $E,	Floprate,	$029,2,$0,$0     ;STE & Rainbow TOS
;-------------------------------------
; Dmaread	equ	$E4412A	;=> E~INVFN if absent
; Dmawrite	equ	$E4412B	;=> E~INVFN if absent
;-------------------------------------
	TRAP_def  $E,	Bconmap,	$02C,1,$0,$0	;-2=>struct  -1=>currdev  0=>test  6=>ST-port  7=>SCC_B  8=>TTMFP  9=>SCC_A
;-------------------------------------
; Nvmaccess	equ	$2E	;???
;-------------------------------------
;0 VOID Metainit ( META_INFO_1 *buffer );
;1 LONG Metaopen ( SHORT drive, META_DRVINFO *buffer );
;2 LONG Metaclose ( SHORT drive );
;3 LONG Metaread ( SHORT drive, VOID *buffer, LONG blockno, SHORT count );
;4 LONG Metawrite ( SHORT drive, VOID *buffer, LONG blockno, SHORT count );
;5
;6 LONG Metastatus ( SHORT drive, VOID *buffer );
;7 LONG Metaioctl ( SHORT drive, LONG magic, SHORT opcode, VOID *buffer );
;8
;9
;A
;B LONG Metastartaudio ( SHORT drive, SHORT flag, UBYTE *bytes_p);
;C LONG Metastopaudio ( SHORT drive );
;D LONG Metasetsongtime (SHORT drive,SHORT repeat,LONG starttime,LONG endtime );
;E LONG Metagettoc ( SHORT drive, SHORT flag, CD_TOC_ENTRY *buffer );
;F LONG Metadiscinfo ( SHORT drive, CD_DISC_INFO *p );
;-------------------------------------
	TRAP_def  $E,	Metainit,	$030,1,$1,$0
	TRAP_def  $E,	Metaopen,	$031,2,$2,$0
	TRAP_def  $E,	Metaclose,	$032,1,$0,$0
	TRAP_def  $E,	Metaread,	$033,4,$2,$4
	TRAP_def  $E,	Metawrite,	$034,4,$2,$4
;code $35 is unknown
	TRAP_def  $E,	Metastatus,	$036,2,$2,$0
	TRAP_def  $E,	Metaioctl,	$037,4,$8,$2
;code $38 is unknown
;code $39 is unknown
;code $3A is unknown
	TRAP_def  $E,	Metastartaudio,	$03B,3,$4,$0	;use: flag=0  bytes_p[2]={songs,startsong}
	TRAP_def  $E,	Metastopaudio,	$03C,1,$0,$0
	TRAP_def  $E,	Metasetsongtime,	$03D,4,$0,$C
	TRAP_def  $E,	Metagettoc,	$03E,3,$4,$0
	TRAP_def  $E,	Metadiscinfo,	$03F,2,$2,$0
;-------------------------------------
	struct	META_INFO_1
	uint32	MI1_drivemap
	char_p	MI1_version
	uint32	MI1_reserved
	struc_p	MI1_meta_info_2
	d_end	META_INFO_1
;
	struct	META_INFO_2
	uint16	MI2_version
	uint32	MI2_magic	;'_MET'
	char_p	MI2_log2phys
	d_end	META_INFO_2
;-------------------------------------
	TRAP_def  $E,	Blitmode,	$040,1,$0,$0	;all late TOS
;-------------------------------------
;	TT030 Video functions
;
	TRAP_def  $E,	EsetShift,	$050,1,$0,$0	;TT only
	TRAP_def  $E,	EgetShift,	$051,0,$0,$0	;TT only
	TRAP_def  $E,	EsetBank,	$052,1,$0,$0	;TT only
	TRAP_def  $E,	EsetColor,	$053,2,$0,$0	;TT only
	TRAP_def  $E,	EsetPalette,	$054,3,$4,$0	;TT only
	TRAP_def  $E,	EgetPalette,	$055,3,$4,$0	;TT only
	TRAP_def  $E,	EsetGray,	$056,1,$0,$0	;TT only
	TRAP_def  $E,	EsetSmear,	$057,1,$0,$0	;TT only
;-------------------------------------
;	F030 Video functions
;
	TRAP_def  $E,	VsetMode,	$058,1,$0,$0	;word(word mode)
	TRAP_def  $E,	Mon_type,	$059,0,$0,$0	;word(void)  0=SM, 1=SC, 2=VGA, 3=TV
	TRAP_def  $E,	VsetSync,	$05A,1,$0,$0	;void(word syncmode)
	TRAP_def  $E,	VgetSize,	$05B,1,$0,$0	;long(word mode)
;
	TRAP_def  $E,	VsetRGB	,	$05D,3,$4,$0	;void(word index,word count,long *array)
	TRAP_def  $E,	VgetRGB	,	$05E,3,$4,$0	;void(word index,word count,long *array)
;
;-------------------------------------
;	DSP functions
;
	TRAP_def  $E,	Dsp_DoBlock,		$060,4,$5,$A
	TRAP_def  $E,	Dsp_BlkHandShake,	$061,4,$5,$A
	TRAP_def  $E,	Dsp_BlkUnpacked,	$062,4,$5,$A	;void(long *TxBf, long TxSz, long *RxBf, long RxSz)
	TRAP_def  $E,	Dsp_InStream,		$063,4,$9,$6	;void(Dwrd *TxBf, long BkSz, long BkCt, long *OkCt)
	TRAP_def  $E,	Dsp_OutStream,		$064,4,$9,$6	;void(Dwrd *RxBf, long BkSz, long BkCt, long *OkCt)
	TRAP_def  $E,	Dsp_IOStream,		$065,6,$23,$1C
	TRAP_def  $E,	Dsp_RemoveInterrupts,	$066,1,$0,$0	;void(word mask)  bit_0 = Tx_off  bit_1 = Rx_off
	TRAP_def  $E,	Dsp_GetWordSize,	$067,0,$0,$0	;word(void)  => size in bytes
	TRAP_def  $E,	Dsp_Lock,		$068,0,$0,$0	;word(void)  => E~OK/E~ERROR
	TRAP_def  $E,	Dsp_Unlock,		$069,0,$0,$0	;void(void)
	TRAP_def  $E,	Dsp_Available,		$06A,2,$3,$0	;void(*xinfo, *yinfo)
	TRAP_def  $E,	Dsp_Reserve,		$06B,2,$0,$3	;word(xsize, ysize)  => E~OK/E~ERROR
	TRAP_def  $E,	Dsp_LoadProg,		$06C,3,$5,$0	;word(*file, word able, *buffer)  =>E~OK/E~ERROR
	TRAP_def  $E,	Dsp_ExecProg,		$06D,3,$1,$2	;void(*base, long size, word able)
	TRAP_def  $E,	Dsp_ExecBoot,		$06E,3,$1,$2	;void(*base, long size, word able)
	TRAP_def  $E,	Dsp_LodToBinary,	$06F,2,$3,$0	;long(*file, *dest)  => length/negative error
	TRAP_def  $E,	Dsp_TriggerHC,		$070,1,$0,$0	;void(word vector)  => DSP acts on vector (13/14 normally)
	TRAP_def  $E,	Dsp_RequestUniqueAbility,	$071,0,$0,$0 ;word(void)  => ability code
	TRAP_def  $E,	Dsp_GetProgAbility,	$072,0,$0,$0	;word(void)  => ability code
	TRAP_def  $E,	Dsp_FlushSubroutines,	$073,0,$0,$0	;void(void)
	TRAP_def  $E,	Dsp_LoadSubroutine,	$074,3,$1,$2	;void(*base, long size, word able) => handle/0
	TRAP_def  $E,	Dsp_InqSubrAbility,	$075,1,$0,$0	;word(word able)  => handle/0
	TRAP_def  $E,	Dsp_RunSubroutine,	$076,1,$0,$0	;word(word handle)  => E~OK/error_code
	TRAP_def  $E,	Dsp_Hf0,		$077,1,$0,$0	;word(word flag)  => set/clr/get HSR_3
	TRAP_def  $E,	Dsp_Hf1,		$078,1,$0,$0	;word(word flag)  => set/clr/get HSR_4
	TRAP_def  $E,	Dsp_Hf2,		$079,0,$0,$0	;word(void)  => get HCR_3
	TRAP_def  $E,	Dsp_Hf3,		$07A,0,$0,$0	;word(void)  => get HCR_4
	TRAP_def  $E,	Dsp_BlockWords,		$07B,4,$5,$A
	TRAP_def  $E,	BlkBytes,		$07C,4,$5,$A
	TRAP_def  $E,	Dsp_HStat,		$07D,0,$0,$0	;byte(void)  => get ISR
	TRAP_def  $E,	Dsp_SetVectors,		$07E,2,$3,$0	;void(*Rx, *Tx)  NULL => no transfer
	TRAP_def  $E,	Dsp_MultBlocks,		$07F,4,$C,$3
;
;-------------------------------------
;	Sound functions
;
	TRAP_def  $E,	Snd_LockSnd,		$080,0,$0,$0	;long(void)  => E_OK/E_ERROR		***
	TRAP_def  $E,	Snd_UnlockSnd,		$081,0,$0,$0	;long(void)  => E_OK/E_ERROR		***
	TRAP_def  $E,	Snd_SoundCmd,		$082,2,$0,$0	;long(word mode,word data)
	TRAP_def  $E,	Snd_SetBuffer,		$083,3,$6,$0	;long(word reg,void *beg_p,void *end_p)
	TRAP_def  $E,	Snd_SetMode,		$084,1,$0,$0	;long(word mode)  0:8b Stereo  1:16b stereo  2:8b mono
	TRAP_def  $E,	Snd_SetTracks,		$085,2,$0,$0	;long(word playtracks,word rectracks) NB: use tracks-1
	TRAP_def  $E,	Snd_SetMonTracks,	$086,1,$0,$0	;long(word montrack)
	TRAP_def  $E,	Snd_SetInterrupt,	$087,2,$0,$0	;long(word how,word when)  0,0 => disable interrupts
	TRAP_def  $E,	Snd_BufOper,		$088,1,$0,$0	;long(word)
	TRAP_def  $E,	Snd_Tristate,		$089,2,$0,$0	;long(word dsptx_f,word dsprx_f)
	TRAP_def  $E,	Snd_Gpio,		$08A,2,$0,$0	;long(word mode,word data)
	TRAP_def  $E,	Snd_DevConnect,		$08B,5,$0,$0	;long(word src,word dst,word clk,word presc,word pcol)
;		src=  0:DMA_out  1:DSP_out  2:Ext_inp  3:AD_mic
;		dst=  0:DMA_inp  1:DSP_inp  2:Ext_out  3:DA_spk
;		clk=  0:Int_25175_kHz  1:Ext_Clk  2:Int_32000kHz
;		presc=  ((clk_fq/256)/datarate)-1
;		pcol=   0:Handshake  1:none
;
	TRAP_def  $E,	Snd_SndStatus,		$08C,1,$0,$0	;long(word reset_f)  1 => reset AD & DA
	TRAP_def  $E,	Snd_BufPtr,		$08D,1,$1,$0	;long(long *destblk)
;
;-------------------------------------
;	more Video functions
;
	TRAP_def  $E,	VsetMask,		$096,3,$0,$0	;void(word ormask,word andmask,word overlay)
;
;----------------------------------------------------------------------------
;	Error code definitions
;
E~OK		equ	  0
E~ERROR		equ	 -1
E~IO		equ	E~ERROR
E~DRVNR		equ	 -2
E~UNCMD		equ	 -3
E~CRC		equ	 -4
E~BADRQ		equ	 -5
E~SEEK		equ	 -6
E~MEDIA		equ	 -7
E~SECNF		equ	 -8
E~PAPER		equ	 -9
E~WRITF		equ	-10
E~READF		equ	-11
E~GENRL		equ	-12
E~WRPRO		equ	-13
E~CHNG		equ	-14
E~AGAIN		equ	E~CHNG
E~UNDEV		equ	-15
E~BADSF		equ	-16
E~OTHER		equ	-17
E~INSERT	equ	-18
E~DEVNRSP	equ	-19
;
;	--------
;
E~INVFN		equ	-32
E~FILNF		equ	-33
E~NOENT		equ	E~FILNF
E~RSCH		equ	E~FILNF
E~PTHNF		equ	-34
E~NOTDIR	equ	E~PTHNF
E~NHNDL		equ	-35
E~ACCDN		equ	-36
E~ACCESS	equ	E~ACCDN
E~EXIST		equ	E~ACCDN
E~IHNDL		equ	-37
; ???		equ	-38
E~NSMEM		equ	-39
E~NOMEM		equ	E~NSMEM
E~IMBA		equ	-40
; ???		equ	-41
E~DFULL		equ	-42
; ???		equ	-43
; ???		equ	-44
; ???		equ	-45
E~DRIVE		equ	-46
; ???		equ	-47
E~NSAME		equ	-48
E~NMFIL		equ	-49
; ???		equ	-50
; ???		equ	-51
; ???		equ	-52
; ???		equ	-53
; ???		equ	-54
; ???		equ	-55
; ???		equ	-56
; ???		equ	-57
E~RLCKD		equ	-58
E~LOCK		equ	E~RLCKD
E~LOCKED	equ	E~RLCKD
E~MLNF		equ	-59
E~NSLOCK	equ	E~MLNF
; ???		equ	-60
; ???		equ	-61
; ???		equ	-62
; ???		equ	-63
E~RANGE		equ	-64
E~NAMETOOLONG	equ	E~RANGE
E~INTRN		equ	-65
E~PLFMT		equ	-66
E~NOEXEC	equ	E~PLFMT
E~GSBF		equ	-67
E~BREAK		equ	-68
E~XCPT		equ	-69	;MagiC
E~PTHOV		equ	-70
;
;	--------
;
E~LOOP		equ	-80
E~MOUNT		equ	-200
;
;----------------------------------------------------------------------------
;	standard file attribute bytes
;	used by Fattrib/Fsfirst
;
FA_RDONLY	equ	$01
FA_HIDDEN	equ	$02
FA_SYSTEM	equ	$04
FA_LABEL	equ	$08
FA_DIR		equ	$10
FA_SUBDIR	equ	$10
FA_CHANGED	equ	$20
;
b_FA_RDONLY	equ	0
b_FA_HIDDEN	equ	1
b_FA_SYSTEM	equ	2
b_FA_LABEL	equ	3
b_FA_DIR	equ	4
b_FA_SUBDIR	equ	4
b_FA_CHANGED	equ	5
;
;----------------------------------------------------------------------------
;	extended attribute structure
;	used by Fxattr/Dxreaddir
;
struct XATTR
	uint16	XATTR_mode		;file mode and access permissions
;bits 12-15:	file types
S_IFMT		equ	$F000		;mask to select file type
S_IFCHR		equ	$2000		;BIOS special file
S_IFDIR		equ	$4000		;directory file
S_IFREG 	equ	$8000		;regular file
S_IFIFO 	equ	$A000		;FIFO
S_IMEM		equ	$C000		;memory region or process
S_IFLNK		equ	$E000		;symbolic link
;
;bits 9-11:	setuid, setgid, sticky bit
S_ISUID		equ	04000
S_ISGID 	equ	02000
S_ISVTX		equ	01000

;bits 0-8:	file access modes for user, group, and other
S_IRUSR		equ	$100
S_IWUSR 	equ	$080
S_IXUSR 	equ	$040
S_IRGRP 	equ	$020
S_IWGRP		equ	$010
S_IXGRP		equ	$008
S_IROTH		equ	$004
S_IWOTH		equ	$002
S_IXOTH		equ	$001
DEFAULT_DIRMODE equ	$1FF
DEFAULT_MODE	equ	$1B6
	long	XATTR_index		;inode equivalent number
	uint16	XATTR_dev		;device number, 0..31 are normal gemdos devices
	uint16	XATTR_reserved1		;
	uint16	XATTR_nlink		;number of links
	uint16	XATTR_uid		;user id
	uint16	XATTR_gid		;group id
	long	XATTR_size		;file size
	long	XATTR_blksize		;block size
	long	XATTR_nblocks		;used blocks
	int16	XATTR_mtime		;\/ Modification time
	int16	XATTR_mdate		;/\ and date
	int16	XATTR_atime		;\/ Access time
	int16	XATTR_adate		;/\ and date
	int16	XATTR_ctime		;\/ Creation time
	int16	XATTR_cdate		;/\ and date
	int16	XATTR_attr		;standard attribute word
	int16	XATTR_reserved2		;
	char	reserved3,8		;
d_end	XATTR
;----------------------------------------------------------------------------
;	Boot sector offsets
;
	struct	boot_sector
	d_w	bs_codebra	;$00  w $60xx
	char	bs_filler,6	;$02 6b
	char	bs_serial,3	;$08 3b
	d_ow	bs_sectbytes	;$0b iw
	d_b	bs_clust_sects	;$0d  b
	d_ow	bs_boot_sects	;$0e iw
	d_b	bs_disk_FATs	;$10  b
	d_ow	bs_root_files	;$11 iw
	d_ow	bs_disk_sects	;$13 iw
	d_b	bs_media	;$15  b
	d_w	bs_FAT_sects	;$16 iw
	d_w	bs_track_sects	;$18 iw
	d_w	bs_disk_sides	;$1a iw
	d_w	bs_hide_sects	;$1c iw
; next follows the part used only by executable boots
	d_alias	bs_textbeg	;$1e program
; next follow definitions used only by executable boots of a special kind
	d_w	bs_execflg	;$1e   w
	d_w	bs_ldmode	;$20   w
	d_w	bs_ssect	;$22   w
	d_w	bs_sectcnt	;$24   w
	d_l	bs_ldaddr	;$26   L
	d_l	bs_fatbuf	;$2a   L
	char	bs_fname,11	;$2e 11b
	d_b	bs_fname_end	;$39   b
	char	bs_bootprog,512-bs_bootprog	;$3a .. $1ff
	d_end	boot_sector
;
;
;----------------------------------------------------------------------------
;	BPB offsets
;
	struct	bpb_struct
	d_w	bpb_sect_bytes	;typ:  512
	d_w	bpb_clust_sects	;typ:    2
	d_w	bpb_clust_bytes	;typ: 1024
	d_w	bpb_root_sects	;typ:    7
	d_w	bpb_FAT_sects	;typ:    5
	d_w	bpb_FAT2_start	;typ:    6
	d_w	bpb_data_start	;typ:   18 = 2*5 + 7 + 1
	d_w	bpb_data_clusts	;typ:  711 = 720 - 18/2
	d_w	bpb_flag	;b0==16bit_FAT/12bit_FAT typ: 0
	d_alias	bpb_size
	d_end	bpb_struct
;
;NB: data_start = FAT_sects*2 + root_sects + 1  rounded up for clust_sects alignment
;NB: data_clusts= Total_clusts - data_start/clust_sect
;
;
;----------------------------------------------------------------------------
;	Directory offsets
;
	struct	dir_struct
	char	dir_fname,8	;first char $00==unused $E5==erased
	char	dir_fext,3
	d_b	dir_fattr
	char	dir_reserved,10
	d_w	dir_ftime	;iw
	d_w	dir_fdate	;iw
	d_w	dir_fcluster	;iw
	d_l	dir_fsize	;il
	d_alias	dir_size
	d_end	dir_struct
;
;
;----------------------------------------------------------------------------
;	DTA offsets
;
	struct	dta_struct
	d_alias	dta_reserved	;this contains dta_gname..dta_gattr
	char	dta_gname,12
	d_l	dta_drive
	d_w	dta_dbpos
	d_w	dta_dsec0
	d_b	dta_gattr
	d_b	dta_fattr	;this is the first 'standard' element
	d_w	dta_ftime
	d_w	dta_fdate
	d_l	dta_fsize
	char	dta_fname,14
	d_alias	dta_size
	d_end	dta_struct
;
;
;----------------------------------------------------------------------------
;	Program file header offsets
;
	struct	program_header
	d_w	ph_codebra	;always $601A
	d_l	ph_textlen
	d_l	ph_datalen
	d_l	ph_bss_len
	d_l	ph_symblen
	d_l	ph_res1
	d_alias	ph_flag		;only TOS >= 1.4: bit 0 == fastload
	d_l	ph_res2
	d_w	ph_res3
	d_alias	ph_textbeg
	d_alias	ph_size
	d_end	program_header
;
;
;----------------------------------------------------------------------------
;	Buffer Control Block offsets
;
	struct	bcb_struct
	d_l	bcb_link_p
	d_w	bcb_drive
	d_w	bcb_type
	d_w	bcb_record
	d_w	bcb_dirty
	d_l	bcb_dm_p
	d_l	bcb_data_p
	d_alias	bcb_size
	d_end	bcb_struct
;
;
;----------------------------------------------------------------------------
;	Basepage offsets
;
	struct	BasePage
	d_l	bp_selfbeg_p
	d_l	bp_selfend_p
	d_l	bp_textbeg_p
	d_l	bp_textlen
	d_l	bp_databeg_p
	d_l	bp_datalen
	d_l	bp_bss_beg_p
	d_l	bp_bss_len
	d_l	bp_dta_p
	d_l	bp_parent_p
	d_l	bp_reserved
	d_l	bp_environ_p
	char	bp_undef,80
	d_b	bp_arglen
	char	bp_argstring,127
	d_end	BasePage
;
;
;----------------------------------------------------------------------------
;	MACRO DEFINITIONS
;
.MACRO	gemdos.md	op,v2,v3,v4,v5,v6,v7
	zz_prep_trap.md	op,v2,v3,v4,v5,v6,v7
	trap	#(trap__&op & !_ind)
	ifne	zzDOSOP-Pterm0
	ifne	zzDOSOP-Pterm
	ifne	zzDOSOP-Ptermres
	ifne	(zzDOSAS>0)&(zzSTkeep=0)
	ifne	(zzDOSAS>8)|zzDUMBcomp
	lea	zzDOSAS(sp),sp
	else
	addq	#zzDOSAS,sp
	endif
	endif
	endif
	endif
	endif
.ENDM	gemdos
;
.MACRO	sub_gemdos.md	op,v2,v3,v4,v5,v6,v7
	zz_prep_trap.md	op,v2,v3,v4,v5,v6,v7
	bsr	gemdos_sub
	ifne	zzDOSOP-Pterm0
	ifne	zzDOSOP-Pterm
	ifne	zzDOSOP-Ptermres
	ifne	(zzDOSAS>0)&(zzSTkeep=0)
	ifne	(zzDOSAS>8)|zzDUMBcomp
	lea	zzDOSAS(sp),sp
	else
	addq	#zzDOSAS,sp
	endif
	endif
	endif
	endif
	endif
.ENDM	sub_gemdos
;
.MACRO	bios.md		op,v2,v3,v4,v5,v6
	zz_prep_trap.md	op,v2,v3,v4,v5,v6
	trap	#(trap__&op & !_ind)
	ifne	(zzDOSAS>0)&(zzSTkeep=0)
	ifne	(zzDOSAS>8)|zzDUMBcomp
	lea	zzDOSAS(sp),sp
	else
	addq	#zzDOSAS,sp
	endif
	endif
.ENDM	bios
;
.MACRO	sub_bios.md	op,v2,v3,v4,v5,v6
	zz_prep_trap.md	op,v2,v3,v4,v5,v6
	bsr	bios_sub
	ifne	(zzDOSAS>0)&(zzSTkeep=0)
	ifne	(zzDOSAS>8)|zzDUMBcomp
	lea	zzDOSAS(sp),sp
	else
	addq	#zzDOSAS,sp
	endif
	endif
.ENDM	sub_bios
;
.MACRO	xbios.md	op,v2,v3,v4,v5,v6,v7,v8,v9,va
	zz_prep_trap.md	op,v2,v3,v4,v5,v6,v7,v8,v9,va
	trap	#(trap__&op & !_ind)
	ifne	(zzDOSAS>0)&(zzSTkeep=0)
	ifne	(zzDOSAS>8)|zzDUMBcomp
	lea	zzDOSAS(sp),sp
	else
	addq	#zzDOSAS,sp
	endif
	endif
.ENDM	xbios
;
.MACRO	sub_xbios.md	op,v2,v3,v4,v5,v6,v7,v8,v9,va
	zz_prep_trap.md	op,v2,v3,v4,v5,v6,v7,v8,v9,va
	bsr	xbios_sub
	ifne	(zzDOSAS>0)&(zzSTkeep=0)
	ifne	(zzDOSAS>8)|zzDUMBcomp
	lea	zzDOSAS(sp),sp
	else
	addq	#zzDOSAS,sp
	endif
	endif
.ENDM	sub_xbios
;
.MACRO	zz_prep_trap.md	op,v2,v3,v4,v5,v6,v7,v8,v9,va
zzDOSIF	set	(op)&_ind	;old style indirection mode ?
	.IF	'&.md'=='.i'	;new style indirection mode ?
zzDOSIF	set	_ind
	.ENDIF
zzDOSAC	set	(argc__&op & !_ind)
zzDOSPF	set	(pm__&op & !_ind)
zzDOSLF	set	(lm__&op & !_ind)
zzDOSOP	set	(op__&op & !_ind)
zzDOSAS	set	-zzSToff
	IFNE	zzDOSAC>0
	IFNE	zzDOSAC>1
	IFNE	zzDOSAC>2
	IFNE	zzDOSAC>3
	IFNE	zzDOSAC>4
	IFNE	zzDOSAC>5
	IFNE	zzDOSAC>6
	IFNE	zzDOSAC>7
	IFNE	zzDOSAC>8
	IFNE	zzDOSLF=15
zzDOSLF	set	130
	endif
	DOSPUSH.md	8,va,op
	else
	ifnb	va
	.error	"More than 8 args in '&op &v2,&v3,&v4,&v5,&v6,&v7,&v8,&v9,&va'"
	endif
	endif
	DOSPUSH.md	7,v9,op
	else
	ifnb	v9
	.error	"More than 7 args in '&op &v2,&v3,&v4,&v5,&v6,&v7,&v8,&v9'"
	endif
	endif
	DOSPUSH.md	6,v8,op
	else
	ifnb	v8
	.error	"More than 6 args in '&op &v2,&v3,&v4,&v5,&v6,&v7,&v8'"
	endif
	endif
	DOSPUSH.md	5,v7,op
	else
	ifnb	v7
	.error	"More than 5 args in '&op &v2,&v3,&v4,&v5,&v6,&v7'"
	endif
	endif
	DOSPUSH.md	4,v6,op
	else
	ifnb	v6
	.error	"More than 4 args in '&op &v2,&v3,&v4,&v5,&v6'"
	endif
	endif
	DOSPUSH.md	3,v5,op
	else
	ifnb	v5
	.error	"More than 3 args in '&op &v2,&v3,&v4,&v5'"
	endif
	endif
	DOSPUSH.md	2,v4,op
	else
	ifnb	v4
	.error	"More than 2 args in '&op &v2,&v3,&v4'"
	endif
	endif
	DOSPUSH.md	1,v3,op
	else
	ifnb	v3
	.error	"More than 1 arg in '&op &v2,&v3'"
	endif
	endif
	DOSPUSH.md	0,v2,op
	else
	ifnb	v2
	.error	"More than 0 args in '&op &v2'"
	endif
	endif
zzDOSAS	set	zzDOSAS+2
	ifne	1&zzDOSAS
	.error	"Stack offset is odd in &op call"
	endif
	ifgt	zzDOSAS
	MOVE	#zzDOSOP,-(sp)
	else
	ifeq	zzDOSAS
	move	#zzDOSOP,(sp)
	else
	.error	"Stack offset is too large in &op call"
	endif
	endif
.ENDM
;
;The next macro handles argument checking for bios, xbios & gemdos functions
;special argument cases are:
;no argument = .error error
;'()'	= Assumes argument is on stack already
;'!'	= Places 0 on stack using "clr"
;'?'	= Places -1 on stack
;
;Actually the above only worked for the DEVPAC version, since PASM does not
;allow tests on argument strings, but only on their symbolic values.
;This is instead handled by special modes and reserved values in PASM.
;Because that uses the same mode argument as the newer indirection method,
;the old method must be used if indirection is needed in a call that also
;uses a 'special' argument.  Only the first four arguments can use special
;argument values, and the 'mode' is a hex digit bit mask for those args.
;
.MACRO	DOSPUSH.md	v1,v2,op
zzDOSmd	set	0
	.IF	'&.md'=='.1'	;special value in arg 1
zzDOSmd	set	1
	.ELSE
	.IF	'&.md'=='.2'	;special value in arg 2
zzDOSmd	set	2
	.ELSE
	.IF	'&.md'=='.3'	;special value in args 2,1
zzDOSmd	set	3
	.ELSE
	.IF	'&.md'=='.4'	;special value in arg 3
zzDOSmd	set	4
	.ELSE
	.IF	'&.md'=='.5'	;special value in args 3,1
zzDOSmd	set	5
	.ELSE
	.IF	'&.md'=='.6'	;special value in args 3,2
zzDOSmd	set	6
	.ELSE
	.IF	'&.md'=='.7'	;special value in args 3,2,1
zzDOSmd	set	7
	.ELSE
	.IF	'&.md'=='.8'	;special value in arg 4
zzDOSmd	set	8
	.ELSE
	.IF	'&.md'=='.9'	;special value in args 4,1
zzDOSmd	set	9
	.ELSE
	.IF	'&.md'=='.a'	;special value in args 4,2
zzDOSmd	set	10
	.ELSE
	.IF	'&.md'=='.b'	;special value in args 4,2,1
zzDOSmd	set	11
	.ELSE
	.IF	'&.md'=='.c'	;special value in args 4,3
zzDOSmd	set	12
	.ELSE
	.IF	'&.md'=='.d'	;special value in args 4,3,1
zzDOSmd	set	13
	.ELSE
	.IF	'&.md'=='.e'	;special value in args 4,3,2
zzDOSmd	set	14
	.ELSE
	.IF	'&.md'=='.f'	;special value in args 4,3,2,1
zzDOSmd	set	15
	.ELSE
;;;patch future extensions here
	.ENDIF	;f
	.ENDIF	;e
	.ENDIF	;d
	.ENDIF	;c
	.ENDIF	;b
	.ENDIF	;a
	.ENDIF	;9
	.ENDIF	;8
	.ENDIF	;7
	.ENDIF	;6
	.ENDIF	;5
	.ENDIF	;4
	.ENDIF	;3
	.ENDIF	;2
	.ENDIF	;1
	.IFNE	(1<<v1)&zzDOSmd
zzDOSmd	set	v2
	.ELSE
zzDOSmd	set	0
	.ENDIF	;bit check	
zzDOSAS	set	zzDOSAS+2
	.IFNE	(1<<v1)&(zzDOSPF|zzDOSLF)
zzDOSAS	set	zzDOSAS+2
	.ENDIF
	.IFNE	1&zzDOSAS
	.error	"Stack offset is odd in &op call"
	.ENDIF
	.IFB	v2
	.error	"Missing operand in &op call"
	.EXITM
	.ENDIF
	.IFNE	(zzDOSmd == __ARG__on_stack)
	.EXITM
	.ENDIF
	ifgt	zzDOSAS
	.IFNE	(1<<v1)&zzDOSPF
	iflt	zzDOSAS-4
	.error	"Pointer argument crosses stack offset in &op call"
	.ENDIF
	.IFNE	(zzDOSmd == __ARG__fast_0)
	clr.l	-(sp)
	.EXITM
	.ENDIF
	.IFNE	(zzDOSmd == __ARG__minus_1)
	pea	(-1).w
	.EXITM
	.ENDIF
	.IFNE	(zzDOSmd == __ARG__plus_1)
	pea	(1).w
	.EXITM
	.ENDIF
	IFEQ	zzDOSIF
	pea	v2
	exitm
	endif
	move.l	v2,-(sp)
	exitm
	endif			;ends pointer case
	IFNE	(1<<v1)&zzDOSLF
	iflt	zzDOSAS-4
	.error	"Long argument crosses stack offset in &op call"
	endif
	.IFNE	(zzDOSmd == __ARG__fast_0)
	clr.l	-(sp)
	.EXITM
	.ENDIF
	.IFNE	(zzDOSmd == __ARG__minus_1)
	pea	(-1).w
	.EXITM
	.ENDIF
	.IFNE	(zzDOSmd == __ARG__plus_1)
	pea	(1).w
	.EXITM
	.ENDIF
	move.l	v2,-(sp)
	exitm
	endif			;ends long case
	.IFNE	(zzDOSmd == __ARG__fast_0)
	clr	-(sp)
	.EXITM
	.ENDIF
	.IFNE	(zzDOSmd == __ARG__minus_1)
	move	#-1,-(sp)
	.EXITM
	.ENDIF
	.IFNE	(zzDOSmd == __ARG__plus_1)
	move	#1,-(sp)
	.EXITM
	.ENDIF
	move	v2,-(sp)	;ends word case
	exitm
	else		;start stack offset cases	
	ifeq	zzDOSAS
	IFNE	(1<<v1)&zzDOSPF
	.IFNE	(zzDOSmd == __ARG__fast_0)
	clr.l	(sp)
	.EXITM
	.ENDIF
	.IFNE	(zzDOSmd == __ARG__minus_1)
	move.l	#-1,(sp)
	.EXITM
	.ENDIF
	.IFNE	(zzDOSmd == __ARG__plus_1)
	move.l	#1,(sp)
	.EXITM
	.ENDIF
	IFEQ	zzDOSIF
	addq	#4,sp
	pea	v2
	exitm
	endif
	move.l	v2,(sp)
	exitm
	endif			;ends pointer case
	IFNE	(1<<v1)&zzDOSLF
	.IFNE	(zzDOSmd == __ARG__fast_0)
	clr.l	(sp)
	.EXITM
	.ENDIF
	.IFNE	(zzDOSmd == __ARG__minus_1)
	move.l	#-1,(sp)
	.EXITM
	.ENDIF
	.IFNE	(zzDOSmd == __ARG__plus_1)
	move.l	#1,(sp)
	.EXITM
	.ENDIF
	move.l	v2,(sp)
	exitm
	endif			;ends long case
	.IFNE	(zzDOSmd == __ARG__fast_0)
	clr	(sp)
	.EXITM
	.ENDIF
	.IFNE	(zzDOSmd == __ARG__minus_1)
	move	#-1,(sp)
	.EXITM
	.ENDIF
	.IFNE	(zzDOSmd == __ARG__plus_1)
	move	#1,(sp)
	.EXITM
	.ENDIF
	move	v2,(sp)	;ends word case
	exitm
	else		;handle negative offset levels
	IFNE	(1<<v1)&zzDOSPF
	.IFNE	(zzDOSmd == __ARG__fast_0)
	clr.l	-zzDOSAS(sp)
	.EXITM
	.ENDIF
	.IFNE	(zzDOSmd == __ARG__minus_1)
	move.l	#-1,-zzDOSAS(sp)
	.EXITM
	.ENDIF
	.IFNE	(zzDOSmd == __ARG__plus_1)
	move.l	#1,-zzDOSAS(sp)
	.EXITM
	.ENDIF
	IFEQ	zzDOSIF
	move.l	#v2,-zzDOSAS(sp)
	exitm
	endif
	move.l	v2,-zzDOSAS(sp)
	exitm
	endif			;ends pointer case
	IFNE	(1<<v1)&zzDOSLF
	.IFNE	(zzDOSmd == __ARG__fast_0)
	clr.l	-zzDOSAS(sp)
	.EXITM
	.ENDIF
	.IFNE	(zzDOSmd == __ARG__minus_1)
	move.l	#-1,-zzDOSAS(sp)
	.EXITM
	.ENDIF
	.IFNE	(zzDOSmd == __ARG__plus_1)
	move.l	#1,-zzDOSAS(sp)
	.EXITM
	.ENDIF
	move.l	v2,-zzDOSAS(sp)
	exitm
	endif			;ends long case
	.IFNE	(zzDOSmd == __ARG__fast_0)
	clr	-zzDOSAS(sp)
	.EXITM
	.ENDIF
	.IFNE	(zzDOSmd == __ARG__minus_1)
	move	#-1,-zzDOSAS(sp)
	.EXITM
	.ENDIF
	.IFNE	(zzDOSmd == __ARG__plus_1)
	move	#1,-zzDOSAS(sp)
	.EXITM
	.ENDIF
	move	v2,-zzDOSAS(sp)	;ends word case
	exitm
	endif
	endif
.ENDM
;
zzDOSAS		set	0
zzSToff		set	0
zzSTkeep	set	0
zzDUMBcomp	set	0
;
;----------------------------------------------------------------------------
;End of file:	URAn_DOS.SH
;----------------------------------------------------------------------------
