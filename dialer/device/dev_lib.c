/*
 * Project    :		Device Library Version 0.20beta
 * Module     :		dev_lib.c
 * Author     :		Jan Kriesten
 * Date       :     07.05.1995
 *
 * Description:		main routines to install/deinstall devices
 *
 * Tabsize 4
 */

/*-----------------------------*/
/*--- includes              ---*/
/*-----------------------------*/

#include "device.h"
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include "types.h"
#include "port.h"

/*-----------------------------*/
/*--- defines               ---*/
/*-----------------------------*/

#define		BMAPSTART		5

#define		MAX_BLOCK		4096L
#define		MAX_SPEEDS		25

/*-----------------------------*/
/*--- variables             ---*/
/*-----------------------------*/

static UWORD atari;
static UWORD tos;
static UWORD MiNT;
static void *MagX;

static FSER_INFO *fser;
static RSVF_DEV *rsvf;

static BOOLEAN has_bconmap;
static BOOLEAN has_drv_u;

static const char *const d_name[] = {
	"Modem 1",
	"Modem 2",
	"Serial 1",
	"Serial 2",
	"Midi"
};
static const char *const d_open[] = {

	"MODEM1",
	"MODEM2",
	"SERIAL1",
	"SERIAL2",
};

static LONG const speeds[] = {
	19200L,
	9600L,
	4800L,
	3600L,
	2400L,
	2000L,
	1800L,
	1200L,
	600L,
	300L,
	200L,
	150L,
	134L,
	110L,
	75L,
	50L,
	-1L
};

static LONG const midi_speeds[] = { 31250, -1 };

static DEVICES *devices;

static void (*pause_1)(void);
void (*pause_2)(void);

/*-----------------------------*/
/*--- local functions       ---*/
/*-----------------------------*/

/*-------------------------------------------------------------------*/

static BOOLEAN InitStdDevices(MAPTAB *maps, BOOLEAN has_bconmap, WORD num_devices)
{
	DEVICES *dptr;
	DEVICES *dwalk = NULL;
	WORD loop;
	WORD i;

	if (!has_bconmap || !num_devices)
		num_devices = 1;

	loop = num_devices > 4 ? 4 : num_devices;

	for (i = 0; i < loop; i++)
	{
		if ((dptr = (DEVICES *) calloc(1, sizeof(DEVICES))) == NULL)
			return FALSE;

		if (dwalk)
			dwalk->device.next = &dptr->device;
		else
			devices = dptr;

		dwalk = dptr;

		switch (i)
		{
		case 0:
			if (atari != 3 || num_devices != 1)
			{
				dwalk->bios = has_bconmap ? 6 : 1;
				dwalk->func_num = PORT_MFP_1 + 1;
				dwalk->device.name = d_name[0];
				if (has_drv_u)
					dwalk->dopen = d_open[0];
				break;
			}
			/* fall through */
		case 1:
			dwalk->bios = 7;
			dwalk->func_num = PORT_SCC_B + 1;
			dwalk->device.name = d_name[1];
			if (has_drv_u)
				dwalk->dopen = d_open[1];
			break;
		case 2:
			dwalk->bios = 8;
			if (atari == 2)
			{
				dwalk->func_num = PORT_MFP_2 + 1;
				dwalk->device.name = d_name[2];
				if (has_drv_u)
					dwalk->dopen = d_open[2];
			} else
			{
				dwalk->func_num = PORT_SCC_A + 1;
				dwalk->device.name = d_name[3];
				if (has_drv_u)
					dwalk->dopen = d_open[3];
			}
			break;
		case 3:
			dwalk->bios = 9;
			dwalk->func_num = PORT_SCC_A + 1;
			dwalk->device.name = d_name[3];
			if (has_drv_u)
				dwalk->dopen = d_open[3];
			break;
		}

		dwalk->dhandle = -1;
		dwalk->num_read = -1;

		if (fser)
			dwalk->type = DEV_FSER;
		else if (MagX)
			dwalk->type = DEV_MAGIC;
		else if (MiNT)
			dwalk->type = DEV_MINT;
		else
			dwalk->type = DEV_STANDARD;

		if (has_bconmap)
			dwalk->func_map = &maps[dwalk->bios - 6];
		else if (i == 0)
			SetMapM1(&dwalk->func_map);
	}

	if ((dptr = (DEVICES *) calloc(1, sizeof(DEVICES))) == NULL)
		return FALSE;

	dwalk->device.next = &dptr->device;

	dptr->bios = 3;
	dptr->device.name = d_name[4];
	dptr->dhandle = -1;
	dptr->num_read = -1;
	dptr->type = MagX ? DEV_MAGIC : DEV_STANDARD;

	SetMapMidi(&dptr->func_map);

	return TRUE;
}

