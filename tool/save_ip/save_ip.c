/*********************************************************************/
/*                                                                   */
/*     STinG : Save-IP Network Tool                                  */
/*                                                                   */
/*                                                                   */
/*      Version 1.0                       from 18. October 1997      */
/*                                                                   */
/*********************************************************************/


#include <aes.h>
#include <tos.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "transprt.h"
#include "layer.h"




STX *stx;

static char *path;
static char port_name[32];

static char const arguments[] = "[1][ | Use 2 args: file + port][ Ok ]";
static char const not_there[] = "[1][ | STinG not loaded and active!][ Hmmm ]";
static char const corrupted[] = "[1][ | STinG structures corrupted !][ Oooops ]";
static char const no_stik[] = "[1][ | You need STinG, not STiK !][ Oooops ]";
static char const no_open[] = "[1][ | Can't create file 'IP.INF' !][ Shit ]";



static long get_sting_cookie(void)
{
	long *work;

	work = *(long **) 0x5a0L;
	if (work == 0)
		return 0;
	for (; *work != 0L; work += 2)
		if (*work == STIK_COOKIE_MAGIC)
			return (*++work);

	return 0;
}


static void do_some_work(void)
{
	PORT *chain;
	uint32 ip;
	int16 file;
	char *name;
	char ip_addr[20];

	if (path[1] == ':')
	{
		Dsetdrv(toupper(path[0]) - 'A');
		path = &path[2];
	}

	name = path;

	if (strrchr(path, '\\') == NULL)
	{
		Dsetpath("\\");
	} else
	{
		*((name = strrchr(path, '\\') + 1) - 1) = '\0';
		Dsetpath(path);
	}

	if ((file = (int16) Fcreate(name, 0)) < 0)
	{
		form_alert(1, no_open);
		return;
	}

	query_chains(&chain, NULL, NULL);

	while (chain != NULL)
	{
		if (strcmp(chain->name, port_name) == 0)
		{
			ip = chain->ip_addr;
			sprintf(ip_addr, "%ld.%ld.%ld.%ld\r\n", (ip >> 24) & 0xff, (ip >> 16) & 0xff, (ip >> 8) & 0xff, ip & 0xff);
			Fwrite(file, strlen(ip_addr), ip_addr);
		}
		chain = chain->next;
	}

	Fclose(file);
}


static void do_alert(const char *alert)
{
	form_alert(1, alert);
	appl_exit();
}


int main(int argc, char **argv)
{
	DRV_LIST *sting_drivers;
	int count;

	appl_init();

	if (argc < 3)
	{
		do_alert(arguments);
		return 1;
	}

	path = argv[1];

	for (count = 2; count < argc; count++)
	{
		strcat(port_name, argv[count]);
		if (count < argc - 1)
			strcat(port_name, " ");
	}

	sting_drivers = (DRV_LIST *) Supexec(get_sting_cookie);

	if (sting_drivers == 0L)
	{
		do_alert(not_there);
		return 1;
	}
	if (strcmp(sting_drivers->magic, STIK_DRVR_MAGIC) != 0)
	{
		do_alert(corrupted);
		return 1;
	}

	stx = (STX *) (*sting_drivers->get_dftab) (MODULE_DRIVER);

	if (stx == NULL)
	{
		do_alert(no_stik);
		return 1;
	}

	do_some_work();

	appl_exit();
	return 0;
}
