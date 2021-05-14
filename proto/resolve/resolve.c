/*----------------------------------------------------------------------------*/
/* File name:	RESOLVE.C						Revision date:	1998.01.07	  */
/* Authors:		Ronald Andersson				Creation date:	1997.01.13	  */
/*----------------------------------------------------------------------------*/
/* Purpose:		High level STinG protocol for an Internet DNS resolver		  */
/*----------------------------------------------------------------------------*/

#include	<ctype.h>
#include	<stdio.h>
#include	<string.h>
#include	<stdlib.h>

#include	"transprt.h"
#include	"layer.h"
#include	"resolve.h"

#ifdef __GNUC__
extern unsigned long _PgmSize;
#endif

/*----------------------------------------------------------------------------*/

/* Change these macros to reflect the compile date, version etc of the module.*/
/* M_YEAR is (year - 1980), M_MONTH is the month from 1 (Jan.) to 12 (Dec.),  */
/* and M_DAY is day of month (1 ... 31)                                       */

#define	M_TITLE		"Resolver"
#define	M_VERSION	"01.08"
#define	M_YEAR		1998
#define	M_MONTH		1
#define	M_DAY		7
#define	M_AUTHOR	"Ronald Andersson"

char *c_file;
TPL *tpl;
STX *stx;

#ifdef __GNUC__
# define _BasPag _base
#endif

/*----------------------------------------------------------------------------*/

static DRV_LIST *stik_drivers;

/* Put your name and version number of this module in here. */
static LAYER my_layer = {
	M_TITLE,
	M_VERSION,
	0L,
	((M_YEAR - 1980) << 9) | (M_MONTH << 5) | M_DAY,
	M_AUTHOR,
	0,
	NULL,
	NULL
};

static char const fault[] = "RESOLVE.STX : STinG extension module. Only to be started by STinG !\r\n";

/*----------------------------------------------------------------------------*/

static long get_stik_cookie(void)
{
	long *work;

	work = *(long **) 0x5a0L;
	if (work == 0)
		return 0;
	for (; *work != 0L; work += 2)
		if (*work == STIK_COOKIE_MAGIC)
			return *++work;

	return 0;
}										/* ends function get_stik_cookie */

/*----------------------------------------------------------------------------*/

