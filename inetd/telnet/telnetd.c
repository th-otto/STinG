#include <aes.h>
#include <tos.h>
#include <stdio.h>

#include "../module.h"

#undef BYTE
#define BYTE char

#include "telnetd.h"
#include "telnetd.rsh"


static int ism_index;
static ISM_API *api;


static void module_term(ISM_PARA *module_data)
{
	(void)module_data;
}


static void module_user(ISM_PARA *module_data)
{
	ism_index = module_data->index;
	api = module_data->server_api;

	api->set_trees(ism_index, rs_trindex, aes_global);

	form_alert(1, "[1][ |  Nothin' here, instead try the   | |     Sample Server !][ Ok ]");

	api->finish_user(ism_index);
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
	ISM_TCP,
	0,
	"Telnet Server"
};


ISM_SPECS *module_init(ISM_PARA *module_data)
{
	OBJECT *tree;

	ism_index = module_data->index;
	api = module_data->server_api;
	api->set_trees(ism_index, rs_trindex, aes_global);

	tree = rs_trindex[ICON];
	my_specs.ism_icon = tree[MY_ICON].ob_spec.iconblk;

	return &my_specs;
}


