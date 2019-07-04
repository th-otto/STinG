/*********************************************************************/
/*                                                                   */
/*     STinG : Modem Dialer, Dial module                             */
/*                                                                   */
/*                                                                   */
/*      Version 1.1                        from 16. Januar 1997      */
/*                                                                   */
/*      Module for performing the Dialup actions                     */
/*                                                                   */
/*********************************************************************/


#include <aes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "device/device.h"

#include "dialer.h"
#include "rem_ctrl/remctrl.h"
#include "transprt.h"
#include "window.h"


#define  TIMEOUT        1
#define  RESTART        2
#define  REDIAL         3
#define  INITIA         4
#define  CLOSE          5
#define  FINISH         6

#define  P_PAP          0
#define  P_SCRIPT       1


static int b_read;
static int b_write;
static int local_phone;
int max_num_dials;
static int scr_to_count;
static int action;
static int prompt;
static int timeout_wait;
static int which_timeout;
static int lock_shortcut;
static int scr_index;
static int batch_what;
static int batch_handle;
static char ring_buffer[256];
static char *array[9];
static char *phone_string;
static char space[] = "                                ";
const char *batch;
static int b_flag;

static char const no_initia[] = "[1][ |  Initialisation failed,   | |  shutting down.][ Hmm ]";
static char const no_negotia[] = "[1][ |  Link negotiation failed,   | |  shutting down.][ Hmm ]";
static char const batch_n_fnd[] = "[1][ |  Can't run login tools,   | |  Batch file not found.][ Hmm ]";
static char const IP_to_short[] = "[1][ |  String too short for reading  | |  IP address (Ignored) !][ Hmm ]";
static char const IP_charac[] = "[1][ |  Numerical character or dot  | |  expected (Ignored) !][ Hmm ]";
static char const IP_format[] = "[1][ |  Wrong format in IP address  | |  (Ignored) !][ Hmm ]";


#if __GNUC_PREREQ(8, 1)
#pragma GCC diagnostic ignored "-Wstringop-truncation"
#endif


static void teddi(_WORD object, _WORD parent, const char *text)
{
	char full[65];

	strncpy(full, text, 32);
	full[32] = '\0';
	strcat(full, space);

	change_tedinfo(DIALER, object, parent, TE_PTEXT, full, -1);
}


static void finish_func(void)
{
	char timer[25];
	char *status;
	char *string;
	uint32 ip_addr;
	int flag;
	int result = FALSE;

	if (which_timeout == CLOSE)
	{
		if ((flag = check_port_flags(&ip_addr)) != 0)
		{
			if (flag == 1)
			{
				ip_address[0] = (int) ((ip_addr >> 24) & 0xff);
				ip_address[1] = (int) ((ip_addr >> 16) & 0xff);
				ip_address[2] = (int) ((ip_addr >> 8) & 0xff);
				ip_address[3] = (int) (ip_addr & 0xff);
				reset_config();
				sprintf(timer, ip_space, ip_address[0], ip_address[1], ip_address[2], ip_address[3]);
				teddi(DL_IP, DL_IP, timer);
				if (run_tools)
				{
					batch = getvstr("LOGIN_BATCH");
					if (*batch != '0')
					{
						spawn_batch(0);
						finish_login();
						return;
					} else
					{
						if (sender < 0)
							form_alert(1, no_batch_alert);
					}
				}
				status = "Link Established";
				string = "Success";
			} else
			{
				status = "Link Negotiation failed";
				string = get_PPP_status();
				string = string ? string : "Failure";
				hangup();
				if (sender < 0)
					form_alert(1, no_negotia);
			}
			finish_login();
			teddi(DL_STAT, DL_STAT, status);
			teddi(DL_STRNG, DL_STRNG, string);
			timeout_wait = 3;
			which_timeout = FINISH;
		}
	}

	if (timeout_wait > 0)
	{
		sprintf(timer, "%3d", --timeout_wait);
		teddi(DL_TIMER, DL_TIMER, timer);
		return;
	}

	switch (which_timeout)
	{
	case INITIA:
		ClearIOBuffer(curr_port, IO_BUFFERS);
		PortParameter(curr_port, _RTSCTS, _8BIT, _1STOP, _NO_PARITY);
		result = do_connect();
		if (result)
		{
			status = "Initialising Link";
			string = "";
			timeout_wait = 60;
			which_timeout = CLOSE;
		} else
		{
			finish_login();
			status = "Passing Connection Failed";
			string = "Shutting down";
			timeout_wait = 1;
			which_timeout = FINISH;
		}
		teddi(DL_STAT, DL_STAT, status);
		teddi(DL_STAT, DL_STRNG, string);
		break;
	case CLOSE:
		finish_login();
		hangup();
		if (sender < 0)
			form_alert(1, no_initia);
		timeout_wait = 1;
		which_timeout = FINISH;
		teddi(DL_STAT, DL_STAT, "Initialisation Timeout");
		teddi(DL_STAT, DL_STRNG, "Shutting down");
		break;
	case FINISH:
		dial_timer = NULL;
		dial_state = S_NONE;
		close_rsc_window(DIALER, -1);
		change_flags(START, ST_CNCT, 0, 0, OS_DISABLED);
		finish_dial(result ? IP_DIAL_OK : IP_OPEN_FAILED);
		break;
	}
}


