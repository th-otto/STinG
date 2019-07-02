/*
 *      resolve.h           (c) Peter Rottengatter  1997
 *                              perot@pallas.amp.uni-hannover.de
 *
 *      Included into the RESOLVE.STX source code files
 */

#ifndef RESOLVE_H
#define RESOLVE_H


extern char *c_file;

/*--------------------------------------------------------------------------*/


/*
 *   Domain Name System Type definitions.
 */

#define   DNS_A         1
#define   DNS_NS        2
#define   DNS_CNAME     5
#define   DNS_SOA       6
#define   DNS_WKS       11
#define   DNS_PTR       12
#define   DNS_HINFO     13
#define   DNS_MX        15



/*--------------------------------------------------------------------------*/


/*
 *   DNS header structure.
 */

typedef  struct dns_hdr  {
     uint16    ident;            /* Identifier for this query               */
     unsigned  qr_flg  : 1;      /* Flag for query (0) or reply (1)         */
     unsigned  op_code : 4;      /* Op code for what do do                  */
     unsigned  aa_flg  : 1;      /* Flag for Authoritative Answer           */
     unsigned  tc_flg  : 1;      /* Truncation flag                         */
     unsigned  rd_flg  : 1;      /* Recursion Desired flag                  */
     unsigned  ra_flg  : 1;      /* Recursion Available flag                */
     unsigned  zero    : 3;      /* Reserved, must be zero                  */
     unsigned  reply   : 4;      /* Replay code (errors)                    */
     uint16    QD_count;         /* Num. of entries in Question Section     */
     uint16    AN_count;         /* Num. of entries in Answer Section       */
     uint16    NS_count;         /* Num. of entries in Authority Section    */
     uint16    AR_count;         /* Num. of entries in Add. Records Section */
} DNS_HDR;



/*--------------------------------------------------------------------------*/


/*
 *   STX internal structure holding a cached domain name.
 */

typedef  struct dname  {
     char      *name;              /* Domain Name                           */
     int16     length;             /* Number of characters in name          */
     int16     type;               /* DNS type of entry ('A', 'MX' etc.)    */
     uint32    expiry;             /* Expiry time of entry (since 1.1.1970) */
     struct dname  *next;          /* In case of alias pointer to next one  */
} DNAME;



/*--------------------------------------------------------------------------*/


/*
 *   STX internal structure holding all cached data.
 */

typedef  struct cache  {
     uint32    IP_address;         /* Host IP address                       */
     DNAME     real;               /* Canonical domain name for host        */
     DNAME     *alias;             /* Linked list of alias names            */
     uint32    used;               /* Latest time this one was used         */
     struct cache  *next;          /* Next cache entry in list              */
} CACHE;


char *Ca_first_dom(void);
char *Ca_next_dom(void);
char *Ca_curr_dom(void);
uint32 Ca_curr_IP(void);

int16 load_cache(void);
int16 save_cache(void);
int16 query_name(const char *name, char *real, uint32 *IP_list, int16 size);
int16 query_IP(uint32 addr, char *real, uint32 *IP_list, int16 size);
void update_cache(const char *name, uint32 addr, uint32 ttl, int16 type);

int16 do_query(char *item, uint32 dns, int16 type, DNS_HDR **hdr, uint8 **data, uint8 **qs, uint8 **as, uint8 **ns, uint8 **ais);

/*--------------------------------------------------------------------------*/

#ifndef GNU_ASM_NAME
#ifdef __GNUC__
#define GNU_ASM_NAME(x) __asm__(x)
#else
#define GNU_ASM_NAME(x)
#endif
#endif

#ifndef NO_CONST
#  ifdef __GNUC__
#    define NO_CONST(p) __extension__({ union { const void *cs; void *s; } x; x.cs = p; x.s; })
#  else
#    define NO_CONST(p) ((void *)(p))
#  endif
#endif

long sys_timer_supx(void) GNU_ASM_NAME("sys_timer_supx");
char *is_domname(char *string, int len) GNU_ASM_NAME("is_domname");
const char *is_IP_addr(const char *string) GNU_ASM_NAME("is_IP_addr");
const char *is_unblank(const char *string) GNU_ASM_NAME("is_unblank");
const char *next_dip(const char *string) GNU_ASM_NAME("next_dip");
const char *skip_space(const char *string) GNU_ASM_NAME("skip_space");
uint32 diptobip(const char *s_p) GNU_ASM_NAME("diptobip");
char *biptodip(uint32 ip_n, char *s_p) GNU_ASM_NAME("biptodip");
char *biptodrip(uint32 ip_n, char *s_p) GNU_ASM_NAME("biptodrip");
uint8 *pass_RRname(uint8 *data_p, uint8 *pos_p, char *dest_p) GNU_ASM_NAME("pass_RRname");

#endif /* RESOLVE_H */
