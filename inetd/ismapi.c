#include <aes.h>
#include <tos.h>
#include <stdio.h>

#include "inetd.h"
#include "module.h"
#include "window.h"


static ISM_API my_api;



static void api_ext_objects(_WORD ism_index, _WORD rsc_tree)
{
	ISM_INTERN *ism = &ism_data[ism_index];
	OBJECT *tree;

	rsrc_gaddr(R_TREE, rsc_tree + ism->rsc_offset, &tree);
	rsc_ext_objects(tree);
}


static void api_set_trees(_WORD ism_index, OBJECT **tree_array, _WORD global[])
{
	ISM_INTERN *ism = &ism_data[ism_index];
	_WORD count;
	_WORD wchar = parameter.char_width;
	_WORD hchar = parameter.char_height;

	for (count = 0; count < ism->rsc_num; count++)
	{
		OBJECT *tree = tree_array[count];
		my_tree_index[ism->rsc_offset + count] = tree;
		for (;;)
		{
			tree->ob_x = wchar * (tree->ob_x & 0xff) + (tree->ob_x >> 8);
			tree->ob_y = hchar * (tree->ob_y & 0xff) + (tree->ob_y >> 8);
			tree->ob_width = wchar * (tree->ob_width & 0xff) + (tree->ob_width >> 8);
			tree->ob_height = hchar * (tree->ob_height & 0xff) + (tree->ob_height >> 8);
			if (tree->ob_flags & OF_LASTOB)
				break;
			tree++;
		}
	}
	
	for (count = 0; count < 15; count++)
		global[count] = aes_global[count];
}


static _WORD api_open_window(_WORD ism_index, _WORD rsc_tree, _WORD edit)
{
	ISM_INTERN *ism = &ism_data[ism_index];

	rsc_tree += ism->rsc_offset;

	return open_rsc_window(rsc_tree, edit, ism->ism_name, "ISM", START);
}


static _WORD api_close_window(_WORD ism_index, _WORD rsc_tree)
{
	ISM_INTERN *ism = &ism_data[ism_index];

	rsc_tree += ism->rsc_offset;

	return close_rsc_window(rsc_tree, -1);
}


static void api_set_callbacks(_WORD ism_index, _WORD rsc_tree, int (*click_func)(_WORD obj), int (*key_func)(unsigned short scan))
{
	ISM_INTERN *ism = &ism_data[ism_index];

	rsc_tree += ism->rsc_offset;

	set_callbacks(rsc_tree, click_func, key_func);
}


static void api_change_flags(_WORD ism_index, _WORD rsc_tree, _WORD object, _WORD to_do, _WORD flags, _WORD state)
{
	ISM_INTERN *ism = &ism_data[ism_index];

	rsc_tree += ism->rsc_offset;

	change_flags(rsc_tree, object, to_do, flags, state);
}


static void api_do_popup(_WORD ism_index, _WORD pu_tree, _WORD *selection, _WORD par_tree, _WORD par_object, _WORD length)
{
	ISM_INTERN *ism = &ism_data[ism_index];

	pu_tree += ism->rsc_offset;
	par_tree += ism->rsc_offset;

	pop_up(pu_tree, selection, par_tree, par_object, length);
}


static void api_int_editing(_WORD ism_index, _WORD rsc_tree, _WORD what, _WORD new_edit)
{
	ISM_INTERN *ism = &ism_data[ism_index];

	rsc_tree += ism->rsc_offset;

	interupt_editing(rsc_tree, what, new_edit);
}


static _WORD api_top_window(_WORD ism_index, _WORD rsc_tree)
{
	ISM_INTERN *ism = &ism_data[ism_index];

	rsc_tree += ism->rsc_offset;

	return top_rsc_window(rsc_tree);
}


static void api_rsc_size(_WORD ism_index, _WORD rsc_tree, _WORD width, _WORD height, _WORD parent)
{
	ISM_INTERN *ism = &ism_data[ism_index];

	rsc_tree += ism->rsc_offset;

	change_rsc_size(rsc_tree, width, height, parent);
}


static void api_free_string(_WORD ism_index, _WORD rsc_tree, _WORD object, _WORD parent, char *text, _WORD length)
{
	ISM_INTERN *ism = &ism_data[ism_index];

	rsc_tree += ism->rsc_offset;

	change_freestring(rsc_tree, object, parent, text, length);
}


static void api_tedinfo(_WORD ism_index, _WORD rsc_tree, _WORD object, _WORD parent, _WORD which, char *text, _WORD length)
{
	ISM_INTERN *ism = &ism_data[ism_index];

	rsc_tree += ism->rsc_offset;

	change_tedinfo(rsc_tree, object, parent, which, text, length);
}


static void api_finish_user(_WORD ism_index)
{
	ISM_INTERN *ism = &ism_data[ism_index];

	ism->action &= ~ACT_USER;
}


static void api_finish_server(_WORD ism_index)
{
	ISM_INTERN *ism = &ism_data[ism_index];

	ism->action &= ~ACT_SERVER;
}


void set_api_struct(void)
{
	_WORD dummy;
	
	my_api.ext_objects = api_ext_objects;
	my_api.set_trees = api_set_trees;
	my_api.open_window = api_open_window;
	my_api.close_window = api_close_window;
	my_api.set_callbacks = api_set_callbacks;
	my_api.change_flags = api_change_flags;
	my_api.do_popup = api_do_popup;
	my_api.editing = api_int_editing;
	my_api.top_window = api_top_window;
	my_api.rsc_size = api_rsc_size;
	my_api.free_string = api_free_string;
	my_api.tedinfo = api_tedinfo;
	my_api.finish_user = api_finish_user;
	my_api.finish_server = api_finish_server;

	parameter.server_api = &my_api;
	graf_handle(&parameter.char_width, &parameter.char_height, &dummy, &dummy);
}
