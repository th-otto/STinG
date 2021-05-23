/*
 * FreeMiNT USB subsystem by David Galvez. 2010 - 2015
 *
 * This file is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This file is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#ifndef _usb_api_h
#define _usb_api_h

#include "usb.h"
#include "hub.h"

/*
 * USB API VERSION. ALL MODULES COMPILED WITH THIS, SO MUST MATCH !
 */
#define USB_API_VERSION 4

/*
 * UCD - USB Controller Driver.
 */
#define UCD_OPEN	1
#define UCD_NAMSIZ	16	/* maximum ucd name len */

#define USB_CONTRLL	0

/*
 * ioctl opcodes
 */
#define LOWLEVEL_INIT		(('U'<< 8) | 0)
#define LOWLEVEL_STOP		(('U'<< 8) | 1)
#define SUBMIT_CONTROL_MSG	(('U'<< 8) | 2)
#define SUBMIT_BULK_MSG		(('U'<< 8) | 3)
#define SUBMIT_INT_MSG		(('U'<< 8) | 4)

struct bulk_msg
{
	struct usb_device	*dev;
	unsigned long		pipe;
	void				*data;
	long				len;
	long				flags;
	unsigned long		timeout;
};

struct control_msg
{
	struct usb_device 	*dev;
	unsigned long 		pipe;
	unsigned short 		value;
	void 			*data;
	unsigned short 		size;
	struct devrequest	*setup;
};

struct int_msg
{
	struct usb_device 	*dev;
	unsigned long 		pipe;
	void 			*buffer;
	long 			transfer_len;
	long 			interval;
};

struct ucdif
{
	struct ucdif	*next;

	long		api_version;

	long		class;
	char		*lname;
	char		name[UCD_NAMSIZ];
	short		unit;

	unsigned short	flags;

	long		(*open)		(struct ucdif *);
	long		(*close)	(struct ucdif *);
	long		resrvd1;	/* (*output)  */
	long		(*ioctl)	(struct ucdif *, short cmd, long arg);
	long		resrvd2;	/* (*timeout) */
	long		*ucd_priv;	/* host controller driver private data */
};


/*
 * UDD - USB Device Driver.
 */
#define UDD_OPEN	1
#define UDD_NAMSIZ	16	/* maximum ucd name len */

#define USB_DEVICE	0

struct uddif
{
	struct uddif	*next;

	long		api_version;

	long		class;
	char		*lname;
	char		name[UDD_NAMSIZ];
	short		unit;

	unsigned short	flags;

	long		cdecl (*probe)	(struct usb_device *, unsigned short ifnum);
	long		cdecl (*disconnect)	(struct usb_device *);
	long		resrvd1;	/* (*output)  */
	long		cdecl (*cioctl)	(struct uddif *, short cmd, long arg);
	long		resrvd2;	/* (*timeout) */
};

struct usb_module_api
{
	/* versioning */
	long 			api_version;
	long			max_devices;
	long			max_hubs;

#if 0
	short				(*getfreeunit)		(char *);
#endif
	long			cdecl	(*udd_register)		(struct uddif *);
	long			cdecl	(*udd_unregister)	(struct uddif *);
	long			cdecl	(*ucd_register)		(struct ucdif *, struct usb_device **);
	long			cdecl	(*ucd_unregister)	(struct ucdif *);

