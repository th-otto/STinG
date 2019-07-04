#include <aes.h>
#include <vdi.h>
#include <tos.h>
#include <string.h>
#include <stdio.h>
#include "module.h"
#include "window.h"


#define  MAX_WIN          32

#define  MIN(a,b)         (((a) < (b)) ? (a) : (b))
#define  MAX(a,b)         (((a) > (b)) ? (a) : (b))
#define  SWAP(a,b)        (zwsp = (a), (a) = (b), (b) = zwsp)

#define  _HZ_200          ((volatile long *) 0x4baL)


typedef struct
{
	_WORD window_handle;
	_WORD tree_index;
	_WORD kind;
	_WORD icnfy_flg;
	_WORD x_pos;
	_WORD y_pos;
	_WORD width;
	_WORD height;
	OBJECT *tree;
	const char *full_name;
	const char *short_name;
	_WORD next;
	_WORD edit;
	_WORD ed_char;
	int (*object_click)(_WORD object);
	int (*key_typed)(unsigned short scan);
} WIN_DESC;


static WIN_DESC win_array[MAX_WIN];
static OBJECT *icnfy_tree;
static GRECT desk;
static MFDB screen;
static MFDB popup;

static long delay;
static long more_time;
static int (*timer_thread)(void);
static int (*message_thread)(_WORD *message);
static int (*menu_thread)(_WORD title, _WORD entry);
static int (*event_thread)(void);
static int window_count;
static _WORD vdi_handle;
static _WORD planes;
static _WORD window_kind = NAME | CLOSER | MOVER | BACKDROP;
static _WORD extra_x_kind = SIZER | LFARROW | RTARROW | HSLIDE;
static _WORD extra_y_kind = SIZER | UPARROW | DNARROW | VSLIDE;



static _WORD __CDECL my_button_handler(PARMBLK *parameter)
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


void rsc_ext_objects(OBJECT *tree)
{
	for (;;)
	{
		if ((tree->ob_type & 0x7f00) && (tree->ob_state & (OS_CROSSED | OS_CHECKED)))
		{
			tree->ob_state &= ~(OS_CROSSED | OS_CHECKED);
			tree->ob_type = G_USERDEF;
			tree->ob_spec.userblk = &my_user_block;
		}
		if (tree->ob_flags & OF_LASTOB)
			break;
		tree++;
	}
}


int initialise_windows(_WORD num_trees, _WORD icnfy_index)
{
	OBJECT *tree;
	_WORD count;
	_WORD work_in[11];
	_WORD work_out[57];

	for (count = 0; count < 10; count++)
		work_in[count] = 1;
	work_in[10] = 2;

	vdi_handle = graf_handle(&count, &count, &count, &count);
	v_opnvwk(work_in, &vdi_handle, work_out);

	if (vdi_handle == 0)
	{
		form_alert(1, "[1][ |  Problems opening a virtual  | |  VDI workstation !][ Hm ]");
		return FALSE;
	}

	vq_extnd(vdi_handle, 1, work_out);
	planes = work_out[4];

	for (count = 0; count < num_trees; count++)
	{
		rsrc_gaddr(R_TREE, count, &tree);
		rsc_ext_objects(tree);
	}

	for (count = 0; count < MAX_WIN; count++)
		win_array[count].tree_index = win_array[count].window_handle = -1;

	wind_get_grect(0, WF_WORKXYWH, &desk);

	more_time = delay = -1L;
	timer_thread = 0;
	message_thread = 0;
	menu_thread = 0;
	event_thread = 0;

	if (icnfy_index != -1)
	{
		rsrc_gaddr(R_TREE, icnfy_index, &icnfy_tree);
		window_kind |= ICONIFIER;
	} else
	{
		icnfy_tree = NULL;
	}

	return TRUE;
}


void leave_windows(void)
{
	_WORD count;

	for (count = 0; count < MAX_WIN; count++)
		if (win_array[count].tree_index >= 0)
		{
			wind_close(win_array[count].window_handle);
			wind_delete(win_array[count].window_handle);
		}

	if (vdi_handle)
		v_clsvwk(vdi_handle);
}


