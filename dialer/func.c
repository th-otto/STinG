/*********************************************************************/
/*                                                                   */
/*     STinG : Modem Dialer, Core related Functions                  */
/*                                                                   */
/*                                                                   */
/*      Version 1.2                        from 23. August 1998      */
/*                                                                   */
/*      Module for Functions that communicate with the STinG core    */
/*                                                                   */
/*********************************************************************/


#include <aes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "device/device.h"

#include "dialer.h"

#include "transprt.h"
#include "port.h"
#include "layer.h"
#include "window.h"


#if __GNUC_PREREQ(8, 1)
#pragma GCC diagnostic ignored "-Wstringop-truncation"
#endif


typedef struct port_list
{
	DEV_LIST *device;
	char port[32];
} ASSOC;


TPL *tpl;
STX *stx;
DEV_LIST *devices;
DEV_LIST *curr_port;
const char *ports[21];
char eff_passwd[32];
int masque_there;

static const char *all_ports[20];
static STING_CONFIG *conf_block;
static LAYER *all_layers[20];
static ASSOC asso[20];
static int num_asso;
static int num_sport;
static int num_slayer;
static int ppp_errlist = 1;
static uint32 logging[2];
static char tcpip_version[6];
static char const masq[] = "Masquerade";

static char ppp_errors[][30] = { "No PPP problem" };

static char const failed[] = "[1][  Passing connection to STinG  | |     failed !][ Hangup ]";
static char const no_reopen[] = "[1][  Reopening port failed !   | |  Hangup manually !][ Disable ]";
static char const access[] = "[3][  Other programs already access  | |  this port !][ Okay ]";
static char const no_lookup[] = "[1][  Internal problem looking up  | |  this port !][ Hmmm ]";



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


int get_version(char *sting_version)
{
	DRV_LIST *sting_drivers;

	sting_drivers = (DRV_LIST *) Supexec(get_sting_cookie);

	if (sting_drivers == NULL)
		return 2;
	if (strcmp(sting_drivers->magic, STIK_DRVR_MAGIC) != 0)
		return 2;

	conf_block = sting_drivers->cfg.sting;

	tpl = (TPL *) (*sting_drivers->get_dftab) (TRANSPORT_DRIVER);
	stx = (STX *) (*sting_drivers->get_dftab) (MODULE_DRIVER);

	if (tpl == (TPL *) NULL || stx == (STX *) NULL)
		return 1;

	strncpy(sting_version, tpl->version, 5);
	sting_version[5] = '\0';
	strcpy(tcpip_version, sting_version);

	return 0;
}


void query_active(int alert)
{
	if ((set_sysvars(-1, -1) & 0xff0000L) == 0L)
	{
		if (alert)
			form_alert(1, "[3][ |  Note !| |   STinG isn't activated yet !  ][ Aha ]");
	}
}


static DEV_LIST *find_device(const char *name)
{
	DEV_LIST *d_walk;

	for (d_walk = devices; d_walk; d_walk = d_walk->next)
		if (strcmp(d_walk->name, name) == 0)
			return d_walk;

	return NULL;
}


