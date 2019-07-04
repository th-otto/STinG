/*
 *   Library for handling Dialogs in Windows ...
 */

#ifndef FALSE
#define  FALSE             0
#define  TRUE              1
#endif

#define  CNTRL_Q          0x1011


extern ISM_INTERN *ism_data;
extern int num_modules;
extern char inetd_path[];
extern _WORD conf_shown;
extern ISM_PARA parameter;
extern char *strings;



int initialise_windows(_WORD number_trees, _WORD icnfy_index);
void leave_windows(void);
_WORD open_rsc_window(_WORD rsc_tree, _WORD edit_object, const char *window_name, const char *short_name, _WORD parent_tree);
int close_rsc_window(_WORD rsc_tree, _WORD wind_handle);
void set_callbacks(_WORD rsc_tree, int click_func(_WORD obj), int key_func(unsigned short scan));
void set_timer_callback(int (*timer_func)(void), long timer_delay);
void set_message_callback(int (*message_func)(_WORD *message));
void set_menu_callback(int (*menu_func)(_WORD title, _WORD item));
void set_event_callback(int (*event_func)(void));
_WORD operate_events(void);
void interupt_editing(_WORD rsc_tree, _WORD what, _WORD new_edit);
void change_rsc_size(_WORD rsc_tree, _WORD new_width, _WORD new_height, _WORD parent_obj);
void change_freestring(_WORD rsc_tree, _WORD object, _WORD parent_obj, const char *text, _WORD length);
void change_tedinfo(_WORD rsc_tree, _WORD object, _WORD parent_obj, _WORD which, const char *text, _WORD length);
void change_flags(_WORD rsc_tree, _WORD object, _WORD change_flag, _WORD flags, _WORD state);
_WORD top_rsc_window(_WORD rsc_tree);
void pop_up(_WORD popup_tree, _WORD *object, _WORD dialog_tree, _WORD string_obj, _WORD length);


void init_configs(void);
void fill_in_config_box(void);
int conf_click(_WORD object);
int conf_typed(unsigned short scancode);
void set_tedinfo_text(_WORD rsc_tree, _WORD object, const char *text);
void get_tedinfo_text(_WORD rsc_tree, _WORD object, char *text);
void set_tedinfo_number(_WORD rsc_tree, _WORD object, int number);
void get_tedinfo_number(_WORD rsc_tree, _WORD object, int *number);
void rsc_ext_objects(OBJECT *tree);

extern char ism_path[];
extern int disp_offset;
extern OBJECT **my_tree_index;
extern char const version[];
extern _WORD const mdle_box[];
extern _WORD const click_box[];
extern _WORD const mdle_icon[];
extern _WORD const mdle_name[];
extern _WORD const edit[];

int get_version(char stik[]);
int init_modules(void);
void terminate_modules(void);
void call_module(int which);
int check_modules(void);
void insert_modules(int draw_flag);
int get_version(char *stik_version);

void set_api_struct(void);
