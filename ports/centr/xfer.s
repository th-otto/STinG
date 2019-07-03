/*********************************************************************/
/*                                                                   */
/*     Low Level Port : Centronics Schnittstelle                     */
/*                                                                   */
/*      Version 0.2                        vom 28. Januar 1997       */
/*                                                                   */
/*********************************************************************/


            .globl    send_dgram                 /* Function for start sending buffer */
            .globl    recve_dgram                /* Function for start receiving buffer */
            .globl    activate                   /* Function for installing */
            .globl    deactivate                 /* Function for deinstalling */

            .xref     init_timer                 /* Initial value of timeout counter */
            .xref     sending                    /* Timeout counter, and flag */
            .xref     send_pend                  /* Flag for pending buffer to be sent */
            .xref     receiving                  /* Flag indicating receive process */
            .xref     timeout                    /* Low level timeout initializer */
            .xref     send_buffer                /* Buffer for packet to be sent */
            .xref     recve_buffer               /* Buffer for received packet */
            .xref     send_length                /* Length of send buffer contents */
            .xref     recve_length               /* Length of receive buffer contents */
            .xref     rcv_max_length             /* Maximum length of receive buffer */


/* ------------------------------------------------------------------------------------- */

SLIP_END    =       192

_hz_200     =       0x4ba

_yamaha     =       0xffff8800
io_ctrl     =       7
io_a        =       14
io_b        =       15

_mfp        =       0xfffffa01
mfp_ierb    =       _mfp + 8
mfp_iprb    =       _mfp + 12
mfp_isrb    =       _mfp + 16
mfp_imrb    =       _mfp + 20

/* ------------------------------------------------------------------------------------- */

send_dgram:
            clr.w   send_pend
                 /* set sending auf init_timer, set send_pend auf FALSE, send SLIP_END */
            rts

recve_dgram:
                 /* Nur start empfangen ! Set receiving auf TRUE. */
            rts

/* ------------------------------------------------------------------------------------- */

activate:
            rts

deactivate:
            rts

/* ------------------------------------------------------------------------------------- */

centr_interupt:
            rte

/* ------------------------------------------------------------------------------------- */

send_packet:
            rts

/* ------------------------------------------------------------------------------------- */

receive_packet:
            movem.l d2/a2,-(sp)
            move.l  recve_buffer,a2
            move.w  rcv_max_length,d2
            sub.w   #1,d2

recpkt_lp:  move.w  timeout,d0
            bsr     receive_byte
            tst.w   d0
            bpl     rec_tout
            cmp.b   #SLIP_END,d0
            beq     rec_ok
            move.b  d0,(a2)+
            dbra    d2,recpkt_lp
            move.l  #-2,d0
            movem.l (sp)+,d2/a2
            rts

rec_ok:     cmp.l   recve_buffer,a2
            beq     recpkt_lp
            sub.l   recve_buffer,a2
            move.l  a2,recve_length
            clr.l   d0
            movem.l (sp)+,d2/a2
            rts

rec_tout:   move.l  #-1,d0
            movem.l (sp)+,d2/a2
            rts

receive_byte:
            move.l  _hz_200,a0
busy_lp:    btst    #0,mfp_iprb
            bne     rec_data
            move.l  _hz_200,d1
            sub.l   a0,d1
            cmp.l   d0,d1
            ble     busy_lp
            clr.w   d0
            rts

rec_data:   move.b  #0xfe,mfp_iprb
            move.w  sr,-(sp)
            or.w    #0x0700,sr
            lea     _yamaha,a0
            move.b  #io_b,(a0)
            move.w  #0xff00,d0
            move.b  (a0),d0
            move.b  #io_a,(a0)
            move.b  (a0),d1
            bclr    #5,d1
            move.b  d1,2(a0)
            nop
            nop
            bset    #5,d1
            move.b  d1,2(a0)
            move.w  (sp)+,sr
            rts
