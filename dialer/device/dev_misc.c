#include "device.h"
#include <mint/sysvars.h>

/*-----------------------------------------------------------------------------
 * Funktionsname:	Calc200Hz, Get200Hz
 *		->	nichts bzw. in d0 die einzustellende Zeit;
 *		<-	d0:	Bisherige Anzahl der 200Hz-Interrupts bzw.
 *				die Timerzahl, wann die Zeit verstrichen sein wird
 *
 * Die Funktion holt die Anzahl der 200Hz-Interrupts aus der
 * Adresse $4ba (_hz_200-Vektor).
 */

ULONG Calc200Hz(ULONG time_to_set)
{
	return time_to_set + time_to_set + Get200Hz();
}


static long get_timer(void)
{
	return *(long *)0x4ba;
}


/*
 * Get200Hz:
 *		returns the value of the 200Hz hardware counter.
 */
ULONG Get200Hz(void)
{
	return Supexec(get_timer);
}


/*-----------------------------------------------------------------------------
 * Funktionsname:	has_drive_u
 *		->	nichts
 *		<-	TRUE wenn U: existiert, sonst FALSE
 *
 * Die Funktion sieht unter der Adresse _DRVBITS nach, ob ein
 * Laufwerk U: existiert
 */
BOOLEAN has_drive_u(void)
{
	return (Drvmap() & 0x100000L) != 0;
}


static long _get_tos(void)
{
	return *(long *)0x4f2;
}

/*-----------------------------------------------------------------------------
 * Funktionsname:	get_tos
 *		->	nichts
 *		<-	nichts
 *
 * Die Funktion holt die TOS-Version Åber den _sysbase-Vektor des Betriebs-
 * systems.
 */
UWORD get_tos(void)
{
	OSHEADER *sys = (OSHEADER *)Supexec(_get_tos);
	return sys->os_version;
}



static long get_jarptr(void)
{
	return *(long *)0x5a0;
}

/*
 * --------------------------------------------------------------------------
 * getcookie:	Funktion zum Suchen nach einem bestimmten Cookie im
 *				Cookie Jar.
 *
 *	->	d0:	- 32 Bit Wert des zu suchenden Cookies;
 *		a0: - Zeiger auf Variable fÅr den gefundenen Wert des Cookies
 *	<-	d0:	- TRUE wenn Cookie gefunden, sonst FALSE
 */
BOOLEAN	getcookie(LONG cookie, LONG *value)
{
	LONG *jar = (LONG *)Supexec(get_jarptr);
	
	if (jar != 0)
	{
		while (*jar != 0)
		{
			if (*jar == cookie)
			{
				if (value)
					*value = jar[1];
				return TRUE;
			}
			jar += 2;
		}
	}
	return FALSE;
}