int init_misc(void)
{
	OBJECT *tree;
	PORT *p_walk;
	LAYER *l_walk;
	PNTA pnta;
	int count;
	int index;
	int aux;
	char buffer[32];

	rsrc_gaddr(R_TREE, START, &tree);
	count = tree[ST_BOX].ob_x + tree[ST_STUFF].ob_x + tree[ST_STUFF].ob_width;
	tree[TL_TTL].ob_x = count - tree[TL_TTL].ob_width;

	rsrc_gaddr(R_TREE, PASSWORD, &tree);
	tree[PW_BUTT].ob_width = tree[PW_BUTT].ob_height = 0;

	rsrc_gaddr(R_TREE, CONF, &tree);
	if (planes == 1)
		tree[CN_BIGBX].ob_width++;

	rsrc_gaddr(R_TREE, OT_DOIT, &tree);
	if (planes == 1)
		tree[OT_BIGBX].ob_width++;

	rsrc_gaddr(R_TREE, PU_S_PRT, &tree);

	for (count = 0, p_walk = conf_block->ports; count < 20 && p_walk; p_walk = p_walk->next)
	{
		all_ports[count++] = p_walk->name;
		strncpy(tree[count].ob_spec.free_string + 2, p_walk->name, 10);
	}
	for (num_sport = count++; count < 20; count++)
		tree[count].ob_flags |= OF_HIDETREE;
	tree->ob_height = num_sport * tree[1].ob_height;

	rsrc_gaddr(R_TREE, PU_S_LAY, &tree);

	for (count = 0, l_walk = conf_block->layers; count < 20 && l_walk; l_walk = l_walk->next)
	{
		all_layers[count++] = l_walk;
		strncpy(tree[count].ob_spec.free_string + 2, l_walk->name, 10);
	}
	for (num_slayer = count++; count < 20; count++)
		tree[count].ob_flags |= OF_HIDETREE;
	tree->ob_height = num_slayer * tree[1].ob_height;

	rsrc_gaddr(R_TREE, PU_C_PRT, &tree);
	pnta.name_len = 32;
	pnta.port_name = &buffer[0];
	aux = cntrl_port(NULL, (uint32) & pnta, CTL_KERN_FIRST_PORT);

	for (count = index = 0; count < 20 && aux == E_NORMAL;)
	{
		aux = count;
		if (strcmp(&buffer[0], "Ser.2/LAN") == 0)
		{
			if ((asso[count].device = find_device("Serial 2")) != NULL)
				strcpy(asso[count++].port, &buffer[0]);
			if ((asso[count].device = find_device("LAN")) != NULL)
				strcpy(asso[count++].port, &buffer[0]);
		} else
		{
			if ((asso[count].device = find_device(&buffer[0])) != NULL)
				strcpy(asso[count++].port, &buffer[0]);
		}
		if (aux != count)
		{
			for (aux = 0; aux < 20; aux++)
				if (strcmp(&buffer[0], all_ports[aux]) == 0)
				{
					ports[index++] = all_ports[aux];
					break;
				}
			if (aux == 20)
				ports[index++] = "Unknown";
			strncpy(tree[index].ob_spec.free_string + 2, &buffer[0], 10);
		}
		aux = cntrl_port(NULL, (uint32) & pnta, CTL_KERN_NEXT_PORT);
	}
	num_ports = index;
	num_asso = count;

	for (index++; index <= 20; index++)
		tree[index].ob_flags |= OF_HIDETREE;
	tree->ob_height = num_ports * tree[1].ob_height;

	has_LAN = find_device("LAN") != NULL ? TRUE : FALSE;

	if (cntrl_port("Masquerade", (uint32) & p_walk, CTL_KERN_FIND_PORT) == E_NORMAL)
		masque_there = TRUE;
	else
		masque_there = FALSE;

	return num_ports != 0;
}


int en_dis_able(int dtr, int alert)
{
	if (dtr)
	{
		if (OpenDevice(curr_port))
		{
			DtrOn(curr_port);
		} else
		{
			if (alert)
				form_alert(1, access);
			return FALSE;
		}
	} else
	{
		if (off_hook)
		{
			if (alert)
				if (form_alert(2, hangup_alert) == 2)
					return FALSE;
			if (dial_state != S_NONE)
			{
				dial_timer = NULL;
				dial_state = S_NONE;
				close_rsc_window(DIALER, -1);
			}
			hangup();
		}
		if (port_lock)
		{
			DtrOff(curr_port);
			CloseDevice(curr_port);
		}
	}
	port_lock = dtr;

	return TRUE;
}