/*-------------------------------------------------------------------*/

static BOOLEAN InitRSVFDevices(MAPTAB *maps, RSVF_DEV *dev)
{
	DEVICES *dwalk;

	while (dev->ptr)
	{
		if (!(dev->typ.device))
		{
			dev = dev->ptr;
			continue;
		}

		dwalk = devices;

		while (dwalk)
		{
			if ((dwalk->bios == dev->bios_nr && strcmp(dev->ptr, "LAN")) ||
				(!strcmp(dev->ptr, "MIDI") && dwalk->bios == 3))
			{
				if (dwalk->bios == 3)
					dwalk->bios = dev->typ.bios ? dev->bios_nr : 3;
				break;
			}

			dwalk = (DEVICES *) dwalk->device.next;
		}

		if (!dwalk)
		{
			DEVICES *dptr;

			if ((dptr = (DEVICES *) calloc(1, sizeof(DEVICES))) == NULL)
				return FALSE;

			dwalk = devices;
			while (dwalk->device.next)
				dwalk = (DEVICES *) dwalk->device.next;

			dwalk->device.next = &dptr->device;
			dwalk = dptr;

			dwalk->device.name = (BYTE *) dev->ptr;
			dwalk->bios = dev->typ.bios ? dev->bios_nr : -1;

			dwalk->dhandle = -1;
			dwalk->num_read = -1;

			dwalk->func_map = dwalk->bios > 5 ? &maps[dwalk->bios - 6] : NULL;
		}

		dwalk->dopen = (BYTE *) dev->ptr;
		dwalk->type = DEV_RSVF;

		dev++;
	}

	return TRUE;
}

/*-------------------------------------------------------------------*/

static BOOLEAN GetFcntlSpeeds(DEVICES *dev)
{
	LONG *dte_ptr;
	LONG last_dte;
	LONG dte;

	/*
	 * Aktuelle DTE-Speed retten:
	 */
	dev->device.curr_dte = -1;
	if (Fcntl(dev->dhandle, (long) &dev->device.curr_dte, TIOCIBAUD) < 0)
		return FALSE;

	/*
	 * Von hohen zu niedrigen Baudraten wandern, solange es
	 * noch weitere gibt:
	 */
	dte_ptr = dev->speeds;
	last_dte = dte = 0x7fffffffL;

	while (TRUE)
	{
		Fcntl(dev->dhandle, (long) &dte, TIOCIBAUD);

		if (dte >= last_dte)
		{
			*dte_ptr = -1;
			break;
		} else
		{
			last_dte = *dte_ptr++ = dte;
		}
		
		dte--;
	}

	/*
	 * DTE-Speed wiederherstellen:
	 */
	Fcntl(dev->dhandle, (long) &dev->device.curr_dte, TIOCIBAUD);

	return TRUE;
}

/*-------------------------------------------------------------------*/

