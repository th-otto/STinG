/*********************************************************************/
/*                                                                   */
/*     LogSTinG : Administrative Stuff                               */
/*                                                                   */
/*      Version 1.2                         from 17. March 1997      */
/*                                                                   */
/*********************************************************************/


#ifdef __GNUC__
#include <gem.h>
#else
#include <vdi.h>
#include <aes.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#include "transprt.h"
#include "port.h"
#undef INT16
#include "logsting.h"

#ifdef __GNUC__
extern unsigned long _PgmSize;
#endif

TPL *tpl;
STX *stx;
int offset = -1;
LOGSTRUC log_this;
unsigned char generic[7];
unsigned char cli_flags[CLI_NUM];
unsigned char mod_flags[MOD_NUM];

static DRV_LIST *sting_drivers;
static TPL *sting_tpl;
static TPL my_tpl;
static STX *sting_stx;
static STX my_stx;
static int cache_len;
static char path[256];
static char const version[] = "1.7";
static char *cache;

#ifdef __GNUC__
#define _BasPag _base
#endif


static char const type_txt[][9] = {
	"(uint8)",
	"(int8)",
	"(uint16)",
	"(int16)",
	"(uint32)",
	"(int32)",
	"(string)",
	"(IP)"
};

static char const errors[][16] = {
	"E_NORMAL", "E_OBUFFULL", "E_NODATA", "E_EOF", "E_RRESET",
	"E_UA", "E_NOMEM", "E_REFUSE", "E_BADSYN", "E_BADHANDLE", "E_LISTEN",
	"E_NOCCB", "E_NOCONNECTION", "E_CONNECTFAIL", "E_BADCLOSE",
	"E_USERTIMEOUT", "E_CNTIMEOUT", "E_CANTRESOLVE", "E_BADDNAME",
	"E_LOSTCARRIER", "E_NOHOSTNAME", "E_DNSWORKLIMIT", "E_NONAMESERVER",
	"E_DNSBADFORMAT", "E_UNREACHABLE", "E_DNSNOADDR", "E_NOROUTINE",
	"E_LOCKED", "E_FRAGMENT", "E_TTLEXCEED"
};

static char const not_there[] = "[1][ |  STinG is not loaded or enabled !   ][ Hmmm ]";
static char const corrupted[] = "[1][ |  STinG structures corrupted !   ][ Fuck ]";
static char const found_it[] = "[3][ |  Driver \'%s\',|  by %s, found,   |  version %s.][ Okay ]";
static char const no_module[] = "[1][ |  STinG Transport Driver not found !   ][ Grmbl ]";
static char const no_memory[] = "[1][ |  Can\'t allocate buffer memory !   ][ Shit ]";
static char const itsthere[] = "[1][ |  LogSTinG already installed !   ][ Ooops ]";
static char const installed[] = "[3][ |  LogSTinG debugging tool    | |    installed ...][ Okay ]";



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


static void install(void)
{
	int count;

	path[0] = 'A' + Dgetdrv();
	path[1] = ':';
	Dgetpath(&path[2], 0);
	strcat(path, "\\STING.LOG");

	cache_len = 0;

	tpl = &my_tpl;
	memcpy(tpl, sting_tpl, sizeof(TPL));

	log_this.cli_num = CLI_NUM;
	for (count = 0; count < log_this.cli_num; count++)
		cli_flags[count] = 1;
	log_this.client_itf = cli_flags;

	stx = &my_stx;
	memcpy(stx, sting_stx, sizeof(STX));

	log_this.mod_num = MOD_NUM;
	for (count = 0; count < log_this.mod_num; count++)
		mod_flags[count] = 0;
	log_this.module_itf = mod_flags;

	generic[0] = 0;
	generic[1] = 1;
	generic[2] = 0;
	generic[3] = 0;
	generic[4] = 0;
	generic[5] = 0;
	generic[6] = 0;

	log_this.generic = generic;
	log_this.path = path;
	log_this.version = version;

	install_api(sting_tpl, sting_stx, sting_drivers);
}


long deinstall(void)
{
	int handle;
	char *real_path;

	real_path = generic[6] ? "U:\\PIPE\\DEBUG" : path;

	if (cache_len > 0)
	{
		if ((handle = (int)Fopen(real_path, FO_RW)) < 0)
			handle = (int)Fcreate(real_path, 0);
		if (handle >= 0)
		{
			Fseek(0, handle, 2);
			Fwrite(handle, cache_len, cache);
			Fclose(handle);
		}
	}

	uninstall_api(sting_drivers);

	memcpy(sting_tpl, tpl, sizeof(TPL));
	memcpy(sting_stx, stx, sizeof(STX));

	Mfree(cache);

	Mfree(_BasPag->p_env);
	Mfree(_BasPag);

	return 0;
}


