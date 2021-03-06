/*********************************************************************/
/*                                                                   */
/*     Konfiguration der STinG Low Level Ports                       */
/*                                                                   */
/*                                                                   */
/*      CPX-Version 1.2                   vom 12. August 1998        */
/*                                                                   */
/*      zu kompilieren mit Pure C ohne String - Merging !!!          */
/*                                                                   */
/*********************************************************************/

#ifdef __GNUC__
#include <gem.h>
#else
#include <aes.h>
#include <vdi.h>
#endif
#undef BYTE
#define BYTE char
#ifndef WORD
# define WORD short
#endif
#ifndef LONG
# define LONG long
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "sting.h"


/*
 *  String for the path to the sting.prt file. Must be array, and first variable !!!
 */
char inf_file[128] = "C:\\STING\\STING.PRT";

#include "sting.rsh"
#include "transprt.h"
#include "port.h"

#include "cpxdata.h"



#define  MESSAGE     -1
#define  CROS_CHK    (OS_CROSSED | OS_CHECKED)

#define  FLG_PRTCL           0x0001
#define  FLG_VJHC            0x0002
#define  FLG_LANBIT          0x0004


typedef struct export
{
	char port[32];
	int16 type;
	char settings[128];
	char changed[128];
} EXPORT;


CPXINFO *cdecl cpx_init(XCPB *para_ptr);



TPL *tpl;
STX *stx;
int errno;
static EXPORT ports[16];
static EXPORT *mask[16];
static XCPB *params;
static OBJECT *box;

static char popup[16][20];
static char *port_name[16];
static char hard_buff[192];
static char *hard[16];
static char *mask_name[16];

static char const type_name[][30] = {
	"",
	"Serial Point to Point Link  ",
	"Parallel Point to Point Link",
	"Serial Bus Type Link        ",
	"Parallel Bus Type Link      ",
	"Serial Ring Type Link       ",
	"Parallel Ring Type Link     ",
	"IP Masquerade Pseudo Port   "
};
static const char *const mode_name[2] = { "  Addressing ", "  General    " };

static _WORD const type_box[] = {
	0, BOX_PAR1, BOX_PAR2, BOX_PAR3, BOX_PAR4, BOX_PAR5, BOX_PAR6, BOX_PAR7
};
static _WORD const edit_box[] = { 0, 0, 0, SBL_MAC, 0, 0, 0, M_IP };

static int port_num;
static int act_port;
static _WORD vdi_handle;
static int box_mode;
static int reload;
static int hard_num;
static int hardware;
static int mask_num;
static int masque;



static void parse_tree(OBJECT *tree, _WORD object, _WORD mode)
{
	_WORD work;

	if (mode == -1)
	{
		if (tree[object].ob_flags & OF_EDITABLE)
			tree[object].ob_type |= 0x100;
	} else
	{
		if (tree[object].ob_type & 0x100)
		{
			if (mode == 1)
				tree[object].ob_flags |= OF_EDITABLE;
			else
				tree[object].ob_flags &= ~OF_EDITABLE;
		}
	}

	if ((work = tree[object].ob_head) == -1)
		return;

	do
	{
		parse_tree(tree, work, mode);
		work = tree[work].ob_next;
	} while (work != object);
}


static char *find_this_value(const char *conf, const char *name)
{
	int index;

	while (*conf)
	{
		for (index = 0; name[index] != '\0'; index++)
			if (name[index] != conf[index])
				break;
		if (name[index] == '\0')
			return (char *)conf;
		conf++;
	}

	return NULL;
}


static const char *chk_value(const char *conf, const char *old, const char *name)
{
	const char *posi_conf;
	const char *posi_old;

	posi_conf = find_this_value(conf, name);
	posi_old = find_this_value(old, name);

	if (posi_conf == NULL)
		return NULL;

	if (posi_old == NULL)
		return strchr(posi_conf, '=') + 1;

	while (*posi_conf != ' ' && *posi_conf != '\0' && *posi_old != ' ' && *posi_old != '\0')
	{
		if (*posi_conf++ != *posi_old++)
			return strchr(find_this_value(conf, name), '=') + 1;
	}

	if ((*posi_conf == ' ' || *posi_conf == '\0') && (*posi_old == ' ' || *posi_old == '\0'))
		return NULL;

	return strchr(find_this_value(conf, name), '=') + 1;
}


static char *get_value(EXPORT *port, const char *name)
{
	char *posi;

	if ((posi = find_this_value(port->changed, name)) != NULL)
		return strchr(posi, '=') + 1;

	if ((posi = find_this_value(port->settings, name)) != NULL)
		return strchr(posi, '=') + 1;

	return NULL;
}


static void set_value(char *conf, const char *name, const char *value)
{
	char *posi;
	const char *space;

	if ((posi = find_this_value(conf, name)) != NULL)
	{
		if ((space = strchr(posi, ' ')) != NULL)
			strcpy(posi, space + 1);
		else
			posi[posi == &conf[0] ? 0 : -1] = '\0';
	}

	posi = &conf[strlen(conf)];
	*posi++ = ' ';
	strcpy(posi, name);
	posi += strlen(name);
	*posi++ = '=';

	for (space = value; *space; space++)
		*posi++ = *space == ' ' ? '~' : *space;

	*posi = '\0';
}