static void GetFserSpeeds(DEVICES *dev)
{
	BAUD_INFO *walk;
	BAUD_INFO *table;
	WORD i;
	WORD nums;
	LONG *lptr;

	table = fser->baud_table_flag ? dev->chan_info->alt_baud_table : dev->chan_info->baud_table;
	nums = 0;

	i = (WORD) Rsconf(-2, -1, -1, -1, -1, -1);
	walk = table;

	while (i)
	{
		walk++;
		i--;
	}
	dev->device.curr_dte = walk->baudrate;

	walk = table;

	while (walk->baudrate)
	{
		if (walk->baudrate > 0)
		{
			lptr = dev->speeds;

			for (i = 0; i < nums; i++)
			{
				if (walk->baudrate >= *lptr)
					break;
				lptr++;
			}

			if (i < nums && !(walk->baudrate == *lptr))
			{
				memcpy(lptr + 1, lptr, (nums - i) * sizeof(LONG));
			} else if (i != nums)
			{
				walk++;
				continue;
			}

			*lptr = walk->baudrate;
			nums++;
		}
		walk++;
	}

	dev->speeds[nums] = -1;

	return;
}

/*-------------------------------------------------------------------*/

static BOOLEAN CreateSpeedlist(DEVICES *dev)
{
	if ((dev->speeds = (LONG *) calloc(MAX_SPEEDS, sizeof(LONG *))) == NULL)
		return FALSE;

	if (dev->dhandle >= 0 && GetFcntlSpeeds(dev))
	{
		return TRUE;
	} else if (dev->bios == 3)			/* Midi */
	{
		const LONG *a_ptr;
		LONG *b_ptr;

		a_ptr = midi_speeds;
		b_ptr = dev->speeds;

		while (*a_ptr > 0)
			*b_ptr++ = *a_ptr++;

		*b_ptr = *a_ptr;

		dev->device.curr_dte = 31250;
	} else if (fser)
	{
		GetFserSpeeds(dev);
	} else
	{
		WORD i;
		const LONG *a_ptr;
		LONG *b_ptr;

		if (tos <= 0x104)
			i = *((WORD *) ((BYTE *) dev->func_map->iorec + 34));
		else
			i = (WORD) Rsconf(-2, -1, -1, -1, -1, -1);

		a_ptr = speeds;

		while (i)
		{
			i--;
			a_ptr++;
		}
		dev->device.curr_dte = *a_ptr;

		a_ptr = speeds;
		b_ptr = dev->speeds;

		while (*a_ptr > 0)
			*b_ptr++ = *a_ptr++;

		*b_ptr = *a_ptr;
	}

	return TRUE;
}

/*-------------------------------------------------------------------*/

static void FreeSpeedlist(DEVICES *dev)
{
	if (dev->speeds)
	{
		free(dev->speeds);
		dev->speeds = NULL;
	}
}

/*-------------------------------------------------------------------*/

static void SetDevFunctions(DEVICES *dev)
{
	if (dev->dhandle >= 0)
		Fcntl(dev->dhandle, (long) dev->ioctrlmap, TIOCCTLMAP);
}

/*-------------------------------------------------------------------*/

static void SetPortProtokoll(DEVICES *dev)
{
	UWORD flow = _RTSCTS;

	if (dev->dhandle >= 0 && !strcmp(dev->dopen, "MIDI"))
		flow = _XONXOFF;

	PortParameter(&dev->device, flow, _8BIT, _1STOP, _NO_PARITY);

#if 0
	/*
	 * Unter MiNT muž noch der Terminal-Typ "raw" eingestellt
	 * werden (grrrrr!):
	 */
	if (dev->dhandle >= 0 && !rsvf && MiNT)
	{
		SGTTYB tty;

		if (!Fcntl(dev->dhandle, (long) &tty, TIOCGETP))
		{
			tty.sg_flags = T_RAW;
			Fcntl(dev->dhandle, (long) &tty, TIOCSETP);
			Fcntl(dev->dhandle, (long) &tty, TIOCGETP);
		}
	}
#endif
}

/*-------------------------------------------------------------------*/

static BOOLEAN DevicePickup(DEVICES *dev)
{
	LONG count;

	dev->curr_pos = 0;
	dev->num_read = -1;

	if (dev->dhandle >= 0)
	{
		if ((count = Fread(dev->dhandle, MAX_BLOCK, dev->buf)) <= 0)
			return FALSE;
	} else
	{
		if ((count = GetBlock(dev, MAX_BLOCK, dev->buf)) <= 0)
			return FALSE;
	}

	dev->num_read = (WORD) count;

	return TRUE;
}

