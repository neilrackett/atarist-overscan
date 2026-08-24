| Copyright (C) 2026 Neil Rackett
| SPDX-License-Identifier: GPL-2.0-or-later

| Bottom-border overscan display (any ST) support
|
| * Bottom border removed with the classic Timer B 50/60 Hz trick;
|   top and side borders left intact
| * 245 lines are visible once the border opens (picture lines 0-244);
|   leave everything below your content as zeros, which display as
|   colour 0 - the same as the border
| * KNOWN LIMITATION: picture line 200 (the first border line) shows
|   as one blank line of border colour. The GLUE evaluates its
|   vertical display window per line against the current sync rate,
|   and the line that runs at 60 Hz while the border test is fooled
|   falls outside the 60 Hz window. Cycle-counted routines dodge this
|   by restoring 50 Hz within a ~60 cycle window at the end of line
|   262, which interrupt-driven code cannot hit reliably at every CPU
|   speed. Align a HUD split or dark band with line 200, or use the
|   top-border variant (overscan_top.s) for a seamless window
| * The hidden line still fetches video memory, so buffer row 200 is
|   consumed but never shown: to display 224 full rows of content,
|   draw rows 0-199 at buffer rows 0-199 and rows 200-223 at buffer
|   rows 201-224
| * No cycle-counted code: Timer B in event count mode ticks once per
|   displayed line (it counts the Display Enable signal), so it fires
|   on an exact scanline at any CPU speed, and every sub-line wait
|   below is pinned to hardware events rather than instruction timing.
|   Works on STF/STFM/Mega ST/STE and a Mega STE at 16 MHz with cache
| * Both 0xffff820a writes land in the blanking gap right after a
|   line's display ends, so every line runs at a single sync rate
|   through both of its horizontal comparators - no "weird length"
|   hybrid lines that would shift the rest of the frame
| * 160 byte linewidth, 320 visible pixels per line
| * If you use the blitter, keep it in shared mode: a long hog-mode
|   blit can stall the CPU past the Timer B window

		.text
		.global	_overscan_bottom_setup
		.global	_overscan_bottom_restore
		.global	overscan_bottom_vbl
		.global	overscan_bottom_timer_b
		.global	vblcnt
		.global	_vblcnt
		.global	scraddr1
		.global	_scraddr1
		.global	scraddr2
		.global	_scraddr2
		.global	backbuf_flag
		.global	_backbuf_flag

overscan_bottom_vbl:	movem.l	d0-a6,-(sp)

		move.l	scraddr1,d0			|Set screen address (high+mid
		lsr.w	#8,d0				|bytes only, so 256-byte aligned:
		move.l	d0,0xffff8200.w			|works on any ST)

		clr.b	0xfffffa1b.w			|Stop Timer B
		move.b	#198,0xfffffa21.w		|Re-arm: fire near the end of the
		move.b	#8,0xfffffa1b.w			|picture, event count mode

		move.l	scraddr1,d0			|Swap screens
		move.l	scraddr2,scraddr1
		move.l	d0,scraddr2
		eori.b	#1,backbuf_flag			|Track back buffer

		addq.w	#1,vblcnt

		movem.l	(sp)+,d0-a6
		rte

|--------------------------------------------------------
|		Timer B interrupt fires two lines before the
|		end of the picture. Step line by line on the
|		timer's own count, then flick to 60 Hz across
|		the GLUE's bottom border test and back again.
overscan_bottom_timer_b:
		movem.l	d0-d2/a0-a1,-(sp)

		lea	0xfffffa21.w,a0			|Timer B data: ticks once per line
		lea	0xffff820a.w,a1			|Sync rate register

		bsr.s	wait_de				|End of picture line 199
		bmi.s	.exit
		bsr.s	wait_de				|End of picture line 200 (the last)
		bmi.s	.exit

		clr.b	(a1)				|60 Hz across the border test:
						|the GLUE never turns the border on

		bsr.s	wait_de				|End of first extra line

		move.b	#2,(a1)				|Back to 50 Hz for the rest
						|of the frame

.exit:		movem.l	(sp)+,d0-d2/a0-a1
		rte

|--------------------------------------------------------
|		Wait for Timer B to tick: the end of the next
|		displayed line. Bounded so a missed border can
|		never hang the machine; returns N set on
|		timeout, N clear on success.
wait_de:	move.w	#2000,d2
		move.b	(a0),d1
.wde:		cmp.b	(a0),d1
		bne.s	.wok
		dbra	d2,.wde
.wok:		tst.w	d2
		rts


_overscan_bottom_setup:
		move.b	0xffff8260.w,save_res		|Save res
		clr.b	0xffff8260.w			|Set ST-LOW

		move.b	0xffff820a.w,save_frq		|Save refresh
		move.b	#2,0xffff820a.w			|Set 50 Hz

		move.l	0xffff8200.w,save_scr		|Save screen address

		move.w	#0x2700,sr			|Save/setup vectors and MFP
		lea	save_irq,a0
		move.l	0x70.w,(a0)+
		move.l	0x68.w,(a0)+
		move.l	0x120.w,(a0)+
		move.b	0xfffffa07.w,(a0)+
		move.b	0xfffffa09.w,(a0)+
		move.b	0xfffffa13.w,(a0)+
		move.b	0xfffffa15.w,(a0)+
		move.b	0xfffffa17.w,(a0)+
		move.b	0xfffffa1b.w,(a0)+
		move.b	0xfffffa21.w,(a0)+

		move.l	#overscan_bottom_vbl,0x70.w
		move.l	#dummy,0x68.w
		move.l	#overscan_bottom_timer_b,0x120.w

		move.b	#1,0xfffffa07.w			|Interrupt enable A: Timer B only
		clr.b	0xfffffa09.w			|Interrupt enable B
		move.b	#1,0xfffffa13.w			|Interrupt mask A: Timer B only
		clr.b	0xfffffa15.w			|Interrupt mask B
		bclr	#3,0xfffffa17.w			|Automatic end of interrupt
		clr.b	0xfffffa1b.w			|Timer B control (VBL arms it)
		clr.b	0xfffffa21.w			|Timer B data

		move.w	#0x2300,sr

		rts

_overscan_bottom_restore:
		move.w	#0x2700,sr			|Restore vectors and MFP
		clr.b	0xfffffa1b.w			|Stop Timer B
		lea	save_irq,a0
		move.l	(a0)+,0x70.w
		move.l	(a0)+,0x68.w
		move.l	(a0)+,0x120.w
		move.b	(a0)+,0xfffffa07.w
		move.b	(a0)+,0xfffffa09.w
		move.b	(a0)+,0xfffffa13.w
		move.b	(a0)+,0xfffffa15.w
		move.b	(a0)+,0xfffffa17.w
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
save_irq:	.space	19
save_res:	.space	1
save_frq:	.space	1
_backbuf_flag:
backbuf_flag:	.space	1
