/*********************************************************************/
/*                                                                   */
/*     High Level Protokoll : DNS Resolver                           */
/*                                                                   */
/*      Version 1.0                      from 13. January 1997       */
/*                                                                   */
/*********************************************************************/


            .globl   wait_flag                  /* Function for waiting for semaphore */
            .globl   rel_flag                   /* Function for releasing semaphore */


/* ------------------------------------------------------------------------------------- */

wait_flag:
            tas     (a0)                         /* Test semaphor, and set bit 7 */
            bne     wait_flag                    /* Was set ? Then loop */
            or.b    #0xff,(a0)                   /* Now the lock is ours */
            rts

rel_flag:
            clr.b   (a0)                         /* Clear semaphor */
            rts