/*-------------------------------------------------------------------*/

static BOOLEAN DeviceSendBlock(DEVICES *dev, const void *block, LONG len, BOOLEAN tst_dcd)
{
	LONG sent = 0;
	LONG help;
	ULONG timeout = 0;

	while (sent < len)
	{
		if (tst_dcd && !IsCarrier(&dev->device))
			return FALSE;

		if ((help = Fwrite(dev->dhandle, len - sent, (const char *)block + sent)) < 0L)
			return FALSE;

		sent += help;

		if (sent < len)
		{
			if (!timeout)
				timeout = Calc200Hz(500L);
			else if (timeout < Get200Hz())
				break;

			if (pause_1)
				pause_1();
		}
	}

	return TRUE;
}

/*-----------------------------*/
/*--- global functions      ---*/
/*-----------------------------*/

/*-------------------------------------------------------------------*/

DEV_LIST *InitDevices(void *timerelease1, void *timerelease2)
{
	/*
	 * Initialisiert alle vorhandenen Ports/Devices.
	 */
	WORD num_devices = 0;
	BOOLEAN mega;
	MAPTAB *maps;
	long cookval = 0;

	devices = NULL;
	tos = get_tos();
	has_drv_u = has_drive_u();
	has_bconmap = (Bconmap(0) == 0L);

	pause_1 = timerelease1;
	pause_2 = timerelease2;

	atari = getcookie(0x5f4d4348L /* '_MCH' */, &cookval) ? (UWORD) (cookval >> 16) : 0;
	mega = atari == 1 && (UWORD) cookval != 0;
	fser = getcookie(0x46534552L /* 'FSER' */, &cookval) ? (FSER_INFO *) cookval : NULL;
	rsvf = getcookie(0x52535646L /* 'RSVF' */, &cookval) ? (RSVF_DEV *) cookval : NULL;
	MiNT = getcookie(0x4d694e54L /* 'MiNT' */, &cookval) ? (WORD) cookval : 0;
	MagX = getcookie(0x4d616758L /* 'MagX' */, &cookval) ? (void *) cookval : NULL;

	/*
	 * Set MiNT process execution domain (to let Fread, Fwrite
	 * behave as stated in the terminal settings):
	 */
	if (MiNT && !rsvf && !fser)
		Pdomain(1);

	if (has_bconmap)
	{
		BCONMAP *bmap = (BCONMAP *) Bconmap(-2);

		maps = bmap->maptab;
		num_devices = bmap->maptabsize;

		if (!maps)
			has_bconmap = FALSE;
	} else
	{
		maps = NULL;
	}
	
	switch (atari)
	{
	case 0: /* plain ST */
	case 3: /* Falcon */
	case 5: /* Aranym */
		num_devices = 1;
		break;
	case 1: /* STE/MSTE */
		num_devices = mega ? 3 : 1;
		break;
	case 2: /* TT */
		num_devices = 4;
		break;
	}

	if (!InitStdDevices(maps, has_bconmap, num_devices))
		return NULL;

	if (rsvf && !InitRSVFDevices(maps, rsvf))
		return NULL;

	return devices ? &devices->device : NULL;
}

/*-------------------------------------------------------------------*/

void TermDevices(void)
{
	/*
	 * Gibt den Speicher der Device-Liste wieder frei:
	 */

	DEVICES *dwalk;

	while ((dwalk = devices) != NULL)
	{
		CloseDevice(&dwalk->device);

		devices = (DEVICES *) dwalk->device.next;
		free(dwalk);
	}
}

/*-------------------------------------------------------------------*/