static int16 cdecl my_resolve(const char *domain_p, char **real_domain_p_p, uint32 *IP_list_p, int16 size)
{
	DNS_HDR *dns_hdr_p;
	uint32 host;
	uint32 main_dns;
	uint32 ttl;
	uint16 qtype;
	uint16 type;
	uint16 class;
	uint16 rd_len;
	int16 result = E_CANTRESOLVE;
	int16 data_size;
	int16 i;
	uint8 *dns_data_p;
	uint8 *qd_p;
	uint8 *an_p;
	uint8 *ns_p;
	uint8 *ar_p;
	uint8 *scan;
	char *work;
	char *dom;
	char *dom_name;
	const char *server;

	if (real_domain_p_p)
		*real_domain_p_p = NULL;
	if (size > 0 && IP_list_p)
		*IP_list_p = 0;

	if ((domain_p = is_unblank(domain_p)) == NULL)
		return E_CANTRESOLVE;

	work = NULL;
	if (*domain_p == '[')
	{
		/* CRAP: that modifies the clients argument */
		for (work = (char *)NO_CONST(domain_p) + 1; isascii(*work) && isalpha(*work); work++)
			*work = toupper(*work);
		if (strncmp(domain_p, "[LOAD]", 6) == 0)
		{
			load_cache();
			goto rescurr_DNS;
		}
		if (strncmp(domain_p, "[SAVE]", 6) == 0)
		{
			save_cache();
			goto rescurr_DNS;
		}
		if (strncmp(domain_p, "[FIRST]", 7) == 0)
		{
			Ca_first_dom();
			goto rescurr_DNS;
		}
		if (strncmp(domain_p, "[NEXT]", 6) == 0)
		{
			Ca_next_dom();
		  rescurr_DNS:;
			if ((work = Ca_curr_dom()) == NULL)
				return E_CANTRESOLVE;
			goto resolve_DNS;
		}
		if (strncmp(domain_p, "[CNAME]", 7) == 0)
		{
			type = DNS_A;
			goto update_DNS;
		}
		if (strncmp(domain_p, "[ALIAS]", 7) == 0)
		{
			type = DNS_CNAME;
		  update_DNS:;
			if ((work = strchr(domain_p, '=')) == NULL)
				return E_CANTRESOLVE;
			*work++ = '\0';
			server = work;
			host = diptobip(server);
			dom = NO_CONST(domain_p);
			/* WTF */
			strcpy(dom, skip_space(strchr(dom, ']') + 1));
			if ((work = strchr(dom, ' ')) != NULL)
				*work = '\0';
			else if ((work = strchr(dom, '\t')) != NULL)
				*work = '\0';
			if (is_domname(dom, (int)strlen(dom)) == NULL || host == 0 || strchr(dom, '.') == NULL)
				return E_CANTRESOLVE;
			if ((work = strchr(server, ':')) == NULL)
				ttl = 0x87654321L;		/* infinite */
			else
				ttl = atol(skip_space(work + 1));
			update_cache(dom, host, ttl, type);
			work = NULL;
			goto resolve_DNS;
		}
		return E_CANTRESOLVE;			/* error for all commands that can't resolve */
	  resolve_DNS:;
	}									/* ends cache command parsing if clause */

	if ((dom_name = KRmalloc(256L)) == NULL)
		return E_CANTRESOLVE;
	if ((dom = KRmalloc(256L)) == NULL)
	{
		KRfree(dom_name);
		return E_CANTRESOLVE;
	}

	if (is_IP_addr(domain_p))			/* pure 4-group dotted IP ? */
	{
		qtype = DNS_PTR;
		host = diptobip(domain_p);
		if (size > 0)					/* If there is room for another ip number */
			*IP_list_p = host;			/* store host in user's array/variable */
	} else
	{
		qtype = DNS_A;
		host = 0;
		if (work == NULL)
			work = (char *)NO_CONST(domain_p);
		strcpy(dom, work);
		if (strchr(dom, '.') == NULL)
		{
			work = NO_CONST(getvstr("DOMAIN"));
			if (work[0] && work[1])
			{
				strcat(dom, ".");
				strcat(dom, work);
			}
		}
	}

	if (qtype == DNS_PTR)
		result = query_IP(host, dom_name, IP_list_p, size);
	else
		result = query_name(dom, dom_name, IP_list_p, size);

	if (result > 0)						/* result found in cache ? */
	{
		if (real_domain_p_p)
			KRrealloc(*real_domain_p_p = dom_name, strlen(dom_name) + 1);
		else
			KRfree(dom_name);
		KRfree(dom);
		return result;				/* return with cache result */
	}

	server = getvstr("NAMESERVER");
	for (result = 0; server && *server; server = next_dip(server))
	{
		main_dns = diptobip(server);
		if (qtype == DNS_PTR)
			strcat(biptodrip(host, dom), ".IN-ADDR.ARPA");
		data_size = do_query(dom, main_dns, qtype, &dns_hdr_p, &dns_data_p, &qd_p, &an_p, &ns_p, &ar_p);
		if (data_size > 0 && an_p != NULL)
		{
			for (i = 0, scan = an_p; (scan < dns_data_p + data_size) && (i++ < dns_hdr_p->AN_count); scan += rd_len)
			{
				scan = pass_RRname(dns_data_p, scan, dom_name);
				type = (scan[0] << 8) + scan[1];
				scan += 2;
				class = (scan[0] << 8) + scan[1];
				scan += 2;
				ttl = ((((((uint32) scan[0] << 8) + scan[1]) << 8) + scan[2]) << 8) + scan[3];
				scan += 4;
				rd_len = (scan[0] << 8) + scan[1];
				scan += 2;
				if (type != qtype || class != 1)
					continue;			/* search on */
				/* Here we found something */
				if ((scan < dns_data_p + data_size) && (i <= dns_hdr_p->AN_count))
				{
					if (qtype == DNS_PTR)
					{
						pass_RRname(dns_data_p, scan, dom_name);
					} else
					{
						host = ((((((uint32) scan[0] << 8) + scan[1]) << 8) + scan[2]) << 8) + scan[3];
					}
					update_cache(dom_name, host, ttl, DNS_A);
					update_cache(dom, host, ttl, DNS_CNAME);
					if (size > result)
						IP_list_p[result++] = host;
				}
			}							/* ends for-loop parsing for correct RRs */
		}
		if (dns_hdr_p)
		{
			KRfree(dns_hdr_p);
			dns_hdr_p = NULL;
		}
		if (dns_data_p)
		{
			KRfree(dns_data_p);
			dns_data_p = NULL;
		}
		if (result)
			break;
	}									/* ends for-loop to try each nameserver */

	if (!result)
	{
		if (qtype == DNS_PTR)
		{
			biptodip(host, dom_name);
			update_cache(dom_name, host, 3600, DNS_A);
			if (size > result)
				IP_list_p[result++] = host;
		} else
		{
			result = E_CANTRESOLVE;
		}
	}
	
	if (real_domain_p_p)
		KRrealloc(*real_domain_p_p = dom_name, strlen(dom_name) + 1);
	else
		KRfree(dom_name);
	KRfree(dom);

	return result;
}

