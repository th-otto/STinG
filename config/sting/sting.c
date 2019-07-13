/*********************************************************************/
/*                                                                   */
/*     Konfiguration der STinG Spezial Funktionen                    */
/*                                                                   */
/*                                                                   */
/*      CPX-Version 1.01                 vom 17. Dezember 1996       */
/*                                                                   */
/*      zu kompilieren mit Pure C ohne String - Merging !!!          */
/*                                                                   */
/*********************************************************************/


#ifdef __GNUC__
#include <gem.h>
#else
#include <aes.h>
#include <vdi.h>
#endif
#undef BYTE
#define BYTE char
#ifndef WORD
# define WORD short
#endif
#ifndef LONG
# define LONG long
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "sting.h"


#define  VAR_ACTIVE    0
#define  VAR_DELAY     1


/*
 *  Saved STinG variables. Must be first variables !!!
 */
short sting[2] = { -1, -1 };


#include "sting.rsh"

#include "transprt.h"
#include "port.h"
#include "layer.h"

#include "cpxdata.h"


#define  MESSAGE     -1
#define  CROS_CHK    (OS_CROSSED | OS_CHECKED)

int errno;


struct mem
{
	long sting_ttl;
	long sting_lblk;
	long st_ttl;
	long st_lblk;
	long alt_ttl;
	long alt_lblk;
} blk;

TPL *tpl;
STX *stx;
static DRIVER **drivs;
static LAYER **prots;
static XCPB *parameters;
static OBJECT *box;

static _WORD const m_idx[] = { 10, 7, 14 };

static _WORD vdi_handle;

static int const slide[] = {
	10, 15, 20, 25, 30, 35, 40, 45, 50, 55, 60, 65, 70, 75, 80,
	90, 100, 110, 120, 130, 140, 150, 160, 180, 200, 220, 240,
	260, 280, 300, 320, 340, 360, 380, 400, 450, 500, 550, 600,
	650, 700, 800, 900
};

static const char *const mons[] = {
	"Jan.", "Feb.", "March", "April", "May", "June",
	"July", "Aug.", "Sept.", "Oct.", "Nov.", "Dec."
};

static char alert[100];
static char dat_str[20];
static char **ptrs;
static char const template[] = "[3][%s :| |  Version %s (%s)   |  by :  %s][ Okay ]";
static char const ram_tmplt[] = "[3][%s ...| |    Total   :  %ld Bytes,   |    Largest :  " "%ld Bytes.][ Okay ]";
static char *mem[] = { " STinG RAM     ", " ST RAM        ", " Alternate RAM " };



static long get_blocks(void)
{
	long *array;
	long length;
	int count;

	blk.sting_ttl = KRgetfree(FALSE);
	blk.sting_lblk = KRgetfree(TRUE);

	if (Sversion() >= 0x1900)
	{
		array = (long *) Mxalloc(blk.st_ttl = blk.st_lblk = (long) Mxalloc(-1L, 0), 0);
		if (array != NULL)
		{
			for (count = 0; (length = (long) Mxalloc(-1L, 0)) > 10; count++)
			{
				blk.st_ttl += length;
				array[count] = (long) Mxalloc(length, 0);
			}
			for (--count; count >= 0; --count)
				Mfree((void *) array[count]);
			Mfree(array);
		}
		array = (long *) Mxalloc(blk.alt_ttl = blk.alt_lblk = (long) Mxalloc(-1L, 1), 1);
		if (array != NULL)
		{
			for (count = 0; (length = (long) Mxalloc(-1L, 1)) > 10; count++)
			{
				blk.alt_ttl += length;
				array[count] = (long) Mxalloc(length, 1);
			}
			for (--count; count >= 0; --count)
				Mfree((void *) array[count]);
			Mfree(array);
		}
	} else
	{
		array = (long *) Malloc(blk.st_ttl = blk.st_lblk = (long) Malloc(-1L));
		if (array != NULL)
		{
			for (count = 0; (length = (long) Malloc(-1L)) > 10; count++)
			{
				blk.st_ttl += length;
				array[count] = (long) Malloc(length);
			}
			for (--count; count >= 0; --count)
				Mfree((void *) array[count]);
			Mfree(array);
		}
	}

	return Sversion() >= 0x1900;
}


static void decode_date(uint16 date)
{
	int year, month, day;

	year = (date >> 9) & 0x7f;
	month = (date >> 5) & 0x0f;
	day = date & 0x1f;

	if (day == 0 || day > 31 || month == 0 || month > 12)
		strcpy(dat_str, "Invalid Date");
	else
		sprintf(dat_str, "%d. %s %d", day, mons[month - 1], 1980 + year);
}


