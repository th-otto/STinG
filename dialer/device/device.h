/*
 * DEVICE.LIB - written by Jan Kriesten 
 *
 * (c) 1994, 1995 by Jan Kriesten, Friedhofstr. 25 b, 35043 Marburg, Germany
 *     EMail:    Maus GI
 *               90:400/1002@nest.ftn
 *               51:601/103@atarinet.ftn
 *               2:2435/707@fidonet.org
 *               kriesten@Mailer.Uni-Marburg.DE
 *
 * Routines for using the ports - supporting HSModem, FastSerial and
 * MiNT block device routines.
 * Including fast sending routines when no block device routines are
 * availbable.
 */

#ifndef __DEVICE_H__
#define __DEVICE_H__

#include <portab.h>

#undef BYTE
#define BYTE char

#ifndef BOOLEAN
#define BOOLEAN int                          /* 2 valued (true/false)       */
#endif

#ifndef FALSE
# define FALSE 0
# define TRUE  1
#endif

/*--- includes              ---*/

#if defined(__GNUC__)
#  include <osbind.h>
#  define MAPTAB _MAPTAB
#  define IOREC _IOREC
#else
#  include <tos.h>
#endif

/*--- defines               ---*/

#define		IO_I_BUFFER		1L		/* Inbound buffer		*/
#define		IO_O_BUFFER		2L		/* Outbound buffer		*/
#define		IO_BUFFERS		3L		/* Both buffers			*/

/*
 * Definitions for the port protocol:
 */
#define		_1STOP			0x0001
#define		_15STOP			0x0002
#define		_2STOP			0x0003

#define		_8BIT			0x0000
#define		_7BIT			0x0004
#define		_6BIT			0x0008
#define		_5BIT			0x000c

#define		_NO_HNDSHAKE	0x0000
#define		_XONXOFF		0x1000
#define		_RTSCTS			0x2000

#define		_NO_PARITY		0x0000
#define		_EVENP			0x4000
#define		_ODDP			0x8000

/*--- types                 ---*/

typedef struct _dev_list
{
	const char *name;		/* Device name			*/
	LONG				curr_dte;	/* current DTE rate		*/
	struct _dev_list	*next;		/* Pointer to next dev	*/
} DEV_LIST;

/*--- variables             ---*/

/*--- prototypes            ---*/

/*
 * InitDevices:
 *		To be called at the beginning of the program to initialize
 *		the all routines.
 *
 * Parameters:
 *		pause_normal: Routine to spent some time or NULL.
 *		pause_super : same as normal, but will be called from 
 *		              supervise modus!
 */
DEV_LIST *InitDevices(void *pause_normal, void *pause_super);

/*
 * TermDevices:
 *		Frees all memory allocated during initialization. Devicelist
 *		is no longer valid after calling this routine.
 *		Just to be called before exiting the program.
 */
void TermDevices(void);

/*
 * Open-/CloseDevice:
 *		The given port will be initialized and the speedlist will be
 *		created. Returns TRUE if everything went well and the Device
 *		could be opened. FALSE is returned if the Device is already in
 *		use or memory allocation failed ...
 */
BOOLEAN OpenDevice(DEV_LIST *port);
void CloseDevice(DEV_LIST *port);

/*
 * GetBiosNr:
 *		Returns the corresponding bios number for the device or -1
 *		if the device isn't known by the bios. Should only be used
 *		if you have to transfer the bios number as a parameter to
 *		another program.
 */
WORD GetBiosNr(DEV_LIST *port);

/*
 * PortParameter:
 *		Sets the parameters for the port. You don't need to call
 *		this routine if you wish to run 8N1 + RTS/CTS handshake.
 *
 * flowctl:		_RTSCTS (default)
 *				_XONXOFF
 *				_NO_HNDSHAKE
 * charlen:		_8BIT (default)
 *				_7BIT
 *				_6BIT
 *				_5BIT
 * stopbits:	_1STOP (default)
 *				_15STOP
 *				_2STOP
 * parity:		_NO_PARITY (default)
 *				_EVENP
 *				_ODDP
 */
void PortParameter(DEV_LIST *port, UWORD flowctl, UWORD charlen, UWORD stopbits, UWORD parity);

/*
 * GetSpeedList:
 *		Returns the speedlist for the corresponding device. The 
 *		return value is only defined for _open_ devices!
 *		The speedlist is an array of long values (from higher dte-
 *		rates to lower). The end of the list is specified by a
 *		speed of -1L.
 */