static void get_port_data(int count)
{
	uint32 number32;
	uint16 number16;
	int16 sign16;
	char *conf;
	char buffer[32];

	ports[count].settings[0] = ports[count].changed[0] = '\0';
	conf = ports[count].settings;

	set_value(conf, "ACTIVE", query_port(ports[count].port) ? "1" : "0");

	cntrl_port(ports[count].port, (uint32) & number32, CTL_GENERIC_GET_IP);
	sprintf(buffer, "%08lx", number32);
	set_value(conf, "IP", buffer);

	if (ports[count].type != L_SER_PTP)
	{
		if (ports[count].type != L_PAR_PTP)
		{
			cntrl_port(ports[count].port, (uint32) & number32, CTL_GENERIC_GET_MASK);
			sprintf(buffer, "%08lx", number32);
			set_value(conf, "SUBMASK", buffer);
		}
	}

	cntrl_port(ports[count].port, (uint32) & number16, CTL_GENERIC_GET_MTU);
	sprintf(buffer, "%d", number16);
	set_value(conf, "MTU", buffer);

	switch (ports[count].type)
	{
	case L_SER_PTP:
		cntrl_port(ports[count].port, (uint32) & number16, CTL_SERIAL_GET_PRTCL);
		sprintf(buffer, "%d", number16);
		set_value(conf, "PROTO", buffer);
		break;
	case L_PAR_PTP:
		/* Nothing to be handled here */
		break;
	case L_SER_BUS:
		cntrl_port(ports[count].port, (uint32) & sign16, CTL_ETHER_GET_TYPE);
		sprintf(buffer, "%d", sign16);
		set_value(conf, "TYPE", buffer);
		if (cntrl_port(ports[count].port, (uint32) buffer, CTL_ETHER_GET_MAC) >= 0)
		{
			sprintf(buffer, "%02x:%02x:%02x:%02x:%02x:%02x",
				(unsigned char)buffer[0],
				(unsigned char)buffer[1],
				(unsigned char)buffer[2],
				(unsigned char)buffer[3],
				(unsigned char)buffer[4],
				(unsigned char)buffer[5]);
			set_value(conf, "MAC", buffer);
		}
		break;
	case L_PAR_BUS:
	case L_SER_RING:
	case L_PAR_RING:
		/* Not in use yet */
		break;
	case L_MASQUE:
		cntrl_port(ports[count].port, (uint32) & number32, CTL_MASQUE_GET_PORT);
		if (number32)
		{
			set_value(conf, "MASKPORT", ((PORT *) number32)->name);
		}
		cntrl_port(ports[count].port, (uint32) & number32, CTL_MASQUE_GET_MASKIP);
		if (number32)
		{
			sprintf(buffer, "%08lx", number32);
			set_value(conf, "MASKIP", buffer);
		}
		break;
	}

	ports[count].changed[0] = '\0';
}


static void get_sting_data(void)
{
	int count;

	for (count = 0; count < port_num; count++)
		get_port_data(count);
}


static uint32 hextoul(const char *string)
{
	uint32 number = 0;
	char charac;
	char digit;

	for (;;)
	{
		charac = *string >= 'a' ? *string - ('a' - 'A') : *string;
		string++;
		if (('0' <= charac && charac <= '9') || ('A' <= charac && charac <= 'F'))
		{
			digit = charac - '0';
			number = 16 * number + (digit <= 9 ? digit : digit - 7);
		} else
			break;
	}

	return number;
}


