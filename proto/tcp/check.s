/*********************************************************************/
/*                                                                   */
/*     High Level Protokoll : TCP                                    */
/*                                                                   */
/*      Version 1.0                        from 11. March 1997       */
/*                                                                   */
/*********************************************************************/


            .globl    wait_flag                  /* Function for waiting for semaphore */
            .globl    req_flag                   /* Function for requesting semaphore */
            .globl    rel_flag                   /* Function for releasing semaphore */

            .globl    dis_intrpt                 /* Function for disabling interupts */
            .globl    en_intrpt                  /* Function for enabling interupts */

            .globl    get_pending                /* Function for getting pending data */

            .globl    check_sum                  /* Function for checksumming data */


			.text

/* ------------------------------------------------------------------------------------- */

wait_flag:
            tas     (a0)                         /* Test semaphore, and set bit 7 */
            bne     wait_flag                    /* Was set ? Then loop */
            or.b    #0xff,(a0)                   /* Now the lock is ours */
            rts

req_flag:
            move.l  #1,d0                        /* Preset TRUE in case of lock */
            tas     (a0)                         /* Test semaphore, and set bit 7 */
            bne     denied                       /* Was set ? Return TRUE */
            or.b    #0xff,(a0)                   /* Now the lock is ours */
            clr.l   d0                           /* And return FALSE */
denied:     nop
            rts

rel_flag:
            clr.b   (a0)                         /* Clear semaphore */
            rts

/* ------------------------------------------------------------------------------------- */

dis_intrpt:
            move.w  sr,srsave                    /* Save CPU status */
            or.w    #0x0700,sr                   /* Disable interupts */
            rts

en_intrpt:
            move.w  srsave,sr                    /* Restore CPU status */
            rts

srsave:     .dc.w    0                           /* CPU status */

/* ------------------------------------------------------------------------------------- */

get_pending:
            move.w  sr,-(sp)                     /* Save CPU status */
            or.w    #0x0700,sr                   /* Disable interupts */
            move.l  (a0),-(sp)                   /* Get value */
            clr.l   (a0)                         /* And reset it */
            move.l  (sp)+,a0                     /* Return value */
            move.w  (sp)+,sr                     /* Restore CPU status */
            rts

/* ------------------------------------------------------------------------------------- */

check_sum:
            and.b   #0xef,ccr                    /* Clear extended carry */
            addx.l  d1,d0                        /* Add dest_ip to src_ip */
            move.l  #0x060000,d1                 /* Prepare TCP pseudo header word */
            move.w  d2,d1                        /* Length for pseudo header word */
            addx.l  d1,d0                        /* Add it to accumulator */
            clr.l   d1                           /* For adding carries */
            addx.l  d1,d0                        /* Add carry to the accumulator */
            move.b  d2,remain                    /* Save last two bits of length */
            and.l   #0xffff,d2                   /* Extend to longword */
            move.l  d2,d1                        /* Save length */
            lsr.l   #6,d1                        /* Length by 64 is loop count */
            and.l   #0x3c,d2                     /* Find fraction of a chunk (loop) */
            neg.l   d2                           /* Negate to offset beyond loop body */
            and.b   #0xef,ccr                    /* Clear extended carry */
            lea     lp_end(pc),a1                /* End of loop body */
            jmp     (a1,d2)                      /* Jump into loop */

loop:       move.l  (a0)+,d2                     /* Fetch  1st longword (4 bytes) */
            addx.l  d2,d0                        /* Add it to accumulator */
            move.l  (a0)+,d2                     /* Fetch  2nd longword (4 bytes) */
            addx.l  d2,d0                        /* Add it to accumulator */
            move.l  (a0)+,d2                     /* Fetch  3rd longword (4 bytes) */
            addx.l  d2,d0                        /* Add it to accumulator */
            move.l  (a0)+,d2                     /* Fetch  4th longword (4 bytes) */
            addx.l  d2,d0                        /* Add it to accumulator */
            move.l  (a0)+,d2                     /* Fetch  5th longword (4 bytes) */
            addx.l  d2,d0                        /* Add it to accumulator */
            move.l  (a0)+,d2                     /* Fetch  6th longword (4 bytes) */
            addx.l  d2,d0                        /* Add it to accumulator */
            move.l  (a0)+,d2                     /* Fetch  7th longword (4 bytes) */
            addx.l  d2,d0                        /* Add it to accumulator */
            move.l  (a0)+,d2                     /* Fetch  8th longword (4 bytes) */
            addx.l  d2,d0                        /* Add it to accumulator */
            move.l  (a0)+,d2                     /* Fetch  9th longword (4 bytes) */
            addx.l  d2,d0                        /* Add it to accumulator */
            move.l  (a0)+,d2                     /* Fetch 10th longword (4 bytes) */
            addx.l  d2,d0                        /* Add it to accumulator */
            move.l  (a0)+,d2                     /* Fetch 11th longword (4 bytes) */
            addx.l  d2,d0                        /* Add it to accumulator */
            move.l  (a0)+,d2                     /* Fetch 12th longword (4 bytes) */
            addx.l  d2,d0                        /* Add it to accumulator */
            move.l  (a0)+,d2                     /* Fetch 13th longword (4 bytes) */
            addx.l  d2,d0                        /* Add it to accumulator */
            move.l  (a0)+,d2                     /* Fetch 14th longword (4 bytes) */
            addx.l  d2,d0                        /* Add it to accumulator */
            move.l  (a0)+,d2                     /* Fetch 15th longword (4 bytes) */
            addx.l  d2,d0                        /* Add it to accumulator */
            move.l  (a0)+,d2                     /* Fetch 16th longword (4 bytes) */
            addx.l  d2,d0                        /* Add it to accumulator */
lp_end:
            dbra    d1,loop                      /* Loop it */

            move.l  d0,d1                        /* Save high word */
            swap    d1                           /* Transfer it to low word */
            addx.w  d1,d0                        /* Add both words */
            btst    #1,remain                    /* Do we have 2 or 3 remaining bytes ? */
            beq     no_word                      /* No ... */
            move.w  (a0)+,d2                     /* Else fetch another word */
            addx.w  d2,d0                        /* And add it to the accumulator */
no_word:    btst    #0,remain                    /* Do we have another remaining byte ? */
            beq     no_byte                      /* No ... */
            move.w  (a0)+,d2                     /* Else fetch another word */
            clr.b   d2                           /* But only use upper byte */
            addx.w  d2,d0                        /* And add it to the accumulator */
no_byte:    clr.w   d2                           /* For adding carries */
            addx.w  d2,d0                        /* Add carry to the accumulator */
            addx.w  d2,d0                        /* Again if another overflow */
            and.l   #0xffff,d0                   /* And truncate to word length */
            eor.w   #0xffff,d0                   /* Invert the result */
            rts

			.bss

remain:     .ds.w    1                           /* Storage for last two length bits */