static void set_slider(_WORD index)
{
	WIN_DESC *window;
	_WORD size;
	_WORD pos;
	_WORD msg[8];

	window = &win_array[index];

	msg[0] = WM_REDRAW;
	msg[1] = gl_apid;
	msg[3] = window->window_handle;
	msg[2] = 0;
	wind_get_grect(window->window_handle, WF_WORKXYWH, (GRECT *)&msg[4]);

	if (window->kind & HSLIDE)
	{
		_WORD w = msg[6];
		if (window->width <= w)
		{
			pos = 0;
		} else
		{
			pos = (_WORD) (window->x_pos * 1000L / (window->width - w));
			if (pos > 1000)
			{
				window->x_pos = window->width - w;
				pos = 1000;
				appl_write(gl_apid, 16, msg);
			}
		}
		wind_set_int(window->window_handle, WF_HSLIDE, pos);
		size = (_WORD) (w * 1000L / window->width);
		wind_set_int(window->window_handle, WF_HSLSIZE, size);
	}

	if (window->kind & VSLIDE)
	{
		_WORD h = msg[7];
		if (window->height <= h)
		{
			pos = 0;
		} else
		{
			pos = (_WORD) (window->y_pos * 1000L / (window->height - h));
			if (pos > 1000)
			{
				window->y_pos = window->height - h;
				pos = 1000;
				appl_write(gl_apid, 16, msg);
			}
		}
		wind_set_int(window->window_handle, WF_VSLIDE, pos);
		size = (_WORD) (h * 1000L / window->height);
		wind_set_int(window->window_handle, WF_VSLSIZE, size);
	}
}


static _WORD search_tree(_WORD rsc_tree)
{
	_WORD count;

	for (count = 0; count < MAX_WIN; count++)
		if (win_array[count].tree_index == rsc_tree)
			break;

	return count == MAX_WIN ? -1 : count;
}


_WORD open_rsc_window(_WORD rsc_tree, _WORD edit, const char *name, const char *shorter, _WORD parent)
{
	WIN_DESC *window;
	OBJECT *tree;
	_WORD count;
	_WORD index;
	_WORD center_x;
	_WORD center_y;
	_WORD kind;
	GRECT pos;
	GRECT all;

	if ((index = search_tree(rsc_tree)) >= 0)
	{
		wind_set_int(win_array[index].window_handle, WF_TOP, 0);
		return index;
	}

	if (parent == -1)
	{
		center_x = desk.g_x + desk.g_w / 2, center_y = desk.g_y + desk.g_h / 2;
	} else
	{
		if ((parent = search_tree(parent)) < 0)
		{
			center_x = desk.g_x + desk.g_w / 2, center_y = desk.g_y + desk.g_h / 2;
		} else
		{
			wind_get_grect(win_array[parent].window_handle, WF_CURRXYWH, &all);
			center_x = all.g_x + all.g_w / 2;
			center_y = all.g_y + all.g_h / 2;
		}
	}

	for (index = 0; index < MAX_WIN; index++)
		if (win_array[index].tree_index == -1)
			break;

	rsrc_gaddr(R_TREE, rsc_tree, &tree);
	form_center_grect(tree, &pos);

	for (count = 0, kind = window_kind; count < 2; count++)
	{
		wind_calc(WC_BORDER, kind, pos.g_x + 1, pos.g_y + 1, pos.g_w - 2, pos.g_h - 2, &all.g_x, &all.g_y, &all.g_w, &all.g_h);
		if (all.g_w > desk.g_w)
			all.g_w = desk.g_w, kind |= extra_x_kind;
		if (all.g_h > desk.g_h)
			all.g_h = desk.g_h, kind |= extra_y_kind;
	}

	all.g_x = center_x - all.g_w / 2;
	all.g_x = (all.g_x > desk.g_x) ? all.g_x : desk.g_x;
	all.g_y = center_y - all.g_h / 2;
	all.g_y = (all.g_y > desk.g_y) ? all.g_y : desk.g_y;

	if (all.g_x + all.g_w > desk.g_x + desk.g_w)
		all.g_x = desk.g_x + desk.g_w - all.g_w;
	if (all.g_y + all.g_h > desk.g_y + desk.g_h)
		all.g_y = desk.g_y + desk.g_h - all.g_h;

	if (index < MAX_WIN)
	{
		window = &win_array[index];
		window->window_handle = wind_create(window->kind = kind, 0, 0, all.g_w, all.g_h);
		if (window->window_handle < 0)
			index = MAX_WIN;
	}
	if (index == MAX_WIN)
	{
		form_alert(1, "[1][ |   Cannot open a window !   ][ Cancel ]");
		return -1;
	}

	wind_set_str(window->window_handle, WF_NAME, name);
	wind_open_grect(window->window_handle, &all);

	window->x_pos = window->y_pos = 0;
	window->width = pos.g_w - 2;
	window->height = pos.g_h - 2;
	set_slider(index);

	window->tree_index = rsc_tree;
	window->tree = tree;
	window->edit = 0;
	window->next = (tree[edit].ob_flags & OF_EDITABLE) ? edit : 0;
	window->object_click = 0;
	window->key_typed = 0;

	window->icnfy_flg = FALSE;
	window->full_name = name;
	window->short_name = shorter;
	window_count++;

	return index;
}