void set_configuration(void)
{
	int count;
	char ip[32];
	char text[128] = "";

	if (get_port() != 0)
		return;

	if (!OpenDevice(curr_port))
	{
		form_alert(1, access);
		change_flags(START, ST_CNCT, TRUE, 0, OS_DISABLED);
		change_flags(START, ST_ABLE, TRUE, 0, OS_DISABLED);
		return;
	}

	change_flags(START, ST_CNCT, TRUE, 0, OS_DISABLED);
	change_flags(START, ST_ABLE, FALSE, 0, OS_DISABLED);

	DtrOff(curr_port);

	CloseDevice(curr_port);

	for (count = 0; count < dns_num; count++)
	{
		sprintf(ip, ip_out, ip_dns[count][0], ip_dns[count][1], ip_dns[count][2], ip_dns[count][3]);
		strcat(text, ip);
		if (count < dns_num - 1)
			strcat(text, ", ");
	}
	setvstr("NAMESERVER", text);

	setvstr("USERNAME", username);
	setvstr("FULLNAME", fullname);
	setvstr("HOSTNAME", fqdn);

	for (count = 0; count < environ_number; count++)
	{
		text[32] = '\0';
		strncpy(text, environ_base[count], 32);
		if (strchr(text, '='))
			*strchr(text, '=') = '\0';
		setvstr(text, strchr(environ_base[count], '=') + 1);
	}
}


static void get_parameter(char *port)
{
	uint32 number32;
	int16 count;
	const char *str;
	unsigned char *walk;

	cntrl_port(port, (uint32) & number32, CTL_GENERIC_GET_IP);
	walk = (unsigned char *) &number32;
	ip_address[0] = *walk++;
	ip_address[1] = *walk++;
	ip_address[2] = *walk++;
	ip_address[3] = *walk++;

	cntrl_port(port, (uint32) & port_mtu, CTL_GENERIC_GET_MTU);
	cntrl_port(port, (uint32) & port_flags, CTL_SERIAL_GET_PRTCL);

	if (*((str = getvstr("USERNAME")) + 1) != '\0')
	{
		strcpy(username, str);
	}
	if (*((str = getvstr("FULLNAME")) + 1) != '\0')
	{
		strcpy(fullname, str);
	}
	if (*((str = getvstr("HOSTNAME")) + 1) != '\0')
	{
		strcpy(fqdn, str);
	}

	if (*((str = getvstr("NAMESERVER")) + 1) != '\0')
	{
		dns_num = 0;
		for (count = 0; count < 4 && str != (char *) 1L; count++)
		{
			load_ip_addr(ip_dns[dns_num++], str);
			str = strchr(str, ',') + 1;
		}
	}
}


int get_port(void)
{
	OBJECT *tree;
	int count;
	int found = FALSE;
	const char *name;

	rsrc_gaddr(R_TREE, START, &tree);
	name = ports[act_port];

	for (count = 0; count < num_asso; count++)
	{
		if (strcmp(asso[count].port, name) == 0)
		{
			if (strcmp(name, "Ser.2/LAN") == 0)
			{
				if (strcmp(asso[count].device->name, (port_flags & 4) ? "LAN" : "Serial 2") == 0)
					found = TRUE;
				else
					continue;
			} else
			{
				found = TRUE;
			}
			break;
		}
	}
	
	port_lock = FALSE;

	if (!found)
	{
		connected = off_hook = FALSE;
		form_alert(1, no_lookup);
		change_flags(START, ST_CNCT, TRUE, 0, OS_DISABLED);
		change_flags(START, ST_ABLE, TRUE, 0, OS_DISABLED);
		return -1;
	}

	rsrc_gaddr(R_TREE, START, &tree);
	curr_port = asso[count].device;

	strcpy(tree[ST_CNCT].ob_spec.free_string, "Connect");
	strcpy(tree[ST_ABLE].ob_spec.free_string, "Enable");

	if (query_port(name))
	{
		connected = off_hook = TRUE;
		get_parameter(asso[count].port);
		change_freestring(START, MOD_STAT, ST_PBOX, " Online", 8);
		strcpy(tree[ST_CNCT].ob_spec.free_string, "Disconnect");
		strcpy(tree[ST_ABLE].ob_spec.free_string, "Disable");
		change_flags(START, ST_CNCT, FALSE, 0, OS_DISABLED);
		change_flags(START, ST_ABLE, TRUE, 0, OS_DISABLED);
		change_flags(CONF, CC_SET, TRUE, 0, OS_DISABLED);
		change_flags(CONF, CC_SAVE, TRUE, 0, OS_DISABLED);
		return -1;
	}

	change_freestring(START, MOD_STAT, ST_PBOX, " Offline", 8);
	connected = off_hook = FALSE;

	return 0;
}