static int set_sting_data(int alert_flag, int boot_flag, int act_port)
{
	uint32 number32;
	uint16 number16;
	uint16 max16;
	char alert[128];
	int count;
	int cnt;
	const char *conf;
	const char *prev;
	const char *posi;
	char buffer[32];
	int locked;
	
	locked = 0;
	for (count = 0; count < port_num; count++)
	{
		if (boot_flag)
		{
			conf = ports[count].settings;
			prev = "";
		} else
		{
			conf = ports[count].changed;
			prev = ports[count].settings;
		}

		if (*conf == '\0' || strcmp(conf, prev) == 0)
			continue;

		if ((posi = chk_value(conf, prev, "IP")) != NULL)
		{
			if (hextoul(posi))
				if (cntrl_port(ports[count].port, hextoul(posi), CTL_GENERIC_SET_IP) == E_LOCKED)
					if (count == act_port)
						locked = 1;
		}
		if ((posi = chk_value(conf, prev, "SUBMASK")) != NULL)
		{
			if (hextoul(posi))
				if (cntrl_port(ports[count].port, hextoul(posi), CTL_GENERIC_SET_MASK) == E_LOCKED)
					if (count == act_port)
						locked = 1;
		}
		if ((posi = chk_value(conf, prev, "MTU")) != NULL)
		{
			number16 = atoi(posi);
			if (number16)
			{
				cntrl_port(ports[count].port, (uint32) &max16, CTL_GENERIC_GET_MMTU);
				if (number16 < 68)
					number16 = 68;
				if (number16 > max16)
					number16 = max16;
				cntrl_port(ports[count].port, number16, CTL_GENERIC_SET_MTU);
			}
		}

		switch (ports[count].type)
		{
		case L_SER_PTP:
			if ((posi = chk_value(conf, prev, "PROTO")) != NULL)
			{
				cntrl_port(ports[count].port, atoi(posi), CTL_SERIAL_SET_PRTCL);
			}
			break;
		case L_PAR_PTP:
			/* Nothing to be handled here */
			break;
		case L_SER_BUS:
			if ((posi = chk_value(conf, prev, "TYPE")) != NULL)
			{
				if (atoi(posi) >= 0)
					cntrl_port(ports[count].port, atoi(posi), CTL_ETHER_SET_TYPE);
				else
					break;
			}
			if ((posi = chk_value(conf, prev, "MAC")) != NULL)
			{
				for (cnt = 0; cnt < 6; cnt++)
				{
					buffer[cnt] = (uint8) hextoul(posi);
					posi += 3;
				}
				cnt = cntrl_port(ports[count].port, (uint32) &buffer[0], CTL_ETHER_SET_MAC);
				if (cnt == E_LOCKED && count == act_port)
					locked = 1;
				if (cnt != E_NORMAL)
				{
					if (alert_flag == TRUE)
					{
						sprintf(alert, rs_frstr[NO_MAC_SET], ports[count].port);
						form_alert(1, alert);
					}
				}
			}
			break;
		case L_PAR_BUS:
		case L_SER_RING:
		case L_PAR_RING:
			/* Not in use yet */
			break;
		case L_MASQUE:
			if ((posi = chk_value(conf, prev, "MASKPORT")) != NULL)
			{
				for (cnt = 0; cnt < 31 && posi[cnt] != '\0' && posi[cnt] != ' '; cnt++)
					buffer[cnt] = posi[cnt] == '~' ? ' ' : posi[cnt];
				buffer[cnt] = '\0';
				number32 = 0;
				cntrl_port(&buffer[0], (uint32) & number32, CTL_KERN_FIND_PORT);
				cntrl_port(ports[count].port, number32, CTL_MASQUE_SET_PORT);
			}
			if ((posi = chk_value(conf, prev, "MASKIP")) != NULL)
			{
				cntrl_port(ports[count].port, hextoul(posi), CTL_MASQUE_SET_MASKIP);
			}
			break;
		}

		if (locked && count == act_port)
			get_port_data(count);

		if ((posi = chk_value(conf, prev, "ACTIVE")) != NULL)
		{
			if (*posi == '1')
			{
				if (query_port(ports[count].port) == FALSE)
				{
					if (on_port(ports[count].port) == FALSE)
					{
						if (alert_flag)
						{
							sprintf(alert, rs_frstr[NO_ACT], ports[count].port);
							form_alert(1, alert);
						}
					}
				}
			} else
			{
				if (query_port(ports[count].port) != FALSE)
					off_port(ports[count].port);
			}
		}
	}
	return locked;
}


static void fetch_line(int handle, char *string)
{
	int count;
	char charac;

	for (count = 0; Fread(handle, 1, &charac) > 0 && count < 126;)
	{
		if (charac == '\n' || charac == '\r')
		{
			if (count > 0)
				break;
		} else
			string[count++] = charac;
	}

	if (count == 126)
	{
		do
		{
			if (charac == '\n' || charac == '\r')
				break;
		} while (Fread(handle, 1, &charac) > 0);
	}

	string[count] = '\0';
}


static void load_config_file(void)
{
	long error;
	int handle;
	long length;
	int count;
	char buffer[128];

	if ((error = Fopen(inf_file, FO_READ)) < 0)
		return;
	handle = (int) error;

	length = Fseek(0, handle, 2);
	Fseek(42, handle, 0);

	while (Fseek(0, handle, 1) < length)
	{
		fetch_line(handle, buffer);
		if (strncmp(buffer, "Port : ", 7) != 0)
			continue;
		for (count = 0; count < port_num; count++)
		{
			if (strcmp(&buffer[7], ports[count].port) == 0)
			{
				ports[count].changed[0] = '\0';
				fetch_line(handle, ports[count].settings);
				break;
			}
		}
		if (count == port_num)
			fetch_line(handle, buffer);
	}

	Fclose(handle);
}