static _WORD search_window(_WORD window_handle)
{
	_WORD count;

	for (count = 0; count < MAX_WIN; count++)
		if (win_array[count].tree_index >= 0)
			if (win_array[count].window_handle == window_handle)
				break;

	return count == MAX_WIN ? -1 : count;
}


static int finish(_WORD index, _WORD reason)
{
	WIN_DESC *window;

	window = &win_array[index];

	if (window->edit != 0)
		objc_edit(window->tree, window->edit, 0, &window->ed_char, ED_END);

	if (window->object_click != NULL)
		window->object_click(reason);

	wind_close(win_array[index].window_handle);
	wind_delete(win_array[index].window_handle);

	win_array[index].tree_index = -1;

	return --window_count > 0 ? 0 : 1;
}


int close_rsc_window(_WORD rsc_tree, _WORD window_handle)
{
	_WORD pre_index = -1;
	_WORD index = -1;

	if (rsc_tree >= 0)
		pre_index = search_tree(rsc_tree);
	if (window_handle >= 0)
		index = search_window(window_handle);

	if (pre_index == -1)
	{
		if (index == -1)
			return -1;
	} else
	{
		if (index == -1)
			index = pre_index;
		else if (index != pre_index)
			return -1;
	}

	finish(index, CLOSER_CLICKED);

	return 0;
}


void set_callbacks(_WORD rsc_tree, int (*object_click) (_WORD obj), int (*key_typed) (unsigned short scan))
{
	_WORD index;

	if (rsc_tree >= 0)
	{
		if ((index = search_tree(rsc_tree)) >= 0)
		{
			win_array[index].object_click = object_click;
			win_array[index].key_typed = key_typed;
		}
	}
}


void set_timer_callback(int (*timer_func)(void), long timer_delay)
{
	timer_thread = timer_func;
	more_time = delay = timer_delay;
}


void set_message_callback(int (*message_func)(_WORD *message))
{
	message_thread = message_func;
}


void set_menu_callback(int (*menu_func)(_WORD title, _WORD item))
{
	menu_thread = menu_func;
}


void set_event_callback(int (*event_func)(void))
{
	event_thread = event_func;
}


static long read_timer(void)
{
	return *_HZ_200 * 5;
}