BOOLEAN OpenDevice(DEV_LIST *port)
{
	/*
	 * ™ffnet port und stellt alle entsprechenden Funktionen zusammen.
	 */
	DEVICES *dev;

	dev = (DEVICES *) port;

	if (dev->is_open)
		return FALSE;

	/*
	 * Erst den Standard-Aux umsetzen:
	 */
	if (has_bconmap && dev->bios > 5)
		Bconmap(dev->bios);

	/*
	 * Bei FastSerial keine Blockdevice-Routinen benutzen:
	 */
	if (fser && (dev->bios == 1 || dev->bios > 5))
	{
		CHAN_INFO *tst;

		tst = (CHAN_INFO *) Rsconf(-3, -3, -1, -1, -1, -1);

		if (tst->task > 0)
			return FALSE;

		dev->chan_info = tst;
		dev->is_open = TRUE;
	}

	/*
	 * Testen, ob es sich um ein Block-Device handelt:
	 */
	if (!dev->is_open && dev->dopen && (dev->type == DEV_RSVF || dev->type == DEV_MINT))
	{
		BYTE path[64];
		LONG rc;

		strcpy(path, "U:\\DEV\\");
		strcpy(path + 7, dev->dopen);

		if ((rc = Fopen(path, FO_RW | O_NDELAY)) >= 0)
		{
			dev->dhandle = (WORD) rc;
			dev->is_open = TRUE;
		}
		/*
		 * Falls -36 (EACCDN) zurckgegeben wird, so wurde das Device
		 * schon von einer anderen Applikation ge”ffnet.
		 */
		else if (rc == -36)
		{
			return FALSE;
		}
	}

	if ((dev->buf = (BYTE *) malloc(MAX_BLOCK * sizeof(BYTE))) == NULL)
	{
		CloseDevice(port);
		return FALSE;
	}

	dev->is_open = TRUE;

	SetDevFunctions(dev);
	SetPortProtokoll(dev);

	if (!CreateSpeedlist(dev))
		CloseDevice(port);
	else
		StartReceiver(port);

	return dev->is_open;
}

/*-------------------------------------------------------------------*/

void CloseDevice(DEV_LIST *port)
{
	/*
	 * Schliežt port und gibt den Speicher wieder frei
	 */
	DEVICES *dev;

	dev = (DEVICES *) port;

	if (!dev->is_open)
		return;

	if (dev->dhandle >= 0)
	{
		Fclose(dev->dhandle);
		dev->dhandle = -1;
	}

	if (dev->buf)
	{
		free(dev->buf);
		dev->buf = NULL;
	}

	if (dev->oldIBufPtr)
	{
		IOREC *iorec;
		BYTE *b;

		iorec = dev->func_map->iorec;
		b = iorec->ibuf;

		SetIorec(iorec, dev->oldIBufPtr, dev->oldIBufSize);

		MiNT ? Mfree(b) : free(b);
		dev->oldIBufPtr = NULL;
	}

	if (dev->oldOBufPtr)
	{
		IOREC *iorec;
		BYTE *b;

		iorec = dev->func_map->iorec;
		++iorec;
		b = iorec->ibuf;

		SetIorec(iorec, dev->oldOBufPtr, dev->oldOBufSize);

		MiNT ? Mfree(b) : free(b);
		dev->oldOBufPtr = NULL;
	}

	FreeSpeedlist(dev);

	dev->is_open = FALSE;

	return;
}

/*-------------------------------------------------------------------*/

WORD GetBiosNr(DEV_LIST *dev)
{
	return ((DEVICES *) dev)->bios;
}

/*-------------------------------------------------------------------*/

LONG *GetSpeedList(DEV_LIST *dev)
{
	return ((DEVICES *) dev)->speeds;
}

/*-------------------------------------------------------------------*/

LONG SetDTESpeed(DEV_LIST *port, LONG speed)
{
	DEVICES *dev;
	long val;

	dev = (DEVICES *) port;

	/*
	 * Erst den Standard-Aux umsetzen:
	 */
	if (has_bconmap && dev->bios > 5)
		Bconmap(dev->bios);

	if (dev->dhandle >= 0)
	{
		val = speed;

		if (!Fcntl(dev->dhandle, (long) &val, TIOCIBAUD))
			dev->device.curr_dte = speed;
	} else if (dev->bios == 3)
	{
		return dev->device.curr_dte;
	} else if (fser)
	{
		WORD i = 0;
		BAUD_INFO *help;

		help = fser->baud_table_flag ? dev->chan_info->alt_baud_table : dev->chan_info->baud_table;

		while (help->baudrate)
		{
			if (help->baudrate == speed)
				break;
			i++, help++;
		}

		if (help->baudrate > 0)
		{
			Rsconf(i, -1, -1, -1, -1, -1);
			dev->device.curr_dte = speed;
		}
	} else
	{
		WORD i = 0;
		LONG *help = dev->speeds;

		while (*help > 0)
		{
			if (*help == speed)
				break;
			i++, help++;
		}

		if (*help > 0)
		{
			Rsconf(i, -1, -1, -1, -1, -1);
			dev->device.curr_dte = speed;
		}
	}

	return dev->device.curr_dte;
}

