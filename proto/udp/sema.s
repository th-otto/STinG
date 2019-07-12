
/*********************************************************************/
/*                                                                   */
/*     High Level Protokoll : UDP                                    */
/*                                                                   */
/*      Version 1.0                        from 11. March 1997       */
/*                                                                   */
/*********************************************************************/


            .export   req_flag                   ;" Function for requesting semaphore
            .export   rel_flag                   ;" Function for releasing semaphore

            .export   dis_intrpt                 ;" Function for disabling interupts
            .export   en_intrpt                  ;" Function for enabling interupts

            .export   get_pending                ;" Function for getting pending data

            .export   check_sum                  ;" Function for checksumming data

			.globl get_sr
			.globl set_sr
			.globl _appl_yield
			.globl poll_super
			.globl unlink_connect
			.xref timer_work
			.xref root_list

;-------------------------------------------------------------------------------------)
			move.w sr,d0
			rts

en_intrpt:
            move.w  d0, sr                       ;" Restore CPU status
            rts

;-------------------------------------------------------------------------------------)

dis_intrpt:
            move.w  sr, d0                       ;" Save CPU status
            or.w    #$0700, sr                   ;" Disable interupts
            rts

            move.w  sr,d1
            move.w  d0,sr
            move.w  d1,sr
            rts

;-------------------------------------------------------------------------------------)

get_sr:
			move.w 8(a7),d0
			rts

;-------------------------------------------------------------------------------------)

set_sr:
			move.w 6(a7),8(a7)
			rts

cli:
			move.w 8(a7),d0
            or.w    #$0700,8(a7)                   ;" Disable interupts
			rts

			move.w 6(a7),sr
			rts


; void *cdecl unlink_connect(CONNECT *connect)
unlink_connect:
			move.l     4(a7),d1
			lea.l      root_list,a0
			bra.s      unlink_connect2
unlink_connect1:
			cmpa.l     d1,a1
			beq.s      unlink_connect3
			lea.l      40(a1),a0
unlink_connect2:
			movea.l    (a0),a1
			move.l     a1,d0
			bgt.s      unlink_connect1
			movea.l    d0,a0
			rts
unlink_connect3:
			move.l     40(a1),(a0)
			move.l     a0,d0
			rts

;-------------------------------------------------------------------------------------)

poll_super:
			move.w     8(a7),d0
			or.w       #$2000,d0
			move.w     d0,sr
			movea.l    4(a7),a0
			jmp        timer_work

;-------------------------------------------------------------------------------------)

illg:
			.dc.w 0x4afc
			rts

;-------------------------------------------------------------------------------------)

_appl_yield:
            move.w  #$c9, d0                     ;" Opcode for _appl_yield
            trap    #2                           ;" Call GEM dispatcher
            rts


req_flag:
            move.l  #1, d0                       ;" Preset TRUE in case of lock
            tas     (a0)                         ;" Test semaphor, and set bit 7
            bne     denied                       ;" Was set ? Return TRUE
            or.b    #$ff, (a0)                   ;" Now the lock is ours
            clr.l   d0                           ;" And return FALSE
denied:     nop
            rts

rel_flag:
            clr.b   (a0)                         ;" Clear semaphor
            rts

;-------------------------------------------------------------------------------------)

get_pending:
            move.w  sr, -(sp)                    ;" Save CPU status
            or.w    #$0700, sr                   ;" Disable interupts
            move.l  (a0), -(sp)                  ;" Get value
            clr.l   (a0)                         ;" And reset it
            move.l  (sp)+, a0                    ;" Return value
            move.w  (sp)+, sr                    ;" Restore CPU status
            rts

;-------------------------------------------------------------------------------------)

check_sum:
            and.b   #$ef, ccr                    ;" Clear extended carry
            addx.l  d1, d0                       ;" Add dest_ip to src_ip
            move.l  #$110000, d1                 ;" Prepare UDP pseudo header word
            move.w  d2, d1                       ;" Length for pseudo header word
            addx.l  d1, d0                       ;" Add it to accumulator
            clr.l   d1                           ;" For adding carries
            addx.l  d1, d0                       ;" Add carry to the accumulator
            move.b  d2, remain                   ;" Save last two bits of length
            and.l   #$ffff, d2                   ;" Extend to longword
            move.l  d2, d1                       ;" Save length
            lsr.l   #6, d1                       ;" Length by 64 is loop count
            and.l   #$3c, d2                     ;" Find fraction of a chunk (loop)
            neg.l   d2                           ;" Negate to offset beyond loop body
            and.b   #$ef, ccr                    ;" Clear extended carry
            lea     lp_end, a1                   ;" End of loop body
            jmp     (a1, d2)                     ;" Jump into loop