static void do_redraw(_WORD handle, GRECT *rect, _WORD sub)
{
	WIN_DESC *window;
	OBJECT *tree;
	GRECT act;
	_WORD index;
	GRECT pos;

	if ((index = search_window(handle)) < 0)
		return;

	window = &win_array[index];

	tree = (window->icnfy_flg) ? icnfy_tree : window->tree;

	wind_get_grect(handle, WF_WORKXYWH, &pos);
	tree->ob_x = pos.g_x - 1 - window->x_pos;
	tree->ob_y = pos.g_y - 1 - window->y_pos;

	if (window->icnfy_flg)
	{
		tree->ob_width = pos.g_w + 2;
		tree->ob_height = pos.g_h + 2;
		tree[1].ob_x = pos.g_x + pos.g_w / 2 - tree[1].ob_width / 2 - tree->ob_x;
		tree[1].ob_y = pos.g_y + pos.g_h / 2 - tree[1].ob_height / 2 - tree->ob_y;
	}

	wind_update(BEG_UPDATE);

	if (window->edit && (!window->icnfy_flg))
		objc_edit(window->tree, window->edit, 0, &window->ed_char, ED_END);

	rc_intersect(&desk, rect);
	wind_get_grect(handle, WF_FIRSTXYWH, &pos);
	do
	{
		act = pos;
		if (rc_intersect(rect, &act))
			objc_draw_grect(tree, sub, MAX_DEPTH, &act);
		wind_get_grect(handle, WF_NEXTXYWH, &pos);
	} while (pos.g_w != 0 || pos.g_h != 0);

	if (window->edit && (!window->icnfy_flg))
		objc_edit(window->tree, window->edit, 0, &window->ed_char, ED_INIT);

	wind_update(END_UPDATE);
}


static int do_message_event(_WORD *mesag)
{
	WIN_DESC *window;
	GRECT act;
	_WORD index;
	_WORD pos;

	switch (mesag[0])
	{
	case AC_CLOSE:
	case AP_TERM:
	case AP_RESCHG:
		if (message_thread != NULL)
			message_thread(mesag);
		return -1;
	}

	if ((index = search_window(mesag[3])) >= 0)
	{
		window = &win_array[index];
		wind_get_grect(mesag[3], WF_WORKXYWH, &act);

		switch (mesag[0])
		{
		case WM_REDRAW:
			act.g_x = mesag[4];
			act.g_y = mesag[5];
			act.g_w = mesag[6];
			act.g_h = mesag[7];
			do_redraw(mesag[3], &act, ROOT);
			break;
		case WM_TOPPED:
			wind_set_int(mesag[3], WF_TOP, 0);
			break;
		case WM_BOTTOMED:
			wind_set_int(mesag[3], WF_BOTTOM, 0);
			break;
		case WM_MOVED:
			wind_set_grect(mesag[3], WF_CURRXYWH, (GRECT *)&mesag[4]);
			wind_get_grect(mesag[3], WF_WORKXYWH, &act);
			window->tree->ob_x = act.g_x - 1;
			window->tree->ob_y = act.g_y - 1;
			break;
		case WM_SIZED:
			mesag[6] = MAX(mesag[6], 120);
			mesag[7] = MAX(mesag[7], 80);
			wind_calc_grect(WC_WORK, window->kind, (GRECT *)&mesag[4], &act);
			act.g_w = (window->kind & HSLIDE) ? MIN(act.g_w, window->width) : window->width;
			act.g_h = (window->kind & VSLIDE) ? MIN(act.g_h, window->height) : window->height;
			wind_calc_grect(WC_BORDER, window->kind, &act, (GRECT *)&mesag[4]);
			wind_set_grect(mesag[3], WF_CURRXYWH, (GRECT *)&mesag[4]);
			set_slider(index);
			break;
		case WM_ARROWED:
			switch (mesag[4])
			{
			case WA_UPPAGE:
				window->y_pos -= act.g_h;
				break;
			case WA_DNPAGE:
				window->y_pos += act.g_h;
				break;
			case WA_UPLINE:
				window->y_pos -= act.g_h / 10;
				break;
			case WA_DNLINE:
				window->y_pos += act.g_h / 10;
				break;
			case WA_LFPAGE:
				window->x_pos -= act.g_w;
				break;
			case WA_RTPAGE:
				window->x_pos += act.g_w;
				break;
			case WA_LFLINE:
				window->x_pos -= act.g_w / 10;
				break;
			case WA_RTLINE:
				window->x_pos += act.g_w / 10;
				break;
			}
			if (window->width > act.g_w)
			{
				window->x_pos = MAX(0, MIN(window->x_pos, window->width - act.g_w));
				pos = (int) (window->x_pos * 1000L / (window->width - act.g_w));
			} else
			{
				window->x_pos = pos = 0;
			}
			wind_set_int(mesag[3], WF_HSLIDE, pos);
			if (window->height > act.g_h)
			{
				window->y_pos = MAX(0, MIN(window->y_pos, window->height - act.g_h));
				pos = (_WORD) (window->y_pos * 1000L / (window->height - act.g_h));
			} else
			{
				window->y_pos = pos = 0;
			}
			wind_set_int(mesag[3], WF_VSLIDE, pos);
			do_redraw(mesag[3], &desk, ROOT);
			break;
		case WM_HSLID:
			wind_set_int(mesag[3], WF_HSLIDE, mesag[4]);
			window->x_pos = (_WORD) (((long) mesag[4] * (window->width - act.g_w)) / 1000);
			do_redraw(mesag[3], &desk, ROOT);
			break;
		case WM_VSLID:
			wind_set_int(mesag[3], WF_VSLIDE, mesag[4]);
			window->y_pos = (_WORD) (((long) mesag[4] * (window->height - act.g_h)) / 1000);
			do_redraw(mesag[3], &desk, ROOT);
			break;
		case WM_ICONIFY:
			window->icnfy_flg = TRUE;
			wind_set_grect(mesag[3], WF_ICONIFY, (GRECT *)&mesag[4]);
			wind_set_str(mesag[3], WF_NAME, window->short_name);
			break;
		case WM_UNICONIFY:
			window->icnfy_flg = FALSE;
			wind_set_grect(mesag[3], WF_UNICONIFY, (GRECT *)&mesag[4]);
			wind_set_str(mesag[3], WF_NAME, window->full_name);
			break;
		case WM_CLOSED:
			return -2;
		default:
			if (message_thread != NULL)
			{
				if (message_thread(mesag) < 0)
					return -3;
			}
		}

		return 0;
	}

	if (message_thread != NULL)
	{
		if (message_thread(mesag) < 0)
			return -3;
	}

	return 0;
}