static void save_config_file(void)
{
	long error;
	_WORD button;
	int handle;
	int count;
	char *work;
	char file[32];
	char buffer[128];

	strcpy(buffer, inf_file);

	if ((work = strrchr(buffer, '\\')) != NULL)
	{
		strncpy(file, work + 1, 32);
		file[31] = '\0';
		strcpy(work + 1, "*.*");
	} else
	{
		strcpy(file, "STING.PRT");
		strcpy(buffer, " :\\*.*");
		buffer[0] = 'A' + Dgetdrv();
	}

	if (aes_global[0] >= 0x0140)
	{
		if (fsel_exinput(buffer, file, &button, "Choose Setup File") == 0)
			return;
	} else
	{
		if (fsel_input(buffer, file, &button) == 0)
			return;
	}
	if (button == 0)
		return;

	if ((work = strrchr(buffer, '\\')) != NULL)
		strcpy(work + 1, file);
	else
		strcat(buffer, file);

	if ((error = Fcreate(buffer, 0)) < 0)
	{
		(*params->XGen_Alert) (FILE_ERR);
		return;
	}
	handle = (int) error;

	(*params->CPX_Save) (buffer, sizeof(inf_file));

	Fwrite(handle, 42, "#\r\n#  Do not edit this file by hand !!!\r\n#");

	for (count = 0; count < port_num; count++)
	{
		Fwrite(handle, 11, "\r\n\r\nPort : ");
		Fwrite(handle, strlen(ports[count].port), ports[count].port);
		Fwrite(handle, 2, "\r\n");
		Fwrite(handle, strlen(ports[count].settings), ports[count].settings);
	}

	Fwrite(handle, 10, "\r\n\r\nEnd.\r\n");
	Fclose(handle);
}


static void get_rsc_data(int act_port)
{
	uint32 addr = 0;
	int count, max;
	char *conf;
	char digit[4] = "---";
	char buffer[32];

	conf = ports[act_port].changed;

	set_value(conf, "ACTIVE", (box[ACTIVE].ob_state & OS_SELECTED) ? "1" : "0");
	reload = (box[ROUTE].ob_state & OS_SELECTED) ? TRUE : FALSE;

	if (strlen(box[IP_ADDR].ob_spec.tedinfo->te_ptext) > 9)
	{
		for (count = 0; count < 12; count += 3)
		{
			strncpy(digit, box[IP_ADDR].ob_spec.tedinfo->te_ptext + count, 3);
			addr = (addr << 8) | atol(digit);
		}
		sprintf(buffer, "%08lx", addr);
		set_value(conf, "IP", buffer);
	} else
	{
		set_value(conf, "IP", "");
	}

	if (strlen(box[SUBNET].ob_spec.tedinfo->te_ptext) > 9)
	{
		for (count = 0; count < 12; count += 3)
		{
			strncpy(digit, box[SUBNET].ob_spec.tedinfo->te_ptext + count, 3);
			addr = (addr << 8) | atol(digit);
		}
		sprintf(buffer, "%08lx", addr);
		set_value(conf, "SUBMASK", buffer);
	} else
	{
		set_value(conf, "SUBMASK", "");
	}

	if ((count = atoi(box[MTU].ob_spec.tedinfo->te_ptext)) < 68)
		count = 68;
	cntrl_port(ports[act_port].port, (uint32) & max, CTL_GENERIC_GET_MMTU);
	if (count > max)
		count = max;
	sprintf(buffer, "%d", count);
	set_value(conf, "MTU", buffer);

	switch (ports[act_port].type)
	{
	case L_SER_PTP:
		count = 0;
		count |= (box[PP_PPP].ob_state & OS_SELECTED) ? FLG_PRTCL : 0;
		count |= (box[PP_VJHC].ob_state & OS_SELECTED) ? FLG_VJHC : 0;
		count |= (box[PP_LAN].ob_state & OS_SELECTED) ? FLG_LANBIT : 0;
		sprintf(buffer, "%d", count);
		set_value(conf, "PROTO", buffer);
		break;
	case L_PAR_PTP:
		/* Nothing to be handled here */
		break;
	case L_SER_BUS:
		if (hardware < 0 || hard_num <= hardware)
			hardware = -1;
		sprintf(buffer, "%d", hardware);
		set_value(conf, "TYPE", buffer);
		if (hardware < 0)
			break;
		if (strlen(box[SBL_MAC].ob_spec.tedinfo->te_ptext) > 11)
		{
			for (count = 0; count < 6; count++)
			{
				strncpy(&buffer[3 * count], box[SBL_MAC].ob_spec.tedinfo->te_ptext + 2 * count, 2);
				buffer[3 * count + 2] = ':';
			}
			buffer[17] = '\0';
			set_value(conf, "MAC", buffer);
		}
		break;
	case L_PAR_BUS:
	case L_SER_RING:
	case L_PAR_RING:
		/* Not in use yet */
		break;
	case L_MASQUE:
		if (0 <= masque && masque < mask_num)
		{
			set_value(conf, "MASKPORT", mask[masque]->port);
		}
		if (strlen(box[M_IP].ob_spec.tedinfo->te_ptext) > 9)
		{
			for (count = 0; count < 12; count += 3)
			{
				strncpy(digit, box[M_IP].ob_spec.tedinfo->te_ptext + count, 3);
				addr = (addr << 8) | atol(digit);
			}
			sprintf(buffer, "%08lx", addr);
			set_value(conf, "MASKIP", buffer);
		} else
			set_value(conf, "MASKIP", "");
		break;
	}
}


