/*
 *      port.h              (c) Peter Rottengatter  1996
 *                              perot@pallas.amp.uni-hannover.de
 *
 *      Include this file to provide port interfacing to STinG
 */

#ifndef STING_PORT_H
#define STING_PORT_H

#ifndef MODULE_DRIVER
#define MODULE_DRIVER    "MODULE_LAYER"
#endif



/*--------------------------------------------------------------------------*/


/*
 *   Internal port descriptor.
 */

typedef  struct port_desc {
    const char      *name;      /* Name of port                             */
    int16     type;             /* Type of port                             */
    int16     active;           /* Flag for port active or not              */
    uint32    flags;            /* Type dependent operational flags         */
    uint32    ip_addr;          /* IP address of this network adapter       */
    uint32    sub_mask;         /* Subnet mask of attached network          */
    int16     mtu;              /* Maximum packet size to go through        */
    int16     max_mtu;          /* Maximum allowed value for mtu            */
    int32     stat_sd_data;     /* Statistics of sent data                  */
    IP_DGRAM  *send;            /* Link to first entry in send queue        */
    int32     stat_rcv_data;    /* Statistics of received data              */
    IP_DGRAM  *receive;         /* Link to first entry in receive queue     */
    int16     stat_dropped;     /* Statistics of dropped datagrams          */
    struct drv_desc   *driver;  /* Driver program to handle this port       */
    struct port_desc  *next;    /* Next port in port chain                  */
 } PORT;


/*
 *   Port driver descriptor.
 */

typedef  struct drv_desc {
    int16 cdecl  (* set_state) (PORT *, int16);       /* Setup and shutdown */
    int16 cdecl  (* cntrl) (PORT *, uint32, int16);   /* Control functions  */
    void  cdecl  (* send) (PORT *);                   /* Send packets       */
    void  cdecl  (* receive) (PORT *);                /* Receive packets    */
    const char       *name;     /* Name of driver                           */
    const char       *version;  /* Version of driver in "xx.yy" format      */
    uint16           date;      /* Compile date in GEMDOS format            */
    const char       *author;   /* Name of programmer                       */
    struct drv_desc  *next;     /* Next driver in driver chain              */
    BASPAG           *basepage; /* Basepage of this module                  */
 } DRIVER;



/*--------------------------------------------------------------------------*/


/*
 *   Link Type Definitions
 */

#define  L_INTERNAL   0           /* Internal pseudo port                   */
#define  L_SER_PTP    1           /*   Serial point to point type link      */
#define  L_PAR_PTP    2           /* Parallel point to point type link      */
#define  L_SER_BUS    3           /*   Serial            bus type link      */
#define  L_PAR_BUS    4           /* Parallel            bus type link      */
#define  L_SER_RING   5           /*   Serial           ring type link      */
#define  L_PAR_RING   6           /* Parallel           ring type link      */
#define  L_MASQUE     7           /*   Masquerading pseudo port             */

/*--------------------------------------------------------------------------*/


#endif /* STING_PORT_H */
