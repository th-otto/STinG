/*
 *      layer.h             (c) Peter Rottengatter  1996
 *                              perot@pallas.amp.uni-hannover.de
 *
 *      Include this file to provide high level protocol interfacing to STinG
 */

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



/*--------------------------------------------------------------------------*/


#endif /* STING_LAYER_H */