_WORD operate_events(void)
{
	WIN_DESC *win;
	_WORD index;
	_WORD kind = MU_KEYBD | MU_BUTTON | MU_MESAG;
	_WORD event;
	_WORD message[8];
	_WORD m_x, m_y;
	_WORD butt;
	_WORD kbd;
	_WORD scan;
	_WORD num;
	_WORD tmp;
	_WORD flag;
	long before;

	more_time = delay;

	for (;;)
	{
		if (window_count == 0)
			return -3;

		win = &win_array[MAX_WIN - 1];
		do
		{
			if (win->tree_index >= 0)
				if (win->next != 0 && win->next != win->edit)
				{
					win->edit = win->next;
					win->next = 0;
					objc_edit(win->tree, win->edit, 0, &win->ed_char, ED_INIT);
				}
		} while (--win >= &win_array[0]);

		if (delay >= 0)
		{
			before = Supexec(read_timer);
			event = kind | MU_TIMER;
		} else
		{
			before = 0;
			event = kind;
		}

		event = evnt_multi(event, 2, 1, 1,
			0, 0, 0, 0, 0,
			0, 0, 0, 0, 0,
			message, more_time,
			&m_x, &m_y, &butt, &kbd, &scan, &num);

		if (delay >= 0)
			more_time -= Supexec(read_timer) - before;

		if (event_thread != NULL)
		{
			if (event_thread())
				return 0;
		}

		wind_get_int(0, WF_TOP, &index);
		if ((index = search_window(index)) >= 0)
			win = &win_array[index];
		else
			win = NULL;

		if (event & MU_KEYBD)
		{
			if (win)
			{
				if (form_keybd(win->tree, win->edit, win->next, scan, &win->next, &scan) == 0)
				{
					if (win->object_click != NULL)
					{
						if (win->object_click(win->next))
							if (finish(index, 0))
								return -1;
					} else
					{
						win->next = 0;
						return 1;
					}
					win->next = 0;
				}
				if (scan != 0)
				{
					if (win->key_typed != NULL)
					{
						tmp = win->key_typed(scan);
						if (tmp > 0)
							objc_edit(win->tree, win->edit, scan, &win->ed_char, ED_CHAR);
						if (tmp < 0)
							if (finish(index, 0))
								return -1;
					} else
					{
						objc_edit(win->tree, win->edit, scan, &win->ed_char, ED_CHAR);
					}
				}
			}
		}

		if (event & MU_MESAG)
		{
			if (message[0] == MN_SELECTED)
			{
				if (menu_thread != NULL)
					if (menu_thread(message[3], message[4]))
						return 0;
			} else
			{
				if ((flag = do_message_event(message)) < 0)
				{
					if (flag == -2)
					{
						if ((index = search_window(message[3])) >= 0)
							if (finish(index, CLOSER_CLICKED))
								return -1;
					} else
					{
						for (index = 0; index < MAX_WIN; index++)
							finish(index, CLOSER_CLICKED);
						return flag == -1 ? -4 : -2;
					}
				}
			}
		}

		if (event & MU_BUTTON)
		{
			if ((index = search_window(wind_find(m_x, m_y))) >= 0)
				if (win_array[index].tree_index >= 0)
				{
					win = &win_array[index];
					if (!win->icnfy_flg)
					{
						win->next = objc_find(win->tree, ROOT, MAX_DEPTH, m_x, m_y);
						if (form_button(win->tree, win->next, num, &win->next) == 0)
						{
							if (win->object_click != NULL)
							{
								if (win->object_click(win->next))
									if (finish(index, 0))
										return -1;
							}
							win->next = 0;
							if (win->object_click == NULL)
								return 1;
						}
					}
				}
		}

		if (event & MU_TIMER && more_time <= 0)
		{
			more_time = delay;
			if (timer_thread != NULL)
			{
				if (timer_thread())
					return 0;
			} else
				return 0;
		}

		win = &win_array[MAX_WIN - 1];
		do
		{
			if (win->tree_index >= 0)
			{
				if (win->next != 0 && win->next != win->edit)
					objc_edit(win->tree, win->edit, 0, &win->ed_char, ED_END);
			}
		} while (--win >= &win_array[0]);
	}
}


