/*  CPX DATA STRUCTURES
 *==========================================================================
 *  XCPB structure is passed TO the CPX
 *  CPXINFO structure pointer is returned FROM the CPX
 *
 *  xcpb structure is initialized in XCONTROL.C
 */

#if !defined(__foobar_defined) && !defined(T0OBJ)
#define __foobar_defined 1
struct foobar
{
	WORD dummy;
	WORD *image;
};
#endif

typedef struct
{
	short vdi_handle;
	short booting;
	short reserved;
	short rsc_init;

	void *cdecl (*Get_Head_Node) (void);	/* ON distribution disk...         */
	WORD cdecl (*Save_Header) (void *ptr);	/* These 2 would be void *reserved */

	void cdecl (*do_resource) (WORD num_obs, WORD num_frstr, WORD num_frimg,
						  WORD num_tree, OBJECT *rs_object,
						  TEDINFO *rs_tedinfo, BYTE *rs_strings[],
						  ICONBLK *rs_iconblk, BITBLK *rs_bitblk,
						  long *rs_frstr, long *rs_frimg, long *rs_trindex, struct foobar *rs_imdope);

	void cdecl (*rsh_obfix) (OBJECT *tree, WORD curob);

	WORD cdecl (*do_pulldown) (char **items, WORD num_items, WORD default_item, WORD font_size, GRECT *button, GRECT *world);

	void cdecl (*Sl_size) (OBJECT *tree, WORD base, WORD slider, WORD num_items, WORD visible, WORD direction, WORD min_size);
#define VERTICAL	0
#define HORIZONTAL	1

	void cdecl (*Sl_x) (OBJECT *tree, WORD base, WORD slider, WORD value, WORD num_min, WORD num_max, void cdecl (*foo) (void));

	void cdecl (*Sl_y) (OBJECT *tree, WORD base, WORD slider, WORD value, WORD num_min, WORD num_max, void cdecl (*foo) (void));

	void cdecl (*Sl_arrow) (OBJECT *tree, WORD base, WORD slider, WORD obj,
						   WORD inc, WORD min, WORD max, WORD *numvar, WORD direction, void cdecl (*foo) (void));

	void cdecl (*Sl_dragx) (OBJECT *tree, WORD base, WORD slider, WORD min, WORD max, WORD *numvar, void cdecl (*foo) (void));

	void cdecl (*Sl_dragy) (OBJECT *tree, WORD base, WORD slider, WORD min, WORD max, WORD *numvar, void cdecl (*foo) (void));

	WORD cdecl (*do_form) (OBJECT *tree, WORD start_field, WORD puntmsg[]);
	GRECT *cdecl (*GetFirstRect) (GRECT *prect);
	GRECT *cdecl (*GetNextRect) (void);

	void cdecl (*Set_Evnt_Mask) (WORD mask, MOBLK *m1, MOBLK *m2, long time);
	WORD cdecl (*alert) (WORD id);
#define SAVE_DEFAULTS	0
#define MEM_ERR			1
#define FILE_ERR		2
#define FILE_NOT_FOUND	3

	WORD cdecl (*write_config) (void *ptr, long num);
	void *cdecl (*Get_Buffer) (void);

	WORD cdecl (*find_cookie) (long cookie, long *p_value);

	WORD Country_Code;

	void cdecl (*MFsave) (WORD saveit, MFORM *mf);
#define MFSAVE 1
#define MFRESTORE 0
} CPX_PARAMS;


/* MRETS struct for mouse parameters returned by
 * Evnt_button(), Evnt_mouse(), Evnt_multi(), Graf_mkstate()
 */
typedef struct
{
	WORD x;
	WORD y;
	WORD buttons;
	WORD kstate;
} MRETS;


typedef struct
{
	WORD cdecl (*cpx_call) (GRECT *work);

	void cdecl (*cpx_draw) (GRECT *clip);
	void cdecl (*cpx_wmove) (GRECT *work);

	void cdecl (*cpx_timer) (WORD *event);
	void cdecl (*cpx_key) (WORD kstate, WORD key, WORD *event);
	void cdecl (*cpx_button) (MRETS *mrets, WORD nclicks, WORD *event);
	void cdecl (*cpx_m1) (MRETS *mrets, WORD *event);
	void cdecl (*cpx_m2) (MRETS *mrets, WORD *event);
	WORD cdecl (*cpx_hook) (WORD event, WORD *msg, MRETS *mrets, WORD *key, WORD *nclicks);

	void cdecl (*cpx_close) (WORD flag);
} CPX_INFO;

#ifndef GNU_ASM_NAME
#ifdef __GNUC__
#define GNU_ASM_NAME(x) __asm__(x)
#else
#define GNU_ASM_NAME(x)
#endif
#endif

CPX_INFO *cdecl cpx_init(CPX_PARAMS *Xcpb) GNU_ASM_NAME("cpx_init");
