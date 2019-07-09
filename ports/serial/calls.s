/*********************************************************************
 *                                                                   *
 *     Low Level Port : Serielle Schnittstellen                      *
 *                                                                   *
 *                                                                   *
 *      Version 1.0                     from 28. September 1997      *
 *                                                                   *
 *      Module for assembler subroutines                             *
 *                                                                   *
 *********************************************************************/


             .globl    execute                   /* Function to execute status calls */
             .globl    inq_cd                    /* Function for inquiring DCD status */
             .globl    set_dtr                   /* Function for setting DTR status */
             .globl    choose_magic              /* Function for getting a random magic */

			 .globl bconmap_bconstat
			 .globl bconmap_bcostat
			 .globl bconmap_read
			 .globl bconmap_write

/* ------------------------------------------------------------------------------------- */

bconmap_bconstat:
			movea.l    20(a0),a1
			moveq.l    #0,d0
			move.w     8(a1),d0
			sub.w      6(a1),d0
			bcc.s      bconmap_bconstat1
			add.w      4(a1),d0
bconmap_bconstat1:
			rts

/* ------------------------------------------------------------------------------------- */

bconmap_bcostat:
			movea.l    20(a0),a1
			moveq.l    #0,d0
			move.w     20(a1),d0
			sub.w      22(a1),d0
			bhi.s      bconmap_bcostat1
			add.w      18(a1),d0
bconmap_bcostat1:
			subq.w     #2,d0
			bpl.s      bconmap_bcostat2
			moveq.l    #0,d0
bconmap_bcostat2:
			rts

/* ------------------------------------------------------------------------------------- */

/* long bconmap_read(MAPTAB *handler, long count, uint8 *buffer); */
bconmap_read:
			movem.l    a2-a4,-(a7)               /* Save CPU register */
			movea.l    a1,a3
			movea.l    a0,a4
			movea.l    20(a4),a2                 /* get IOREC pointer */
			move.w     4(a2),d2
			moveq.l    #0,d1
			move.w     8(a2),d1
			sub.w      6(a2),d1
			bcc.s      bconmap_read1
			add.w      d2,d1
bconmap_read1:
			cmp.l      d0,d1
			bcc.s      bconmap_read2
			move.l     d1,d0
bconmap_read2:
			move.l     d0,-(a7)
			beq.s      bconmap_read6
			subq.l     #2,d0
			bcs.s      bconmap_read5
			movea.l    (a2),a0
			move.w     6(a2),d1
bconmap_read3:
			addq.w     #1,d1
			cmp.w      d2,d1
			bcs.s      bconmap_read4
			moveq.l    #0,d1
bconmap_read4:
			move.b     0(a0,d1.l),(a3)+
			dbf        d0,bconmap_read3
			move.w     d1,6(a2)
bconmap_read5:
			movea.l    4(a4),a1
			jsr        (a1)
			move.b     d0,(a3)
bconmap_read6:
			movem.l    (a7)+,d0/a2-a4            /* Restore CPU register */
			rts

/* ------------------------------------------------------------------------------------- */

/* long bconmap_write(MAPTAB *handler, long count, const uint8 *buffer); */

bconmap_write:
			movem.l    a2/a4,-(a7)
			movea.l    a0,a4
			movea.l    20(a4),a2
			move.w     18(a2),d2
			moveq.l    #0,d1
			move.w     20(a2),d1
			sub.w      22(a2),d1
			bhi.s      bconmap_write1
			add.w      d2,d1
bconmap_write1:
			subq.l     #2,d1
			bpl.s      bconmap_write2
			moveq.l    #0,d1
bconmap_write2:
			cmp.l      d0,d1
			bcc.s      bconmap_write3
			move.l     d1,d0
bconmap_write3:
			move.l     d0,-(a7)
			beq.s      bconmap_write7
			subq.l     #2,d0
			bcs.s      bconmap_write6
			movea.l    14(a2),a0
			move.w     22(a2),d1
bconmap_write4:
			addq.w     #1,d1
			cmp.w      d2,d1
			bcs.s      bconmap_write5
			moveq.l    #0,d1
bconmap_write5:
			move.b     (a1)+,0(a0,d1.l)
			dbf        d0,bconmap_write4
			move.w     d1,22(a2)
bconmap_write6:
			move.b     (a1),d0
			move.l     d0,-(a7)
			movea.l    12(a4),a1
			jsr        (a1)
			addq.l     #4,a7
