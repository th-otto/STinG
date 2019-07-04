#ifndef GNU_ASM_NAME
#ifdef __GNUC__
#define GNU_ASM_NAME(x) __asm__(x)
#else
#define GNU_ASM_NAME(x)
#endif
#endif

#define PORT_MFP_1 0
#define PORT_SCC_B 1
#define PORT_MFP_2 2
#define PORT_SCC_A 3

/*-----------------------------*/
/*--- types                 ---*/
/*-----------------------------*/

typedef enum 
{
	DEV_STANDARD = 0,
	DEV_FSER,
	DEV_RSVF,
	DEV_MINT,
	DEV_MAGIC
} DEV_TYPES;

typedef struct
{
	/*
	 * Sichtbar fÅr den User:
	 */
	DEV_LIST device;					/* Liste der Device-Namen   */

	/*
	 * Sytemdaten:
	 */
	WORD bios;							/* BIOS-Nr. des Devices                 */
	WORD func_num;						/* "normierte" Bconmap-Zugriffsnummer   */
	const char *dopen;					/* Name des Devices fÅr Fopen( )        */

	/*
	 * Daten zur Verwaltung des Devices:
	 */
	DEV_TYPES type;						/* Typ des Devices (RSVF, MiNT, Mag!C)  */
	BOOLEAN is_open;					/* TRUE, falls Device geîffnet          */
	WORD dhandle;						/* handle, falls mit Fopen( ) geîffnet  */
	WORD curr_pos;						/* aktulle Position im Puffer           */
	WORD num_read;						/* Anzahl der eingelesenen Zeichen      */

	BYTE *buf;							/* Zeiger Blockdevice-Puffer            */
	LONG *speeds;						/* Liste der einstellbaren DTE-speeds   */
	CHAN_INFO *chan_info;				/* Struktur unter FastSeriell           */
	MAPTAB *func_map;					/* Tabelle der einzelnen Funktionen     */
	WORD oldIBufSize;
	WORD oldOBufSize;
	BYTE *oldIBufPtr;
	BYTE *oldOBufPtr;
	LONG ioctrlmap[6];					/* Index der mîglichen Fcntl-Funktionen */
} DEVICES;

typedef union
{
	UWORD word;
	struct
	{
		UWORD bit7_15:9;
		UWORD bit5_6:2;
		UWORD bit3_4:2;
		UWORD bit2:1;
		UWORD bit1:1;
		UWORD bit0:1;
	} bits;
} UCR;

extern void (*pause_2)(void);

BOOLEAN is_dcd(WORD func);
void high_dtr(WORD func, MAPTAB *map);
void low_dtr(WORD func, MAPTAB *map);
BOOLEAN	SendBlock(DEVICES *dev, const void *block, LONG len, BOOLEAN tst_dcd);
LONG GetBlock(DEVICES *dev, LONG bufflen, void *buff);
void SetMapM1(MAPTAB **map);
void SetMapMidi(MAPTAB **map);
void SetIorec(IOREC *iorec, BYTE *blk, WORD len);