	void			cdecl	(*usb_rh_wakeup)	(struct ucdif *);
	long			cdecl	(*usb_hub_events)	(struct usb_hub_device *dev);
	long			cdecl	(*usb_set_protocol)	(struct usb_device *dev, long ifnum, long protocol);
	long			cdecl	(*usb_set_idle)		(struct usb_device *dev, long ifnum, long duration,
								long report_id);
	struct usb_device *	cdecl	(*usb_get_dev_index)	(long idx);
	struct usb_hub_device *	cdecl	(*usb_get_hub_index)	(long idx);
	long 			cdecl	(*usb_control_msg)	(struct usb_device *dev, unsigned long pipe,
								unsigned char request, unsigned char requesttype,
								unsigned short value, unsigned short idx,
								void *data, unsigned short size, long timeout);
	long			cdecl	(*usb_bulk_msg)		(struct usb_device *dev, unsigned long pipe,
								void *data, long len, long *actual_length, long timeout, long flags);
	long 			cdecl	(*usb_submit_int_msg)	(struct usb_device *dev, unsigned long pipe,
								void *buffer, long transfer_len, long interval);
	long 			cdecl	(*usb_disable_asynch)	(long disable);
	long			cdecl	(*usb_maxpacket)	(struct usb_device *dev, unsigned long pipe);
	long			cdecl	(*usb_get_configuration_no)	(struct usb_device *dev, long cfgno);
	long			cdecl	(*usb_get_report)	(struct usb_device *dev, long ifnum, unsigned char type,
								unsigned char id, void *buf, long size);
	long 			cdecl	(*usb_get_class_descriptor)	(struct usb_device *dev, long ifnum,
									unsigned char type, unsigned char id, void *buf,
									long size);
	long 			cdecl	(*usb_clear_halt)	(struct usb_device *dev, long pipe);
	long 			cdecl	(*usb_string)		(struct usb_device *dev, long idx, char *buf, long size);
	long 			cdecl	(*usb_set_interface)	(struct usb_device *dev, long interface, long alternate);
	long			cdecl	(*usb_parse_config)	(struct usb_device *dev, unsigned char *buffer, long cfgno);
	long 			cdecl	(*usb_set_maxpacket)	(struct usb_device *dev);
	long 			cdecl	(*usb_get_descriptor)	(struct usb_device *dev, unsigned char type,
								unsigned char idx, void *buf, long size);
	long 			cdecl	(*usb_set_address)	(struct usb_device *dev);
	long 			cdecl	(*usb_set_configuration)(struct usb_device *dev, long configuration);
	long 			cdecl	(*usb_get_string)	(struct usb_device *dev, unsigned short langid,
			   					unsigned char idx, void *buf, long size);
	struct usb_device *	cdecl	(*usb_alloc_new_device) (void *controller);
	long 			cdecl	(*usb_new_device)	(struct usb_device *dev);
	
	/* For now we leave this hub specific stuff out of the api */
#if 0
	long 			cdecl	(*usb_get_hub_descriptor)	(struct usb_device *dev, void *data, long size);
	long 			cdecl	(*usb_clear_port_feature)	(struct usb_device *dev, long port, long feature);
	long 			cdecl	(*usb_get_hub_status)	(struct usb_device *dev, void *data);
	long 			cdecl	(*usb_set_port_feature)	(struct usb_device *dev, long port, long feature);
	long 			cdecl	(*usb_get_port_status)	(struct usb_device *dev, long port, void *data);
	struct usb_hub_device *	cdecl	(*usb_hub_allocate)	(void);
	void 			cdecl	(*usb_hub_port_connect_change)	(struct usb_device *dev, long port);
	long 			cdecl	(*usb_hub_configure)	(struct usb_device *dev);
#endif
};

#ifndef MAINUSB
#define	udd_register 		(*api->udd_register)
#define	udd_unregister 		(*api->udd_unregister)
#define	ucd_register 		(*api->ucd_register)
#define	ucd_unregister 		(*api->ucd_unregister)
#if 0
#define	fname 			(*api->fname)
#endif