void interupt_editing(_WORD rsc_tree, _WORD mode, _WORD new_edit)
{
	WIN_DESC *window;
	_WORD index;

	if ((index = search_tree(rsc_tree)) < 0)
		return;

	window = &win_array[index];

	if (new_edit >= 0 && mode != BEGIN)
		window->edit = new_edit;

	if (window->edit != 0)
	{
		objc_edit(window->tree, window->edit, 0, &window->ed_char, (mode == BEGIN) ? ED_END : ED_INIT);
	}
	if (mode == BEGIN)
		window->edit = 0;
}


void change_rsc_size(_WORD rsc_tree, _WORD new_width, _WORD new_height, _WORD parent)
{
	WIN_DESC *window;
	OBJECT *tree;
	GRECT pos;
	GRECT all;
	_WORD index;
	_WORD tmp_x, tmp_y;

	rsrc_gaddr(R_TREE, rsc_tree, &tree);

	if (new_width > 0)
		tree->ob_width = new_width;
	if (new_height > 0)
		tree->ob_height = new_height;

	if ((index = search_tree(rsc_tree)) < 0)
		return;
	window = &win_array[index];

	tmp_x = tree->ob_x;
	tmp_y = tree->ob_y;
	form_center_grect(tree, &pos);
	tree->ob_x = tmp_x;
	tree->ob_y = tmp_y;
	window->width = pos.g_w - 2;
	window->height = pos.g_h - 2;

	wind_calc(WC_BORDER, window->kind, pos.g_x + 1, pos.g_y + 1, pos.g_w - 2, pos.g_h - 2, &all.g_x, &all.g_y, &all.g_w, &all.g_h);
	wind_get_grect(window->window_handle, WF_CURRXYWH, &pos);

	if (new_width > 0)
	{
		if ((window->kind & HSLIDE) == 0 || pos.g_w > all.g_w)
			pos.g_w = all.g_w;
	}
	if (new_height > 0)
	{
		if ((window->kind & VSLIDE) == 0 || pos.g_h > all.g_h)
			pos.g_h = all.g_h;
	}
	wind_set_grect(window->window_handle, WF_CURRXYWH, &pos);
	set_slider(index);

	if (!window->icnfy_flg)
		if (parent >= 0)
			do_redraw(window->window_handle, &desk, parent);
}


