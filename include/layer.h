/*--------------------------------------------------------------------------*/
/*	File name:	LAYER.H							Revision date:	1999.09.25	*/
/*	Revised by:	Ulf Ronald Andersson			Revision start:	1999.09.21	*/
/*	Created by:	Peter Rottengatter				Creation date:	1996.xx.xx	*/
/*--------------------------------------------------------------------------*/
/*	Header file for STinG protocol module source files.						*/
/*--------------------------------------------------------------------------*/
/*	Copyright:	(c) Peter Rottengatter  1996                                */
/*	Released as FREEWARE for use and distribution, but not as Public Domain	*/
/*--------------------------------------------------------------------------*/

#ifndef STING_LAYER_H
#define STING_LAYER_H

#include "stx.h"
#include "port.h"

/*--------------------------------------------------------------------------*/


/*
 *	 High level protocol module descriptor.
 */

struct lay_desc {
	const char		 *name; 		 /* Name of layer						*/
	const char		 *version;		 /* Version of layer in xx.yy format	*/
	uint32			 flags; 		 /* Private data						*/
	uint16			 date;			 /* Compile date in GEMDOS format		*/
	const char		 *author;		 /* Name of programmer					*/
	int16			 stat_dropped;	 /* Statistics of dropped data units	*/
	struct lay_desc  *next; 		 /* Next layer in driver chain			*/
	BASPAG			 *basepage; 	 /* Basepage of this module 			*/
};

#define PROTO_DO_ICMP 0x10000ul

/*--------------------------------------------------------------------------*/

#endif /* STING_LAYER_H */
