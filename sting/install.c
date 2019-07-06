/*********************************************************************/
/*                                                                   */
/*     STinG : API and IP kernel package                             */
/*                                                                   */
/*                                                                   */
/*      Version 1.0                      from 23. November 1996      */
/*                                                                   */
/*      Module for Installation, Config Strings, *.STX Loading       */
/*                                                                   */
/*********************************************************************/


#include <tos.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#include "globdefs.h"


#define  MAX_SEMAPHOR    64


char sting_path[245];
char semaphors[MAX_SEMAPHOR] GNU_ASM_NAME("semaphors");



static long get_boot_drv(void)
{
	unsigned short *_bootdev = (void *) 0x446L;

	return 'A' + *_bootdev;
}


static void get_path(void)
{
	int handle;
	long len;
	char *ptr;
	static char const file[] = "\\STING.INF";
	static char const path[] = "\\STING\\";

	sting_path[0] = 'A' + Dgetdrv();
	sting_path[1] = ':';
	Dgetpath(&sting_path[2], 0);
	strcat(sting_path, file);
	handle = (int) Fopen(sting_path, FO_READ);

	if (handle < 0)
	{
		strcpy(&sting_path[3], "AUTO");
		strcat(sting_path, file);
		handle = (int) Fopen(sting_path, FO_READ);

		if (handle < 0)
		{
			strcpy(&sting_path[2], file);
			handle = (int) Fopen(sting_path, FO_READ);

			if (handle < 0)
			{
				sting_path[0] = (char) Supexec(get_boot_drv);
				handle = (int) Fopen(sting_path, FO_READ);

				if (handle < 0)
				{
					strcpy(sting_path, path);
					return;
				}
			}
		}
	}

	len = Fread(handle, 240L, sting_path);
	Fclose(handle);

	if (len > 0)
	{
		if ((ptr = strchr(sting_path, '\r')) != NULL)
			*ptr = '\0';
		if ((ptr = strchr(sting_path, '\n')) != NULL)
			*ptr = '\0';
		sting_path[len] = '\0';
	} else
	{
		strcpy(sting_path, path);
	}
}


static int16 compare(const char *string_1, const char *string_2, int16 number)
{
	int16 count;

	for (count = 0; count < number; count++)
	{
		if (toupper(string_1[count]) != toupper(string_2[count]))
			return FALSE;
		if (!string_1[count] || !string_2[count])
			return FALSE;
	}
	
	if (string_1[count] != '=')
		return FALSE;

	return TRUE;
}


static int16 search_value(char **string)
{
	while (**string == ' ' || **string == '\t')
		(*string)++;

	if (**string != '=')
		return **string == '\r' || **string == '\n' ? 1 : -1;

	(*string)++;

	while (**string == ' ' || **string == '\t')
		(*string)++;

	return 0;
}


static int16 init_cfg(char fname[])
{
	int32 status;
	int32 length;
	int32 count;
	int32 memory;
	int16 handle;
	char *cfg_ptr;
	char *work;
	char *name;
	const char *value;
	char *tmp;

	if ((status = Fopen(fname, FO_READ)) < 0)
		return -1;
	handle = (int16) status;

	length = Fseek(0, handle, 2);
	Fseek(0, handle, 0);

	if ((cfg_ptr = (char *) Malloc(length + 3)) == NULL)
	{
		Fclose(handle);
		return -1;
	}
	status = Fread(handle, length, cfg_ptr);
	Fclose(handle);

	if (status != length)
	{
		Mfree(cfg_ptr);
		return -1;
	}
	strcpy(&cfg_ptr[length], "\r\n");

	for (count = 0; count < length; count++)
	{
		if (compare(&cfg_ptr[count], "ALLOCMEM", 8))
		{
			work = &cfg_ptr[count - 1];
			if (count != 0 && *work != '\r' && *work != '\n')
				continue;
			work = &cfg_ptr[count + 8];
			if (search_value(&work) != 0)
			{
				Mfree(cfg_ptr);
				return -1;
			}
			if ((memory = atol(work)) < 1024)
			{
				Mfree(cfg_ptr);
				return -2;
			}
			if (KRinitialize(memory) < 0)
				return -3;
			break;
		}
	}

	if (count >= length)
	{
		Mfree(cfg_ptr);
		return -1;
	}

	for (count = 0; count <= STIK_CFG_NUM; count++)
		conf.cv[count] = NULL;

	work = cfg_ptr;

	while (*work && work < &cfg_ptr[length] && count > 0)
	{
		if (isalpha(*work))
		{
			name = work;
			while (isalnum(*work) || *work == '_')
				work++;
			tmp = work;
			value = work;
			switch (search_value(&work))
			{
			case 0:
				if (*work == '\r' || *work == '\n')
				{
					value = "0";
					break;
				}
				/* fall through */
			case -1:
				value = work;
				break;
			case 1:
				value = "1";
				break;
			}
			while (*work && *work != '\r' && *work != '\n')
				work++;
			--work;
			while (*work == ' ' || *work == '\t')
				--work;
			work++;
			*work++ = *tmp = '\0';
			setvstr(name, value);
			count--;
		}
		while (*work && *work != '\r' && *work != '\n')
			work++;
		while (*work == '\r' || *work == '\n')
			work++;
	}

	Mfree(cfg_ptr);
	return 0;
}