static void send_script_str(const char *string)
{
	int index;
	char buffer[128];
	char *cr;

	strcpy(buffer, string);
	cr = &buffer[(index = (int) strlen(buffer)) - 1];

	if (cr[0] != '\\' && cr[0] != '|' && cr[0] != '/')
	{
		cr++;
		index++;
		cr[0] = '/';
		cr[1] = '\0';
	}

	if (cr[0] == '\\')
		cr[0] = '\033';
	if (cr[0] == '|')
		cr[0] = '\n';
	if (cr[0] == '/')
		cr[0] = '\r';

	PortSendBlock(curr_port, buffer, index, TRUE);
}


static int mod_256(int value)
{
	if (value < 0)
		return value + 256;
	if (value >= 256)
		return value - 256;

	return value;
}


static int fill_ring_buffer(void)
{
	int max_write;

	max_write = mod_256(b_read + 128);

	while (CharAvailable(curr_port))
	{
		if (b_write == max_write)
			break;
		ring_buffer[b_write] = PortGetByte(curr_port);
		b_write = mod_256(b_write + 1);
	}

	return b_write == max_write;
}


static int look_up(const char *find_string, int *posi)
{
	int proceed;
	int flag;
	int length;
	int last_search;
	int count;
	int cnt;
	int line;

	do
	{
		proceed = fill_ring_buffer();

		flag = (find_string != NULL);
		length = flag ? (int) strlen(find_string) : 12;
		last_search = mod_256(b_write - length + 1);

		for (count = b_read; count != b_write; count = mod_256(count + 1))
		{
			for (line = 0; line < 9; line++)
			{
				if (!flag)
				{
					if ((length = (int) strlen(find_string = array[line])) == 0)
						continue;
				}
				for (cnt = 0; cnt < length; cnt++)
				{
					if (mod_256(count + cnt) == b_write)
						break;
					if (ring_buffer[mod_256(count + cnt)] != find_string[cnt])
						break;
				}
				if (cnt == length)
				{
					*posi = count;
					b_read = mod_256(count + length);
					return flag ? -1 : (line / 3 + 1);
				}
				if (flag)
					break;
			}
		}
		b_read = last_search;

	} while (proceed);

	return 0;
}


static void get_line(int index, char *line)
{
	int count;

	while (ring_buffer[index] != '\r' && ring_buffer[index] != '\n')
	{
		if (index == b_write)
			break;
		index = mod_256(index - 1);
	}

	for (count = 0; count < 28; count++)
	{
		index = mod_256(index + 1);
		line[count] = ring_buffer[index];
		if (line[count] == '\r' || line[count] == '\n' || index == b_write)
			break;
	}

	line[count] = '\0';
}


static char get_ip_char(int *eof)
{
	char chr;

	if (b_read == b_write)
		fill_ring_buffer();

	if (b_read == b_write)
	{
		*eof = TRUE;
		return '\0';
	} else
	{
		*eof = FALSE;
		chr = ring_buffer[b_read];
		b_read = mod_256(b_read + 1);
		return chr;
	}
}


