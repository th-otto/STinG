/*----------------------------------------------------------------------------*/
/* File name:	CACHE.C							Revision date:	1998.01.19	  */
/* Authors:		Peter Rottengatter  &			Creation date:	1997.04.09	  */
/*				Ronald Andersson											  */
/*----------------------------------------------------------------------------*/
/* Purpose:		DNS Cache functions											  */
/*----------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "transprt.h"
#include "resolve.h"


void wait_flag(volatile signed char *semaphore) GNU_ASM_NAME("wait_flag");
void rel_flag(volatile signed char *semaphore) GNU_ASM_NAME("rel_flag");

static CACHE *cache_root = NULL;
static int16 cache_num = 0;
static volatile signed char sema_cache;
static CACHE *scan_c_p = NULL;
static DNAME *scan_d_p = NULL;

#ifdef __PUREC__
signed char tas(volatile signed char *sema) 0x4ad0; /* tas (a0) */
#endif

/*----------------------------------------------------------------------------*/

char *Ca_first_dom(void)
{
	scan_c_p = cache_root;
	scan_d_p = NULL;
	if (scan_c_p)
		return scan_c_p->real.name;
	return NULL;
}

/*----------------------------------------------------------------------------*/

char *Ca_next_dom(void)
{
	if (scan_c_p)
	{
		if (scan_d_p)
			scan_d_p = scan_d_p->next;	/* next alias */
		else
			scan_d_p = scan_c_p->alias;	/* first alias */
		if (scan_d_p)
			return scan_d_p->name;
		scan_c_p = scan_c_p->next;
		if (scan_c_p)
			return scan_c_p->real.name;
	}
	return NULL;
}

/*----------------------------------------------------------------------------*/

char *Ca_curr_dom(void)
{
	if (scan_d_p)
		return scan_d_p->name;
	if (scan_c_p)
		return scan_c_p->real.name;
	return NULL;
}

/*----------------------------------------------------------------------------*/

uint32 Ca_curr_IP(void)
{
	if (scan_c_p)
		return scan_c_p->IP_address;
	return 0;
}

/*----------------------------------------------------------------------------*/

static void erase_cache(void)
{
	CACHE *walk;
	CACHE *c_tmp;
	DNAME *alias;
	DNAME *d_tmp;

	wait_flag(&sema_cache);

	for (walk = cache_root; walk;)
	{
		for (alias = walk->alias; alias;)
		{
			d_tmp = alias->next;
			KRfree(alias->name);
			KRfree(alias);
			alias = d_tmp;
		}
		c_tmp = walk->next;
		KRfree(walk->real.name);
		KRfree(walk);
		walk = c_tmp;
	}
	scan_d_p = NULL;
	scan_c_p = NULL;
	cache_root = NULL;
	cache_num = 0;

	rel_flag(&sema_cache);
}

/*----------------------------------------------------------------------------*/

static void clean_up(void)
{
	CACHE *c_walk;
	CACHE **c_prev;
	CACHE **crush;
	DNAME *d_walk;
	DNAME **d_prev;
	DNAME *temp;
	uint32 now;
	uint32 ago;
	int16 max_number;
	int16 chg_flag = 0;

	now = time(NULL);

	for (c_walk = *(c_prev = &cache_root); c_walk; c_walk = *c_prev)
	{
		if (c_walk->real.expiry < now && c_walk->real.expiry != 0)
		{
			for (d_walk = c_walk->alias; d_walk; d_walk = temp)
			{
				temp = d_walk->next;
				KRfree(d_walk->name);
				KRfree(d_walk);
			}
			*c_prev = c_walk->next;
			--cache_num;
			KRfree(c_walk->real.name);
			KRfree(c_walk);
			chg_flag++;
		} else
		{
			for (d_walk = *(d_prev = &c_walk->alias); d_walk; d_walk = *d_prev)
			{
				if (d_walk->expiry < now && c_walk->real.expiry != 0)
				{
					*d_prev = d_walk->next;
					KRfree(d_walk->name);
					KRfree(d_walk);
					chg_flag++;
				} else
				{
					d_prev = &d_walk->next;
				}
			}
			c_prev = &c_walk->next;
		}
	}

	if ((max_number = atoi(getvstr("DNS_CACHE"))) < 5)
		max_number = 32;

	while (cache_num > max_number)
	{
		ago = now;
		crush = NULL;
		for (c_walk = *(c_prev = &cache_root); c_walk; c_walk = *c_prev)
		{
			if (c_walk->used < ago)
			{
				crush = c_prev;
				ago = c_walk->used;
			}
			c_prev = &c_walk->next;
		}
		if (crush)
		{
			c_walk = *crush;
			--cache_num;
			for (d_walk = c_walk->alias; d_walk; d_walk = temp)
			{
				temp = d_walk->next;
				KRfree(d_walk->name);
				KRfree(d_walk);
			}
			*crush = c_walk->next;
			KRfree(c_walk->real.name);
			KRfree(c_walk);
			chg_flag++;
		}
	}
	if (chg_flag)
	{
		scan_c_p = cache_root;
		scan_d_p = NULL;
	}
}