loop:       move.l  (a0)+, d2                    ;" Fetch  1st longword (4 bytes)
            addx.l  d2, d0                       ;" Add it to accumulator
            move.l  (a0)+, d2                    ;" Fetch  2nd longword (4 bytes)
            addx.l  d2, d0                       ;" Add it to accumulator
            move.l  (a0)+, d2                    ;" Fetch  3rd longword (4 bytes)
            addx.l  d2, d0                       ;" Add it to accumulator
            move.l  (a0)+, d2                    ;" Fetch  4th longword (4 bytes)
            addx.l  d2, d0                       ;" Add it to accumulator
            move.l  (a0)+, d2                    ;" Fetch  5th longword (4 bytes)
            addx.l  d2, d0                       ;" Add it to accumulator
            move.l  (a0)+, d2                    ;" Fetch  6th longword (4 bytes)
            addx.l  d2, d0                       ;" Add it to accumulator
            move.l  (a0)+, d2                    ;" Fetch  7th longword (4 bytes)
            addx.l  d2, d0                       ;" Add it to accumulator
            move.l  (a0)+, d2                    ;" Fetch  8th longword (4 bytes)
            addx.l  d2, d0                       ;" Add it to accumulator
            move.l  (a0)+, d2                    ;" Fetch  9th longword (4 bytes)
            addx.l  d2, d0                       ;" Add it to accumulator
            move.l  (a0)+, d2                    ;" Fetch 10th longword (4 bytes)
            addx.l  d2, d0                       ;" Add it to accumulator
            move.l  (a0)+, d2                    ;" Fetch 11th longword (4 bytes)
            addx.l  d2, d0                       ;" Add it to accumulator
            move.l  (a0)+, d2                    ;" Fetch 12th longword (4 bytes)
            addx.l  d2, d0                       ;" Add it to accumulator
            move.l  (a0)+, d2                    ;" Fetch 13th longword (4 bytes)
            addx.l  d2, d0                       ;" Add it to accumulator
            move.l  (a0)+, d2                    ;" Fetch 14th longword (4 bytes)
            addx.l  d2, d0                       ;" Add it to accumulator
            move.l  (a0)+, d2                    ;" Fetch 15th longword (4 bytes)
            addx.l  d2, d0                       ;" Add it to accumulator
            move.l  (a0)+, d2                    ;" Fetch 16th longword (4 bytes)
            addx.l  d2, d0                       ;" Add it to accumulator
lp_end:
            dbra    d1, loop                     ;" Loop it

            move.l  d0, d1                       ;" Save high word
            swap    d1                           ;" Transfer it to low word
            addx.w  d1, d0                       ;" Add both words
            btst    #1, remain                   ;" Do we have 2 or 3 remaining bytes ?
            beq     no_word                      ;" No ...
            move.w  (a0)+, d2                    ;" Else fetch another word
            addx.w  d2, d0                       ;" And add it to the accumulator
no_word:    btst    #0, remain                   ;" Do we have another remaining byte ?
            beq     no_byte                      ;" No ...
            move.w  (a0)+, d2                    ;" Else fetch another word
            clr.b   d2                           ;" But only use upper byte
            addx.w  d2, d0                       ;" And add it to the accumulator
no_byte:    clr.w   d2                           ;" For adding carries
            addx.w  d2, d0                       ;" Add carry to the accumulator
            addx.w  d2, d0                       ;" Again if another overflow
            and.l   #$ffff, d0                   ;" And truncate to word length
            eor.w   #$ffff, d0                   ;" Invert the result
            bne     chk_ok                       ;" Is it non-Zero ?
            move.w  #$ffff, d0                   ;" No : Use -0 instead
chk_ok:     nop                                  ;" Done
            rts

remain:     ds.w    1                            ;" Storage for last two length bits