static int find_default_route(PORT **port_ptr, uint32 *gate_ptr)
{
	PORT *port;
	uint32 net;
	uint32 mask;
	uint32 gate;
	int number;
	int count;

	if ((number = get_route_entry(0, &net, &mask, &port, &gate)) < 1)
	{
		count = -1;
	} else
	{
		for (count = 0; count < number; count++)
		{
			if (get_route_entry(count, &net, &mask, &port, &gate) > 0)
				if (net == 0L && mask == 0L)
					break;
		}
		if (count == number)
			count = -1;
	}

	if (count >= 0)
	{
		if (port_ptr)
			*port_ptr = port;
		if (gate_ptr)
			*gate_ptr = gate;
	}

	return count;
}


int do_connect(void)
{
	PORT *port;
	PORT *masque;
	PORT *deflt;
	uint32 ip;
	uint32 dummy;
	int16 count;
	char *the_port;
	char *pap[2];
	char *file = NULL;
	char line[512];

	if (!connected)
	{
		CloseDevice(curr_port);
		port_lock = FALSE;
		for (count = 0; count < num_asso; count++)
		{
			if (asso[count].device == curr_port)
				break;
		}
		the_port = &asso[count].port[0];

		if (debugging)
		{
			if ((file = Malloc(count = 32765)) == NULL)
				file = Malloc(count = 8190);
			if (file)
			{
				logging[0] = (uint32) file;
				logging[1] = count;
				cntrl_port(the_port, (uint32) & logging[0], CTL_SERIAL_SET_LOGGING);
			}
		}

		if (port_flags & 1)
		{
			pap[0] = pap_id;
			pap[1] = eff_passwd;
			cntrl_port(the_port, (uint32) & pap[0], CTL_SERIAL_SET_PAP);
		}

		cntrl_port (the_port, (uint32) &ip, CTL_GENERIC_GET_IP);
		sprintf(line, "P:%ld", ip);
		setvstr("SAVD_PORT_IP", line);
		for (ip = count = 0; count < 4; count++)
			ip = (ip << 8) | (uint32) ip_address[count];
		sprintf(line, "C:%ld", ip);
		setvstr("SAVD_CONF_IP", line);
		cntrl_port(the_port, (uint32) ip, CTL_GENERIC_SET_IP);
		cntrl_port(the_port, (uint32) port_mtu, CTL_GENERIC_SET_MTU);
		cntrl_port(the_port, (uint32) port_flags, CTL_SERIAL_SET_PRTCL);

		connected = on_port(the_port);

		if (connected)
		{
			cntrl_port(the_port, (uint32) & port, CTL_KERN_FIND_PORT);

			if (query_port(masq) == FALSE)
			{
				if (masquerade)
				{
					if (cntrl_port(masq, (uint32) & masque, CTL_KERN_FIND_PORT) == E_NORMAL)
					{
						if (cntrl_port(masq, (uint32) port, CTL_MASQUE_SET_PORT) >= 0)
							port = masque;
					}
				}
			}

			if (def_route)
			{
				count = find_default_route(&deflt, &ip);
				if (count >= 0)
				{
					sprintf(line, "P:%ld,G:%ld", (uint32) deflt, ip);
					setvstr("SAVD_ROUTE", line);
				}
				set_route_entry(count, 0L, 0L, port, 0L);
			}

			change_freestring(START, MOD_STAT, ST_PBOX, " Online", 8);
			change_freestring(START, ST_CNCT, -1, "Disconnect", 11);
			change_flags(START, ST_CNCT, FALSE, 0, OS_DISABLED);
			change_flags(START, ST_ABLE, TRUE, 0, OS_DISABLED);
			if ((file = strrchr(script_path, '\\') + 1) == (void *) 1L)
				file = script_path;
			sprintf(line, "IP connection established via %s.", file);
			write_time_log(line);

			if (strcmp(port->name, masq) == 0)
				on_port(masq);
		} else
		{
			if (debugging && file != NULL)
			{
				dummy = 0L;
				cntrl_port(the_port, (uint32) & dummy, CTL_SERIAL_SET_LOGGING);
				Mfree(file);
			}

			if (OpenDevice(curr_port))
			{
				hangup();
			} else
			{
				if (sender < 0)
					form_alert(1, no_reopen);
				change_flags(START, ST_CNCT, TRUE, 0, OS_DISABLED);
				change_flags(START, ST_ABLE, TRUE, 0, OS_DISABLED);
				set_mode(port_lock = off_hook = FALSE, FALSE);
			}

			if (sender < 0)
				form_alert(1, failed);
		}
	}

	return connected;
}