static int fetch_ip_address(void)
{
	int eof;
	int char_cnt = 0;
	int dot_cnt = 0;
	int count;
	char buffer[32];
	char chr;
	unsigned char ip[4];

	do
	{
		chr = get_ip_char(&eof);
		if (eof)
		{
			form_alert(1, IP_to_short);
			return 0;
		}
	} while (chr == ' ');

	do
	{
		if (((chr < '0' || chr > '9') && chr != '.') || char_cnt > 30)
		{
			if (dot_cnt < 3 || buffer[char_cnt - 1] == '.')
			{
				form_alert(1, IP_charac);
				return 0;
			} else
			{
				break;
			}
		}
		if ((buffer[char_cnt++] = chr) == '.')
		{
			dot_cnt++;
			if (dot_cnt == 4)
				break;
		}
		chr = get_ip_char(&eof);
	} while (!eof);

	if (eof)
		if (dot_cnt < 3 || buffer[char_cnt - 1] == '.')
		{
			form_alert(1, IP_to_short);
			return 0;
		}
	buffer[char_cnt] = '\0';

	ip[0] = 0;
	load_ip_addr(ip, buffer);
	if (ip[0] == 0)
	{
		form_alert(1, IP_format);
		return 0;
	} else
	{
		for (count = 0; count < 4; count++)
			ip_address[count] = ip[count];
	}
	
	return 1;
}


static void scripter(void);

static int passwd_click(_WORD object)
{
	OBJECT *tree;
	char *ptr;

	evnt_timer(60);
	change_flags(DIALER, object & 0x7fff, 0, 0, OS_SELECTED);

	switch (object & 0x7fff)
	{
	case PW_BUTT:
	case CLOSER_CLICKED:
		if (!top_rsc_window(DIALER))
		{
			rsrc_gaddr(R_TREE, PASSWORD, &tree);
			ptr = tree[PW_STRNG].ob_spec.tedinfo->te_ptext;
			if (prompt == P_SCRIPT)
			{
				dial_timer = scripter;
				PortSendBlock(curr_port, ptr, strlen(ptr), TRUE);
				PortSendBlock(curr_port, "\r", 1, TRUE);
			} else
			{
				dial_timer = finish_func;
				strncpy(eff_passwd, ptr, 32);
			}
		}
		return -1;
	}

	return 0;
}


static int passwd_key_typed(unsigned short scan)
{
	if (scan == CNTRL_Q)
		return passwd_click(CLOSER_CLICKED);

	return 1;
}


static char *fetch_speed(char *line)
{
	long speed;
	int count;
	int index;
	int length;

	evnt_timer(10);
	fill_ring_buffer();

	for (count = 0; count < 62; count++)
	{
		line[count] = ring_buffer[index = mod_256(b_read + count)];
		if (line[count] == '\r' || line[count] == '\n' || index == b_write)
			break;
	}
	line[count] = '\0';
	length = count;

	for (count = 0; count < length; count++)
	{
		speed = atol(&line[count]);
		if (299 < speed && speed < 120000L)
		{
			sprintf(line, "%ld bps", speed);
			break;
		}
	}

	if (count == length)
		strcpy(line, "Unknown");

	return line;
}


static void abort_dial(int flag)
{
	char command[] = "\r";

	off_hook = FALSE;

	PortSendBlock(curr_port, command, strlen(command), FALSE);

	if (flag)
	{
		change_freestring(START, ST_CNCT, -1, "Connect", 8);
		change_flags(START, ST_CNCT, 0, 0, OS_DISABLED);
		change_flags(START, ST_ABLE, 0, 0, OS_DISABLED);
	}
}


static void timeout_func(void);