/*-------------------------------------------------------------------*/

void StartReceiver(DEV_LIST *port)
{
	DEVICES *dev;

	dev = (DEVICES *) port;

	if (dev->dhandle >= 0)
		Fcntl(dev->dhandle, (long) 0, TIOCSTART);
}

/*-------------------------------------------------------------------*/

void StopReceiver(DEV_LIST *port)
{
	DEVICES *dev;

	dev = (DEVICES *) port;

	if (dev->dhandle >= 0)
		Fcntl(dev->dhandle, (long) 0, TIOCSTOP);
}

/*-------------------------------------------------------------------*/

WORD SetTxBuffer(DEV_LIST *port, WORD size)
{
	DEVICES *dev;
	IOREC *iorec;
	BYTE *mem;
	BYTE *b = NULL;
	long cntrls[4];

	dev = (DEVICES *) port;

	if (dev->dhandle >= 0)
	{
		cntrls[0] = cntrls[1] = cntrls[2] = -1;
		cntrls[3] = size;

		if (!Fcntl(dev->dhandle, (long) cntrls, TIOCBUFFER))
			return (WORD) cntrls[3];
	}

	if ((mem = (BYTE *) (MiNT ? Mxalloc(size, 0x23) : malloc(size))) == NULL)
		return -1;

	iorec = dev->func_map->iorec;
	++iorec;

	if (!dev->oldOBufPtr)
	{
		dev->oldOBufSize = iorec->ibufsiz;
		dev->oldOBufPtr = iorec->ibuf;
	} else
	{
		b = iorec->ibuf;
	}
	
	SetIorec(iorec, mem, size);

	if (b)
	{
		MiNT ? Mfree(b) : free(b);
	}

	return size;
}

/*-------------------------------------------------------------------*/

WORD GetTxBuffer(DEV_LIST *port)
{
	DEVICES *dev;
	long cntrls[4];

	dev = (DEVICES *) port;

	if (dev->dhandle >= 0)
	{
		cntrls[0] = cntrls[1] = cntrls[2] = cntrls[3] = -1;

		if (!Fcntl(dev->dhandle, (long) cntrls, TIOCBUFFER))
			return (WORD) cntrls[3];
	}

	return -1;
}

/*-------------------------------------------------------------------*/

WORD SetRxBuffer(DEV_LIST *port, WORD size)
{
	DEVICES *dev;
	IOREC *iorec;
	BYTE *mem;
	BYTE *b = NULL;
	long cntrls[4];

	dev = (DEVICES *) port;

	if (dev->dhandle >= 0)
	{
		cntrls[0] = size;
		cntrls[1] = size >> 2;
		cntrls[2] = (size + size + size) >> 2;
		cntrls[3] = -1;

		if (!Fcntl(dev->dhandle, (long) cntrls, TIOCBUFFER))
			return (WORD) cntrls[0];
	}

	if ((mem = (BYTE *) (MiNT ? Mxalloc(size, 0x23) : malloc(size))) == NULL)
		return -1;

	iorec = dev->func_map->iorec;

	if (!dev->oldIBufPtr)
	{
		dev->oldIBufSize = iorec->ibufsiz;
		dev->oldIBufPtr = iorec->ibuf;
	} else
	{
		b = iorec->ibuf;
	}
	
	SetIorec(iorec, mem, size);

	if (b)
	{
		MiNT ? Mfree(b) : free(b);
	}

	return size;
}

/*-------------------------------------------------------------------*/