int do_disconnect(void)
{
	PORT *port;
	int count;
	const char *route;
	const char *ip;
	uint32 saved_ip;

	if (connected)
	{
		change_freestring(START, MOD_STAT, ST_PBOX, " Offline", 8);
		change_freestring(START, ST_CNCT, -1, "Connect", 8);
		change_flags(START, ST_CNCT, FALSE, 0, OS_DISABLED);
		change_flags(START, ST_ABLE, FALSE, 0, OS_DISABLED);

		connected = FALSE;
		write_time_log("IP link disconnected.");

		if (*(route = getvstr("SAVD_ROUTE")) != '0')
		{
			if (*route == 'P')
			{
				port = (PORT *) atol(route + 2);
				if ((route = strchr(route, 'G')) != NULL)
					if ((count = find_default_route(NULL, NULL)) >= 0)
						set_route_entry(count, 0L, 0L, port, atol(route + 2));
			}
			setvstr("SAVD_ROUTE", "0");
		}

		for (count = 0; count < num_asso; count++)
		{
			if (asso[count].device == curr_port)
				break;
		}
		if (count < num_asso)
		{
			if (query_port(masq))
			{
				cntrl_port(masq, (uint32) & port, CTL_MASQUE_GET_PORT);
				if (strcmp(port->name, asso[count].port) == 0)
					off_port(masq);
			}
			off_port(asso[count].port);
			ip = getvstr("SAVD_PORT_IP");
			if (*ip != '0')
			{
				if (*ip == 'P')
				{
					saved_ip = atol(ip + 2);
					cntrl_port(asso[count].port, saved_ip, CTL_GENERIC_SET_IP);
				}
				setvstr("SAVD_PORT_IP", "0");
			}
			ip = getvstr("SAVD_CONF_IP");
			if (*ip != '0')
			{
				if (*ip == 'C')
				{
					saved_ip = atol(ip + 2);
					ip_address[0] = (unsigned char)(saved_ip >> 24);
					ip_address[1] = (unsigned char)(saved_ip >> 16);
					ip_address[2] = (unsigned char)(saved_ip >> 8);
					ip_address[3] = (unsigned char)(saved_ip);
				}
				setvstr("SAVD_CONF_IP", "0");
			}
		}
		port_lock = TRUE;

		if (!OpenDevice(curr_port))
		{
			if (sender < 0)
				form_alert(1, no_reopen);
			set_mode(port_lock = off_hook = FALSE, FALSE);
			return FALSE;
		}
	}

	return TRUE;
}


