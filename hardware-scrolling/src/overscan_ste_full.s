| Fullscreen display (STe-only) support
|
| * 1 datareg wasted during the overscan
| * 464 cycles (116 nops) free per overscan scanline
| * 224 byte linewidth, 400 visible pixels per line (408 achievable, but goes off right side)

		.text
		.global	_overscan_ste_setup
		.global	_overscan_ste_restore
		.global	overscan_ste_vbl
		.global	overscan_ste_timer_a
		.global	vblcnt
		.global	_vblcnt
		.global	scraddr1
		.global	_scraddr1
		.global	scraddr2
		.global	_scraddr2
		.global	backbuf_flag
		.global	_backbuf_flag
		.global	scroll_offset
		.global	_scroll_offset
		.global	scroll_hpixel
		.global	_scroll_hpixel

overscan_ste_vbl:	movem.l	d0-a6,-(sp)

		move.l	scraddr1,d0			|Get completed frame (just drawn to)
		moveq	#0,d1
		move.w	scroll_offset,d1		|Get scroll offset
		add.l	d1,d0				|Add to full address for parallax
		lsr.w	#8,d0
		move.l	d0,0xffff8200.w			|Set screen address

		move.b	scroll_hpixel,0xffff820f.w	|Set horizontal pixel scroll (STE)

		move.b	#97,0xfffffa1f.w		|Timer A data
		move.b	#4,0xfffffa19.w			|Timer A control

		move.l	scraddr1,d0			|Swap screens
		move.l	scraddr2,scraddr1
		move.l	d0,scraddr2
		eori.b	#1,backbuf_flag			|Track back buffer


		addq.w	#1,vblcnt

		movem.l	(sp)+,d0-a6
		rte

|--------------------------------------------------------
|		Timer A interupt just in time for
|		top border removal.
|		All synced code will run in this interrupt.
overscan_ste_timer_a:
		movem.l	d0-a6,-(sp)

		move.w	#0x2100,sr			|Enable HBL
		stop	#0x2100				|Wait for HBL
		move.w	#0x2700,sr			|Stop all interrupts
		clr.b	0xfffffa19.w			|Stop Timer A

		.fill 	84,2,0x4e71			|Zzz..

		clr.b	0xffff820a.w			|Remove the top border
		.fill 	9,2,0x4e71
		move.b	#2,0xffff820a.w

		lea	0xffff8209.w,a0			|Hardsync
		moveq	#127,d1
.sync:		tst.b	(a0)
		beq.s	.sync
		move.b	(a0),d2
		sub.b	d2,d1
		lsr.l	d1,d1

		.fill	71,2,0x4e71			|Zzz..
		moveq	#2,d7				|d7 used for the overscan code


		|First 227 scanlines
	.rept	227
		move.b	d7,0xffff8260.w			|Left border
		move.w	d7,0xffff8260.w
		.fill	90,2,0x4e71
		move.w	d7,0xffff820a.w			|Right border
		move.b	d7,0xffff820a.w
		.fill	26,2,0x4e71
	.endr

		|Two special lines for lower border
		move.b	d7,0xffff8260.w			|Left border
		move.w	d7,0xffff8260.w
		.fill	90,2,0x4e71
		move.w	d7,0xffff820a.w			|Right border
		move.b	d7,0xffff820a.w
		.fill	23,2,0x4e71
		|-----------------------------------
		move.w	d7,0xffff820a.w			|Lower border
		move.b	d7,0xffff8260.w			|Left border
		move.w	d7,0xffff8260.w
		move.b	d7,0xffff820a.w
		.fill	87,2,0x4e71
		move.w	d7,0xffff820a.w			|Right border
		move.b	d7,0xffff820a.w
		.fill	26,2,0x4e71

		|Ending 44 lines
	.rept	44
		move.b	d7,0xffff8260.w			|Left border
		move.w	d7,0xffff8260.w
		.fill	90,2,0x4e71
		move.w	d7,0xffff820a.w			|Right border
		move.b	d7,0xffff820a.w
		.fill	26,2,0x4e71
	.endr

		movem.l	(sp)+,d0-a6
		rte


_overscan_ste_setup:
		move.b	0xffff8260.w,save_res		|Save res
		clr.b	0xffff8260.w			|Set ST-LOW

		move.b	0xffff820a.w,save_frq		|Save refresh
		move.b	#2,0xffff820a.w			|Set 50 Hz

		move.l	0xffff8200.w,save_scr		|Save screen address

		move.w	#0x2700,sr			|Save/setup vectors and MFP
		lea	save_irq,a0
		move.l	0x70.w,(a0)+
		move.l	0x68.w,(a0)+
		move.l	0x134.w,(a0)+
		move.l	0x120.w,(a0)+
		move.l	0x114.w,(a0)+
		move.l	0x110.w,(a0)+
		move.l	0x118.w,(a0)+
		move.b	0xfffffa07.w,(a0)+
		move.b	0xfffffa09.w,(a0)+
		move.b	0xfffffa15.w,(a0)+
		move.b	0xfffffa17.w,(a0)+
		move.b	0xfffffa07.w,(a0)+
		move.b	0xfffffa13.w,(a0)+
		move.b	0xfffffa19.w,(a0)+
		move.b	0xfffffa1f.w,(a0)+

		move.l	#overscan_ste_vbl,0x70.w
		move.l	#dummy,0x68.w
		move.l	#overscan_ste_timer_a,0x134.w
		move.l	#dummy,0x120.w
		move.l	#dummy,0x114.w
		move.l	#dummy,0x110.w
		move.l	#dummy,0x118.w

		bset	#5,0xfffffa07.w			|Interrupt enable A
		clr.b	0xfffffa09.w			|Interrupt enable B
		bset	#5,0xfffffa13.w			|Interrupt mask A
		clr.b	0xfffffa15.w			|Interrupt mask B
		bclr	#3,0xfffffa17.w			|Automatic end of interrupt
		clr.b	0xfffffa19.w			|Timer A control
		clr.b	0xfffffa1f.w			|Timer A data

		move.w	#0x2300,sr

		rts

_overscan_ste_restore:
		move.w	#0x2700,sr			|Restore vectors and MFP
		lea	save_irq,a0
		move.l	(a0)+,0x70.w
		move.l	(a0)+,0x68.w
		move.l	(a0)+,0x134.w
		move.l	(a0)+,0x120.w
		move.l	(a0)+,0x114.w
		move.l	(a0)+,0x110.w
		move.l	(a0)+,0x118.w
		move.b	(a0)+,0xfffffa07.w
		move.b	(a0)+,0xfffffa09.w
		move.b	(a0)+,0xfffffa15.w
		move.b	(a0)+,0xfffffa17.w
		move.b	(a0)+,0xfffffa07.w
		move.b	(a0)+,0xfffffa13.w
		move.b	(a0)+,0xfffffa19.w
		move.b	(a0)+,0xfffffa1f.w
		move.w	#0x2300,sr

		move.l	save_scr,0xffff8200.w		|Restore screen address
		move.b	save_res,0xffff8260.w		|Restore resolution
		move.b	save_frq,0xffff820a.w		|Restore refresh
		rts

dummy:		rte


		.data

_vblcnt:
vblcnt:		.word	0
_scroll_offset:
scroll_offset:	.word	0
_scroll_hpixel:
scroll_hpixel:	.byte	0
		.even

		.bss

_scraddr1:
scraddr1:	.space	4
_scraddr2:
scraddr2:	.space	4
save_scr:	.space	4
save_irq:	.space	36
save_res:	.space	1
save_frq:	.space	1
_backbuf_flag:
backbuf_flag:	.space	1