WORD GetRxBuffer(DEV_LIST *port)
{
	DEVICES *dev;
	long cntrls[4];

	dev = (DEVICES *) port;

	if (dev->dhandle >= 0)
	{
		cntrls[0] = cntrls[1] = cntrls[2] = cntrls[3] = -1;

		if (!Fcntl(dev->dhandle, (long) cntrls, TIOCBUFFER))
			return (WORD) cntrls[0];
	}

	return -1;
}

/*-------------------------------------------------------------------*/

BOOLEAN PortSendByte(DEV_LIST *port, BYTE c)
{
	DEVICES *dev;

	dev = (DEVICES *) port;

	if (dev->dhandle >= 0)
	{
		return DeviceSendBlock(dev, &c, 1L, FALSE);
	} else if (dev->bios >= 0)
	{
		while (!Bcostat(dev->bios == 3 ? 4 : dev->bios))
		{
			if (pause_1)
				pause_1();
		}

		Bconout(dev->bios, c);

		return TRUE;
	}

	return FALSE;
}

/*-------------------------------------------------------------------*/

BOOLEAN PortSendBlock(DEV_LIST *port, const void *block, LONG len, BOOLEAN tst_dcd)
{
	DEVICES *dev;

	dev = (DEVICES *) port;

	if (dev->dhandle >= 0)
	{
		return DeviceSendBlock(dev, block, len, tst_dcd);
	} else if (dev->bios == 3)
	{
		Midiws((WORD) len - 1, block);
		return TRUE;
	} else if (dev->bios >= 0)
	{
		return SendBlock(dev, block, len, tst_dcd);
	}
	
	return FALSE;
}

/*-------------------------------------------------------------------*/

WORD PortGetByte(DEV_LIST *port)
{
	DEVICES *dev;

	dev = (DEVICES *) port;

	if (dev->dhandle >= 0 || dev->bios >= 0)
	{

		while (!CharAvailable(port))
		{
			if (!IsCarrier(port))
				return -1;

			if (pause_1)
				pause_1();
		}

		return dev->buf[dev->curr_pos++];
	}

	return -1;
}

/*-------------------------------------------------------------------*/

WORD PortPeekByte(DEV_LIST *port)
{
	DEVICES *dev;

	dev = (DEVICES *) port;

	if (dev->dhandle >= 0 || dev->bios >= 0)
	{
		if (CharAvailable(port))
			return dev->buf[dev->curr_pos] & 0xff;
	}

	return -1;
}

/*-------------------------------------------------------------------*/

BOOLEAN OutIsEmpty(DEV_LIST *port)
{
	DEVICES *dev;
	IOREC *iorec;
	long count = 0;

	dev = (DEVICES *) port;

	if (dev->dhandle >= 0)
	{
		if (!Fcntl(dev->dhandle, (long) &count, TIONOTSEND))
			return count == 0;
	}

	iorec = dev->func_map->iorec;
	iorec++;

	return iorec->ibufhd == iorec->ibuftl;
}

/*-------------------------------------------------------------------*/

BOOLEAN WaitOutEmpty(DEV_LIST *port, BOOLEAN tst_dcd, UWORD wait)
{
	ULONG time_to_wait;

	if (wait)
		time_to_wait = Calc200Hz((ULONG) wait);

	while (!OutIsEmpty(port))
	{
		if ((tst_dcd && !IsCarrier(port)) || (wait && time_to_wait < Get200Hz()))
		{
			ClearIOBuffer(port, IO_O_BUFFER);
			return FALSE;
		}

		if (pause_1)
			pause_1();
	}

	return TRUE;
}

/*-------------------------------------------------------------------*/

BOOLEAN CharAvailable(DEV_LIST *port)
{
	DEVICES *dev;

	dev = (DEVICES *) port;

	if (dev->dhandle >= 0 || dev->bios >= 0)
		return dev->num_read > dev->curr_pos || DevicePickup(dev);

	return FALSE;
}

/*-------------------------------------------------------------------*/