#define usb_rh_wakeup		(*api->usb_rh_wakeup)
#define usb_hub_events		(*api->usb_hub_events)
#define	usb_set_protocol 	(*api->usb_set_protocol)
#define	usb_set_idle 		(*api->usb_set_idle)
#define	usb_get_dev_index 	(*api->usb_get_dev_index)
#define	usb_get_hub_index 	(*api->usb_get_hub_index)
#if 0
#define	usb_control_msg 	(*api->usb_control_msg)
#endif
#define	usb_bulk_msg 		(*api->usb_bulk_msg)
#define	usb_submit_int_msg 	(*api->usb_submit_int_msg)
#define	usb_disable_asynch 	(*api->usb_disable_asynch)
#define	usb_maxpacket 		(*api->usb_maxpacket)
#define	usb_get_configuration_no 	(*api->usb_get_configuration_no)
#if 0
#define	usb_get_report 		(*api->usb_get_report)
#define	usb_get_class_descriptor 	(*api->usb_get_class_descriptor)
#endif
#define	usb_clear_halt 		(*api->usb_clear_halt)
#define	usb_string 		(*api->usb_string)
#define	usb_set_interface 	(*api->usb_set_interface)
#define	usb_parse_config 	(*api->usb_parse_config)
#define	usb_set_maxpacket 	(*api->usb_set_maxpacket)
#if 0
#define	usb_get_descriptor 	(*api->usb_get_descriptor)
#endif
#define	usb_set_address 	(*api->usb_set_address)
#define	usb_set_configuration 	(*api->usb_set_configuration)
#if 0
#define	usb_get_string 		(*api->usb_get_string)
#endif
#define	usb_alloc_new_device 	(*api->usb_alloc_new_device)
#define	usb_new_device 		(*api->usb_new_device)

/* For now we leave this hub specific
 * stuff out of the api.
 */	
#if 0
#define	usb_get_hub_descriptor 	(*api->usb_get_hub_descriptor)
#define	usb_clear_port_feature 	(*api->usb_clear_port_feature)
#define	usb_clear_hub_feature 	(*api->usb_clear_hub_feature)
#define	usb_get_hub_status 	(*api->usb_get_hub_status)
#define	usb_set_port_feature 	(*api->usb_set_port_feature)
#define	usb_get_port_status 	(*api->usb_get_port_status)
#define	usb_hub_allocate 	(*api->usb_hub_allocate)
#define	usb_hub_port_connect_change 	(*api->usb_hub_port_connect_change)
#define	usb_hub_configure 	(*api->usb_hub_configure)	
#endif

#if INT_MAX != 32767
#ifdef __GNUC__
/* We have to do usb_control_msg the hard way because we're passing chars and shorts
 * to an API that uses -mshort (and thus 16 bit alignment) while we don't use -mshort.
 */
/*
	long 			cdecl	(*usb_control_msg)	(struct usb_device *dev, unsigned long pipe,
								unsigned char request, unsigned char requesttype,
								unsigned short value, unsigned short idx,
								void *data, unsigned short size, long timeout);
*/								
								
#define usb_control_msg(a, b, c, d, e, f, g, h, j)			\
__extension__								\
({									\
	register long retvalue __asm__("d0");				\
	long  _a = (long) (a);						\
	long  _b = (long) (b);						\
	short _c = (short)(c);						\
	short _d = (short)(d);						\
	short _e = (short)(e);						\
	short _f = (short)(f);						\
	long  _g = (long) (g);						\
	short _h = (short)(h);						\
	long  _j = (long) (j);						\
	    								\
	__asm__ volatile						\
	(								\
		"movl	%9,sp@-\n\t"					\
		"movw	%8,sp@-\n\t"					\
		"movl	%7,sp@-\n\t"					\
		"movw	%6,sp@-\n\t"					\
		"movw	%5,sp@-\n\t"					\
		"movw	%4,sp@-\n\t"					\
		"movw	%3,sp@-\n\t"					\
		"movl	%2,sp@-\n\t"					\
		"movl	%1,sp@-\n\t"					\
		"jsr	(%10)\n\t"					\
		"lea	sp@(26),sp "					\
	: "=r"(retvalue)			   /* outputs */	\
	: "r"(_a), "r"(_b), "r"(_c),				\
	  "r"(_d), "r"(_e), "r"(_f), "r"(_g), "r"(_h), "r"(_j), "a"(api->usb_control_msg) /* inputs  */	\
	: "d1", "d2", "a0", "a1", "a2", "memory"			\
	);								\
	retvalue;							\
})
#endif
#else
#define usb_control_msg (*api->usb_control_msg)
#endif
#endif

#endif /* usb_api_h */
