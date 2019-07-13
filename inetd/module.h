#include "transprt.h"

/*
 *   Structures for interfacing to INetD ...
 */
typedef struct ism_api
{
	void (*ext_objects) (_WORD ism_index, _WORD rsc);
	void (*set_trees) (_WORD ism_index, OBJECT *trees[], _WORD global[]);
	_WORD (*open_window) (_WORD ism_index, _WORD rsc, _WORD edit);
	_WORD (*close_window) (_WORD ism_index, _WORD rsc);
	void (*set_callbacks) (_WORD ism_index, _WORD rsc, int (*click)(_WORD obj), int (*key)(unsigned short scan));
	void (*change_flags) (_WORD ism_index, _WORD rsc, _WORD obj, _WORD what, _WORD flags, _WORD state);
	void (*do_popup) (_WORD ism_index, _WORD pu, _WORD *sel, _WORD par, _WORD obj, _WORD len);
	void (*editing) (_WORD ism_index, _WORD rsc, _WORD what, _WORD edit);
	_WORD (*top_window) (_WORD ism_index, _WORD rsc);
	void (*rsc_size) (_WORD ism_index, _WORD rsc, _WORD width, _WORD height, _WORD parent);
	void (*free_string) (_WORD ism_index, _WORD rsc, _WORD obj, _WORD par, char txt[], _WORD len);
	void (*tedinfo) (_WORD ism_index, _WORD rsc, _WORD obj, _WORD par, _WORD w, char txt[], _WORD len);
	void (*finish_user) (_WORD ism_index);
	void (*finish_server) (_WORD ism_index);
} ISM_API;

typedef struct ism_para
{
	void *module_resident;
	short index;
	_WORD char_width;
	_WORD char_height;
	short protocol;
	short connection;
	ISM_API *server_api;
} ISM_PARA;

typedef struct ism_specs ISM_SPECS;

struct ism_specs
{
	ISM_SPECS *(*ism_init)(ISM_PARA *module_data);
	void (*ism_term)(ISM_PARA *module_data);
	void (*ism_user)(ISM_PARA *module_data);
	void (*ism_server)(ISM_PARA *module_data);
	ICONBLK *ism_icon;
	short ism_num_trees;
	short ism_protocol;
	short ism_tos;
	char ism_name[17];
};

#define  ISM_UDP       1
#define  ISM_TCP       2

#define  ACT_USER      1
#define  ACT_SERVER    2

typedef struct ism_internals
{
	char ism_name[17];
	char ism_ictxt[13];
	char ism_icon[96];
	char file[14];
	char resident[64];
	short action;
	short rsc_offset;
	short rsc_num;
	short protocol;
	short tos;
	long ism_dterm;
	long ism_duser;
	long ism_dserver;
	ISM_SPECS *(*ism_init)(ISM_PARA *module_data);
	void (*ism_term)(ISM_PARA *module_data);
	void (*ism_user)(ISM_PARA *module_data);
	void (*ism_server)(ISM_PARA *module_data);
	BASPAG *basepage;
} ISM_INTERN;


#ifndef GNU_ASM_NAME
#ifdef __GNUC__
#define GNU_ASM_NAME(x) __asm__(x)
#else
#define GNU_ASM_NAME(x)
#endif
#endif

ISM_SPECS *module_init(ISM_PARA *module_data) GNU_ASM_NAME("module_init");


/*
 *   for handling Dialogs in Windows ...
 */

#define  BEGIN            1
#define  END              2

#define  TE_PTEXT         0
#define  TE_PTMPLT        1
#define  TE_PVALID        2

#define  CLOSER_CLICKED   0x7654
