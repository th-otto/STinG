/*
 *   Library for handling Dialogs in Windows ...
 */

#define  BEGIN            1
#define  END              2

#define  TE_PTEXT         0
#define  TE_PTMPLT        1
#define  TE_PVALID        2

#define  CLOSER_CLICKED   0x7654

#define  CNTRL_Q        0x1011
#define  CNTRL_C         0x2e03
#define  ESC            0x11b

#define  S_NONE         0
#define  S_DIAL         1
#define  S_SCRIPT       2
#define  S_CONNECT      3
#define  S_BATCH        4

#define  MIN(a,b)     (((a) < (b)) ? (a) : (b))
#define  MAX(a,b)     (((a) > (b)) ? (a) : (b))

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
_WORD pop_up(_WORD popup_tree, _WORD *object, _WORD dialog_tree, _WORD string_obj, _WORD length);

int get_port(void);
int read_fee(char *buffer);
void write_time_log(const char *mask);
void write_extra_line(void);
void write_log_text(const char *text);


extern _WORD conf_shown;
extern int port_lock;
extern int has_LAN;
extern char script_path[256];
extern char modem_init[];
extern char modem_dial[];
extern char modem_hangup[];
extern int connect_timeout;
extern int redial_delay;
extern char s_conn[3][13];
extern char s_abrt[3][13];
extern char s_redl[3][13];
extern int script_length;
extern int script_timeout;
#define  MAX_SCRIPT_SIZE   32
extern char script[MAX_SCRIPT_SIZE + 1][4][32];
extern int port_flags;
extern int port_mtu;
extern int protocol;
extern int papp_flag;
extern char pap_id[32];
extern char pap_passwd[32];
extern char username[17];
extern char fullname[33];
extern char fqdn[43];
extern int dns_num;
extern int phone_num;
extern char phone[8][17];
extern char fee_file[13];
extern long ISP_u_time;
extern long ISP_u_sent;
extern long ISP_u_recvd;
extern long ISP_c_time;
extern long ISP_c_sent;
extern long ISP_c_recvd;
extern char **environ_base;
extern int environ_number;
extern int num_ports;
extern _WORD act_port;
extern int def_route;
extern int masquerade;
extern int run_tools;
extern int compuserve;
extern int resident;
extern int debugging;
extern char const ip_out[];
extern _WORD const edit[];
extern int max_num_dials;


int init_config_stuff(void);
int conf_click(_WORD object);
void fill_in_config_box(void);
void reset_config(void);
int load_dial_script(const char *file);
uint32 load_ip_addr(unsigned char *ip, const char *str);

extern int dial_state;
extern int digits;
extern int masque_there;


extern char config_path[];
extern const char *ports[];


extern char unit[];

extern unsigned char ip_address[4];
extern unsigned char ip_dns[4][4];

extern void (*dial_timer)(void);
extern void (*mem_timer)(void);
extern void (*stat_timer)(void);
extern void (*ping_timer)(void);
extern void (*trace_timer)(void);

void finish_dial(int return_code);
void spawn_dialer(void);
void spawn_batch(int what);
int hangup(void);


extern DEV_LIST *curr_port;
extern char const no_batch_alert[];
extern int sender;
extern char const ip_space[];
extern char eff_passwd[32];
extern int off_hook;
extern int dialer_delay;
extern const char *batch;

int do_connect(void);
int do_disconnect(void);
int check_port_flags(uint32 *ip_ptr);
char *get_PPP_status(void);
void finish_login(void);
void set_configuration(void);

extern char const hangup_alert[];
extern _WORD stat_port;
extern _WORD stat_layer;
extern _WORD planes;
extern int connected;
extern DEV_LIST *devices;
extern DEV_LIST *curr_port;

int set_mode(int mode, int alert_flag);
void set_stat_string(long value, _WORD index, int redraw);
int get_version(char *sting);
void query_active(int alert_flag);
int init_misc(void);
int en_dis_able(int dtr, int alert);
void read_counter(long *read, long *write);
void set_statistics(void);
void show_statistics(void);

void set_memory(void);
void show_memory(void);
int mem_click(_WORD object);
int mem_key_typed(unsigned short scancode);
int stat_click(_WORD object);
int stat_key_typed(unsigned short scancode);
void set_routing(void);
int routing_click(_WORD object);
int init_resolve(void);
int resolve_click(_WORD object);
int spawn_ping(_WORD object);
int spawn_traceroute(_WORD object);