/*----------------------------------------------------------------------------*/

int16 load_cache(void)
{
	CACHE *c_walk;
	CACHE **c_prev;
	DNAME *d_walk;
	DNAME **d_prev;
	char *n_walk;
	long error;
	int handle;
	int cf;
	int af;

	if ((error = Fopen(c_file, FO_RW)) < 0)
		return -1;

	handle = (int) error;

	if (cache_root)
		erase_cache();

	wait_flag(&sema_cache);

	c_prev = &cache_root;

	af = 0;
	do
	{
		*c_prev = NULL;
		if (af == 1)
			break;
		cf = 1;

		if ((c_walk = KRmalloc(sizeof(CACHE))) == NULL)
			break;
		if (Fread(handle, sizeof(CACHE), c_walk) != sizeof(CACHE) ||
			(n_walk = KRmalloc(c_walk->real.length + 1)) == NULL)
		{
			KRfree(c_walk);
			break;
		}

		if (Fread(handle, c_walk->real.length, n_walk) != c_walk->real.length ||
			(is_domname(n_walk, c_walk->real.length) == NULL && is_IP_addr(n_walk) == NULL))
		{
			KRfree(n_walk);
			KRfree(c_walk);
			break;
		}
		*(n_walk + c_walk->real.length) = '\0';
		c_walk->real.name = n_walk;

		*c_prev = c_walk;
		c_prev = &c_walk->next;
		cache_num++;

		for (d_prev = &c_walk->alias, af = 0; *d_prev != NULL;)
		{
			*d_prev = NULL;
			af = 1;

			if ((d_walk = KRmalloc(sizeof(DNAME))) == NULL)
				break;
			if (Fread(handle, sizeof(DNAME), d_walk) != sizeof(DNAME) ||
				(n_walk = KRmalloc(d_walk->length + 1)) == NULL)
			{
				KRfree(d_walk);
				break;
			}

			if (Fread(handle, d_walk->length, n_walk) != d_walk->length ||
				(is_domname(n_walk, d_walk->length) == NULL && is_IP_addr(n_walk) == NULL))
			{
				KRfree(n_walk);
				KRfree(d_walk);
				break;
			}
			*(n_walk + d_walk->length) = '\0';
			d_walk->name = n_walk;

			*d_prev = d_walk;
			d_prev = &d_walk->next;
			af = 0;
		}
		cf = 0;
	} while (*c_prev != NULL);

	clean_up();
	rel_flag(&sema_cache);
	Fclose(handle);
	scan_c_p = cache_root;
	scan_d_p = NULL;

	return cf == 1 ? -1 : 0;
}

/*----------------------------------------------------------------------------*/

int16 save_cache(void)
{
	CACHE *walk;
	DNAME *alias;
	long error;
	int handle;

	if ((error = Fcreate(c_file, 0)) < 0)
		return -1;

	handle = (int) error;
	wait_flag(&sema_cache);

	for (walk = cache_root; walk; walk = walk->next)
	{
		if (Fwrite(handle, sizeof(CACHE), walk) != sizeof(CACHE))
			break;
		if (Fwrite(handle, walk->real.length, walk->real.name) != walk->real.length)
			break;
		for (alias = walk->alias; alias; alias = alias->next)
		{
			if (Fwrite(handle, sizeof(DNAME), alias) != sizeof(DNAME))
				break;
			if (Fwrite(handle, alias->length, alias->name) != alias->length)
				break;
		}
		if (alias)
			break;
	}
	rel_flag(&sema_cache);
	Fclose(handle);
	return walk != NULL ? -1 : 0;
}

/*----------------------------------------------------------------------------*/

static int16 compare(const char *first, const char *second)
{
	char one, two;

	while (*first != '\0' && *second != '\0')
	{
		one = *first++;
		two = *second++;
		if (one >= 'a' && one <= 'z')
			one -= 'a' - 'A';
		if (two >= 'a' && two <= 'z')
			two -= 'a' - 'A';
		if (one != two)
			return 1;
	}
	return 0;
}

