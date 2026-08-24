| Copyright (C) 2026 Neil Rackett
| SPDX-License-Identifier: GPL-2.0-or-later

| Top-border overscan display (any ST) support
|
| * Top border removed with a 50/60 Hz switch held across the GLUE's
|   top border test; bottom and side borders left intact
| * 227 seamless lines are displayed (lines 36-262 of the frame), so
|   a 224 line game window fits with 3 lines to spare; leave unused
|   buffer lines as zeros, which display as colour 0 - the border
| * A line only shows video when it runs at the screen's base rate,
|   so the two lines still at 60 Hz while the switch-back is pinned
|   (lines 34-35) display as border colour. They sit at the very top
|   edge, directly against the real border, so unlike the bottom
|   variant's mid-picture seam they are invisible
| * Those two hidden lines still fetch video memory, so the first
|   visible line is buffer row 2: start your content 320 bytes into
|   the screen buffer (visible_offset 2 in the demo)
| * No cycle-counted code: the 60 Hz switch only has to land
|   somewhere in lines 25-33 (a nine line window), which Timer A in
|   delay mode hits from the VBL at any CPU speed since it runs off
|   the MFP's own 2.4576 MHz clock; the 50 Hz restore is pinned to
|   Timer B counting displayed lines (Display Enable events).
|   Works on STF/STFM/Mega ST/STE and a Mega STE at 16 MHz with cache
| * As a safety net the VBL forces 50 Hz at the top of every frame,
|   so a missed window can never leave the screen stuck in 60 Hz
| * 160 byte linewidth, 320 visible pixels per line
| * If you use the blitter, keep it in shared mode: a long hog-mode
|   blit can stall the CPU past the timer windows

		.text
		.global	_overscan_top_setup
		.global	_overscan_top_restore
		.global	overscan_top_vbl
		.global	overscan_top_timer_a
		.global	vblcnt
		.global	_vblcnt
		.global	scraddr1
		.global	_scraddr1
		.global	scraddr2
		.global	_scraddr2
		.global	backbuf_flag
		.global	_backbuf_flag

overscan_top_vbl:	movem.l	d0-a6,-(sp)

		move.b	#2,0xffff820a.w			|Safety net: make sure each
						|frame starts back at 50 Hz

		move.l	scraddr1,d0			|Set screen address (high+mid
		lsr.w	#8,d0				|bytes only, so 256-byte aligned:
		move.l	d0,0xffff8200.w			|works on any ST)

		clr.b	0xfffffa19.w			|Stop Timer A
		clr.b	0xfffffa1b.w			|Stop Timer B
		move.b	#90,0xfffffa1f.w		|Timer A: ~1.83 ms from now,
		move.b	#4,0xfffffa19.w			|delay mode /50 -> lands ~line 29

		move.l	scraddr1,d0			|Swap screens
		move.l	scraddr2,scraddr1
		move.l	d0,scraddr2
		eori.b	#1,backbuf_flag			|Track back buffer

		addq.w	#1,vblcnt

		movem.l	(sp)+,d0-a6
		rte

|--------------------------------------------------------
|		Timer A fires in the vertical blank around
|		line 29. Switching to 60 Hz here makes the
|		GLUE start the picture at line 34 instead of
|		line 63: the top border is gone. Timer B is
|		then used as a silent line counter (interrupt
|		masked, data register read directly): two
|		Display Enable ticks pin us to the blanking
|		gap after line 35, where 50 Hz is restored
|		and every line from 36 down to the normal
|		bottom border displays seamlessly.
overscan_top_timer_a:
		movem.l	d0-d2/a0,-(sp)

		clr.b	0xfffffa19.w			|Stop Timer A
		clr.b	0xffff820a.w			|60 Hz: picture starts at line 34

		lea	0xfffffa21.w,a0			|Timer B data register
		clr.b	0xfffffa1b.w			|Timer B: silent Display Enable
		move.b	#200,(a0)			|counter, event count mode
		move.b	#8,0xfffffa1b.w

		bsr.s	wait_de				|End of line 34
		bsr.s	wait_de				|End of line 35

		move.b	#2,0xffff820a.w			|50 Hz: lines 36+ all display

		clr.b	0xfffffa1b.w			|Stop Timer B

		movem.l	(sp)+,d0-d2/a0
		rte

|--------------------------------------------------------
|		Wait for Timer B to tick: the end of the next
|		displayed line. Bounded so a missed window can
|		never hang the machine; returns N set on
|		timeout, N clear on success.
wait_de:	move.w	#2000,d2
		move.b	(a0),d1
.wde:		cmp.b	(a0),d1
		bne.s	.wok
		dbra	d2,.wde
.wok:		tst.w	d2
		rts


_overscan_top_setup:
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
		move.b	0xfffffa07.w,(a0)+
		move.b	0xfffffa09.w,(a0)+
		move.b	0xfffffa13.w,(a0)+
		move.b	0xfffffa15.w,(a0)+
		move.b	0xfffffa17.w,(a0)+
		move.b	0xfffffa19.w,(a0)+
		move.b	0xfffffa1f.w,(a0)+
		move.b	0xfffffa1b.w,(a0)+
		move.b	0xfffffa21.w,(a0)+

		move.l	#overscan_top_vbl,0x70.w
		move.l	#dummy,0x68.w
		move.l	#overscan_top_timer_a,0x134.w
		move.l	#dummy,0x120.w

		move.b	#0x20,0xfffffa07.w		|Interrupt enable A: Timer A only
		clr.b	0xfffffa09.w			|Interrupt enable B
		move.b	#0x20,0xfffffa13.w		|Interrupt mask A: Timer A only
		clr.b	0xfffffa15.w			|Interrupt mask B
		bclr	#3,0xfffffa17.w			|Automatic end of interrupt
		clr.b	0xfffffa19.w			|Timer A control (VBL arms it)
		clr.b	0xfffffa1f.w			|Timer A data
		clr.b	0xfffffa1b.w			|Timer B control
		clr.b	0xfffffa21.w			|Timer B data

		move.w	#0x2300,sr

		rts

_overscan_top_restore:
		move.w	#0x2700,sr			|Restore vectors and MFP
		clr.b	0xfffffa19.w			|Stop Timer A
		clr.b	0xfffffa1b.w			|Stop Timer B
		lea	save_irq,a0
		move.l	(a0)+,0x70.w
		move.l	(a0)+,0x68.w
		move.l	(a0)+,0x134.w
		move.l	(a0)+,0x120.w
		move.b	(a0)+,0xfffffa07.w
		move.b	(a0)+,0xfffffa09.w
		move.b	(a0)+,0xfffffa13.w
		move.b	(a0)+,0xfffffa15.w
		move.b	(a0)+,0xfffffa17.w
		move.b	(a0)+,0xfffffa19.w
		move.b	(a0)+,0xfffffa1f.w
		move.b	(a0)+,0xfffffa1b.w
		move.b	(a0)+,0xfffffa21.w
		move.w	#0x2300,sr

		move.l	save_scr,0xffff8200.w		|Restore screen address
		move.b	save_res,0xffff8260.w		|Restore resolution
		move.b	save_frq,0xffff820a.w		|Restore refresh
		rts

dummy:		rte


		.data

_vblcnt:
vblcnt:		.word	0

		.bss

_scraddr1:
scraddr1:	.space	4
_scraddr2:
scraddr2:	.space	4
save_scr:	.space	4
save_irq:	.space	25
save_res:	.space	1
save_frq:	.space	1
_backbuf_flag:
backbuf_flag:	.space	1
