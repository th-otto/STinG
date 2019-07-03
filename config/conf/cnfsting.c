/*********************************************************************/
/*                                                                   */
/*     Konfiguration der STinG Low Level Ports                       */
/*                                                                   */
/*      AUTO-Version 0.1                 vom 5. Dezember 1996        */
/*                                                                   */
/*********************************************************************/


#include <tos.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "transprt.h"
#include "layer.h"


TPL *tpl;
STX *stx;
static DRV_LIST *sting_drivers;
static PORT *ports[12];
static int port_num;



static long get_sting_cookie(void)
{
	long *work;

	work = *(long **) 0x5a0L;
	if (work == 0)
		return 0;
	for (; *work != 0L; work += 2)
		if (*work == STIK_COOKIE_MAGIC)
			return *++work;

	return 0;
}


static int locate_sting(void)
{
	PORT *port_ptr;

	sting_drivers = (DRV_LIST *) Supexec(get_sting_cookie);

	if (sting_drivers == 0)
		return FALSE;

	if (strcmp(sting_drivers->magic, STIK_DRVR_MAGIC) != 0)
		return FALSE;

	tpl = (TPL *) (*sting_drivers->get_dftab) (TRANSPORT_DRIVER);
	stx = (STX *) (*sting_drivers->get_dftab) (MODULE_DRIVER);

	if (tpl == (TPL *) NULL || stx == (STX *) NULL)
		return FALSE;

	if ((long) (port_ptr = (PORT *) (long)query_port("")) < 1024) /* WTF? query_port returns flag only */
		return FALSE;

	for (port_num = 0; port_ptr; port_ptr = port_ptr->next)
		if (port_ptr->type)
			if (port_num < 12)
				ports[port_num++] = port_ptr;

	if (port_num == 0)
		return FALSE;

	return TRUE;
}


static int fetch_line(int handle, char *string)
{
	int count;
	char charac;
	char *begin;

	begin = string;

	for (count = 0; Fread(handle, 1, &charac) > 0 && count < 60; count++)
	{
		*string++ = charac;
		if (charac == '\n')
			break;
	}
	*string = '\0';

	return strncmp(begin, "Port : ", 7) == 0;
}


static uint32 hextoul(const char *string)
{
	uint32 number = 0;
	uint32 digit;

	while (('0' <= *string && *string <= '9') || ('A' <= *string && *string <= 'F'))
	{
		digit = *string++ - '0';
		number = 16 * number + (digit <= 9 ? digit : digit - 7);
	}

	return number;
}


static int read_config_file(void)
{
	long error;
	int handle;
	int count;
	char temp[64];
	char *work;

	if ((error = Fopen("\\AUTO\\STING.SET", FO_READ)) < 0)
		return FALSE;
	handle = (int16) error;

	while (fetch_line(handle, temp))
	{
		if ((work = strchr(temp, '\r')) != NULL)
			*work = '\0';
		for (count = 0; count < port_num; count++)
		{
			if (strcmp(ports[count]->name, temp + 7) == 0)
				break;
		}
		if (count == port_num)
		{
			for (count = 0; count < 5; count++)
				fetch_line(handle, temp);
		} else
		{
			fetch_line(handle, temp);
			ports[count]->active = (atoi(temp)) ? 1 : 0;
			fetch_line(handle, temp);
			ports[count]->flags = hextoul(temp);
			fetch_line(handle, temp);
			ports[count]->ip_addr = hextoul(temp);
			ports[count]->sub_mask = hextoul(temp + 10);
			fetch_line(handle, temp);
			ports[count]->mtu = atoi(temp);
			fetch_line(handle, temp);
		}
	}

	Fclose(handle);

	return TRUE;
}


#if 0 /* unused */
static void set_sting_data(void)
{
	PORT *src;
	PORT *dest;
	int count;

	setvstr("CONFSTING", "TRUE");

	for (count = 0; count < port_num; count++)
	{
		src = ports[count];
		dest = src->next;
		if (dest->active != src->active)
		{
			if (src->active)
			{
				if (on_port(src->name) == 0)
					printf("Configuring port \'%s\' failed !", src->name);
			} else
			{
				off_port(src->name);
			}
		}
		dest->flags = src->flags;
		dest->mtu = src->mtu;
		dest->ip_addr = src->ip_addr;
		dest->sub_mask = src->sub_mask;
	}
}
#endif


int main(void)
{
	puts("\n\033p ConfSTinG : STinG configuration tool \033q");

	if (!locate_sting())
	{
		puts("Problem locating STinG !");
		return 1;
	}

	if (!read_config_file())
	{
		puts("Problem finding or reading file !");
		return 1;
	}

	puts("Configuration done.");
	return 0;
}