/*----------------------------------------------------------------------------*/

static int16 install(void)
{
	LAYER *layers;
	int16 path_len;

	if ((c_file = KRmalloc(256)) == NULL)
		return FALSE;

	Dgetpath(c_file, 0);
	path_len = (int16) strlen(c_file);
	KRfree(c_file);

	if ((c_file = KRmalloc(path_len + 15)) == NULL)
		return FALSE;

	c_file[0] = 'A' + Dgetdrv();
	c_file[1] = ':';
	Dgetpath(&c_file[2], 0);
	strcat(c_file, "\\CACHE.DNS");

	if (load_cache() < 0)
	{
		puts("\r\nDNS cache load failed !!!\r\n");
	}


	/* Fetch address of high level protocol layer chain. */

	query_chains(NULL, NULL, &layers);

	/* Find last entry of high protocol layer chain. */

	while (layers->next)
		layers = layers->next;

	/* Link our entry in high protocol layer chain. */

	my_layer.basepage = _BasPag;
	layers->next = &my_layer;

	/* Link our service (resolve) into TPL structure. */

	tpl->resolve = my_resolve;

	return TRUE;
}										/* ends function install */

/*----------------------------------------------------------------------------*/

int main(void)
{
	if (strcmp(_BasPag->p_cmdlin, "\012STinG_Load") != 0)
	{
		(void) Cconws(fault);
		return 1;
	}

	stik_drivers = (DRV_LIST *) Supexec(get_stik_cookie);

	if (stik_drivers == 0)
		return 1;

	if (strcmp(stik_drivers->magic, STIK_DRVR_MAGIC) != 0)
		return 1;

	tpl = (TPL *) (*stik_drivers->get_dftab) (TRANSPORT_DRIVER);
	stx = (STX *) (*stik_drivers->get_dftab) (MODULE_DRIVER);

	if (tpl != NULL && stx != NULL)
	{
		if (install())
			Ptermres(_PgmSize, 0);
	}
	return 1;
}


#if 0
int16 is_IP_addr(const char *text)
{
	int16 count;
	int16 length;
	int16 dots;
	char chr;

	length = strlen(text);

	for (count = dots = 0; count < length; count++)
	{
		chr = text[count];
		if (chr == ' ' || ('0' <= chr && chr <= '9') || chr == '.')
		{
			if (chr == '.')
				dots++;
		} else
			return (FALSE);
	}

	return dots != 3 ? FALSE : TRUE;
}
#endif
