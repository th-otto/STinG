#include <aes.h>
#include <tos.h>
#include <stdio.h>
#include <string.h>

#include "inetd.h"
#include "module.h"
#include "window.h"


#define  VA_START         0x4711

int disp_offset;
char ism_path[256];
char inetd_path[256];

static _WORD sender;
static int main_is_open;
static int exit_inetd;



static void get_path(void)
{
	int handle;
	long len;
	char *ptr;
	static char const file[] = "\\inetd.inf";
	static char const path[] = "\\ISM";

	inetd_path[0] = 'A' + Dgetdrv();
	inetd_path[1] = ':';
	Dgetpath(&inetd_path[2], 0);

	strcpy(ism_path, inetd_path);

	strcat(ism_path, file);
	handle = (int) Fopen(ism_path, FO_READ);

	if (handle < 0)
	{
		strcpy(&ism_path[2], file);
		handle = (int) Fopen(ism_path, FO_READ);

		if (handle < 0)
		{
			strcpy(ism_path, path);
			return;
		}
	}

	len = Fread(handle, 250, ism_path);
	Fclose(handle);

	if (len > 0)
	{
		if ((ptr = strchr(ism_path, '\r')) != NULL)
			*ptr = '\0';
		if ((ptr = strchr(ism_path, '\n')) != NULL)
			*ptr = '\0';
		ism_path[len] = '\0';
	} else
	{
		strcpy(&ism_path[2], path);
	}
}


static void terminate(void)
{
	_WORD message[8];
	static char const problem[] = "[1][ |  Problem occured during   | |    initialisation !][ Hmm ]";

	if (_app)
	{
		appl_exit();
		return;
	} else
	{
		for (;;)
		{
			evnt_mesag(message);
			if (message[0] == AC_OPEN)
				form_alert(1, problem);
		}
	}
}


static void do_slider(_WORD position, _WORD offset)
{
	OBJECT *tree;
	int difference;

	rsrc_gaddr(R_TREE, START, &tree);

	if (position < 0)
	{
		disp_offset += offset;
		if (disp_offset < 0)
			disp_offset = 0;
		if (disp_offset > num_modules - 4)
			disp_offset = num_modules - 4;
	} else
		disp_offset = position;

	difference = tree[ST_S_GND].ob_height - tree[ST_S_BTN].ob_height;
	tree[ST_S_BTN].ob_y = difference * disp_offset / (num_modules - 4);
	insert_modules(TRUE);
	change_flags(START, ST_SLIDE, FALSE, 0, 0);
}


static int key_typed(unsigned short scan)
{
	return scan == CNTRL_Q ? -1 : 1;
}


static int main_click(_WORD object)
{
	OBJECT *tree;
	_WORD pos_x, pos_y;
	_WORD key_state;
	_WORD mouse_state;
	int offset;
	int count;

	rsrc_gaddr(R_TREE, START, &tree);
	graf_mkstate(&pos_x, &pos_y, &mouse_state, &key_state);

	if ((object & 0x7fff) != ST_S_BTN)
		evnt_timer(60);
	change_flags(START, object & 0x7fff, 0, 0, OS_SELECTED);

	switch (object & 0x7fff)
	{
	case ST_CONF:
		open_rsc_window(CONF, edit[conf_shown], " STiK INetD : Configuration ", " Config ", START);
		fill_in_config_box();
		set_callbacks(CONF, conf_click, conf_typed);
		break;
	case AUTHORS:
		open_rsc_window(CREDITS, 0, " STiK INetD : Credits ", " Credits ", START);
		set_callbacks(CREDITS, 0, key_typed);
		break;
	case ST_MCK1:
	case ST_MCK2:
	case ST_MCK3:
	case ST_MCK4:
		for (count = 0; count < 4; count++)
			if (click_box[count] == (object & 0x7fff))
				break;
		change_flags(START, mdle_box[count], 0, 0, 0);
		call_module(disp_offset + count);
		break;
	case ST_S_UP:
		do_slider(-1, -1);
		break;
	case ST_S_DWN:
		do_slider(-1, 1);
		break;
	case ST_S_BTN:
		graf_mkstate(&pos_x, &pos_y, &mouse_state, &key_state);
		if (num_modules > 4)
		{
			offset = 500 / (num_modules - 4);
			if (mouse_state & 0x01)
			{
				count = offset + graf_slidebox(tree, ST_S_GND, ST_S_BTN, TRUE);
				do_slider((num_modules - 4) * count / 1000, 0);
			}
		}
		break;
	case ST_S_GND:
		offset = pos_y;
		objc_offset(tree, ST_S_BTN, &pos_x, &pos_y);
		do_slider(-1, (offset < pos_y) ? -4 : ((offset > pos_y + tree[ST_S_BTN].ob_height) ? 4 : 0));
		break;
	case CLOSER_CLICKED:
		if ((key_state & 0x04) != 0)
		{
			close_rsc_window(CREDITS, -1);
			close_rsc_window(CONF, -1);
			exit_inetd = TRUE;
			return 1;
		}
	}

	return 0;
}


static void do_some_work(void);

static int message_handler(_WORD message[8])
{
	sender = message[1];

	switch (message[0])
	{
	case AC_OPEN:
	case VA_START:
		if (!main_is_open)
			do_some_work();
		else
			top_rsc_window(START);
		break;
	case AC_CLOSE:
	case AP_TERM:
	case AP_RESCHG:
		exit_inetd = TRUE;
		break;
	}

	return 0;
}


static void do_some_work(void)
{
	_WORD event = 0;

	main_is_open = TRUE;

	open_rsc_window(START, 0, " STiK  Super Server ", " Main ", -1);
	set_callbacks(START, main_click, key_typed);
	set_message_callback(message_handler);

	while (event >= 0)
		if ((event = operate_events()) == -4)
			exit_inetd = TRUE;

	main_is_open = FALSE;
}


int main(void)
{
	_WORD message[8];
	_WORD flag;
	char string[6];

	gl_apid = appl_init();

	if (!_app)
	{
		menu_register(gl_apid, "  Super Server");
	}

	if (!rsrc_load("inetd.rsc"))
	{
		form_alert(1, "[1][ |  Cannot find INETD.RSC !  ][ Cancel ]");
		terminate();
		return 1;
	}

	get_path();

	if (initialise_windows(4, ICONIFY) == 0)
	{
		leave_windows();
		rsrc_free();
		terminate();
		return 1;
	}

	if (get_version(string) > 0)
	{
		form_alert(1, "[1][ |   STiK is not loaded,    | |      or corrupted !][ Cancel ]");
		leave_windows();
		rsrc_free();
		terminate();
		return 1;
	}
	change_freestring(START, ST_VERS, -1, string, 5);
	change_freestring(START, SS_VERS, -1, version, 5);

	set_api_struct();

	if (init_modules() == 0)
	{
		leave_windows();
		rsrc_free();
		terminate();
		return 1;
	}
	set_event_callback(check_modules);

	init_configs();

	graf_mouse(ARROW, NULL);

	if (_app)
	{
		do_some_work();
		while (!exit_inetd)
		{
			evnt_mesag(message);
			message_handler(message);
			while ((flag = operate_events()) >= 0) ;
			if (flag == -4)
				exit_inetd = TRUE;
		}
	} else
	{
		for (;;)
		{
			evnt_mesag(message);
			message_handler(message);
		}
	}

	if (strings)
		Mfree(strings);

	terminate_modules();

	leave_windows();

	rsrc_free();

	appl_exit();
	
	return 0;
}
