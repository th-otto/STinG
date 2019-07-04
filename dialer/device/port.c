#include "device.h"
#include "types.h"
#include "port.h"
#include <mint/sysvars.h>


/* *** Hardware-Adressen: */
#define mfp1		(*((volatile unsigned char *)0xfffffa01UL))
#define mfp2		(*((volatile unsigned char *)0xfffffa81UL))
#define sccctl_a	(*((volatile unsigned char *)0xffff8c81UL))
#define sccdat_a	(*((volatile unsigned char *)0xffff8c83UL))
#define sccctl_b	(*((volatile unsigned char *)0xffff8c85UL))
#define sccdat_b	(*((volatile unsigned char *)0xffff8c87UL))



#ifdef __PUREC__

static unsigned short spl7_0(unsigned short) 0x0700;
static unsigned short spl7_1(unsigned short) 0x007c;
static unsigned short spl7_2(void) 0x40c0;

#define spl7() spl7_0(spl7_1(spl7_2()))

static void spl(unsigned short sr) 0x46c0;

#endif

#ifdef __GNUC__

static unsigned short spl7(void)
{
	register unsigned short sr;
	
	__asm__ volatile(
		" move.w %%sr,%0\n"
		" ori.w #0x700,%%sr\n"
	: "=d"(sr));
	return sr;
}

static void spl(unsigned short sr)
{
	__asm__ volatile(
		" move.w %0,%%sr\n"
	:
	: "d"(sr)
	: "cc");
}

#endif


static IOREC *iorec;

/*
 * DTR-OFF-Routinen
 */
static long low_it2(void)
{
	unsigned short sr;
	unsigned char wr5_shadow;
	unsigned char *dev;
	
	sr = spl7();
	dev = (unsigned char *)iorec;
	wr5_shadow = dev[29];
	wr5_shadow &= 0x7f;
	dev[29] = wr5_shadow;
	sccctl_b = 5;
	sccctl_b = wr5_shadow;
	spl(sr);
	return 0;
}


static long low_it4(void)
{
	unsigned short sr;
	unsigned char wr5_shadow;
	unsigned char *dev;
	
	sr = spl7();
	dev = (unsigned char *)iorec;
	wr5_shadow = dev[29];
	wr5_shadow &= 0x7f;
	dev[29] = wr5_shadow;
	sccctl_a = 5;
	sccctl_a = wr5_shadow;
	spl(sr);
	return 0;
}


void low_dtr(WORD func, MAPTAB *map)
{
	switch (func)
	{
	case PORT_MFP_1:
		Ongibit(0x10);
		break;
	case PORT_SCC_B:
		iorec = map->iorec;
		Supexec(low_it2);
		break;
	case PORT_MFP_2:
		/* nothing */
		break;
	case PORT_SCC_A:
		iorec = map->iorec;
		Supexec(low_it4);
		break;
	}
}


/*
 * DTR-ON-Routinen
 */
 
static long high_it2(void)
{
	unsigned short sr;
	unsigned char wr5_shadow;
	unsigned char *dev;
	
	sr = spl7();
	dev = (unsigned char *)iorec;
	wr5_shadow = dev[29];
	wr5_shadow |= 0x82;			/* DTR and RTS on */
	dev[29] = wr5_shadow;
	sccctl_b = 5;
	sccctl_b = wr5_shadow;
	spl(sr);
	return 0;
}


static long high_it4(void)
{
	unsigned short sr;
	unsigned char wr5_shadow;
	unsigned char *dev;
	
	sr = spl7();
	dev = (unsigned char *)iorec;
	wr5_shadow = dev[29];
	wr5_shadow |= 0x82;			/* DTR and RTS on */
	dev[29] = wr5_shadow;
	sccctl_a = 5;
	sccctl_a = wr5_shadow;
	spl(sr);
	return 0;
}


void high_dtr(WORD func, MAPTAB *map)
{
	switch (func)
	{
	case PORT_MFP_1:
		Offgibit(~0x18); /* set DTR and also RTS */
		break;
	case PORT_SCC_B:
		iorec = map->iorec;
		Supexec(high_it2);
		break;
	case PORT_MFP_2:
		/* nothing */
		break;
	case PORT_SCC_A:
		iorec = map->iorec;
		Supexec(high_it4);
		break;
	}
}

/*
 * DCD-Routinen
 */

static long dcd1(void)
{
	return (~mfp1 & 0x02) != 0;
}


static long dcd2(void)
{
	return (sccctl_b & 0x08) != 0;
}


static long dcd3(void)
{
	return 1;
}


static long dcd4(void)
{
	return (sccctl_a & 0x08) != 0;
}

static long (*const dcd_funcs[4])(void) = { dcd1, dcd2, dcd3, dcd4 };