static void set_rsc_data(int act_port)
{
	uint32 addr;
	int count, max;
	char buffer[32];
	char **ware;
	char *posi;
	const char *format = "%-3d%-3d%-3d%-d";

	strncpy(box[PNAME].ob_spec.free_string, ports[act_port].port, 12);
	strncpy(box[TYPE].ob_spec.tedinfo->te_ptext, type_name[ports[act_port].type], 28);

	if (*get_value(&ports[act_port], "ACTIVE") == '1')
		box[ACTIVE].ob_state |= OS_SELECTED;
	else
		box[ACTIVE].ob_state &= ~OS_SELECTED;

	if (reload)
		box[ROUTE].ob_state |= OS_SELECTED;
	else
		box[ROUTE].ob_state &= ~OS_SELECTED;

	if ((posi = get_value(&ports[act_port], "IP")) == NULL)
	{
		buffer[0] = '\0';
	} else
	{
		addr = hextoul(posi);
		sprintf(buffer, format, (int) ((addr >> 24) & 0xff), (int) ((addr >> 16) & 0xff), (int) ((addr >> 8) & 0xff),
				(int) (addr & 0xff));
		if (addr == 0L || addr == 0xffffffffUL)
			buffer[0] = '\0';
	}
	strncpy(box[IP_ADDR].ob_spec.tedinfo->te_ptext, buffer, box[IP_ADDR].ob_spec.tedinfo->te_txtlen - 1);

	if ((posi = get_value(&ports[act_port], "SUBMASK")) == NULL)
	{
		buffer[0] = '\0';
	} else
	{
		addr = hextoul(posi);
		sprintf(buffer, format, (int) ((addr >> 24) & 0xff), (int) ((addr >> 16) & 0xff), (int) ((addr >> 8) & 0xff),
				(int) (addr & 0xff));
		if (addr == 0L || addr == 0xffffffffUL)
			buffer[0] = '\0';
	}
	strncpy(box[SUBNET].ob_spec.tedinfo->te_ptext, buffer, box[SUBNET].ob_spec.tedinfo->te_txtlen - 1);

	if ((posi = get_value(&ports[act_port], "MTU")) == NULL)
	{
		buffer[0] = '\0';
	} else
	{
		addr = atol(posi);
		sprintf(buffer, "%u", (unsigned int) addr);
	}
	strncpy(box[MTU].ob_spec.tedinfo->te_ptext, buffer, box[MTU].ob_spec.tedinfo->te_txtlen - 1);

	if (ports[act_port].type == L_SER_PTP || ports[act_port].type == L_PAR_PTP)
	{
		box[SUBNET].ob_state |= OS_DISABLED;
		if (box_mode == 0)
			box[SUBNET].ob_flags &= ~OF_EDITABLE;
	} else
	{
		box[SUBNET].ob_state &= ~OS_DISABLED;
		if (box_mode == 0)
			box[SUBNET].ob_flags |= OF_EDITABLE;
	}

	switch (ports[act_port].type)
	{
	case L_SER_PTP:
		if ((posi = get_value(&ports[act_port], "PROTO")) == NULL)
			count = 0;
		else
			count = atoi(posi);
		if ((count & FLG_PRTCL) != 0)
		{
			box[PP_SLIP].ob_state &= ~OS_SELECTED;
			box[PP_PPP].ob_state |= OS_SELECTED;
		} else
		{
			box[PP_SLIP].ob_state |= OS_SELECTED;
			box[PP_PPP].ob_state &= ~OS_SELECTED;
		}
		if ((count & FLG_VJHC) != 0)
			box[PP_VJHC].ob_state |= OS_SELECTED;
		else
			box[PP_VJHC].ob_state &= ~OS_SELECTED;
		if (strcmp(ports[act_port].port, "Ser.2/LAN") == 0)
		{
			if ((count & FLG_LANBIT) != 0)
				box[PP_LAN].ob_state |= OS_SELECTED;
			else
				box[PP_LAN].ob_state &= ~OS_SELECTED;
			box[PP_LAN].ob_state &= ~OS_DISABLED;
			box[PP_N_LAN].ob_state &= ~OS_DISABLED;
		} else
		{
			box[PP_LAN].ob_state &= ~OS_SELECTED;
			box[PP_LAN].ob_state |= OS_DISABLED;
			box[PP_N_LAN].ob_state |= OS_DISABLED;
		}
		break;
	case L_PAR_PTP:
		/* Nothing to be handled here */
		break;
	case L_SER_BUS:
		ware = NULL;
		cntrl_port(ports[act_port].port, (uint32) & ware, CTL_ETHER_INQ_SUPPTYPE);
		if (ware != NULL)
		{
			for (count = max = 0; ware[count]; count++)
				if (max < (int)strlen(ware[count]))
					max = (int)strlen(ware[count]);
			for (count = 0; ware[count]; count++)
			{
				hard[count] = &hard_buff[count * (max + 4)];
				memset(hard[count], ' ', max + 3);
				hard[count][max + 3] = '\0';
				memcpy(&hard[count][2], ware[count], strlen(ware[count]));
			}
			hard_num = count;
			if ((posi = get_value(&ports[act_port], "TYPE")) == NULL)
			{
				hardware = -1;
			} else
			{
				hardware = atoi(posi);
				if (hardware < 0 || hard_num <= hardware)
					hardware = -1;
			}
			posi = box[SBL_HARD].ob_spec.free_string;
			strncpy(posi, hardware < 0 ? "Unset" : &hard[hardware][2], 17);
			posi += 16;
			while (*posi == ' ')
				*posi-- = '\0';
		} else
		{
			strcpy(hard_buff, "  Unknown ");
			hard[0] = &hard_buff[0];
			hardware = -1;
			hard_num = 0;
			strcpy(box[SBL_HARD].ob_spec.free_string, "Unknown");
		}
		buffer[0] = '\0';
		if ((posi = get_value(&ports[act_port], "MAC")) != NULL)
		{
			for (count = 0; count < 6; count++)
				strncpy(&buffer[2 * count], &posi[3 * count], 2);
			buffer[12] = '\0';
		} else if (cntrl_port(ports[act_port].port, (uint32) buffer, CTL_ETHER_GET_MAC) >= 0)
		{
			sprintf(buffer, "%02x:%02x:%02x:%02x:%02x:%02x",
				(unsigned char)buffer[0],
				(unsigned char)buffer[1],
				(unsigned char)buffer[2],
				(unsigned char)buffer[3],
				(unsigned char)buffer[4],
				(unsigned char)buffer[5]);
		}
		strcpy(box[SBL_MAC].ob_spec.tedinfo->te_ptext, buffer);
		break;
	case L_PAR_BUS:
	case L_SER_RING:
	case L_PAR_RING:
		/* Not in use yet */
		break;
	case L_MASQUE:
		if ((posi = get_value(&ports[act_port], "MASKPORT")) == NULL)
		{
			masque = -1;
			strcpy(box[M_PORT].ob_spec.free_string, "Unset");
		} else
		{
			for (count = 0; count < 31 && posi[count] != '\0' && posi[count] != ' '; count++)
				buffer[count] = posi[count] == '~' ? ' ' : posi[count];
			buffer[count] = '\0';
			for (count = 0; count < mask_num; count++)
				if (strcmp(buffer, mask[count]->port) == 0)
					break;
			masque = count == mask_num ? -1 : count;
			strcpy(box[M_PORT].ob_spec.free_string, masque < 0 ? "Unset" : buffer);
		}
		if ((posi = get_value(&ports[act_port], "MASKIP")) == NULL)
			buffer[0] = '\0';
		else
		{
			addr = hextoul(posi);
			sprintf(buffer, format, (int) ((addr >> 24) & 0xff), (int) ((addr >> 16) & 0xff),
					(int) ((addr >> 8) & 0xff), (int) (addr & 0xff));
			if (addr == 0L || addr == 0xffffffffUL)
				buffer[0] = '\0';
		}
		strncpy(box[M_IP].ob_spec.tedinfo->te_ptext, buffer, box[M_IP].ob_spec.tedinfo->te_txtlen - 1);
		break;
	}
}


