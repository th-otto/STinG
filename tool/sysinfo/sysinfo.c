
/*********************************************************************/
/*                                                                   */
/*     STinG : List System Info Tool                                 */
/*                                                                   */
/*                                                                   */
/*      Version 1.0                       from 18. October 1997      */
/*                                                                   */
/*********************************************************************/


#include <tos.h>
#include <stdio.h>
#include <string.h>

#include "transprt.h"
#include "port.h"
#include "layer.h"





static DRV_LIST *sting_drivers;
static STIK_CONFIG *config;
TPL *tpl;
STX *stx;

static char const port_type[][30] = {
	"None",
	"Serial Point to Point Link  ",
	"Parallel Point to Point Link",
	"Serial Bus Type Link        ",
	"Parallel Bus Type Link      ",
	"Serial Ring Type Link       ",
	"Parallel Ring Type Link     ",
	"IP Masquerade Pseudo Port   "
};

static char const not_there[] = "STinG is not loaded or enabled !";
static char const corrupted[] = "STinG structures corrupted !";



static long get_sting_cookie(void)
{
	long *work;

	work = *(long **)0x5a0L;
	if (work == 0)
		return 0;
	for (; *work != 0L; work += 2)
		if (*work == STIK_COOKIE_MAGIC)
			return *++work;

	return 0L;
}


static void do_some_work(void)
{
	DRIVER *driv;
	PORT *port;
	LAYER *layr;
	int num_drivers = 0;
	int num_ports = 0;
	int num_layers = 0;

	puts("\n   *** STinG TCP/IP InterNet Connection Layer ***");
	puts("   ----------------------------------------------");
	printf("Transport Driver V%s by %s.\n", tpl->version, tpl->author);
	printf("Module Driver    V%s by %s.\n", stx->version, stx->author);

	for (driv = config->drivers; driv; driv = driv->next)
		num_drivers++;
	for (port = config->ports; port; port = port->next)
		num_ports++;
	for (layr = config->layers; layr; layr = layr->next)
		num_layers++;

	printf("Basepage address :  0x%lxL\n", sting_drivers->sting_basepage);

	printf("Loaded :  %d drivers comprising %d ports, %d high level protocols.\n\n",
		   num_drivers, num_ports, num_layers);

	puts("Dropped packets statistics:");
	printf("          Low memory : %8ld\n", config->stat_lo_mem);
	printf("        TTL exceeded : %8ld\n", config->stat_ttl_excd);
	printf("      Wrong checksum : %8ld\n", config->stat_chksum);
	printf("       Undeliverable : %8ld\n", config->stat_unreach);
	printf("Total datagrams : %ld\n", config->stat_all);

	puts("\n   --- Drivers & Ports ---\n");

	for (driv = config->drivers; driv; driv = driv->next)
	{
		num_ports = 0;
		for (port = config->ports; port; port = port->next)
		{
			if (port->driver == driv)
				num_ports++;
		}
		printf("Driver \"%s\" (%d port%s)\n", driv->name, num_ports, (num_ports > 1) ? "s" : "");
		printf("  Version %s, %d/%d/%d by %s\n", driv->version,
			   driv->date & 0x1f, (driv->date >> 5) & 0xf, 80 + ((driv->date >> 9) & 0x7f), driv->author);
		printf("  Basepage address : 0x%lxL\n", driv->basepage);
		for (port = config->ports; port; port = port->next)
		{
			if (port->driver == driv)
			{
				printf("Port \"%s\" (%sctive)\n", port->name, (port->active) ? "A" : "Ina");
				printf("  Type : %s\n", port_type[port->type]);
				printf("  IP-Address : %ld.%ld.%ld.%ld,", (port->ip_addr >> 24) & 0xff,
					   (port->ip_addr >> 16) & 0xff, (port->ip_addr >> 8) & 0xff, port->ip_addr & 0xff);
				printf("  MTU : %d  (%d maximum)\n", port->mtu, port->max_mtu);
				printf("  Statistics : %ld bytes sent, %ld bytes received, %d packets dropped\n",
					   port->stat_sd_data, port->stat_rcv_data, port->stat_dropped);
			}
		}
		puts("");
	}

	puts("   --- High Level Protocols ---\n");

	for (layr = config->layers; layr; layr = layr->next)
	{
		printf("Protocol \"%s\"\n", layr->name);
		printf("  Version %s, %d/%d/%d by %s\n", layr->version,
			   layr->date & 0x1f, (layr->date >> 5) & 0xf, 1980 + ((layr->date >> 9) & 0x7f), layr->author);
		printf("  Basepage address : 0x%lxL\n", layr->basepage);
		printf("  Statistics : %d datagrams dropped\n", layr->stat_dropped);
		puts("");
	}

	puts("STinG Sysinfo finished ...\n");
}


int main(int argc, char **argv)
{
	sting_drivers = (DRV_LIST *) Supexec(get_sting_cookie);

	if (sting_drivers == 0L)
	{
		puts(not_there);
		return 1;
	}
	if (strcmp(sting_drivers->magic, MAGIC) != 0)
	{
		puts(corrupted);
		return 1;
	}
	
	config = sting_drivers->cfg;

	tpl = (TPL *) (*sting_drivers->get_dftab) (TRANSPORT_DRIVER);
	stx = (STX *) (*sting_drivers->get_dftab) (MODULE_DRIVER);

	if (tpl != NULL && stx != NULL)
		do_some_work();

	if (argc == 0 || argv[0] == NULL || argv[0][0] == '\0')
		getchar();
	return 0;
}
