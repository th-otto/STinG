#include <gem.h>

/** forces an AES process switch.
 *
 *  @return none
 *
 *  @since not available in PC-GEM
 *
 *  @sa mt_appl_yield()
 *  
 */

#if defined(__GNUC__)
void _appl_yield(void)
{
	__asm__ volatile (
		"move.w	#201,%%d0\n\t"
		"trap	#2"
		:
		:
		: "d0","d1","d2","a0","a1","a2","memory","cc"
	);
}
#endif

#if defined(__VBCC__)
__regsused("d0/d1/a0/a1") void _appl_yield(void) =
  "\tmove.l\td2,-(sp)\n"
  "\tmove.l\ta2,-(sp)\n"
  "\tmove.w\t#201,d0\n"
  "\ttrap\t#2\n"
  "\tmove.l\t(sp)+,a2\n"
  "\tmove.l\t(sp)+,d2";
#endif

#if defined(__PUREC__)
static void trap2(short func) 0x4e42;
static void push_a2(void) 0x2f0a;
static void pop_a2(void) 0x245f;
void _appl_yield(void)
{
	push_a2();
	trap2(201);
	pop_a2();
}
#endif
