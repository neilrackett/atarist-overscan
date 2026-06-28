# Copyright (C) 2026 Neil Rackett
# SPDX-License-Identifier: GPL-2.0-or-later

.PHONY: blitter hardware-scrolling all clean

all: blitter hardware-scrolling

blitter:
	$(MAKE) -C blitter

hardware-scrolling:
	$(MAKE) -C hardware-scrolling

clean:
	$(MAKE) -C blitter clean
	$(MAKE) -C hardware-scrolling clean