LONG *GetSpeedList(DEV_LIST *port);

/*
 * SetDTESpeed:
 *		Sets the DTE speed to `speed'. If `speed' isn't available,
 *		the current DTE is returned.
 *		In all cases the new DTE speed is returned by the function.
 *		Just call this function for open devices!
 */
LONG SetDTESpeed(DEV_LIST *port, LONG speed);

/*
 * PortSendByte:
 *		Sends character `c' to the port `port'.
 */
BOOLEAN PortSendByte(DEV_LIST *port, BYTE c);

/*
 * PortSendBlock:
 *		Sends `len' characters from `block' to device `port'. 
 *		If `tst_dcd' is TRUE, the carrier is checked during sending.
 */
BOOLEAN PortSendBlock(DEV_LIST *port, const void *block, LONG len, BOOLEAN tst_dcd);

/*
 * PortGetByte:
 *		Returns the next available character. While no character is
 *		available the function pause_1 is called.
 *		The routine will not pause, if there's no carrier available!
 */
WORD PortGetByte(DEV_LIST *port);

/*
 * PortPeekByte:
 *		Returns the next available character _without_ actually read
 *		it from the port (i.e. the character is still available from
 *		port).
 *		If no char is available, -1 is returned.
 */
WORD PortPeekByte(DEV_LIST *port);

/*
 * OutIsEmpty:
 *		Checks, if the outbound iorec buffer is empty.
 */
BOOLEAN OutIsEmpty(DEV_LIST *port);

/*
 * WaitOutEmpty:
 *		Waits, until the outbound iorec buffer is empty. If tst_dcd
 *		is set to TRUE, the carrier is checked while waiting.
 *		If wait_hundreds is != 0, WaitOutEmpty waits a maximum time
 *		of wait_hundreds hundreds seconds. If the outbound buffer is
 *		not empty  in this period, the outbound buffer is cleared.
 */
BOOLEAN WaitOutEmpty(DEV_LIST *dev, BOOLEAN tst_dcd, UWORD wait_hundreds);

/*
 * CharAvailable:
 *		Returns TRUE if a char is available from `port', else FALSE.
 */
BOOLEAN CharAvailable(DEV_LIST *port);

/*
 * ClearIOBuffer:
 *		Clears the IO buffer `which' from device port.
 *		IO_I_BUFFER:	Inbound buffer;
 *		IO_O_BUFFER:	Outbound buffer;
 *		IO_BUFFERS:		Both buffers;
 */
void ClearIOBuffer(DEV_LIST *port, LONG which);

/*
 * DtrOn:
 * DtrOff:
 *		Sets/Unsets the DTR of port `port'.
 */
void DtrOn(DEV_LIST *port);
void DtrOff(DEV_LIST *port);

/*
 * IsCarrier:
 *		Returns TRUE if carrier on port `port' is available, else
 *		FALSE;
 */
BOOLEAN IsCarrier(DEV_LIST *port);

/*
 * StartReceiver:
 * StopReceiver:
 *		Enables/disables the reiceiver. 
 *		StopReceiver should be used before disk access and
 *		StartReceiver after finished disk i/o.
 */
void StartReceiver(DEV_LIST *port);
void StopReceiver(DEV_LIST *port);

/*
 * Set/Get*xBuffer:
 *		Sets/returns the size of the inbound/outbound iorec
 *		buffers.
 */
WORD SetRxBuffer(DEV_LIST *port, WORD size);
WORD GetRxBuffer(DEV_LIST *port);
WORD SetTxBuffer(DEV_LIST *port, WORD size);
WORD GetTxBuffer(DEV_LIST *port);

/*
 * Misceallaneous functions:
 */

/*
 * get_tos:
 *		returns TOS version
 */
UWORD get_tos(void);

/*
 * getcookie:
 *		searches `cookie' in the cookie jar. If found TRUE is
 *		returned. `value' then holds the value of the found
 *		cookie.
 */
BOOLEAN getcookie(LONG cookie, LONG *value);

/*
 * Get200Hz:
 *		returns the value of the 200Hz hardware counter.
 */
ULONG Get200Hz(void);

/*
 * Calc200Hz:
 *		returns the value the 200Hz hardware counter will have
 *		after `time_to_set' hundreds seconds.
 */
ULONG Calc200Hz(ULONG time_to_set);

BOOLEAN has_drive_u(void);

#endif
