/*
 * COPS (c) 1995 - 2003 Sven & Wilfried Behne
 *                 2004 F.Naumann & O.Skancke
 *
 * A XCONTROL compatible CPX manager.
 *
 * This file is part of COPS.
 *
 * COPS is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * COPS is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with XaAES; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef _adaptrsc_h
#define _adaptrsc_h

#ifdef __GNUC__
#include <gem.h>
#else
#include <aes.h>
#include <vdi.h>
#endif

#define	GAI_WDLG	0x0001UL	/* wdlg_xx()-functions available */
#define	GAI_LBOX	0x0002UL	/* lbox_xx()-functions available */
#define	GAI_FNTS	0x0004UL	/* fnts_xx()-functions available */
#define	GAI_FSEL	0x0008UL	/* new file selector (fslx_xx) available */
#define	GAI_PDLG	0x0010UL	/* pdlg_xx()-functions available */

#define	GAI_MAGIC	0x0100UL	/* MagiC-AES present */
#define	GAI_INFO	0x0200UL	/* appl_getinfo() supported */
#define	GAI_3D		0x0400UL	/* 3D-Look supported */
#define	GAI_CICN	0x0800UL	/* Color-Icons supported */
#define	GAI_APTERM	0x1000UL	/* AP_TERM supported */
#define	GAI_GSHORTCUT	0x2000UL	/* object type G_SHORTCUT supported */
#define	GAI_FLYDIAL	0x4000UL	/* form_xdo/form_xdial supported */
#define	GAI_MOUSE	0x8000UL	/* graf_mouse(M_SAVE/M_RESTORE) supported */
#define GAI_POPUP	0x10000UL	/* form_popup() supported */
#define GAI_SCROLLPOPUP	0x20000UL	/* xfrm_popup() supported */
#define GAI_OBJC_WEDIT	0x40000UL	/* objc_wedit() supported */

extern _WORD aes_handle;
extern _WORD vdi_handle;
extern _WORD gl_wchar;
extern _WORD gl_hchar;
extern _WORD aes_font;
extern _WORD aes_height;
extern unsigned long aes_flags;

short get_cookie(long cookie, long *p_value);

unsigned long get_aes_info(_WORD *font_id, _WORD *font_height, _WORD *hor_3d, _WORD *ver_3d);
void adapt3d_rsrc(OBJECT *objs, unsigned short no_objs, _WORD hor_3d, _WORD ver_3d);
void no3d_rsrc(OBJECT *objs, unsigned short no_objs, short ftext_to_fboxtext);
void substitute_objects(OBJECT *objs, unsigned short no_objs, unsigned long aes_flags, OBJECT *rslct, OBJECT *rdeslct);
void substitute_free(void);
char *is_userdef_title(OBJECT *obj);

#endif /* _adaptrsc_h */