static int collect_protocols(void)
{
	LAYER *my_layers;
	LAYER *walk;
	LAYER **array;
	int number = 0;
	int max_len = 0;
	int count;
	char **cptr;
	char *string;
	char *wrk;

	query_chains(NULL, NULL, &my_layers);

	for (walk = my_layers; walk != NULL; walk = walk->next)
	{
		number++;
		if (max_len < (int)strlen(walk->name))
			max_len = (int)strlen(walk->name);
	}

	if (number == 0)
		return 0;

	if ((wrk = KRmalloc(number * (2 * sizeof(char *) + max_len + 3))) == NULL)
		return 0;

	cptr = (char **) wrk;
	string = (char *) wrk + number * 2 * sizeof(char *);

	for (count = 0; count < number; count++)
	{
		*cptr++ = string;
		memset(string, ' ', max_len + 2);
		string[max_len + 2] = '\0';
		string += max_len + 3;
	}

	cptr = (char **) wrk;
	array = (LAYER **) wrk + number;

	for (walk = my_layers; walk != NULL; walk = walk->next)
	{
		*array++ = walk;
		memcpy(*cptr++ + 1, walk->name, strlen(walk->name));
	}

	prots = (LAYER **) wrk + number;
	ptrs = (char **) wrk;

	return number;
}


static int collect_drivers(void)
{
	DRIVER *my_drivers;
	DRIVER *walk;
	DRIVER **array;
	int number = 0;
	int max_len = 0;
	int count;
	char **cptr;
	char *string;
	char *wrk;

	query_chains(NULL, &my_drivers, NULL);

	for (walk = my_drivers; walk != NULL; walk = walk->next)
	{
		number++;
		if (max_len < (int)strlen(walk->name))
			max_len = (int)strlen(walk->name);
	}

	if (number == 0)
		return 0;

	if ((wrk = KRmalloc(number * (2 * sizeof(char *) + max_len + 3))) == NULL)
		return 0;

	cptr = (char **) wrk;
	string = (char *) wrk + number * 2 * sizeof(char *);

	for (count = 0; count < number; count++)
	{
		*cptr++ = string;
		memset(string, ' ', max_len + 2);
		string[max_len + 2] = '\0';
		string += max_len + 3;
	}

	cptr = (char **) wrk;
	array = (DRIVER **) wrk + number;

	for (walk = my_drivers; walk != NULL; walk = walk->next)
	{
		*array++ = walk;
		memcpy(*cptr++ + 1, walk->name, strlen(walk->name));
	}

	drivs = (DRIVER **) wrk + number;
	ptrs = (char **) wrk;

	return number;
}


static _WORD cdecl cpx_call(GRECT *wind)
{
	GRECT rect;
	_WORD button;
	int count;
	int delay;
	_WORD choice;
	_WORD abort_flg = FALSE;
	_WORD msg_buff[8];
	_WORD work_in[11];
	_WORD work_out[57];
	const char *version;

	graf_mouse(ARROW, NULL);

	for (count = 0; count < 10; count++)
		work_in[count] = 1;
	work_in[10] = 2;

	vdi_handle = parameters->handle;
	v_opnvwk(work_in, &vdi_handle, work_out);

	box[ROOT].ob_x = wind->g_x;
	box[ROOT].ob_y = wind->g_y;
	objc_draw(box, ROOT, MAX_DEPTH, wind->g_x, wind->g_y, wind->g_w, wind->g_h);

	do
	{
		button = (*parameters->Xform_do) (box, 0, msg_buff);
		if (button > 0)
		{
			objc_offset(box, button &= 0x7fff, &rect.g_x, &rect.g_y);
			rect.g_w = box[button].ob_width;
			rect.g_h = box[button].ob_height;
		}
		switch (button)
		{
		case SAVE:
			if ((*parameters->XGen_Alert) (SAVE_DEFAULTS))
			{
				sting[VAR_ACTIVE] = (box[ACTIVE].ob_state & OS_SELECTED) ? 1 : 0;
				set_sysvars(sting[VAR_ACTIVE], sting[VAR_DELAY]);
				(*parameters->CPX_Save) (sting, sizeof(sting));
			}
			break;
		case SET:
			sting[VAR_ACTIVE] = (box[ACTIVE].ob_state & OS_SELECTED) ? 1 : 0;
			set_sysvars(sting[VAR_ACTIVE], sting[VAR_DELAY]);
			abort_flg = TRUE;
			break;
		case CANCEL:
			abort_flg = TRUE;
			break;
		case MESSAGE:
			if (msg_buff[0] == WM_CLOSED)
			{
				abort_flg = TRUE;
				sting[VAR_ACTIVE] = (box[ACTIVE].ob_state & OS_SELECTED) ? 1 : 0;
				set_sysvars(sting[VAR_ACTIVE], sting[VAR_DELAY]);
			}
			if (msg_buff[0] == AC_CLOSE)
				abort_flg = TRUE;
			break;
		case F_SLIDE:
			delay = graf_slidebox(box, F_PARENT, F_SLIDE, 1);
			box[F_SLIDE].ob_y = (delay + 12) / 24;
			delay = slide[(1012 - delay) / 24] / 5;
			sting[VAR_DELAY] = delay;
			sprintf(box[FREQ].ob_spec.tedinfo->te_ptext, "%d", delay * 5);
			button = F_PARENT;
			objc_draw(box, F_REDRW, 2, wind->g_x, wind->g_y, wind->g_w, wind->g_h);
			break;
		case PROTOCOL:
			if ((count = collect_protocols()) == 0)
				break;
			choice = (*parameters->Popup) ((const char *const *)ptrs, count, -1, IBM, &rect, wind);
			KRfree(ptrs);
			if (choice == -1)
				break;
			decode_date(prots[choice]->date);
			version = prots[choice]->version;
			sprintf(alert, template, prots[choice]->name, *version == '0' ? version + 1 : version,
					dat_str, prots[choice]->author);
			form_alert(1, alert);
			break;
		case DRIVERS:
			if ((count = collect_drivers()) == 0)
				break;
			choice = (*parameters->Popup) ((const char *const *)ptrs, count, -1, IBM, &rect, wind);
			KRfree(ptrs);
			if (choice == -1)
				break;
			decode_date(drivs[choice]->date);
			version = drivs[choice]->version;
			sprintf(alert, template, drivs[choice]->name, *version == '0' ? version + 1 : version,
					dat_str, drivs[choice]->author);
			form_alert(1, alert);
			break;
		case MEMORY:
			count = (Supexec(get_blocks)) ? 3 : 2;
			choice = (*parameters->Popup) ((const char *const *)mem, count, -1, IBM, &rect, wind);
			if (0 <= choice && choice <= 2)
			{
				mem[choice][m_idx[choice]] = '\0';
				switch (choice)
				{
				case 0:
					sprintf(alert, ram_tmplt, mem[0], blk.sting_ttl, blk.sting_lblk);
					break;
				case 1:
					sprintf(alert, ram_tmplt, mem[1], blk.st_ttl, blk.st_lblk);
					break;
				case 2:
					sprintf(alert, ram_tmplt, mem[2], blk.alt_ttl, blk.alt_lblk);
					break;
				}
				mem[choice][m_idx[choice]] = ' ';
				form_alert(1, alert);
			}
			break;
		}
		if (button > 0)
		{
			box[button].ob_state &= ~OS_SELECTED;
			objc_draw(box, button, 3, wind->g_x, wind->g_y, wind->g_w, wind->g_h);
		}
	} while (!abort_flg);

	v_clsvwk(vdi_handle);

	return FALSE;
}