static void dialer(void)
{
	char command[32];
	char *w_src;
	char *w_dest;
	int count;
	static int ok_count;

	switch (action)
	{
	case 0:
		ClearIOBuffer(curr_port, IO_BUFFERS);
		for (b_read = b_write = count = 0; count < 256; count++)
			ring_buffer[count] = ' ';
		w_src = modem_init;
		ok_count = 0;
		do
		{
			if (*w_src == ',')
			{
				evnt_timer(1100);
				w_src++;
			} else
			{
				for (w_dest = command; *w_src != ',' && *w_src != '\0';)
					*w_dest++ = *w_src++;
				*w_dest = '\0';
				teddi(DL_STRNG, DL_STRNG, command);
				strcat(command, "\r");
				PortSendBlock(curr_port, command, strlen(command), FALSE);
				ok_count++;
			}
		} while (*w_src != '\0');
		break;
	case 1:
		if (ok_count)
		{
			if (look_up("OK", &count) == 0)
				return;
			if (--ok_count > 0)
				return;
		}
		strcpy(command, modem_dial);
		phone_string = "";
		if (phone[local_phone][0])
		{
			strcat(command, " ");
			strcat(command, phone_string = phone[local_phone]);
		}
		if (++local_phone == phone_num)
			local_phone = 0;
		teddi(DL_STRNG, DL_STRNG, command);
		strcat(command, "\r");
		PortSendBlock(curr_port, command, strlen(command), FALSE);
		off_hook = TRUE;
		dial_timer = timeout_func;
		dialer_delay = 5;
		break;
	}

	action++;
}


static void timeout_func(void)
{
	char timer[20];
	char line[64];
	char *status;
	char *string;
	int result;
	int index;

	if (which_timeout == TIMEOUT)
	{
		if ((result = look_up(NULL, &index)) > 0)
		{
			get_line(index, line);
		}
		if (result > 0)
		{
			teddi(DL_STAT, DL_STAT, result == 1 ? "Dial Success" : "Dial Failed");
			teddi(DL_STRNG, DL_STRNG, line);
			switch (result)
			{
			case 1:
				write_log_text(" ");
				sprintf(line, "Modem connects to %s", phone_string);
				write_time_log(line);
				dial_timer = scripter;
				dialer_delay = 1;
				dial_state = S_SCRIPT;
				scr_index = action = 0;
				teddi(DL_SPEED, DL_SPEED, fetch_speed(line));
				return;
			case 2:
				off_hook = FALSE;
				timeout_wait = 3;
				which_timeout = RESTART;
				break;
			case 3:
				abort_dial(TRUE);
				dial_timer = NULL;
				dial_state = S_NONE;
				finish_dial(IP_FATAL_PROBLEM);
				change_flags(DIALER, DL_RDIAL, 0, 0, OS_DISABLED);
				return;
			}
		}
	}

	if (timeout_wait > 0)
	{
		sprintf(timer, "%3d", --timeout_wait);
		teddi(DL_TIMER, DL_TIMER, timer);
		return;
	}

	switch (which_timeout)
	{
	case REDIAL:
		lock_shortcut = TRUE;
		change_flags(DIALER, DL_RDIAL, 1, 0, OS_DISABLED);
		teddi(DL_STAT, DL_STAT, "Dialing (ESC to abort)");
		action = 0;
		dial_timer = dialer;
		dialer_delay = 1;
		timeout_wait = connect_timeout;
		which_timeout = TIMEOUT;
		break;
	case TIMEOUT:
		teddi(DL_STAT, DL_STAT, "Dial Failed");
		teddi(DL_STRNG, DL_STRNG, "Timeout");
		abort_dial(FALSE);
		timeout_wait = 1;
		which_timeout = RESTART;
		break;
	case RESTART:
		if (--max_num_dials == 0)
		{
			dial_timer = NULL;
			dial_state = S_NONE;
			finish_dial(IP_MAX_EXCEEDED);
			status = "Maximum Redials Exceeded";
			string = "";
		} else
		{
			lock_shortcut = FALSE;
			timeout_wait = redial_delay;
			which_timeout = REDIAL;
			status = "Redial Delay";
			string = "Any key to dial next";
		}
		teddi(DL_STAT, DL_STAT, status);
		teddi(DL_STRNG, DL_STRNG, string);
		change_flags(DIALER, DL_RDIAL, 0, 0, OS_DISABLED);
		break;
	}
}