static _WORD cdecl cpx_call(GRECT *wind)
{
	GRECT rect;
	_WORD button;
	_WORD count;
	_WORD def_edit;
	_WORD old_this;
	_WORD type = BOX_ADDR;
	int abort_flg = FALSE;
	_WORD msg_buff[8];
	_WORD work_in[11];
	_WORD work_out[57];
	char *walk;

	graf_mouse(ARROW, NULL);

	for (count = 0; count < 10; count++)
		work_in[count] = 1;
	work_in[10] = 2;

	vdi_handle = params->handle;
	v_opnvwk(work_in, &vdi_handle, work_out);

	box[ROOT].ob_x = wind->g_x;
	box[ROOT].ob_y = wind->g_y;
	objc_draw(box, ROOT, MAX_DEPTH, wind->g_x, wind->g_y, wind->g_w, wind->g_h);

	do
	{
		def_edit = box_mode ? edit_box[ports[act_port].type] : IP_ADDR;
		button = (*params->Xform_do) (box, def_edit, msg_buff);
		if (button > 0)
		{
			objc_offset(box, button &= 0x7fff, &rect.g_x, &rect.g_y);
			rect.g_w = box[button].ob_width;
			rect.g_h = box[button].ob_height;
		}
		switch (button)
		{
		case SAVE:
			if ((*params->XGen_Alert) (SAVE_DEFAULTS))
			{
				get_rsc_data(act_port);
				set_sting_data(TRUE, FALSE, act_port);
				get_sting_data();
				save_config_file();
				set_rsc_data(act_port);
				objc_draw(box, box_mode ? type_box[ports[act_port].type] : BOX_ADDR,
						  MAX_DEPTH, wind->g_x, wind->g_y, wind->g_w, wind->g_h);
			}
			break;
		case SET:
			get_rsc_data(act_port);
			if (set_sting_data(TRUE, FALSE, act_port))
				set_rsc_data(act_port);
			if (reload)
			{
				if ((type = load_routing_table()) != 0)
				{
					form_alert(1, type == E_NODATA ?
							   "[1][ Error :| |   Cannot open file ROUTE.TAB !   ][ Hmmm ]" :
							   "[1][ Error :| |   Out of internal memory !   ][ Hmmm ]");
				}
			}
			abort_flg = TRUE;
			break;
		case CANCEL:
			abort_flg = TRUE;
			break;
		case MESSAGE:
			if (msg_buff[0] == WM_CLOSED)
			{
				get_rsc_data(act_port);
				set_sting_data(FALSE, FALSE, act_port);
				abort_flg = TRUE;
			}
			if (msg_buff[0] == AC_CLOSE)
				abort_flg = TRUE;
			break;
		case MODE:
			old_this = box_mode;
			box_mode = (*params->Popup) (mode_name, 2, old_this, IBM, &rect, wind);
			if (box_mode < 0 || 1 < box_mode)
				box_mode = old_this;
			if (old_this != box_mode)
			{
				type = type_box[ports[act_port].type];
				if (old_this == 0 || edit_box[ports[act_port].type] != 0)
					parse_tree(box, old_this ? type : BOX_ADDR, 0);
				get_rsc_data(act_port);
				box[old_this ? type : BOX_ADDR].ob_flags |= OF_HIDETREE;
				if (box_mode == 0 || edit_box[ports[act_port].type] != 0)
					parse_tree(box, box_mode ? type : BOX_ADDR, 1);
				set_rsc_data(act_port);
				box[box_mode ? type : BOX_ADDR].ob_flags &= ~OF_HIDETREE;
				objc_draw(box, box_mode ? type : BOX_ADDR, MAX_DEPTH, wind->g_x, wind->g_y, wind->g_w, wind->g_h);
				strcpy(box[MODE].ob_spec.free_string, mode_name[box_mode] + 2);
				*strchr(box[MODE].ob_spec.free_string, ' ') = '\0';
			}
			break;
		case PNAME:
			old_this = act_port;
			act_port = (*params->Popup) ((const char *const *)port_name, port_num, old_this, IBM, &rect, wind);
			if (act_port < 0 || port_num <= act_port)
				act_port = old_this;
			if (old_this != act_port)
			{
				if (box_mode)
				{
					box[type = type_box[ports[old_this].type]].ob_flags |= OF_HIDETREE;
					if (edit_box[ports[old_this].type] != 0)
						parse_tree(box, type, 0);
					box[type = type_box[ports[act_port].type]].ob_flags &= ~OF_HIDETREE;
					if (edit_box[ports[act_port].type] != 0)
						parse_tree(box, type, 1);
				}
				get_rsc_data(old_this);
				set_rsc_data(act_port);
				objc_draw(box, box_mode ? type : BOX_ADDR, MAX_DEPTH, wind->g_x, wind->g_y, wind->g_w, wind->g_h);
				objc_draw(box, ACTIVE, 1, wind->g_x, wind->g_y, wind->g_w, wind->g_h);
				objc_draw(box, TYPE, 1, wind->g_x, wind->g_y, wind->g_w, wind->g_h);
			}
			break;
		case SBL_HARD:
			type = hard_num == 0 ? 1 : hard_num;
			old_this = hardware;
			count = (*params->Popup) ((const char *const *)hard, type, old_this, IBM, &rect, wind);
			if (0 <= count && count < hard_num)
			{
				hardware = count;
				strncpy(walk = box[SBL_HARD].ob_spec.free_string, &hard[count][2], 17);
				walk += 16;
				while (*walk == ' ')
					*walk-- = '\0';
			}
			break;
		case M_PORT:
			old_this = masque;
			count = (*params->Popup) ((const char *const *)mask_name, mask_num, old_this, IBM, &rect, wind);
			if (0 <= count && count < mask_num)
			{
				masque = count;
				strcpy(box[M_PORT].ob_spec.free_string, mask[count]->port);
			}
		}
		if (button > 0)
		{
			box[button].ob_state &= ~OS_SELECTED;
			objc_draw(box, button, 3, wind->g_x, wind->g_y, wind->g_w, wind->g_h);
		}
	} while (!abort_flg);

	v_clsvwk(vdi_handle);

	return FALSE;
}