void change_freestring(_WORD rsc_tree, _WORD object, _WORD parent, const char *text, _WORD length)
{
	OBJECT *tree;
	_WORD index;

	rsrc_gaddr(R_TREE, rsc_tree, &tree);
	strncpy(tree[object].ob_spec.free_string, text, length);

	if ((index = search_tree(rsc_tree)) < 0)
		return;

	if (!win_array[index].icnfy_flg)
		if (parent >= 0)
			do_redraw(win_array[index].window_handle, &desk, parent);
}


void change_tedinfo(_WORD rsc_tree, _WORD object, _WORD parent, _WORD which, const char *text, _WORD length)
{
	OBJECT *tree;
	TEDINFO *ted;
	_WORD index;
	char *text_ptr;

	rsrc_gaddr(R_TREE, rsc_tree, &tree);
	ted = tree[object].ob_spec.tedinfo;

	switch (which)
	{
	case TE_PTEXT:
		text_ptr = ted->te_ptext;
		break;
	case TE_PTMPLT:
		text_ptr = ted->te_ptmplt;
		break;
	case TE_PVALID:
		text_ptr = ted->te_pvalid;
		break;
	default:
		return;
	}
	strncpy(text_ptr, text, length);

	if ((index = search_tree(rsc_tree)) < 0)
		return;

	if (!win_array[index].icnfy_flg)
		if (parent >= 0)
			do_redraw(win_array[index].window_handle, &desk, parent);
}


void change_flags(_WORD rsc_tree, _WORD object, _WORD chg_flag, _WORD flags, _WORD state)
{
	OBJECT *obj_ptr;
	_WORD index;

	if (object == 0 || object == CLOSER_CLICKED)
		return;

	rsrc_gaddr(R_TREE, rsc_tree, &obj_ptr);
	obj_ptr = &obj_ptr[object];

	if (chg_flag)
		obj_ptr->ob_flags |= flags, obj_ptr->ob_state |= state;
	else
		obj_ptr->ob_flags &= ~flags, obj_ptr->ob_state &= ~state;

	if ((index = search_tree(rsc_tree)) < 0)
		return;

	if (!win_array[index].icnfy_flg)
		do_redraw(win_array[index].window_handle, &desk, object);
}


_WORD top_rsc_window(_WORD rsc_tree)
{
	_WORD index;

	index = search_tree(rsc_tree);

	if (index >= 0)
		wind_set_int(win_array[index].window_handle, WF_TOP, 0);

	return index >= 0 ? 0 : -1;
}