static void scripter(void)
{
	char ip[25];
	char buffer[128];
	static char const password[] = " STinG Dialer : Password Prompt ";
	long wait_time;
	int index;
	int flag = FALSE;
	static int draw_flag;

	if (scr_index == script_length)
	{
		teddi(DL_STAT, DL_STAT, "Connected ...");
		teddi(DL_STRNG, DL_STRNG, "");
		timeout_wait = 1;
		which_timeout = INITIA;
		dial_state = S_CONNECT;
		if (protocol != 0 && papp_flag != 0)
		{
			dial_timer = NULL;
			prompt = P_PAP;
			change_tedinfo(PASSWORD, PW_STRNG, -1, TE_PTEXT, "", 2);
			open_rsc_window(PASSWORD, PW_STRNG, password, " Password ", DIALER);
			set_callbacks(PASSWORD, passwd_click, passwd_key_typed);
		} else
		{
			strncpy(eff_passwd, pap_passwd, 32);
			dial_timer = finish_func;
		}
		dialer_delay = 5;
		return;
	}

	switch (action)
	{
	case 0:
		dialer_delay = (int) ((wait_time = atol(script[scr_index][0])) / 200 + 1);
		scr_to_count = script_timeout * 10;
		sprintf(buffer, "Waiting %ld ms", wait_time);
		teddi(DL_STAT, DL_STAT, buffer);
		teddi(DL_TIMER, DL_TIMER, "");
		draw_flag = TRUE;
		break;
	case 1:
		if (script[scr_index][2][0] != '\0')
		{
			if (draw_flag)
			{
				sprintf(buffer, "Find \"%s", script[scr_index][2]);
				if (strlen(script[scr_index][2]) > 21)
					strcpy(&buffer[24], "...");
				strcat(buffer, "\"");
				teddi(DL_STAT, DL_STAT, buffer);
				dial_timer = scripter;
				dialer_delay = 1;
			}
			draw_flag = FALSE;
			if (--scr_to_count % 10 == 0)
			{
				sprintf(buffer, "%3d", scr_to_count / 10);
				teddi(DL_TIMER, DL_TIMER, buffer);
			}
			if (scr_to_count < 0)
			{
				teddi(DL_STAT, DL_STAT, "Login Failed");
				teddi(DL_STRNG, DL_STRNG, "Timeout");
				hangup();
				dial_state = S_DIAL;
				dial_timer = timeout_func;
				dialer_delay = 5;
				timeout_wait = 1;
				which_timeout = RESTART;
			}
			if (look_up(script[scr_index][2], &index) == 0)
			{
				if (script[scr_index][1][0] != '\0')
					send_script_str(script[scr_index][1]);
				return;
			}
			get_line(index, buffer);
			teddi(DL_STRNG, DL_STRNG, buffer);
		}
		break;
	case 2:
		if (script[scr_index][3][0] == '\0')
		{
			action = 0;
			scr_index++;
			return;
		}
		if (strcmp(script[scr_index][3], "$GET_IP") == 0)
		{
			flag = TRUE;
			fetch_ip_address();
			sprintf(ip, ip_space, ip_address[0], ip_address[1], ip_address[2], ip_address[3]);
			teddi(DL_IP, DL_IP, ip);
			reset_config();
		}
		if (strcmp(script[scr_index][3], "$PASSWD") == 0)
		{
			prompt = P_SCRIPT;
			flag = TRUE;
			open_rsc_window(PASSWORD, PW_STRNG, password, " Password ", DIALER);
			set_callbacks(PASSWORD, passwd_click, passwd_key_typed);
			dial_timer = NULL;
			change_tedinfo(PASSWORD, PW_STRNG, -1, TE_PTEXT, "", 2);
		}
		if (flag == FALSE)
		{
			send_script_str(script[scr_index][3]);
		}
		action = -1;
		scr_index++;
		break;
	}

	action++;
}


static int dial_click(_WORD object)
{
	evnt_timer(60);
	change_flags(DIALER, object & 0x7fff, 0, 0, OS_SELECTED);

	switch (object & 0x7fff)
	{
	case CLOSER_CLICKED:
	case DL_ABORT:
		switch (dial_state)
		{
		case S_DIAL:
			abort_dial(TRUE);
			break;
		case S_SCRIPT:
			hangup();
			break;
		case S_CONNECT:
			finish_login();
			hangup();
			break;
		case S_BATCH:
			Fclose(batch_handle);
			if (b_flag >= 0)
			{
				change_flags(START, ST_CNCT, (b_flag & 1) ? 1 : 0, 0, OS_DISABLED);
				change_flags(START, ST_ABLE, (b_flag & 2) ? 1 : 0, 0, OS_DISABLED);
			} else
			{
				hangup();
			}
			dial_timer = NULL;
			break;
		}
		finish_dial(IP_USER_ABORT);
		close_rsc_window(PASSWORD, -1);
		dial_timer = NULL;
		dial_state = S_NONE;
		return -1;
	case DL_RDIAL:
		if (dial_state == S_DIAL)
		{
			teddi(DL_TIMER, DL_TIMER, "  0 ");
			dial_timer = timeout_func;
			dialer_delay = 5;
			timeout_wait = 0;
			which_timeout = REDIAL;
		}
		break;
	}

	return 0;
}