void read_counter(long *recvd_counter, long *sent_counter)
{
	int16 count;
	uint32 array[3];

	for (count = 0; count < num_asso; count++)
	{
		if (curr_port == asso[count].device)
			break;
	}

	if (count < num_asso)
	{
		cntrl_port(asso[count].port, (uint32) & array[0], CTL_GENERIC_GET_STAT);
		*sent_counter = array[1];
		*recvd_counter = array[2];
	} else
	{
		*recvd_counter = *sent_counter = 0;
	}
}


int check_port_flags(uint32 *ip_ptr)
{
	int16 count;
	int16 state;

	for (count = 0; count < num_asso; count++)
	{
		if (curr_port == asso[count].device)
		{
			state = 0;
			cntrl_port(asso[count].port, (uint32) & state, CTL_SERIAL_INQ_STATE);
			if (state)
			{
				cntrl_port(asso[count].port, (uint32) ip_ptr, CTL_GENERIC_GET_IP);
				return state;
			} else
			{
				return 0;
			}
		}
	}
	
	return -1;
}


char *get_PPP_status(void)
{
	int16 count;
	int16 state;

	for (count = 0; count < num_asso; count++)
		if (curr_port == asso[count].device)
		{
			state = 1;
			cntrl_port(asso[count].port, (uint32) & state, CTL_SERIAL_INQ_STATE);
			if (state < 0 || ppp_errlist <= state)
				return NULL;
			else
				return &ppp_errors[state][0];
		}

	return NULL;
}


void finish_login(void)
{
	uint32 length = 0L;
	int count;
	int handle;
	char log_path[256];
	char *pap[2] = { NULL, NULL };

	for (count = 0; count < num_asso; count++)
		if (curr_port == asso[count].device)
			break;

	if (count == num_asso)
		return;

	if (port_flags & 1)
	{
		cntrl_port(asso[count].port, (uint32) & pap[0], CTL_SERIAL_SET_PAP);
	}

	if (debugging == FALSE)
		return;

	cntrl_port(asso[count].port, (uint32) & length, CTL_SERIAL_SET_LOGGING);

	strcpy(log_path, config_path);

	if (strrchr(log_path, '\\'))
		strcpy(strrchr(log_path, '\\') + 1, "DEBUG.LOG");
	else
		strcat(log_path, "DEBUG.LOG");

	handle = (int) Fcreate(log_path, 0);

	if (handle >= 0)
	{
		Fwrite(handle, length, (char *) logging[0]);
		Fclose(handle);
	}

	Mfree((void *) logging[0]);
}


static void do_statistics(int redraw_flag)
{
	int32 array[3];

	cntrl_port(all_ports[stat_port], (uint32) & array[0], CTL_GENERIC_GET_STAT);

	set_stat_string(conf_block->stat_lo_mem, OS_LOMEM, redraw_flag);
	set_stat_string(conf_block->stat_ttl_excd, OS_TTLEX, redraw_flag);
	set_stat_string(conf_block->stat_chksum, OS_WCHKS, redraw_flag);
	set_stat_string(conf_block->stat_unreach, OS_UNDEL, redraw_flag);
	set_stat_string(conf_block->stat_all, OS_TOTAL, redraw_flag);

	set_stat_string(array[0], OSP_DROP, redraw_flag);
	set_stat_string(array[1], OSP_SENT, redraw_flag);
	set_stat_string(array[2], OSP_RECV, redraw_flag);

	set_stat_string(all_layers[stat_layer]->stat_dropped, OSL_DROP, redraw_flag);
}


void set_statistics(void)
{
	OBJECT *tree;

	stat_port = stat_layer = 0;

	rsrc_gaddr(R_TREE, O_STAT, &tree);
	strncpy(tree[OS_PORT].ob_spec.free_string, all_ports[stat_port], 10);
	strncpy(tree[OS_LAYER].ob_spec.free_string, all_layers[stat_layer]->name, 10);

	do_statistics(FALSE);
}


void show_statistics(void)
{
	do_statistics(TRUE);
}