void pop_up(_WORD popup_ind, _WORD *object, _WORD dial_ind, _WORD str_obj, _WORD length)
{
	OBJECT *pu;
	OBJECT *dial;
	OBJECT *wrk;
	GRECT box;
	char *chr;
	_WORD button;
	_WORD zwsp;
	_WORD p_x, p_y;
	_WORD t_x, t_y;
	_WORD xy[8];
	_WORD abort_flg = FALSE;
	_WORD event;
	_WORD state;
	_WORD butt;
	_WORD dummy;
	_WORD kret;
	_WORD bret;

	rsrc_gaddr(R_TREE, popup_ind, &pu);
	objc_offset(pu, *object, &t_x, &t_y);
	rsrc_gaddr(R_TREE, dial_ind, &dial);
	objc_offset(dial, str_obj, &p_x, &p_y);
	pu->ob_x += p_x - t_x;
	pu->ob_y += p_y - t_y;

	pu->ob_x = MIN(desk.g_x + desk.g_w - pu->ob_width - 3, pu->ob_x);
	pu->ob_x = MAX(desk.g_x + 3, pu->ob_x);
	pu->ob_y = MIN(desk.g_y + desk.g_h - pu->ob_height - 3, pu->ob_y);
	pu->ob_y = MAX(desk.g_y + 3, pu->ob_y);

	box.g_x = pu->ob_x - 1;
	box.g_w = pu->ob_width + 4;
	box.g_y = pu->ob_y - 1;
	box.g_h = pu->ob_height + 4;

	wrk = pu;
	for (;;)
	{
		wrk->ob_state &= ~OS_CHECKED;
		if (wrk->ob_flags & OF_LASTOB)
			break;
		wrk++;
	}
	button = *object;
	pu[button].ob_state |= OS_CHECKED | OS_SELECTED;

	popup.fd_w = box.g_w;
	popup.fd_h = box.g_h;
	popup.fd_nplanes = planes;
	popup.fd_wdwidth = (box.g_w + 15) / 16;
	popup.fd_addr = Malloc(planes * (box.g_h * (box.g_w + 15L) / 8));

	xy[0] = box.g_x;
	xy[2] = box.g_x + box.g_w - 1;
	xy[1] = box.g_y;
	xy[3] = box.g_y + box.g_h - 1;
	xy[4] = xy[5] = 0;
	xy[6] = box.g_w - 1;
	xy[7] = box.g_h - 1;

	wind_update(BEG_UPDATE);
	wind_update(BEG_MCTRL);

	if (popup.fd_addr)
	{
		graf_mouse(M_OFF, NULL);
		vro_cpyfm(vdi_handle, S_ONLY, xy, &screen, &popup);
		graf_mouse(M_ON, NULL);
		graf_mouse(ARROW, NULL);
	} else
		form_dial(FMD_START, 0, 0, 0, 0, box.g_x, box.g_y, box.g_w, box.g_h);

	graf_mkstate(&dummy, &dummy, &butt, &dummy);
	state = (butt & 1) ? 0 : 1;

	objc_draw(pu, ROOT, MAX_DEPTH, box.g_x, box.g_y, box.g_w, box.g_h);

	do
	{
		objc_offset(pu, button, &p_x, &p_y);
		event = evnt_multi(MU_KEYBD | MU_BUTTON | MU_M1, 1, 1, state, (button > 0),
			p_x, p_y, pu[button].ob_width, pu[button].ob_height,
			0, 0, 0, 0, 0,
			NULL, 0,
			&t_x, &t_y, &butt, &dummy, &kret, &bret);

		if ((event & MU_KEYBD) || (event & MU_BUTTON))
			abort_flg = TRUE;
		pu[button].ob_state &= ~OS_SELECTED;

		if (event & MU_KEYBD)
			button = 0;
		if (event & MU_M1)
		{
			if (button)
				objc_draw(pu, ROOT, 2, p_x, p_y, pu[button].ob_width, pu[button].ob_height);
			button = objc_find(pu, ROOT, MAX_DEPTH, t_x, t_y);
			if (button > 0)
			{
				if ((pu[button].ob_state & OS_DISABLED) == 0)
				{
					pu[button].ob_state |= OS_SELECTED;
					objc_draw(pu, button, 1, box.g_x, box.g_y, box.g_w, box.g_h);
				} else
					button = 0;
			} else
				button = 0;
		}
	} while (!abort_flg);

	SWAP(xy[0], xy[4]);
	SWAP(xy[1], xy[5]);
	SWAP(xy[2], xy[6]);
	SWAP(xy[3], xy[7]);

	if (popup.fd_addr)
	{
		graf_mouse(M_OFF, NULL);
		vro_cpyfm(vdi_handle, S_ONLY, xy, &popup, &screen);
		graf_mouse(M_ON, NULL);
		Mfree(popup.fd_addr);
	} else
	{
		form_dial(FMD_FINISH, 0, 0, 0, 0, box.g_x, box.g_y, box.g_w, box.g_h);
	}
	
	wind_update(END_MCTRL);
	wind_update(END_UPDATE);

	if (button)
	{
		for (chr = pu[button].ob_spec.free_string; *chr == ' '; chr++) ;
		change_freestring(dial_ind, str_obj, str_obj, chr, length);
		*object = button;
	}
}