static _WORD cdecl my_button_handler(PARMBLK *parameter)
{
	_WORD clip[4];
	_WORD pxy[4];
	_WORD pos_x, pos_y;
	_WORD radius;

	clip[0] = parameter->pb_xc;
	clip[2] = clip[0] + parameter->pb_wc - 1;
	clip[1] = parameter->pb_yc;
	clip[3] = clip[1] + parameter->pb_hc - 1;
	vs_clip(vdi_handle, 1, clip);

	radius = (parameter->pb_w + parameter->pb_h) / 6;
	pos_x = parameter->pb_x + parameter->pb_w / 2;
	pos_y = parameter->pb_y + parameter->pb_h / 2;

	vsf_interior(vdi_handle, FIS_HOLLOW);

	if (parameter->pb_tree[parameter->pb_obj].ob_flags & OF_RBUTTON)
	{
		v_circle(vdi_handle, pos_x, pos_y, radius);

		if (parameter->pb_currstate & OS_SELECTED)
		{
			vsf_interior(vdi_handle, FIS_SOLID);
			v_circle(vdi_handle, pos_x, pos_y, radius / 2);
		}
	} else
	{
		pxy[0] = pos_x - radius;
		pxy[2] = pos_x + radius;
		pxy[1] = pos_y - radius;
		pxy[3] = pos_y + radius;
		v_bar(vdi_handle, pxy);

		if (parameter->pb_currstate & OS_SELECTED)
		{
			pxy[0] += 2;
			pxy[1] += 2;
			pxy[2] -= 2;
			pxy[3] -= 2;
			v_pline(vdi_handle, 2, pxy);
			radius = pxy[1];
			pxy[1] = pxy[3];
			pxy[3] = radius;
			v_pline(vdi_handle, 2, pxy);
		}
	}

	vs_clip(vdi_handle, 0, clip);

	return parameter->pb_currstate & ~OS_SELECTED;
}