int main(void)
{
	char alert[256];

	appl_init();

	sting_drivers = (DRV_LIST *) Supexec(get_sting_cookie);

	if (sting_drivers == 0L)
	{
		form_alert(1, not_there);
		return 1;
	}
	if (strcmp(sting_drivers->magic, STIK_DRVR_MAGIC) != 0)
	{
		form_alert(1, corrupted);
		return 1;
	}

	if ((*sting_drivers->get_dftab) ("LOGSTING : QUERY") != NULL)
	{
		form_alert(1, itsthere);
		return 1;
	}

	sting_tpl = (TPL *) (*sting_drivers->get_dftab) (TRANSPORT_DRIVER);
	sting_stx = (STX *) (*sting_drivers->get_dftab) (MODULE_DRIVER);

	if (sting_tpl == NULL || sting_stx == NULL)
	{
		form_alert(1, no_module);
	} else
	{
		if ((cache = (char *) Malloc(8192L)) == NULL)
		{
			form_alert(1, no_memory);
		} else
		{
			sprintf(alert, found_it, sting_tpl->module, sting_tpl->author, sting_tpl->version);
			form_alert(1, alert);
			install();
			form_alert(1, installed);
			appl_exit();
			Ptermres(_PgmSize, 0);
		}
	}

	appl_exit();

	return 0;
}


const char *get_error(int16 error)
{
	error *= -1;

	if (error <= 0 || E_LASTERROR < error)
		return "";

	return errors[error];
}


void write_log_text(const char *text)
{
	int limit;
	int handle;
	char *real_path;
	const char *offset_text = "||||||||||||||||||||||||";

	if (generic[0] == 0 || (generic[4] == 0 && offset > 0))
		return;

	real_path = generic[6] ? "U:\\PIPE\\DEBUG" : path;
	limit = generic[5] ? 8191 - ((int)strlen(text) + offset + 2) : 0;

	if (cache_len > limit)
	{
		if ((handle = (int)Fopen(real_path, FO_RW)) < 0)
			if ((handle = (int)Fcreate(real_path, 0)) < 0)
				return;
		Fseek(0, handle, 2);
		Fwrite(handle, cache_len, cache);
		cache_len = 0;
		Fclose(handle);
	}

	if (generic[5])
	{
		memcpy(cache + cache_len, offset_text, offset);
		cache_len += offset;
		limit = (int)strlen(text);
		memcpy(cache + cache_len, text, limit);
		cache_len += limit;
		memcpy(cache + cache_len, "\r\n", 2L);
		cache_len += 2;
	} else
	{
		if ((handle = (int)Fopen(real_path, FO_RW)) < 0)
			if ((handle = (int)Fcreate(real_path, 0)) < 0)
				return;
		Fseek(0, handle, 2);
		if (offset > 0)
			Fwrite(handle, offset, offset_text);
		Fwrite(handle, strlen(text), text);
		Fwrite(handle, 2L, "\r\n");
		Fclose(handle);
	}
}


void write_function(const char *name)
{
	char line[100];

	write_log_text("\r\n");

	sprintf(line, "Call to \'%s\' ($%lx).", name, TIMER_now());
	write_log_text(line);
}


void write_parameter(const char *name, int type, const void *value, const char *supple)
{
	uint32 number;
	int16 ip_1, ip_2, ip_3, ip_4;
	char val_str[32];
	char supp[32];
	char line[100];

	switch (type)
	{
	case UINT8:
		number = *((const uint8 *) value);
		sprintf(val_str, "%u ($%x)", (uint16) number, (uint16) number);
		break;
	case INT8:
		number = *((int8 *) value);
		sprintf(val_str, "%d", (int16) number);
		break;
	case UINT16:
		number = *((const uint16 *) value);
		sprintf(val_str, "%u ($%x)", (uint16) number, (uint16) number);
		break;
	case INT16:
		number = *((int16 *) value);
		sprintf(val_str, "%d", (int16) number);
		break;
	case UINT32:
		number = *((const uint32 *) value);
		sprintf(val_str, "%lu ($%lx)", (unsigned long)number, (unsigned long)number);
		break;
	case INT32:
		number = *((int32 *) value);
		sprintf(val_str, "%ld", (long)number);
		break;
	case STRING:
		strncpy(val_str, (const char *) value, 31);
		val_str[31] = '\0';
		break;
	case IP_ADDR:
		number = *((const uint32 *) value);
		ip_1 = (number >> 24) & 0xff;
		ip_2 = (number >> 16) & 0xff;
		ip_3 = (number >> 8) & 0xff;
		ip_4 = number & 0xff;
		sprintf(val_str, "%d.%d.%d.%d", ip_1, ip_2, ip_3, ip_4);
		break;
	}

	if (*supple)
	{
		supp[0] = '(';
		strcpy(&supp[1], supple);
		strcat(supp, ")");
	} else
	{
		supp[0] = '\0';
	}

	sprintf(line, "   %-16s %10s  :  %s  %s", name, type_txt[type], val_str, supp);

	write_log_text(line);
}


void write_buffer(const void *_buffer, int length)
{
	int index;
	int num;
	int count;
	unsigned char byte;
	char hex_line[50];
	char hex[5];
	char ascii[20];
	char line[100];
	const unsigned char *buffer = (const unsigned char *)_buffer;

	if (buffer == NULL)
		return;

	for (index = 0; index < length; index += 16)
	{
		num = length - index;
		if (num > 16)
			num = 16;
		hex_line[0] = '\0';
		for (count = 0; count < num; count++)
		{
			byte = buffer[index + count];
			sprintf(hex, " %02x", byte);
			strcat(hex_line, hex);
			if (byte < 32 || 127 < byte)
				byte = '.';
			ascii[count] = byte;
		}
		ascii[count] = '\0';
		sprintf(line, "     $%04x : %-48s \'%s", index, hex_line, ascii);
		write_log_text(line);
	}
}