static void load_stx(void)
{
	DTA *my_dta;
	char *walk = sting_path;
	char temp[32];
	int16 error;
	int16 modules = FALSE;

	if (sting_path[0] != '\\')
	{
		walk += 2;
		Dsetdrv(sting_path[0] - 'A');
	}
	Dsetpath(walk);

	my_dta = Fgetdta();

	printf("Loading Modules : ");

	error = Fsfirst("*.STX", 0);

	while (error >= 0)
	{
		strcpy(temp, my_dta->d_fname);
		if (strchr(temp, '.'))
			*strchr(temp, '.') = '\0';
		printf("%s, ", temp);
		Pexec(0, my_dta->d_fname, "\012STinG_Load", "");
		modules = TRUE;
		error = Fsnext();
	}

	if (modules)
		printf("\b\b.\n");
	else
		printf("None.\n");
}


int main(void)
{
	int count;
	char def_conf[255];

	puts("\n\033p  *** STinG TCP/IP InterNet Connection Layer ***  \033q");

	for (count = 0; count < MAX_SEMAPHOR; count++)
		semaphors[count] = 0;

	get_path();

	strcpy(def_conf, sting_path);
	strcat(def_conf, "DEFAULT.CFG");

	install_PrivVio();

	switch (init_cfg(def_conf))
	{
	case -3:
		puts("Could not allocate enough memory ! No installation ...");
		uninst_PrivVio();
		return 1;
	case -2:
		puts("ALLOCMEM must be at least 1024 bytes ! No installation ...");
		uninst_PrivVio();
		return 1;
	case -1:
		puts("Problem finding/reading DEFAULT.CFG ! No installation ...");
		uninst_PrivVio();
		return 1;
	}

	if (Supexec(init_cookie) < 0)
	{
		puts("STinG already installed ! No installation ...");
		uninst_PrivVio();
		if (memory)
			Mfree(memory);
		return 1;
	}

	install();
	load_stx();
	routing_table();

	printf("STinG version %s (%s) installed ...\n", TCP_DRIVER_VERSION, STX_LAYER_VERSION);

	{
		const char *config;
		int active;

		config = getvstr("ACTIVATE");
		if (*config != '0' && *config != 'F')
		{
			active = atoi(getvstr("THREADING"));
			if (active == 0)
				active = 50;
			if (active < 10)
				active = 10;
			if (active > 1000)
				active = 1000;
			set_sysvars(1, active / 5);
		}
	}

	Ptermres(_PgmSize, 0);
	return 0;
}


int16 cdecl setvstr(const char *name, const char *value)
{
	uint16 length;
	uint16 status;
	uint16 count;
	char *work;

	length = strlen(name);

	if (!isalpha(name[0]) && name[0] != '_')
		return FALSE;

	for (count = 1; count < length; count++)
	{
		if (!isalnum(name[count]) && name[count] != '_')
		{
			return FALSE;
		}
	}

	status = lock_exec(0);

	for (count = 0; count < STIK_CFG_NUM; count++)
	{
		if (conf.cv[count])
		{
			if (compare(name, conf.cv[count], length))
				break;
		} else
		{
			break;
		}
	}

	if (count >= STIK_CFG_NUM || (length = length + strlen(value) + 3) > 253)
	{
		lock_exec(status);
		return FALSE;
	}

	work = conf.cv[count];
	if (work)
	{
		if (length > (unsigned char)work[-1])
		{
			if ((work = KRmalloc(length)) == NULL)
			{
				lock_exec(status);
				return FALSE;
			}
			KRfree(conf.cv[count] - 1);
			*work++ = (unsigned char) length;
		}
	} else
	{
		if ((work = KRmalloc(length)) == NULL)
		{
			lock_exec(status);
			return FALSE;
		}
		*work++ = (unsigned char) length;
	}

	conf.cv[count] = work;
	strcpy(work, name);
	strcat(work, "=");
	strcat(work, value);

	lock_exec(status);

	return TRUE;
}


const char *cdecl getvstr(const char *name)
{
	uint16 length;
	uint16 status;
	uint16 count;
	const char *result;

	length = strlen(name);

	status = lock_exec(0);

	for (count = 0; count < STIK_CFG_NUM; count++)
	{
		if (conf.cv[count])
		{
			if (compare(name, conf.cv[count], length))
				break;
		} else
		{
			break;
		}
	}
		
	result = conf.cv[count] + length + 1;

	if (count == STIK_CFG_NUM || conf.cv[count] == NULL)
		result = "0";

	lock_exec(status);

	return result;
}