/*----------------------------------------------------------------------------*/

int16 query_name(const char *name, char *real, uint32 *IP_list, int16 size)
{
	CACHE *walk;
	DNAME *alias;
	int16 length;
	int16 count = 0;
	int16 alias_flag = TRUE;

	wait_flag(&sema_cache);

	clean_up();
	length = strlen(name);

	for (walk = cache_root; walk && count < size; walk = walk->next)
	{
		if (walk->real.length == length && compare(walk->real.name, name) == 0)
		{
			IP_list[count++] = walk->IP_address;
			alias_flag = 0;
			strcpy(real, walk->real.name);
			walk->used = time(NULL);
			continue;
		}
		for (alias = walk->alias; alias && count < size; alias = alias->next)
		{
			if (alias->length == length && compare(alias->name, name) == 0)
			{
				IP_list[count++] = walk->IP_address;
				if (alias_flag)
					strcpy(real, walk->real.name);
				walk->used = time(NULL);
				break;
			}
		}
	}
	rel_flag(&sema_cache);
	return count;
}

/*----------------------------------------------------------------------------*/

int16 query_IP(uint32 addr, char *real, uint32 *IP_list, int16 size)
{
	CACHE *walk;
	int16 found = 0;

	wait_flag(&sema_cache);
	clean_up();

	for (walk = cache_root; walk; walk = walk->next)
	{
		if (walk->IP_address == addr)
		{
			if (size)
				*IP_list = addr;
			strcpy(real, walk->real.name);
			found = 1;
			walk->used = time(NULL);
			break;
		}
	}

	rel_flag(&sema_cache);

	return found;
}

/*----------------------------------------------------------------------------*/

void update_cache(const char *name, uint32 addr, uint32 ttl, int16 type)
{
	CACHE *walk;
	DNAME *alias;
	uint32 now;
	uint32 expiry;
	int16 length;

	wait_flag(&sema_cache);

	now = time(NULL);
	if (ttl != 0x87654321L)
	{
		expiry = now + ttl;
	} else
	{
		expiry = 0;						/* infinite */
	}
	length = (int)strlen(name);

	if (type == DNS_A)					/* update DNS_A entry */
	{
		walk = cache_root;
		while
			(walk &&
			 (walk->real.length != length ||
			  compare(walk->real.name, name) != 0 ||
			  walk->IP_address != addr ||
			  walk->real.type != type))
			walk = walk->next;
		if (walk == NULL)
		{
			if ((walk = KRmalloc(sizeof(CACHE))) == NULL)
			{
				rel_flag(&sema_cache);
				return;
			}
			if ((walk->real.name = KRmalloc(length + 1)) == NULL)
			{
				KRfree(walk);
				rel_flag(&sema_cache);
				return;
			}
			strcpy(walk->real.name, name);
			walk->real.length = length;
			walk->real.type = type;
			walk->real.next = walk->alias = NULL;
			cache_num++;
			walk->next = cache_root;
			cache_root = walk;
			walk->IP_address = addr;
		}
		walk->real.expiry = expiry;
	} else								/* update non-DNS_A entry (alias) */
	{
		for (walk = cache_root; walk; walk = walk->next)
		{
			if (walk->IP_address == addr)
			{
				if (walk->real.length == length && compare(walk->real.name, name) == 0)
				{
					walk->real.expiry = expiry;
					break;
				}
				alias = walk->alias;
				while (alias && (alias->length != length || compare(alias->name, name) != 0))
					alias = alias->next;
				if (alias == NULL)
				{
					if ((alias = KRmalloc(sizeof(DNAME))) == NULL)
					{
						rel_flag(&sema_cache);
						return;
					}
					if ((alias->name = KRmalloc(length + 1)) == NULL)
					{
						KRfree(alias);
						rel_flag(&sema_cache);
						return;
					}
					strcpy(alias->name, name);
					alias->length = length;
					alias->type = type;
					alias->expiry = expiry;
					alias->next = walk->alias;
					walk->alias = alias;
				}
				break;
			}
		}
	}
	if (walk)
		walk->used = now;
	clean_up();

	rel_flag(&sema_cache);

	if (strcmp(getvstr("DNS_SAVE"), "1") == 0 || strcmp(getvstr("DNS_SAVE"), "TRUE") == 0)
		save_cache();
}