static USERBLK my_user_block = { my_button_handler, 0 };

static CPXINFO fkts = {
	cpx_call, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL
};


CPXINFO *cdecl cpx_init(XCPB *para)
{
	PNTA pnta;
	char pnta_buffer[32];
	int result;
	int type;
	int count;
	int len;
	int max_len = 0;
	DRV_LIST *sting_drivers;

	params = para;

	if ((*params->getcookie) (STIK_COOKIE_MAGIC, (long *) &sting_drivers) == 0)
		return NULL;

	if (sting_drivers == 0)
		return NULL;

	if (strcmp(sting_drivers->magic, STIK_DRVR_MAGIC) != 0)
		return NULL;

	tpl = (TPL *) (*sting_drivers->get_dftab) (TRANSPORT_DRIVER);
	stx = (STX *) (*sting_drivers->get_dftab) (MODULE_DRIVER);

	if (tpl == NULL || stx == NULL)
		return NULL;

	appl_init();
	
	pnta.port_name = pnta_buffer;
	pnta.name_len = 32;
	port_num = mask_num = 0;
	result = cntrl_port(NULL, (uint32) & pnta, CTL_KERN_FIRST_PORT);

	while (result != E_NODATA)
	{
		cntrl_port(pnta.port_name, (uint32) & type, CTL_GENERIC_GET_TYPE);
		if (type != 0)
		{
			if (port_num < 16)
			{
				strncpy(ports[port_num].port, pnta.port_name, 32);
				ports[port_num].port[31] = '\0';
				strcpy(popup[port_num], "  ");
				strncpy(&popup[port_num][2], pnta.port_name, 14);
				popup[port_num][15] = '\0';
				if (max_len < (len = (int)strlen(popup[port_num])))
					max_len = len;
				if (type != L_MASQUE)
				{
					mask_name[mask_num] = popup[port_num];
					mask[mask_num++] = &ports[port_num];
				}
				port_name[port_num] = popup[port_num];
				ports[port_num++].type = type;
			}
		}
		result = cntrl_port(NULL, (uint32) & pnta, CTL_KERN_NEXT_PORT);
	}

	for (count = 0; count < port_num; count++)
	{
		len = (int)strlen(popup[count]);
		strncpy(&popup[count][len], "                ", max_len + 3 - len);
		popup[count][max_len + 3] = '\0';
		ports[count].settings[0] = ports[count].changed[0] = '\0';
	}

	if (params->booting)
	{
		if (inf_file[0] != '\0' && *getvstr("CONFSTING") == '0')
		{
			load_config_file();
			set_sting_data(FALSE, TRUE, -1);
		}
		return (CPXINFO *)1;
	}

	if (!params->SkipRshFix)
	{
		box = rs_trindex[STING];

		for (count = 0; count < NUM_OBS; count++)
		{
			params->rsh_obfix(box, count);
			if (box[count].ob_type & 0x7f00)
				if ((box[count].ob_state & CROS_CHK) == CROS_CHK)
				{
					box[count].ob_type = G_USERDEF;
					box[count].ob_spec.userblk = &my_user_block;
					box[count].ob_state &= ~CROS_CHK;
				}
		}
		parse_tree(box, ROOT, -1);

		for (count = 1; count < 8; count++)
		{
			box[type_box[count]].ob_flags |= OF_HIDETREE;
			parse_tree(box, type_box[count], 0);
		}
		box_mode = 0;

		strcpy(box[MODE].ob_spec.free_string, mode_name[box_mode] + 2);
		*strchr(box[MODE].ob_spec.free_string, ' ') = '\0';
		strncpy(box[VERSION].ob_spec.free_string + 7, tpl->version, 5);
		reload = FALSE;

		get_sting_data();
		set_rsc_data(act_port = 0);
	}

	return &fkts;
}
