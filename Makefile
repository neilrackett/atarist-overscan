# Copyright (C) 2026 Neil Rackett
# SPDX-License-Identifier: GPL-2.0-or-later

.PHONY: any-st blitter hardware-scrolling all clean

all: any-st blitter hardware-scrolling

any-st:
	$(MAKE) -C any-st

blitter:
	$(MAKE) -C blitter

hardware-scrolling:
	$(MAKE) -C hardware-scrolling

clean:
	$(MAKE) -C any-st clean
	$(MAKE) -C blitter clean
	$(MAKE) -C hardware-scrolling clean