static int dial_key_typed(unsigned short scan)
{
	if (scan == CNTRL_Q || scan == ESC)
		return dial_click(CLOSER_CLICKED);

	if (dial_state == S_DIAL && !lock_shortcut)
	{
		teddi(DL_TIMER, DL_TIMER, "  0 ");
		timeout_wait = 0;
	}

	return 0;
}


void spawn_dialer(void)
{
	int count;

	teddi(DL_STAT, -1, "");
	teddi(DL_STRNG, -1, "");
	teddi(DL_SPEED, -1, "");
	teddi(DL_IP, -1, "");
	teddi(DL_TIMER, -1, "");

	open_rsc_window(DIALER, 0, " STinG Dialer : Connecting ", " Connecting ", START);
	set_callbacks(DIALER, dial_click, dial_key_typed);

	change_flags(START, ST_CNCT, 1, 0, OS_DISABLED);
	change_flags(START, ST_ABLE, 1, 0, OS_DISABLED);
	change_flags(DIALER, DL_RDIAL, 1, 0, OS_DISABLED);
	change_flags(DIALER, DL_ABORT, 0, 0, OS_DISABLED);

	for (count = 0; count < 3; count++)
		if (strcmp(s_conn[count], "DIRECT") == 0)
			break;

	if (count == 3)
	{
		for (count = 0; count < 3; count++)
		{
			array[count] = s_conn[count];
			array[count + 3] = s_redl[count];
			array[count + 6] = s_abrt[count];
		}
		dial_timer = timeout_func;
		dialer_delay = 5;
		if (compuserve)
			PortParameter(curr_port, _RTSCTS, _7BIT, _1STOP, _EVENP);
		else
			PortParameter(curr_port, _RTSCTS, _8BIT, _1STOP, _NO_PARITY);
		local_phone = 0;
		timeout_wait = 0;
		which_timeout = REDIAL;
		dial_state = S_DIAL;
	} else
	{
		teddi(DL_STAT, DL_STAT, "Direct Connection");
		teddi(DL_STRNG, DL_STRNG, "No Dialing");
		scr_index = action = 0;
		dial_timer = scripter;
		dialer_delay = 1;
		ClearIOBuffer(curr_port, IO_BUFFERS);
		PortParameter(curr_port, _RTSCTS, _8BIT, _1STOP, _NO_PARITY);
		off_hook = TRUE;
		dial_state = S_SCRIPT;
	}
}


static int get_batch_line(char *path, char *command)
{
	int pos;
	int total;
	long length;
	long file;
	char buffer[256];
	char *ptr;

	file = Fseek(0, batch_handle, 1);

	if ((length = Fread(batch_handle, 255L, buffer)) <= 0)
		return FALSE;

	if (!strchr(buffer, '\r'))
		buffer[length] = '\r';
	if (buffer[(total = pos = (int) (strchr(buffer, '\r') - buffer)) + 1] == '\n')
		total++;
	Fseek(file + total + 1, batch_handle, 0);
	buffer[pos] = '\0';

	if (path && command)
	{
		command++;
		strncpy(path, buffer, pos = length < 127 ? (int) length : 127);
		path[pos] = '\0';
		if (strchr(path, ' '))
		{
			*strchr(path, ' ') = '\0';
		}
		if (strchr(buffer, ' '))
		{
			for (ptr = strchr(buffer, ' '); *ptr == ' '; ptr++) ;
			length -= ptr - buffer;
			strncpy(command, ptr, pos = length < 126 ? (int) length : 126);
			command[pos] = '\0';
		} else
		{
			command[0] = '\0';
		}
	}

	return TRUE;
}


