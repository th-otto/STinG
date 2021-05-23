/*
 * ARP caching routines
 *
 * Copyright Roger Burrows (June 2018), based on unpublished SCSILINK code
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

/*
 * IMPORTANT: you must compile with default short ints because the
 * STinG & USB APIs expect this ...
 */
#include <limits.h>
#if INT_MAX != 32767
#error you must compile with short ints!
#endif
#include <string.h>

#include "arpcache.h"					/* application-specific */

#define ARP_NUM     61					/* # of ARP cache entries, prime for better hashing */

/*
 *  ARP cache entry
 */
typedef struct arp_entry
{
	uint32 ip_addr;						/* IP address */
	char ether[ETH_ALEN];				/* EtherNet station address */
	uint16 used;						/* flag to signal in use */
} ARP_ENTRY;

static ARP_ENTRY arpEntries[ARP_NUM];

/* function prototypes */
static void update(ARP_ENTRY *arp, uint32 ip, char *mac);


int16 arp_init(void)
{
	ARP_ENTRY *walk;
	int i;

	/* clear ARP cache */
	for (i = 0, walk = arpEntries; i < ARP_NUM; i++, walk++)
	{
		walk->ip_addr = 0;
		memset(walk->ether, 0, ETH_ALEN);
		walk->used = 0;
	}

	return ARP_NUM;
}

char *arp_cache(uint32 ip_addr)
{
	ARP_ENTRY *walk;
	int16 i;
	int16 n;

	n = ((uint16) (ip_addr & 0x000000ffL)) % ARP_NUM;	/* starting point */

	for (i = n, walk = arpEntries + n; i < ARP_NUM; i++, walk++)
		if (walk->used && (walk->ip_addr == ip_addr))
			return walk->ether;

	for (i = 0, walk = arpEntries; i < n; i++, walk++)
		if (walk->used && (walk->ip_addr == ip_addr))
			return walk->ether;

	return NULL;
}

void arp_enter(uint32 ip_addr, char *ether_addr)
{
	ARP_ENTRY *walk;
	int16 i;
	int16 n;

	n = ((uint16) (ip_addr & 0x000000ffL)) % ARP_NUM;	/* starting point */

	for (i = n, walk = arpEntries + n; i < ARP_NUM; i++, walk++)
		if (!walk->used)
		{
			update(walk, ip_addr, ether_addr);
			return;
		}

	for (i = 0, walk = arpEntries; i < n; i++, walk++)
		if (!walk->used)
		{
			update(walk, ip_addr, ether_addr);
			return;
		}

	/* ARP cache is apparently full, so replace one entry */
	update(arpEntries + n, ip_addr, ether_addr);
}

static void update(ARP_ENTRY *arp, uint32 ip, char *mac)
{
	arp->ip_addr = ip;
	memcpy(arp->ether, mac, ETH_ALEN);
	arp->used = 1;
}

/*
 * instrumentation
 */
int16 arp_count(void)
{
	ARP_ENTRY *walk;
	int i;
	int16 count = 0;

	/* just look through all entries */
	for (i = 0, walk = arpEntries; i < ARP_NUM; i++, walk++)
		if (walk->used)
			count++;

	return count;
}

void arp_table(ARP_INFO *info)
{
	ARP_ENTRY *walk;
	int i;

	/* just look through all entries */
	for (i = 0, walk = arpEntries; i < ARP_NUM; i++, walk++)
		if (walk->used)
		{
			info->ip_addr = walk->ip_addr;	/* copy entry to output */
			memcpy(info->ether, walk->ether, ETH_ALEN);
			info++;
		}
}
