/*
 *	 Module driver structure / functions
 */

#ifndef MOD_DRIVER
#define MOD_DRIVER

#ifndef MODULE_DRIVER
#define MODULE_DRIVER	 "MODULE_LAYER"
#endif

typedef struct lay_desc LAYER;
typedef struct port_desc PORT;


#undef set_dgram_ttl
#undef check_dgram_ttl
#undef load_routing_table
#undef set_sysvars
#undef query_chains
#undef IP_send
#undef IP_fetch
#undef IP_handler
#undef IP_discard
#undef PRTCL_announce
#undef PRTCL_get_parameters
#undef PRTCL_request
#undef PRTCL_release
#undef PRTCL_lookup
#undef TIMER_call
#undef TIMER_now
#undef TIMER_elapsed
#undef protect_exec
#undef get_route_entry
#undef set_route_entry

/*--------------------------------------------------------------------------*/


/*
 *	 CN functions structure for TCP and UDP
 */

typedef struct cn_funcs {
	int16  cdecl  (* CNkick) (void *);
	int16  cdecl  (* CNbyte_count) (void *);
	int16  cdecl  (* CNget_char) (void *);
	NDB *  cdecl  (* CNget_NDB) (void *);
	int16  cdecl  (* CNget_block) (void *, void *, int16);
	CIB *  cdecl  (* CNgetinfo) (void *);
	int16  cdecl  (* CNgets) (void *, char *, int16, char);
} CN_FUNCS;

/*
 *	 Port driver descriptor.
 */

typedef  struct drv_desc {
	int16 cdecl  (* set_state) (PORT *, int16); 	  /* Setup and shutdown */
	int16 cdecl  (* cntrl) (PORT *, uint32, int16);   /* Control functions	*/
	void  cdecl  (* send) (PORT *); 				  /* Send packets		*/
	void  cdecl  (* receive) (PORT *);				  /* Receive packets	*/
	const char		 *name; 	/* Name of driver							*/
	const char		 *version;	/* Version of driver in "xx.yy" format		*/
	uint16			 date;		/* Compile date in GEMDOS format			*/
	const char		 *author;	/* Name of programmer						*/
	struct drv_desc  *next; 	/* Next driver in driver chain				*/
	BASPAG			 *basepage; /* Basepage of this module					*/
} DRIVER;


typedef struct stx {
	const char * module;	  /* Specific string that can be searched for	  */
	const char * author;	  /* Any string 								  */
	const char * version;	  /* Format `00.00' Version:Revision			  */
	void	   cdecl  (* set_dgram_ttl) (IP_DGRAM *datagram);
	int16	   cdecl  (* check_dgram_ttl) (IP_DGRAM *datagram);
	int16	   cdecl  (* load_routing_table) (void);
	int32	   cdecl  (* set_sysvars) (int16 new_act, int16 new_frac);
	void	   cdecl  (* query_chains) (PORT ** port, DRIVER ** drv, LAYER ** layer);
	int16	   cdecl  (* IP_send) (uint32 src, uint32 dest, uint8 tos, uint16 frg, uint8 ttl, uint8 prctl, uint16 id, void *data, uint16 dlen, void *opt, uint16 olen);
	IP_DGRAM * cdecl  (* IP_fetch) (int16 prtcl);
	int16	   cdecl  (* IP_handler) (int16 prtctl, int16 cdecl (*handleler) (IP_DGRAM *), int16 flag);
	void	   cdecl  (* IP_discard) (IP_DGRAM *datagram, int16 all_flag);
	int16	   cdecl  (* PRTCL_announce) (int16 protocol);
	int16	   cdecl  (* PRTCL_get_parameters) (uint32 rem_host, uint32 *src_ip, int16 *ttl, uint16 *mtu);
	int16	   cdecl  (* PRTCL_request) (void *anonymous, CN_FUNCS *cn_functions);
	void	   cdecl  (* PRTCL_release) (int16 handle);
	void *	   cdecl  (* PRTCL_lookup) (int16, CN_FUNCS *);
	int16	   cdecl  (* TIMER_call) (int16 cdecl (*handler) (IP_DGRAM *), int16);
	int32	   cdecl  (* TIMER_now) (void);
	int32	   cdecl  (* TIMER_elapsed) (int32 then);
	int32	   cdecl  (* protect_exec) (void *parameter, int32 cdecl (*handler) (void *));
	int16	   cdecl  (* get_route_entry) (int16 no, uint32 *tmplt, uint32 *mask, PORT **port, uint32 *gateway);
	int16	   cdecl  (* set_route_entry) (int16 no, uint32 tmplt, uint32 mask, PORT *port, uint32 gateway);
	/* reserved fields; since LAYER_VERSION >= 1.06 */
	void *reserved1;
	void *reserved2;
	void *reserved3;
	void *reserved4;
} STX;

extern STX *stx;


/*
 *	 Definitions of module driver functions for direct use
 */

#define set_dgram_ttl(x)				 (*stx->set_dgram_ttl)(x)
#define check_dgram_ttl(x)				 (*stx->check_dgram_ttl)(x)
#define load_routing_table()			 (*stx->load_routing_table)()
#define set_sysvars(x,y)				 (*stx->set_sysvars)(x,y)
#define query_chains(x,y,z) 			 (*stx->query_chains)(x,y,z)
#define IP_send(a,b,c,d,e,f,g,h,i,j,k)	 (*stx->IP_send)(a,b,c,d,e,f,g,h,i,j,k)
#define IP_fetch(x) 					 (*stx->IP_fetch)(x)
#define IP_handler(x,y,z)				 (*stx->IP_handler)(x,y,z)
#define IP_discard(x,y) 				 (*stx->IP_discard)(x,y)
#define PRTCL_announce(x)				 (*stx->PRTCL_announce)(x)
#define PRTCL_get_parameters(w,x,y,z)	 (*stx->PRTCL_get_parameters)(w,x,y,z)
#define PRTCL_request(x,y)				 (*stx->PRTCL_request)(x,y)
#define PRTCL_release(x)				 (*stx->PRTCL_release)(x)
#define PRTCL_lookup(x,y)				 (*stx->PRTCL_lookup)(x,y)
#define TIMER_call(x,y) 				 (*stx->TIMER_call)(x,y)
#define TIMER_now() 					 (*stx->TIMER_now)()
#define TIMER_elapsed(x)				 (*stx->TIMER_elapsed)(x)
#define protect_exec(x,y)				 (*stx->protect_exec)(x,y)
#define get_route_entry(a,b,c,d,e)		 (*stx->get_route_entry)(a,b,c,d,e)
#define set_route_entry(a,b,c,d,e)		 (*stx->set_route_entry)(a,b,c,d,e)

#endif /* MOD_DRIVER */