bconmap_write7:
			movem.l    (a7)+,d0/a2/a4
			rts

/* ------------------------------------------------------------------------------------- */

execute:
             movem.l   d3-d7/a2-a6,cpu_state     /* Save CPU register */
             jsr       (a0)                      /* Call status code */
             movem.l   cpu_state,a2-a6/d3-d7     /* Restore CPU register */
             rts

/* ------------------------------------------------------------------------------------- */

inq_cd:
             move.l    #0x5482,d0                /* Do a Fcntl (..., TIOCCTLGET); */
             move.l    #0x0010,cstatus           /* Inquire Carrier Detect */
             bsr       exec_hsm                  /* Call HSMODEM code */
             move.w    #0x0010,d0                /* Preset return to DCD = TRUE */
             tst.l     d0                        /* Error occured ? */
             bmi       cd_set                    /* Yes, return TRUE */
             move.l    cstatus,d0                /* Otherwise get status */
cd_set:      and.w     #0x0010,d0                /* And isolate DCD */
             rts

set_dtr:
             clr.l     cstatus + 4               /* Preset status to DTR = FALSE */
             tst.w     d0                        /* Was that intended ? */
             beq       dtr_ok                    /* Yes, OK */
             move.l    #0x0002,cstatus + 4       /* Otherwise set DTR = TRUE */
dtr_ok:      move.l    #0x5483,d0                /* Do a Fcntl (..., TIOCCTLSET); */
             move.l    #0x0002,cstatus           /* Data Terminal Ready is meant */
             bsr       exec_hsm                  /* Call HSMODEM code */
             rts

exec_hsm:
             movem.l   d3-d7/a2-a6,cpu_state     /* Save CPU register */
             move.l    a0,a2                     /* Command code */
             lea       cstatus,a1                /* Pointer for return value */
             suba.l    a0,a0                     /* No file descriptor */
             jsr       (a2)                      /* Call HSMODEM code */
             movem.l   cpu_state,a2-a6/d3-d7     /* Restore CPU register */
             rts

/* ------------------------------------------------------------------------------------- */

bus_vec      =         0x08
_vblqueue    =         0x456
_frclock     =         0x466
vid_c_hi     =         0xffff8205
vid_c_med    =         0xffff8207
vid_c_low    =         0xffff8209

choose_magic:
             movem.l   d1-d2,-(sp)               /* Save D1 and D2 */
             move.w    sr,cstatus                /* Save status word */
             or.w      #0x0700,sr                /* Disable all interrupts */
             move.l    sp,stack                  /* Save stack pointer */
             move.l    bus_vec,saved_bus         /* Save bus error vector */
             move.l    #failure,bus_vec          /* Enable our bus error handler */

             move.b    vid_c_low,d0              /* Video counter low byte */
             asl.l     #8,d0                     /* In bits 8 bis 15 */
             move.b    vid_c_med,d0              /* Video counter medium byte */
             asl.l     #8,d0                     /* In bits 8 bis 15 */
             move.b    vid_c_hi,d0               /* Video counter high byte */
             asl.l     #8,d0                     /* Whole counter in D0 */
skip_it:     move.l    _vblqueue,d1              /* Queue pointer, little random */
             eor.l     d1,d0                     /* Exclusive or'd */
             move.l    _frclock,d1               /* Number of screen refreshs */
             move.w    d1,d2                     /* Is pretty random */
             mulu.w    d2,d1                     /* Now squared */
             and.l     #0x1f,d1                  /* But only between 0 and 31 */
             rol.l     d1,d0                     /* Rotate as many times */

             move.l    saved_bus,bus_vec         /* Restore bus error vector */
             move.l    stack,sp                  /* Restore stack pointer */
             move.w    cstatus,sr                /* Restore status word */
             movem.l   (sp)+,d1-d2               /* Restore D1 and D2 */
             rts

failure:
             move.l    stack,sp                  /* Restore stack pointer */
             bra       skip_it                   /* Continue */

/* ------------------------------------------------------------------------------------- */

walk:        ds.l      1                         /* Pointer into buffer */
remain:      ds.w      1                         /* Remaining bytes */
mark:        ds.w      1                         /* End-of-data mark */
code:        ds.l      1                         /* Pointer to code to be called */
cstatus:     ds.l      2                         /* Pointer to check function */
stack:       ds.l      1                         /* Saved stack pointer */
saved_bus:   ds.l      1                         /* Saved bus error vector */

cpu_state:   ds.l      10                        /* Save area for CPU registers */