static _WORD cdecl my_button_handler(PARMBLK *parameter)
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

static CPXINFO fkts = {
	cpx_call, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL
};


CPXINFO *cdecl cpx_init(XCPB *para)
{
	int count;
	long total;
	DRV_LIST *sting_drivers;

	parameters = para;

	if ((*parameters->getcookie) (STIK_COOKIE_MAGIC, (long *) &sting_drivers) == 0)
		return NULL;

	if (sting_drivers == 0)
		return NULL;

	if (strcmp(sting_drivers->magic, STIK_DRVR_MAGIC) != 0)
		return NULL;

	tpl = (TPL *) (*sting_drivers->get_dftab) (TRANSPORT_DRIVER);
	stx = (STX *) (*sting_drivers->get_dftab) (MODULE_DRIVER);

	if (tpl == NULL || stx == NULL)
		return NULL;

	if (parameters->booting)
	{
		if (*getvstr("CONFSTING") == '0')
		{
			set_sysvars(sting[VAR_ACTIVE], sting[VAR_DELAY]);
		}
		return (CPXINFO *)1;
	}
	if (!parameters->SkipRshFix)
	{
		(*parameters->rsh_fix) (NUM_OBS, NUM_FRSTR, NUM_FRIMG, NUM_TREE,
								rs_object, rs_tedinfo, rs_strings, rs_iconblk,
								rs_bitblk, rs_frstr, rs_frimg, rs_trindex, rs_imdope);
		box = rs_object;

		for (count = 0; count < NUM_OBS; count++)
		{
			if (box[count].ob_type & 0x7f00)
				if ((box[count].ob_state & CROS_CHK) == CROS_CHK)
				{
					box[count].ob_type = G_USERDEF;
					box[count].ob_spec.userblk = &my_user_block;
					box[count].ob_state &= ~CROS_CHK;
				}
		}

		total = set_sysvars(-1, -1);
		sting[VAR_ACTIVE] = (total >> 16) ? 1 : 0;
		sting[VAR_DELAY] = total & 0xffffL;

		strncpy(box[VERSION].ob_spec.free_string + 8, tpl->version, 5);

		for (count = 0; count < 42; count++)
			if (slide[count] / 5 >= sting[VAR_DELAY])
				break;
		sprintf(box[FREQ].ob_spec.tedinfo->te_ptext, "%d", slide[count]);
		total = box[F_PARENT].ob_height - box[F_SLIDE].ob_height;
		box[F_SLIDE].ob_y = (int) (total - count * total / 42);

		if (sting[VAR_ACTIVE])
			box[ACTIVE].ob_state |= OS_SELECTED;
	}

	return &fkts;
}
