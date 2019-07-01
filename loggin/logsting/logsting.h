#define  UINT8      0
#define  INT8       1
#define  UINT16     2
#define  INT16      3
#define  UINT32     4
#define  INT32      5
#define  STRING     6
#define  IP_ADDR    7


typedef struct logstruc
{
	const char *version;
	short cli_num;
	short mod_num;
	unsigned char *generic;
	unsigned char *client_itf;
	unsigned char *module_itf;
	char *path;
} LOGSTRUC;

#define  CLI_NUM    37
#define  MOD_NUM    20


extern LOGSTRUC log_this;
extern int offset;
extern unsigned char generic[7];
extern unsigned char cli_flags[CLI_NUM];
extern unsigned char mod_flags[MOD_NUM];

long deinstall(void);
const char *get_error(int16 error);
void write_log_text(const char *text);
void write_function(const char *name);
void write_parameter(const char *name, int type, const void *value, const char *supple);
void write_buffer(const void *buffer, int length);

void install_api(TPL *sting_tpl, STX *sting_stx, DRV_LIST *sting_drivers);
void uninstall_api(DRV_LIST *sting_drivers);

