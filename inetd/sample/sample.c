#include <aes.h>
#include <tos.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../module.h"

#undef BYTE
#define BYTE char

#include "sample.h"
#include "sample.rsh"


#define  CNTRL_Q      0x1011

typedef struct our_data
{
	char *our_memory;
	short port;
	short continent;
	short beverage;
} OUR_DATA;

int errno;

static int ism_index;
static ISM_API *api;

static OUR_DATA *my_data_ptr;
static _WORD continent;
static _WORD const radios[] = { COKE, FANTA, SPRITE };



/* This should prepare everything for leaving. Malloc'ed memory must be freed, etc. */

static void module_term(ISM_PARA *module_data)
{
	my_data_ptr = (OUR_DATA *) module_data->module_resident;

	/* The memory we grabbed earlier must be freed now !                               */
	if (my_data_ptr->our_memory)
		Mfree(my_data_ptr->our_memory);
}


static void set_data(void)
{
	OBJECT *tree;
	OBJECT *popup;
	char temp[10];

	tree = rs_trindex[MAIN];
	popup = rs_trindex[PU_TEST];

	sprintf(temp, "%3d", my_data_ptr->port);
	strncpy(tree[PORT].ob_spec.tedinfo->te_ptext, temp, tree[PORT].ob_spec.tedinfo->te_txtlen - 1);

	continent = my_data_ptr->continent;
	strcpy(tree[CONTIN].ob_spec.free_string, popup[continent].ob_spec.free_string + 2);

	tree[COKE].ob_state &= ~OS_SELECTED;
	tree[FANTA].ob_state &= ~OS_SELECTED;
	tree[SPRITE].ob_state &= ~OS_SELECTED;
	tree[radios[my_data_ptr->beverage]].ob_state |= OS_SELECTED;
}


static void get_data(void)
{
	OBJECT *tree;
	int count;

	tree = rs_trindex[MAIN];

	my_data_ptr->port = atoi(tree[PORT].ob_spec.tedinfo->te_ptext);
	my_data_ptr->continent = continent;

	for (count = 0; count < 2; count++)
		if (tree[radios[count]].ob_state & OS_SELECTED)
			break;
	my_data_ptr->beverage = count;
}


static int click_function(_WORD object)
{
	evnt_timer(60);
	api->change_flags(ism_index, MAIN, object & 0x7fff, 0, 0, OS_SELECTED);

	switch (object & 0x7fff)
	{
	case CLOSER_CLICKED:
		api->finish_user(ism_index);
		break;
	case CONTIN:
		api->do_popup(ism_index, PU_TEST, &continent, MAIN, CONTIN, 14);
		break;
	case SAVE:
		form_alert(1, "[3][  |  Can't save, we're just a   | |     sample module !][ Ok ]");
		break;
	case ACCEPT:
		get_data();
		break;
	case CANCEL:
		api->finish_user(ism_index);
		return -1;
	}

	return 0;
}


static int key_function(unsigned short scancode)
{
	return scancode == CNTRL_Q ? -1 : 1;
}


/* This is all the interaction with the user, only place to use resource trees ! */

static void module_user(ISM_PARA *module_data)
{
	ism_index = module_data->index;
	api = module_data->server_api;

	my_data_ptr = (OUR_DATA *) module_data->module_resident;

	/* Tell the INetD the addresses of our resource trees.                             */
	module_data->server_api->set_trees(ism_index, rs_trindex, aes_global);

	/* We do use extended objects in our resource trees.                               */
	module_data->server_api->ext_objects(ism_index, MAIN);

	/* Set all input fields and buttons to default values.                             */
	set_data();

	/* Open main window and let INetD operate clicks and keys.                         */
	module_data->server_api->open_window(ism_index, MAIN, PORT);
	api->set_callbacks(ism_index, MAIN, click_function, key_function);
}


static void module_server(ISM_PARA *module_data)
{
	ism_index = module_data->index;
	api = module_data->server_api;

	api->finish_server(ism_index);
}


static ISM_SPECS my_specs = {
	module_init,
	module_term,
	module_user,
	module_server,
	NULL,
	NUM_TREE,
	ISM_UDP | ISM_TCP,
	0,
	"Sample Server"
};


/* This initializes everything. Resident memory can be ordered, etc. */

ISM_SPECS *module_init(ISM_PARA *module_data)
{
	OBJECT *tree;

	ism_index = module_data->index;
	api = module_data->server_api;

	my_data_ptr = (OUR_DATA *) module_data->module_resident;

	/* We'd just like a little more memory, so we grab it. As the 'module_data'        */
	/* structure is the only part which is resident, the pointer must be saved there ! */
	my_data_ptr->our_memory = (char *) Malloc(1024L);

	/* Set the default values for all the variables here !                             */
	my_data_ptr->port = 123;
	my_data_ptr->continent = 6;
	my_data_ptr->beverage = 0;

	/* We wanna return the icon, thus resource structure must be alright.              */
	api->set_trees(ism_index, rs_trindex, aes_global);

	/* Put icon into structure to be passed to the INetD.                              */
	tree = rs_trindex[ICON];
	my_specs.ism_icon = tree[MY_ICON].ob_spec.iconblk;

	return &my_specs;
}