static void batch_func(void)
{
	int posi;
	char path[130];
	char command[130];
	char string[64];
	char timer[20];
	char drive;
	char own_path[130];

	if (!get_batch_line(path, command))
	{
		if (batch_what)
		{
			close_rsc_window(DIALER, -1);
		} else
		{
			Fclose(batch_handle);
			timeout_wait = 3;
			which_timeout = FINISH;
			dialer_delay = 5;
			dial_timer = finish_func;
			teddi(DL_STAT, DL_STAT, "Link Established");
			teddi(DL_STRNG, DL_STRNG, "Success");
		}
		return;
	}

	strncpy(string, strrchr(path, '\\') ? strrchr(path, '\\') + 1 : path, 30);
	string[30] = '\0';
	strcat(string, " ");
	strncpy(string + strlen(string), &command[1], 30);
	string[30] = '\0';
	strcat(string, space);
	teddi(DL_STRNG, DL_STRNG, string);

	sprintf(timer, "%3d ", --timeout_wait);
	teddi(DL_TIMER, DL_TIMER, timer);

	command[0] = (int) strlen(&command[1]);

	if (strchr(path, '\\') != NULL)
	{
		if (path[1] == ':')
		{
			posi = 2;
			drive = 'A' + Dgetdrv();
			Dsetdrv(path[0] - 'A');
		} else
		{
			posi = 0;
			drive = 'A' - 1;
		}
		Dgetpath(own_path, drive - 'A' + 1);
		*strrchr(path, '\\') = '\0';
		Dsetpath(path[posi] ? &path[posi] : "\\");
		path[strlen(path)] = '\\';
		Pexec(0, path, command, "");
		if (drive >= 'A')
			Dsetdrv(drive - 'A');
		Dsetpath(own_path);
	} else
	{
		Pexec(0, path, command, "");
	}
}


void spawn_batch(int what)
{
	OBJECT *tree;
	long error;
	char timer[5];

	if ((error = Fopen(batch, FO_READ)) < 0)
	{
		if (sender < 0)
			form_alert(1, batch_n_fnd);
		return;
	}
	batch_handle = (int) error;

	for (timeout_wait = 0; get_batch_line(NULL, NULL); timeout_wait++) ;
	Fseek(0, batch_handle, 0);
	sprintf(timer, "%3d ", timeout_wait);

	teddi(DL_STAT, what ? what : DL_STAT, "Executing Login Batch");
	teddi(DL_STRNG, what ? what : DL_STRNG, "");
	teddi(DL_TIMER, what ? what : DL_TIMER, timer);

	if (what)
	{
		teddi(DL_IP, what, "   ---");
		open_rsc_window(DIALER, 0, " STinG Dialer : Connecting ", " Connecting ", START);
		set_callbacks(DIALER, dial_click, dial_key_typed);

		rsrc_gaddr(R_TREE, START, &tree);
		b_flag = (tree[ST_CNCT].ob_state & OS_DISABLED) ? 1 : 0;
		b_flag |= (tree[ST_ABLE].ob_state & OS_DISABLED) ? 2 : 0;

		change_flags(START, ST_CNCT, 1, 0, OS_DISABLED);
		change_flags(START, ST_ABLE, 1, 0, OS_DISABLED);
		change_flags(DIALER, DL_RDIAL, 1, 0, OS_DISABLED);
		change_flags(DIALER, DL_ABORT, 0, 0, OS_DISABLED);
	} else
	{
		b_flag = -1;
	}
	
	dial_timer = batch_func;
	dialer_delay = 1;
	dial_state = S_BATCH;
	batch_what = what;
}


int hangup(void)
{
	char *work;

	if (do_disconnect())
	{
		if (!strcmp(modem_hangup, "DTR"))
		{
			DtrOff(curr_port);
			evnt_timer(500);
			DtrOn(curr_port);
		} else
		{
			for (work = modem_hangup; *work != '\0'; work++)
			{
				if (*work == ',')
				{
					evnt_timer(1100);
				} else
				{
					PortSendByte(curr_port, *work);
				}
			}
			PortSendByte(curr_port, '\r');
		}
		write_time_log("Modem hung up.");
		write_extra_line();
	} else
	{
		return FALSE;
	}
	
	change_flags(START, ST_CNCT, 0, 0, OS_DISABLED);
	change_flags(START, ST_ABLE, 0, 0, OS_DISABLED);
	off_hook = FALSE;

	return TRUE;
}