BOOLEAN is_dcd(WORD func)
{
	switch (func)
	{
	case PORT_MFP_1:
		return (BOOLEAN)Supexec(dcd1);
	case PORT_SCC_B:
		return (BOOLEAN)Supexec(dcd2);
	case PORT_MFP_2:
		return TRUE; /* assume always on */
	case PORT_SCC_A:
		return (BOOLEAN)Supexec(dcd4);
	}
	return FALSE;
}


/*
 * SetMapM1
 * ->	a0: Adresse fuer Zeiger auf MAPTAB-Struktur
 * <-	nichts
 *
 * Die Funktion setzt die Adressen der I/O-Betriebssystemroutinen
 * fuer Modem 1 in die lokale Tabelle.
 *
 * Achtung: Nur aufrufen, wenn es _kein_ Bconmap() gibt!!!
 */

static MAPTAB map_m1;


static long set_m1(void)
{
	map_m1.Bconstat = (short __CDECL (*)(short))(*((long *)0x522));
	map_m1.Bconin = (long __CDECL (*)(short))(*((long *)0x542));
	map_m1.Bcostat = (long __CDECL (*)(short))(*((long *)0x562));
	map_m1.Bconout = (void __CDECL (*)(short, short))(*((long *)0x582));
	return 0;
}


void SetMapM1(MAPTAB **map)
{
	*map = &map_m1;
	Supexec(set_m1);
	map_m1.iorec = Iorec(0);
}


/*
 * SetMapMidi
 * ->	a0: Adresse fuer Zeiger auf MAPTAB-Struktur
 * <-	nichts
 *
 * Die Funktion setzt die Adressen der I/O-Betriebssystemroutinen
 * fuer die Midi-Schnittstelle in die lokale Tabelle.
 */

static MAPTAB map_midi;

static long set_midi(void)
{
	map_midi.Bconstat = (short __CDECL (*)(short))(*((long *)0x52a));
	map_midi.Bconin = (long __CDECL (*)(short))(*((long *)0x54a));
	map_midi.Bcostat = (long __CDECL (*)(short))(*((long *)0x56a));
	map_midi.Bconout = (void __CDECL (*)(short, short))(*((long *)0x58a));
	return 0;
}


void SetMapMidi(MAPTAB **map)
{
	*map = &map_midi;
	Supexec(set_midi);
	map_midi.iorec = Iorec(2);
}



/*
 * SetIorec
 * ->	a0: Zeiger auf IOREC-Struktur
 *		a1: Zeiger auf Speicherbereich
 *		d0: Laenge des Speicherblocks
 * <-	nichts
 *
 * Die Funktion setzt die Adresse des Speicherbereichs in die
 * IOREC-Struktur.
 */
static BYTE *blk;
static UWORD len;

static long sets(void)
{
	IOREC *dev;
	unsigned short sr;
	unsigned short lo;
	
	sr = spl7();
	dev = iorec;
	dev->ibuf = blk;				/* set new block */
	dev->ibufsiz = len;			/* set new length */
	dev->ibufhd = dev->ibuftl = 0;	/* clear read-/write positions */
	lo = len >> 2;					/* set low water mark */
	dev->ibuflow = lo;
	dev->ibufhi = lo + lo + lo;		/* set hi water mark */
	spl(sr);
	return 0;
}

void SetIorec(IOREC *dev, BYTE *ptr, WORD length)
{
	iorec = dev;
	blk = ptr;
	len = length;
	Supexec(sets);
}


/*
 * Funktionsname:	SendBlock
 * ->	a0:	Pointer auf DEVICES-Struktur
 *		a1:	Adresse des zu sendenden Blocks
 *		d0:	Groesse des Blocks
 *		d1:	TRUE, wenn Carrier geprueft werden soll, sonst FALSE
 * <-	TRUE wenn alles ok, sonst FALSE;
 *
 * Die Funktion sendet den Block ueber die serielle Schnittstelle.
 */
static DEVICES *_dev;
static const void *_block;
static long _len;
static BOOLEAN _tst_dcd;

#if defined(__PUREC__)
static void *push_a2(void *, short) 0x2f0a;
static void *push_a2_2(void *, short, short) 0x2f0a;
static long pop_a2(long) 0x245f;
static long addq2_a7(long) 0x544f;
static long addq4_a7(long) 0x584f;
static void *push_d0(void *) 0x3f00;
static void *push_d1(void *) 0x3f01;
static long jsr_a0(void *fun) 0x4e90;
#define callout(func, dev) pop_a2(addq2_a7(jsr_a0(push_d0(push_a2(func, dev)))))
#define callout2(func, dev, ch) pop_a2(addq4_a7(jsr_a0(push_d0(push_d1(push_a2_2(func, dev, ch))))))
#endif