void ClearIOBuffer(DEV_LIST *port, LONG io)
{
	DEVICES *dev;
	IOREC *iorec;

	dev = (DEVICES *) port;

	if (dev->dhandle >= 0)
	{
		if (!Fcntl(dev->dhandle, io, TIOCFLUSH))
		{
			if (io != IO_O_BUFFER)
			{
				dev->curr_pos = 0;
				dev->num_read = -1;
				StartReceiver(port);
			}
			return;
		}
	}

	iorec = dev->func_map->iorec;

	if (io != IO_O_BUFFER)
	{
		/* Inbuffer l”schen: */
		iorec->ibuftl = iorec->ibufhd;
		dev->curr_pos = 0;
		dev->num_read = -1;
		StartReceiver(port);
	}

	if (io != IO_I_BUFFER)
	{
		/* Outbuffer l”schen: */
		iorec++;
		iorec->ibufhd = iorec->ibuftl;
	}
}

/*-------------------------------------------------------------------*/

void DtrOn(DEV_LIST *port)
{
	DEVICES *dev;
	long cntrls[2];

	dev = (DEVICES *) port;

	if (dev->dhandle >= 0 && (dev->ioctrlmap[0] & TIOCM_DTR))
	{
		cntrls[0] = cntrls[1] = TIOCM_DTR;

		Fcntl(dev->dhandle, (long) cntrls, TIOCCTLSET);
	} else if (dev->func_num)
	{
		high_dtr(dev->func_num - 1, dev->func_map);
	}
}

/*-------------------------------------------------------------------*/

void DtrOff(DEV_LIST *port)
{
	DEVICES *dev;
	long cntrls[2];

	dev = (DEVICES *) port;

	if (dev->dhandle >= 0 && (dev->ioctrlmap[0] & TIOCM_DTR))
	{
		cntrls[0] = TIOCM_DTR;
		cntrls[1] = 0;

		Fcntl(dev->dhandle, (long) cntrls, TIOCCTLSET);
	} else if (dev->func_num)
	{
		low_dtr(dev->func_num - 1, dev->func_map);
	}
}

/*-------------------------------------------------------------------*/

BOOLEAN IsCarrier(DEV_LIST *port)
{
	DEVICES *dev;
	long val_and_mask;

	dev = (DEVICES *) port;

	if (dev->dhandle >= 0 && (dev->ioctrlmap[0] & TIOCM_CAR))
	{
		val_and_mask = TIOCM_CAR;

		Fcntl(dev->dhandle, (long) &val_and_mask, TIOCCTLGET);

		if ((val_and_mask & TIOCM_CAR) == 0)
			return FALSE;
	} else if (dev->func_num)
	{
		return is_dcd(dev->func_num - 1) != 0;
	}

	return TRUE;
}

/*-------------------------------------------------------------------*/

void PortParameter(DEV_LIST *port, UWORD flowctl, UWORD charlen, UWORD stopbits, UWORD parity)
{
	DEVICES *dev;

	dev = (DEVICES *) port;

	if (dev->dhandle >= 0)
	{
		UWORD flags;

		if (!Fcntl(dev->dhandle, (long) &flags, TIOCGFLAGS))
		{
			flags &= ~(TF_STOPBITS | TF_CHARBITS | TF_FLAG);
			flags |= (flowctl | charlen | stopbits | parity);

			if (!Fcntl(dev->dhandle, (long) &flags, TIOCSFLAGS))
				return;
		}
	}

	if (dev->bios == 1 || dev->bios > 5)
	{
		LONG flags;
		UCR ucr;

		if (has_bconmap && dev->bios > 5)
			Bconmap(dev->bios);

		flags = Rsconf(-1, -1, -1, -1, -1, -1);

		ucr.word = *((BYTE *) & flags);
		ucr.word &= 0x0081;

		if (parity)
		{
			ucr.bits.bit2 = 1;
			if (parity == _EVENP)
				ucr.bits.bit1 = 1;
		}

		ucr.bits.bit3_4 = stopbits;
		ucr.bits.bit5_6 = charlen >> 2;

		Rsconf(-1, flowctl, ucr.word, -1, -1, -1);
	}
}