static long do_sendblock(void)
{
	DEVICES *dev = _dev;
	const unsigned char *block = (const unsigned char *)_block;
	long len = _len;
	BOOLEAN tst_dcd = _tst_dcd;
	long (*dcd_func)(void) = dcd_funcs[dev->func_num - 1];
	MAPTAB *map = dev->func_map;
	
	for (;;)
	{
		if (tst_dcd)
		{
			if (dcd_func() == FALSE)
				return FALSE;
		}
		if (--len < 0)
			return TRUE;

#ifdef __GNUC__
		{
			register void *callout __asm__("a0");
			register short ret __asm__("d0");
			
			for (;;)
			{
				/* Kann Zeichen gesendet werden? */
				callout = map->Bcostat;
				__asm__ volatile(
					" move%.w %[dev],-(%%sp)\n"
					" jsr (%[callout])\n"
					" addq.w #2,%%sp\n"
				: "=d"(ret) /* outputs */
				: [dev]"g"(dev->bios), [callout]"a"(callout) /* inputs  */
				: "d1", "d2", "a1", "a2", "cc" AND_MEMORY); /* clobbered regs */
	
				if (ret == 0)
					break;
				/* Warteroutine gesetzt? */
				if (pause_2 != 0)
					pause_2();
			}
			/* send character */
			callout = map->Bconout;
			__asm__ volatile(
				" move%.w %[ch],-(%%sp)\n"
				" move%.w %[dev],-(%%sp)\n"
				" jsr (%[callout])\n"
				" addq.w #4,%%sp\n"
			: "=d"(ret) /* outputs */
			: [dev]"g"(dev->bios), [ch]"g"(*block), [callout]"a"(callout) /* inputs  */
			: "d1", "d2", "a1", "a2", "cc" AND_MEMORY); /* clobbered regs */
		}
#endif

#ifdef __PUREC__
		for (;;)
		{
			if ((short)callout(map->Bcostat, dev->bios))
				break;
			/* Warteroutine gesetzt? */
			if (pause_2 != 0)
				pause_2();
			callout2(map->Bconout, dev->bios, *block);
		}
#endif

		block++;
	}
}


BOOLEAN SendBlock(DEVICES *dev, const void *block, long len, BOOLEAN tst_dcd)
{
	_dev = dev;
	_block = block;
	_len = len;
	_tst_dcd = tst_dcd;
	return (BOOLEAN)Supexec(do_sendblock);
}


/*
 * Funktionsname:	GetBlock
 * ->	a0:	Pointer auf DEVICES-Struktur
 *		a1:	Adresse Buffers
 *		d0:	Groesse des Buffers in Bytes
 * <-	Anzahl der gelesenen Bytes;
 *
 * Die Funktion holt die an der serielle Schnittstelle vorhandenen
 * Bytes in einen Buffer.
 */
static void *_buff;

static long do_getblock(void)
{
	DEVICES *dev = _dev;
	long len = _len;
	unsigned char *buff = (unsigned char *)_buff;
	long result = 0;
	MAPTAB *map = dev->func_map;
	
	while (result < len)
	{
#ifdef __GNUC__
		{
			register void *callout __asm__("a0");
			register short ret __asm__("d0");

			/* Zeichen vorhanden? */

			callout = map->Bconstat;
			__asm__ volatile(
				" move%.w %[dev],-(%%sp)\n"
				" jsr (%[callout])\n"
				" addq.w #2,%%sp\n"
			: "=d"(ret) /* outputs */
			: [dev]"g"(dev->bios), [callout]"a"(callout) /* inputs  */
			: "d1", "d2", "a1", "a2", "cc" AND_MEMORY); /* clobbered regs */
			if (ret == 0)
				break;

			callout = map->Bconin;
			__asm__ volatile(
				" move%.w %[dev],-(%%sp)\n"
				" jsr (%[callout])\n"
				" addq.w #2,%%sp\n"
			: "=d"(ret) /* outputs */
			: [dev]"g"(dev->bios), [callout]"a"(callout) /* inputs  */
			: "d1", "d2", "a1", "a2", "cc" AND_MEMORY); /* clobbered regs */

			*buff++ = ret;
		}		
#endif

#ifdef __PUREC__
		if ((short)callout(map->Bconstat, dev->bios) == 0)
			break;

		*buff++ = (unsigned char)callout(map->Bconin, dev->bios);
#endif

		result++;
	}
	
	return result;
}


LONG GetBlock(DEVICES *dev, long bufflen, void *buff)
{
	_dev = dev;
	_len = bufflen;
	_buff = buff;
	return Supexec(do_getblock);
}
